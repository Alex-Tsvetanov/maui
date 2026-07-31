// activity_indicator_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.ProgressRing,
// the same native type ActivityIndicatorHandler.Windows.cs creates (`ViewHandler<IActivityIndicator,
// ProgressRing>` — NOT a ProgressBar/RangeBase; ProgressRing is a plain Control with its own
// IsActive/IsIndeterminate surface, no Value/Minimum/Maximum). Ported from
// ActivityIndicatorHandler.Windows.cs (cross-platform mapper in ActivityIndicatorHandler.cs) +
// Platform/Windows/ActivityIndicatorExtensions.cs.
//
// DOCUMENTED SIMPLIFICATIONS / DEVIATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. Visibility is NOT coupled to IsRunning here, unlike iOS/Android. ActivityIndicatorHandler.cs's
//     shared Mapper only remaps `[nameof(IActivityIndicator.Visibility)] = MapIsRunning` under
//     `#if __ANDROID__ || __IOS__ || MACCATALYST` ("Android/iOS do not respect both properties
//     independently") — Windows keeps Visibility on the generic ViewHandler mapping, because
//     ProgressRing.IsActive and UIElement.Visibility genuinely are independent there
//     (ActivityIndicatorExtensions.UpdateIsRunning is exactly `IsActive = IsRunning;`, no Visibility
//     reference at all). This port's SHARED cross-platform mapper table
//     (src/core/activity_indicator_handler.cpp) still redirects the "visibility" key to map_is_running
//     unconditionally for every backend (a fine simplification for headless/apple/android, which all DO
//     want that coupling). Rather than add a MAUI_PLATFORM_WINDOWS branch to that shared, non-conditional
//     table, map_is_running below reproduces BOTH independent native pushes itself: IsActive tracks
//     IsRunning alone, and Visibility is pushed through the exact same apply_visibility the generic
//     view_mapper would have used. Net native behavior matches the oracle; only the C++ entry-point
//     shape differs from the C# mapper's own `#if`.
//  2. CONTAINER (NeedsContainer/SetupContainer), NOW PORTED (fixes the 4%+ light/dark parity gap on this
//     page). Originally NOT ported — the file used to reason that ProgressRing IS a Control with its own
//     Background DependencyProperty, so painting update_background directly onto the bare ring needed no
//     wrapper. That is true for COLOUR, but wrong for EXTENT: with no explicit WidthRequest, this view's
//     HorizontalOptions default (Fill — View.HorizontalOptionsProperty's default in view.hpp/View.cs) hands
//     platform_arrange a `frame` as wide as the whole row (compute_frame/AlignHorizontal, view.hpp), and a
//     bare ring left at its own natural DesiredSize just sat at frame's LEFT EDGE instead of centred in it
//     (AlignHorizontal's `Fill` case falls through with no centring — MAUI relies on the wrapped
//     WrapperView's OWN Stretch arrange plus the ring's Center/Center content alignment to do that
//     visually). A `BackgroundColor="Yellow"` row therefore painted only a small yellow square hugging the
//     ring at the left, instead of MAUI's full-width bar; measured on this page's captures (see
//     PARITY_REVIEW.md): MAUI's default-size ring sits horizontally CENTRED (x-centre ~504 of a ~984-wide
//     row) and BackgroundColor="Yellow" spans the full row (x 20..1003); the unfixed port's ring sat at
//     x-centre ~32 and its yellow patch spanned only x 21..50 — 91% of the page's diff pixels were that one
//     undersized bar. Ported the same way label_handler.cpp/image_handler.cpp already closed the identical
//     "no container seam on this Canvas backend" gap: wrap the ring in a chromeless Border (Padding/
//     BorderThickness never touched, so it measures/arranges identically to a bare ring when unbackgrounded)
//     UNCONDITIONALLY rather than reproducing NeedsContainer's conditional attach/detach. `native` now
//     boxes the Border HOST, not the ring — see as_host/as_ring below. apply_background's existing
//     three-way try_as (winui_visual_ops.cpp) already fills a Border, so the yellow row's background now
//     lands on the (correctly Fill-sized) host, matching MAUI's WrapperView paint target.
//  3. ARRANGE WIDTH/HEIGHT (ActivityIndicatorExtensions.UpdateWidth/UpdateHeight, both Windows-only
//     Mapper keys in the C# oracle): "Only set a value for this if it's been explicitly set... Otherwise,
//     don't set it to anything (even NaN) because it will try to fill all the space you give it." This
//     is still honoured, but now realised on the HOST/RING split instead of a single element (image_
//     handler.cpp's identical restructuring for AspectFill): platform_arrange stamps the HOST's Width/
//     Height to the resolved `frame` UNCONDITIONALLY (frame.width/height already resolve correctly for
//     both cases via the shared compute_frame/AlignHorizontal — Fill-with-no-explicit-size yields the
//     full row width, an explicit WidthRequest collapses Fill to Center-over-that-exact-size — see note 2
//     above), while the RING's own Width/Height DP is still only ever stamped when the developer set an
//     explicit WidthRequest/HeightRequest (this port's NaN-is-unspecified convention), exactly as before
//     this fix. A ProgressRing has no Image-style Stretch/Aspect knob to resize its glyph without touching
//     Width/Height directly, so unlike image_handler.cpp's Image child (whose Width/Height are NEVER
//     touched, left permanently Auto), this ring's own Width/Height still needs the explicit-only pin —
//     the ring's default Center/Center alignment inside the host (create_platform_view below) is what
//     keeps its own natural/explicit size centred once the host, not the ring, owns the Fill extent.

