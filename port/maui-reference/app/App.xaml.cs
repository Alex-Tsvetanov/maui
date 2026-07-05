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
        // Parity-comparison theme toggle: MAUI_THEME=Dark|Light forces the app appearance so captures can be
        // matched light-vs-light and dark-vs-dark against the C++ galleries (default: Light). Pairs with the
        // C++ galleries' MAUI_APPEARANCE toggle.
        // On Android `am start` does NOT pass process env vars, so the theme (like the page key) is carried
        // as a launch-intent extra which ResolveValue() reads; env var is the fallback for iOS/maccatalyst.
        UserAppTheme = ResolveValue("MAUI_THEME") == "Dark"
            ? Microsoft.Maui.ApplicationModel.AppTheme.Dark
            : Microsoft.Maui.ApplicationModel.AppTheme.Light;
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
        var key = ResolveValue("MAUI_COMPARE_PAGE") ?? "controls_stack";
        var page = PageDispatch.Create(key);
        var window = new Window(page);

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
