// run_app — the WINDOWS (WinUI 3) backend body of maui::hosting::run_app (host_run.hpp).
//
// The same mount_window + drive_layout the headless lane runs, wrapped in the WinUI 3 application
// lifecycle. Everything structural here was proven first by tools/parity/windows/winui_probe (a
// standalone code-only C++/WinRT app) precisely so that a failure in THIS file would be about the port
// and not about the toolchain. The three things that probe established, all of which are load-bearing:
//
//   1. THE BOOTSTRAPPER. An UNPACKAGED app must call MddBootstrapInitialize2 before touching any WinUI
//      type, or every activation fails with REGDB_E_CLASSNOTREG - an error that says nothing about
//      bootstrapping.
//   2. IXamlMetadataProvider. Normally generated from App.xaml by the XAML compiler. A code-only app
//      supplies it by delegating to the WinUI controls' own provider, or control activation throws.
//   3. XamlControlsResources MUST BE MERGED IN OnLaunched, NOT IN THE CONSTRUCTOR. Activating a XAML
//      type from the Application ctor dies with a stowed exception (0xC000027B) inside combase, because
//      the App object is not yet fully constructed as far as the XAML framework is concerned.

#include "maui/hosting/host_run.hpp"

#include <windows.h>
// windows.h defines GetCurrentTime as a function-like macro (it aliases GetTickCount). The C++/WinRT
// projection for Microsoft.UI.Xaml.Media.Animation declares a Timeline::GetCurrentTime method, and the
// macro then eats its argument list: "C4002 too many arguments for function-like macro invocation".
// Every WinUI C++ app has to undo it; there is no NOxxx guard that suppresses it.
#undef GetCurrentTime

#include <MddBootstrap.h>

