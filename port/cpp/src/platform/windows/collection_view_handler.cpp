// collection_view_handler — WinUI 3 platform partial: a real Microsoft.UI.Xaml.Controls.ScrollViewer
// whose Content is a Canvas panel, into which the handler realizes a native UIElement per collection
// element and positions each ABSOLUTELY. The windows twin of src/platform/android/
// collection_view_handler.cpp (a real ScrollView->MauiLayout host) and the real-native sibling of the
// headless mirror (src/platform/headless/collection_view_handler.cpp, which the cross-platform
// simulator in src/controls/items/collection_view_handler.cpp still runs on every backend as the
// in-memory state mirror — unaffected by this file).
//
// ORACLE: src/Controls/src/Core/Handlers/Items/{ItemsViewHandler,StructuredItemsViewHandler,
// SelectableItemsViewHandler,GroupableItemsViewHandler,ReorderableItemsViewHandler}.Windows.cs +
// CollectionViewHandler.Windows.cs (an empty partial — the whole surface lives up that chain) +
// src/Core/src/Platform/Windows/*Extensions.cs. MAUI's real Windows CollectionView is a
// ListViewBase/ItemsRepeater with virtualization, item templates (via a WinUI DataTemplate +
// CollectionViewSource), grouping and selection — reproducing that (XAML resource-keyed templates,
// ObservableVector-backed ItemsSource, ICollectionView grouping projections) is explicitly OUT OF
// SCOPE for this wave. What follows instead is a NON-VIRTUALIZED realization: every element is
// realized and absolutely positioned every arrange_native pass (the android partial's exact stance —
// "favor render correctness over recycling" — applied here because the gallery pages have small fixed
// item counts and the parity board needs LAYOUT, not virtualization).
//
// THE REALIZATION MODEL (mirrors the android/apple/ios controller, adapted to the WinUI Canvas seam):
//   - C++ drives layout — the port runs its own cross-platform measure/arrange, and a Canvas positions
//     children absolutely without sizing itself to them, so arrange_native (called on every
//     platform_arrange, before the cross-platform simulator's own refresh_realization — see this file's
//     "LAYOUT SEAM" note below) reads the handler's i_items_view_source + templates DIRECTLY (the
//     ListViewBase-adapter analog) and lays out one native view per element.
//   - An ITEM with an ItemTemplate realizes the template's content as a real native view bound to the
//     item (realize_template_content: create_content -> BindingContext -> create_handler ->
//     set_maui_context -> set_handler — the C# TemplatedCell2.Bind path, identical to the android/apple/
//     ios realize path). A container-rooted template (e.g. header_footer_template's Image+Label stack)
//     is mounted post-order via ensure_mounted (the android boxed-VIEW/chrome mount, ported 1:1) so its
//     own children's native views exist before this file hosts the root. With no template, the DEFAULT
//     cell is a plain TextBlock mirroring item.text() (C#'s DefaultCell2).
//   - HEADER / FOOTER (structured, global) and GROUP HEADER / FOOTER (grouped, per-section) realize the
//     same way, hosted full-cross-width, OUTSIDE the item loop but INLINE on the main-axis cursor (see
//     "NOT PORTED" below for the horizontal-orientation simplification this implies).
//   - EMPTY VIEW (shown while the source has no items) realizes its template content or a centered
//     TextBlock, filling the viewport's main extent.
//   - GRID: a GridItemsLayout(span, orientation) lays `span` item columns across the cross axis (a
//     linear list is span 1); each row's extent is the MAX of its columns' real measured extents — for a
//     templated cell this is the cross-platform i_view::measure() (which recurses through the whole
//     realized subtree's real Windows handlers, matching the android measure_element rationale: a bare
//     native Measure() on a container root under-reports, since layout_handler::get_desired_size()
//     deliberately returns {0,0} — the layout sizes itself through its OWN layout_manager, not the
//     handler); for a bare default TextBlock this is a direct native UIElement::Measure()/DesiredSize().
//
// LAYOUT SEAM: platform_arrange (src/controls/items/collection_view_handler.cpp) calls arrange_native
// FIRST, then updates the viewport-extent mirrors and runs the cross-platform simulator's own
// refresh_realization() — so `platform->realized` reflects the PREVIOUS pass, not this one, by the time
// arrange_native runs. Rather than host off a one-frame-stale mirror, arrange_native does its own
// complete pass every time it is called (the android precedent this file follows), independent of
// `platform->realized`/`content_extent`/`scroll_offset` (those mirrors keep updating as before, for any
// test or future consumer that reads them — untouched by this file).
//
// NOT PORTED (documented deviations, per this task's "leave the existing mirror behaviour and say so"):
//   - Virtualization / cell recycling — every element is realized fresh each pass (explicitly out of
//     scope per this task's brief; matches the android partial).
//   - Selection highlight paint — platform->selected_paths / allows_selection keep updating via the
//     existing shared update_platform_selection()/update_selection_mode() (untouched), but no native
//     fill is drawn for a selected cell (android draws one; cut here for this wave's budget).
//   - Interactive drag-reorder — can_reorder_items keeps mirroring, but there is no native drag gesture
//     (matches the android/apple/ios precedent: the drag itself is native and out of scope everywhere).
//   - CarouselView's one-item-per-page layout (CreateCarouselLayout's FractionalWidth/Height(1)) — a
//     CarouselView reuses this handler (carousel_view.cpp), but this partial does NOT special-case it
//     the way android does; a carousel renders as a plain all-items flow. Cut because this task's scope
//     is CollectionView specifically (the 18 red pages), not CarouselView.
//   - ItemsUpdatingScrollMode's native scroll-follow (KeepItemsInView / KeepLastItemInView) and the
//     ScrollTo command (map_scroll_to) — neither drives the native ScrollViewer (no ChangeView call);
//     both keep updating their cross-platform mirrors only. scroll_view_handler.cpp already has the
//     ChangeView recipe if a future pass wants to wire this up.
//   - A HORIZONTAL CollectionView's Header/Footer: MAUI lays these out as fixed bands OUTSIDE the item
//     region (top/bottom of the viewport, independent of the horizontal scroll). This partial instead
//     places them INLINE on the main-axis cursor for BOTH orientations (before the first item / after
//     the last) — correct for the (default, far more common) vertical case, a simplification for
//     horizontal. Documented, not a guess: no horizontal-header gallery page was in scope to verify
//     against.
//   - item_sizing_strategy (MeasureAllItems vs MeasureFirstItem) is not distinguished — every item is
//     always individually measured (the strictly-more-correct, more-expensive option; MeasureFirstItem
//     is a perf optimization this partial does not need).
//   - get_desired_size()'s native-measurement hook: the shared cross-platform .cpp only special-cases
//     get_desired_size's content estimate for iOS and Android (`#if !defined(MAUI_PLATFORM_IOS) &&
//     !defined(MAUI_PLATFORM_ANDROID)` — see native_content_size there); Windows falls into that SAME
//     default branch as apple/AppKit (nullopt = "no native size", the flat item_extent*count estimate).
//     To narrow the resulting mismatch WITHOUT touching the shared file, arrange_native below feeds the
//     REAL measured average row extent back into platform->item_extent after every pass, so the
//     estimate converges toward the true value across layout passes. This is a port-only device (it
//     changes no C#-observable behavior, only the internal estimate's accuracy) — not an oracle
//     citation, and explicitly flagged as such at its call site.

