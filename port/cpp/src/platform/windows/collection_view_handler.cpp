// collection_view_handler — Windows (WinUI 3) platform partial: a real Microsoft.UI.Xaml.Controls.
// ScrollViewer whose Content is a Canvas host panel, into which the handler realizes a native XAML
// element per collection element and positions each ABSOLUTELY (the manual-frame Canvas model every
// windows handler shares). The windows twin of src/platform/android/collection_view_handler.cpp (the
// ScrollView → MauiCollectionContent recipe, mirrored 1:1 in scope) and the real-native sibling of the
// headless mirror (src/platform/headless/collection_view_handler.cpp).
//
// THE REALIZATION MODEL (the android partial's, adapted to the Canvas absolute-frame seam):
//   - C++ drives layout — the Canvas never lays out anything itself (children keep the absolute frames
//     their own platform_arrange / this pass set). So arrange_native reads the handler's
//     i_items_view_source + the control's templates DIRECTLY, realizes one native element per element,
//     and frames each absolutely in the host panel.
//   - An ITEM with an ItemTemplate realizes the template's content as a real native element bound to the
//     item (realize_template_content: create_handler → set_maui_context → set_handler builds the native
//     view + runs the mapper — the C# TemplatedCell2.Bind path, identical to the android/apple/ios
//     partials). With no template the default cell is a plain TextBlock mirroring item.text()
//     (DefaultCell).
//   - HEADER / FOOTER (structured, global) and GROUP HEADER / FOOTER (grouped, per-section) realize the
//     same way: their template's content when set, a boxed VIEW's mounted native, else a TextBlock of
//     the value's / group key's text.
//   - EMPTY VIEW (shown while the source is empty) realizes its template content filling the viewport,
//     else a TextBlock of the empty value's text — the C# UpdateEmptyView.
//   - GRID: a GridItemsLayout(span, orientation) lays `span` item columns across the cross axis; a
//     linear list is span 1. Both orientations are honored. Each realized element is MEASURED (the
//     cross-platform measure of the realized content, max'd with the native Measure/DesiredSize) so a
//     text row takes its natural height.
//   - CAROUSEL (the paged path): CarouselView reuses this handler; when the virtual view is a
//     carousel_view, arrange_native realizes ONLY the item at the clamped current Position filling the
//     whole viewport (LayoutFactory2.CreateCarouselLayout's FractionalWidth/Height(1) item). Live swipe
//     paging is deferred with the gesture fan-out; the Prev/Next buttons drive Position, which re-runs
//     arrange_native.
//
// DOCUMENTED DEVIATIONS from the C# oracle (infrastructure gaps + the render-first guidance the android
// partial documents, NOT behavior guesses):
//   - NO virtualization / view recycling. MAUI's Windows CV is a FormsListView/GridView (ListViewBase)
//     with container recycling; the gallery pages have small fixed item counts, so this partial favors
//     render correctness and realizes every in-content element directly into the host panel (the same
//     cut as the android partial). The cross-platform simulator still records the realize/recycle/bind
//     trail for any consumer that wants it.
//   - The host is a plain ScrollViewer → Canvas, not ListViewBase; the scroll axis follows the
//     orientation (a vertical CV scrolls vertically, a horizontal one horizontally — the ScrollViewer's
//     scroll modes are set per pass). The horizontal header/footer render as full-viewport-width bands
//     at the top / bottom with the item columns flowing between them (the android band model — without
//     a pinned supplementary there is no way to hold a full-height header while items scroll past it).
//   - The selection highlight is // deferred: the realized cell ROOT is an arbitrary FrameworkElement
//     with no uniform Background slot (C#'s highlight rides the ItemContainer wrapper), so painting it
//     needs a per-cell wrapper panel — the cross-platform selected_paths mirror carries the state.
//   - snap points / peek insets / scroll-bar-visibility nuances are iOS-only knobs with no plain-
//     ScrollViewer analog here (the cross-platform mirror carries them as state) — the android scope.
//   - The empty-view TextBlock centers HORIZONTALLY (TextAlignment.Center); vertical centering of the
//     text run within its own pinned bounds has no TextBlock knob (a wrapper Grid can add it later).
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps all slots null, while the cross-platform fake-viewport simulator
// is ALWAYS live — that suite observes exactly the headless partial's behavior.

#include "maui/controls/items/collection_view_handler.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // ContentControl/Panel base-class consume methods
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h> // the Children UIElementCollection consume methods
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.ViewManagement.h> // UISettings — the system accent for the multi-select adorner
#include <winrt/Windows.UI.h>
#include <winrt/base.h>

