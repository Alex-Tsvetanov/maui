#pragma once
// maui::xaml — the XAML node tree  <=  Microsoft.Maui.Controls.Xaml (XamlNode.cs)
//
// Cluster header (PROFILE.md §3 cluster rule). The C# original keeps the whole node family in ONE
// file — INode / IValueNode / IListNode / BaseNode / ValueNode / MarkupNode / ElementNode / RootNode /
// ListNode plus the XmlName property-lookup extensions — and the family is meaningless apart, so the
// port mirrors that as one header. The tightly coupled value types XmlName.cs / XmlType.cs, the xmlns
// URI constants (XamlParser.Namespaces.cs — hoisted here so xml_name/xml_type can reference them
// without depending on the parser), the namespace-resolver snapshot (the System.Xml
// IXmlNamespaceResolver role), and the visitor seam (XamlNodeVisitor.cs: IXamlNodeVisitor +
// TreeVisitingMode + the delegate adapter) complete the cluster.
//
// Ownership (PROFILE.md §8): the tree owns children via shared_ptr (properties + collection items);
// the Parent back-reference is a NON-owning raw pointer (assigned by a visitor pass, exactly like the
// C# `node.Parent = parentNode` visitor in XamlLoader.Visit), which breaks the cycle.
//
// Wave-1 scope notes (M7): ValueNode.Value is `object` in C# (visitors later replace it with
// converted instances); here it is a std::string — the hydration-era object payload arrives with the
// wave-2 visitors. ElementNode.NameScopeRef (INameScope) and IsOnPlatformDefaultValue (SourceGen)
// are deferred the same way. // TODO: verify against src/Controls/src/Xaml/XamlNode.cs when porting
// the wave-2 visitors.

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace maui::xaml
{
    // ---- xmlns URIs  <=  Microsoft.Maui.Controls.Xaml.XamlParser (XamlParser.Namespaces.cs) ----
    // (FormsUri is omitted: obsolete, migration-error-message-only.)
    inline constexpr std::string_view maui_global_uri = "http://schemas.microsoft.com/dotnet/maui/global";
    inline constexpr std::string_view default_implicit_uri = maui_global_uri;
    inline constexpr std::string_view maui_uri = "http://schemas.microsoft.com/dotnet/2021/maui";
    inline constexpr std::string_view maui_design_uri = "http://schemas.microsoft.com/dotnet/2021/maui/design";
    inline constexpr std::string_view x2006_uri = "http://schemas.microsoft.com/winfx/2006/xaml";
    inline constexpr std::string_view x2009_uri = "http://schemas.microsoft.com/winfx/2009/xaml";
    inline constexpr std::string_view mc_uri = "http://schemas.openxmlformats.org/markup-compatibility/2006";

    // ---- xml_name  <=  Microsoft.Maui.Controls.Xaml.XmlName ----
    // A (namespace-URI, local-name) pair. The x:* directive names use the literal pseudo-namespace
    // "x" (and "_" for the framework-internal CreateContent), exactly like the C# statics.
    struct xml_name
    {
        std::string namespace_uri;
        std::string local_name;

        friend bool operator==(const xml_name&, const xml_name&) = default;

        [[nodiscard]] static const xml_name& create_content();   // XmlName._CreateContent ("_", "CreateContent")
        [[nodiscard]] static const xml_name& x_arguments();      // XmlName.xArguments ("x", "Arguments")
        [[nodiscard]] static const xml_name& x_class();          // XmlName.xClass
        [[nodiscard]] static const xml_name& x_class_modifier(); // XmlName.xClassModifier
        [[nodiscard]] static const xml_name& x_data_type();      // XmlName.xDataType
        [[nodiscard]] static const xml_name& x_factory_method(); // XmlName.xFactoryMethod
        [[nodiscard]] static const xml_name& x_field_modifier(); // XmlName.xFieldModifier
        [[nodiscard]] static const xml_name& x_key();            // XmlName.xKey
        [[nodiscard]] static const xml_name& x_name();           // XmlName.xName
        [[nodiscard]] static const xml_name& x_type_arguments(); // XmlName.xTypeArguments
        [[nodiscard]] static const xml_name& mc_ignorable();     // XmlName.mcIgnorable (McUri, "Ignorable")
        [[nodiscard]] static const xml_name& empty();            // XmlName.Empty
    };

    // ---- xml_type  <=  Microsoft.Maui.Controls.Xaml.XmlType ----
    // The textual type reference of an element: namespace URI + name + optional x:TypeArguments.
    // C#'s `IList<XmlType> TypeArguments` is null when absent; here an empty vector means absent.
    class xml_type
    {
    public:
        xml_type(std::string namespace_uri, std::string name, std::vector<xml_type> type_arguments = {});

        [[nodiscard]] const std::string& namespace_uri() const
        {
            return namespace_uri_;
        }
        [[nodiscard]] const std::string& name() const
        {
            return name_;
        }
        [[nodiscard]] const std::vector<xml_type>& type_arguments() const
        {
            return type_arguments_;
        }

        // XmlType.IsOfAnyType: name matches AND the namespace is one of the maui xmlns.
        [[nodiscard]] bool is_of_any_type(std::initializer_list<std::string_view> types) const;

        friend bool operator==(const xml_type& a, const xml_type& b);
        friend bool operator!=(const xml_type& a, const xml_type& b);

    private:
        std::string namespace_uri_;
        std::string name_;
        std::vector<xml_type> type_arguments_;
    };

    // ---- xml_namespace_resolver  <=  the System.Xml.IXmlNamespaceResolver role ----
    // C# nodes capture the live XmlReader as their resolver; the prefixes a node can resolve are the
    // ones in scope where it appeared (in practice: the root's declarations). The port captures that
    // as an immutable per-scope SNAPSHOT of prefix → URI, shared (shared_ptr) by every node of the
    // scope; elements that declare xmlns extend their parent's snapshot. Seeded with the standard
    // "xml"/"xmlns" bindings like XmlNamespaceManager.
    class xml_namespace_resolver
    {
    public:
        // The built-in root scope: xml → XML namespace, xmlns → xmlns namespace.
        [[nodiscard]] static std::shared_ptr<const xml_namespace_resolver> built_in();

        // A child scope: this scope's bindings plus (overriding with) the given declarations.
        [[nodiscard]] std::shared_ptr<const xml_namespace_resolver> extend(
            const std::vector<std::pair<std::string, std::string>>& declarations) const;

        // IXmlNamespaceResolver.LookupNamespace: URI for a prefix ("" = the default xmlns); nullopt
        // when undeclared (C# returns null).
        [[nodiscard]] std::optional<std::string> lookup_namespace(std::string_view prefix) const;
        // IXmlNamespaceResolver.LookupPrefix: a prefix mapped to the URI; nullopt when none.
        [[nodiscard]] std::optional<std::string> lookup_prefix(std::string_view namespace_uri) const;

    private:
        std::map<std::string, std::string, std::less<>> by_prefix_;
    };

    class value_node;
    class markup_node;
    class element_node;
    class root_node;
    class list_node;
    class i_xaml_node;

    // ---- tree_visiting_mode  <=  Microsoft.Maui.Controls.Xaml.TreeVisitingMode ----
    enum class tree_visiting_mode
    {
        top_down,
        bottom_up
    };

    // ---- i_xaml_node_visitor  <=  Microsoft.Maui.Controls.Xaml.IXamlNodeVisitor ----
    // The visitor seam. Concrete visitors (apply-properties, expand-markups, …) are wave 2; the
    // ordering knobs and the per-node-type dispatch are fixed here.
    class i_xaml_node_visitor
    {
    public:
        virtual ~i_xaml_node_visitor() = default;

        [[nodiscard]] virtual tree_visiting_mode visiting_mode() const = 0;
        [[nodiscard]] virtual bool stop_on_data_template() const = 0;
        [[nodiscard]] virtual bool visit_node_on_data_template() const = 0;
        [[nodiscard]] virtual bool stop_on_resource_dictionary() const = 0;

        virtual void visit(value_node& node, i_xaml_node* parent_node) = 0;
        virtual void visit(markup_node& node, i_xaml_node* parent_node) = 0;
        virtual void visit(element_node& node, i_xaml_node* parent_node) = 0;
        virtual void visit(root_node& node, i_xaml_node* parent_node) = 0;
        virtual void visit(list_node& node, i_xaml_node* parent_node) = 0;
        [[nodiscard]] virtual bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) = 0;
        [[nodiscard]] virtual bool is_resource_dictionary(element_node& node) = 0;

    protected:
        i_xaml_node_visitor() = default;
        i_xaml_node_visitor(const i_xaml_node_visitor&) = default;
        i_xaml_node_visitor(i_xaml_node_visitor&&) = default;
        i_xaml_node_visitor& operator=(const i_xaml_node_visitor&) = default;
        i_xaml_node_visitor& operator=(i_xaml_node_visitor&&) = default;
    };

    // ---- i_xaml_node  <=  Microsoft.Maui.Controls.Xaml.INode + BaseNode ----
    // The C# pair (interface + the abstract base every node derives) is collapsed into one abstract
    // class: every INode implementation IS a BaseNode. Carries the IXmlLineInfo surface too
    // (line/position are 1-based; -1 = no info).
    class i_xaml_node
    {
    public:
        virtual ~i_xaml_node() = default;
        i_xaml_node(const i_xaml_node&) = delete;
        i_xaml_node(i_xaml_node&&) = delete;
        i_xaml_node& operator=(const i_xaml_node&) = delete;
        i_xaml_node& operator=(i_xaml_node&&) = delete;

        // INode.IgnorablePrefixes (C#: List<string>, null until assigned — empty here means unset).
        [[nodiscard]] std::vector<std::string>& ignorable_prefixes()
        {
            return ignorable_prefixes_;
        }
        [[nodiscard]] const std::vector<std::string>& ignorable_prefixes() const
        {
            return ignorable_prefixes_;
        }

        // INode.NamespaceResolver
        [[nodiscard]] const std::shared_ptr<const xml_namespace_resolver>& namespace_resolver() const
        {
            return namespace_resolver_;
        }

        // INode.Parent — non-owning back-reference (see the header note).
        [[nodiscard]] i_xaml_node* parent() const
        {
            return parent_;
        }
        void set_parent(i_xaml_node* value)
        {
            parent_ = value;
        }

        // IXmlLineInfo (BaseNode.LineNumber / LinePosition / HasLineInfo)
        [[nodiscard]] int line_number() const
        {
            return line_number_;
        }
        void set_line_number(int value)
        {
            line_number_ = value;
        }
        [[nodiscard]] int line_position() const
        {
            return line_position_;
        }
        void set_line_position(int value)
        {
            line_position_ = value;
        }
        [[nodiscard]] bool has_line_info() const
        {
            return line_number_ >= 0 && line_position_ >= 0;
        }

        virtual void accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) = 0;
        [[nodiscard]] virtual std::shared_ptr<i_xaml_node> clone() const = 0;

    protected:
        explicit i_xaml_node(std::shared_ptr<const xml_namespace_resolver> namespace_resolver, int line_number = -1,
                             int line_position = -1)
            : namespace_resolver_(std::move(namespace_resolver)), line_number_(line_number),
              line_position_(line_position)
        {
        }

    private:
        std::vector<std::string> ignorable_prefixes_;
        std::shared_ptr<const xml_namespace_resolver> namespace_resolver_;
        i_xaml_node* parent_ = nullptr;
        int line_number_ = -1;
        int line_position_ = -1;
    };

    // INodeExtensions.SkipPrefix: is `prefix` ignorable on this node or any ancestor?
    [[nodiscard]] bool skip_prefix(const i_xaml_node& node, std::string_view prefix);

    // ---- i_value_node  <=  Microsoft.Maui.Controls.Xaml.IValueNode ----
    // Marker interface (value_node, markup_node and element_node carry it). Unlike C#'s
    // `IValueNode : INode`, it does NOT inherit i_xaml_node — that would force virtual inheritance;
    // cross-cast with dynamic_cast instead.
    class i_value_node
    {
    public:
        virtual ~i_value_node() = default;

    protected:
        i_value_node() = default;
        i_value_node(const i_value_node&) = default;
        i_value_node(i_value_node&&) = default;
        i_value_node& operator=(const i_value_node&) = default;
        i_value_node& operator=(i_value_node&&) = default;
    };

    // ---- i_list_node  <=  Microsoft.Maui.Controls.Xaml.IListNode ----
    // The CollectionItems carrier (element_node and list_node). Same non-inheriting mixin shape as
    // i_value_node.
    class i_list_node
    {
    public:
        virtual ~i_list_node() = default;

        [[nodiscard]] virtual std::vector<std::shared_ptr<i_xaml_node>>& collection_items() = 0;

    protected:
        i_list_node() = default;
        i_list_node(const i_list_node&) = default;
        i_list_node(i_list_node&&) = default;
        i_list_node& operator=(const i_list_node&) = default;
        i_list_node& operator=(i_list_node&&) = default;
    };

    // ---- xml_name_map  <=  Dictionary<XmlName, INode> + XmlNameExtensions ----
    // The Properties store. C#'s Dictionary iterates in insertion order in practice and the visitors
    // rely on document order, so the port makes that order explicit (vector of pairs; property
    // counts are small, lookups are linear).
    class xml_name_map
    {
    public:
        using value_type = std::pair<xml_name, std::shared_ptr<i_xaml_node>>;
        using const_iterator = std::vector<value_type>::const_iterator;
        using iterator = std::vector<value_type>::iterator;

        // Dictionary.Add: throws std::invalid_argument on a duplicate key (ArgumentException).
        void add(xml_name name, std::shared_ptr<i_xaml_node> node);
        [[nodiscard]] bool contains(const xml_name& name) const;
        // Dictionary indexer / TryGetValue by full name.
        [[nodiscard]] std::shared_ptr<i_xaml_node> try_get(const xml_name& name) const;
        // XmlNameExtensions.TryGetValue: exact ("", local) key first, then the first key whose
        // LocalName matches; returns the matched name through `matched`.
        [[nodiscard]] std::shared_ptr<i_xaml_node> try_get(std::string_view local_name, xml_name& matched) const;
        // Dictionary.Remove.
        bool remove(const xml_name& name);

        [[nodiscard]] std::size_t size() const
        {
            return entries_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return entries_.empty();
        }
        [[nodiscard]] iterator begin()
        {
            return entries_.begin();
        }
        [[nodiscard]] iterator end()
        {
            return entries_.end();
        }
        [[nodiscard]] const_iterator begin() const
        {
            return entries_.begin();
        }
        [[nodiscard]] const_iterator end() const
        {
            return entries_.end();
        }

    private:
        std::vector<value_type> entries_;
    };

    // ---- value_node  <=  Microsoft.Maui.Controls.Xaml.ValueNode ----
    class value_node final : public i_xaml_node, public i_value_node
    {
    public:
        value_node(std::string value, std::shared_ptr<const xml_namespace_resolver> namespace_resolver,
                   int line_number = -1, int line_position = -1, bool is_escaped = false)
            : i_xaml_node(std::move(namespace_resolver), line_number, line_position), value_(std::move(value)),
              is_escaped_(is_escaped)
        {
        }

        [[nodiscard]] const std::string& value() const
        {
            return value_;
        }
        void set_value(std::string value)
        {
            value_ = std::move(value);
        }

        // ValueNode.IsEscaped: created from an escaped markup string ("{}{Foo}") — treat as literal.
        [[nodiscard]] bool is_escaped() const
        {
            return is_escaped_;
        }

        void accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) override;
        [[nodiscard]] std::shared_ptr<i_xaml_node> clone() const override;

    private:
        std::string value_;
        bool is_escaped_ = false;
    };

    // ---- markup_node  <=  Microsoft.Maui.Controls.Xaml.MarkupNode ----
    // An UNPARSED "{…}" attribute value; the wave-2 expand-markups visitor tokenizes it.
    class markup_node final : public i_xaml_node, public i_value_node
    {
    public:
        markup_node(std::string markup_string, std::shared_ptr<const xml_namespace_resolver> namespace_resolver,
                    int line_number = -1, int line_position = -1)
            : i_xaml_node(std::move(namespace_resolver), line_number, line_position),
              markup_string_(std::move(markup_string))
        {
        }

        [[nodiscard]] const std::string& markup_string() const
        {
            return markup_string_;
        }

        void accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) override;
        [[nodiscard]] std::shared_ptr<i_xaml_node> clone() const override;

    private:
        std::string markup_string_;
    };

    // ---- element_node  <=  Microsoft.Maui.Controls.Xaml.ElementNode ----
    class element_node : public i_xaml_node, public i_value_node, public i_list_node
    {
    public:
        element_node(xml_type type, std::string namespace_uri,
                     std::shared_ptr<const xml_namespace_resolver> namespace_resolver, int line_number = -1,
                     int line_position = -1)
            : i_xaml_node(std::move(namespace_resolver), line_number, line_position), xml_type_(std::move(type)),
              namespace_uri_(std::move(namespace_uri))
        {
        }

        [[nodiscard]] xml_name_map& properties()
        {
            return properties_;
        }
        [[nodiscard]] const xml_name_map& properties() const
        {
            return properties_;
        }
        [[nodiscard]] std::vector<xml_name>& skip_properties()
        {
            return skip_properties_;
        }
        [[nodiscard]] std::vector<std::shared_ptr<i_xaml_node>>& collection_items() override
        {
            return collection_items_;
        }
        [[nodiscard]] const xml_type& type() const // ElementNode.XmlType
        {
            return xml_type_;
        }
        [[nodiscard]] const std::string& namespace_uri() const // ElementNode.NamespaceURI
        {
            return namespace_uri_;
        }

        void accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) override;
        [[nodiscard]] std::shared_ptr<i_xaml_node> clone() const override;

    protected:
        // ElementNode.SkipChildren / SkipVisitNode — the data-template / resource-dictionary stops.
        // (Non-const: skip_children hands *this to the visitor's is_resource_dictionary.)
        [[nodiscard]] bool skip_children(i_xaml_node_visitor& visitor, i_xaml_node& node, i_xaml_node* parent_node);
        [[nodiscard]] bool skip_visit_node(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) const;

        // The `visitor.Visit(this, parentNode)` dispatch — static in C# (BaseNode.Accept is
        // re-implemented per type); virtual here so root_node redirects to visit(root_node&)
        // without duplicating the traversal skeleton.
        virtual void visit_self(i_xaml_node_visitor& visitor, i_xaml_node* parent_node);

    private:
        // ElementNode.IsDataTemplate: this node is the parent's _CreateContent property value.
        [[nodiscard]] bool is_data_template(i_xaml_node* parent_node) const;

        xml_name_map properties_;
        std::vector<xml_name> skip_properties_;
        std::vector<std::shared_ptr<i_xaml_node>> collection_items_;
        xml_type xml_type_;
        std::string namespace_uri_;
    };

    // ---- root_node  <=  Microsoft.Maui.Controls.Xaml.RootNode ----
    // Concrete here (C# is abstract with XamlLoader.RuntimeRootNode adding the hydration Root
    // object — wave 2). Carries the parse warnings the C# parser collects on the root.
    class root_node : public element_node
    {
    public:
        struct warning
        {
            std::string message;
            int line_number = -1;
            int line_position = -1;
        };

        root_node(const xml_type& type, std::shared_ptr<const xml_namespace_resolver> namespace_resolver,
                  int line_number = -1, int line_position = -1);

        [[nodiscard]] std::vector<warning>& warnings()
        {
            return warnings_;
        }
        [[nodiscard]] const std::vector<warning>& warnings() const
        {
            return warnings_;
        }

        // NOTE: clone() is inherited from element_node — like C#, where RootNode has no Clone
        // override, cloning a root yields a plain element copy (warnings are not cloned).

    protected:
        void visit_self(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) override;

    private:
        std::vector<warning> warnings_;
    };

    // ---- list_node  <=  Microsoft.Maui.Controls.Xaml.ListNode ----
    // The multi-child value of a property element (e.g. several elements under <Grid.RowDefinitions>).
    class list_node final : public i_xaml_node, public i_value_node, public i_list_node
    {
    public:
        list_node(std::vector<std::shared_ptr<i_xaml_node>> nodes,
                  std::shared_ptr<const xml_namespace_resolver> namespace_resolver, int line_number = -1,
                  int line_position = -1)
            : i_xaml_node(std::move(namespace_resolver), line_number, line_position),
              collection_items_(std::move(nodes))
        {
        }

        [[nodiscard]] const xml_name& name() const // ListNode.XmlName
        {
            return xml_name_;
        }
        void set_name(xml_name value)
        {
            xml_name_ = std::move(value);
        }
        [[nodiscard]] std::vector<std::shared_ptr<i_xaml_node>>& collection_items() override
        {
            return collection_items_;
        }

        void accept(i_xaml_node_visitor& visitor, i_xaml_node* parent_node) override;
        // NOTE: like C#, the clone copies the items but NOT XmlName.
        [[nodiscard]] std::shared_ptr<i_xaml_node> clone() const override;

    private:
        xml_name xml_name_;
        std::vector<std::shared_ptr<i_xaml_node>> collection_items_;
    };

    // ---- xaml_node_visitor  <=  Microsoft.Maui.Controls.Xaml.XamlNodeVisitor ----
    // The delegate-based adapter: every Visit overload forwards to one action. StopOnResourceDictionary
    // is always false; SkipChildren/IsResourceDictionary always answer false, as in C#.
    class xaml_node_visitor final : public i_xaml_node_visitor
    {
    public:
        using action_type = std::function<void(i_xaml_node&, i_xaml_node*)>;

        explicit xaml_node_visitor(action_type action, tree_visiting_mode visiting_mode = tree_visiting_mode::top_down,
                                   bool stop_on_data_template = false, bool visit_node_on_data_template = true)
            : action_(std::move(action)), visiting_mode_(visiting_mode), stop_on_data_template_(stop_on_data_template),
              visit_node_on_data_template_(visit_node_on_data_template)
        {
        }

        [[nodiscard]] tree_visiting_mode visiting_mode() const override
        {
            return visiting_mode_;
        }
        [[nodiscard]] bool stop_on_data_template() const override
        {
            return stop_on_data_template_;
        }
        [[nodiscard]] bool visit_node_on_data_template() const override
        {
            return visit_node_on_data_template_;
        }
        [[nodiscard]] bool stop_on_resource_dictionary() const override
        {
            return false;
        }

        void visit(value_node& node, i_xaml_node* parent_node) override;
        void visit(markup_node& node, i_xaml_node* parent_node) override;
        void visit(element_node& node, i_xaml_node* parent_node) override;
        void visit(root_node& node, i_xaml_node* parent_node) override;
        void visit(list_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool skip_children(i_xaml_node& node, i_xaml_node* parent_node) override;
        [[nodiscard]] bool is_resource_dictionary(element_node& node) override;

    private:
        action_type action_;
        tree_visiting_mode visiting_mode_;
        bool stop_on_data_template_;
        bool visit_node_on_data_template_;
    };
} // namespace maui::xaml