#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/window_handler.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "winui_interop.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    namespace backdrops = winrt::Microsoft::UI::Composition::SystemBackdrops;
    namespace dispatching = winrt::Microsoft::UI::Dispatching;

    // The fallback content size, used only until the real window reports one. A desktop-ish default
    // rather than the headless lane's phone viewport: this backend always has a real window, so this is
    // just the value for the single frame before the first SizeChanged arrives.
    constexpr double k_default_width = 1024.0;
    constexpr double k_default_height = 800.0;

    // The Windows App SDK major/minor the app is built against (1.7). It must match the package restored
    // by tools/parity/windows/build_winui_probe.ps1; a mismatch fails the bootstrap at startup.
    constexpr UINT32 k_wasdk_major_minor = 0x00010007;

    // Per-step boot logging, OFF unless MAUI_WINUI_LOG names a file. This exists because a WinUI
    // failure gives you nothing: a stowed exception (0xC000027B) unwinds through combase with no
    // message, no stack and no console -- the process is simply gone, and WER names a system DLL
    // rather than the call that provoked it. The winui_probe carried the same logging for the same
    // reason; without it you are reduced to guessing, and two guesses have already been wrong on the
    // `device` page. Env-gated so a normal run pays one getenv and writes nothing.
    //
    // Opened and closed per line, deliberately: the whole point is to survive a process that dies
    // mid-call, and a buffered stream would lose exactly the last line -- the one naming the step
    // that killed it.
    void boot_log(std::string_view step)
    {
        const char* const path = std::getenv("MAUI_WINUI_LOG");
        if (path == nullptr)
        {
            return;
        }
        std::FILE* file = nullptr;
        if (fopen_s(&file, path, "a") != 0 || file == nullptr)
        {
            return;
        }
        std::fwrite(step.data(), 1, step.size(), file);
        std::fputc('\n', file);
        std::fclose(file);
    }

    // The native Window behind a mounted maui window, or a null projected object if it is not attached.
    winui::Window native_window_of(maui::controls::window& window)
    {
        auto* handler = dynamic_cast<maui::core::window_handler*>(window.handler().get());
        if (handler == nullptr)
        {
            return nullptr;
        }
        auto* platform = handler->typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return nullptr;
        }
        return maui::platform::windows::ref<winui::Window>(platform->native);
    }

    // The port's own layout viewport: `raw_height` (Window.Bounds.Height) minus the reserved title-bar
    // band, when there is one (winui_interop.hpp's has_extended_title_bar/k_app_title_bar_height).
    // Bounds now covers the WHOLE window rect (window_handler.cpp's create_platform_view extends
    // content into the title bar), so handing it to drive_layout unadjusted would lay out 32 DIPs past
    // where MAUI's own page viewport ends (WindowRootView's content is inset by that same band).
    double content_viewport_height(const winui::Window& native, double raw_height)
    {
        const double inset = maui::platform::windows::has_extended_title_bar(native)
                                 ? maui::platform::windows::k_app_title_bar_height
                                 : 0.0;
        return std::max(0.0, raw_height - inset);
    }

    // The code-only WinUI application object. ApplicationT supplies the Application base; the extra
    // IXamlMetadataProvider is what the XAML compiler would otherwise have generated.
    struct maui_winui_app : winui::ApplicationT<maui_winui_app, winui::Markup::IXamlMetadataProvider>
    {
        explicit maui_winui_app(maui::hosting::app_configurator configure) : configure_(std::move(configure))
        {
            // Deliberately EMPTY of XAML work - see note 3 in the file header.
        }

        void OnLaunched(const winui::LaunchActivatedEventArgs&)
        {
            // Merge the WinUI control styles. Without them the controls still activate but render
            // unstyled, which looks like a broken layout rather than a missing resource dictionary.
            boot_log("OnLaunched: entered");
            Resources().MergedDictionaries().Append(winui::Controls::XamlControlsResources{});
            boot_log("OnLaunched: XamlControlsResources merged");

            // (1) Build the app from a FRESH builder the user's configurator populates.
            boot_log("build: configure_ + build");
            app_ = configure_(maui::hosting::maui_app::create_builder()).build();
            boot_log("build: maui_app built");
            const std::shared_ptr<maui::controls::application>& application = app_->application();
            if (application == nullptr)
            {
                // EXIT, do not just return: Application::Start owns the message loop and keeps pumping
                // after OnLaunched returns, so a bare return leaves a windowless process running forever
                // - invisible in the E2E runner, which then reports a capture timeout rather than a
                // startup failure. The headless lane can `return 0` here because it has no loop to leave.
                Exit();
                return;
            }
            boot_log("window: create_window");
            window_ = dynamic_cast<maui::controls::window*>(application->create_window());
            boot_log("window: created");
            if (window_ == nullptr)
            {
                Exit();
                return;
            }

            // (1a) THE OS THEME INTO THE APP, before (1b) pushes the app's theme back down to XAML.
            //
            //      Windows is the one backend with NO OS-theme source: essentials_app_info has no windows
            //      partial (CMake gives it the HEADLESS fake, which answers `unspecified`), so the ctor seed
            //      that Application.cs:61 performs — platform_app_theme_ = AppInfo.RequestedTheme — reads
            //      "don't know" here and every other lane's fix passes Windows by. Without this, an app that
            //      never sets a theme would sit on {AppThemeBinding}'s Light slot on a dark-mode desktop.
            //
            //      Reading Application.Current().RequestedTheme() is what AppInfo.windows.cs:61-71 does. It
            //      is read BEFORE (1b) writes it, so it still holds the system value rather than our own.
            //      The `unspecified` guard keeps this from overwriting a theme somebody already pushed, and
            //      makes the whole block a no-op the day a real windows app_info partial lands — which is
            //      the honest remaining gap: AppInfo::requested_theme() still answers `unspecified` on
            //      Windows for any caller other than this one.
            if (application->platform_app_theme() == maui::core::app_theme::unspecified)
            {
                if (const auto current = winui::Application::Current())
                {
                    application->set_platform_app_theme(current.RequestedTheme() == winui::ApplicationTheme::Dark
                                                            ? maui::core::app_theme::dark
                                                            : maui::core::app_theme::light);
                }
            }

            // (1b) The APPLICATION theme, set here and nowhere else: Application.RequestedTheme may only
            //      be assigned before the first window exists, and mount_window below is what creates it.
            //      This slot is the only point where both are true - the app has been built (so
            //      requested_theme() is known) and no Window has been made yet.
            //
            //      Why this and not just the content root's FrameworkElement.RequestedTheme (which the
            //      first attempt used): a per-element theme correctly restyles the CONTROLS, but every
            //      manual `Application.Resources.Lookup` still resolves against the APP theme. The dark
            //      capture proved it - white text on a light page, 97% differing. Setting the app theme
            //      makes every theme-resource resolution, mine and the framework's, agree.
            switch (application->requested_theme())
            {
                case maui::core::app_theme::dark:
                    winui::Application::Current().RequestedTheme(winui::ApplicationTheme::Dark);
                    break;
                case maui::core::app_theme::light:
                    winui::Application::Current().RequestedTheme(winui::ApplicationTheme::Light);
                    break;
                case maui::core::app_theme::unspecified:
                    break; // leave the system default
            }
            boot_log("theme: application theme set");

            // (2) Generic mount: handlers across the tree (children before parents), the window handler
            //     last - which creates the native Window and hosts the page as its Content.
            boot_log("mount: mount_window");
            maui::hosting::mount_window(*app_, *window_);
            boot_log("mount: mounted");

            const winui::Window native = native_window_of(*window_);
            if (native == nullptr)
            {
                Exit(); // see the note above: never leave the loop pumping with nothing on screen
                return;
            }

            // (3) Re-layout on every resize. This is not a nicety: the E2E runner pins the window to an
            //     explicit rect AFTER launching the process, so the size the app boots at is never the
            //     size it is captured at. Without this the capture would show the boot layout.
            native.SizeChanged([this, native](const winrt::Windows::Foundation::IInspectable&,
                                              const winui::WindowSizeChangedEventArgs& args) {
                if (window_ != nullptr)
                {
                    maui::hosting::drive_layout(*window_, args.Size().Width,
                                                content_viewport_height(native, args.Size().Height));
                }
            });

            // (3b) Native theme from the app's REQUESTED theme - the parity-capture dark/light path, and
            //      the twin of what the iOS host does with overrideUserInterfaceStyle. The gallery sets
            //      the cross-platform theme in pure C++ (application::set_platform_app_theme, seeded from
            //      MAUI_APPEARANCE); without pushing it to XAML here, every native control would keep
            //      rendering in the system theme and a "dark" capture would come back LIGHT - a
            //      plausible-looking but entirely wrong board row.
            //
            //      Belt-and-braces on top of the APPLICATION theme set at (1b), which is the actual
            //      mechanism. Kept because it makes the root's theme explicit in the visual tree and
            //      costs one property set; if (1b) ever cannot run (an already-initialised Application),
            //      this still restyles the controls even though resource lookups would go light.
            if (const auto themed = native.Content().try_as<winui::FrameworkElement>())
            {
                switch (application->requested_theme())
                {
                    case maui::core::app_theme::dark:
                        themed.RequestedTheme(winui::ElementTheme::Dark);
                        break;
                    case maui::core::app_theme::light:
                        themed.RequestedTheme(winui::ElementTheme::Light);
                        break;
                    case maui::core::app_theme::unspecified:
                        themed.RequestedTheme(winui::ElementTheme::Default);
                        break;
                }
            }

            // (3b-ii) Application.Windows.cs `ApplyThemeToWindow` calls SetTitleBarButtonColors(
            //      platformWindow, isDark) IMMEDIATELY after setting root.RequestedTheme, inside the same
            //      dispatcher lambda (:112-119) — so this is that call, in that position.
            //
            //      This is the SECOND, theme-keyed half of the oracle's title-bar colour handling. The
            //      first half (MauiWinUIWindow's own ctor: ExtendsContentIntoTitleBar + the two
            //      theme-INDEPENDENT transparent BACKGROUNDS) is already ported, in window_handler.cpp's
            //      create_platform_view. That file's comment claimed the remaining properties "only paint
            //      on pointer interaction, which a static parity capture never exercises" — TRUE of the
            //      four hover/pressed properties, but FALSE of ButtonForegroundColor, which is the RESTING
            //      colour of the minimize/maximize/close glyphs. Left at its default, that strip follows
            //      the SYSTEM theme rather than the app's, so a dark capture drew near-black glyphs on the
            //      dark caption bar. Measured: an identical 98-pixel diff at y11-20 x896-997 on ALL 58
            //      dark pages of the board — universal and constant, not a per-page or capture-time
            //      artifact.
            //
            //      Ported faithfully rather than foreground-only: the hover/pressed values cost one call
            //      each and omitting them would leave the port silently diverging the moment pointer
            //      capture is ever added. Values are Application.Windows.cs's TitleBarColors (:144-152):
            //      Light/DarkForeground = Black/White, Hover = #18 alpha, Pressed = #1F alpha.
            if (winrt::Microsoft::UI::Windowing::AppWindowTitleBar::IsCustomizationSupported())
            {
                if (const auto app_window = native.AppWindow())
                {
                    if (const auto title_bar = app_window.TitleBar())
                    {
                        const bool is_dark = application->requested_theme() == maui::core::app_theme::dark;
                        // Aggregate-initialised rather than via ColorHelper::FromArgb (the oracle's C#
                        // ergonomics for the same struct): calling a ColorHelper MEMBER would drag in
                        // winrt/Windows.UI.h under this backend's full-header-for-members rule, and the
                        // rest of the backend already builds Color{a,r,g,b} directly.
                        const std::uint8_t ch = is_dark ? 255 : 0;
                        const winrt::Windows::UI::Color fg{255, ch, ch, ch};
                        const winrt::Windows::UI::Color hover_bg{24, ch, ch, ch};   // TitleBarColors #18 alpha
                        const winrt::Windows::UI::Color pressed_bg{31, ch, ch, ch}; // TitleBarColors #1F alpha
                        title_bar.ButtonHoverBackgroundColor(hover_bg);
                        title_bar.ButtonPressedBackgroundColor(pressed_bg);
                        title_bar.ButtonHoverForegroundColor(fg);
                        title_bar.ButtonPressedForegroundColor(fg);
                        title_bar.ButtonForegroundColor(fg);
                    }
                }
            }

            // (3c) The themed PAGE BACKGROUND — an opaque fallback paint, now applied ONLY when the
            //      window has no Mica backdrop to show through.
            //
            //      MauiWinUIWindow.cs:52-55 DOES set a Mica BaseAlt SystemBackdrop
            //      (window_handler.cpp's create_platform_view now ports that line verbatim, guarded
            //      the same way: `if (MicaController.IsSupported())`). An EARLIER attempt applied it and
            //      measured light 232 at the body — that IS the raw Mica value, so the backdrop DID
            //      reach the screen (this block must have been bypassed or not yet written for that
            //      test, since an opaque #F3F3F3 paint would have read back 243, not 232). It was
            //      retired as "worse" purely by body-vs-body comparison: 232 is farther from MAUI's 244
            //      than the flat #F3F3F3 fallback's 243 was. PARITY_REVIEW.md's TASK 1 (2026-07-31)
            //      later showed why: the missing piece was never the backdrop, it was (3d)'s translucent
            //      content layer, which that test never had — measured over the raw 232 base it composes
            //      to exactly MAUI's 244 (see (3d) below). So the earlier retirement was correct about
            //      the SYMPTOM (232 alone looks worse) but wrong about the CAUSE (nothing to do with an
            //      opaque overpaint, which this block did not yet gate at the time).
            //
            //      What DOES have to change now that both pieces exist together: an opaque Background
            //      assigned here composites IN FRONT of the window's SystemBackdrop (the backdrop draws
            //      behind the whole XAML content island), so if this block ran unconditionally today it
            //      WOULD hide the backdrop just applied in create_platform_view — a real defeat, just not
            //      the one that sank the earlier attempt. So: on a Mica-capable system this block must do
            //      NOTHING — leave the panel's Background unset/transparent so the backdrop shows through
            //      directly, exactly like MAUI's own WindowRootView (WindowRootViewStyle.xaml's root
            //      `Page` sets no explicit Background of its own — verified by reading the style, not
            //      assumed).
            //
            //      The flat-brush paint below is kept ONLY as the non-Mica fallback: on a system where
            //      `MicaController.IsSupported()` is false, the window never got a backdrop (mirroring
            //      the oracle's own guard in the ctor), so there is nothing for a transparent panel to
            //      expose — MAUI's Page would fall back to its own default themed background there too,
            //      which this reproduces.
            //
            //      Logged either way (boot_log below): the two arms are visually indistinguishable from
            //      "the change didn't take" if MicaController::IsSupported() ever comes back false on a
            //      guest believed to support Mica, and that would otherwise cost a full guest round trip
            //      to notice.
            //
            //      Applied ONLY when the page set no Background of its own: ReadLocalValue returns
            //      UnsetValue exactly when the mapper did not push one, so an explicit page colour
            //      still wins. Looked up AFTER RequestedTheme, so it is the right theme's brush.
            //
            //      `native.Content()` is now the title-bar-band wrapper Grid when the window extends
            //      content into the title bar (window_handler.cpp's host_content), not the page itself
            //      — the page's own Background (if any) is set independently, on the page's native view
            //      one level down, and paints over this default everywhere except the reserved band.
            //      So `panel` here never carries its own local value and this always paints the Grid —
            //      which is exactly right: it is what shows through the exposed band, matching MAUI's
            //      title bar having its own theme-coloured strip independent of the page's background.
            if (!backdrops::MicaController::IsSupported())
            {
                boot_log("theme: window base = flat fallback -- MicaController not supported");
                if (const auto panel = native.Content().try_as<winui::Controls::Panel>())
                {
                    const bool has_own_background =
                        panel.ReadLocalValue(winui::Controls::Panel::BackgroundProperty()) !=
                        winui::DependencyProperty::UnsetValue();
                    if (!has_own_background)
                    {
                        const auto resources = Resources();
                        const auto key = winrt::box_value(winrt::hstring{L"ApplicationPageBackgroundThemeBrush"});
                        if (resources.HasKey(key))
                        {
                            panel.Background(resources.Lookup(key).as<winui::Media::Brush>());
                        }
                    }
                }
            }
            else
            {
                boot_log("theme: window base = Mica backdrop (BaseAlt)");
            }

            // (3d) The themed CONTENT LAYER, painted translucently on the PAGE's own native view — one
            //      level below (3c)'s panel. PARITY_REVIEW.md "TASK 1 SOLVED" (2026-07-31): real MAUI
            //      hosts page content inside a NavigationView whose content area is painted
            //      `NavigationViewContentBackground`, a StaticResource to `LayerFillColorDefaultBrush`
            //      (WinUI generic.xaml) — itself translucent (#4C3A3A3A dark / #80FFFFFF light) — layered
            //      OVER whatever base is visible beneath the page: on the normal Mica-capable system
            //      that base is the RAW SystemBackdrop (3c) now deliberately leaves unobscured (measured
            //      232 light / 32 dark); on the non-Mica fallback it is (3c)'s opaque paint instead.
            //      Composited over the Mica base this reproduces the observed body colour in both themes
            //      (dark 32+(76/255)*(58-32)=39.75, light 232+(128/255)*(255-232)=243.55). Before this
            //      layer existed the port painted content with the window base itself, so the title-bar
            //      band and the body were IDENTICAL (delta 0) where MAUI shows +7 dark / +12 light.
            //
            //      Reached via window_->content()'s own handler (the same lookup window_handler.cpp's
            //      host_content() uses), not by walking native.Content()'s children, so this lands on the
            //      page's native view whether or not the title bar is extended. Gated on
            //      has_extended_title_bar because only then does (3c)'s panel above name a DIFFERENT
            //      element (the wrapper Grid) from this one (window_handler.cpp's host_content) — without
            //      the wrapper, native.Content() IS the page's own view, and on the non-Mica fallback
            //      (3c) already painted it there; painting it again here would replace that base with
            //      this layer instead of compositing over it, and there is no reserved band on that
            //      system to distinguish anyway. (On the Mica path (3c) paints nothing either way, so
            //      this guard only matters for the fallback.)
            //
            //      Same has_own_background guard as (3c): an explicit page Background is already a LOCAL
            //      value on this property by now (content_page_handler.cpp's update_background ran during
            //      mount_window, above), so ReadLocalValue is non-Unset and this leaves it untouched.
            //      Confirmed, not assumed: view_mapper.cpp's map_background pushes `view.background()`
            //      unconditionally for every view including pages with none set, and winui_visual_ops.cpp's
            //      apply_background(slot, nullptr) CLEARS the property rather than painting Transparent —
            //      so an unset page background really does read back UnsetValue here.
            //
            //      ponytail: this DP is now OWNED by this step — map_background (view_mapper.cpp) still
            //      pushes here on every future page Background change (including a clear back to unset),
            //      which would silently wipe the layer with no repaint, and a page swapped into the window
            //      post-boot gets no layer at all, since this fires once in OnLaunched like every other
            //      step in this function. Both are fine for the parity lane (one page, no post-boot
            //      Background churn, per process launch). Fixing either means moving this into
            //      content_page_handler.cpp's own mapper — deliberately NOT done here, because the
            //      resource lookup must happen AFTER the app theme is set at (1b)/(3b), which
            //      create_platform_view (mount time) precedes.
            if (maui::platform::windows::has_extended_title_bar(native))
            {
                auto* const page = window_->content();
                auto* const page_handler =
                    page != nullptr ? dynamic_cast<maui::core::i_view_handler*>(page->handler().get()) : nullptr;
                if (page_handler != nullptr && page_handler->native_view() != nullptr)
                {
                    const winui::UIElement page_view =
                        maui::platform::windows::ref<winui::UIElement>(page_handler->native_view());
                    if (const auto page_panel = page_view.try_as<winui::Controls::Panel>())
                    {
                        const bool has_own_background =
                            page_panel.ReadLocalValue(winui::Controls::Panel::BackgroundProperty()) !=
                            winui::DependencyProperty::UnsetValue();
                        if (!has_own_background)
                        {
                            const auto resources = Resources();
                            const auto key = winrt::box_value(winrt::hstring{L"NavigationViewContentBackground"});
                            if (resources.HasKey(key))
                            {
                                page_panel.Background(resources.Lookup(key).as<winui::Media::Brush>());
                                boot_log("theme: content layer painted");
                            }
                            else
                            {
                                // Silent-miss guard: HasKey==false leaves the page unpainted, which looks
                                // identical to a successful no-op. One line turns a mystery rescore into an
                                // immediate answer instead of a re-derivation of this whole comment block.
                                boot_log("theme: content layer resource NOT FOUND -- NavigationViewContentBackground "
                                         "missing");
                            }
                        }
                    }
                    else
                    {
                        // Same silent-miss concern as above: a root page whose native view isn't a Panel
                        // (not true for content_page_handler.cpp's Canvas today, but nothing enforces it)
                        // would skip the whole block with nothing logged.
                        boot_log("theme: content layer skipped -- page native view is not a Panel");
                    }
                }
            }

            // (4) Show it, then lay out at the size it actually got. Activate first because a WinUI
            //     window has no client size until it is shown - laying out before would use the fallback
            //     and then immediately be corrected by the SizeChanged above.
            //
            //     CORRECTED (this block used to say "no suppression needed here" -- that conclusion was
            //     WRONG, disproven by direct pixel measurement, not by argument). The reasoning below this
            //     paragraph is the OLD claim, kept for the trail; do not trust its conclusion.
            //
            //     Re-measured directly (Pillow, committed captures, 2026-07-31): `pickers_light.png` and
            //     `pickers_dark.png` both show a sharp, full-width near-black/white rectangle (light: RGB
            //     26,26,26 at rows 55-56/93-94 around the "Pick a room" ComboBox; dark: the matching
            //     high-contrast band at the same two rows, ~252k-pixel delta vs the `maui` column) PLUS a
            //     solid accent-blue vertical bar inside the box (a caret) -- on BOTH the `cpp` AND `xaml`
            //     capture columns, byte-identical between the two, absent from `maui` in both themes. So
            //     this port's own gallery (both framework builds -- they share this file) CAN and DOES
            //     carry a keyboard-focus visual at capture time, on at least the `pickers` page. Separately,
            //     `search_bar_light.png`/`search_bar_dark.png` show the same mechanism but on the OTHER
            //     side (the `maui` column carries the accent-color focus underline under the SearchBar;
            //     `cpp`/`xaml` are clean there) -- i.e. this is a capture-time input-focus race that can
            //     land on EITHER side, not a one-directional MAUI-only artifact.
            //
            //     The OLD reasoning below is still factually true as far as it goes -- this Windows backend
            //     really has no explicit initial-focus Focus() call anywhere (grepped again: only
            //     time_picker_handler.cpp's popup open/close focus juggling and editor_handler.cpp /
            //     search_bar_handler.cpp's Got/LostFocus listeners, none of them a startup focus setter) --
            //     but that fact does not imply nothing ever becomes focused. WinUI's own window-activation
            //     path can hand keyboard focus to the first focusable descendant when a window with no
            //     explicit focus owner is Activate()'d, independent of MAUI's Frame-style
            //     AddPage/TryMoveFocusToPage this port never runs. Whatever the exact trigger, the fix does
            //     not need to name it: read back whatever IS focused right before capture and neutralize
            //     it, the same lever the reference column already uses successfully (see the block right
            //     after drive_layout, below).
            boot_log("activate: before");
            native.Activate();
            boot_log("activate: after");
            // Force a XAML layout pass BEFORE the port measures anything. Until one runs, a templated
            // control (a Button) has no template applied, so UIElement::Measure reports only its bare
            // content size - the port then arranges every button ~19px tall with no chrome, which is
            // exactly what the first Windows capture showed. UpdateLayout() applies templates and
            // settles the tree synchronously, so the DesiredSize the port reads afterwards is the real
            // themed one.
            if (const auto root = native.Content().try_as<winui::FrameworkElement>())
            {
                boot_log("layout: UpdateLayout");
                root.UpdateLayout();
                boot_log("layout: UpdateLayout done");
            }
            // Window.Bounds, NOT AppWindow.ClientSize: Bounds is in DIPs, which is the space XAML lays
            // out in and therefore the space the port's own measure/arrange works in. ClientSize is in
            // PHYSICAL PIXELS, so on any display above 100% scale it would hand the layout a viewport
            // 1.25x/1.5x too large -- correct-looking on the 100% parity guest and quietly wrong
            // everywhere else.
            double width = k_default_width;
            double height = k_default_height;
            const auto bounds = native.Bounds();
            if (bounds.Width > 0 && bounds.Height > 0)
            {
                width = bounds.Width;
                height = content_viewport_height(native, bounds.Height);
            }
            boot_log("layout: drive_layout");
            maui::hosting::drive_layout(*window_, width, height);
            boot_log("layout: drive_layout done -- boot complete");

            // Capture-determinism opt-in, the PORT-SIDE half of MAUI_SUPPRESS_FOCUS_VISUAL -- the twin of
            // port/maui-reference/app/App.xaml.cs's #if WINDOWS block. That block existed alone until the
            // comment above this one was found wrong by measurement: this port's own `pickers` capture
            // (both the `cpp` and `xaml` columns, which share this file) can carry the identical keyboard-
            // focus rectangle + caret the reference column shows on OTHER pages, so a one-sided fix leaves
            // a page where the noise floor sits on this port's side undetected by the reference-only
            // suppression. run_comparison.py already sets this env var unconditionally on every column's
            // launch (same shape as MAUI_CAPTURE_TINT_NORMAL) -- it was simply never read on this side
            // until now.
            //
            // Same recipe as the reference, on purpose (Focus(FocusState::Pointer) is what WinUI's stock
            // OnGotFocus/GoToState template logic treats as "no rectangle needed"; UseSystemFocusVisuals on
            // a Control is the template-level switch that suppresses the system-drawn rect independently of
            // any FocusState transition): a one-shot 200ms DispatcherQueueTimer (mirroring the reference's
            // `await Task.Delay(200)` on page.Loaded -- there is no page.Loaded here, so this is armed right
            // after the first drive_layout instead, the closest equivalent "boot settled" point) reads back
            // whatever FocusManager reports focused at that moment and neutralizes it. Deliberately does
            // NOT special-case which control it is or why it got focus -- see the corrected comment above:
            // the trigger doesn't need to be named for this to neutralize it.
            //
            // Microsoft::UI::Dispatching, NOT Windows::System -- same WinUI-3-vs-UWP DispatcherQueue
            // collision image_source_services.cpp's fetch_uri_async note already documents for this
            // backend; GetForCurrentThread() on the other (UWP) type returns null on this app's thread.
            //
            // The timer is a MEMBER (focus_suppress_timer_), not a local -- a local one-shot timer that
            // goes out of scope before Start()'s 200ms elapses is a real risk this codebase's own house
            // rule (winrt handle capture, not raw pointers) exists to avoid; keeping it alive as long as
            // the app object removes any doubt rather than trusting undocumented DispatcherQueue-internal
            // keep-alive behavior neither this comment nor its author has verified.
            if (const char* const suppress_focus = std::getenv("MAUI_SUPPRESS_FOCUS_VISUAL");
                suppress_focus != nullptr && std::string_view{suppress_focus} == "1")
            {
                if (const auto queue = dispatching::DispatcherQueue::GetForCurrentThread())
                {
                    focus_suppress_timer_ = queue.CreateTimer();
                    focus_suppress_timer_.Interval(std::chrono::milliseconds(200));
                    focus_suppress_timer_.IsRepeating(false);
                    focus_suppress_timer_.Tick([native](auto&&, auto&&) {
                        const auto root = native.Content().try_as<winui::FrameworkElement>();
                        const auto xaml_root = root ? root.XamlRoot() : nullptr;
                        if (xaml_root == nullptr)
                        {
                            boot_log("defocus: no XamlRoot yet");
                            return;
                        }
                        const auto focused_obj = winui::Input::FocusManager::GetFocusedElement(xaml_root);
                        if (const auto focused = focused_obj.try_as<winui::UIElement>())
                        {
                            const bool refocused = focused.Focus(winui::FocusState::Pointer);
                            if (const auto control = focused.try_as<winui::Controls::Control>())
                            {
                                control.UseSystemFocusVisuals(false);
                            }
                            boot_log(refocused ? "defocus: applied, refocused=true"
                                               : "defocus: applied, refocused=false");
                        }
                        else
                        {
                            boot_log("defocus: nothing focused");
                        }
                    });
                    focus_suppress_timer_.Start();
                    boot_log("defocus: timer armed");
                }
            }

            // Install the relayout hook (window::request_relayout) AFTER the first pass -- mirrors the
            // Android-only jni/relayout.hpp precedent, generalized to every backend (see window.hpp's
            // header comment). Re-reads native.Bounds() at call time, like the SizeChanged handler above,
            // rather than replaying the captured boot size -- so a leaf's invalidate_measure() (e.g. a
            // margin/orientation/SafeAreaEdges change post-boot) lays out at whatever size the window is
            // CURRENTLY showing. Deliberately does NOT wire anything new to CALL this hook (e.g. Image's
            // ImageOpened) -- that is the consumer, out of scope for this seam.
            window_->set_relayout_hook([this, native] {
                if (window_ == nullptr)
                {
                    return;
                }
                const auto current_bounds = native.Bounds();
                maui::hosting::drive_layout(*window_, current_bounds.Width,
                                            content_viewport_height(native, current_bounds.Height));
            });
        }

        // ---- IXamlMetadataProvider: delegate to the WinUI controls' own provider ---------------------
        // Every XAML type resolved at runtime comes through here. Returning empty (the tempting stub)
        // makes control activation fail later with an opaque error.
        winui::Markup::IXamlType GetXamlType(const winrt::Windows::UI::Xaml::Interop::TypeName& type)
        {
            return provider_.GetXamlType(type);
        }
        winui::Markup::IXamlType GetXamlType(const winrt::hstring& full_name)
        {
            return provider_.GetXamlType(full_name);
        }
        winrt::com_array<winui::Markup::XmlnsDefinition> GetXmlnsDefinitions()
        {
            return provider_.GetXmlnsDefinitions();
        }

    private:
        maui::hosting::app_configurator configure_;
        std::unique_ptr<maui::hosting::maui_app> app_;
        // NON-owning: the window is owned by the application (IApplication.CreateWindow), which the
        // maui_app above owns - so it outlives every use here.
        maui::controls::window* window_ = nullptr;
        winui::XamlTypeInfo::XamlControlsXamlMetaDataProvider provider_;
        // Owns the MAUI_SUPPRESS_FOCUS_VISUAL one-shot timer (see the block right after
        // "layout: drive_layout done" in OnLaunched) for the app's lifetime, so it cannot be destroyed
        // before its 200ms elapses. Null (unarmed) unless that env var is set.
        dispatching::DispatcherQueueTimer focus_suppress_timer_{nullptr};
    };
} // namespace

