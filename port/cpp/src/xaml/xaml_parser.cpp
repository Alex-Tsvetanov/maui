// maui::xaml::xaml_parser  <=  src/Controls/src/Xaml/XamlParser.cs (parse half) + the XamlLoader
// root loop (XamlLoader.cs) + TypeArgumentsParser.cs + XmlnsHelper.cs (targetPlatform clause) + the
// MarkupExpressionParser.cs string tokenization.
//
// C# pulls events from an XmlReader; the port walks a pugixml DOM. The mapping is 1:1 per decision:
//   ParseXamlElementFor's reader loop      → iterate an element's DOM children in document order
//   ReadNode(nested: true)  (single child) → build_element_node(child)
//   ReadNode(nested: false) (prop element) → read_node_for_property_element(prop_elem)
//   XmlReader namespace scoping            → per-element xml_namespace_resolver snapshots
//   XmlReader well-formedness errors       → pugixml parse status / explicit duplicate-attribute +
//                                            undeclared-prefix checks (XmlException → xaml_parse_exception)
//   IXmlLineInfo (1-based line/position)   → xml_node::offset_debug() through a line-start index
#include "maui/xaml/xaml_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <pugixml.hpp>

#include "maui/xaml/xaml_node.hpp"

namespace maui::xaml
{
    namespace
    {
        // ---- text helpers ----
        constexpr std::string_view whitespace_chars = " \t\n\r\f\v";

        [[nodiscard]] std::string_view trim(std::string_view text)
        {
            const auto first = text.find_first_not_of(whitespace_chars);
            if (first == std::string_view::npos)
            {
                return {};
            }
            const auto last = text.find_last_not_of(whitespace_chars);
            return text.substr(first, last - first + 1);
        }

        [[nodiscard]] bool is_null_or_whitespace(std::string_view text)
        {
            return trim(text).empty();
        }

        // ---- line info: byte offset → 1-based (line, position), the IXmlLineInfo equivalent ----
        class line_index
        {
        public:
            explicit line_index(std::string_view text)
            {
                line_starts_.push_back(0);
                for (std::size_t i = 0; i < text.size(); ++i)
                {
                    if (text[i] == '\n')
                    {
                        line_starts_.push_back(i + 1);
                    }
                }
            }

            [[nodiscard]] std::pair<int, int> position(std::ptrdiff_t offset) const
            {
                if (offset < 0)
                {
                    return {-1, -1};
                }
                const auto off = static_cast<std::size_t>(offset);
                const auto upper = std::ranges::upper_bound(line_starts_, off);
                const auto line = static_cast<int>(std::distance(line_starts_.begin(), upper));
                const auto column = static_cast<int>(off - *std::prev(upper) + 1);
                return {line, column};
            }

        private:
            std::vector<std::size_t> line_starts_;
        };

        // ---- xmlns / qualified-name helpers ----
        struct qualified_name
        {
            std::string_view prefix;
            std::string_view local;
        };

        [[nodiscard]] qualified_name split_qualified(std::string_view raw)
        {
            const auto colon = raw.find(':');
            if (colon == std::string_view::npos)
            {
                return {.prefix = {}, .local = raw};
            }
            return {.prefix = raw.substr(0, colon), .local = raw.substr(colon + 1)};
        }

        [[nodiscard]] bool is_xmlns_declaration(std::string_view attribute_name)
        {
            return attribute_name == "xmlns" || attribute_name.starts_with("xmlns:");
        }

        // The xmlns declarations on one element: ("", uri) for the default namespace, (prefix, uri)
        // otherwise — the ParseXamlAttributes `xmlns` out-list.
        [[nodiscard]] std::vector<std::pair<std::string, std::string>> xmlns_declarations(const pugi::xml_node& element)
        {
            std::vector<std::pair<std::string, std::string>> declarations;
            for (const pugi::xml_attribute& attribute : element.attributes())
            {
                const std::string_view name = attribute.name();
                if (name == "xmlns")
                {
                    declarations.emplace_back("", attribute.value());
                }
                else if (name.starts_with("xmlns:"))
                {
                    declarations.emplace_back(std::string(name.substr(6)), attribute.value());
                }
            }
            return declarations;
        }