#include "maui/controls/items/collection_view_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule: the FULL header for every namespace whose MEMBERS you call.
// Canvas::Children() is an IVector<UIElement>, so Append/Clear need this one; without it they
// are only forward-declared and every call is C3779 -- an error that names neither the header
// nor the concept. Third time in this backend; every base class contributes a header.
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/groupable_items_view.hpp"
#include "maui/controls/items/i_items_view.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/structured_items_view.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using scroll_viewer = winui::Controls::ScrollViewer;
    using canvas = winui::Controls::Canvas;
    using text_block = winui::Controls::TextBlock;

    // The floor a realized row/band is clamped to when its real measure comes back 0 (a container root
    // whose own layout_manager has not run yet, or a genuinely empty label) — so a measurement failure
    // can't collapse a row to nothing and let the next one overlap it. Matches the android partial's
    // k_min_row_extent (same rationale, same value).
    constexpr double k_min_row_extent = 24;

    // GROUP HEADER container chrome — src/Controls/src/Core/Platform/Windows/CollectionView/
    // ItemsViewStyles.xaml's ListViewHeaderItem AND GridViewHeaderItem styles (both identical) override
    // Padding="12,8,12,0" (left,top,right,bottom) around the header content, plus Margin="0,0,0,4"
    // trailing the header before the group's first item. The GROUP FOOTER gets NEITHER of these: UWP
    // ListViewBase has no native group-footer slot, so GroupTemplateContext.cs (same directory) fakes one
    // by appending the footer as a plain trailing ITEM to the group's own item list — it rides the
    // regular item container, not ListViewHeaderItem. Confirmed against the real Windows capture: the
    // footer's orange band sits flush against both edges while the header's green band is inset 12px each
    // side — so only the GROUP HEADER call site below opts into this chrome.
    constexpr double k_group_header_cross_padding = 12; // Padding left/right
    constexpr double k_group_header_lead_padding = 8;   // Padding top (inset before content)
    constexpr double k_group_header_trail_margin = 4;   // Margin bottom (gap after, before next sibling)

    scroll_viewer as_scroll_viewer(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<scroll_viewer>();
    }

    // The Canvas panel inside the ScrollViewer's Content slot (created in create_platform_view). Mirrors
    // scroll_view_handler.cpp's as_panel exactly (same layout seam, same reason: ScrollViewer.Content is
    // a single ContentControl slot and a Canvas does not size itself to its children).
    canvas as_panel(void* native)
    {
        return as_scroll_viewer(native).Content().as<canvas>();
    }

    // Measure a BARE native UIElement (the default, no-template TextBlock cell/header/footer) at `cross`
    // on the cross axis and UNBOUNDED on the main axis, returning the real main-axis DesiredSize — the
    // label_handler::get_desired_size seam (clear any pinned Width/Height first, so a second-and-later
    // pass re-measures instead of reading back the previous frame; NaN is XAML's "Auto").
    double measure_main_extent(const winui::UIElement& element, double cross_extent, bool vertical)
    {
        if (auto framework_element = element.try_as<winui::FrameworkElement>())
        {
            framework_element.Width(std::numeric_limits<double>::quiet_NaN());
            framework_element.Height(std::numeric_limits<double>::quiet_NaN());
        }
        const float cross = maui::platform::windows::measure_constraint(cross_extent);
        const float unbounded = maui::platform::windows::measure_constraint(std::numeric_limits<double>::infinity());
        element.Measure(vertical ? winrt::Windows::Foundation::Size{cross, unbounded}
                                 : winrt::Windows::Foundation::Size{unbounded, cross});
        const auto desired = element.DesiredSize();
        return vertical ? desired.Height : desired.Width;
    }
} // namespace

