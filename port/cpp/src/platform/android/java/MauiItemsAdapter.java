// MauiItemsAdapter — the Java half of the RecyclerView items host.
//
// WHY A JAVA CLASS AT ALL. RecyclerView drives its content through an abstract Java class
// (RecyclerView.Adapter), and JNI cannot subclass a Java type: NewObject/CallVoidMethod only CALL Java, they
// cannot supply an implementation of one. So the adapter has to exist as real Java, and the C++ side becomes
// the thing it calls. That is the same shape MauiDialogBridge already uses for the click/dialog callbacks,
// and this class deliberately copies its discipline rather than inventing a second one.
//
// PORTS: src/Controls/src/Core/Handlers/Items/Android/MauiRecyclerView.cs + ItemsViewAdapter — the adapter
// that turns an items source + an ItemTemplate into recycled cells. MAUI's CarouselView is
// MauiCarouselRecyclerView (CarouselViewHandler.Android.cs:26-28), a RecyclerView subclass, NOT a
// ViewPager2; paging comes from SnapHelpers/SnapManager.cs attaching a PagerSnapHelper.
//
// THE PEER IS AN OPAQUE TOKEN, never a pointer Java dereferences. The native side resolves it through its
// live-peer registry, so a callback arriving after the owning handler has been torn down resolves to
// nothing and returns — see android_dialog_ops.hpp's four lifetime rules, which this class relies on.
//
// AN ADAPTER IS HARDER THAN A DIALOG, and the difference is why the rules matter more here. A dialog
// callback fires once; these fire REPEATEDLY on the UI thread throughout a fling, and RecyclerView caches
// and reuses the Views it is given. Two consequences the native side must honour:
//   - onBindViewHolder can arrive for a position whose data has already changed, or after teardown. It is
//     always safe to bind nothing: an empty container renders blank rather than crashing.
//   - the holder's itemView is owned by RECYCLERVIEW, not by the port. The native side must fill it and
//     must never retain it past a bind — hence the container-and-fill shape below rather than handing
//     RecyclerView a View the C++ side owns. A port-owned View handed to the recycler would be freed
//     underneath its cache, which is the crash this design exists to avoid.
package dev.mauicpp;

import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.PagerSnapHelper;
import androidx.recyclerview.widget.RecyclerView;

public final class MauiItemsAdapter extends RecyclerView.Adapter<MauiItemsAdapter.Holder> {
    private final long peer;

    public MauiItemsAdapter(long peer) {
        this.peer = peer;
    }

    // The cell container. A MauiLayout (the port's own ViewGroup) rather than a FrameLayout, because the
    // native side already knows how to add and frame children into one.
    //
    // ITS crossPlatformPeer IS DELIBERATELY LEFT AT 0, and that is load-bearing rather than incidental.
    // MauiLayout.onLayout RE-RUNS the cross-platform arrange when a peer is installed (it must, or a
    // nested layout collapses on a late requestLayout — see MauiLayout's header). A CELL must not do that:
    // its children are framed by the port during nativeBindHolder, against the page rect the LayoutManager
    // hands it, and a second arrange driven from a layout_handler this container does not have would fight
    // that. With the peer at 0 onLayout is the documented unwired no-op, so the explicit frames stand.
    // onMeasure is safe either way — it resolves the incoming spec and reports it, never measuring children.
    // If a future change installs a peer here, this cell's layout has to be rethought, not just rewired.
    public static final class Holder extends RecyclerView.ViewHolder {
        Holder(@NonNull MauiLayout container) {
            super(container);
        }

        MauiLayout container() {
            return (MauiLayout) itemView;
        }
    }

    @NonNull
    @Override
    public Holder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        final MauiLayout container = new MauiLayout(parent.getContext());
        // MATCH_PARENT on both axes: CreateCarouselLayout sizes every carousel item to
        // FractionalWidth(1)/FractionalHeight(1), i.e. one item per page filling the viewport. The
        // LayoutManager supplies the page-sized bounds; the container just fills whatever it is given.
        container.setLayoutParams(new RecyclerView.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        return new Holder(container);
    }

    @Override
    public void onBindViewHolder(@NonNull Holder holder, int position) {
        // The container is REUSED across positions, so it must be emptied before the native side refills
        // it — otherwise a recycled cell shows the previous item's children underneath the new ones.
        holder.container().removeAllViews();
        nativeBindHolder(peer, position, holder.container());
    }