        // XmlReader enforces attribute uniqueness while reading; pugixml does not — mirrored check
        // (raw names; XmlException's wording).
        void ensure_no_duplicate_attributes(const pugi::xml_node& element, int line_number, int line_position)
        {
            for (auto first = element.attributes_begin(); first != element.attributes_end(); ++first)
            {
                for (auto second = std::next(first); second != element.attributes_end(); ++second)
                {
                    if (std::string_view(first->name()) == std::string_view(second->name()))
                    {
                        throw xaml_parse_exception(std::format("'{}' is a duplicate attribute name.", first->name()),
                                                   line_number, line_position);
                    }
                }
            }
        }

        // The element-name namespace: the default xmlns for unprefixed names; an undeclared prefix
        // is an XmlReader well-formedness error.
        [[nodiscard]] std::string element_namespace(std::string_view prefix, const xml_namespace_resolver& scope,
                                                    int line_number, int line_position)
        {
            if (prefix.empty())
            {
                return scope.lookup_namespace("").value_or("");
            }
            auto namespace_uri = scope.lookup_namespace(prefix);
            if (!namespace_uri)
            {
                throw xaml_parse_exception(std::format("'{}' is an undeclared prefix.", prefix), line_number,
                                           line_position);
            }
            return *std::move(namespace_uri);
        }

        // ---- XmlnsHelper.ParseXmlns (ParseClrNamespace / ParseUsing) ----
        struct xmlns_parts
        {
            std::optional<std::string> type_name;
            std::optional<std::string> clr_namespace;
            std::optional<std::string> assembly;
            std::optional<std::string> target_platform;
        };

        [[nodiscard]] xmlns_parts parse_xmlns(std::string_view xmlns)
        {
            xmlns_parts parts;
            const std::string_view trimmed = trim(xmlns);

            if (trimmed.starts_with("using:"))
            {
                // XmlnsHelper.ParseUsing: only the using: clause is read.
                std::string_view rest = trimmed;
                while (!rest.empty())
                {
                    const auto semicolon = rest.find(';');
                    const std::string_view decl = rest.substr(0, semicolon);
                    rest = semicolon == std::string_view::npos ? std::string_view{} : rest.substr(semicolon + 1);
                    if (decl.starts_with("using:"))
                    {
                        parts.clr_namespace = std::string(decl.substr(6));
                    }
                }
                return parts;
            }

            // XmlnsHelper.ParseClrNamespace.
            std::string_view rest = trimmed;
            while (!rest.empty())
            {
                const auto semicolon = rest.find(';');
                const std::string_view decl = rest.substr(0, semicolon);
                rest = semicolon == std::string_view::npos ? std::string_view{} : rest.substr(semicolon + 1);

                if (decl.starts_with("clr-namespace:"))
                {
                    parts.clr_namespace = std::string(decl.substr(14));
                    continue;
                }
                if (decl.starts_with("assembly="))
                {
                    parts.assembly = std::string(decl.substr(9));
                    continue;
                }
                if (decl.starts_with("targetPlatform="))
                {
                    parts.target_platform = std::string(decl.substr(15));
                    continue;
                }
                const auto last_dot = decl.rfind('.');
                if (last_dot != std::string_view::npos && last_dot > 0)
                {
                    parts.clr_namespace = std::string(decl.substr(0, last_dot));
                    parts.type_name = std::string(decl.substr(last_dot + 1));
                }
                else
                {
                    parts.type_name = std::string(decl);
                }
            }
            return parts;
        }