namespace maui::controls
{
    namespace
    {
        // Resolve a possibly-selector template against one item (DataTemplateSelector.SelectTemplate;
        // the container is the items view itself, like C# passes the ItemsView). Mirrors the
        // cross-platform resolve_template / the android resolve_item_template (both .cpp-internal).
        std::shared_ptr<data_template> resolve_item_template(const std::shared_ptr<data_template>& candidate,
                                                             const boxed_item& item,
                                                             maui::core::bindable_object* container)
        {
            if (auto selector = std::dynamic_pointer_cast<data_template_selector>(candidate))
            {
                return selector->select_template(item.context_box(), container);
            }
            return candidate;
        }

        // Recursively MOUNT a boxed VIEW / template-content chrome subtree (the android ensure_mounted,
        // ported 1:1 — see that file for the full rationale). Depth-first POST-ORDER (children first, so
        // each child's native view exists before its parent hosts it), attach each element's registered
        // handler by its runtime handler_type_tag, then re-fire the container's host command
        // (mount_into_handler) so the now-attached children's native views are hosted. Idempotent — an
        // element that already carries a handler is skipped. Needed for a template whose ROOT is a
        // CONTAINER (a header_footer_template photo_cell of Image+Label): realize_template_content below
        // attaches only the ROOT's handler; without this walk the children stay unbuilt and the panel
        // hosts an empty container.
        void ensure_mounted(maui::core::i_maui_context* context, maui::controls::element& root)
        {
            if (context == nullptr)
            {
                return;
            }
            root.visit_logical_children([context](maui::controls::element& child) { ensure_mounted(context, child); });

            auto* element_face = dynamic_cast<maui::core::i_element*>(&root);
            if (element_face == nullptr)
            {
                return;
            }
            if (!element_face->handler())
            {
                if (const std::optional<maui::core::type_tag> tag = root.handler_type_tag(); tag.has_value())
                {
                    if (std::shared_ptr<maui::core::i_element_handler> handler =
                            context->handlers().create_handler(*tag))
                    {
                        handler->set_maui_context(context); // SetMauiContext precedes SetVirtualView (C#)
                        element_face->set_handler(std::move(handler));
                    }
                }
            }
            root.mount_into_handler();
        }