#include "maui/core/activity_indicator_handler.hpp"

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <span>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_activity_indicator.hpp"
#include "maui/core/view_chrome_ops.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias — see picker_handler.cpp's
    // identical note.
    namespace winui = winrt::Microsoft::UI::Xaml;
    using progress_ring = winui::Controls::ProgressRing;
    using host_border = winui::Controls::Border;

    // CONTAINER (file header note 2): `native` boxes the Border HOST, not the bare ring — mirrors
    // label_handler.cpp's as_host/as_text_block and image_handler.cpp's identical host/child split.
    host_border as_host(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<host_border>();
    }

    progress_ring as_ring(void* native)
    {
        return as_host(native).Child().as<progress_ring>();
    }

    // ActivityIndicatorExtensions.cs's ProgressRingForegroundResourceKeys — a single per-state-agnostic
    // brush key (ComboBox/CalendarDatePicker need several per-visual-state keys; ProgressRing has just
    // this one), which the control template's ring-stroke brush binds to. A plain Foreground override
    // alone would be dropped the same way picker_handler.cpp's k_text_color_keys note explains.
    constexpr std::array<std::wstring_view, 1> k_foreground_keys{L"ProgressRingForegroundThemeBrush"};

    void set_resources(const progress_ring& ring, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            ring.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const progress_ring& ring, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            ring.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden — identical to picker_handler.cpp's helper.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // "Was this property explicitly set?" — see picker_handler.cpp/date_picker_handler.cpp/
    // label_handler.cpp for why this must not be a value comparison
    // ([[cpp-unset-color-sentinel-collision]]): the port's Color is a non-nullable value type whose
    // default-constructed value is opaque BLACK, so it cannot stand in for C#'s `color.IsDefault()`.
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }
} // namespace