        // ---- XamlParser.PrefixesToIgnore ----
        [[nodiscard]] std::vector<std::string> prefixes_to_ignore(
            const std::vector<std::pair<std::string, std::string>>& xmlns, const parse_options& options)
        {
            std::vector<std::string> prefixes;
            for (const auto& [prefix, uri] : xmlns)
            {
                const xmlns_parts parts = parse_xmlns(uri);
                if (!parts.target_platform)
                {
                    continue;
                }
                if (!options.target_platform)
                {
                    // C#: DeviceInfo.Platform unavailable (InvalidOperationException) → ignorable.
                    prefixes.push_back(prefix);
                    continue;
                }
                if (*parts.target_platform != *options.target_platform)
                {
                    // Special case for Windows backward compatibility.
                    if (*parts.target_platform == "Windows" && *options.target_platform == "WinUI")
                    {
                        continue;
                    }
                    prefixes.push_back(prefix);
                }
            }
            return prefixes;
        }

        // ---- XamlParser.GetValueNode: "{}"-escape and "{…}" markup detection ----
        [[nodiscard]] std::shared_ptr<i_xaml_node> get_value_node(
            std::string_view value, const std::shared_ptr<const xml_namespace_resolver>& resolver, int line_number,
            int line_position)
        {
            const std::string_view trimmed = trim(value);
            if (trimmed.starts_with("{}"))
            {
                // C# substrings the ORIGINAL string by 2 even though the check trims — mirrored
                // (only observable when the raw attribute value has leading whitespace).
                return std::make_shared<value_node>(std::string(value.substr(2)), resolver, line_number, line_position,
                                                    /*is_escaped*/ true);
            }
            if (trimmed.starts_with("{"))
            {
                return std::make_shared<markup_node>(std::string(trimmed), resolver, line_number, line_position);
            }
            return std::make_shared<value_node>(std::string(value), resolver, line_number, line_position);
        }

        // ---- XamlParser.ParseXamlAttributes (the non-xmlns attributes → properties) ----
        // Deviation: pugixml has no per-attribute offsets, so attribute value nodes carry the owning
        // element's line info.
        [[nodiscard]] std::vector<std::pair<xml_name, std::shared_ptr<i_xaml_node>>> parse_xaml_attributes(
            const pugi::xml_node& element, const std::shared_ptr<const xml_namespace_resolver>& resolver,
            int line_number, int line_position)
        {
            std::vector<std::pair<xml_name, std::shared_ptr<i_xaml_node>>> attributes;
            for (const pugi::xml_attribute& attribute : element.attributes())
            {
                const std::string_view raw_name = attribute.name();
                if (is_xmlns_declaration(raw_name))
                {
                    continue; // consumed by the scope push (the C# `xmlns` out-list)
                }

                const auto [prefix, local] = split_qualified(raw_name);
                // Attributes do NOT inherit the default xmlns (per XML namespaces; XmlReader gives
                // "" for unprefixed attributes)…
                std::string namespace_uri;
                if (!prefix.empty())
                {
                    auto resolved = resolver->lookup_namespace(prefix);
                    if (!resolved)
                    {
                        throw xaml_parse_exception(std::format("'{}' is an undeclared prefix.", prefix), line_number,
                                                   line_position);
                    }
                    namespace_uri = *std::move(resolved);
                }
                // …except the attached-property form ("Grid.Row"), which C# rebinds to the default
                // namespace.
                if (local.contains('.') && namespace_uri.empty())
                {
                    namespace_uri = resolver->lookup_namespace("").value_or("");
                }

                xml_name property_name = xaml_parser::parse_property_name(
                    xml_name{.namespace_uri = namespace_uri, .local_name = std::string(local)});
                if (property_name == xml_name::empty())
                {
                    continue; // unhandled x-namespace attribute
                }

                // C# parses x:TypeArguments into IList<XmlType> here and stores the list as the
                // ValueNode's object Value. value_node is string-typed in wave 1, so the node keeps
                // the raw expression; the list is parsed (and its errors thrown) by the
                // get_type_arguments call that always follows this walk, before the element node is
                // built — the same observable error point.
                attributes.emplace_back(std::move(property_name),
                                        get_value_node(attribute.value(), resolver, line_number, line_position));
            }
            return attributes;
        }

