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

// C++/WinRT include rule, worth stating because every handler file will hit it: you must include the
// FULL header for every namespace whose members you call, not merely the one declaring the type. The
// impl/*.0.h headers that come in transitively only forward-declare, so calling e.g. Slider::Value()
// (IRangeBase, in ...Controls.Primitives) or IVector::Append (Windows.Foundation.Collections) without
// their headers fails with "error C3779: a function that returns 'auto' cannot be used before it is
// defined" -- which does not obviously mean "add an include".
#include <windows.h>

#include <MddBootstrap.h>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <cstdint>
#include <cstdio>
#include <format>
#include <string>
#include <string_view>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace
{
    // Step logging to a file. A WIN32-subsystem app has no console, and a stowed exception
    // (0xC000027B) in combase leaves only "APPCRASH" in the event log -- no HRESULT, no message. Logging
    // each step is the only way to see WHERE activation dies rather than guessing between the
    // bootstrapper, the metadata provider and the resources.
    void probe_log(std::string_view msg)
    {
        if (FILE* f = nullptr; fopen_s(&f, "C:\\maui-winui\\probe.log", "a") == 0 && f != nullptr)
        {
            std::fwrite(msg.data(), 1, msg.size(), f);
            std::fputc('\n', f);
            std::fclose(f);
        }
    }

    std::string hr_text(winrt::hresult_error const& e)
    {
        const std::wstring w{e.message()};
        std::string out(w.size(), '?');
        for (size_t i = 0; i < w.size(); ++i)
        {
            out[i] = w[i] < 128 ? static_cast<char>(w[i]) : '?';
        }
        return std::format("hr=0x{:08X} {}", static_cast<uint32_t>(e.code()), out);
    }
} // namespace

namespace
{
    // The code-only application object. ApplicationT gives us the WinUI Application base; the extra
    // IXamlMetadataProvider is what the XAML compiler would normally have generated for us.
    struct App : ApplicationT<App, Markup::IXamlMetadataProvider>
    {
        App()
        {
            // NOTHING but logging here. The resource merge used to live in this constructor and crashed
            // with a stowed exception in combase before the next log line -- the App object is not yet
            // fully constructed as far as the XAML framework is concerned, so activating a XAML type
            // (XamlControlsResources) from here is too early. It is done in OnLaunched instead.
            probe_log("App ctor: entered (no XAML work here on purpose)");
        }

        void OnLaunched(LaunchActivatedEventArgs const&)
        {
            probe_log("OnLaunched: entered");
            // Merge the WinUI control styles. Without them Button/TextBlock still ACTIVATE but render
            // unstyled -- a window that looks broken rather than an error saying why.
            probe_log("OnLaunched: activating XamlControlsResources");
            Controls::XamlControlsResources res;
            probe_log("OnLaunched: XamlControlsResources activated");
            Resources().MergedDictionaries().Append(res);
            probe_log("OnLaunched: resources merged");
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

            probe_log("OnLaunched: tree built");
            window_.Content(root);
            window_.Activate();
            probe_log("OnLaunched: window activated");
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

    probe_log("main: bootstrap ok");
    try
    {
        init_apartment(apartment_type::single_threaded);
        probe_log("main: apartment initialised");
        Application::Start([](auto&&) { make<App>(); });
        probe_log("main: Application::Start returned");
    }
    catch (winrt::hresult_error const& e)
    {
        probe_log("main: hresult_error " + hr_text(e));
        MddBootstrapShutdown();
        return 2;
    }
    catch (std::exception const& e)
    {
        probe_log(std::string{"main: std::exception "} + e.what());
        MddBootstrapShutdown();
        return 3;
    }
    MddBootstrapShutdown();
    return 0;
}