namespace maui::hosting
{
    int run_app(int /*argc*/, char** /*argv*/, app_configurator configure)
    {
        // The bootstrapper FIRST: unpackaged apps have no package graph, so without it every WinUI
        // activation below fails with REGDB_E_CLASSNOTREG.
        if (FAILED(MddBootstrapInitialize2(k_wasdk_major_minor, L"", PACKAGE_VERSION{},
                                           MddBootstrapInitializeOptions_OnNoMatch_ShowUI)))
        {
            ::MessageBoxW(nullptr, L"The Windows App Runtime is not installed.", L"maui", MB_ICONERROR);
            return 1;
        }

        int exit_code = 0;
        try
        {
            boot_log("main: bootstrap ok");
            winrt::init_apartment(winrt::apartment_type::single_threaded);
            boot_log("main: apartment initialised");
            // Application::Start owns the message loop and returns only when the app exits, so this call
            // IS the run loop - the Windows analogue of UIApplicationMain.
            winui::Application::Start([configure = std::move(configure)](auto&&) mutable {
                winrt::make<maui_winui_app>(std::move(configure));
            });
        }
        catch (const winrt::hresult_error&)
        {
            exit_code = 2;
        }
        catch (const std::exception&)
        {
            exit_code = 3;
        }
        MddBootstrapShutdown();
        return exit_code;
    }
} // namespace maui::hosting