        // ---- XamlParser.GetTypeArguments: the parsed x:TypeArguments attribute, if any ----
        [[nodiscard]] std::vector<xml_type> get_type_arguments(const pugi::xml_node& element,
                                                               const xml_namespace_resolver& resolver, int line_number,
                                                               int line_position)
        {
            for (const pugi::xml_attribute& attribute : element.attributes())
            {
                const std::string_view raw_name = attribute.name();
                if (is_xmlns_declaration(raw_name))
                {
                    continue;
                }
                const auto [prefix, local] = split_qualified(raw_name);
                if (local != "TypeArguments" || prefix.empty())
                {
                    continue;
                }
                if (resolver.lookup_namespace(prefix) == x2009_uri)
                {
                    return type_arguments_parser::parse_expression(attribute.value(), resolver, line_number,
                                                                   line_position);
                }
            }
            return {};
        }

        // The recursive element walk; holds the per-parse ambient state (line index + options).
        class document_parser
        {
        public:
            document_parser(const line_index& lines, const parse_options& options) : lines_(&lines), options_(&options)
            {
            }

            [[nodiscard]] std::shared_ptr<element_node> build_element_node(
                const pugi::xml_node& element, const std::shared_ptr<const xml_namespace_resolver>& scope,
                root_node& root)
            {
                const auto [line_number, line_position] = lines_->position(element.offset_debug());
                ensure_no_duplicate_attributes(element, line_number, line_position);

                const auto [prefix, local] = split_qualified(element.name());
                std::string namespace_uri = element_namespace(prefix, *scope, line_number, line_position);

                auto attributes = parse_xaml_attributes(element, scope, line_number, line_position);
                auto prefixes = prefixes_to_ignore(xmlns_declarations(element), *options_);
                auto type_arguments = get_type_arguments(element, *scope, line_number, line_position);

                // (xml_type built before the ctor call: namespace_uri is read by one argument and
                // moved into another — keep the two sequenced.)
                xml_type type(namespace_uri, std::string(local), std::move(type_arguments));
                auto node = std::make_shared<element_node>(std::move(type), std::move(namespace_uri), scope,
                                                           line_number, line_position);
                for (auto& [name, value] : attributes)
                {
                    node->properties().add(std::move(name), std::move(value));
                }
                node->ignorable_prefixes().insert(node->ignorable_prefixes().end(),
                                                  std::make_move_iterator(prefixes.begin()),
                                                  std::make_move_iterator(prefixes.end()));

                parse_xaml_element_for(*node, element, scope, root);
                return node;
            }

            // ---- XamlParser.ParseXamlElementFor: the content of one element ----
            void parse_xaml_element_for(element_node& node, const pugi::xml_node& element,
                                        const std::shared_ptr<const xml_namespace_resolver>& scope, root_node& root)
            {
                const std::string_view element_name = element.name(); // qualified, like reader.Name

                for (const pugi::xml_node& child : element.children())
                {
                    switch (child.type())
                    {
                        case pugi::node_element:
                            parse_child_element(node, element_name, child, scope, root);
                            break;
                        case pugi::node_pcdata:
                        case pugi::node_cdata: {
                            // XmlNodeType.Text/CDATA: trimmed; consecutive text appends onto a
                            // single leading value_node. (C# creates these without line info.)
                            std::string text{trim(child.value())};
                            auto& items = node.collection_items();
                            value_node* leading =
                                items.size() == 1 ? dynamic_cast<value_node*>(items.front().get()) : nullptr;
                            if (leading != nullptr)
                            {
                                leading->set_value(leading->value() + text);
                            }
                            else
                            {
                                items.push_back(std::make_shared<value_node>(std::move(text), scope));
                            }
                            break;
                        }
                        default:
                            // Comments / PIs / declarations are skipped (pugixml's default parse
                            // flags omit them entirely, like the C# switch ignores them).
                            break;
                    }
                }
            }

