// dev.mauicpp.MauiCollectionContent — the inner content host for the Android CollectionView
// (src/platform/android/collection_view_handler.cpp). A ViewGroup that, like MauiLayout, NEVER positions
// its own children (onLayout is a no-op — the CV handler frames each realized item/header/footer ABSOLUTELY
// via child.layout(l,t,r,b), and re-laying-out here would collapse them; the same reason MauiLayout.onLayout
// is empty — see MauiLayout.java).
//
// WHY THIS EXISTS SEPARATELY FROM MauiLayout: this host is the single document child of an
// android.widget.ScrollView. ScrollView.measureChild ALWAYS measures its child with an UNSPECIFIED height
// spec (so the content may exceed the viewport and scroll), IGNORING the child's LayoutParams height.
// MauiLayout.onMeasure returns resolveSize(0, spec), which is ZERO under an UNSPECIFIED spec — so a
// MauiLayout host would collapse to zero height inside the scroller and clip every realized cell. This host
// instead reports its CONTENT extent in onMeasure: the union of its children's right/bottom edges (the
// frames the CV handler already laid out). The scroller then gets the true scrollable extent. On the cross
// axis it honours an Exactly/AtMost spec (the scroller's width) and falls back to the content width for
// Unspecified — never below the children's extent.
package dev.mauicpp;

import android.content.Context;
import android.view.ViewGroup;

public final class MauiCollectionContent extends ViewGroup {
    public MauiCollectionContent(Context context) {
        super(context);
    }

    // No-op: the CollectionView handler positions every child absolutely via child.layout(l,t,r,b).
    // Re-laying-out here would override those frames and collapse the list (same as MauiLayout.onLayout).
    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        // intentionally empty
    }

    // Report the CONTENT extent — the union of the children's already-laid-out right/bottom edges — so a
    // ScrollView (which measures this child with an UNSPECIFIED height spec) gets the real scrollable size
    // rather than zero. Honour an Exactly/AtMost spec per axis, but never report less than the content.
    // MauiLayout now measures the same way (its resolveSize(0, spec) reported 0 under UNSPECIFIED and froze
    // every scrolling page), so both hosts share one implementation.
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(
            MauiLayout.resolveContent(MauiLayout.contentExtent(this, false), widthMeasureSpec),
            MauiLayout.resolveContent(MauiLayout.contentExtent(this, true), heightMeasureSpec));
    }
}
