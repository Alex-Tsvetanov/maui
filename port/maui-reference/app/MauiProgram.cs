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

#if DEBUG
		builder.Logging.AddDebug();
#endif

		return builder.Build();
	}
}