        // Realize a type-activated template's content into a native WinUI UIElement (the C#
        // TemplatedCell2.Bind: CreateContent -> BindingContext -> ToPlatform(mauiContext)). Returns the
        // realized content (which OWNS its attached handler + native view — the caller retains it for as
        // long as it is hosted) and, out-param, its native UIElement (null when the template is
        // loader-only / no handler is registered for its content type — the caller then falls back to
        // the item-text mirror). Identical to the android/apple/ios realize path.
        std::shared_ptr<maui::core::bindable_object> realize_template_content(
            maui::core::i_maui_context* context, const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
            winui::UIElement& out_native)
        {
            out_native = nullptr;
            if (tmpl == nullptr || context == nullptr || !tmpl->content_type().has_value())
            {
                return nullptr;
            }
            std::shared_ptr<maui::core::bindable_object> content = tmpl->create_content();
            if (!content)
            {
                return nullptr;
            }
            // BindingContext = the item, set BEFORE attaching the handler so the first mapper pass
            // already sees the bound property values.
            content->set_binding_context_box(value.context_box());

            std::shared_ptr<maui::core::i_element_handler> child_handler =
                context->handlers().create_handler(*tmpl->content_type());
            auto* element = dynamic_cast<maui::core::i_element*>(content.get());
            if (!child_handler || element == nullptr)
            {
                return nullptr; // no registered handler (or non-element content) — fall back to text
            }
            child_handler->set_maui_context(context);
            element->set_handler(child_handler); // creates the platform view + runs the mapper

            // A container-rooted template (header_footer_template's chrome stack) needs its children
            // mounted too — set_handler alone attached only the root.
            if (auto* chrome = dynamic_cast<maui::controls::element*>(content.get()); chrome != nullptr)
            {
                ensure_mounted(context, *chrome);
            }

            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(child_handler.get()))
            {
                if (void* native = view_handler->native_view())
                {
                    out_native = maui::platform::windows::ref<winui::UIElement>(native);
                }
            }
            return content;
        }

