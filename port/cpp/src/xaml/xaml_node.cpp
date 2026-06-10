// maui::xaml node tree  <=  src/Controls/src/Xaml/XamlNode.cs (+ XmlName.cs, XmlType.cs,
// XamlNodeVisitor.cs). See xaml_node.hpp for the cluster layout and wave-1 scope notes.
#include "maui/xaml/xaml_node.hpp"

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace maui::xaml
{
    // ---- xml_name statics (XmlName.cs) ----
    const xml_name& xml_name::create_content()
    {
        static const xml_name name{.namespace_uri = "_", .local_name = "CreateContent"};
        return name;
    }
    const xml_name& xml_name::x_arguments()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "Arguments"};
        return name;
    }
    const xml_name& xml_name::x_class()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "Class"};
        return name;
    }
    const xml_name& xml_name::x_class_modifier()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "ClassModifier"};
        return name;
    }
    const xml_name& xml_name::x_data_type()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "DataType"};
        return name;
    }
    const xml_name& xml_name::x_factory_method()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "FactoryMethod"};
        return name;
    }
    const xml_name& xml_name::x_field_modifier()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "FieldModifier"};
        return name;
    }
    const xml_name& xml_name::x_key()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "Key"};
        return name;
    }
    const xml_name& xml_name::x_name()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "Name"};
        return name;
    }
    const xml_name& xml_name::x_type_arguments()
    {
        static const xml_name name{.namespace_uri = "x", .local_name = "TypeArguments"};
        return name;
    }
    const xml_name& xml_name::mc_ignorable()
    {
        static const xml_name name{.namespace_uri = std::string(mc_uri), .local_name = "Ignorable"};
        return name;
    }
    const xml_name& xml_name::empty()
    {
        static const xml_name name{};
        return name;
    }

    // ---- xml_type (XmlType.cs) ----
    xml_type::xml_type(std::string namespace_uri, std::string name, std::vector<xml_type> type_arguments)
        : namespace_uri_(std::move(namespace_uri)), name_(std::move(name)), type_arguments_(std::move(type_arguments))
    {
    }

    bool xml_type::is_of_any_type(std::initializer_list<std::string_view> types) const
    {
        if (namespace_uri_ != maui_uri && namespace_uri_ != maui_global_uri)
        {
            return false;
        }
        return std::ranges::any_of(types, [this](std::string_view type) { return type == name_; });
    }

    bool operator==(const xml_type& a, const xml_type& b)
    {
        return a.namespace_uri_ == b.namespace_uri_ && a.name_ == b.name_ && a.type_arguments_ == b.type_arguments_;
    }
    bool operator!=(const xml_type& a, const xml_type& b)
    {
        return !(a == b);
    }

    // ---- xml_namespace_resolver ----
    std::shared_ptr<const xml_namespace_resolver> xml_namespace_resolver::built_in()
    {
        auto resolver = std::make_shared<xml_namespace_resolver>();
        resolver->by_prefix_.emplace("xml", "http://www.w3.org/XML/1998/namespace");
        resolver->by_prefix_.emplace("xmlns", "http://www.w3.org/2000/xmlns/");
        // XmlNamespaceManager pre-binds the EMPTY prefix to the empty URI, so LookupNamespace("")
        // yields "" (never null) when no default xmlns is declared — mirrored for fidelity (the
        // TypeArgumentsParser relies on it for unprefixed type names).
        resolver->by_prefix_.emplace("", "");
        return resolver;
    }

    std::shared_ptr<const xml_namespace_resolver> xml_namespace_resolver::extend(
        const std::vector<std::pair<std::string, std::string>>& declarations) const
    {
        auto child = std::make_shared<xml_namespace_resolver>(*this);
        for (const auto& [prefix, uri] : declarations)
        {
            child->by_prefix_.insert_or_assign(prefix, uri);
        }
        return child;
    }

    std::optional<std::string> xml_namespace_resolver::lookup_namespace(std::string_view prefix) const
    {
        const auto found = by_prefix_.find(prefix);
        if (found == by_prefix_.end())
        {
            return std::nullopt;
        }
        return found->second;
    }

    std::optional<std::string> xml_namespace_resolver::lookup_prefix(std::string_view namespace_uri) const
    {
        for (const auto& [prefix, uri] : by_prefix_)
        {
            if (uri == namespace_uri)
            {
                return prefix;
            }
        }
        return std::nullopt;
    }

    // ---- skip_prefix (INodeExtensions.SkipPrefix) ----
    bool skip_prefix(const i_xaml_node& node, std::string_view prefix)
    {
        for (const i_xaml_node* current = &node; current != nullptr; current = current->parent())
        {
            const auto& prefixes = current->ignorable_prefixes();
            if (std::ranges::find(prefixes, prefix) != prefixes.end())
            {
                return true;
            }
        }
        return false;
    }

    // ---- xml_name_map (Dictionary<XmlName, INode> + XmlNameExtensions) ----
    void xml_name_map::add(xml_name name, std::shared_ptr<i_xaml_node> node)
    {
        if (contains(name))
        {
            throw std::invalid_argument("An item with the same key has already been added: " + name.local_name);
        }
        entries_.emplace_back(std::move(name), std::move(node));
    }

    bool xml_name_map::contains(const xml_name& name) const
    {
        return std::ranges::any_of(entries_, [&name](const value_type& entry) { return entry.first == name; });
    }

    std::shared_ptr<i_xaml_node> xml_name_map::try_get(const xml_name& name) const
    {
        const auto found =
            std::ranges::find_if(entries_, [&name](const value_type& entry) { return entry.first == name; });
        return found != entries_.end() ? found->second : nullptr;
    }

    std::shared_ptr<i_xaml_node> xml_name_map::try_get(std::string_view local_name, xml_name& matched) const
    {
        // XmlNameExtensions.TryGetValue: the exact ("", name) key first…
        matched = xml_name{.namespace_uri = "", .local_name = std::string(local_name)};
        if (auto node = try_get(matched))
        {
            return node;
        }
        // …then the first entry whose LocalName matches.
        const auto found = std::ranges::find_if(
            entries_, [local_name](const value_type& entry) { return entry.first.local_name == local_name; });
        if (found == entries_.end())
        {
            return nullptr;
        }
        matched = found->first;
        return found->second;
    }

    bool xml_name_map::remove(const xml_name& name)
    {
        const auto found =
            std::ranges::find_if(entries_, [&name](const value_type& entry) { return entry.first == name; });
        if (found == entries_.end())
        {
            return false;
        }
        entries_.erase(found);
        return true;
    }

    // ---- value_node (ValueNode) ----
    void value_node::accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node)
    {
        visitor.visit(*this, parent_node);
    }

    std::shared_ptr<i_xaml_node> value_node::clone() const
    {
        auto copy =
            std::make_shared<value_node>(value_, namespace_resolver(), line_number(), line_position(), is_escaped_);
        copy->ignorable_prefixes() = ignorable_prefixes();
        return copy;
    }

    // ---- markup_node (MarkupNode) ----
    void markup_node::accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node)
    {
        visitor.visit(*this, parent_node);
    }

    std::shared_ptr<i_xaml_node> markup_node::clone() const
    {
        auto copy = std::make_shared<markup_node>(markup_string_, namespace_resolver(), line_number(), line_position());
        copy->ignorable_prefixes() = ignorable_prefixes();
        return copy;
    }

    // ---- element_node (ElementNode) ----
    void element_node::accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node)
    {
        if (visitor.visiting_mode() == tree_visiting_mode::top_down && !skip_visit_node(visitor, parent_node))
        {
            visit_self(visitor, parent_node);
        }

        if (!skip_children(visitor, *this, parent_node))
        {
            // C# snapshots (Properties.Values.ToArray() / CollectionItems.ToArray()): visitors may
            // mutate the stores while the tree is walked.
            std::vector<std::shared_ptr<i_xaml_node>> property_values;
            property_values.reserve(properties_.size());
            for (const auto& [name, node] : properties_)
            {
                property_values.push_back(node);
            }
            for (const auto& node : property_values)
            {
                node->accept(visitor, this);
            }
            const std::vector<std::shared_ptr<i_xaml_node>> items = collection_items_;
            for (const auto& node : items)
            {
                node->accept(visitor, this);
            }
        }

        if (visitor.visiting_mode() == tree_visiting_mode::bottom_up && !skip_visit_node(visitor, parent_node))
        {
            visit_self(visitor, parent_node);
        }
    }

    void element_node::visit_self(i_xaml_node_visitor& visitor, i_xaml_node* parent_node)
    {
        visitor.visit(*this, parent_node);
    }

    bool element_node::is_data_template(i_xaml_node* parent_node) const
    {
        const auto* parent_element = dynamic_cast<element_node*>(parent_node);
        if (parent_element == nullptr)
        {
            return false;
        }
        const auto create_content = parent_element->properties().try_get(xml_name::create_content());
        return create_content != nullptr && create_content.get() == this;
    }

    bool element_node::skip_children(i_xaml_node_visitor& visitor, i_xaml_node& node, i_xaml_node* parent_node)
    {
        return (visitor.stop_on_data_template() && is_data_template(parent_node)) ||
               (visitor.stop_on_resource_dictionary() && visitor.is_resource_dictionary(*this)) ||
               visitor.skip_children(node, parent_node);
    }

    bool element_node::skip_visit_node(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) const
    {
        return !visitor.visit_node_on_data_template() && is_data_template(parent_node);
    }

    std::shared_ptr<i_xaml_node> element_node::clone() const
    {
        auto copy = std::make_shared<element_node>(xml_type_, namespace_uri_, namespace_resolver(), line_number(),
                                                   line_position());
        copy->ignorable_prefixes() = ignorable_prefixes();
        for (const auto& [name, node] : properties_)
        {
            copy->properties_.add(name, node->clone());
        }
        copy->skip_properties_ = skip_properties_;
        for (const auto& item : collection_items_)
        {
            copy->collection_items_.push_back(item->clone());
        }
        return copy;
    }

    // ---- root_node (RootNode) ----
    root_node::root_node(const xml_type& type, std::shared_ptr<const xml_namespace_resolver> namespace_resolver,
                         int line_number, int line_position)
        : element_node(type, type.namespace_uri(), std::move(namespace_resolver), line_number, line_position)
    {
    }

    void root_node::visit_self(i_xaml_node_visitor& visitor, i_xaml_node* parent_node)
    {
        visitor.visit(*this, parent_node);
    }

    // ---- list_node (ListNode) ----
    void list_node::accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node)
    {
        if (visitor.visiting_mode() == tree_visiting_mode::top_down)
        {
            visitor.visit(*this, parent_node);
        }
        // C# iterates CollectionItems directly (no snapshot) — mirrored.
        for (const auto& node : collection_items_)
        {
            node->accept(visitor, this);
        }
        if (visitor.visiting_mode() == tree_visiting_mode::bottom_up)
        {
            visitor.visit(*this, parent_node);
        }
    }

    std::shared_ptr<i_xaml_node> list_node::clone() const
    {
        std::vector<std::shared_ptr<i_xaml_node>> items;
        items.reserve(collection_items_.size());
        for (const auto& item : collection_items_)
        {
            items.push_back(item->clone());
        }
        auto copy = std::make_shared<list_node>(std::move(items), namespace_resolver(), line_number(), line_position());
        copy->ignorable_prefixes() = ignorable_prefixes();
        return copy;
    }

    // ---- xaml_node_visitor (XamlNodeVisitor) ----
    void xaml_node_visitor::visit(value_node& node, i_xaml_node* parent_node)
    {
        action_(node, parent_node);
    }
    void xaml_node_visitor::visit(markup_node& node, i_xaml_node* parent_node)
    {
        action_(node, parent_node);
    }
    void xaml_node_visitor::visit(element_node& node, i_xaml_node* parent_node)
    {
        action_(node, parent_node);
    }
    void xaml_node_visitor::visit(root_node& node, i_xaml_node* parent_node)
    {
        action_(node, parent_node);
    }
    void xaml_node_visitor::visit(list_node& node, i_xaml_node* parent_node)
    {
        action_(node, parent_node);
    }
    bool xaml_node_visitor::skip_children(i_xaml_node& /*node*/, i_xaml_node* /*parent_node*/)
    {
        return false;
    }
    bool xaml_node_visitor::is_resource_dictionary(element_node& /*node*/)
    {
        return false;
    }
} // namespace maui::xaml