        private:
            void parse_child_element(element_node& node, std::string_view element_name, const pugi::xml_node& child,
                                     const std::shared_ptr<const xml_namespace_resolver>& scope, root_node& root)
            {
                const std::string_view child_name = child.name(); // qualified
                const auto [line_number, line_position] = lines_->position(child.offset_debug());

                // XmlReader rejects duplicate attributes on ANY element — covered here for the
                // property-element / x:Arguments branches (build_element_node re-checks the rest).
                ensure_no_duplicate_attributes(child, line_number, line_position);

                const auto declarations = xmlns_declarations(child);
                const auto child_scope = declarations.empty() ? scope : scope->extend(declarations);

                const auto [child_prefix, child_local] = split_qualified(child_name);
                const std::string child_namespace =
                    element_namespace(child_prefix, *child_scope, line_number, line_position);

                // 1. Property element.
                if (child_name.contains('.'))
                {
                    xml_name name;
                    if (child_name.starts_with(std::string(element_name) + "."))
                    {
                        name = xml_name{.namespace_uri = child_namespace,
                                        .local_name = std::string(child_name.substr(element_name.size() + 1))};
                    }
                    else // attached BP
                    {
                        name = xml_name{.namespace_uri = child_namespace, .local_name = std::string(child_local)};
                    }

                    if (node.properties().contains(name))
                    {
                        throw xaml_parse_exception(std::format("'{}' is a duplicate property name.", child_name),
                                                   line_number, line_position);
                    }

                    // Property elements should not have attributes (except xmlns declarations).
                    for (const pugi::xml_attribute& attribute : child.attributes())
                    {
                        if (!is_xmlns_declaration(attribute.name()))
                        {
                            root.warnings().push_back(
                                {.message = std::format("Property element '{}' cannot have attributes. "
                                                        "Attribute '{}' will be ignored.",
                                                        name.local_name, attribute.name()),
                                 .line_number = line_number,
                                 .line_position = line_position});
                        }
                    }

                    auto prop = read_node_for_property_element(child, child_scope, root);
                    if (prop != nullptr)
                    {
                        node.properties().add(std::move(name), std::move(prop));
                    }
                    return;
                }

                // 2. Xaml2009 primitives, x:Arguments, …
                if (child_namespace == x2009_uri && child_local == "Arguments")
                {
                    if (node.properties().contains(xml_name::x_arguments()))
                    {
                        throw xaml_parse_exception("'x:Arguments' is a duplicate directive name.", line_number,
                                                   line_position);
                    }
                    auto prop = read_node_for_property_element(child, child_scope, root);
                    if (prop != nullptr)
                    {
                        node.properties().add(xml_name::x_arguments(), std::move(prop));
                    }
                    return;
                }

                // 3. DataTemplate / ControlTemplate: the single child becomes _CreateContent.
                if ((node.type().namespace_uri() == maui_uri || node.type().namespace_uri() == maui_global_uri) &&
                    (node.type().name() == "DataTemplate" || node.type().name() == "ControlTemplate"))
                {
                    if (node.properties().contains(xml_name::create_content()))
                    {
                        throw xaml_parse_exception(std::format("Multiple child elements in {}", node.type().name()),
                                                   line_number, line_position);
                    }
                    auto prop = build_element_node(child, child_scope, root);
                    node.properties().add(xml_name::create_content(), std::move(prop));
                    return;
                }

                // 4. Implicit content, implicit collection, or collection syntax — resolved later.
                node.collection_items().push_back(build_element_node(child, child_scope, root));
            }

            // ---- ReadNode(nested: false): the value of a property element / x:Arguments ----
            // null for an empty element; the single child; or a list_node of all children.
            [[nodiscard]] std::shared_ptr<i_xaml_node> read_node_for_property_element(
                const pugi::xml_node& property_element, const std::shared_ptr<const xml_namespace_resolver>& scope,
                root_node& root)
            {
                std::vector<std::shared_ptr<i_xaml_node>> nodes;
                for (const pugi::xml_node& child : property_element.children())
                {
                    switch (child.type())
                    {
                        case pugi::node_element: {
                            const auto declarations = xmlns_declarations(child);
                            const auto child_scope = declarations.empty() ? scope : scope->extend(declarations);
                            nodes.push_back(build_element_node(child, child_scope, root));
                            break;
                        }
                        case pugi::node_pcdata:
                        case pugi::node_cdata: {
                            const auto [line_number, line_position] = lines_->position(child.offset_debug());
                            nodes.push_back(std::make_shared<value_node>(std::string(trim(child.value())), scope,
                                                                         line_number, line_position));
                            break;
                        }
                        default:
                            break;
                    }
                }
                if (nodes.empty())
                {
                    return nullptr;
                }
                if (nodes.size() == 1)
                {
                    return nodes.front();
                }
                // C# stamps the list with the closing tag's position; pugixml exposes no end-tag
                // offsets — the property element's start position instead (documented deviation).
                const auto [line_number, line_position] = lines_->position(property_element.offset_debug());
                return std::make_shared<list_node>(std::move(nodes), scope, line_number, line_position);
            }

