// WinUI 3 feasibility probe: can a CODE-ONLY C++/WinRT WinUI 3 desktop app be built with CMake + Ninja
// (no MSBuild, no .xaml, no XAML compiler) and show REAL Microsoft.UI.Xaml controls?
//
// This is the gate for the port's Windows backend. MAUI's Windows backend is WinUI 3
// (Microsoft.UI.Xaml.Controls.Button / TextBlock -- verified in src/Core/src/Handlers/*), so only a
// WinUI 3 render can be compared against the MAUI reference board. The port builds its UI in C++ code,
// never from XAML markup, which means the XAML *compiler* is not needed -- and that removes the single
// biggest reason WinUI 3 C++ apps normally require MSBuild. What is still required, and is the actual
// subtlety this probe exists to prove out:
//
//   1. THE BOOTSTRAPPER. An UNPACKAGED app must call the Windows App Runtime bootstrapper before any
//      WinUI type is touched, otherwise activation fails with a class-not-registered error that looks
//      nothing like "you forgot to bootstrap".
//   2. IXamlMetadataProvider. Normally generated from App.xaml by the XAML compiler. A code-only app has
//      to supply it, delegating to the WinUI controls' own metadata provider, or every control
//      activation throws.
//   3. XamlControlsResources. Without it the controls exist but render unstyled/invisible -- the exact
//      "plausible but wrong" failure this project keeps guarding against.
//
// If this renders a real WinUI Button + TextBlock, the backend is a matter of writing handlers. If it
// does not, the fallback is a thin MSBuild .vcxproj shell linking the port's CMake-built core.

#include <windows.h>

#include <MddBootstrap.h>

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>

#include <cstdio>
#include <string>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace
{
    // The code-only application object. ApplicationT gives us the WinUI Application base; the extra
    // IXamlMetadataProvider is what the XAML compiler would normally have generated for us.
    struct App : ApplicationT<App, Markup::IXamlMetadataProvider>
    {
        App()
        {
            // Merge the WinUI control styles. Skip this and Button/TextBlock still ACTIVATE but draw
            // nothing recognisable -- a window that looks broken rather than an error that says why.
            Resources().MergedDictionaries().Append(Controls::XamlControlsResources{});
        }

        void OnLaunched(LaunchActivatedEventArgs const&)
        {
            window_ = Window{};
            window_.Title(L"maui winui probe");

            Controls::StackPanel root;
            root.Padding(Thickness{24, 24, 24, 24});
            root.Spacing(12);

            Controls::TextBlock title;
            title.Text(L"WinUI 3 from CMake + Ninja");
            title.FontSize(28);
            root.Children().Append(title);

            Controls::TextBlock note;
            note.Text(L"code-only C++/WinRT, no XAML compiler");
            note.FontSize(14);
            root.Children().Append(note);

            // A real WinUI Button -- the same type MAUI's ButtonHandler.Windows.cs creates. Its native
            // chrome is what the parity board must match, which is the whole point of using WinUI here.
            Controls::Button button;
            button.Content(box_value(L"Button"));
            root.Children().Append(button);

            Controls::Button disabled;
            disabled.Content(box_value(L"Button (disabled)"));
            disabled.IsEnabled(false);
            root.Children().Append(disabled);

            Controls::CheckBox check;
            check.Content(box_value(L"CheckBox"));
            check.IsChecked(true);
            root.Children().Append(check);

            Controls::Slider slider;
            slider.Minimum(0);
            slider.Maximum(100);
            slider.Value(40);
            root.Children().Append(slider);

            window_.Content(root);
            window_.Activate();
        }

        // ---- IXamlMetadataProvider: delegate to the WinUI controls' provider ----------------------
        // Every XAML type the framework resolves at runtime comes through here. Returning empty (the
        // tempting stub) makes control activation fail later with an opaque error, so it forwards to the
        // provider that ships with the WinUI controls.
        Markup::IXamlType GetXamlType(Windows::UI::Xaml::Interop::TypeName const& type)
        {
            return provider_.GetXamlType(type);
        }
        Markup::IXamlType GetXamlType(hstring const& fullName)
        {
            return provider_.GetXamlType(fullName);
        }
        com_array<Markup::XmlnsDefinition> GetXmlnsDefinitions()
        {
            return provider_.GetXmlnsDefinitions();
        }

    private:
        Window window_{nullptr};
        XamlTypeInfo::XamlControlsXamlMetaDataProvider provider_;
    };
} // namespace

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    // (1) Bootstrap the Windows App Runtime FIRST. Unpackaged apps have no package graph, so without
    // this every WinUI activation fails with REGDB_E_CLASSNOTREG. The major version must match the
    // Windows App SDK the app was built against.
    constexpr UINT32 k_major_minor = 0x00010007; // 1.7
    if (FAILED(MddBootstrapInitialize2(k_major_minor, L"", PACKAGE_VERSION{},
                                       MddBootstrapInitializeOptions_OnNoMatch_ShowUI)))
    {
        MessageBoxW(nullptr, L"MddBootstrapInitialize failed: the Windows App Runtime is not installed.",
                    L"maui winui probe", MB_ICONERROR);
        return 1;
    }

    init_apartment(apartment_type::single_threaded);
    Application::Start([](auto&&) { make<App>(); });
    MddBootstrapShutdown();
    return 0;
}
