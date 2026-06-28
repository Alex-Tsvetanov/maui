// maui_xaml_codegen — the build-time "compile-time XAML" tool (PUBLIC_API_DESIGN.md §6).
//
// It REUSES the framework's runtime XAML parser (maui::xaml::xaml_parser, over pugixml) to turn a .xaml
// document into the node tree, then EMITS idiomatic maui::ui builder C++ from that tree — so the markup is
// compiled into the app (zero runtime XAML parsing) WITHOUT any reflection or a C++26 toolchain. The parser
// is shared with the runtime loader; the new piece is just the node-tree -> C++ emitter below.
//
// Usage:  maui_xaml_codegen <input.xaml> [factory_name] [output.hpp]   (stdout if no output)
//
// Output shape:
//   - no x:Name in the markup  -> a plain factory:  ui::view_ref<content_page> <factory>();
//   - any x:Name present        -> a handles struct: <factory>_handles { ui::view_ref<content_page> root;
//                                  ui::weak_ref<T> <x:Name>...; }; and <factory>_handles <factory>();
//                                  so code-behind can reach named controls (events/bindings).
//
// Scope (current slice): ContentPage / VerticalStackLayout / HorizontalStackLayout / Label / Button / Entry,
// with Text (label/button), Spacing + Padding (stacks), x:Name on any element. Bindings, events, more
// controls/attributes/converters follow.

#include "maui/xaml/xaml_node.hpp"
#include "maui/xaml/xaml_parser.hpp"

#include <cctype>
#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xaml = maui::xaml;

namespace
{
    // Accumulates the named-control locals + the handles-struct fields as the tree is walked.
    struct gen_state
    {
        std::string decls;                                        // "auto <var> = ...; handles.<name> = <var>.weak();"
        std::vector<std::pair<std::string, std::string>> handles; // (x:Name, cpp control type) -> struct fields
        bool has_bindings = false; // any {Binding} present -> VM-templated factory + a handles.bindings vector
        int local_counter = 0;     // names auto-generated locals for unnamed bound controls
    };

    // One parsed `{Binding Path[, Mode=TwoWay]}` on a control property.
    struct binding_spec
    {
        std::string property; // the XAML property local name (e.g. "Text")
        std::string path;     // the source path (e.g. "Message")
        bool two_way = false; // Mode=TwoWay
    };

    // PascalCase XAML name -> snake_case C++ identifier ("ErrorMessage" -> "error_message").
    std::string to_snake(std::string_view name)
    {
        std::string out;
        for (std::size_t i = 0; i < name.size(); ++i)
        {
            const auto ch = static_cast<unsigned char>(name[i]);
            if (std::isupper(ch) != 0)
            {
                if (i != 0)
                {
                    out += '_';
                }
                out += static_cast<char>(std::tolower(ch));
            }
            else
            {
                out += static_cast<char>(ch);
            }
        }
        return out;
    }

    // Parse a `{Binding ...}` markup string; std::nullopt if it is not a {Binding} or has no path.
    std::optional<binding_spec> parse_binding(std::string_view markup)
    {
        if (markup.size() < 2 || markup.front() != '{' || markup.back() != '}')
        {
            return std::nullopt;
        }
        markup = markup.substr(1, markup.size() - 2); // strip the braces

        std::vector<std::string> tokens;
        std::string current;
        for (const char raw : markup)
        {
            if (raw == ' ' || raw == ',')
            {
                if (!current.empty())
                {
                    tokens.push_back(std::move(current));
                    current.clear();
                }
            }
            else
            {
                current += raw;
            }
        }
        if (!current.empty())
        {
            tokens.push_back(std::move(current));
        }
        if (tokens.empty() || tokens.front() != "Binding")
        {
            return std::nullopt;
        }

        binding_spec spec;
        for (std::size_t i = 1; i < tokens.size(); ++i)
        {
            const std::string& token = tokens[i];
            if (token.starts_with("Mode="))
            {
                spec.two_way = token.substr(5) == "TwoWay";
            }
            else if (token.starts_with("Path="))
            {
                spec.path = token.substr(5);
            }
            else if (spec.path.empty() && !token.contains('='))
            {
                spec.path = token; // positional Path
            }
        }
        if (spec.path.empty())
        {
            return std::nullopt; // {Binding} (self) not supported by the codegen yet
        }
        return spec;
    }