            const line_index* lines_;
            const parse_options* options_;
        };
    } // namespace

    // ---- xaml_parse_exception (XamlParseException.FormatMessage) ----
    namespace
    {
        [[nodiscard]] std::string format_parse_message(const std::string& message, int line_number, int line_position)
        {
            if (line_number < 0 || line_position < 0)
            {
                return message;
            }
            return std::format("Position {}:{}. {}", line_number, line_position, message);
        }
    } // namespace

    xaml_parse_exception::xaml_parse_exception(const std::string& message, int line_number, int line_position)
        : std::runtime_error(format_parse_message(message, line_number, line_position)), unformatted_message_(message),
          line_number_(line_number), line_position_(line_position)
    {
    }

    // ---- type_arguments_parser (TypeArgumentsParser.cs) ----
    namespace
    {
        // TypeArgumentsParser.Parse: one type from `match`; the rest (after a top-level ',') goes
        // to `remaining`.
        [[nodiscard]] xml_type parse_one_type_argument(std::string_view match, std::string_view& remaining,
                                                       const xml_namespace_resolver& resolver, int line_number,
                                                       int line_position)
        {
            remaining = {};
            std::size_t pos = 0;
            int parens_count = 0;
            bool is_generic = false;

            for (pos = 0; pos < match.size(); ++pos)
            {
                if (match[pos] == '(')
                {
                    ++parens_count;
                    is_generic = true;
                }
                else if (match[pos] == ')')
                {
                    --parens_count;
                }
                else if (match[pos] == ',' && parens_count == 0)
                {
                    remaining = match.substr(pos + 1);
                    break;
                }
            }
            std::string_view type = trim(match.substr(0, pos));

            std::vector<xml_type> type_arguments;
            if (is_generic)
            {
                const auto open_bracket = type.find('(');
                const auto close_bracket = type.rfind(')');
                type_arguments = type_arguments_parser::parse_expression(
                    type.substr(open_bracket + 1, close_bracket - open_bracket - 1), resolver, line_number,
                    line_position);
                type = type.substr(0, open_bracket);
            }

            // C# returns a null XmlType for a malformed name (>1 ':') and fails later on use; the
            // port fails eagerly (documented deviation).
            std::string_view prefix;
            std::string_view name = type;
            const auto colon = type.find(':');
            if (colon != std::string_view::npos)
            {
                prefix = type.substr(0, colon);
                name = type.substr(colon + 1);
                if (name.contains(':'))
                {
                    throw xaml_parse_exception(std::format("Invalid type name '{}'.", type), line_number,
                                               line_position);
                }
            }

            const auto namespace_uri = resolver.lookup_namespace(prefix);
            if (!namespace_uri)
            {
                throw xaml_parse_exception(std::format("No xmlns declaration for prefix '{}'.", prefix), line_number,
                                           line_position);
            }
            return {*namespace_uri, std::string(name), std::move(type_arguments)};
        }
    } // namespace

    std::vector<xml_type> type_arguments_parser::parse_expression(std::string_view expression,
                                                                  const xml_namespace_resolver& resolver,
                                                                  int line_number, int line_position)
    {
        std::vector<xml_type> type_list;
        std::string_view remaining = expression;
        while (!is_null_or_whitespace(remaining))
        {
            const std::string_view match = remaining;
            type_list.push_back(parse_one_type_argument(match, remaining, resolver, line_number, line_position));
        }
        return type_list;
    }

