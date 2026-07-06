using System.Globalization;

namespace MauiReference;

// Minimal IValueConverter used ONLY by pages/gap_onplatform_converter.xaml to give real MAUI a genuine,
// resolvable Converter={StaticResource ...} target for OnPlatform's Converter=/ConverterParameter=
// attributes (Microsoft.Maui.Controls.Xaml.OnPlatformExtension.Converter / ConverterParameter). The port's
// XAML loader rejects both attribute names outright (register_xaml_extensions.cpp / markup_extensions.cpp
// reject_unsupported_attributes), so this class exists purely to make the gap page well-formed, working
// XAML for the acceptance gate (MAUI must render it) — it performs no real conversion (passthrough).
public sealed class PassthroughDoubleConverter : IValueConverter
{
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture) => value;

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture) => value;
}
