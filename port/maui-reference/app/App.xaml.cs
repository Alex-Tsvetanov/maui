using Microsoft.Maui.Media;
using Microsoft.Maui.Storage;
#if ANDROID
using Microsoft.Maui.ApplicationModel;
#endif

namespace MauiReference;

public partial class App : Application
{
    public App()
    {
        InitializeComponent();
        // NOTE: UserAppTheme is set in CreateWindow (below), NOT here. On Android the theme is carried as a
        // launch-intent extra, but at App-ctor time Platform.CurrentActivity is still null (the Activity is
        // created AFTER the App), so ResolveValue("MAUI_THEME") reads nothing here and the app stayed Light on
        // every dark run. CreateWindow runs once the Activity + its Intent exist (same place MAUI_COMPARE_PAGE
        // is read), so the theme toggle is reliable there.
    }

    // Resolves a parity knob from the platform first (Android intent extra), then the process env var.
    // On Android `am start ... --es NAME value` carries the knob in the launcher Activity's intent extras
    // (process env is NOT inherited from am start); on iOS/maccatalyst the env var route is unchanged.
    static string? ResolveValue(string name)
    {
#if ANDROID
        var intent = Platform.CurrentActivity?.Intent;
        var fromIntent = intent?.GetStringExtra(name);
        if (!string.IsNullOrEmpty(fromIntent))
            return fromIntent;
#endif
        return Environment.GetEnvironmentVariable(name);
    }

    protected override Window CreateWindow(IActivationState? activationState)
    {
        // Parity-comparison theme toggle: MAUI_THEME=Dark|Light forces the app appearance so captures match
        // light-vs-light and dark-vs-dark against the C++ galleries (default: Light). Read here (not in the
        // ctor) so Platform.CurrentActivity + its intent extras exist — same reliable point as the page key.
        UserAppTheme = ResolveValue("MAUI_THEME") == "Dark"
            ? Microsoft.Maui.ApplicationModel.AppTheme.Dark
            : Microsoft.Maui.ApplicationModel.AppTheme.Light;
        var key = ResolveValue("MAUI_COMPARE_PAGE") ?? "controls_stack";
        var page = PageDispatch.Create(key);
        var window = new Window(page);

#if WINDOWS
        // Capture-determinism opt-in (MAUI_SUPPRESS_FOCUS_VISUAL=1, set identically on every column by
        // run_comparison.py -- same shape as MAUI_CAPTURE_TINT_NORMAL in MauiProgram.cs). WindowRootViewContainer
        // auto-focuses the page's first focusable control on every AddPage
        // (src/Core/src/Platform/Windows/WindowRootViewContainer.cs's TryMoveFocusToPage -> SetFocusToFirstElement,
        // `focusableElement.Focus(FocusState.Programmatic)`), and WinUI's stock control templates paint their
        // keyboard-style focus outline for Programmatic focus exactly like Keyboard focus -- only Pointer/touch
        // focus is exempt. That outline is what PARITY_REVIEW.md's "MAUI's own CollectionView captures carry a
        // ~0.50pp focus-visual noise floor" / "Focus-visual suppression" sections measured: ~0.50% of pixels,
        // present or absent from run to run with the port's code byte-identical, pushing up to ~22 pages under
        // the SSIM gate. A prior attempt suppressed it via SetForegroundWindow(GetShellWindow()) right before
        // the host's screenshot -- it worked (0.50% -> 0.01%) but deactivated the session-1 capture agent's own
        // desktop mid-run and had to be reverted (e2e.py's --defocus, opt-in / OFF by default since). This is
        // the in-process alternative that review left as the next thing to try: leave the focused element
        // exactly where AddPage put it, just re-affirm it with FocusState.Pointer, which every stock control
        // template's OnGotFocus/GoToState logic treats as "no rectangle needed" -- nothing here touches window
        // activation or the shell, so it cannot repeat that failure.
        if (Environment.GetEnvironmentVariable("MAUI_SUPPRESS_FOCUS_VISUAL") == "1")
        {
            page.Loaded += async (_, _) =>
            {
                // AddPage's own initial-focus call is synchronous with (or chained directly off) this same
                // page.Loaded event, so by the time any continuation queued after it runs, that focus is
                // already set -- there is nothing to race here. run_comparison.py's own --settle default is
                // 1.0s (two of them elapse -- launch and the scenario's one step -- before the host's
                // --shot), so this 200ms delay, which only waits out layout settling and not the focus
                // itself, has a wide margin before any capture can happen.
                await Task.Delay(200);
                if (page.Handler?.PlatformView is Microsoft.UI.Xaml.FrameworkElement root &&
                    root.XamlRoot is { } xamlRoot &&
                    Microsoft.UI.Xaml.Input.FocusManager.GetFocusedElement(xamlRoot) is Microsoft.UI.Xaml.UIElement focused)
                {
                    // Two independent levers, neither assumed sufficient alone: (a) re-Focus with
                    // FocusState.Pointer, which WinUI's stock OnGotFocus/GoToState template logic treats as
                    // "no rectangle needed" -- BUT Focus(FocusState)'s own documented "Programmatic retains
                    // the prior state" behavior means a re-Focus call on an ALREADY-focused element is not
                    // safe to assume always reassigns FocusState; and (b) UseSystemFocusVisuals=false on
                    // that element when it is a Control -- a template-level switch that suppresses the
                    // system-drawn rect independently of any FocusState transition happening at all. Logged,
                    // not assumed -- this repo's own idiom for "did the workaround actually take" (see
                    // _defocus_before_shot's requested_ok/verified in vm_agent_windows.py) -- so a board run
                    // that still shows the band points at this line instead of a fresh forensics pass.
                    var refocused = focused.Focus(Microsoft.UI.Xaml.FocusState.Pointer);
                    if (focused is Microsoft.UI.Xaml.Controls.Control control)
                    {
                        control.UseSystemFocusVisuals = false;
                    }
                    Console.Out.WriteLine($"[reference] DEFOCUS {focused.GetType().Name} refocused={refocused}");
                }
            };
        }
#endif

        if (Environment.GetEnvironmentVariable("MAUI_SHOT") == "1")
        {
            page.Loaded += async (_, _) =>
            {
                // Sandbox-safe path inside the app container; the harness copies it out via the logged path.
                var shot = Path.Combine(FileSystem.CacheDirectory, key + ".png");
                try
                {
                    await Task.Delay(1200);
                    var result = await Screenshot.Default.CaptureAsync();
                    using (var src = await result.OpenReadAsync())
                    using (var dst = File.Create(shot))
                        await src.CopyToAsync(dst);
                    Console.Out.WriteLine($"[reference] SHOT {shot}");
                }
                catch (Exception ex) { Console.Out.WriteLine("[reference] SHOTFAIL " + ex.Message); }
                Console.Out.Flush();
                Application.Current?.Quit();
            };
        }
        return window;
    }
}