namespace maui::core
{
    activity_indicator_platform::~activity_indicator_platform()
    {
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<activity_indicator_platform> activity_indicator_handler::create_platform_view()
    {
        auto platform = std::make_unique<activity_indicator_platform>();
        // ActivityIndicatorHandler.Windows.cs's CreatePlatformView: `new ProgressRing { IsIndeterminate =
        // true }`.
        progress_ring ring;
        ring.IsIndeterminate(true);
        // CONTAINER (file header note 2): Center/Center, unconditionally — a ring never stretches to fill
        // extra space (unlike this backend's other Border-host handlers: label's TextBlock stretches for
        // TextAlignment, image's Image stretches unless AspectFill opts into Center/Center). Once the HOST
        // below owns the Fill extent, the ring's own natural (or explicit-WidthRequest) size stays centred
        // inside it — this is what fixes the "ring glued to the row's left edge" half of note 2's bug.
        ring.HorizontalAlignment(winui::HorizontalAlignment::Center);
        ring.VerticalAlignment(winui::VerticalAlignment::Center);
        host_border host;
        host.Child(ring);
        platform->native = maui::platform::windows::take<winui::UIElement>(host);
        return platform;
    }

    void activity_indicator_handler::map_is_running(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // Headless-mirror bookkeeping, kept in the same shape every other backend uses (the shared
        // "is_running && visible" observable), even though the NATIVE push below deliberately keeps the
        // two signals independent — see the file-top deviation note 1.
        const bool visible = view.visibility() == visibility::visible;
        platform->is_running = view.is_running() && visible;
        platform->hidden = !visible;
        if (platform->native == nullptr)
        {
            return;
        }
        // ActivityIndicatorExtensions.UpdateIsRunning: `IsActive = IsRunning` — no Visibility reference.
        as_ring(platform->native).IsActive(view.is_running());
        // The independent Visibility push this handler's shared mapper would otherwise have skipped (its
        // "visibility" key is redirected here for every backend) — see the file-top deviation note 1.
        maui::platform::windows::apply_visibility(platform->native, view.visibility());
    }

    void activity_indicator_handler::map_color(activity_indicator_handler& handler, i_activity_indicator& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->color = view.color();
        const progress_ring ring = as_ring(platform->native);
        // ActivityIndicatorExtensions.UpdateColor: an unset Color (`color.IsDefault()`, with
        // ActivityIndicatorHandler.Windows.cs always passing `foregroundDefault: null`) clears the local
        // Foreground so the theme brush (ProgressRingForegroundThemeBrush) shows through instead of
        // painting transparent black.
        if (!is_set(view, "color"))
        {
            remove_resources(ring, k_foreground_keys);
            ring.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(ring);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->color)};
        set_resources(ring, k_foreground_keys, brush);
        ring.Foreground(brush);
        refresh_theme_resources(ring);
    }

