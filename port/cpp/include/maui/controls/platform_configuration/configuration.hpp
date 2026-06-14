#pragma once
// maui::controls::platform_configuration  <=  Microsoft.Maui.Controls.PlatformConfiguration
//
// The platform-configuration MECHANISM (W2-24): the platform marker tags (IConfigPlatform + the six
// sealed marker classes from PlatformConfiguration/ExtensionPoints.cs) and the typed config accessor
// (Configuration<TPlatform, TElement> / IPlatformElementConfiguration<TPlatform, TElement> from
// Configuration.cs). `element::on<TPlatform>()` (element.hpp) mints a config over the element; the
// per-platform knob sets (the *Specific namespaces under this directory) are free functions over the
// element's platform-spec attached store, each with config-chaining overloads standing in for C#'s
// extension methods on IPlatformElementConfiguration.
//
// DEVIATION (documented): C#'s PlatformConfigurationRegistry<TElement> caches one Configuration object
// per platform type (Dictionary<Type, object>, created lazily by On<T>()). The port's config<> is a
// stateless two-word value accessor, so on<>() mints a fresh one per call — observably identical (the
// C# cache is reference-identity only; nothing keys off it) and nothing to own or tear down.

#include <type_traits>

namespace maui::controls::platform_configuration
{
    // C# Microsoft.Maui.Controls.IConfigPlatform — the marker interface every platform tag derives.
    // Open like C#: a vendor may define its own tag deriving this and write knobs over the same
    // element store (PlatformSpecificsTests' ImAVendor namespaces do exactly that).
    struct i_config_platform
    {
    };

    // The six sealed marker classes from PlatformConfiguration/ExtensionPoints.cs.
    struct android final : i_config_platform // <=  ...PlatformConfiguration.Android
    {
    };
    struct ios final : i_config_platform // <=  ...PlatformConfiguration.iOS
    {
    };
    struct windows final : i_config_platform // <=  ...PlatformConfiguration.Windows
    {
    };
    struct tizen final : i_config_platform // <=  ...PlatformConfiguration.Tizen
    {
    };
    struct macos final : i_config_platform // <=  ...PlatformConfiguration.macOS
    {
    };
    struct gtk final : i_config_platform // <=  ...PlatformConfiguration.GTK
    {
    };

    // C# Configuration<TPlatform, TElement> : IPlatformElementConfiguration<TPlatform, TElement> — the
    // typed accessor a knob set's config-chaining overloads flow through. Holds the element NON-owning
    // (C# holds the reference; the accessor never outlives the statement chain it is used in).
    // TElement is the static type at the on<>() call site, so the knob overloads constrain on it the
    // way C#'s extension methods constrain on the FormsElement type parameter (covariance over derived
    // elements falls out of the std::derived_from constraints the knob headers use).
    template <class TPlatform, class TElement> class config
    {
        static_assert(std::is_base_of_v<i_config_platform, TPlatform>,
                      "TPlatform must be a platform marker tag (i_config_platform)");

    public:
        explicit config(TElement& element) : element_(&element)
        {
        }

        // C# IConfigElement<TElement>.Element.
        [[nodiscard]] TElement& element() const
        {
            return *element_;
        }

    private:
        TElement* element_;
    };
} // namespace maui::controls::platform_configuration