    xml_type type_arguments_parser::parse_single(std::string_view expression, const xml_namespace_resolver& resolver,
                                                 int line_number, int line_position)
    {
        std::string_view remaining;
        xml_type type = parse_one_type_argument(expression, remaining, resolver, line_number, line_position);
        if (!is_null_or_whitespace(remaining))
        {
            throw xaml_parse_exception(
                std::format("Invalid type expression or more than one type declared in '{}'", expression), line_number,
                line_position);
        }
        return type;
    }

    // ---- xaml_parser ----
    xml_name xaml_parser::parse_property_name(const xml_name& name)
    {
        if (name.namespace_uri == x2006_uri)
        {
            if (name.local_name == "Key")
            {
                return xml_name::x_key();
            }
            if (name.local_name == "Name")
            {
                return xml_name::x_name();
            }
            if (name.local_name == "Class")
            {
                return xml_name::x_class();
            }
            if (name.local_name == "FieldModifier")
            {
                return xml_name::x_field_modifier();
            }
            return xml_name::empty(); // unhandled attribute
        }

        if (name.namespace_uri == x2009_uri)
        {
            if (name.local_name == "Key")
            {
                return xml_name::x_key();
            }
            if (name.local_name == "Name")
            {
                return xml_name::x_name();
            }
            if (name.local_name == "TypeArguments")
            {
                return xml_name::x_type_arguments();
            }
            if (name.local_name == "DataType")
            {
                return xml_name::x_data_type();
            }
            if (name.local_name == "Class")
            {
                return xml_name::x_class();
            }
            if (name.local_name == "FieldModifier")
            {
                return xml_name::x_field_modifier();
            }
            if (name.local_name == "FactoryMethod")
            {
                return xml_name::x_factory_method();
            }
            if (name.local_name == "Arguments")
            {
                return xml_name::x_arguments();
            }
            if (name.local_name == "ClassModifier")
            {
                return xml_name::x_class_modifier();
            }
            return xml_name::empty(); // unhandled attribute
        }

        return name;
    }

    std::shared_ptr<root_node> xaml_parser::parse(std::string_view xaml, const parse_options& options)
    {
        pugi::xml_document document;
        // Default flags mirror the C# loop's node handling: CDATA + entity expansion on; comments,
        // PIs, the XML declaration, DOCTYPE and whitespace-only text omitted. UTF-8 is forced so
        // offset_debug stays in the caller's byte space (PROFILE §5: std::string is UTF-8).
        const pugi::xml_parse_result result =
            document.load_buffer(xaml.data(), xaml.size(), pugi::parse_default, pugi::encoding_utf8);
        const line_index lines(xaml);
        if (!result)
        {
            const auto [line_number, line_position] = lines.position(result.offset);
            throw xaml_parse_exception(result.description(), line_number, line_position);
        }
        const pugi::xml_node root_element = document.document_element();
        if (!root_element)
        {
            // XmlReader: "Root element is missing." (e.g. a comments-only document).
            throw xaml_parse_exception("Root element is missing.");
        }

        const auto [line_number, line_position] = lines.position(root_element.offset_debug());
        ensure_no_duplicate_attributes(root_element, line_number, line_position);

        const auto declarations = xmlns_declarations(root_element);
        const auto scope = declarations.empty() ? xml_namespace_resolver::built_in()
                                                : xml_namespace_resolver::built_in()->extend(declarations);

        const auto [prefix, local] = split_qualified(root_element.name());
        std::string namespace_uri = element_namespace(prefix, *scope, line_number, line_position);
        auto type_arguments = get_type_arguments(root_element, *scope, line_number, line_position);

        // XamlLoader.Create: new XmlType(reader.NamespaceURI, reader.Name, typeArguments) — note
        // the root keeps the QUALIFIED name (prefix included), unlike inner elements (LocalName).
        auto root = std::make_shared<root_node>(
            xml_type(std::move(namespace_uri), root_element.name(), std::move(type_arguments)), scope, line_number,
            line_position);

        // XamlParser.ParseXaml: root attributes + ignorable prefixes, then the content.
        auto attributes = parse_xaml_attributes(root_element, scope, line_number, line_position);
        auto prefixes = prefixes_to_ignore(declarations, options);
        root->ignorable_prefixes().insert(root->ignorable_prefixes().end(), std::make_move_iterator(prefixes.begin()),
                                          std::make_move_iterator(prefixes.end()));
        for (auto& [name, value] : attributes)
        {
            root->properties().add(std::move(name), std::move(value));
        }

        document_parser parser(lines, options);
        parser.parse_xaml_element_for(*root, root_element, scope, *root);
        return root;
    }

