using System.Linq;
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
        // Parity-comparison theme toggle: MAUI_THEME=Dark|Light forces the app appearance. It is an OVERRIDE,
        // and ONLY an override — unset means FOLLOW THE SYSTEM THEME, which is what UserAppTheme.Unspecified
        // means to MAUI (RequestedTheme falls through to PlatformAppTheme = AppInfo.RequestedTheme).
        //
        // This used to assign Light whenever MAUI_THEME was absent, which made the reference column ignore the
        // OS exactly as the C++ galleries did before 2a3a4eb090. With both sides pinned by an env var, the
        // board could never answer the only question that matters here: does each framework follow the
        // SYSTEM-WIDE theme? Leaving it Unspecified lets a system-light and a system-dark capture pass be
        // genuinely comparable, because both columns are then reading the same source the OS exposes.
        //
        // Read here (not in the ctor) so Platform.CurrentActivity + its intent extras exist — same reliable
        // point as the page key.
        var themeOverride = ResolveValue("MAUI_THEME");
        if (!string.IsNullOrEmpty(themeOverride))
        {
            UserAppTheme = themeOverride == "Dark"
                ? Microsoft.Maui.ApplicationModel.AppTheme.Dark
                : Microsoft.Maui.ApplicationModel.AppTheme.Light;
        }
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

                    // (c) A THIRD lever, because (a) and (b) are STRUCTURALLY INCAPABLE of clearing a text
                    // control's focus paint. Both act only on the SYSTEM-DRAWN focus rect. That is the only
                    // focus pixel a Button has -- WinUI's generic.xaml declares DefaultButtonStyle at :27399
                    // and its ControlTemplate's CommonStates group is Normal/PointerOver/Pressed/Disabled
                    // with NO Focused state at all -- which is why (a)+(b) alone took `label` light from
                    // 0.50% to 0.01%. But AutoSuggestBoxTextBoxStyle (:15988, wired to AutoSuggestBox at
                    // :34199) -> ControlTemplate TargetType="TextBox" (:16006) -> CommonStates ->
                    // <VisualState x:Name="Focused"> repaints the CONTROL ITSELF: BorderElement.Background =
                    // TextControlBackgroundFocused, BorderElement.BorderBrush = TextControlBorderBrushFocused,
                    // BorderElement.BorderThickness = TextControlBorderThemeThicknessFocused = "1,1,1,2"
                    // (:1965 Dark dict / :7544 Light) -- a 2px BOTTOM edge in the accent gradient. That is
                    // exactly the measured search_bar artifact: rows 109-110, 1946 px, plus the box interior
                    // switching ControlFillColorDefault -> ControlFillColorInputActive. CommonStates carries
                    // no FocusState discrimination, so Pointer focus enters "Focused" identically to
                    // Programmatic focus. Only genuinely LOSING focus clears it.
                    //
                    // VERIFIED NECESSARY, not assumed: a full-board recapture on a build already containing
                    // (a)+(b) still scored search_bar light 0.9927/0.24%, dark 0.9754/0.43%.
                    //
                    // Blur to the OUTERMOST Control ancestor, never the nearest: an AutoSuggestBox is the
                    // nearest Control above its own inner TextBox (SearchBarHandler.Windows.cs:6 makes the
                    // Windows SearchBar an AutoSuggestBox, and its own comment notes the inner TextBox is
                    // what actually takes focus), and focusing it just delegates straight back down.
                    // WindowRootView sets IsTabStop=false in its ctor (WindowRootView.cs:42), so it must be
                    // re-enabled before Focus() will take; its ControlTemplate
                    // (Styles/WindowRootViewStyle.xaml:63) declares no VisualStateManager at all, so
                    // focusing it paints nothing of its own beyond the system rect, which we also disable.
                    Microsoft.UI.Xaml.Controls.Control blurTarget = null;
                    for (var node = Microsoft.UI.Xaml.Media.VisualTreeHelper.GetParent(focused);
                         node is not null;
                         node = Microsoft.UI.Xaml.Media.VisualTreeHelper.GetParent(node))
                    {
                        if (node is Microsoft.UI.Xaml.Controls.Control candidate)
                        {
                            blurTarget = candidate;
                        }
                    }

                    var blurred = false;
                    if (blurTarget is not null)
                    {
                        blurTarget.IsTabStop = true;
                        blurTarget.UseSystemFocusVisuals = false;
                        // Re-read focus rather than trusting Focus()'s return: WinUI can report true while
                        // delegating focus straight back to the same descendant, and a bounce-back would
                        // otherwise be indistinguishable from a real blur in the log below.
                        blurred = blurTarget.Focus(Microsoft.UI.Xaml.FocusState.Programmatic) &&
                                  !ReferenceEquals(
                                      Microsoft.UI.Xaml.Input.FocusManager.GetFocusedElement(xamlRoot), focused);
                    }

                    var nowFocused = Microsoft.UI.Xaml.Input.FocusManager.GetFocusedElement(xamlRoot);
                    Console.Out.WriteLine(
                        $"[reference] DEFOCUS {focused.GetType().Name} refocused={refocused} " +
                        $"blurred={blurred} now={(nowFocused as object)?.GetType().Name ?? "none"}");
                }
            };
        }
