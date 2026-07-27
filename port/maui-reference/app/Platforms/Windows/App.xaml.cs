using Microsoft.UI.Xaml;

// The Windows head of MauiReference. Mirrors Platforms/{iOS,MacCatalyst,Android}: the platform entry
// point does nothing but hand control to the shared MauiProgram.CreateMauiApp(), so all three lanes
// render the same pages from the same code and the Windows column stays a faithful reference.
namespace MauiReference.WinUI;

/// <summary>Provides application-specific behavior to supplement the default Application class.</summary>
public partial class App : MauiWinUIApplication
{
	public App()
	{
		InitializeComponent();
	}

	protected override MauiApp CreateMauiApp() => MauiProgram.CreateMauiApp();
}
