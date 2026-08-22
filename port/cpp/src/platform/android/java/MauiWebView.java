// dev.mauicpp.MauiWebView — the port twin of Microsoft.Maui.Platform.MauiWebView
// (src/Core/src/Platform/Android/MauiWebView.cs), which WebViewHandler.CreatePlatformView
// (src/Core/src/Handlers/WebView/WebViewHandler.Android.cs:23) instantiates instead of a stock
// android.webkit.WebView. C#'s subclass carries the IWebViewDelegate LoadHtml/LoadUrl recipe, which the
// port already drives from JNI; THIS subclass exists for one thing the port cannot get any other way.
//
// THE ASYNC INTRINSIC HEIGHT.
// android.webkit.WebView is the only widget on the board whose intrinsic size is unknown when it is first
// measured: Chromium publishes the document's content size to the Java layer only once the load has
// produced a frame. Measured on emulator-5554 (density 2.75), context_flyout's
// <WebView Source="https://example.com/">, sampling getContentHeight() every 200 ms WITHOUT measuring:
//     t=200ms  contentHeight=0    progress=80
//     t=400ms  contentHeight=187  progress=100      <- appears here, on its own
//     a measure(AT_MOST(1080), UNSPECIFIED) at any later time returns 514 px
// and, decisively, Chromium emits NO requestLayout() when that value lands (the last requestLayout on
// this view arrives before it). So the value is there, silently, and nothing asks to be re-measured.
//
// In MAUI a re-measure comes for free: MAUI's layout is Android's own, so any later system measure
// traversal re-enters LayoutViewGroup.OnMeasure, which re-runs CrossPlatformMeasure
// (src/Core/src/Platform/Android/LayoutViewGroup.cs:96-129) and re-measures the WebView through
// GetDesiredSizeFromHandler. The port drives measure/arrange top-down from C++ and measures each leaf
// exactly once per pass, so a value that arrives between passes is never observed. That is precisely the
// case maui::controls::view::invalidate_measure's header already covers — "this port's hosts drive the
// equivalent top-down remeasure IMPERATIVELY instead" — and this class is that translation.
//
// THE TRIGGER IS THE FRAME, NOT THE PAGE-LOAD CALLBACK. onPageFinished is the obvious hook and it is the
// WRONG one — measured, twice, by two different sessions: it fires ~200 ms BEFORE the content size is
// published, so a re-measure hung off it reads 0 and settles on 0 (07:00:45.783 onPageFinished ->
// measure 1080x0; content height became 187 at ~07:00:46.0, with nothing left to observe it). Chromium
// publishes the content size WITH the frame, so onDraw is the first moment it is readable, and it is the
// only callback that also fires again when a document reflows after being laid out at its new height —
// which is what makes this converge instead of needing a poll or a magic delay. The gate is the content
// height CHANGING, so a settled document (and an animating one, which redraws forever at a fixed content
// height) stops asking after one pass.
//
// A base WebViewClient is installed here as well, for the reason web_view_handler.cpp's
// k_web_view_client_class note gives: with NO client, a server redirect escapes to the system browser.
// create_platform_view therefore skips its own client install on this path.
package dev.mauicpp;

import android.content.Context;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public final class MauiWebView extends WebView {
    // The web_view_platform* whose handler owns this widget; installed by web_view_handler.cpp once the
    // widget is fully built and cleared to 0 on disconnect, before the platform struct can die (the
    // MauiLayout.setCrossPlatformPeer convention). 0 = not wired: every callback below is then a no-op.
    private long peer;
    // The content height the last frame observed. -1 = never drawn, so the first frame always counts as a
    // change; once the document settles this stops matching and the loop ends by itself.
    private int lastContentHeight = -1;
    // One pending invalidate at a time — onDraw fires far more often than the relayout it would schedule,
    // and that relayout re-measures and redraws this view, so an ungated post would never idle.
    private boolean pending;

    private final Runnable invalidate = new Runnable() {
        @Override
        public void run() {
            pending = false;
            if (peer != 0L) {
                nativeInvalidateMeasure(peer);
            }
        }
    };

    public MauiWebView(Context context) {
        super(context);
        setWebViewClient(new WebViewClient());
    }

    public void setPeer(long peer) {
        this.peer = peer;
    }

    // Every frame carries the content size Chromium just published; a change means the last layout pass
    // sized this view from stale content, so ask for another one.
    @Override
    protected void onDraw(android.graphics.Canvas canvas) {
        super.onDraw(canvas);
        final int contentHeight = getContentHeight();
        if (contentHeight != lastContentHeight) {
            lastContentHeight = contentHeight;
            scheduleInvalidate();
        }
    }

    // post() so the relayout never runs inside a live measure/layout traversal.
    private void scheduleInvalidate() {
        if (peer == 0L || pending) {
            return;
        }
        pending = true;
        post(invalidate);
    }

    // Bound from C++ via RegisterNatives (the MauiShapeView.nativeDraw / MauiLayout.nativeArrange recipe —
    // no Java_* export symbol). Asks the cross-platform view to re-measure; the port's window relayout hook
    // re-runs the whole measure/arrange pass, which re-measures this WebView natively.
    private native void nativeInvalidateMeasure(long peer);
}