        // Reuse a boxed VIEW's native view (a Header/Footer/EmptyView set to a live View). The boxed
        // chrome usually arrives UNMOUNTED here (it is not a CollectionView logical child, so the
        // gallery's own mount pass never walks it) — mount its whole native subtree on demand first (the
        // C# `Header is View` arm, where ToPlatform builds the platform view lazily), then return the
        // root handler's native_view(). Null when the value is not an element or has no view handler even
        // after mounting (the caller then falls back to the text mirror).
        winui::UIElement boxed_view_native(maui::core::i_maui_context* context, const boxed_item& value)
        {
            const std::shared_ptr<maui::core::bindable_object>& bindable = value.as_bindable();
            auto* element_face = dynamic_cast<maui::core::i_element*>(bindable.get());
            if (element_face == nullptr)
            {
                return nullptr;
            }
            if (auto* chrome = dynamic_cast<maui::controls::element*>(bindable.get()); chrome != nullptr)
            {
                ensure_mounted(context, *chrome);
            }
            if (const std::shared_ptr<maui::core::i_element_handler>& existing = element_face->handler())
            {
                if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(existing.get()))
                {
                    if (void* native = view_handler->native_view())
                    {
                        return maui::platform::windows::ref<winui::UIElement>(native);
                    }
                }
            }
            return nullptr;
        }

        // Run the cross-platform measure+arrange pass on a realized content view so its OWN children get
        // framed (a container-rooted template/header/footer, or a nested CollectionView cell whose inner
        // items need its own arrange_native to run — the android arrange_realized_view, ported 1:1). The
        // rect is the ABSOLUTE slot this file placed the native at (the port drives arrange in absolute
        // coordinates — a 0-origin arrange would re-frame the realized root to (0,0) and stack every
        // placed element at the panel's top-left).
        void arrange_realized_view(const std::shared_ptr<maui::core::bindable_object>& bindable,
                                   const maui::graphics::rect& frame)
        {
            auto* const view = dynamic_cast<maui::core::i_view*>(bindable.get());
            if (view == nullptr)
            {
                return; // a non-view realized content (e.g. a bare string mirror) — nothing to arrange
            }
            view->measure(frame.width, frame.height);
            view->arrange(frame);
        }

        // A default (no-template) cell / header / footer / empty-view: a plain TextBlock mirroring
        // item.text() — C#'s DefaultCell2 / SimpleViewHolder.FromText.
        text_block make_text_block(const std::string& text)
        {
            text_block block;
            block.Text(maui::platform::windows::to_hstring(text));
            block.TextWrapping(winui::TextWrapping::Wrap);
            return block;
        }
    } // namespace

    // ---- creation + teardown ----

    collection_view_platform::~collection_view_platform()
    {
        // Drop the realized subtrees FIRST (their handlers/native views release while the ScrollViewer
        // that hosted them still exists), then the root — button_platform's exact discipline.
        retained_natives.clear();
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<collection_view_platform>();
        // The layout seam (see this file's header comment): a ScrollViewer whose Content is a Canvas
        // panel, exactly like scroll_view_handler — a bare ScrollViewer cannot host absolutely-positioned
        // children AND provide a scrollable extent (Canvas::MeasureOverride returns (0,0) regardless of
        // children), so arrange_native stamps the PANEL's own Width/Height to the realized content extent
        // on every pass.
        scroll_viewer viewer;
        canvas panel;
        viewer.Content(panel);
        platform->native = maui::platform::windows::take<winui::UIElement>(viewer);
        return platform;
    }

    void collection_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite (the house convention every windows
        // partial in this port follows — see e.g. scroll_view_handler.cpp's platform_arrange).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }

        const scroll_viewer viewer = as_scroll_viewer(platform->native);
        canvas::SetLeft(viewer, frame.x);
        canvas::SetTop(viewer, frame.y);
        viewer.Width(frame.width);
        viewer.Height(frame.height);

        const canvas panel = as_panel(platform->native);
        panel.Children().Clear();
        // Drop the PREVIOUS pass's realized content now: the pass below realizes a fresh generation, and
        // clearing late (after the new generation exists) would risk the two overlapping in the retain
        // vector — matching the create-before-destroy discipline used everywhere else in this port.
        platform->retained_natives.clear();

        auto* view = virtual_view();
        maui::core::i_maui_context* const context = maui_context();
        const std::shared_ptr<i_items_view_source>& src = items_view_source();
        const bool vertical = platform->orientation == items_layout_orientation::vertical;
        const double cross_extent = vertical ? frame.width : frame.height;

        if (view == nullptr || context == nullptr)
        {
            // Nothing to realize yet (unattached / not yet connected to a maui_context) — still size the
            // panel to the viewport so the ScrollViewer has a sane extent.
            panel.Width(vertical ? cross_extent : frame.height);
            panel.Height(vertical ? frame.height : cross_extent);
            return;
        }

        const int span = std::max(1, platform->span);
        const double spacing = platform->item_spacing;
        auto* const container = dynamic_cast<maui::core::bindable_object*>(view);
        auto* const structured = dynamic_cast<structured_items_view*>(view);

        double cursor = 0;       // main-axis content cursor (dp)
        double measured_sum = 0; // for the item_extent feedback at the bottom of this function
        int measured_rows = 0;

        // TEMPORARY PROBE (env-gated, remove once header_footer_grid's crash is fixed).
        // Open-and-close per line so the last line survives an abrupt process death -- the technique
        // that found the `device` page crash and, in c59ed6981d, the resize boundary on this page.
        const auto cv_trace = [](const char* const fmt, auto... args) {
            const char* const trace = std::getenv("MAUI_WINUI_LOG");
            if (trace == nullptr)
            {
                return;
            }
            std::FILE* f = nullptr;
            if (fopen_s(&f, trace, "a") == 0 && f != nullptr)
            {
                std::fprintf(f, fmt, args...);
                std::fclose(f);
            }
        };

        cv_trace("cv.begin cross=%.1f span=%d vertical=%d structured=%d\n", cross_extent, span, vertical ? 1 : 0,
                 structured != nullptr ? 1 : 0);

        // Realize a full-cross-width row (structured header/footer, empty view, group header/footer):
        // template content > boxed view > text mirror — the android realize_supplemental_native recipe.
        // `is_group_header` opts into the ListViewHeaderItem/GridViewHeaderItem chrome documented at
        // k_group_header_cross_padding above — every OTHER caller (structured header/footer, empty view,
        // group footer) passes false and keeps the prior flush-to-the-edges behavior.
        auto realize_full_width = [&](const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
                                      bool is_group_header) {
            if (!tmpl && !value.has_value())
            {
                return; // nothing to show (matches android: no template AND no value)
            }
            cv_trace("cv.full enter tmpl=%d value=%d group=%d cursor=%.1f cross=%.1f\n", tmpl != nullptr ? 1 : 0,
                     value.has_value() ? 1 : 0, is_group_header ? 1 : 0, cursor, cross_extent);
            winui::UIElement native{nullptr};
            std::shared_ptr<maui::core::bindable_object> realized =
                realize_template_content(context, tmpl, value, native);
            if (native == nullptr)
            {
                if (winui::UIElement boxed = boxed_view_native(context, value))
                {
                    native = boxed;
                    realized = value.as_bindable();
                }
            }
            if (native == nullptr)
            {
                native = make_text_block(value.text());
            }
            // The header container's Padding insets its ContentPresenter too (Margin="{TemplateBinding
            // Padding}" in the ListViewHeaderItem/GridViewHeaderItem ControlTemplate), so a group header
            // measures against the padded-down cross extent, not the full row width.
            const double cross_pad = is_group_header ? k_group_header_cross_padding * 2 : 0.0;
            const double measure_cross = std::max(0.0, cross_extent - cross_pad);
            // Same preference as the item loop below: the realized content's CROSS-PLATFORM measure when
            // there is one (a header/footer template or boxed View can be container-rooted, and a bare
            // native Measure() on a container under-reports — see the item loop's comment), else a direct
            // native measure for the plain-text fallback.
            cv_trace("cv.full realized native=%d view=%d measure_cross=%.1f\n", native != nullptr ? 1 : 0,
                     dynamic_cast<maui::core::i_view*>(realized.get()) != nullptr ? 1 : 0, measure_cross);
            double extent = 0;
            if (auto* const content_view = dynamic_cast<maui::core::i_view*>(realized.get()))
            {
                const maui::graphics::size desired =
                    vertical ? content_view->measure(measure_cross, std::numeric_limits<double>::infinity())
                             : content_view->measure(std::numeric_limits<double>::infinity(), measure_cross);
                extent = vertical ? desired.height : desired.width;
            }
            else
            {
                extent = measure_main_extent(native, measure_cross, vertical);
            }
            // Left UNCONDITIONAL on purpose, unlike the item-row floor below: these bands are not item
            // containers (the structured header/footer is ListViewBase.Header/Footer content, a group
            // header is a ListViewHeaderItem), so the linear MinHeight = 0 setter the item rows are
            // governed by does not speak to them, and no measurement implicates them -- this page's
            // header/footer measure 127 and 140, an order above the floor. Deliberately not "fixed" in
            // the same pass; it would be an unmeasured change riding a measured one.
            extent = std::max(extent, k_min_row_extent);
            const double lead = is_group_header ? k_group_header_lead_padding : 0.0;
            const double trail = is_group_header ? k_group_header_trail_margin : 0.0;
            const double cross_origin = is_group_header ? k_group_header_cross_padding : 0.0;
            canvas::SetLeft(native, vertical ? cross_origin : cursor + lead);
            canvas::SetTop(native, vertical ? cursor + lead : cross_origin);
            if (auto framework_element = native.try_as<winui::FrameworkElement>())
            {
                framework_element.Width(vertical ? measure_cross : extent);
                framework_element.Height(vertical ? extent : measure_cross);
            }
            cv_trace("cv.full measured extent=%.1f\n", extent);
            panel.Children().Append(native);
            if (realized)
            {
                arrange_realized_view(
                    realized, vertical ? maui::graphics::rect{cross_origin, cursor + lead, measure_cross, extent}
                                       : maui::graphics::rect{cursor + lead, cross_origin, extent, measure_cross});
                platform->retained_natives.push_back(std::move(realized));
            }
            cursor += lead + extent + trail + spacing;
            cv_trace("cv.full exit cursor=%.1f\n", cursor);
        };

        // The global (structured) header — realized BEFORE the items/empty region, independent of it
        // (HeaderFooterView's empty source still shows its View header/footer — C#'s UpdateHeader runs
        // ahead of the data region regardless of item count).
        if (structured != nullptr)
        {
            realize_full_width(structured->header_template(), structured->header(), false);
        }

        const bool empty = src == nullptr || src->item_count() == 0;
        if (empty)
        {
            const bool has_empty_view = view->empty_view_template() != nullptr || view->empty_view().has_value();
            if (has_empty_view)
            {
                realize_full_width(view->empty_view_template(), view->empty_view(), false);
            }
        }
        else
        {
            const auto* const groupable = dynamic_cast<const groupable_items_view*>(view);
            const bool grouped = platform->grouped;
            const std::shared_ptr<data_template> group_header_t =
                grouped && groupable != nullptr ? groupable->group_header_template() : nullptr;
            const std::shared_ptr<data_template> group_footer_t =
                grouped && groupable != nullptr ? groupable->group_footer_template() : nullptr;
            const std::shared_ptr<data_template> item_t = view->item_template();
            const double col_cross = cross_extent / span;

            const int sections = src->group_count();
            for (int section = 0; section < sections; ++section)
            {
                if (group_header_t)
                {
                    realize_full_width(group_header_t, src->group(index_path{.section = section, .item = -1}), true);
                }

                const int count = src->item_count_in_group(section);
                for (int first = 0; first < count; first += span)
                {
                    // TEMPORARY PROBE (env-gated, remove once header_footer_grid's crash is fixed).
                    // c59ed6981d localized that page's failure precisely: the app enters drive_layout at
                    // the RESIZED client size (1008x761) and never returns, while the SAME page lays out
                    // fine at the boot size (944x504). A larger viewport realizes more cells and widens
                    // cross_extent per column, so this loop is the prime suspect. Logging section/row/
                    // count/extent per iteration means the LAST line written names the row being realized
                    // when the process died -- the same open-and-close-per-line technique that found the
                    // `device` page crash and, two probes ago, the resize boundary itself.
                    cv_trace("cv.row section=%d first=%d of %d span=%d cross=%.1f col=%.1f\n", section, first, count,
                             span, cross_extent, col_cross);
                    const int row_n = std::min(span, count - first);
                    struct realized_col
                    {
                        winui::UIElement native{nullptr};
                        std::shared_ptr<maui::core::bindable_object> retain;
                    };
                    std::vector<realized_col> cols;
                    cols.reserve(static_cast<std::size_t>(row_n));
                    double row_extent = 0;

                    // Realize each column, measuring its REAL extent — prefer the realized cell's
                    // CROSS-PLATFORM measure (it includes the cell's own children, the whole point of a
                    // templated row: layout_handler::get_desired_size() deliberately returns {0,0}, so a
                    // bare native UIElement::Measure() on a container root under-reports).
                    for (int c = 0; c < row_n; ++c)
                    {
                        const index_path path{.section = section, .item = first + c};
                        const boxed_item value = src->item(path);
                        const std::shared_ptr<data_template> resolved =
                            item_t ? resolve_item_template(item_t, value, container) : nullptr;
                        realized_col col;
                        col.retain = realize_template_content(context, resolved, value, col.native);
                        if (col.native == nullptr)
                        {
                            col.native = make_text_block(value.text());
                        }
                        double extent = 0;
                        if (auto* const cell_view = dynamic_cast<maui::core::i_view*>(col.retain.get()))
                        {
                            const maui::graphics::size desired =
                                vertical ? cell_view->measure(col_cross, std::numeric_limits<double>::infinity())
                                         : cell_view->measure(std::numeric_limits<double>::infinity(), col_cross);
                            extent = vertical ? desired.height : desired.width;
                        }
                        else
                        {
                            extent = measure_main_extent(col.native, col_cross, vertical);
                        }
                        row_extent = std::max(row_extent, extent);
                        cols.push_back(std::move(col));
                    }
                    // The floor is a GRID-ONLY constraint, and on a linear list only a total measure
                    // failure (every column 0) falls back to it — see k_min_row_extent's comment.
                    //
                    // src/Controls/src/Core/Handlers/Items/StructuredItemsViewHandler.Windows.cs draws
                    // the line explicitly, per items-layout TYPE:
                    //   * GetVerticalItemContainerStyle (LinearItemsLayout)   -> MinHeight = 0
                    //   * GetHorizontalItemContainerStyle (LinearItemsLayout) -> MinWidth  = 0
                    //   * GetItemContainerStyle (GridItemsLayout)             -> NO minimum setter, so
                    //     GridViewItem's BasedOn default minimum survives.
                    // Both linear arms zero the MAIN-AXIS minimum, which is exactly the axis this floor
                    // clamps -- so a linear row must use its true measured extent, and a grid row keeps
                    // the floor as this port's (still underived) stand-in for that SDK default.
                    //
                    // MEASURED, not reasoned: the ink profile of basic_grouping puts MAUI's text rows at
                    // 104/122/142/161/180/199/... -- a ~19px pitch -- against the port's
                    // 96/119/144/168/192/216/... -- exactly 24. 5px per row, compounding to the +52px
                    // drift rowshift.py reports by y=432.
                    //
                    // THIS WAS TRIED AND REVERTED ONCE (37a15f24da -> 904735d8c7) and the revert was
                    // right at the time for a reason nobody spotted: both landed ~12h BEFORE the theme
                    // font fix (406830415a), when a Label measured 17px, so dropping the floor gave 17px
                    // rows against MAUI's 19 -- wrong in the other direction. Labels now measure 19px.
                    // The revert's STATED reason (a Fluent ListViewItem minimum) is false for linear
                    // lists: MinHeight = 0 is set right there in the oracle above.
                    if (platform->grid || row_extent <= 0)
                    {
                        row_extent = std::max(row_extent, k_min_row_extent);
                    }

                    for (int c = 0; c < static_cast<int>(cols.size()); ++c)
                    {
                        realized_col& col = cols[static_cast<std::size_t>(c)];
                        if (col.native == nullptr)
                        {
                            continue;
                        }
                        const double col_origin = static_cast<double>(c) * col_cross;
                        canvas::SetLeft(col.native, vertical ? col_origin : cursor);
                        canvas::SetTop(col.native, vertical ? cursor : col_origin);
                        if (auto framework_element = col.native.try_as<winui::FrameworkElement>())
                        {
                            framework_element.Width(vertical ? col_cross : row_extent);
                            framework_element.Height(vertical ? row_extent : col_cross);
                        }
                        panel.Children().Append(col.native);
                        if (col.retain)
                        {
                            const maui::graphics::rect cell_rect =
                                vertical ? maui::graphics::rect{col_origin, cursor, col_cross, row_extent}
                                         : maui::graphics::rect{cursor, col_origin, row_extent, col_cross};
                            arrange_realized_view(col.retain, cell_rect);
                            platform->retained_natives.push_back(std::move(col.retain));
                        }
                    }
                    measured_sum += row_extent;
                    ++measured_rows;
                    cursor += row_extent + spacing;
                }

                if (group_footer_t)
                {
                    realize_full_width(group_footer_t, src->group(index_path{.section = section, .item = -1}), false);
                }
            }
        }

        // The global (structured) footer — realized AFTER the items/empty region, independent of item
        // count (like the header).
        if (structured != nullptr)
        {
            realize_full_width(structured->footer_template(), structured->footer(), false);
        }

        // Size the panel to the greater of the real content and the viewport (the scroll_view_handler
        // layout-seam pattern) so ScrollableWidth/Height falls out of native ScrollViewer arithmetic.
        const double content_main = cursor > spacing ? cursor - spacing : cursor; // drop the trailing spacing
        const double viewport_main = vertical ? frame.height : frame.width;
        const double panel_main = std::max(content_main, viewport_main);
        panel.Width(vertical ? cross_extent : panel_main);
        panel.Height(vertical ? panel_main : cross_extent);

        // Feed the REAL measured average row extent back into the flat-layout-model estimate
        // (platform->item_extent) — see this file's header "NOT PORTED" note on get_desired_size for why
        // this exists (Windows shares apple/AppKit's nullopt native_content_size fallback, so
        // get_desired_size()/build_entries() would otherwise report the 100pt/row placeholder forever).
        // Port-only device; changes no C#-observable behavior, only the internal estimate's accuracy.
        if (measured_rows > 0)
        {
            platform->item_extent = measured_sum / measured_rows;
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions, exactly like every other windows control
    // (see e.g. scroll_view_handler.cpp / layout_handler.cpp's identical blocks).
    void collection_view_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void collection_view_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void collection_view_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void collection_view_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void collection_view_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::controls