    // The {Binding} properties on `node` (markup-valued attributes that resolve to a {Binding}).
    std::vector<binding_spec> collect_bindings(xaml::element_node& node)
    {
        std::vector<binding_spec> out;
        for (auto& [name, value] : node.properties())
        {
            if (auto* markup = dynamic_cast<xaml::markup_node*>(value.get()))
            {
                if (std::optional<binding_spec> spec = parse_binding(markup->markup_string()))
                {
                    spec->property = name.local_name;
                    out.push_back(std::move(*spec));
                }
            }
        }
        return out;
    }

    // The C++ statement that wires one binding (routed through the control's set_*; the source is vm.<path>).
    std::string emit_binding(const std::string& var, const std::string& cpp_type, const binding_spec& spec)
    {
        const std::string setter = "set_" + to_snake(spec.property);
        const std::string source = "vm." + to_snake(spec.path);
        if (spec.two_way)
        {
            const std::string getter = to_snake(spec.property);
            const std::string event = to_snake(spec.property) + "_changed";
            return "    handles.bindings.push_back(ui::bind(" + var + ".impl(), &" + cpp_type + "::" + setter + ", &" +
                   cpp_type + "::" + getter + ", &" + var + ".impl()." + event + ").to_two_way(" + source + "));\n";
        }
        return "    handles.bindings.push_back(ui::bind(" + var + ".impl(), &" + cpp_type + "::" + setter + ").to(" +
               source + "));\n";
    }

    // The first literal (value_node) attribute on `node` whose local name is `local`; "" if absent.
    std::string literal_attr(xaml::element_node& node, std::string_view local)
    {
        for (auto& [name, value] : node.properties())
        {
            if (name.local_name == local)
            {
                if (auto* literal = dynamic_cast<xaml::value_node*>(value.get()))
                {
                    return literal->value();
                }
            }
        }
        return {};
    }

    std::string x_name(xaml::element_node& node)
    {
        return literal_attr(node, "Name"); // x:Name (the x: prefix maps to the canonical "Name" directive)
    }

    // The C++ control type for a XAML element local name (for weak_ref<T> handle fields).
    std::string cpp_type(std::string_view tag)
    {
        if (tag == "Label")
        {
            return "maui::controls::label";
        }
        if (tag == "Button")
        {
            return "maui::controls::button";
        }
        if (tag == "Entry")
        {
            return "maui::controls::entry";
        }
        if (tag == "VerticalStackLayout")
        {
            return "maui::controls::vertical_stack_layout";
        }
        if (tag == "HorizontalStackLayout")
        {
            return "maui::controls::horizontal_stack_layout";
        }
        if (tag == "ContentPage")
        {
            return "maui::controls::content_page";
        }
        return "maui::controls::element";
    }

    // A C++ string literal for a XAML attribute value.
    std::string quote(std::string_view text)
    {
        std::string out = "\"";
        for (const char ch : text)
        {
            if (ch == '"' || ch == '\\')
            {
                out += '\\';
            }
            out += ch;
        }
        out += '"';
        return out;
    }

    std::vector<xaml::element_node*> child_elements(xaml::element_node& node)
    {
        std::vector<xaml::element_node*> kids;
        for (auto& item : node.collection_items())
        {
            if (auto* child = dynamic_cast<xaml::element_node*>(item.get()))
            {
                kids.push_back(child);
            }
        }
        return kids;
    }

    // A Grid ColumnDefinitions/RowDefinitions shorthand ("*,Auto,100") -> the .columns(...)/.rows(...) args.
    std::string parse_track_defs(std::string_view defs)
    {
        std::string out;
        std::string token;
        bool first = true;
        const auto flush = [&] {
            if (token.empty())
            {
                return;
            }
            if (!first)
            {
                out += ", ";
            }
            if (token == "*")
            {
                out += "ui::star()";
            }
            else if (token == "Auto" || token == "auto")
            {
                out += "ui::automatic()";
            }
            else
            {
                out += "ui::absolute(" + token + ")";
            }
            first = false;
            token.clear();
        };
        for (const char ch : defs)
        {
            if (ch == ',')
            {
                flush();
            }
            else if (ch != ' ')
            {
                token += ch;
            }
        }
        flush();
        return out;
    }