#include "maui/controls/element.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/items/groupable_items_view.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/controls/items/selectable_items_view.hpp"
#include "maui/controls/items/selection_mode.hpp"
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
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace wnative = maui::platform::win;

    // A modest default supplemental/item extent fallback (dp) for elements that measure to nothing (e.g.
    // a realized template whose handler built no measured native view). Keeps a row from collapsing to
    // 0 — the android partial's constant.
    constexpr double k_min_row_extent = 24;

    // Pin `child` to a host-relative rect inside the Canvas host (Canvas.SetLeft/SetTop + explicit
    // Width/Height — the shared Canvas recipe, the android add_and_frame layout half).
    void frame_element(const mux::UIElement& child, const maui::graphics::rect& rect)
    {
        auto element = child.try_as<mux::FrameworkElement>();
        if (element == nullptr)
        {
            return;
        }
        muxc::Canvas::SetLeft(element, rect.x);
        muxc::Canvas::SetTop(element, rect.y);
        element.Width((std::max)(rect.width, 0.0));
        element.Height((std::max)(rect.height, 0.0));
    }

    // Measure `child` at an exact cross-axis extent and an UNBOUNDED main axis, then read back its
    // desired main extent (DesiredSize.Height for a vertical list, .Width for a horizontal one) — the
    // android measure_main_extent twin on the XAML Measure pass. Any previously pinned explicit size is
    // reset to Auto first so the last pass cannot pin DesiredSize (the measure_native dance). 0 when the
    // child is not a FrameworkElement — the caller floors the row extent anyway.
    [[nodiscard]] double measure_native_main(const mux::UIElement& child, double cross, bool vertical)
    {
        auto element = child.try_as<mux::FrameworkElement>();
        if (element == nullptr)
        {
            return 0.0;
        }
        element.Width(std::numeric_limits<double>::quiet_NaN()); // Auto — drop any previous pin
        element.Height(std::numeric_limits<double>::quiet_NaN());
        const auto cross_f = static_cast<float>((std::max)(cross, 0.0));
        constexpr float unbounded = std::numeric_limits<float>::infinity();
        element.Measure(vertical ? winrt::Windows::Foundation::Size{cross_f, unbounded}
                                 : winrt::Windows::Foundation::Size{unbounded, cross_f});
        const winrt::Windows::Foundation::Size desired = element.DesiredSize();
        return static_cast<double>(vertical ? desired.Height : desired.Width);
    }

    // Construct a plain TextBlock showing `text` (the DefaultCell label / the supplemental text mirror).
    // `center` applies TextAlignment.Center for the empty view (MAUI's SimpleViewHolder.FromText fill
    // path centers the label; the vertical half is the documented deviation — file header).
    [[nodiscard]] muxc::TextBlock make_text_block(const std::string& text, bool center = false)
    {
        muxc::TextBlock block;
        block.Text(wnative::to_hstring_utf8(text));
        if (center)
        {
            block.TextAlignment(mux::TextAlignment::Center);
        }
        return block;
    }

    // A viewport-filling Grid that centers a text label on BOTH axes — the EmptyView's centered host.
    // MAUI shows the empty-view text at the exact center of the CollectionView bounds; framing a bare
    // TextBlock to the region only horizontal-centered it (text pinned to the top). The Grid honors the
    // child's Vertical/HorizontalAlignment, so the label centers vertically too once framed to the region.
    [[nodiscard]] muxc::Grid make_centered_text_host(const std::string& text)
    {
        muxc::Grid grid;
        muxc::TextBlock block;
        block.Text(wnative::to_hstring_utf8(text));
        block.TextAlignment(mux::TextAlignment::Center);
        block.HorizontalAlignment(mux::HorizontalAlignment::Center);
        block.VerticalAlignment(mux::VerticalAlignment::Center);
        grid.Children().Append(block);
        return grid;
    }

    // ---- multi-select adornment (SelectionMode.Multiple) ----
    // MAUI's Windows CollectionView wraps a native ListViewBase; its multi-select mode draws a selection
    // CHECKBOX on each row plus a selected-row highlight band. The port's custom-canvas layout has no
    // native ListView, so it renders that chrome itself. These are used STRICTLY when the view's
    // selection_mode is `multiple`, so single/none CV pages are byte-identical to before.
    constexpr double k_cb_gutter = 34.0; // leading checkbox column width (dp) reserved in a vertical list
    constexpr double k_cb_box = 20.0;    // the checkbox glyph square (dp)

    // A display-only selection-checkbox glyph mirroring the item's selected state. A full native
    // muxc::CheckBox per cell is far too heavy for large collections (a 50-item grid does not paint
    // within the capture-settle window); this is a LIGHTWEIGHT Border + checkmark that matches the native
    // multi-select adorner: a checked box is filled with the system ACCENT (the exact red/blue the native
    // ListView uses) + a white check; an unchecked box is a thin gray rounded outline. Purely chrome —
    // selection is driven by the cross-platform SelectedItems, not a tap here.
    [[nodiscard]] muxc::Border make_selection_checkbox(bool checked, const mux::Media::Brush& accent)
    {
        muxc::Border box;
        box.CornerRadius(mux::CornerRadius{4.0, 4.0, 4.0, 4.0});
        if (checked)
        {
            box.Background(accent);
            muxc::TextBlock mark;
            mark.Text(winrt::hstring{L"✓"}); // ✓
            mark.Foreground(wnative::to_brush(maui::graphics::color::from_rgba(255, 255, 255, 255)));
            mark.FontSize(13.0);
            mark.HorizontalAlignment(mux::HorizontalAlignment::Center);
            mark.VerticalAlignment(mux::VerticalAlignment::Center);
            box.Child(mark);
        }
        else
        {
            box.BorderThickness(mux::Thickness{1.0, 1.0, 1.0, 1.0});
            box.BorderBrush(wnative::to_brush(maui::graphics::color::from_rgba(140, 140, 140, 200)));
        }
        return box;
    }

    // The selected-row highlight band (the ListViewItem selected background) — a subtle translucent gray
    // Grid framed behind the row content.
    [[nodiscard]] muxc::Grid make_selection_highlight()
    {
        muxc::Grid grid;
        grid.Background(wnative::to_brush(maui::graphics::color::from_rgba(128, 128, 128, 46)));
        return grid;
    }
} // namespace