    maui::graphics::size activity_indicator_handler::get_desired_size(double width_constraint,
                                                                      double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing. XAML's
        // Measure THROWS on a negative Size, so this is a crash guard, not a formality.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const host_border host = as_host(platform->native);
        const progress_ring ring = as_ring(platform->native);
        // See the file-top deviation note 3: only pin Width/Height when EXPLICIT; otherwise leave the ring
        // to its own natural size rather than the ARRANGE/EXPLICIT-SIZE FIX every other Windows handler
        // uses (which pins Width/Height to the request even when NaN).
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        if (std::isfinite(explicit_width))
        {
            ring.Width(explicit_width);
        }
        if (std::isfinite(explicit_height))
        {
            ring.Height(explicit_height);
        }
        const double adjusted_width_constraint =
            std::isnan(explicit_width) ? width_constraint : std::max(width_constraint, explicit_width);
        const double adjusted_height_constraint =
            std::isnan(explicit_height) ? height_constraint : std::max(height_constraint, explicit_height);
        const winrt::Windows::Foundation::Size measure_size{
            maui::platform::windows::measure_constraint(adjusted_width_constraint),
            maui::platform::windows::measure_constraint(adjusted_height_constraint)};
        // Also measure the HOST, matching label_handler.cpp/image_handler.cpp's identical host-measure step
        // — belt-and-braces so the Border participates in WinUI's own measure-before-arrange bookkeeping
        // once it (not the bare ring) is what platform_arrange positions in the parent Canvas. The RETURNED
        // desired size still comes from the ring below, unchanged from before this fix — a chromeless
        // Border (Padding/BorderThickness never touched) measures identically to its child, so this is a
        // hygiene addition, not a behavior change.
        host.Measure(measure_size);
        ring.Measure(measure_size);
        const auto desired = ring.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void activity_indicator_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp/picker_handler.cpp's
        // platform_arrange for why (a NaN reaching XAML's arrange is an unrecoverable stowed exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        // CONTAINER (file header note 2): size/position the HOST to `frame`, exactly like label_handler.cpp
        // /image_handler.cpp's identical host-arrange step. frame.width/frame.height already resolve
        // correctly for BOTH cases via the shared cross-platform compute_frame/AlignHorizontal (view.hpp):
        // an explicit WidthRequest/HeightRequest collapses HorizontalOptions Fill to Center-over-the-
        // explicit-size, so frame.width IS that explicit size; an unset request keeps Fill, so frame.width
        // is the full row/cell width. Either way stamping it straight onto the host is correct — no
        // explicit/unset branch needed here (unlike the ring's own Width/Height just below).
        const host_border host = as_host(platform->native);
        winui::Controls::Canvas::SetLeft(host, frame.x);
        winui::Controls::Canvas::SetTop(host, frame.y);
        host.Width(frame.width);
        host.Height(frame.height);
        // The RING's own size still needs the explicit-only pin — see file-top deviation note 3. A
        // ProgressRing has no Image-style Stretch/Aspect knob to resize its glyph without touching Width/
        // Height directly, so ONLY an explicit WidthRequest/HeightRequest may resize it; otherwise it keeps
        // its native default DesiredSize and just sits recentred by its Center/Center alignment
        // (create_platform_view above) inside the host, which now correctly owns the Fill extent instead.
        const progress_ring ring = as_ring(platform->native);
        const auto* view = virtual_view();
        const double explicit_width = (view != nullptr) ? view->width() : std::numeric_limits<double>::quiet_NaN();
        const double explicit_height = (view != nullptr) ? view->height() : std::numeric_limits<double>::quiet_NaN();
        if (std::isfinite(explicit_width))
        {
            ring.Width(explicit_width);
        }
        if (std::isfinite(explicit_height))
        {
            ring.Height(explicit_height);
        }
        // Clip target resolution (host vs. its Child) is handled INSIDE apply_native_clip itself —
        // view_chrome_ops.cpp's HOST-VS-CHILD note already special-cases a Border-boxed native and
        // redirects the mask onto host.Child(), so passing the host here (as label/image already do) is
        // correct with no changes needed there. Still today's only reachable outcome is a no-op (no
        // gallery page sets Clip on an ActivityIndicator); that helper's own TODO about a non-Stretch
        // Border child (this ring is Center/Center, not Stretch, unlike every current Border-host child)
        // is a pre-existing, already-documented gap this fix does not newly introduce or need to close.
        if (view != nullptr)
        {
            apply_native_clip(platform->native, view->clip());
        }
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so every control behaves identically; see
    // that header for why they are free functions taking the void* slot. `native` now boxes the Border
    // HOST (file header note 2), which is correct for visibility/opacity/background — they should apply to
    // the whole host+ring subtree, and apply_background's three-way try_as already fills a Border — but
    // NOT for is_enabled/automation_id, which are redirected to the ring below.
    void activity_indicator_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void activity_indicator_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void activity_indicator_platform::update_is_enabled(bool value)
    {
        // NOT apply_is_enabled(native, ...): that helper's try_as<Control> silently no-ops on a Border,
        // and `native` is now the Border host, not the ring. label_handler.cpp/image_handler.cpp accept
        // that same no-op because their wrapped content (TextBlock/Image) was never a Control either — but
        // a ProgressRing IS a Control, and this port pushed IsEnabled to it directly before the Border
        // wrap existed. Redirect to the ring so that keeps working instead of silently regressing.
        if (native == nullptr)
        {
            return;
        }
        as_ring(native).IsEnabled(value);
    }

    void activity_indicator_platform::update_automation_id(std::string_view value)
    {
        // Same redirect as update_is_enabled just above, for the same reason: identify the real control
        // for automation/accessibility tooling, not its transparent Border host.
        if (native == nullptr)
        {
            return;
        }
        winui::Automation::AutomationProperties::SetAutomationId(as_ring(native),
                                                                 maui::platform::windows::to_hstring(value));
    }

    void activity_indicator_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
