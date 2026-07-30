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
//   - HEADER / FOOTER (structured, global) and GROUP HEADER (grouped, per-section) realize the same way,
//     hosted full-cross-width, OUTSIDE the item loop but INLINE on the main-axis cursor (see "NOT PORTED"
//     below for the horizontal-orientation simplification this implies). The GROUP FOOTER does NOT: UWP
//     ListViewBase has no native group-footer slot (GroupTemplateContext.cs:25-32), so it is realized
//     INSIDE the item loop instead, as a trailing ITEM appended to the group's own item list — it rides
//     a regular item cell (grid column width, the item-row floor), not a full-cross-width band.
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
//   - Selection highlight paint IS drawn (landed after this file's initial wave — see the
//     "SELECTION CHROME" block below this header comment for the full recipe): platform->selected_paths
//     / allows_multiple_selection (kept current by the existing shared update_platform_selection()/
//     update_selection_mode(), untouched by this change) now drive a from-scratch repaint of the real
//     Windows ListViewItem/GridViewItem container chrome this partial has no container to bind a
//     VisualState to. Still NOT ported: any tap/click gesture that would let a user CHANGE selection by
//     touching a cell — this file has no PointerPressed/Tapped wiring at all (a separate, pre-existing
//     gap; selection only ever changes here via the mapper, i.e. programmatically), so the new chrome is
//     read-only paint, matching this task's brief.
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
//   - get_desired_size()'s native-measurement hook: the shared cross-platform .cpp used to special-case
//     get_desired_size's content estimate for iOS and Android only (`#if !defined(MAUI_PLATFORM_IOS) &&
//     !defined(MAUI_PLATFORM_ANDROID)` — see native_content_size there), leaving Windows in that SAME
//     default nullopt branch as apple/AppKit (the flat item_extent*count estimate) — a self-sustaining
//     fixed point for an unbounded-cross CollectionView (a VerticalStackLayout parent with no
//     HeightRequest): get_desired_size fell back to the simulator's 400px `viewport_cross_extent` fake-
//     viewport placeholder, platform_arrange wrote that SAME 400 back from the frame it produced, forever
//     (header_footer_grid_horizontal rendered 400px tall where MAUI renders 132 = 3 rows x a measured
//     44px cell). Windows now widens that guard (`&& !defined(MAUI_PLATFORM_WINDOWS)`) and defines its own
//     native_content_size below (end of this file), reporting the REAL main/cross extents THIS pass
//     already measured — platform->real_main_extent (content_main below) and platform->real_cross_extent
//     (span * the first realized cell's own natural cross measurement, captured alongside the existing
//     per-column main-axis measure in the item loop — no second measurement pass, per this task's "prefer
//     reporting what the file already knows" guidance). arrange_native ALSO still feeds the REAL measured
//     average row extent back into platform->item_extent after every pass (unchanged, kept for the same
//     reason it always existed: some callers — build_entries(), scroll math — still read the flat
//     item_extent estimate directly, not just get_desired_size). Both are port-only devices (no
//     C#-observable behavior change, only the internal estimates' accuracy) — not oracle citations.