    @Override
    public void onViewRecycled(@NonNull Holder holder) {
        // Drop the children as soon as the cell leaves the screen: they are port-realized views whose
        // lifetime the native side tracks, and leaving them parented to a cached holder would keep them
        // reachable from Java after the port has released them.
        holder.container().removeAllViews();
        nativeRecycleHolder(peer, holder.getAbsoluteAdapterPosition());
        super.onViewRecycled(holder);
    }

    @Override
    public int getItemCount() {
        return nativeGetItemCount(peer);
    }

    // Install this adapter on a RecyclerView as a PAGER: one item per page, snapped.
    //
    // ALL THREE PIECES ARE ATTACHED HERE, IN JAVA, ON PURPOSE. Both the LayoutManager and the scroll
    // listener are abstract-or-open Java classes, so a C++ caller would need a JNI-side subclass of each —
    // and JNI cannot subclass (the reason MauiItemsAdapter exists at all). Doing it in one Java method keeps
    // the native seam at two calls instead of a dozen, and keeps the snap helper and the listener that reads
    // it in the same scope, which is what makes the settled position trustworthy.
    //
    // PORTS SnapHelpers/SnapManager.cs: MAUI's CarouselView paging is a PagerSnapHelper on a plain
    // RecyclerView (MauiCarouselRecyclerView, CarouselViewHandler.Android.cs:26-28), NOT a ViewPager2.
    // PagerSnapHelper is what makes a fling settle on exactly one page boundary — a fling with no snap
    // helper stops wherever momentum runs out, which is precisely the "does not move like MAUI" symptom.
    //
    // A STOCK PagerSnapHelper here is NOT what MAUI ships, and the difference is visible at rest, not just
    // mid-motion. CarouselView.cs:351-352 wires SnapPointsType.MandatorySingle + Center, which
    // SnapManager.CreateSnapHelper (SnapHelpers/SnapManager.cs) turns into SingleSnapHelper — a
    // PagerSnapHelper SUBCLASS whose findSnapView only returns non-null once findTargetSnapPosition has
    // latched a target, and AndroidX only calls findTargetSnapPosition from the FLING path
    // (SnapHelper.onFling -> snapFromFling), never from the plain idle-settle path
    // (onScrollStateChanged(IDLE) -> snapToTargetExistingView -> findSnapView). A release below the fling
    // velocity threshold — e.g. a deliberate drag held still for a few ms before lifting — never fires a
    // fling, so MAUI's real carousel performs NO corrective scroll at all and parks wherever the finger let
    // go, peek and all. A stock PagerSnapHelper has no such latch: findSnapView always returns the nearest
    // child, so the port ALWAYS snapped fully regardless of release velocity — which is what produced
    // carousel_page's ~32%-peek diff against MAUI's own settled frame (confirmed live on-device against
    // the real MauiReference app: `input motionevent` DOWN/MOVE.../dwell/UP — zero release velocity, same
    // technique tools/parity/lib/capture_android.py drives every scenario with — parks MAUI's own carousel
    // exactly like the port now does; `input swipe` — full release velocity, a real fling — pages it fully
    // on both apps). MauiSingleSnapHelper below is a straight port of SingleSnapHelper.cs to reproduce that
    // asymmetry instead of "fixing" it away.
    public void attach(@NonNull RecyclerView recycler, boolean horizontal) {
        final LinearLayoutManager manager = new LinearLayoutManager(
                recycler.getContext(),
                horizontal ? RecyclerView.HORIZONTAL : RecyclerView.VERTICAL,
                false);
        recycler.setLayoutManager(manager);
        recycler.setAdapter(this);

        final PagerSnapHelper snap = new MauiSingleSnapHelper();
        snap.attachToRecyclerView(recycler);

        recycler.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(@NonNull RecyclerView rv, int newState) {
                // SETTLED only. Firing per-frame would write CarouselView.Position dozens of times during a
                // single fling, and every one of those writes re-enters the cross-platform side.
                if (newState != RecyclerView.SCROLL_STATE_IDLE) {
                    return;
                }
                // PORTS RecyclerExtensions.GetCenteredView, as used by CarouselViewOnScrollListener — MAUI
                // computes CarouselView.Position GEOMETRICALLY (whichever child sits under the
                // RecyclerView's own center), entirely independent of the snap helper. That independence
                // matters here: MauiSingleSnapHelper's findSnapView deliberately returns null after a
                // non-fling settle (see the note above), so reading position off the snap helper — as this
                // listener used to — would silently stop reporting Position for exactly the slow-drag case
                // this change exists to handle. FindChildViewUnder(width/2, height/2) never depends on the
                // snap latch, so Position keeps tracking "what's actually centered" whether or not a
                // corrective scroll ran.
                final View centered = rv.findChildViewUnder(rv.getWidth() / 2f, rv.getHeight() / 2f);
                if (centered == null) {
                    return;
                }
                final int position = rv.getChildAdapterPosition(centered);
                if (position == RecyclerView.NO_POSITION) {
                    return;
                }
                onPageSettled(position);
            }
        });
    }

    // Straight port of Controls/src/Core/Handlers/Items/Android/SnapHelpers/SingleSnapHelper.cs (the
    // MandatorySingle+Center case SnapManager.CreateSnapHelper picks for CarouselView's default
    // ItemsLayout). See the long comment on attach() above for WHY this exists instead of a stock
    // PagerSnapHelper: it reproduces MAUI's fling-only corrective-snap asymmetry.
    private static final class MauiSingleSnapHelper extends PagerSnapHelper {
        // currentTargetPosition mirrors SingleSnapHelper.CurrentTargetPosition: RecyclerView.NO_POSITION
        // until the user has actually flung once (or a ScrollTo/item-count change resets it).
        private int currentTargetPosition = RecyclerView.NO_POSITION;
        private int previousItemCount = 0;

        @Override
        public View findSnapView(RecyclerView.LayoutManager layoutManager) {
            if (layoutManager.getItemCount() == 0 || !(layoutManager instanceof LinearLayoutManager)) {
                return null;
            }
            if (currentTargetPosition == RecyclerView.NO_POSITION) {
                return null;
            }
            return ((LinearLayoutManager) layoutManager).findViewByPosition(currentTargetPosition);
        }

        @Override
        public int findTargetSnapPosition(RecyclerView.LayoutManager layoutManager, int velocityX, int velocityY) {
            final int itemCount = layoutManager.getItemCount();
            // Reset if the ItemCount changed is requested — an item could have been added before
            // currentTargetPosition.
            if (previousItemCount != itemCount) {
                currentTargetPosition = RecyclerView.NO_POSITION;
                previousItemCount = itemCount;
            }

            if (currentTargetPosition == RecyclerView.NO_POSITION) {
                currentTargetPosition = super.findTargetSnapPosition(layoutManager, velocityX, velocityY);
                return currentTargetPosition;
            }

            int increment = 1;
            if (layoutManager.canScrollHorizontally()) {
                if (velocityX < 0) {
                    increment = -1;
                }
            } else if (layoutManager.canScrollVertically()) {
                if (velocityY < 0) {
                    increment = -1;
                }
            }
            if (isLayoutReversed(layoutManager)) {
                increment *= -1;
            }

            if (currentTargetPosition == itemCount - 1 && increment == 1) {
                return currentTargetPosition;
            }

            currentTargetPosition += increment;
            return currentTargetPosition;
        }

        private static boolean isLayoutReversed(RecyclerView.LayoutManager layoutManager) {
            if (layoutManager.getLayoutDirection() == View.LAYOUT_DIRECTION_RTL) {
                return true;
            }
            if (layoutManager instanceof LinearLayoutManager) {
                return ((LinearLayoutManager) layoutManager).getReverseLayout();
            }
            return false;
        }
    }

    // notifyDataSetChanged() wrapped rather than called over JNI: it is final on RecyclerView.Adapter, so a
    // native GetMethodID would have to resolve through the superclass, and this is one line.
    public void refresh() {
        notifyDataSetChanged();
    }

    // The initial CarouselView.Position. scrollToPosition (not smoothScrollToPosition) because this is the
    // at-rest starting page, not a user-visible move.
    public static void scrollToPosition(@NonNull RecyclerView recycler, int position) {
        recycler.scrollToPosition(position);
    }

    // Called by the RecyclerView.OnScrollListener the native side installs, once a swipe has SETTLED on a
    // page (SCROLL_STATE_IDLE). This is the CarouselView.Position write-back — MAUI's UpdateFromPosition /
    // UpdateFromCurrentItem round trip. Fired on settle only, never per-frame, so a fling produces one
    // write rather than a stream of them.
    public void onPageSettled(int position) {
        nativeOnPageSettled(peer, position);
    }

    // Bound from C++ via JNIEnv.RegisterNatives before any instance is constructed. Every one takes the
    // peer and validates it native-side against the live-peer registry.
    private static native int nativeGetItemCount(long peer);

    private static native void nativeBindHolder(long peer, int position, View container);

    private static native void nativeRecycleHolder(long peer, int position);

    private static native void nativeOnPageSettled(long peer, int position);
}