#endif

#if MACCATALYST
        // TEMPORARY FORENSICS, OPT-IN (MAUI_GEOMETRY_DUMP=1) and OFF for every board run. lldb cannot attach
        // on this VM -- `DevToolsSecurity` is disabled and enabling it needs an admin password -- so the two
        // open Catalyst questions are answered from inside the app instead. Same shape as the
        // MAUI_SUPPRESS_FOCUS_VISUAL block above: gated, post-Loaded, and it never touches rendering.
        //
        // Q1  the 32px page offset (clip / swipe_item_size / path_gallery are RED, scroll_view is GREEN):
        //     which UIKit term differs -- the page ContentView's origin, the scroller's frame, its
        //     adjustedContentInset, or its resting contentOffset?
        // Q2  the multi-select band (cpp paints it on Catalyst, MAUI and the port's XAML column do not):
        //     does MAUI actually have those cells SELECTED here, or is it not selecting at all?
        if (Environment.GetEnvironmentVariable("MAUI_GEOMETRY_DUMP") == "1")
        {
            page.Loaded += async (_, _) =>
            {
                await Task.Delay(600); // let layout + the selection restore settle, as the capture settle does
                void Line(string s) => Console.Error.WriteLine("[GEOMDUMP] " + s);
                Line($"page={key}");
                if (page.Handler?.PlatformView is UIKit.UIView pv)
                {
                    Line($"page.ContentView frame={pv.Frame} safeArea={pv.SafeAreaInsets}");
                }
                int i = 0;
                foreach (var el in Descendants(page))
                {
                    if (el.Handler?.PlatformView is UIKit.UIScrollView sv && el is not Microsoft.Maui.Controls.CollectionView)
                    {
                        Line($"scroll[{i}] {el.GetType().Name} frame={sv.Frame} contentSize={sv.ContentSize} " +
                             $"adjInset={sv.AdjustedContentInset} offset={sv.ContentOffset}");
                    }
                    if (el is Microsoft.Maui.Controls.CollectionView cv &&
                        cv.Handler?.PlatformView is UIKit.UIView cvRoot)
                    {
                        var ucv = cvRoot as UIKit.UICollectionView ?? FindCollectionView(cvRoot);
                        var sel = ucv?.GetIndexPathsForSelectedItems();
                        var selDesc = sel is null ? "n/a" : string.Join(",", sel.Select(x => $"{x.Section}:{x.Item}"));
                        Line($"cv[{i}] mode={cv.SelectionMode} selectedItems={cv.SelectedItems?.Count ?? -1} " +
                             $"nativeSelected=[{selDesc}] allowsMulti={ucv?.AllowsMultipleSelection}");
                    }
                    i++;
                }
                Line("end");
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
#if MACCATALYST
    // Helpers for the MAUI_GEOMETRY_DUMP block above. Temporary, and deliberately private/static so they
    // cannot be mistaken for app behaviour.
    // IVisualTreeElement is the PUBLIC walk (LogicalChildrenInternal is internal to Controls).
    static IEnumerable<Microsoft.Maui.Controls.Element> Descendants(Microsoft.Maui.Controls.Element root)
    {
        if (root is not Microsoft.Maui.IVisualTreeElement vte)
        {
            yield break;
        }
        foreach (var child in vte.GetVisualChildren())
        {
            if (child is Microsoft.Maui.Controls.Element el)
            {
                yield return el;
                foreach (var d in Descendants(el))
                {
                    yield return d;
                }
            }
        }
    }

    // A CollectionView's PlatformView is the controller's container, not always the UICollectionView itself.
    static UIKit.UICollectionView? FindCollectionView(UIKit.UIView v)
    {
        if (v is UIKit.UICollectionView c)
        {
            return c;
        }
        foreach (var sub in v.Subviews)
        {
            var hit = FindCollectionView(sub);
            if (hit is not null)
            {
                return hit;
            }
        }
        return null;
    }
#endif

}
