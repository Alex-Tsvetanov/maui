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