namespace maui::controls
{
    namespace
    {
        // Resolve a possibly-selector template against one item (DataTemplateSelector.SelectTemplate;
        // the container is the items view itself, like C# passes the ItemsView). Mirrors the android
        // partial / the cross-platform resolve_template (kept local — that one is .cpp-internal).
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

        // Forward declaration: recursively mount a boxed VIEW / template-content subtree (defined below,
        // the android ensure_mounted verbatim — no JNI in it).
        void ensure_mounted(maui::core::i_maui_context* context, maui::controls::element& root);

        // Realize a type-activated template's content into a native XAML element (the C#
        // TemplatedCell.Bind: CreateContent → set BindingContext → ToPlatform(mauiContext)). Returns the
        // realized content (which OWNS its attached handler + native view — the caller keeps it alive
        // for as long as it is hosted) and, out-param, its native UIElement. Yields {nullptr, nullptr}
        // when the template is loader-only (no static control type) or no handler is registered for that
        // type — the caller then falls back to the item-text mirror. Identical to the android/apple
        // realize path.
        std::shared_ptr<maui::core::bindable_object> realize_template_content(
            collection_view_handler& handler, const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
            mux::UIElement& out_native)
        {
            out_native = nullptr;
            maui::core::i_maui_context* const context = handler.maui_context();
            if (tmpl == nullptr || context == nullptr || !tmpl->content_type().has_value())
            {
                return nullptr;
            }
            std::shared_ptr<maui::core::bindable_object> content = tmpl->create_content();
            if (!content)
            {
                return nullptr;
            }
            // BindingContext = the item (so the template's staged bindings resolve against it). Set
            // BEFORE attaching the handler so the first mapper pass already sees the bound values.
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

            // A realized ROOT that is a CONTAINER owning children needs its subtree mounted (each
            // descendant's handler attached + the container host command re-fired) — the android
            // partial's on-demand mount; a single-leaf template root is a no-op here.
            if (auto* chrome = dynamic_cast<maui::controls::element*>(content.get()); chrome != nullptr)
            {
                ensure_mounted(context, *chrome);
            }

            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(child_handler.get()))
            {
                out_native = wnative::borrow_as<mux::UIElement>(view_handler->native_view());
            }
            return content;
        }