    // A Grid.Row / Grid.Column attached-property value on a child (default "0").
    std::string grid_index(xaml::element_node& child, std::string_view attached)
    {
        const std::string value = literal_attr(child, attached);
        return value.empty() ? "0" : value;
    }

    std::string inline_expr(xaml::element_node& node, gen_state& state);

    // The expression to USE for `node` inside its parent: a named local moved in (so code-behind can reach
    // it through the handles struct), or the inline builder expression for an unnamed control.
    std::string use_expr(xaml::element_node& node, gen_state& state)
    {
        const std::string name = x_name(node);
        const std::vector<binding_spec> bindings = collect_bindings(node);
        if (name.empty() && bindings.empty())
        {
            return inline_expr(node, state);
        }

        // A named control and/or one with {Binding}s needs a local so code-behind / the binding wiring can
        // reach it. (Unnamed-but-bound controls get an auto-generated local name.)
        const std::string ctype = cpp_type(node.type().name());
        const std::string var = name.empty() ? ("ctrl_" + std::to_string(state.local_counter++)) : name;
        const std::string expr = inline_expr(node, state);
        state.decls += "    auto " + var + " = " + expr + ";\n";
        if (!name.empty())
        {
            state.decls += "    handles." + name + " = " + var + ".weak();\n";
            state.handles.emplace_back(name, ctype);
        }
        for (const binding_spec& spec : bindings)
        {
            state.decls += emit_binding(var, ctype, spec);
            state.has_bindings = true;
        }
        return "std::move(" + var + ")";
    }

    std::string children_csv(xaml::element_node& node, gen_state& state)
    {
        std::string out;
        bool first = true;
        for (auto* child : child_elements(node))
        {
            if (!first)
            {
                out += ", ";
            }
            out += use_expr(*child, state);
            first = false;
        }
        return out;
    }

    std::string inline_expr(xaml::element_node& node, gen_state& state)
    {
        const std::string& tag = node.type().name();
        std::string expr;

        if (tag == "Label")
        {
            expr = "ui::label(" + quote(literal_attr(node, "Text")) + ")";
        }
        else if (tag == "Button")
        {
            expr = "ui::button(" + quote(literal_attr(node, "Text")) + ")";
        }
        else if (tag == "Entry")
        {
            expr = "ui::entry()";
        }
        else if (tag == "VerticalStackLayout")
        {
            expr = "ui::vstack(" + children_csv(node, state) + ")";
        }
        else if (tag == "HorizontalStackLayout")
        {
            expr = "ui::hstack(" + children_csv(node, state) + ")";
        }
        else if (tag == "Grid")
        {
            expr = "ui::grid()";
            if (const std::string cols = literal_attr(node, "ColumnDefinitions"); !cols.empty())
            {
                expr += ".columns(" + parse_track_defs(cols) + ")";
            }
            if (const std::string rows = literal_attr(node, "RowDefinitions"); !rows.empty())
            {
                expr += ".rows(" + parse_track_defs(rows) + ")";
            }
            if (const std::string row_spacing = literal_attr(node, "RowSpacing"); !row_spacing.empty())
            {
                expr += ".row_spacing(" + row_spacing + ")";
            }
            if (const std::string column_spacing = literal_attr(node, "ColumnSpacing"); !column_spacing.empty())
            {
                expr += ".column_spacing(" + column_spacing + ")";
            }
            for (xaml::element_node* child : child_elements(node))
            {
                expr += ".cell(" + grid_index(*child, "Grid.Row") + ", " + grid_index(*child, "Grid.Column") + ", " +
                        use_expr(*child, state) + ")";
            }
        }
        else if (tag == "ContentPage")
        {
            const std::vector<xaml::element_node*> kids = child_elements(node);
            expr = "ui::page(" + (kids.empty() ? std::string{} : use_expr(*kids.front(), state)) + ")";
        }
        else
        {
            return "/* unsupported element <" + tag + "> */";
        }

        if (const std::string placeholder = literal_attr(node, "Placeholder"); !placeholder.empty())
        {
            expr += ".placeholder(" + quote(placeholder) + ")";
        }
        if (const std::string spacing = literal_attr(node, "Spacing"); !spacing.empty())
        {
            expr += ".spacing(" + spacing + ")";
        }
        if (const std::string padding = literal_attr(node, "Padding"); !padding.empty())
        {
            expr += ".padding(ui::thickness{" + padding + "})"; // single uniform value for now
        }
        return expr;
    }