    // ---- markup-extension string tokenization (MarkupExpressionParser.cs) ----
    markup_match match_markup(std::string_view expression)
    {
        if (expression.size() < 2)
        {
            return {.matched = false, .match = {}, .end = 1};
        }
        if (expression[0] != '{')
        {
            return {.matched = false, .match = {}, .end = 2};
        }

        std::size_t i = 1;
        bool found = false;
        for (; i < expression.size(); ++i)
        {
            if (expression[i] == ' ')
            {
                continue;
            }
            found = true;
            break;
        }
        if (!found)
        {
            return {.matched = false, .match = {}, .end = 3};
        }

        std::size_t c = 0;
        for (; c + i < expression.size(); ++c)
        {
            if (expression[i + c] == ' ' || expression[i + c] == '}')
            {
                break;
            }
        }
        if (i + c == expression.size())
        {
            return {.matched = false, .match = {}, .end = 6};
        }

        return {.matched = true, .match = std::string(expression.substr(i, c)), .end = i + c};
    }

    markup_piece get_next_piece(std::string_view remaining)
    {
        bool in_string = false;
        std::size_t end = 0;
        char string_terminator = '\0';
        std::string piece;

        // Inside a quoted string every char is appended until the matching terminator.
        while (end < remaining.size() &&
               (in_string || (remaining[end] != '}' && remaining[end] != ',' && remaining[end] != '=')))
        {
            if (in_string)
            {
                if (remaining[end] == string_terminator)
                {
                    in_string = false;
                    ++end;
                    // C# indexes unguarded here (IndexOutOfRangeException at end-of-string); the
                    // port bounds-checks and falls through to the "Unexpected end" error below.
                    while (end < remaining.size() && remaining[end] == ' ')
                    {
                        ++end;
                    }
                    break;
                }
            }
            else
            {
                if (remaining[end] == '\'' || remaining[end] == '"')
                {
                    in_string = true;
                    string_terminator = remaining[end];
                    ++end;
                    continue;
                }
            }

            // An escape char: consume it and append the next char.
            if (remaining[end] == '\\')
            {
                ++end;
                if (end == remaining.size())
                {
                    break;
                }
            }
            piece.push_back(remaining[end]);
            ++end;
        }

        if (in_string && end == remaining.size())
        {
            throw xaml_parse_exception("Unterminated quoted string");
        }
        if (end == 0)
        {
            throw xaml_parse_exception("Empty value string in markup expression");
        }
        if (end >= remaining.size())
        {
            throw xaml_parse_exception("Unexpected end of markup expression");
        }

        const char next = remaining[end];
        const std::string_view rest = remaining.substr(end + 1);

        // Whitespace is trimmed from the end of the piece (before any quote stripping the caller
        // may do).
        while (!piece.empty() && (std::isspace(static_cast<unsigned char>(piece.back())) != 0))
        {
            piece.pop_back();
        }

        return {.piece = std::move(piece), .next = next, .remaining = rest};
    }

    std::pair<std::string, std::string> parse_markup_name(std::string_view name)
    {
        const auto first_colon = name.find(':');
        if (first_colon == std::string_view::npos)
        {
            return {"", std::string(name)};
        }
        if (name.find(':', first_colon + 1) != std::string_view::npos)
        {
            throw std::invalid_argument("name"); // C#: ArgumentException(null, nameof(name))
        }
        return {std::string(name.substr(0, first_colon)), std::string(name.substr(first_colon + 1))};
    }
} // namespace maui::xaml