        // Recursively MOUNT a boxed VIEW chrome subtree (the android partial's ensure_mounted, which is
        // pure cross-platform: depth-first POST-ORDER, attach each element's registered handler by its
        // runtime handler_type_tag — SetMauiContext before SetVirtualView, the C# order — then re-fire
        // the container host command so the now-attached children's native views are hosted).
        // Idempotent: an element that already carries a handler is skipped.
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
            if (!element_face->handler()) // skip an already-mounted element (idempotent re-mount guard)
            {
                if (const std::optional<maui::core::type_tag> tag = root.handler_type_tag(); tag.has_value())
                {
                    if (std::shared_ptr<maui::core::i_element_handler> handler =
                            context->handlers().create_handler(*tag))
                    {
                        handler->set_maui_context(context);            // SetMauiContext precedes SetVirtualView (C#)
                        element_face->set_handler(std::move(handler)); // the view owns its handler (PROFILE §11)
                    }
                }
            }
            root.mount_into_handler(); // re-host the (now-attached) children's native views
        }

        // Reuse a boxed VIEW's native element (a Header/Footer/EmptyView set to a live View via
        // boxed_item::of(view)). The boxed chrome is not a CV logical child, so it usually arrives
        // UNMOUNTED here — ensure_mounted builds its whole native subtree first (the C# `Header is View`
        // arm where ToPlatform builds the platform view on demand), then this returns the root handler's
        // native_view(). Empty when the value is not an element or has no view handler even after
        // mounting (the caller then falls back to the text mirror).
        mux::UIElement boxed_view_native(maui::core::i_maui_context* context, const boxed_item& value)
        {
            const std::shared_ptr<maui::core::bindable_object>& bindable = value.as_bindable();
            auto* element_face = dynamic_cast<maui::core::i_element*>(bindable.get());
            if (element_face == nullptr)
            {
                return nullptr;
            }
            if (auto* chrome = dynamic_cast<maui::controls::element*>(bindable.get()); chrome != nullptr)
            {
                ensure_mounted(context, *chrome); // build the native subtree on demand if not mounted
            }
            if (const std::shared_ptr<maui::core::i_element_handler>& existing = element_face->handler())
            {
                if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(existing.get()))
                {
                    return wnative::borrow_as<mux::UIElement>(view_handler->native_view());
                }
            }
            return nullptr;
        }

        // Run the cross-platform layout pass on a realized content view (a boxed Header/Footer chrome OR
        // a templated cell whose root is itself a container) so its CHILDREN / inner cells get framed —
        // frame_element only pins the realized ROOT. Uses the SAME host-relative rect the root was
        // framed at (the arrange coordinate convention the android partial documents): a container's
        // children then land inside the framed root, and a nested CollectionView cell root re-runs its
        // OWN arrange_native so its inner cells realize.
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
    } // namespace

    // ---- creation + teardown ----

    // Releases the strong refs pinning the ScrollViewer + Canvas host; `native` aliases `scroll` (not
    // separately retained — the android twin's shape). The retained_natives subtrees free their own
    // handlers + native views when the vector clears.
    collection_view_platform::~collection_view_platform()
    {
        retained_natives.clear();
        wnative::release(host);
        wnative::release(scroll);
        native = nullptr; // aliases `scroll` (not separately retained)
    }

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<collection_view_platform>();
        try
        {
            // The composed native: a ScrollViewer whose Content is the Canvas host panel the realized
            // children keep absolute frames inside (the android ScrollView → MauiCollectionContent
            // twin on the windows Canvas layout model).
            const muxc::ScrollViewer scroller;
            const muxc::Canvas host;
            scroller.Content(host);
            platform->scroll = wnative::store(scroller); // released in ~collection_view_platform
            platform->host = wnative::store(host);
            platform->native = platform->scroll; // the composed native (aliases scroll; NOT re-retained)
        }
        catch (const winrt::hresult_error&)
        {
            platform->scroll = nullptr; // XAML-less degradation (header note)
            platform->host = nullptr;
            platform->native = nullptr;
        }
        return platform;
    }

    // arrange_native — the backend half of platform_arrange (the one hook called on every backend).
    // Frame the ScrollViewer to the arranged rect, then re-realize the WHOLE content into the host panel
    // and position each element absolutely. The shared platform_arrange re-sets the viewport mirror +
    // re-runs the simulator after this returns; the native render here is independent of the windowing
    // simulator (it realizes the full content — the render-first model, the android partial's shape).
    void collection_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->scroll == nullptr || platform->host == nullptr)
        {
            return; // headless / XAML-less: no native tree to render
        }
        auto scroller = wnative::borrow<muxc::ScrollViewer>(platform->scroll);
        auto host = wnative::borrow<muxc::Canvas>(platform->host);
        if (scroller == nullptr || host == nullptr)
        {
            return;
        }

        // Frame the ScrollViewer to the arranged rect (the shared Canvas recipe).
        wnative::arrange_native(platform->scroll, frame);

        // Drop the previous pass: clear the host's children and release the retained template subtrees
        // (the apple prepareForReuse analog — every realized native is rebuilt fresh this pass).
        host.Children().Clear();
        platform->retained_natives.clear();

        // The mapped layout mirrors (orientation / span) drive the flow; the cross-platform
        // refresh_layout_mirrors keeps them current from the items_layout.
        const bool vertical = platform->orientation == items_layout_orientation::vertical;
        const int span = (std::max)(1, platform->span);
        const double cross_extent = vertical ? frame.width : frame.height;
        const double main_viewport = vertical ? frame.height : frame.width;
        constexpr double unbounded = std::numeric_limits<double>::infinity();

        // The scroll axis follows the orientation (deviation note in the header: the android twin only
        // scrolls vertically; the ScrollViewer makes the horizontal channel native).
        scroller.VerticalScrollMode(vertical ? muxc::ScrollMode::Auto : muxc::ScrollMode::Disabled);
        scroller.VerticalScrollBarVisibility(vertical ? muxc::ScrollBarVisibility::Auto
                                                      : muxc::ScrollBarVisibility::Disabled);
        scroller.HorizontalScrollMode(vertical ? muxc::ScrollMode::Disabled : muxc::ScrollMode::Auto);
        scroller.HorizontalScrollBarVisibility(vertical ? muxc::ScrollBarVisibility::Disabled
                                                        : muxc::ScrollBarVisibility::Auto);

        auto* view = virtual_view();
        const std::shared_ptr<i_items_view_source>& src = items_view_source();
        auto* container = dynamic_cast<maui::core::bindable_object*>(view);

        // SelectionMode.Multiple draws native multi-select chrome (a per-row checkbox + a selected-row
        // highlight) on MAUI's Windows CollectionView; the custom-canvas layout renders it below. Gated
        // strictly on `multiple` so single/none selection CV pages are unaffected.
        auto* selectable = dynamic_cast<maui::controls::selectable_items_view*>(view);
        const bool multi_select =
            selectable != nullptr && selectable->selection_mode() == maui::controls::selection_mode::multiple;
        // The checked-box fill: the system accent (the exact color the native ListView multi-select
        // adorner uses). Resolved once per pass from UISettings; a WinUI-blue fallback if unavailable.
        mux::Media::Brush selection_accent{nullptr};
        if (multi_select)
        {
            try
            {
                const winrt::Windows::UI::ViewManagement::UISettings ui;
                selection_accent = mux::Media::SolidColorBrush{
                    ui.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Accent)};
            }
            catch (const winrt::hresult_error&)
            {
                selection_accent = wnative::to_brush(maui::graphics::color::from_rgba(0, 120, 215, 255));
            }
        }

        // The main-axis flow position (dp); header → items/empty → footer all advance it. The
        // HORIZONTAL header/footer band model mirrors the android partial: a header/footer always spans
        // the whole cross axis (MAUI's GridLayoutSpanSizeLookup gives it the full Span for any
        // orientation), so on a horizontal CV they render as full-viewport-width bands at the top /
        // bottom and the item columns flow in the vertical band between them.
        double cursor = 0;
        double band_top = 0;    // vertical extent consumed by the horizontal top header band
        double band_bottom = 0; // vertical extent consumed by the horizontal bottom footer band
        const double viewport_width = frame.width;

        // The source is empty (UpdateEmptyView) — but the global Header/Footer STILL render around the
        // empty region (the C# `Header is View` arm is independent of item count).
        const bool empty = view == nullptr || !src || src->item_count() == 0;

        // Append `child` to the host (detaching from a stale parent first — the re-parent guard, the
        // shared wnative helper: a boxed View header re-hosted across passes arrives parented).
        auto add_child = [&host](const mux::UIElement& child) {
            wnative::detach_from_parent(child);
            host.Children().Append(child);
        };

        // Place a realized supplemental (header/footer) FULL VIEWPORT WIDTH and advance the flow.
        // Measures the realized MAUI view first (a boxed StackLayout header takes its true height incl.
        // children), max'd with the native Measure and the min floor; then places, arranges the children
        // over the placed rect, and returns the band height (the android place_full_width twin).
        auto place_full_width = [&](const mux::UIElement& child,
                                    const std::shared_ptr<maui::core::bindable_object>& realized,
                                    bool is_footer) -> double {
            if (child == nullptr)
            {
                return 0.0;
            }
            double height = 0.0;
            if (auto* const v = dynamic_cast<maui::core::i_view*>(realized.get()); v != nullptr)
            {
                height = v->measure(viewport_width, unbounded).height;
            }
            height = (std::max)(height, measure_native_main(child, viewport_width, /*vertical=*/true));
            height = (std::max)(height, k_min_row_extent);
            double top = 0.0;
            if (vertical)
            {
                // A vertical footer over an EMPTY source pins to the viewport BOTTOM: MAUI's CollectionView
                // fills its container, so an empty item region expands and the footer + its band land at the
                // bottom with the empty region filling between it and the header (header_footer_view). max()
                // keeps it inline once real content grows past the viewport; non-empty lists keep the footer
                // flowing right after the items (unchanged).
                top = (is_footer && empty) ? (std::max)(cursor, main_viewport - height) : cursor;
            }
            else if (is_footer)
            {
                // Pin the bottom footer band to the viewport bottom, stacking upward, never over the
                // top band (the android band model, with the axis mix-up straightened).
                top = (std::max)(frame.height - band_bottom - height, band_top);
            }
            else
            {
                top = band_top; // the top header band grows downward
            }
            add_child(child);
            const maui::graphics::rect band{0.0, top, viewport_width, height};
            frame_element(child, band);
            // Frame the realized view's CHILDREN via the cross-platform arrange over the placed rect —
            // frame_element only pinned the realized ROOT.
            arrange_realized_view(realized, band);
            if (vertical)
            {
                cursor = top + height; // advance to the band bottom (== cursor+height inline, or the pinned bottom)
            }
            else if (is_footer)
            {
                band_bottom += height;
            }
            else
            {
                band_top += height;
            }
            return height;
        };

        // Realize a supplemental (header/footer/group header/footer): template content > boxed view >
        // text mirror, hosted full cross-width. Appends to the host + retains.
        auto realize_supplemental_native = [&](const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
                                               bool is_footer) {
            if (!tmpl && !value.has_value())
            {
                return; // nothing to show
            }
            mux::UIElement native{nullptr};
            std::shared_ptr<maui::core::bindable_object> realized =
                realize_template_content(*this, tmpl, value, native);
            if (native == nullptr)
            {
                // A boxed VIEW (HeaderFooterView/Grid): mount its subtree on demand, then host it.
                native = boxed_view_native(maui_context(), value);
                if (native != nullptr)
                {
                    realized = value.as_bindable(); // arrange this boxed view's children via place_full_width
                }
            }
            if (native == nullptr && value.has_value())
            {
                native = make_text_block(value.text());
            }
            place_full_width(native, realized, is_footer);
            if (realized && realized != value.as_bindable())
            {
                platform->retained_natives.push_back(std::move(realized));
            }
        };

        // ── CarouselView paged path: show ONLY the current item, full-viewport (file header) ──────────
        const bool is_carousel = dynamic_cast<carousel_view*>(view) != nullptr;
        if (is_carousel && !empty)
        {
            auto* carousel = dynamic_cast<carousel_view*>(view);
            const int item_count = src->item_count();
            // CarouselView.Position clamped into [0, count-1] (the settled page; a fresh carousel is 0).
            int position = carousel != nullptr ? carousel->position() : 0;
            position = std::clamp(position, 0, item_count - 1);
            const boxed_item value = src->item(index_path{.section = 0, .item = position});
            const std::shared_ptr<data_template> item_t = view->item_template();
            const std::shared_ptr<data_template> resolved =
                item_t ? resolve_item_template(item_t, value, container) : nullptr;

            mux::UIElement native{nullptr};
            std::shared_ptr<maui::core::bindable_object> realized =
                realize_template_content(*this, resolved, value, native);
            if (native == nullptr)
            {
                native = make_text_block(value.text(), /*center=*/true);
            }
            if (native != nullptr)
            {
                // Frame the single current item filling the WHOLE viewport rect (the C#
                // CreateCarouselLayout FractionalWidth(1)/FractionalHeight(1) item — one item per page).
                add_child(native);
                const maui::graphics::rect page{0.0, 0.0, frame.width, frame.height};
                frame_element(native, page);
                arrange_realized_view(realized, page);
                // Advance the cursor by the viewport's MAIN extent so the host panel sizes to one page.
                cursor += main_viewport;
                if (realized && realized != value.as_bindable())
                {
                    platform->retained_natives.push_back(std::move(realized));
                }
            }
        }

        // The global (structured) header — realized BEFORE the items/empty region (independent of it).
        auto* structured = dynamic_cast<structured_items_view*>(view);
        if (!is_carousel && view != nullptr && structured != nullptr)
        {
            realize_supplemental_native(structured->header_template(), structured->header(), /*is_footer=*/false);
        }

        if (is_carousel)
        {
            // The carousel paged path above already realized the single current item; skip the CV's
            // header/items/empty/footer flow entirely (a carousel carries none of that chrome).
        }
        else if (empty)
        {
            // ---- the empty view region (UpdateEmptyView) ----
            // Only reserve the empty region when an empty view is actually SET (a template, a boxed
            // View, or a non-empty text value) — HeaderFooterView's source starts empty with NO empty
            // view, and the footer must then follow the header directly (the android partial's fix).
            const bool has_empty_view =
                view != nullptr && (view->empty_view_template() != nullptr || view->empty_view().has_value());
            if (view != nullptr && has_empty_view)
            {
                mux::UIElement empty_native{nullptr};
                std::shared_ptr<maui::core::bindable_object> realized =
                    realize_template_content(*this, view->empty_view_template(), view->empty_view(), empty_native);
                if (empty_native == nullptr)
                {
                    empty_native = boxed_view_native(maui_context(), view->empty_view());
                    if (empty_native != nullptr)
                    {
                        realized = view->empty_view().as_bindable();
                    }
                }
                if (empty_native == nullptr)
                {
                    // Center the text on BOTH axes in the reserved viewport region (MAUI's centered
                    // EmptyView host) — a bare TextBlock would top-align within the tall region.
                    empty_native = make_centered_text_host(view->empty_view().text());
                }
                if (empty_native != nullptr)
                {
                    // Fill the viewport with the empty view (the C# EmptyView centered host): a vertical
                    // CV reserves a viewport-tall region at the cursor; a horizontal one fills the band
                    // between the top/bottom header/footer bands.
                    const maui::graphics::rect region =
                        vertical ? maui::graphics::rect{0.0, cursor, frame.width, main_viewport}
                                 : maui::graphics::rect{0.0, band_top, frame.width,
                                                        (std::max)(frame.height - band_top, k_min_row_extent)};
                    add_child(empty_native);
                    frame_element(empty_native, region);
                    arrange_realized_view(realized, region);
                    if (vertical)
                    {
                        cursor += region.height;
                    }
                    if (realized && realized != view->empty_view().as_bindable())
                    {
                        platform->retained_natives.push_back(std::move(realized));
                    }
                }
            }
        }
        else
        {
            // ---- the realized items: [group header → items → group footer]* ----
            const auto* groupable = dynamic_cast<const groupable_items_view*>(view);
            const bool grouped = platform->grouped;
            const std::shared_ptr<data_template> group_header_t =
                groupable != nullptr ? groupable->group_header_template() : nullptr;
            const std::shared_ptr<data_template> group_footer_t =
                groupable != nullptr ? groupable->group_footer_template() : nullptr;
            const std::shared_ptr<data_template> item_t = view->item_template();

            // HORIZONTAL item band: the item COLUMNS flow in the vertical space BELOW the top header
            // band. (A vertical CV is unaffected: origin 0, band = full cross extent.)
            const double item_cross_origin = vertical ? 0.0 : band_top;
            const double item_band_cross = vertical ? cross_extent : (std::max)(cross_extent - band_top, 1.0);
            const double item_col = (std::max)(item_band_cross / span, 1.0);

            const int sections = src->group_count();
            for (int section = 0; section < sections; ++section)
            {
                if (grouped && group_header_t)
                {
                    // A group header/footer advances the main cursor inline (is_footer=false keeps it on
                    // the vertical flow, not a horizontal band — the android partial's choice).
                    realize_supplemental_native(group_header_t, src->group(index_path{.section = section, .item = -1}),
                                                /*is_footer=*/false);
                }
                const int count = src->item_count_in_group(section);
                // Lay items across `span` columns; each row's extent is the max measured of its columns.
                for (int first = 0; first < count; first += span)
                {
                    const int row_n = (std::min)(span, count - first);
                    double row_extent = k_min_row_extent;
                    struct realized_col
                    {
                        mux::UIElement native{nullptr};
                        std::shared_ptr<maui::core::bindable_object> retain;
                        bool selected = false; // in SelectedItems (multi-select chrome)
                    };
                    std::vector<realized_col> cols;
                    cols.reserve(static_cast<std::size_t>(row_n));
                    for (int c = 0; c < row_n; ++c)
                    {
                        const index_path path{.section = section, .item = first + c};
                        const boxed_item value = src->item(path);
                        realized_col col;
                        col.selected = multi_select && selectable->selected_items().contains(value);
                        const std::shared_ptr<data_template> resolved =
                            item_t ? resolve_item_template(item_t, value, container) : nullptr;
                        col.retain = realize_template_content(*this, resolved, value, col.native);
                        if (col.native == nullptr)
                        {
                            col.native = make_text_block(value.text()); // DefaultCell — the text mirror
                        }
                        if (col.native != nullptr)
                        {
                            // Main extent: prefer the realized cell's CROSS-PLATFORM measure (it
                            // includes the cell's children — the whole point of a templated row),
                            // max'd with the native Measure (the android partial's dual measure that
                            // fixed the collapsed-templated-row overlap).
                            if (auto* const cell_view = dynamic_cast<maui::core::i_view*>(col.retain.get());
                                cell_view != nullptr)
                            {
                                const maui::graphics::size desired = vertical
                                                                         ? cell_view->measure(item_col, unbounded)
                                                                         : cell_view->measure(unbounded, item_col);
                                row_extent = (std::max)(row_extent, vertical ? desired.height : desired.width);
                            }
                            row_extent = (std::max)(row_extent, measure_native_main(col.native, item_col, vertical));
                        }
                        cols.push_back(std::move(col));
                    }
                    for (int c = 0; c < static_cast<int>(cols.size()); ++c)
                    {
                        auto& col = cols[static_cast<std::size_t>(c)];
                        if (col.native == nullptr)
                        {
                            continue;
                        }
                        const double col_start = item_cross_origin + (static_cast<double>(c) * item_col);
                        // Multi-select chrome (SelectionMode.Multiple, vertical only): a leading checkbox
                        // gutter indents a LIST cell's content; a GRID cell keeps its width and gets a
                        // corner checkbox overlay. Non-multi passes leave `indent` 0 → identical to before.
                        const bool is_list = span == 1;
                        const double indent = (multi_select && vertical && is_list) ? k_cb_gutter : 0.0;
                        const maui::graphics::rect cell_rect =
                            vertical ? maui::graphics::rect{col_start + indent, cursor,
                                                           (std::max)(item_col - indent, 1.0), row_extent}
                                     : maui::graphics::rect{cursor, col_start, row_extent, item_col};
                        // The selected-row highlight band, framed BEHIND the cell content (added first).
                        if (multi_select && vertical && col.selected)
                        {
                            auto highlight = make_selection_highlight();
                            add_child(highlight);
                            frame_element(highlight,
                                          is_list ? maui::graphics::rect{0.0, cursor, cross_extent, row_extent}
                                                  : maui::graphics::rect{col_start, cursor, item_col, row_extent});
                        }
                        add_child(col.native);
                        frame_element(col.native, cell_rect);
                        if (col.retain)
                        {
                            // Frame the cell's CHILDREN / inner cells via the cross-platform arrange at
                            // the cell's placed rect — a templated cell whose root is itself a
                            // CollectionView (nested_collection) re-runs its own arrange_native here.
                            arrange_realized_view(col.retain, cell_rect);
                            platform->retained_natives.push_back(std::move(col.retain));
                        }
                        // The selection checkbox, framed ON TOP of the cell (added last): left of a LIST
                        // row (vertically centered in the gutter), top-right corner of a GRID cell.
                        if (multi_select && vertical)
                        {
                            auto checkbox = make_selection_checkbox(col.selected, selection_accent);
                            add_child(checkbox);
                            const double cb_y = cursor + (std::max)((row_extent - k_cb_box) / 2.0, 0.0);
                            frame_element(checkbox,
                                          is_list ? maui::graphics::rect{col_start + 7.0, cb_y, k_cb_box, k_cb_box}
                                                  : maui::graphics::rect{col_start + item_col - k_cb_box - 6.0,
                                                                         cursor + 6.0, k_cb_box, k_cb_box});
                        }
                    }
                    cursor += row_extent;
                }
                if (grouped && group_footer_t)
                {
                    realize_supplemental_native(group_footer_t, src->group(index_path{.section = section, .item = -1}),
                                                /*is_footer=*/false);
                }
            }
        }

        // The global (structured) footer — realized AFTER the items/empty region (and, like the header,
        // independent of item count, so HeaderFooterView's empty source still shows its View footer).
        // A carousel carries no footer (its paged path realized only the current item).
        if (!is_carousel && view != nullptr && structured != nullptr)
        {
            realize_supplemental_native(structured->footer_template(), structured->footer(), /*is_footer=*/true);
        }

        // ---- size the inner host panel to the content ----
        // The Canvas reports no natural size, so it is pinned EXPLICITLY to the content extent along the
        // scroll axis (at least the viewport, so a short list still fills the scroller) — that is what
        // gives the ScrollViewer its scrollable extent (the android explicit-LayoutParams twin).
        const double content_main = (std::max)(cursor, main_viewport);
        if (vertical)
        {
            host.Width((std::max)(frame.width, 0.0));
            host.Height(content_main + band_top + band_bottom); // bands are 0 on the vertical flow
        }
        else
        {
            host.Width(content_main);
            host.Height((std::max)(frame.height, 0.0));
        }
    }
} // namespace maui::controls