    int run(std::span<char*> args)
    {
        const std::string input_path = args[1];
        const std::string factory = args.size() >= 3 ? std::string(args[2]) : "build_page";

        const std::ifstream input(input_path);
        if (!input)
        {
            std::cerr << "maui_xaml_codegen: cannot open " << input_path << "\n";
            return 2;
        }
        std::stringstream buffer;
        buffer << input.rdbuf();
        const std::string markup = buffer.str();

        const std::shared_ptr<xaml::root_node> root = xaml::xaml_parser::parse(markup);

        // The document root may be a wrapper whose first child is the real root element.
        xaml::element_node* root_element = root.get();
        {
            gen_state probe;
            if (inline_expr(*root_element, probe).starts_with("/* unsupported"))
            {
                const std::vector<xaml::element_node*> kids = child_elements(*root_element);
                if (!kids.empty())
                {
                    root_element = kids.front();
                }
            }
        }

        gen_state state;
        const std::string tree = inline_expr(*root_element, state);

        std::ofstream file_output;
        std::ostream* out = &std::cout;
        if (args.size() >= 4)
        {
            file_output.open(args[3]);
            if (!file_output)
            {
                std::cerr << "maui_xaml_codegen: cannot open output " << args[3] << "\n";
                return 2;
            }
            out = &file_output;
        }

        *out << "// Generated from " << input_path << " by maui_xaml_codegen — DO NOT EDIT.\n";
        *out << "#pragma once\n\n";
        if (state.has_bindings)
        {
            *out << "#include <vector>\n\n";
        }
        *out << "#include \"maui/ui.hpp\"\n\n";
        *out << "namespace ui = maui::ui;\n\n";

        const bool needs_struct = !state.handles.empty() || state.has_bindings;
        if (!needs_struct)
        {
            // No x:Name, no bindings: a plain factory returning the owning root.
            *out << "[[nodiscard]] inline ui::view_ref<maui::controls::content_page> " << factory << "()\n";
            *out << "{\n    return " << tree << ";\n}\n";
        }
        else
        {
            // A handles struct: the owning root, a weak_ref to each x:Named control (so code-behind can wire
            // events), and — when the markup has {Binding}s — the RAII binding handles. The factory is then
            // templated on the view-model type and wires the bindings against vm.<snake(path)>.
            *out << "struct " << factory << "_handles\n{\n";
            *out << "    ui::view_ref<maui::controls::content_page> root;\n";
            for (const auto& [name, type] : state.handles)
            {
                *out << "    ui::weak_ref<" << type << "> " << name << ";\n";
            }
            if (state.has_bindings)
            {
                *out << "    std::vector<maui::core::binding_handle> bindings;\n";
            }
            *out << "};\n\n";
            if (state.has_bindings)
            {
                *out << "template <class VM>\n";
                *out << "[[nodiscard]] inline " << factory << "_handles " << factory << "(VM& vm)\n";
            }
            else
            {
                *out << "[[nodiscard]] inline " << factory << "_handles " << factory << "()\n";
            }
            *out << "{\n    " << factory << "_handles handles;\n";
            *out << state.decls;
            *out << "    handles.root = " << tree << ";\n";
            *out << "    return handles;\n}\n";
        }
        return 0;
    }
} // namespace

int main(int argc, char** argv)
{
    try
    {
        const std::span<char*> args(argv, static_cast<std::size_t>(argc));
        if (args.size() < 2)
        {
            std::cerr << "usage: maui_xaml_codegen <input.xaml> [factory_name] [output.hpp]\n";
            return 2;
        }
        return run(args);
    }
    catch (const std::exception& error)
    {
        std::cerr << "maui_xaml_codegen: " << error.what() << "\n";
        return 1;
    }
}
