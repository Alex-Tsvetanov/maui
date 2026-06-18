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
#include <vector>

#include "maui/controls/application.hpp"
#include "maui/controls/bindings/binding.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/bindings/relative_binding_source.hpp"
#include "maui/controls/dynamic_resource.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/file_image_source.hpp" // image_source::from_font ({FontImage})
#include "maui/controls/font_image_source.hpp" // font_image_source::default_size
#include "maui/controls/resource_dictionary.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/app_theme.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/font.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/xaml/i_markup_extension.hpp"
#include "maui/xaml/name_scope.hpp" // {x:Reference} IReferenceProvider
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

        // ---- {x:Array Type=… }  <=  Microsoft.Maui.Controls.Xaml.ArrayExtension ----
        // C# ProvideValue requires Type (else "Type argument mandatory for x:Array extension") and
        // builds Array.CreateInstance(Type, Items.Count). The reflection-free port returns an xaml_array
        // marker carrying the resolved element type_tag + the items in source form (markup_extensions
        // .hpp documents the deviation + the loader's curly-form-only item path). Type arrives as a
        // PRE-RESOLVED {x:Type} value (a type_tag in the `values` map); a raw string has no
        // string→type lookup at this seam (the type registry needs a namespace too — {x:Type} does it).
        class array_extension final : public i_markup_extension
        {
        public:
            array_extension(std::optional<maui::core::type_tag> element_type, std::vector<std::any> items)
                : element_type_(element_type), items_(std::move(items))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                if (!element_type_.has_value())
                {
                    throw xaml_parse_exception("Type argument mandatory for x:Array extension");
                }
                return std::any{xaml_array{.element_type = *element_type_, .items = items_}};
            }

        private:
            std::optional<maui::core::type_tag> element_type_;
            std::vector<std::any> items_;
        };

        // ---- {x:Reference Name=… }  <=  Microsoft.Maui.Controls.Xaml.ReferenceExtension ----
        // C# resolves Name via IReferenceProvider.FindByName, then falls back to walking ParentObjects
        // for a BindableObject carrying a NameScope. The port collapses both into one lookup against
        // the service provider's reference_provider (the nearest enclosing element's name scope, which
        // the register_x_names pass populated before apply). A miss throws (the C# XamlParseException).
        class reference_extension final : public i_markup_extension
        {
        public:
            explicit reference_extension(std::optional<std::string> name) : name_(std::move(name))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& services) override
            {
                if (!name_.has_value() || name_->empty())
                {
                    // ContentProperty(nameof(Name)); an empty/absent Name finds nothing.
                    throw xaml_parse_exception("Cannot find the object referenced by ``");
                }
                if (services.reference_provider != nullptr)
                {
                    if (const std::any* value = services.reference_provider->find_by_name(*name_))
                    {
                        return *value; // the registered object, in its stored form (a shared_ptr box)
                    }
                }
                throw xaml_parse_exception(std::format("Cannot find the object referenced by `{}`", *name_));
            }

        private:
            std::optional<std::string> name_;
        };

        // ---- {FontImage …}  <=  Microsoft.Maui.Controls.Xaml.FontImageExtension ----
        // C# FontImageExtension IS a FontImageSource (obsolete subclass), so the markup builds one from
        // Glyph (ContentProperty) / FontFamily / Size (default 30) / Color / FontAutoScalingEnabled.
        // The port mints the same source via image_source::from_font, assembling the font from
        // FontFamily + Size + auto-scaling exactly like the Y3 font_image_source tests.
        class font_image_extension final : public i_markup_extension
        {
        public:
            font_image_extension(std::string glyph, std::string font_family, double size, maui::graphics::color color,
                                 bool auto_scaling)
                : glyph_(std::move(glyph)), font_family_(std::move(font_family)), size_(size), color_(color),
                  auto_scaling_(auto_scaling)
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                // FontImageSource builds its Font from FontFamily + Size; auto-scaling folds in via
                // WithAutoScaling. An empty FontFamily keeps the system family (font::of_size("") ).
                maui::core::font font = maui::core::font::of_size(font_family_, size_).with_auto_scaling(auto_scaling_);
                return std::any{maui::controls::image_source::from_font(glyph_, std::move(font), color_)};
            }

        private:
            std::string glyph_;
            std::string font_family_;
            double size_ = maui::controls::font_image_source::default_size;
            maui::graphics::color color_;
            bool auto_scaling_ = false;
        };

        // ---- {TemplateBinding …}  <=  Microsoft.Maui.Controls.Xaml.TemplateBindingExtension ----
        // C# returns `new Binding { Source = RelativeBindingSource.TemplatedParent, Path, Mode,
        // Converter, ConverterParameter, StringFormat }`. The port builds the same maui::controls::
        // binding with its source set to relative_binding_source::templated_parent() and carries it in a
        // binding_request to the existing SetBinding applier (the same route {Binding} uses).
        class template_binding_extension final : public i_markup_extension
        {
        public:
            explicit template_binding_extension(binding_request request) : request_(std::move(request))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                return std::any{request_};
            }

        private:
            binding_request request_;
        };

        // RelativeBindingSourceMode parsing — the [ContentProperty] Mode (Enum.Parse, case-sensitive
        // names, the port's enumerator values for the numeric form).
        [[nodiscard]] maui::controls::relative_binding_source_mode parse_relative_source_mode(const std::string& text)
        {
            using maui::controls::relative_binding_source_mode;
            static constexpr std::array<enum_entry<relative_binding_source_mode>, 4> k_names{{
                {.name = "TemplatedParent", .value = relative_binding_source_mode::templated_parent},
                {.name = "Self", .value = relative_binding_source_mode::self},
                {.name = "FindAncestor", .value = relative_binding_source_mode::find_ancestor},
                {.name = "FindAncestorBindingContext",
                 .value = relative_binding_source_mode::find_ancestor_binding_context},
            }};
            if (const auto parsed = try_parse_enum<relative_binding_source_mode>(text, k_names))
            {
                return *parsed;
            }
            throw xaml_parse_exception(std::format("Cannot convert \"{}\" into RelativeBindingSourceMode", text));
        }

        // ---- {RelativeSource …}  <=  Microsoft.Maui.Controls.Xaml.RelativeSourceExtension ----
        // C# returns a RelativeBindingSource for Self / TemplatedParent, or one carrying AncestorType
        // for FindAncestor[BindingContext]. The reflection-free port fully supports Self and
        // TemplatedParent (the shared singletons, no type needed). FindAncestor[BindingContext] need a
        // runtime type predicate the port cannot synthesize from a markup {x:Type} tag (relative_binding
        // _source uses COMPILE-TIME find_ancestor<T> probes — PROFILE §6), so they are a loud deferral.
        class relative_source_extension final : public i_markup_extension
        {
        public:
            relative_source_extension(maui::controls::relative_binding_source_mode mode, bool has_ancestor_type)
                : mode_(mode), has_ancestor_type_(has_ancestor_type)
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& /*services*/) override
            {
                using maui::controls::relative_binding_source;
                using maui::controls::relative_binding_source_mode;
                if (has_ancestor_type_ || mode_ == relative_binding_source_mode::find_ancestor ||
                    mode_ == relative_binding_source_mode::find_ancestor_binding_context)
                {
                    // C# permits AncestorType to drive the mode; the port can model neither without a
                    // runtime type predicate (see the header note) — fail loudly rather than silently.
                    throw xaml_parse_exception(
                        "{RelativeSource} FindAncestor / FindAncestorBindingContext (and AncestorType) are not "
                        "supported by the port: a runtime ancestor-type predicate needs reflection (STATUS.md M7 "
                        "deferrals). Self and TemplatedParent are supported.");
                }
                if (mode_ == relative_binding_source_mode::self)
                {
                    return std::any{relative_binding_source::self()};
                }
                if (mode_ == relative_binding_source_mode::templated_parent)
                {
                    return std::any{relative_binding_source::templated_parent()};
                }
                throw xaml_parse_exception("Invalid Mode"); // C# RelativeSourceExtension's final else
            }

        private:
            maui::controls::relative_binding_source_mode mode_;
            bool has_ancestor_type_ = false;
        };

        // ---- {DataTemplate TypeName=… }  <=  Microsoft.Maui.Controls.Xaml.DataTemplateExtension ----
        // C# resolves TypeName through IXamlTypeResolver and returns `new DataTemplate(type)`. The port
        // resolves the markup name in the type registry (services-provided, else the default) and builds
        // a data_template whose loader activates the type through the registry factory (the reflection-
        // free Activator.CreateInstance analog). DEVIATION: the loader form does not carry the per-type
        // recycle id_string the C# Type ctor does (no reflection type name) — documented.
        class data_template_extension final : public i_markup_extension
        {
        public:
            explicit data_template_extension(std::string type_name) : type_name_(std::move(type_name))
            {
            }

            [[nodiscard]] std::any provide_value(const xaml_service_provider& services) override
            {
                if (type_name_.empty())
                {
                    throw xaml_parse_exception("TypeName isn't set."); // C# XamlParseException
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
                        throw xaml_parse_exception(std::format("No xmlns declaration for prefix '{}'.", prefix));
                    }
                }
                const xaml_type_registry& types =
                    services.type_registry != nullptr ? *services.type_registry : default_xaml_type_registry();
                const xaml_type_registry::registration* registration = types.find(name, ns);
                if (registration == nullptr)
                {
                    // C#: "DataTemplateExtension: Could not locate type for {TypeName}."
                    throw xaml_parse_exception(
                        std::format("DataTemplateExtension: Could not locate type for {}.", type_name_));
                }
                // new DataTemplate(type): each created content activates the type via the registry
                // factory (the no-reflection Activator.CreateInstance stand-in).
                maui::controls::data_template::loader load = [factory = registration->create] { return factory(); };
                return std::any{std::make_shared<maui::controls::data_template>(std::move(load))};
            }

        private:
            std::string type_name_;
        };

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
            // UpdateSourceEventName is now accepted and threaded into `new Binding(…)` (set below):
            // the binding honors it for element targets via the reflection-free named-event seam
            // (binding.hpp). everything else BindingExtension.ProvideValue forwards is honored below.
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
            // BindingExtension.ProvideValue assigns UpdateSourceEventName onto the new Binding (a plain
            // string property). The port honors it for element targets (binding.hpp deviation note).
            if (auto update_source_event = string_argument(args, "UpdateSourceEventName", "Binding"))
            {
                built->set_update_source_event_name(std::move(*update_source_event));
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

        extensions.register_extension("x:Array", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 3> k_known{"", "Type", "Items"};
            require_known_attributes(args, "x:Array", k_known);
            // Type: a PRE-RESOLVED {x:Type} value (a type_tag). A raw string has no string→type lookup
            // at this seam (the type registry keys by name+namespace — {x:Type} owns that).
            std::optional<maui::core::type_tag> element_type;
            if (const std::any* value = find_value(args, "Type"))
            {
                if (const auto* tag = std::any_cast<maui::core::type_tag>(value))
                {
                    element_type = *tag;
                }
                else
                {
                    throw xaml_parse_exception("Property 'Type' of x:Array requires a type provided by {x:Type …}");
                }
            }
            else if (find_attribute(args, "Type") != nullptr)
            {
                throw xaml_parse_exception("Property 'Type' of x:Array requires a markup extension "
                                           "providing a type (e.g. {x:Type …})");
            }
            // Items (the [ContentProperty]): the loader's curly form passes a pre-resolved vector through
            // `values` (the element-children content form is a loader-side collection concern — see
            // markup_extensions.hpp). The positional "" piece is folded in behind a named Items.
            std::vector<std::any> items;
            if (auto content = any_argument(args, "Items"))
            {
                if (auto* vector = std::any_cast<std::vector<std::any>>(&*content))
                {
                    items = std::move(*vector);
                }
                else
                {
                    items.push_back(std::move(*content)); // a single pre-resolved item
                }
            }
            return std::make_unique<array_extension>(element_type, std::move(items));
        });

        extensions.register_extension("x:Reference", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_known{"", "Name"};
            require_known_attributes(args, "x:Reference", k_known);
            return std::make_unique<reference_extension>(string_content_argument(args, "Name", "x:Reference"));
        });

        extensions.register_extension("FontImage", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 6> k_known{"",     "Glyph", "FontFamily",
                                                                     "Size", "Color", "FontAutoScalingEnabled"};
            require_known_attributes(args, "FontImage", k_known);
            // Glyph is the [ContentProperty]; the others are plain string-typed attributes converted
            // through the same converter functions the loader would use for these property types.
            const std::string glyph = string_content_argument(args, "Glyph", "FontImage").value_or(std::string{});
            const std::string font_family = string_argument(args, "FontFamily", "FontImage").value_or(std::string{});
            double size = maui::controls::font_image_source::default_size;
            if (const auto size_text = string_argument(args, "Size", "FontImage"))
            {
                try
                {
                    // FontImageSource.Size carries [TypeConverter(FontSizeConverter)] — so a NamedSize
                    // (Large/Small/Body/…) is valid here, not just a number. Use the ported FontSizeConverter
                    // (convert_font_size), the converter the loader itself applies to the Size property.
                    size = convert_font_size(*size_text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            }
            maui::graphics::color color; // FontImageSource.Color default (transparent)
            if (const auto color_text = string_argument(args, "Color", "FontImage"))
            {
                try
                {
                    color = convert_color(*color_text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            }
            bool auto_scaling = false; // FontImageSource.FontAutoScalingEnabled default
            if (const auto scaling_text = string_argument(args, "FontAutoScalingEnabled", "FontImage"))
            {
                try
                {
                    auto_scaling = convert_bool(*scaling_text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            }
            return std::make_unique<font_image_extension>(glyph, font_family, size, color, auto_scaling);
        });

        extensions.register_extension("TemplateBinding", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 6> k_known{
                "", "Path", "Mode", "Converter", "ConverterParameter", "StringFormat"};
            require_known_attributes(args, "TemplateBinding", k_known);
            binding_request request;
            if (auto path = string_content_argument(args, "Path", "TemplateBinding"))
            {
                request.path = std::move(*path); // [ContentProperty(nameof(Path))]; default SelfPath "."
            }
            if (const auto mode = string_argument(args, "Mode", "TemplateBinding"))
            {
                request.mode = parse_binding_mode(*mode);
            }
            std::shared_ptr<maui::controls::binding> built;
            try
            {
                built = std::make_shared<maui::controls::binding>(request.path, request.mode);
            }
            catch (const std::invalid_argument& error)
            {
                throw xaml_parse_exception(error.what());
            }
            // C#: Source = RelativeBindingSource.TemplatedParent.
            built->set_source(maui::controls::relative_binding_source::templated_parent());
            if (auto format = string_argument(args, "StringFormat", "TemplateBinding"))
            {
                built->set_string_format(std::move(*format));
            }
            // Converter — a pre-resolved IValueConverter instance only (same rule as {Binding}).
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
                        "Property 'Converter' of TemplateBinding requires an i_value_converter instance");
                }
                built->set_converter(std::move(converter));
            }
            else if (find_attribute(args, "Converter") != nullptr)
            {
                throw xaml_parse_exception("Property 'Converter' of TemplateBinding requires a markup extension "
                                           "providing an i_value_converter (e.g. {StaticResource …})");
            }
            if (auto parameter = any_argument(args, "ConverterParameter"))
            {
                built->set_converter_parameter(std::move(*parameter));
            }
            request.instance = std::move(built);
            return std::make_unique<template_binding_extension>(std::move(request));
        });

        extensions.register_extension("RelativeSource", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 4> k_known{"", "Mode", "AncestorLevel", "AncestorType"};
            require_known_attributes(args, "RelativeSource", k_known);
            // Mode is the [ContentProperty]. C# RelativeBindingSourceMode's default is its 0 value (no
            // enumerator), so an unset Mode WITHOUT AncestorType hits the extension's "Invalid Mode"
            // else-throw — surfaced eagerly here. A named Mode is parsed; AncestorType (with or without a
            // Mode) routes to the FindAncestor deferral in provide_value.
            const auto mode_text = string_content_argument(args, "Mode", "RelativeSource");
            const bool has_ancestor_type =
                find_value(args, "AncestorType") != nullptr || find_attribute(args, "AncestorType") != nullptr;
            maui::controls::relative_binding_source_mode mode =
                maui::controls::relative_binding_source_mode::self; // placeholder; overwritten or rejected below
            if (mode_text.has_value())
            {
                mode = parse_relative_source_mode(*mode_text);
            }
            else if (!has_ancestor_type)
            {
                // No Mode and no AncestorType — C# RelativeBindingSourceMode default is 0 (unused), which
                // falls to the extension's "Invalid Mode" throw. Surface that here.
                throw xaml_parse_exception("Invalid Mode");
            }
            return std::make_unique<relative_source_extension>(mode, has_ancestor_type);
        });

        extensions.register_extension("DataTemplate", [](const markup_extension_arguments& args) {
            static constexpr std::array<std::string_view, 2> k_known{"", "TypeName"};
            require_known_attributes(args, "DataTemplate", k_known);
            return std::make_unique<data_template_extension>(
                string_content_argument(args, "TypeName", "DataTemplate").value_or(std::string{}));
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