#include "maui/controls/items/collection_view_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.h>
// The C++/WinRT include rule: the FULL header for every namespace whose MEMBERS you call.
// Canvas::Children() is an IVector<UIElement>, so Append/Clear need this one; without it they
// are only forward-declared and every call is C3779 -- an error that names neither the header
// nor the concept. Third time in this backend; every base class contributes a header.
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
// SELECTION CHROME (below): Border.Background/BorderBrush need Media's SolidColorBrush, and a
// SolidColorBrush ctor needs Windows.UI's Color -- both projected types are only forward-declared by
// the headers above, the same C3779 trap the comment above names for Canvas::Children().
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.UI.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
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
#include "maui/core/view_chrome_ops.hpp"
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
    // k_min_row_extent (same rationale, same value). Grid rows do NOT use this — see
    // k_grid_item_min_extent below, a distinct SDK-default minimum this constant is not a stand-in for.
    constexpr double k_min_row_extent = 24;

    // GRID item cell minimum — src/Controls/src/Core/Handlers/Items/StructuredItemsViewHandler.
    // Windows.cs:246-263's GetItemContainerStyle (the GridItemsLayout arm) deliberately sets NO
    // MinHeight, unlike the linear arm's explicit `MinHeightProperty, 0` at :283-286 — so a GridViewItem
    // keeps its SDK BasedOn style's default minimum instead of being zeroed. DOCUMENTED DEVIATION (parity
    // ruling 11): that default isn't derivable from `src/` (no WindowsAppSDK generic.xaml on this
    // machine — confirmed absent, only `{ThemeResource GridViewItemMinHeight}`-shaped references exist).
    // VALUE measured from the grid_grouping ground-truth capture: item-text ink lands at an exact 44.0px
    // pitch (99, 143, 187, 231, 275, 319), vs the port's prior 24 (k_min_row_extent misapplied as this
    // value) at an exact 24px pitch. Distinct constant from k_min_row_extent on purpose — that one is a
    // measurement-failure guard, this one is a real, always-on container minimum on the grid path only.
    constexpr double k_grid_item_min_extent = 44;

    // GROUP HEADER container chrome — src/Controls/src/Core/Platform/Windows/CollectionView/
    // ItemsViewStyles.xaml's ListViewHeaderItem AND GridViewHeaderItem styles override
    // Padding="12,8,12,0" (left,top,right,bottom) around the header content, plus Margin="0,0,0,4"
    // trailing the header before the group's first item (both styles agree on Padding/Margin — see
    // :207-215 linear / :235-243 grid). The GROUP FOOTER gets NEITHER of these: UWP ListViewBase has no
    // native group-footer slot, so GroupTemplateContext.cs (same directory) fakes one by appending the
    // footer as a plain trailing ITEM to the group's own item list — it rides the regular item container,
    // not ListViewHeaderItem/GridViewHeaderItem. Confirmed against the real Windows capture: the footer's
    // orange band sits flush against both edges while the header's green band is inset 12px each side —
    // so only the GROUP HEADER call site below opts into this chrome.
    constexpr double k_group_header_cross_padding = 12; // Padding left/right
    constexpr double k_group_header_lead_padding = 8;   // Padding top (inset before content)
    constexpr double k_group_header_trail_margin = 4;   // Margin bottom (gap after, before next sibling)

    // GROUP HEADER container MinHeight — the one setter ItemsViewStyles.xaml's ListViewHeaderItem
    // (:215, `MinHeight="{ThemeResource ListViewHeaderItemMinHeight}"`) and GridViewHeaderItem
    // (:243, `MinHeight="{ThemeResource GridViewHeaderItemMinHeight}"`) both carry that the block
    // above copied Padding/Margin from but not this. The minimum applies to the CONTAINER, not the
    // header's content — the content renders at its natural height inset by Padding, and the container
    // tops up to MinHeight only if `lead_padding + natural_content_extent` falls short (see the
    // realize_full_width is_group_header branch below). DOCUMENTED DEVIATION (parity ruling 11): neither
    // theme resource's value is derivable from `src/` (no WindowsAppSDK generic.xaml on this machine),
    // so the VALUE is measured from ground truth. It is 44 on BOTH paths.
    //
    // Both captures agree once the label's ink inset is MEASURED instead of assumed. That inset is
    // calibrated off the port's own pre-fix geometry, where every term is known from the code: the old
    // model put basic_grouping's first item box at container_top 55 + lead 8 + extent 24 + trail 4 = 91,
    // and its ink measured at 96 -- so a 19px label's glyphs start 5px into their box. Same font, same
    // size, therefore the same 5px inset in MAUI. Applying it:
    //   - linear (basic_grouping): container top 51 (= 59 label top - 8 Padding-top); MAUI's first item
    //     ink 104 - 5 inset = box top 99; - 4 trail margin = container bottom 95; 95 - 51 = 44.
    //     Cross-checked on group 2: label top 369 -> container top 361; 361 + 44 + 4 + 5 = 414, and 414
    //     is exactly a measured MAUI ink top. Two independent groups, same answer.
    //   - grid (grid_grouping): container 51..95 = 44, first item row top 99, ink at +5 => 104.
    // An earlier pass carried 48 here for the linear path. That came from assuming a 1px ink inset on
    // linear while granting grid the real 5px -- the same label in both, so the asymmetry was the tell.
    // If a future capture ever shows the two resources genuinely diverging, split this back into two
    // constants; today the measurement says one value serves both.
    constexpr double k_group_header_min_height = 44;

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
    // on the cross axis and UNBOUNDED on the main axis, returning the real DesiredSize on BOTH axes (most
    // callers keep only the main-axis component; the grid cell's cross-axis capture in arrange_native's
    // column loop, feeding native_content_size, needs the other) — the label_handler::get_desired_size
    // seam (clear any pinned Width/Height first, so a second-and-later pass re-measures instead of reading
    // back the previous frame; NaN is XAML's "Auto").
    maui::graphics::size measure_desired(const winui::UIElement& element, double cross_extent, bool vertical)
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
        return {desired.Width, desired.Height};
    }

    // ============================================================================================
    // SELECTION CHROME — the real Windows ListViewItem/GridViewItem container's "Selected" paint.
    // ============================================================================================
    // ORACLE: SelectableItemsViewHandler.Windows.cs's UpdatePlatformSelection binds ListViewBase.
    // SelectedItem(s)/SelectionMode, and the DEFAULT (UNMODIFIED — ItemsViewStyles.xaml overrides only
    // ListViewHeaderItem/GridViewHeaderItem, never ListViewItem/GridViewItem, per that file) Fluent
    // container control templates paint the rest via VisualStateManager (Selected/SelectedPointerOver/
    // SelectedPressed). This partial realizes a bare UIElement straight into a Canvas with NO item
    // container at all (this file's header comment), so there is no template to bind a VisualState to —
    // every function below is a from-scratch repaint of what that container's template would have shown,
    // keyed off platform->selected_paths / allows_multiple_selection, exactly like the fill android's
    // collection_view_handler.cpp paints for the identical "no container" reason.
    //
    // MEASURED from the ground-truth captures — DOCUMENTED DEVIATION, parity ruling 11: there is no
    // WindowsAppSDK generic.xaml on this machine to read the real ListViewItemBackgroundSelected /
    // GridViewItemBackgroundSelected / etc. resource VALUES, so the render is authoritative instead.
    // preselected_items_{light,dark}.png (a GridItemsLayout, Multiple selection) vs preselected_item_
    // {light,dark}.png and multiple_bound_selection_{light,dark}.png (linear lists, Single and Multiple)
    // show TWO DIFFERENT chrome shapes, matching the real ListViewItem-vs-GridViewItem template split:
    //   - LINEAR list (ListViewItem): a flush, UNROUNDED, full-cell-slot background FILL under the
    //     content (measured 235,235,235 light / 52,52,52 dark, vs this page's own ~244/~39 background —
    //     a subtle darken, NOT the loud wash android paints). No border stroke. Single/None selection
    //     additionally shows a small rounded accent "selection indicator" bar flush against the leading
    //     (vertical: LEFT) edge — ~3dip wide, vertically centered, ~5dip inset top/bottom (a ~26px row's
    //     bar spans y+5..y+21). Multiple selection shows a CheckBox glyph INSTEAD of the bar (never
    //     both) — see paint_selection_checkbox.
    //   - GRID (GridViewItem): a ~4dip-rounded full-cell-slot FILL (238,238,238 light / 48,48,48 dark —
    //     a smaller darken than linear's) PLUS a ~2dip rounded rectangular BORDER stroke in the accent
    //     color, both sized to the WHOLE cell slot (margin included, matching android's "whole slot"
    //     convention for the identical native-container-is-a-box-model reason). Multiple selection
    //     additionally shows a CheckBox glyph inset in the TOP-RIGHT corner.
    //   The accent color (border stroke, indicator bar, AND a checked CheckBox's fill — all three
    //     measured the SAME per theme) is 0,103,192 light / 76,194,255 dark: WinUI's small-UI-element
    //     accent TINT (darkened on a light backdrop, lightened on a dark one, for legibility), NOT the
    //     flat SystemAccentColor — a resource-key lookup would risk resolving the wrong (untinted, too
    //     bright/dark) variant, so this is a measured constant like every other value here.
    //   NOT MEASURED (no in-scope page exercises it): GRID + Single/None selection. Assumed to keep the
    //     border+fill without a CheckBox (Single selection never shows one anywhere per the linear
    //     evidence above) — an INFERENCE, not a capture-backed fact.
    //   The horizontal-orientation indicator bar (paint_selection_indicator's `!vertical` branch) is
    //     likewise unmeasured — no horizontal-CollectionView + Single-selection page is in scope — and
    //     mirrors this file's existing horizontal-header 90-degree-rotation simplification stance.
    //
    // Every paint_* function below APPENDS to `panel` directly and returns void: the created Border/
    // TextBlock objects need no C++-side retention (unlike a template's realized bindable_object) —
    // Canvas.Children() itself holds the owning COM reference for as long as the element stays a child,
    // exactly like this file's own bare `make_text_block` TextBlocks above, which are never pushed to
    // retained_natives either. Clearing on every pass (and thus on deselect / re-realization) falls out
    // for free from arrange_native's existing `panel.Children().Clear()` at the top of every pass — there
    // is nothing extra to tear down.

    bool is_dark_theme(const winui::FrameworkElement& element)
    {
        return element.ActualTheme() == winui::ElementTheme::Dark;
    }

    winui::Media::SolidColorBrush solid_brush(std::uint8_t r, std::uint8_t g, std::uint8_t b)
    {
        return winui::Media::SolidColorBrush{winrt::Windows::UI::Color{255, r, g, b}};
    }

    winui::Media::SolidColorBrush selection_accent_brush(bool dark)
    {
        return dark ? solid_brush(76, 194, 255) : solid_brush(0, 103, 192);
    }

    winui::Media::SolidColorBrush selection_fill_brush(bool dark, bool grid)
    {
        if (grid)
        {
            return dark ? solid_brush(48, 48, 48) : solid_brush(238, 238, 238);
        }
        return dark ? solid_brush(52, 52, 52) : solid_brush(235, 235, 235);
    }

    constexpr double k_selection_border_thickness = 2; // GridViewItem selected-state border stroke
    constexpr double k_selection_corner_radius = 4;    // GridViewItem selected-state corner rounding
    constexpr double k_selection_indicator_width = 3;  // ListViewItem's left "selection indicator" bar
    constexpr double k_selection_indicator_inset = 5;  // bar's top/bottom inset off the full row height
    constexpr double k_selection_checkbox_size = 20;   // check_box_handler.cpp's own CheckBoxSize, reused
    constexpr double k_selection_checkbox_corner_radius = 3;
    constexpr double k_selection_checkbox_margin = 4;      // grid: inset from the cell's top/right corner
    constexpr double k_selection_checkbox_left_inset = 10; // list: inset from the row's left edge

    // The GridViewItem fill+border (both header comment "chrome shapes" fold into one Border: Background
    // gives the fill, BorderBrush/Thickness/CornerRadius give the stroke when `grid`) or the ListViewItem
    // flush fill (no stroke, no rounding, when `!grid`) — appended BEFORE the content native so it sits
    // BEHIND it (Canvas z-order = append order, the same convention android's add-before-content documents).
    void paint_selection_fill(const canvas& panel, bool dark, bool grid, const maui::graphics::rect& slot)
    {
        winui::Controls::Border fill;
        fill.Background(selection_fill_brush(dark, grid));
        if (grid)
        {
            fill.BorderBrush(selection_accent_brush(dark));
            fill.BorderThickness(winui::Thickness{k_selection_border_thickness, k_selection_border_thickness,
                                                  k_selection_border_thickness, k_selection_border_thickness});
            fill.CornerRadius(winui::CornerRadius{k_selection_corner_radius, k_selection_corner_radius,
                                                  k_selection_corner_radius, k_selection_corner_radius});
        }
        canvas::SetLeft(fill, slot.x);
        canvas::SetTop(fill, slot.y);
        fill.Width(slot.width);
        fill.Height(slot.height);
        panel.Children().Append(fill);
    }

    // ListViewItem's Single/None-selection accent bar — flush against the leading edge (vertical: LEFT;
    // horizontal: TOP, unmeasured per the header comment), vertically/horizontally centered on the cross
    // axis, inset on the main axis (measured geometry, header comment). Multiple selection shows a
    // CheckBox instead (paint_selection_checkbox) — the two are mutually exclusive at the call site below.
    void paint_selection_indicator(const canvas& panel, bool dark, const maui::graphics::rect& slot, bool vertical)
    {
        winui::Controls::Border bar;
        bar.Background(selection_accent_brush(dark));
        bar.CornerRadius(winui::CornerRadius{k_selection_indicator_width / 2, k_selection_indicator_width / 2,
                                             k_selection_indicator_width / 2, k_selection_indicator_width / 2});
        if (vertical)
        {
            canvas::SetLeft(bar, slot.x);
            canvas::SetTop(bar, slot.y + k_selection_indicator_inset);
            bar.Width(k_selection_indicator_width);
            bar.Height(std::max(0.0, slot.height - (2 * k_selection_indicator_inset)));
        }
        else
        {
            canvas::SetLeft(bar, slot.x + k_selection_indicator_inset);
            canvas::SetTop(bar, slot.y);
            bar.Width(std::max(0.0, slot.width - (2 * k_selection_indicator_inset)));
            bar.Height(k_selection_indicator_width);
        }
        panel.Children().Append(bar);
    }

    // Multiple-selection's CheckBox glyph — HAND-DRAWN (a Border + a checkmark TextBlock), not a real
    // winui::Controls::CheckBox: CheckBox's default Fluent style carries its own MinWidth/MinHeight/
    // Padding (the exact reason check_box_handler.cpp needs its own AdjustCheckBoxForNoText margin fixup
    // for a text-less CheckBox), so a bare Width(20)/Height(20) on a REAL CheckBox would not actually
    // arrange the box at 20x20 — the style's minimum would win, smearing the glyph off the position this
    // function computes. A hand-drawn Border has no such style fighting it.
    //
    // Shown on EVERY realized cell (checked or not), not just selected ones — measured: preselected_
    // items_light.png's UNSELECTED cells still show an empty box, and multiple_bound_selection_light.png
    // confirms the same for a linear list. Sized ~20x20 (measured slightly larger, ~22-24px, on the grid
    // capture; 20 is check_box_handler.cpp's own established CheckBoxSize constant, reused rather than
    // adding a second, barely-different, unmeasured-elsewhere magic number). Added AFTER the content
    // native so it draws ON TOP of it (measured: the checkmark glyph visibly overlaps the label's leading
    // character in multiple_bound_selection_light.png — content is NOT inset to make room for the
    // checkbox). IsHitTestVisible(false): this backend wires no tap-to-select gesture yet (this file's
    // header comment), so the glyph is decorative only and must never intercept a future gesture
    // recognizer's hits.
    void paint_selection_checkbox(const canvas& panel, bool dark, bool selected, bool grid,
                                  const maui::graphics::rect& slot)
    {
        winui::Controls::Border box;
        box.Width(k_selection_checkbox_size);
        box.Height(k_selection_checkbox_size);
        box.CornerRadius(winui::CornerRadius{k_selection_checkbox_corner_radius, k_selection_checkbox_corner_radius,
                                             k_selection_checkbox_corner_radius, k_selection_checkbox_corner_radius});
        box.IsHitTestVisible(false);
        if (selected)
        {
            box.Background(selection_accent_brush(dark));
            // The checkmark glyph — Segoe Fluent Icons' CheckMark codepoint (U+E73E), the glyph every
            // other WinUI3 checked-CheckBox/selection affordance uses. Segoe Fluent Icons ships inside
            // the Windows App SDK framework package this backend already targets (not the OS image), so
            // it is present regardless of Windows version — UNVERIFIED ON THE ACTUAL GUEST (this port
            // does not build on the Windows VM; see this file's own build note), flag if it renders as
            // tofu/missing-glyph boxes instead of a check mark.
            text_block glyph;
            glyph.Text(winrt::hstring{L"\uE73E"});
            glyph.FontFamily(winui::Media::FontFamily{L"Segoe Fluent Icons"});
            glyph.FontSize(10);
            glyph.Foreground(solid_brush(255, 255, 255));
            glyph.HorizontalAlignment(winui::HorizontalAlignment::Center);
            glyph.VerticalAlignment(winui::VerticalAlignment::Center);
            box.Child(glyph);
        }
        else
        {
            box.Background(dark ? solid_brush(32, 32, 32) : solid_brush(253, 253, 253));
            box.BorderBrush(dark ? solid_brush(157, 157, 157) : solid_brush(135, 135, 135));
            box.BorderThickness(winui::Thickness{1, 1, 1, 1});
        }
        if (grid)
        {
            canvas::SetLeft(box, slot.x + slot.width - k_selection_checkbox_size - k_selection_checkbox_margin);
            canvas::SetTop(box, slot.y + k_selection_checkbox_margin);
        }
        else
        {
            canvas::SetLeft(box, slot.x + k_selection_checkbox_left_inset);
            canvas::SetTop(box, slot.y + ((slot.height - k_selection_checkbox_size) / 2));
        }
        panel.Children().Append(box);
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
        // Clip is bounds-dependent: view_chrome_ops.cpp's apply_native_clip reads the Width/Height just
        // stamped above, and map_clip's own push (view_mapper.cpp) always runs BEFORE the first arrange,
        // so this re-invoke is what actually installs it. Identical to scroll_view_handler.cpp's call and
        // for the same reason — the clip masks the VIEWPORT (the ScrollViewer that `native` boxes), not
        // the larger scrollable content panel. This is the one handler the 2026-07-30 clip sweep left out,
        // by its own account in view_chrome_ops.cpp's header: this file was mid-edit by another change at
        // the time, so the call was deferred rather than landed blind.
        // QUALIFIED, unlike every other handler's identical call: this file is `namespace maui::controls`
        // (line 249) while apply_native_clip is declared in `maui::core`, and ADL cannot bridge them --
        // the arguments are void* and maui::graphics::i_shape*, so the associated namespaces are graphics,
        // not core. Unqualified gives error C3861 "identifier not found", which reads like a missing
        // include rather than a namespace mismatch. scroll_view_handler.cpp and friends are themselves
        // `namespace maui::core`, which is why their unqualified calls resolve.
        if (const auto* const clipped = virtual_view(); clipped != nullptr)
        {
            maui::core::apply_native_clip(platform->native, clipped->clip());
        }

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
        // SELECTION CHROME (see that block's header comment above): resolved ONCE per pass, not per
        // cell — ActualTheme() is a live tree query and every cell in one arrange_native pass shares the
        // same resolved theme.
        const bool dark_theme = is_dark_theme(panel);

        double cursor = 0;       // main-axis content cursor (dp)
        double measured_sum = 0; // for the item_extent feedback at the bottom of this function
        int measured_rows = 0;
        // The FIRST realized item cell's own natural cross-axis extent (captured once, below) — an
        // ItemsWrapGrid uses a UNIFORM cell size derived from the first item (FormsGridView.cs /
        // ItemsViewStyles.xaml), not a running max, so first-item is the faithful source for
        // native_content_size's real_cross_extent (published at the bottom of this function).
        double first_cell_cross = 0;
        bool have_first_cell_cross = false;

        // Realize a full-cross-width row (structured header/footer, empty view, group header): template
        // content > boxed view > text mirror — the android realize_supplemental_native recipe. The GROUP
        // FOOTER is deliberately NOT a caller here any more — see the item loop below, which realizes it
        // as a trailing ITEM instead (k_group_header_cross_padding's comment explains why: UWP has no
        // native group-footer slot, and the real footer rides the regular item container, not this band).
        // `is_group_header` opts into the ListViewHeaderItem/GridViewHeaderItem chrome documented at
        // k_group_header_cross_padding above — every OTHER caller (structured header/footer, empty view)
        // passes false and keeps the prior flush-to-the-edges behavior. `bottom_anchored` opts into the
        // viewport-bottom pin below — only the structured FOOTER call site passes true.
        auto realize_full_width = [&](const std::shared_ptr<data_template>& tmpl, const boxed_item& value,
                                      bool is_group_header, bool bottom_anchored) {
            if (!tmpl && !value.has_value())
            {
                return; // nothing to show (matches android: no template AND no value)
            }
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
                const maui::graphics::size desired = measure_desired(native, measure_cross, vertical);
                extent = vertical ? desired.height : desired.width;
            }
            const double lead = is_group_header ? k_group_header_lead_padding : 0.0;
            const double trail = is_group_header ? k_group_header_trail_margin : 0.0;
            const double cross_origin = is_group_header ? k_group_header_cross_padding : 0.0;
            // Left UNCONDITIONAL on purpose for the non-group-header callers (structured header/footer,
            // empty view): these bands are not item containers (ListViewBase.Header/Footer content), so
            // the linear MinHeight = 0 setter the item rows are governed by does not speak to them, and
            // no measurement implicates them -- this page's header/footer measure 127 and 140, an order
            // above the floor. Deliberately not "fixed" in the same pass; it would be an unmeasured
            // change riding a measured one.
            //
            // The GROUP HEADER is different: it DOES carry a MinHeight (k_group_header_min_height
            // above), but that minimum belongs to the CONTAINER, not the content -- the label
            // renders at its own NATURAL height inset by Padding, and only the container's total (lead
            // padding + that natural height) tops up to MinHeight when it falls short. So for a group
            // header `extent` is left UNFLOORED (it drives the content's own placement/size below) and a
            // separate `container_extent` -- container_extent = max(min_height, lead + extent) -- carries
            // the floor instead, exactly mirroring ItemsViewStyles.xaml's Padding-then-MinHeight model.
            double container_extent = lead + extent;
            if (is_group_header)
            {
                container_extent = std::max(k_group_header_min_height, container_extent);
            }
            else
            {
                // MEASURE-FAILURE GUARD ONLY, not a minimum. Earlier today this was an unconditional
                // std::max here, with a comment reasoning that "no measurement implicates" these bands
                // because header_footer_template's header/footer measure 127 and 140 -- an order above the
                // floor. That was true of a TEMPLATED header and false of a STRING one: a view-level
                // `Header = "..."` realizes a bare TextBlock measuring 19px at the theme font, so the floor
                // silently inflated it to 24 and pushed every row below it exactly +5px down. Measured on
                // basic_grouping and grid_grouping, where that single wrong extent is ~80% of the page diff,
                // and MAUI's own header band is a bare 19px TextBlock with no minimum
                // (StructuredItemsViewHandler.Windows.cs assigns ListViewBase.Header directly; nothing in
                // ItemsViewStyles.xaml gives ListViewBase.Header a MinHeight -- only the ITEM and
                // GROUP-HEADER containers get one, and the linear item arm explicitly zeroes it).
                // So these bands take their true measured extent, exactly like the linear item rows since
                // 458e488dd3, and 24 goes back to being what k_min_row_extent's own comment always said it
                // was: the fallback when a measure comes back 0.
                if (extent <= 0)
                {
                    extent = k_min_row_extent;
                }
                container_extent = extent; // lead is 0 for every non-group-header caller
            }
            // ListViewBase.Footer is routed to ItemsPresenter.Footer inside the FormsListView template's
            // ScrollViewer (StructuredItemsViewHandler.Windows.cs:190-191; ItemsViewStyles.xaml:71,85,88).
            // Observed on three ground-truth Windows captures (header_footer_template, header_footer,
            // footer_only_string): the footer arranges flush with the viewport's BOTTOM edge whenever
            // header+items don't fill it (the WinUI-internal arrange mechanism itself is not in `src/` —
            // inferred from the captures, not asserted as fact). `std::max` keeps the pre-existing
            // sequential placement when content already overflows the viewport, so the footer scrolls
            // below the items instead of being dragged up into them. Gated on `vertical`: only the
            // vertical case is measured here — horizontal header/footer placement is the pre-existing
            // NOT PORTED simplification documented in this file's header comment.
            if (bottom_anchored && vertical)
            {
                cursor = std::max(cursor, frame.height - container_extent);
            }
            canvas::SetLeft(native, vertical ? cross_origin : cursor + lead);
            canvas::SetTop(native, vertical ? cursor + lead : cross_origin);
            if (auto framework_element = native.try_as<winui::FrameworkElement>())
            {
                framework_element.Width(vertical ? measure_cross : extent);
                framework_element.Height(vertical ? extent : measure_cross);
            }
            panel.Children().Append(native);
            if (realized)
            {
                arrange_realized_view(
                    realized, vertical ? maui::graphics::rect{cross_origin, cursor + lead, measure_cross, extent}
                                       : maui::graphics::rect{cursor + lead, cross_origin, extent, measure_cross});
                platform->retained_natives.push_back(std::move(realized));
            }
            cursor += container_extent + trail + spacing;
        };

        // The global (structured) header — realized BEFORE the items/empty region, independent of it
        // (HeaderFooterView's empty source still shows its View header/footer — C#'s UpdateHeader runs
        // ahead of the data region regardless of item count).
        if (structured != nullptr)
        {
            realize_full_width(structured->header_template(), structured->header(), false, false);
        }

        const bool empty = src == nullptr || src->item_count() == 0;
        if (empty)
        {
            const bool has_empty_view = view->empty_view_template() != nullptr || view->empty_view().has_value();
            if (has_empty_view)
            {
                realize_full_width(view->empty_view_template(), view->empty_view(), false, false);
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
                    realize_full_width(group_header_t, src->group(index_path{.section = section, .item = -1}), true,
                                       false);
                }

                // GROUP FOOTER-as-trailing-ITEM: UWP ListViewBase has no native group-footer slot, so
                // GroupTemplateContext.cs:25-32 fakes one by appending the footer as a plain extra item
                // to the group's OWN item list; GroupFooterItemTemplateContext.cs:6 confirms it is just an
                // ItemTemplateContext subtype with no footer-specific measure/arrange. So it rides the
                // regular item container (col_cross width, participates in the row floor below) at virtual
                // index `item_count`, not a full-cross-width band — matching the real capture where an
                // odd-count group's footer shares its last row with the final item (grid_grouping:
                // Defenders, 7 items, footer lands column 1 of row 4 alongside "Yellowjacket").
                const int item_count = src->item_count_in_group(section);
                const int count = item_count + (group_footer_t ? 1 : 0);
                for (int first = 0; first < count; first += span)
                {
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
                        const int index = first + c;
                        boxed_item value;
                        std::shared_ptr<data_template> resolved;
                        if (group_footer_t && index == item_count)
                        {
                            // The trailing footer slot — same group-context value the header uses (the
                            // C# footer item wraps the group object, not one of its children), template
                            // used directly like the header (no selector resolution for group chrome).
                            value = src->group(index_path{.section = section, .item = -1});
                            resolved = group_footer_t;
                        }
                        else
                        {
                            const index_path path{.section = section, .item = index};
                            value = src->item(path);
                            resolved = item_t ? resolve_item_template(item_t, value, container) : nullptr;
                        }
                        realized_col col;
                        col.retain = realize_template_content(context, resolved, value, col.native);
                        if (col.native == nullptr)
                        {
                            col.native = make_text_block(value.text());
                        }
                        double extent = 0;
                        double cross_measured = 0; // this column's own natural CROSS-axis extent (see below)
                        if (auto* const cell_view = dynamic_cast<maui::core::i_view*>(col.retain.get()))
                        {
                            const maui::graphics::size desired =
                                vertical ? cell_view->measure(col_cross, std::numeric_limits<double>::infinity())
                                         : cell_view->measure(std::numeric_limits<double>::infinity(), col_cross);
                            extent = vertical ? desired.height : desired.width;
                            cross_measured = vertical ? desired.width : desired.height;
                        }
                        else
                        {
                            const maui::graphics::size desired = measure_desired(col.native, col_cross, vertical);
                            extent = vertical ? desired.height : desired.width;
                            cross_measured = vertical ? desired.width : desired.height;
                        }
                        row_extent = std::max(row_extent, extent);
                        // Capture the FIRST realized cell's cross extent only (see the declaration above) —
                        // native_content_size's real_cross_extent feedback, published at the bottom of this
                        // function. cross_measured came from the SAME measure() call as `extent` above, just
                        // reading the other axis of its DesiredSize — no second measurement pass. col_cross
                        // was the constraint fed to that call; a Label (this page's content) does not grow
                        // its desired cross size to fill a generous constraint, so this is the content's true
                        // natural extent as long as col_cross stays >= it (true here — col_cross starts at
                        // the wrong-but-generous 400/span and only shrinks toward the real, still-sufficient
                        // value). DOCUMENTED LIMITATION: a cell that actually wants to STRETCH-FILL the cross
                        // axis would instead report back col_cross itself (self-referential) — untrue for
                        // this page's plain Label cells; a future page with expanding cell content would need
                        // Android's off-tree measure-at-full-width approach instead (collection_view_handler.
                        // cpp's measure_element in src/platform/android).
                        if (!have_first_cell_cross)
                        {
                            first_cell_cross = cross_measured;
                            have_first_cell_cross = true;
                        }
                        cols.push_back(std::move(col));
                    }
                    // The floor is a GRID-ONLY constraint (k_grid_item_min_extent), and on a linear list
                    // only a total measure failure (every column 0) falls back to k_min_row_extent's
                    // measurement-failure guard.
                    //
                    // src/Controls/src/Core/Handlers/Items/StructuredItemsViewHandler.Windows.cs draws
                    // the line explicitly, per items-layout TYPE:
                    //   * GetVerticalItemContainerStyle (LinearItemsLayout)   -> MinHeight = 0
                    //   * GetHorizontalItemContainerStyle (LinearItemsLayout) -> MinWidth  = 0
                    //   * GetItemContainerStyle (GridItemsLayout)             -> NO minimum setter, so
                    //     GridViewItem's BasedOn default minimum survives (k_grid_item_min_extent).
                    // Both linear arms zero the MAIN-AXIS minimum, which is exactly the axis this floor
                    // clamps -- so a linear row must use its true measured extent, and a grid row keeps
                    // its own SDK-default floor. k_grid_item_min_extent's comment carries the DOCUMENTED
                    // DEVIATION note: the SDK value isn't derivable from `src/`, so it is measured from the
                    // ground-truth capture (parity ruling 11) instead of this port's prior 24 stand-in.
                    //
                    // MEASURED: basic_grouping (LINEAR) puts MAUI's text rows at 104/122/142/161/180/199/
                    // ... -- a ~19px pitch, matched once a linear row uses its true measured extent
                    // (unfloored). grid_grouping (GRID) puts MAUI's text rows at an exact 44.0px pitch
                    // (99, 143, 187, 231, 275, 319) -- k_grid_item_min_extent's value, replacing the prior
                    // 24 (an exact 24px port pitch, 20px short per row).
                    //
                    // THIS WAS TRIED AND REVERTED ONCE (37a15f24da -> 904735d8c7) and the revert was
                    // right at the time for a reason nobody spotted: both landed ~12h BEFORE the theme
                    // font fix (406830415a), when a Label measured 17px, so dropping the floor gave 17px
                    // rows against MAUI's 19 -- wrong in the other direction. Labels now measure 19px.
                    // The revert's STATED reason (a Fluent ListViewItem minimum) is false for linear
                    // lists: MinHeight = 0 is set right there in the oracle above.
                    if (platform->grid)
                    {
                        row_extent = std::max(row_extent, k_grid_item_min_extent);
                    }
                    else if (row_extent <= 0)
                    {
                        row_extent = k_min_row_extent;
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
                        // The cell's slot rect (dp) — the SAME rect arrange_realized_view below frames the
                        // content at (the arrange coordinate convention: one absolute rect drives both
                        // placement and layout), and what the selection chrome paints against (MAUI's
                        // container chrome fills/strokes the WHOLE slot, margin included — this file's
                        // "SELECTION CHROME" header comment above / android's identical "whole slot"
                        // convention for the same reason).
                        const maui::graphics::rect cell_rect =
                            vertical ? maui::graphics::rect{col_origin, cursor, col_cross, row_extent}
                                     : maui::graphics::rect{cursor, col_origin, row_extent, col_cross};

                        // ---- selection chrome (PAINT ONLY) --------------------------------------------
                        // Selection STATE keeps updating via the existing shared
                        // update_platform_selection()/update_selection_mode() (untouched by this change,
                        // this file's header comment) — selected_paths / allows_multiple_selection are
                        // simply READ here. Fill/border/indicator paint only a SELECTED cell; the
                        // CheckBox glyph paints EVERY cell (checked or not) once Multiple selection is on
                        // — see paint_selection_checkbox's header comment for the measured evidence.
                        const index_path cell_path{.section = section, .item = first + c};
                        const bool selected =
                            std::ranges::find(platform->selected_paths, cell_path) != platform->selected_paths.end();
                        if (selected)
                        {
                            paint_selection_fill(panel, dark_theme, platform->grid, cell_rect);
                            if (!platform->grid && !platform->allows_multiple_selection)
                            {
                                paint_selection_indicator(panel, dark_theme, cell_rect, vertical);
                            }
                        }
                        panel.Children().Append(col.native);
                        if (platform->allows_multiple_selection)
                        {
                            paint_selection_checkbox(panel, dark_theme, selected, platform->grid, cell_rect);
                        }
                        // ---------------------------------------------------------------------------------

                        if (col.retain)
                        {
                            // NO MARGIN TRANSLATION HERE, and this is a RECORDED NEGATIVE RESULT rather
                            // than an omission. A 2026-07-30 change offset this rect's origin by the realized
                            // root's leading margin, on the theory that MAUI double-applies it: ItemContentControl
                            // routes a non-ICrossPlatformLayout root (a bare Label) through ContentLayoutPanel,
                            // whose ArrangeOverride Arranges the view a second time so ComputeFrame insets by the
                            // margin twice (the branch is real -- ItemContentControl.cs:220-227 -- and the port's
                            // own view->arrange() insets exactly once, view.hpp's compute_frame).
                            // THE BOARD FALSIFIED IT. The translation helped 16 pages (preselected_items
                            // 5.20 -> 1.14, collectionview 1.99 -> 0.26, empty_view 1.71 -> 0.25) and REGRESSED
                            // 12 -- and the regressions were exactly the pages already scoring ~0.00%:
                            // basic_grouping 0.01 -> 1.72, grouping_plus_selection 0.01 -> 1.77, grid_grouping
                            // 1.63 -> 3.31, nested_collection 0.00 -> 0.54, items 0.00 -> 0.50. It cost nine
                            // light-SSIM-green pages and five dark ones (dark green 34 -> 29).
                            // basic_grouping is the decisive counter-example: its item template root is a bare
                            // `label` with thickness(5,0,0,0) (basic_grouping_page.hpp:80) -- a non-layout root
                            // with a nonzero margin, precisely the case the double-apply theory covers -- and it
                            // was ALREADY pixel-correct without the translation. So whatever the 16 helped pages
                            // are compensating for, it is not a uniformly-missing margin application.
                            // Do not re-land this without first explaining why basic_grouping does not need it.
                            arrange_realized_view(col.retain, cell_rect);
                            platform->retained_natives.push_back(std::move(col.retain));
                        }
                    }
                    measured_sum += row_extent;
                    ++measured_rows;
                    cursor += row_extent + spacing;
                }
            }
        }

        // The global (structured) footer — realized AFTER the items/empty region, independent of item
        // count (like the header).
        if (structured != nullptr)
        {
            realize_full_width(structured->footer_template(), structured->footer(), false, true);
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
        // this exists (build_entries()/scroll math still read item_extent directly, not just
        // get_desired_size). Port-only device; changes no C#-observable behavior, only the internal
        // estimate's accuracy.
        if (measured_rows > 0)
        {
            platform->item_extent = measured_sum / measured_rows;

            // Publish the real extents for native_content_size (below) — see this file's header comment
            // and the hpp's collection_view_platform field comments. The cross-axis floor mirrors the
            // grid/linear split k_grid_item_min_extent already applies to row_extent (this floor's
            // main-axis twin): neither GetItemContainerStyle arm sets a Min* for the grid path (see that
            // constant's comment above), so a GRID cell keeps GridViewItem's SDK default minimum on
            // whichever axis is "cross" for this orientation too; a LINEAR cell only floors on a total
            // measure failure (0), matching k_min_row_extent's fallback role.
            double real_cross_cell = first_cell_cross;
            if (platform->grid)
            {
                real_cross_cell = std::max(real_cross_cell, k_grid_item_min_extent);
            }
            else if (real_cross_cell <= 0)
            {
                real_cross_cell = k_min_row_extent;
            }
            platform->real_main_extent = content_main;
            platform->real_cross_extent = static_cast<double>(span) * real_cross_cell;
            platform->has_real_extent = true;
        }
    }

    // The Windows override of the get_desired_size seam (C# ItemsViewHandler.Windows.cs /
    // ViewHandlerExtensions.Windows.cs's platformView.Measure(...).DesiredSize — the shared cross-platform
    // .cpp's native_content_size() doc comment). Reports what arrange_native's LAST pass already measured
    // (this file's header comment + the hpp's collection_view_platform field comments) instead of the
    // shared file's flat item_extent estimate. nullopt before the first successful pass (has_real_extent
    // starts false) keeps pass 1 byte-identical to before this fix existed — a resize (the E2E runner's
    // boot-then-resize-to-capture-size choreography, or any later relayout) drives the second pass that
    // converges on the real value, the same two-pass device item_extent's feedback above already relies on.
    std::optional<maui::graphics::size> collection_view_handler::native_content_size(double /*width_constraint*/,
                                                                                     double /*height_constraint*/)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || !platform->has_real_extent)
        {
            return std::nullopt;
        }
        return platform->orientation == items_layout_orientation::vertical
                   ? maui::graphics::size{platform->real_cross_extent, platform->real_main_extent}
                   : maui::graphics::size{platform->real_main_extent, platform->real_cross_extent};
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
