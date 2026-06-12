// maui::xaml — the v1 markup extensions (markup_extensions.hpp). Each extension class below ports
// the C# class of the same name from src/Controls/src/Xaml/MarkupExtensions/ (FQN comments inline);
// the factories replace MarkupExtensionParser.Parse's reflective Activator.CreateInstance +
// property-setter walk with explicit attribute parsing (PROFILE §6).
#include "maui/xaml/markup_extensions.hpp"

#include <algorithm>
#include <any>
#include <array>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/application.hpp"
#include "maui/controls/bindings/binding.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/dynamic_resource.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_node.hpp" // maui_uri / x2009_uri (the {x:Type} miss message)
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_runtime_environment.hpp"
#include "maui/xaml/xaml_static_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // ---- attribute-map helpers (the factory half of MarkupExtensionParser.SetPropertyValue) ----

        [[nodiscard]] const std::any* find_value(const markup_extension_arguments& args, const std::string& name)
        {
            const auto it = args.values.find(name);
            return it == args.values.end() ? nullptr : &it->second;
        }

        [[nodiscard]] const std::string* find_attribute(const markup_extension_arguments& args, const std::string& name)
        {
            const auto it = args.attributes.find(name);
            return it == args.attributes.end() ? nullptr : &it->second;
        }

        // The argument for `name` in its source form: the pre-resolved boxed value when present
        // (values wins over the raw string — see i_markup_extension.hpp), else the attribute string
        // boxed as std::string; nullopt when the name appears in neither map.
        [[nodiscard]] std::optional<std::any> any_argument(const markup_extension_arguments& args,
                                                           const std::string& name)
        {
            if (const std::any* value = find_value(args, name))
            {
                return *value;
            }
            if (const std::string* text = find_attribute(args, name))
            {
                return std::any{*text};
            }
            return std::nullopt;
        }

        // A STRING-typed extension property (Key / Member / TypeName / Path / Mode / Converter): a
        // pre-resolved values entry must itself hold std::string — C#'s reflective assignment of a
        // non-string to a string CLR property surfaces as a XamlParseException through the visitor.
        [[nodiscard]] std::optional<std::string> string_argument(const markup_extension_arguments& args,
                                                                 const std::string& name, std::string_view extension)
        {
            if (const std::any* value = find_value(args, name))
            {
                if (const auto* text = std::any_cast<std::string>(value))
                {
                    return *text;
                }
                throw xaml_parse_exception(std::format("Property '{}' of {} requires a string value", name, extension));
            }
            if (const std::string* text = find_attribute(args, name))
            {
                return *text;
            }
            return std::nullopt;
        }

        // The [ContentProperty] route: the named spelling wins, the positional "" piece (the value of
        // "{Name value}") falls in behind it (MarkupExtensionParser.SetPropertyValue prop==null →
        // GetContentPropertyName).
        [[nodiscard]] std::optional<std::string> string_content_argument(const markup_extension_arguments& args,
                                                                         const std::string& name,
                                                                         std::string_view extension)
        {
            if (auto named = string_argument(args, name, extension))
            {
                return named;
            }
            return string_argument(args, "", extension);
        }

        [[nodiscard]] bool contains_name(std::span<const std::string_view> names, std::string_view name)
        {
            return std::ranges::any_of(names, [name](std::string_view known) { return known == name; });
        }

        // An attribute the extension has no property for throws (C#'s reflective GetRuntimeProperty
        // miss, surfaced as a XamlParseException by the visitor). `known` includes "" when the
        // extension has a [ContentProperty].
        void require_known_attributes(const markup_extension_arguments& args, std::string_view extension,
                                      std::span<const std::string_view> known)
        {
            const auto check = [&](const std::string& name) {
                if (!contains_name(known, name))
                {
                    throw xaml_parse_exception(
                        std::format("Markup extension {} has no property '{}'", extension, name));
                }
            };
            for (const auto& [name, text] : args.attributes)
            {
                check(name);
            }
            for (const auto& [name, value] : args.values)
            {
                check(name);
            }
        }

        // A property C# supports but the port defers (Binding.Source/StringFormat/…, the value
        // converters): fail LOUDLY instead of silently dropping markup the C# loader would honor.
        void reject_unsupported_attributes(const markup_extension_arguments& args, std::string_view extension,
                                           std::span<const std::string_view> unsupported)
        {
            const auto check = [&](const std::string& name) {
                if (contains_name(unsupported, name))
                {
                    throw xaml_parse_exception(
                        std::format("Property '{}' of {} is not supported by the port yet (STATUS.md M7 deferrals)",
                                    name, extension));
                }
            };
            for (const auto& [name, text] : args.attributes)
            {
                check(name);
            }
            for (const auto& [name, value] : args.values)
            {
                check(name);
            }
        }

        // ---- {StaticResource}  <=  Microsoft.Maui.Controls.Xaml.StaticResourceExtension ----
        // ProvideValue: walk the parent objects' resource dictionaries (the port's
        // element::try_get_resource self→ancestor chain), then the application-level fallback
        // (TryGetApplicationLevelResource — Application.Current has no port singleton, so the loader
        // passes the app through the service provider). The resolved value is returned AS STORED;
        // C#'s CastTo conversion toward the target property's type (TypeConversionHelper.TryConvert)
        // is a documented deviation — the property registry's typed unboxing rejects mismatches.
        class static_resource_extension final : public i_markup_extension
        {
        public:
            explicit static_resource_extension(std::optional<std::string> key) : key_(std::move(key))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& services) override
            {
                if (!key_.has_value())
                {
                    throw xaml_parse_exception("you must specify a key in {StaticResource}");
                }
                // The load-time chain: the node ancestors' own dictionaries, nearest first (C#'s
                // ParentObjects walk in TryGetResource — each parent contributes only its own
                // Resources, merged dictionaries included via resource_dictionary::try_get).
                for (const maui::controls::resource_dictionary* dictionary : services.parent_resources)
                {
                    if (dictionary == nullptr)
                    {
                        continue;
                    }
                    if (const std::any* value = dictionary->try_get(*key_))
                    {
                        return *value;
                    }
                }
                // The live chain (post-load callers / tests wiring real parents).
                if (services.resource_scope != nullptr)
                {
                    if (const std::any* value = services.resource_scope->try_get_resource(*key_))
                    {
                        return *value;
                    }
                }
                // StaticResourceExtension.TryGetApplicationLevelResource — try_get_resource never
                // creates the dictionary, matching C#'s IsResourcesCreated pre-check.
                if (services.application != nullptr)
                {
                    if (const std::any* value = services.application->try_get_resource(*key_))
                    {
                        return *value;
                    }
                }
                throw xaml_parse_exception(std::format("StaticResource not found for key {}", *key_));
            }

        private:
            std::optional<std::string> key_;
        };

        // ---- {DynamicResource}  <=  Microsoft.Maui.Controls.Xaml.DynamicResourceExtension ----
        // ProvideValue returns the DynamicResource OBJECT (the key reference), never the value — the
        // applier turns it into element::set_dynamic_resource (markup_extensions.hpp contract).
        class dynamic_resource_extension final : public i_markup_extension
        {
        public:
            explicit dynamic_resource_extension(std::optional<std::string> key) : key_(std::move(key))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                if (!key_.has_value())
                {
                    throw xaml_parse_exception("DynamicResource markup require a Key"); // sic — the C# message
                }
                return std::any{maui::controls::dynamic_resource{*key_}};
            }

        private:
            std::optional<std::string> key_;
        };

        // ---- {x:Static}  <=  Microsoft.Maui.Controls.Xaml.StaticExtension ----
        // C# resolves "[prefix:]typeName.staticMemberName" through IXamlTypeResolver and reflects the
        // static property/field/const/enum member; the port looks the whole member name up in the
        // explicit xaml_static_registry (an unknown xmlns prefix is retried without the prefix, since
        // registrations are keyed by the bare "Type.Member").
        class static_extension final : public i_markup_extension
        {
        public:
            explicit static_extension(std::string member) : member_(std::move(member))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                if (member_.empty() || !member_.contains('.'))
                {
                    throw xaml_parse_exception("Syntax for x:Static is [Member=][prefix:]typeName.staticMemberName");
                }
                const xaml_static_registry& statics = xaml_static_registry::instance();
                if (const auto* producer = statics.find(member_))
                {
                    return (*producer)();
                }
                if (const auto colon = member_.find(':'); colon != std::string::npos)
                {
                    if (const auto* producer = statics.find(std::string_view{member_}.substr(colon + 1)))
                    {
                        return (*producer)();
                    }
                }
                throw xaml_parse_exception(std::format("No static member found for {}", member_));
            }

        private:
            std::string member_;
        };

        // ---- {x:Type}  <=  Microsoft.Maui.Controls.Xaml.TypeExtension ----
        // C# returns the System.Type via IXamlTypeResolver.Resolve; the port resolves the markup name
        // in the xaml_type_registry (the services-provided one, else the process default) and returns
        // the registration's type_tag — the port's Type identity (keys the handler/property registries).
        class type_extension final : public i_markup_extension
        {
        public:
            explicit type_extension(std::string type_name) : type_name_(std::move(type_name))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& services) override
            {
                if (type_name_.empty())
                {
                    throw xaml_parse_exception("TypeName isn't set.");
                }
                std::string_view name = type_name_;
                xaml_namespace ns = xaml_namespace::maui;
                if (const auto colon = name.find(':'); colon != std::string_view::npos)
                {
                    const std::string_view prefix = name.substr(0, colon);
                    name = name.substr(colon + 1);
                    if (prefix == "x")
                    {
                        ns = xaml_namespace::x;
                    }
                    else
                    {
                        // The v1 registry models the two built-in namespaces only (xaml_type_registry
                        // .hpp); any other prefix is undeclared — TypeArgumentsParser's message.
                        throw xaml_parse_exception(std::format("No xmlns declaration for prefix '{}'.", prefix));
                    }
                }
                const xaml_type_registry& types =
                    services.type_registry != nullptr ? *services.type_registry : default_xaml_type_registry();
                if (const auto* registration = types.find(name, ns))
                {
                    return std::any{registration->type};
                }
                // XamlParser.GetElementType's message shape.
                throw xaml_parse_exception(
                    std::format("Type {} not found in xmlns {}", name, ns == xaml_namespace::x ? x2009_uri : maui_uri));
            }

        private:
            std::string type_name_;
        };

        // ---- {x:Null}  <=  Microsoft.Maui.Controls.Xaml.NullExtension ----
        class null_extension final : public i_markup_extension
        {
        public:
            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                return std::any{xaml_null{}}; // C# returns null; the marker is the port's typed-null
            }
        };

        // One attribute-name → runtime-platform/idiom probe row.
        struct platform_row
        {
            std::string_view attribute;
            device_platform platform;
        };
        struct idiom_row
        {
            std::string_view attribute;
            device_idiom idiom;
        };

        // OnPlatformExtension.TryGetValueForPlatform's probe order. DevicePlatform.UWP is the SAME
        // platform value as WinUI in C# (UWP = new DevicePlatform(nameof(WinUI))) and is probed after
        // it; the legacy DevicePlatform.Create("UWP") string has no closed-enum analog (documented in
        // xaml_runtime_environment.hpp).
        constexpr std::array<platform_row, 9> k_platform_rows{{
            {.attribute = "Android", .platform = device_platform::android},
            {.attribute = "GTK", .platform = device_platform::gtk},
            {.attribute = "iOS", .platform = device_platform::ios},
            {.attribute = "macOS", .platform = device_platform::mac_os},
            {.attribute = "MacCatalyst", .platform = device_platform::mac_catalyst},
            {.attribute = "Tizen", .platform = device_platform::tizen},
            {.attribute = "WinUI", .platform = device_platform::win_ui},
            {.attribute = "UWP", .platform = device_platform::win_ui},
            {.attribute = "WPF", .platform = device_platform::wpf},
        }};

        // OnIdiomExtension.GetValue's idiom set.
        constexpr std::array<idiom_row, 5> k_idiom_rows{{
            {.attribute = "Phone", .idiom = device_idiom::phone},
            {.attribute = "Tablet", .idiom = device_idiom::tablet},
            {.attribute = "Desktop", .idiom = device_idiom::desktop},
            {.attribute = "TV", .idiom = device_idiom::tv},
            {.attribute = "Watch", .idiom = device_idiom::watch},
        }};

        using slot_map = std::map<std::string, std::any, std::less<>>;

        // Collect the per-platform/idiom slots in their source form (markup_extensions.hpp value-form
        // contract); the positional "" piece maps to Default ([ContentProperty(nameof(Default))]).
        [[nodiscard]] slot_map collect_slots(const markup_extension_arguments& args,
                                             std::span<const std::string_view> names)
        {
            slot_map slots;
            for (const std::string_view name : names)
            {
                std::string key{name};
                if (auto value = any_argument(args, key))
                {
                    slots.insert_or_assign(std::move(key), std::move(*value));
                }
            }
            if (!slots.contains("Default"))
            {
                if (auto value = any_argument(args, std::string{}))
                {
                    slots.insert_or_assign("Default", std::move(*value));
                }
            }
            return slots;
        }

        // ---- {OnPlatform}  <=  Microsoft.Maui.Controls.Xaml.OnPlatformExtension ----
        // The port has no target-property type metadata at ProvideValue time, so the chosen value is
        // returned in its source form for the applier's late conversion, and C#'s "no value for this
        // platform → BindableProperty.GetDefaultValue" turns into the empty-any skip (both documented
        // in markup_extensions.hpp). C#'s InvalidOperationException for an undeterminable property
        // type has no port analog for the same reason.
        class on_platform_extension final : public i_markup_extension
        {
        public:
            explicit on_platform_extension(slot_map slots) : slots_(std::move(slots))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                if (slots_.empty())
                {
                    throw xaml_parse_exception(
                        "OnPlatformExtension requires a value to be specified for at least one platform or Default.");
                }
                const device_platform current = xaml_runtime_environment::current().platform;
                for (const platform_row& row : k_platform_rows)
                {
                    if (row.platform != current)
                    {
                        continue;
                    }
                    if (const auto it = slots_.find(row.attribute); it != slots_.end())
                    {
                        return it->second;
                    }
                }
                if (const auto it = slots_.find("Default"); it != slots_.end())
                {
                    return it->second;
                }
                return {}; // no value for this platform — the applier skips the assignment
            }

        private:
            slot_map slots_;
        };

        // ---- {OnIdiom}  <=  Microsoft.Maui.Controls.Xaml.OnIdiomExtension ----
        class on_idiom_extension final : public i_markup_extension
        {
        public:
            explicit on_idiom_extension(slot_map slots) : slots_(std::move(slots))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                if (slots_.empty())
                {
                    throw xaml_parse_exception(
                        "OnIdiomExtension requires a non-null value to be specified for at least one idiom or "
                        "Default.");
                }
                const device_idiom current = xaml_runtime_environment::current().idiom;
                for (const idiom_row& row : k_idiom_rows)
                {
                    if (row.idiom != current)
                    {
                        continue;
                    }
                    if (const auto it = slots_.find(row.attribute); it != slots_.end())
                    {
                        return it->second;
                    }
                    break; // C#: `Phone ?? Default` — a matched idiom with no value falls to Default
                }
                if (const auto it = slots_.find("Default"); it != slots_.end())
                {
                    return it->second;
                }
                return {};
            }

        private:
            slot_map slots_;
        };

        // ---- {AppThemeBinding}  <=  Microsoft.Maui.Controls.Xaml.AppThemeBindingExtension ----
        // C# builds and returns an AppThemeBinding (a BindingBase); applying it — and re-applying on
        // RequestedThemeChanged — is the binding's Apply/ApplyCore, which is the U3 applier's half of
        // the app_theme_binding marker contract (markup_extensions.hpp).
        class app_theme_binding_extension final : public i_markup_extension
        {
        public:
            explicit app_theme_binding_extension(app_theme_binding binding) : binding_(std::move(binding))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                if (!binding_.has_light && !binding_.has_dark && !binding_.default_value.has_value())
                {
                    throw xaml_parse_exception("AppThemeBindingExtension requires a non-null value to be specified "
                                               "for at least one theme or Default.");
                }
                return std::any{binding_};
            }

        private:
            app_theme_binding binding_;
        };

        // ---- {Binding}  <=  Microsoft.Maui.Controls.Xaml.BindingExtension ----
        // C# builds `new Binding(Path, Mode, Converter, ConverterParameter, StringFormat, Source)`;
        // the factory below builds the same maui::controls::binding (the W1-02 string-path engine)
        // and the binding_request carries it to the applier (markup_extensions.hpp documents the
        // contract; register_runtime_bindings installs the element::set_binding route).
        class binding_extension final : public i_markup_extension
        {
        public:
            explicit binding_extension(binding_request request) : request_(std::move(request))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                return std::any{request_};
            }

        private:
            binding_request request_;
        };

        // BindingMode parsing — TypeConversionExtensions.ConvertTo's Enum.Parse(typeof(BindingMode),
        // value, ignoreCase: false), via the converter unit's established enum helper (trimmed,
        // case-sensitive names; the numeric form follows the port's enumerator values — the
        // documented deviation in xaml_converters.hpp).
        [[nodiscard]] maui::core::binding_mode parse_binding_mode(const std::string& text)
        {
            using maui::core::binding_mode;
            static constexpr std::array<enum_entry<binding_mode>, 5> k_names{{
                {.name = "Default", .value = binding_mode::default_mode},
                {.name = "TwoWay", .value = binding_mode::two_way},
                {.name = "OneWay", .value = binding_mode::one_way},
                {.name = "OneWayToSource", .value = binding_mode::one_way_to_source},
                {.name = "OneTime", .value = binding_mode::one_time},
            }};
            if (const auto parsed = try_parse_enum<binding_mode>(text, k_names))
            {
                return *parsed;
            }
            // TypeConversionExtensions' message shape for a failed conversion.
            throw xaml_parse_exception(std::format("Cannot convert \"{}\" into BindingMode", text));
        }

        // "alice_blue" → "AliceBlue": recover the C# PascalCase member name from the X-macro's
        // snake_case token (the names are plain ASCII [a-z_]).
        [[nodiscard]] std::string pascalize(std::string_view snake)
        {
            std::string result;
            result.reserve(snake.size());
            bool upper_next = true;
            for (const char c : snake)
            {
                if (c == '_')
                {
                    upper_next = true;
                    continue;
                }
                result.push_back(upper_next && c >= 'a' && c <= 'z' ? static_cast<char>(c - 'a' + 'A') : c);
                upper_next = false;
            }
            return result;
        }
    } // namespace

    const std::any& app_theme_binding::pick(maui::core::app_theme theme) const
    {
        // AppThemeBinding.GetValue: Dark → Dark-if-set else Default; everything else (Light AND
        // Unspecified) → Light-if-set else Default.
        if (theme == maui::core::app_theme::dark)
        {
            return has_dark ? dark : default_value;
        }
        return has_light ? light : default_value;
    }

    void register_standard_markup_extensions()
    {
        markup_extension_registry& extensions = markup_extension_registry::instance();

        extensions.register_extension("StaticResource", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_known{"", "Key"};
            require_known_attributes(args, "StaticResource", k_known);
            return std::make_unique<static_resource_extension>(string_content_argument(args, "Key", "StaticResource"));
        });

        extensions.register_extension("DynamicResource", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_known{"", "Key"};
            require_known_attributes(args, "DynamicResource", k_known);
            return std::make_unique<dynamic_resource_extension>(
                string_content_argument(args, "Key", "DynamicResource"));
        });

        extensions.register_extension("x:Static", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_known{"", "Member"};
            require_known_attributes(args, "x:Static", k_known);
            return std::make_unique<static_extension>(
                string_content_argument(args, "Member", "x:Static").value_or(std::string{}));
        });

        extensions.register_extension("x:Type", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_known{"", "TypeName"};
            require_known_attributes(args, "x:Type", k_known);
            return std::make_unique<type_extension>(
                string_content_argument(args, "TypeName", "x:Type").value_or(std::string{}));
        });

        extensions.register_extension("x:Null", [](const markup_extension_arguments& args) {
            require_known_attributes(args, "x:Null", {}); // NullExtension has no properties
            return std::make_unique<null_extension>();
        });

        extensions.register_extension("OnPlatform", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_unsupported{"Converter", "ConverterParameter"};
            static constexpr std::array<std::string_view, 13> k_known{
                "",      "Default",   "Android",           "GTK", "iOS", "macOS", "MacCatalyst", "Tizen", "UWP", "WPF",
                "WinUI", "Converter", "ConverterParameter"};
            static constexpr std::array<std::string_view, 10> k_slots{"Default",     "Android", "GTK", "iOS", "macOS",
                                                                      "MacCatalyst", "Tizen",   "UWP", "WPF", "WinUI"};
            reject_unsupported_attributes(args, "OnPlatform", k_unsupported);
            require_known_attributes(args, "OnPlatform", k_known);
            return std::make_unique<on_platform_extension>(collect_slots(args, k_slots));
        });

        extensions.register_extension("OnIdiom", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_unsupported{"Converter", "ConverterParameter"};
            static constexpr std::array<std::string_view, 9> k_known{
                "", "Default", "Phone", "Tablet", "Desktop", "TV", "Watch", "Converter", "ConverterParameter"};
            static constexpr std::array<std::string_view, 6> k_slots{"Default", "Phone", "Tablet",
                                                                     "Desktop", "TV",    "Watch"};
            reject_unsupported_attributes(args, "OnIdiom", k_unsupported);
            require_known_attributes(args, "OnIdiom", k_known);
            return std::make_unique<on_idiom_extension>(collect_slots(args, k_slots));
        });

        extensions.register_extension("AppThemeBinding", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 4> k_known{"", "Light", "Dark", "Default"};
            require_known_attributes(args, "AppThemeBinding", k_known);
            app_theme_binding binding;
            if (auto value = any_argument(args, "Light"))
            {
                binding.light = std::move(*value);
                binding.has_light = true; // _isLightSet
            }
            if (auto value = any_argument(args, "Dark"))
            {
                binding.dark = std::move(*value);
                binding.has_dark = true; // _isDarkSet
            }
            if (auto value = any_argument(args, "Default"))
            {
                binding.default_value = std::move(*value);
            }
            else if (auto content = any_argument(args, std::string{}))
            {
                binding.default_value = std::move(*content); // [ContentProperty(nameof(Default))]
            }
            return std::make_unique<app_theme_binding_extension>(std::move(binding));
        });

        extensions.register_extension("Binding", [](const markup_extension_arguments& args) {
            // UpdateSourceEventName needs the (unported) reflective event wiring — still a loud
            // deferral; everything else BindingExtension.ProvideValue forwards into `new Binding(…)`
            // is honored below.
            static constexpr std::array<std::string_view, 1> k_unsupported{"UpdateSourceEventName"};
            static constexpr std::array<std::string_view, 10> k_known{"",
                                                                      "Path",
                                                                      "Mode",
                                                                      "Converter",
                                                                      "Source",
                                                                      "StringFormat",
                                                                      "ConverterParameter",
                                                                      "UpdateSourceEventName",
                                                                      "TargetNullValue",
                                                                      "FallbackValue"};
            reject_unsupported_attributes(args, "Binding", k_unsupported);
            require_known_attributes(args, "Binding", k_known);
            binding_request request;
            if (auto path = string_content_argument(args, "Path", "Binding"))
            {
                request.path = std::move(*path); // [ContentProperty(nameof(Path))]; default SelfPath "."
            }
            if (const auto mode = string_argument(args, "Mode", "Binding"))
            {
                request.mode = parse_binding_mode(*mode);
            }
            // BindingExtension.ProvideValue: new Binding(Path, Mode, Converter, ConverterParameter,
            // StringFormat, Source). The Binding ctor's whitespace-path ArgumentException surfaces
            // through the XAML error channel like every other malformed attribute.
            std::shared_ptr<maui::controls::binding> built;
            try
            {
                built = std::make_shared<maui::controls::binding>(request.path, request.mode);
            }
            catch (const std::invalid_argument& error)
            {
                throw xaml_parse_exception(error.what());
            }
            if (auto format = string_argument(args, "StringFormat", "Binding"))
            {
                built->set_string_format(std::move(*format));
            }
            // Converter: an IValueConverter INSTANCE — only a nested extension can provide one
            // ({StaticResource …} hands back the boxed resource; a XAML-hydrated resource arrives as
            // the create pass's shared_ptr<bindable_object> box and is downcast). A raw string has
            // no instance lookup (C#'s reflective string→IValueConverter assignment fails too).
            if (const std::any* value = find_value(args, "Converter"))
            {
                std::shared_ptr<maui::controls::i_value_converter> converter;
                if (const auto* direct = std::any_cast<std::shared_ptr<maui::controls::i_value_converter>>(value))
                {
                    converter = *direct;
                }
                else if (const auto* object = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(value))
                {
                    converter = std::dynamic_pointer_cast<maui::controls::i_value_converter>(*object);
                }
                if (converter == nullptr)
                {
                    throw xaml_parse_exception(
                        "Property 'Converter' of Binding requires an i_value_converter instance");
                }
                built->set_converter(std::move(converter));
            }
            else if (find_attribute(args, "Converter") != nullptr)
            {
                throw xaml_parse_exception("Property 'Converter' of Binding requires a markup extension "
                                           "providing an i_value_converter (e.g. {StaticResource …})");
            }
            if (auto parameter = any_argument(args, "ConverterParameter"))
            {
                built->set_converter_parameter(std::move(*parameter)); // object — the source form passes
            }
            if (auto target_null = any_argument(args, "TargetNullValue"))
            {
                built->set_target_null_value(std::move(*target_null));
            }
            if (auto fallback = any_argument(args, "FallbackValue"))
            {
                built->set_fallback_value(std::move(*fallback));
            }
            // Source: an explicit source OBJECT — supported when a nested extension provides a
            // walkable bindable_object (the create-pass / code-seeded resource box). Other boxed
            // shapes and raw strings are loud deferrals (C# accepts any object as a value-only
            // source; the port's set_source needs the walkable form).
            if (const std::any* value = find_value(args, "Source"))
            {
                const auto* object = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(value);
                if (object == nullptr)
                {
                    throw xaml_parse_exception("Property 'Source' of Binding requires a bindable_object "
                                               "provided by a markup extension (STATUS.md M7 deferrals)");
                }
                built->set_source(*object);
            }
            else if (find_attribute(args, "Source") != nullptr)
            {
                throw xaml_parse_exception("Property 'Source' of Binding requires a markup extension "
                                           "providing a bindable_object (e.g. {StaticResource …})");
            }
            request.instance = std::move(built);
            return std::make_unique<binding_extension>(std::move(request));
        });

        // ---- the standard {x:Static} members: the 147 named colors as "Colors.<Name>" ----
        // (Microsoft.Maui.Graphics.Colors — the C# member names recovered from the single-source
        // X-macro's snake tokens.)
        xaml_static_registry& statics = xaml_static_registry::instance();
#define MAUI_XAML_REGISTER_STATIC_COLOR(name, str, argb)                                                               \
    statics.register_member("Colors." + pascalize(#name), [] { return std::any{maui::graphics::colors::name}; });
        MAUI_GRAPHICS_NAMED_COLORS(MAUI_XAML_REGISTER_STATIC_COLOR)
#undef MAUI_XAML_REGISTER_STATIC_COLOR
    }
} // namespace maui::xaml
