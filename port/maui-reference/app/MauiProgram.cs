using Microsoft.Extensions.Logging;

namespace MauiReference;

public static class MauiProgram
{
	public static MauiApp CreateMauiApp()
	{
		var builder = MauiApp.CreateBuilder();
		builder
			.UseMauiApp<App>()
			// Register the EffectsFactory service so any page attaching a RoutingEffect resolves it to a
			// no-op instead of throwing "No service for type 'EffectsFactory'" on Android (inherited from
			// the retired maui-compare app's EffectsPage requirement).
			.ConfigureEffects(effects => { })
			.ConfigureFonts(fonts =>
			{
				fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
				fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
			});

#if MACCATALYST
		// Capture-determinism opt-in (MAUI_CAPTURE_TINT_NORMAL=1, set by port/tools/e2e/e2e.py): the
		// parity tool launches this app in the BACKGROUND (`open -g`, so the operator's focus is never
		// stolen), leaving the window inactive — and Catalyst then renders every default-tinted control
		// dimmed (system-blue button text renders gray) via the UITraitActiveAppearance trait (plus the
		// legacy tintAdjustmentMode path). Forcing BOTH to their active values keeps the CONTENT pixels
		// identical to the active-window render; the window CHROME (title text, traffic lights) still
		// draws inactive, which the parity review already exempts. Default (no env) keeps faithful
		// UIKit behavior. Mirrors the same env hook in the C++ port's ios/host_run.mm.
		if (Environment.GetEnvironmentVariable("MAUI_CAPTURE_TINT_NORMAL") is not null)
		{
			Microsoft.Maui.Handlers.WindowHandler.Mapper.AppendToMapping(
				"CaptureTintNormal",
				(handler, window) =>
				{
					if (handler.PlatformView is UIKit.UIWindow uiWindow)
					{
						uiWindow.TintAdjustmentMode = UIKit.UIViewTintAdjustmentMode.Normal;
						if (OperatingSystem.IsMacCatalystVersionAtLeast(17))
						{
							uiWindow.TraitOverrides.ActiveAppearance = UIKit.UIUserInterfaceActiveAppearance.Active;
						}
					}
				});
		}
#endif

#if DEBUG
		builder.Logging.AddDebug();
#endif

		return builder.Build();
	}
}
