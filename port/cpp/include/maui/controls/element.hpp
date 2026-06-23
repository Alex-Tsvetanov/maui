#pragma once
// maui::controls::element  <=  Microsoft.Maui.Controls.Element / VisualElement (lifecycle subset)
//
// The non-template base between bindable_object (the value / binding-context layer) and view<> (the
// handler / geometry layer). It carries the cross-cutting *tree lifecycle* every control shares:
//   - logical-children visitation (the port's stand-in for Element.LogicalChildren) — a container
//     overrides for_each_logical_child to expose its children; leaves keep the no-op default;
//   - the logical PARENT back-reference (Element.Parent) — set when a container attaches a child, so the
//     resource lookup + implicit-style match can walk UP the tree;
//   - BindingContext INHERITANCE — on_binding_context_changed propagates this element's context down to
//     each logical child (the role of Element.OnBindingContextChanged → SetChildInheritedBindingContext);
//   - the Window back-reference + Loaded/Unloaded events (VisualElement.Window + Loaded/Unloaded) — when a
//     root is hosted in a window, set_containing_window flows the window down the subtree;
//   - RESOURCES + DynamicResource + implicit styles (M5d): a lazily-created resource_dictionary (the
//     IResourcesProvider role), try_get_resource walking self→ancestors (ResourcesExtensions.TryGetResource),
//     set/remove_dynamic_resource binding a property to a resource key (re-applied when the resource changes
//     or the element reparents), and a merged_style resolving the implicit (TargetType-keyed) + class styles.
// attach_logical_child / detach_logical_child are the hooks a container calls when it gains / loses a child
// so the child inherits (or loses) the parent's context + window + resources (Element.OnChildAdded/Removed).
//
// Scope (M5d): the propagation machinery + resources/DynamicResource/implicit styles. Effects, the visual-
// vs-logical split, platform loaded-event wiring, modal stacks, system resources, and style sheets are out
// of scope (STATUS.md).

#include <any>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits> // --- platform configuration (W2-24) ---
#include <unordered_map>
#include <utility> // --- platform configuration (W2-24) ---
#include <vector>

#include "maui/controls/effect_collection.hpp" // --- effects (G3): the unique_ptr<effect_collection> member ---
#include "maui/controls/merged_style.hpp"
#include "maui/controls/platform_configuration/configuration.hpp" // --- platform configuration (W2-24) ---
#include "maui/controls/resource_dictionary.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::xaml
{
    class name_scope; // forward — the x:Name registry attached to an element (Setter/Trigger TargetName)
} // namespace maui::xaml

namespace maui::controls
{
    class window;                    // forward — the host; element only holds a non-owning back-ref
    class binding_base;              // forward — runtime bindings (W1-02); element only stores shared_ptrs
    class effect;                    // forward — effects (G3); the element's effect collection owns these
    class i_effect_control_provider; // forward — effects (G3); the settable provider seam
    // --- templates (W1-09): forward declarations for the template-scope members below ---
    class template_binding;   // the templated-parent binding an element can carry (templates/)
    class template_utilities; // the ControlTemplate application machinery (friend — walks children)
    // --- end templates (W1-09) ---

    // --- animations (W1-14) ---
    // The per-element slice of C# AnimationExtensions' static animation tables (defined in
    // src/controls/detail/element_animations.hpp — internal, PROFILE §3).
    namespace detail
    {
        class element_animations;
    } // namespace detail
    // --- end animations (W1-14) ---

    class element : public maui::core::bindable_object
    {
    public:
        // --- animations (W1-14) ---
        // C# IAnimatable, folded into element: VisualElement.BatchBegin/BatchCommit nest a counter;
        // the last commit raises batch_committed (VisualElement.BatchCommitted). DEVIATION
        // (documented): the separate IAnimatable interface is not introduced because the hot-file
        // rule forbids editing this class's base list — the batch hooks live directly on element.
        void batch_begin();
        void batch_commit();
        [[nodiscard]] bool batched() const
        {
            return batched_ > 0;
        }
        maui::core::event<> batch_committed;
        // Internal seam for the animation extensions: the named-animation + kinetic registry of this
        // element (C# AnimationExtensions' s_animations/s_kinetics keyed by AnimatableKey(this, …)),
        // lazily created on first use.
        [[nodiscard]] detail::element_animations& animation_state();
        [[nodiscard]] bool has_animation_state() const
        {
            return animation_state_ != nullptr;
        }
        // --- end animations (W1-14) ---

        // ---- Window back-ref + Loaded/Unloaded (VisualElement.Window / Loaded / Unloaded) ----
        [[nodiscard]] window* containing_window() const
        {
            return window_;
        }
        maui::core::event<> loaded;
        maui::core::event<> unloaded;

        // C# Element.OnParentChangingCore — invoked on this element just before its logical parent changes
        // (attach_logical_child passes the new parent; detach passes nullptr). The default is a no-op;
        // ImmutableBrush overrides it to throw, so a named brush static refuses to be parented (the public-
        // facing analog of C#'s settable Element.Parent —
        // BrushTypeConverterUnitTests.InvalidOperationExceptionWhenSettingParentOnImmutableBrush). Public so
        // it is reachable both from attach_logical_child and as the developer-facing "set Parent" throw.
        virtual void on_logical_parent_changing(element* new_parent)
        {
            (void)new_parent;
        }

        // Attach (non-null) or detach (null) this element + its logical subtree to a window. A non-null
        // transition fires loaded (top-down: this element first, then children); a null transition fires
        // unloaded (bottom-up: children first, then this element). Idempotent — same window is a no-op.
        // Mirrors VisualElement.Window propagation + SendLoaded/SendUnloaded.
        void set_containing_window(window* value);

        // ---- Resources (IResourcesProvider.Resources / VisualElement.Resources) ----
        // The element's resource dictionary, lazily created on first access (IsResourcesCreated flips true).
        // Changing a value re-applies any bound DynamicResource on this element + its subtree and re-resolves
        // the merged (implicit/class) style. NON-owning toward merged dictionaries (the caller owns those).
        [[nodiscard]] resource_dictionary& resources();
        [[nodiscard]] bool is_resources_created() const
        {
            return resources_ != nullptr;
        }

        // The logical parent (Element.Parent) — set when a container attaches this as a child. The resource
        // lookup and implicit-style match walk up this chain. NON-owning (a child never owns its parent).
        [[nodiscard]] element* logical_parent() const
        {
            return logical_parent_;
        }

        // ---- generic mount surface (the hosting app_host driver) ----------------------------------------
        // The three public hooks a backend-agnostic mount driver needs to walk + host an arbitrary element
        // tree WITHOUT special-casing each control (maui/hosting/app_host.hpp). They are the public face of
        // the protected for_each_logical_child + the per-container host command.
        //
        // (1) visit_logical_children — the public window onto for_each_logical_child (which stays protected
        //     so only containers OVERRIDE it). The driver enumerates a node's direct logical children here.
        void visit_logical_children(const std::function<void(element&)>& visit) const
        {
            for_each_logical_child(visit);
        }

        // (2) handler_type_tag — the CONCRETE control's type_tag the handler_registry keys its factory on
        //     (handler_registry::create_handler(type_tag)). attach_handler<View> keys on the STATIC type;
        //     a generic driver only holds element& (static type element), so it needs the RUNTIME tag. The
        //     reflection-free port has the control declare it; it defaults to the implicit-style target type
        //     (set_style_target_type<T>(), which every registered control already populates with
        //     type_tag::of<T>() — the same concrete registered type). A control that registers a handler but
        //     has NO implicit style (the lone case today: collection_view) calls set_handler_type_tag<T>()
        //     instead. nullopt = no handler to mount (a leaf chrome element / a control with neither set).
        [[nodiscard]] virtual std::optional<maui::core::type_tag> handler_type_tag() const
        {
            return handler_type_tag_.has_value() ? handler_type_tag_ : style_target_type_;
        }

        // (3) mount_into_handler — re-fire this container's host command so its (now-attached) handler hosts
        //     its logical children's native views. The element tree is typically built in a control's
        //     CONSTRUCTOR, BEFORE any handler exists, so the content/add command fired then found no handler
        //     and hosted nothing; once the driver attaches handlers bottom-up it calls this to replay the
        //     host command (the generic form of the gallery_rehost_* helpers). Default no-op — a leaf has
        //     nothing to host. A container overrides it to invoke its own command ("set_content" / "add" /
        //     the multi-page child-sync) on handler(). Called AFTER this element's own handler is attached
        //     and after all its children's handlers are attached (post-order), so every child native view
        //     exists when the parent hosts it.
        virtual void mount_into_handler()
        {
        }

        // C# VisualElement IsEnabled cascade (IPropertyPropagationController.PropagatePropertyChanged for
        // IsEnabledProperty / RefreshIsEnabledProperty): an ANCESTOR's IsEnabled changed, so re-push THIS
        // element's now-recomputed effective (coerced) IsEnabled to its handler + re-run ChangeVisualState,
        // then recurse into the logical children so the whole subtree re-coerces. Default no-op — a non-view
        // element has no IsEnabled (the cascade stops at it, exactly as a non-VisualElement Parent breaks
        // C#'s chain). Public so a parent's propagation can invoke it through a base element& reference;
        // view<> overrides it (the actual coerce + handler re-push live there).
        virtual void refresh_is_enabled_subtree()
        {
        }

        // Resolve `key` against this element's resources then each ancestor's (ResourcesExtensions
        // .TryGetResource). Returns the borrowed stored value or nullptr. The reference is valid until the
        // owning dictionary mutates.
        [[nodiscard]] const std::any* try_get_resource(std::string_view key) const;

        // Bind the property named `name` to the resource `key` (BindableObject.SetDynamicResource): resolve
        // it now (if found) at setter_specificity::dynamic_resource_setter, and re-apply whenever that
        // resource changes or this element reparents. Idempotent re-binding updates the key.
        void set_dynamic_resource(std::string name, std::string key);
        // Stop tracking the DynamicResource on `name` (BindableObject.RemoveDynamicResource). The already-set
        // value is kept (C# only stops future updates).
        void remove_dynamic_resource(std::string_view name);

        // ---- NameScope (x:Name registry) — NameScope.SetNameScope / GetNameScope + Element.FindByName ----
        // The C# model attaches an INameScope to a BindableObject through NameScope.NameScopeProperty and has
        // Element.GetNameScope walk RealParent for the nearest one. The reflection-free port has no central
        // attached-property bag (PROFILE §7 removed it), so the scope is a first-class element slot here —
        // this is the element-side scope the name_scope.hpp placement note deferred. NON-owning toward the
        // CONTROLS it registers in the usual sense (the scope's shared_ptr<bindable_object> entries co-own the
        // element tree, exactly like C#'s GC-rooted dictionary); the element owns its scope shared_ptr (a
        // nested ControlTemplate scope and its outer scope are distinct objects, one per element that declares
        // x:Name children — see name_scope_ref for the shared-ref case during a XAML load).
        //
        // NameScope.SetNameScope: attach `value` as THIS element's scope. C# only sets it when none is
        // attached yet (GetValue(NameScopeProperty) == null) — the port mirrors that: a second set is ignored.
        //
        // OWNERSHIP CAVEAT (documented): a scope's shared_ptr<bindable_object> entries co-own the registered
        // elements. The intended shape — a parent's scope registers its DESCENDANTS, and resolution walks UP
        // (logical_parent_ is a raw back-ref) — is an acyclic graph. Registering an element in ITS OWN scope
        // (or an ancestor's, where the ancestor also owns that scope) would form a shared_ptr cycle; C# is
        // immune via GC but the port is not. The code-first Setter/Trigger TargetName path here only registers
        // descendants, so no cycle arises. The XAML loader (M7), which is where root self-registration
        // happens, still keeps its scopes on the load nodes/result (name_scope.hpp), not on elements — so this
        // element-side slot does not change that path. If a future loader integration attaches scopes to
        // elements, self/ancestor registrations must hold a weak entry to break the cycle.
        void set_name_scope(std::shared_ptr<maui::xaml::name_scope> value);

        // Element.GetNameScope: the nearest attached scope walking THIS element then up the logical-parent
        // chain (C# walks RealParent). nullptr when no element in the chain carries a scope (C# returns null).
        // Borrowed — valid while the owning element + its scope live.
        [[nodiscard]] maui::xaml::name_scope* get_name_scope() const;

        // Element.FindByName: resolve `name` against this element's effective namescope (GetNameScope) and
        // return the registered object as a bindable_object*, or nullptr when there is no scope in the chain
        // or the name is unregistered / not a control (C#'s FindByName returns null; the cast narrows it).
        // Borrowed (the scope / element tree owns the target).
        [[nodiscard]] maui::core::bindable_object* find_by_name(std::string_view name) const;

        // Setter.FindTargetByName: the named-target resolution a Setter/Trigger TargetName uses. Try the
        // standard FindByName first (own or inherited scope), then walk the logical-parent chain consulting
        // EACH ancestor's own GetNameScope — this reaches a name registered in an OUTER scope when the
        // element sits inside a nested ControlTemplate namescope (Maui3793). nullptr when unresolvable.
        [[nodiscard]] maui::core::bindable_object* find_target_by_name(std::string_view name) const;

    protected:
        // Out-of-line (= default in element_templates.cpp): the inline form would instantiate the
        // unwind destructor of the forward-declared template_bindings_ vector. --- templates (W1-09) ---
        element();

        // Visit each direct logical child (default: none — a leaf). Containers override to expose theirs.
        // Every control is-a element, so propagation hands back element& and needs no cast at the call site.
        virtual void for_each_logical_child(const std::function<void(element&)>& visit) const
        {
            (void)visit;
        }

        // A container calls these when a child is added / removed so the child inherits (or loses) this
        // element's binding context + window + resources immediately (Element.OnChildAdded / OnChildRemoved).
        // detach is static — it only operates on `child` (clear its window + parent, then re-resolve its now
        // shorter resource chain); attach needs `this` (to flow this element's context/window/resources down).
        void attach_logical_child(element& child);
        static void detach_logical_child(element& child);

        // bindable_object::on_binding_context_changed override: raise the event (base) then propagate the
        // new context to every logical child as an inherited context.
        void on_binding_context_changed() override;

        // Declare this element's style target type (the implicit-style key + the type a style targets). A
        // control calls this in its constructor — `set_style_target_type<button>()` — so resources keyed by
        // type_tag::of<button>() match it. C# derives this from the runtime type; the reflection-free port
        // has the control declare it explicitly. Until set, the element has no implicit style.
        template <class TControl> void set_style_target_type()
        {
            style_target_type_ = maui::core::type_tag::of<TControl>();
            merged_style_.set_target_type(*style_target_type_);
        }

        // Declare the CONCRETE control type the handler_registry keys this control's handler factory on, for
        // the generic mount driver's runtime resolution (handler_type_tag above). A control whose
        // set_style_target_type already supplies the same concrete tag needs NOT call this — the default
        // handler_type_tag() returns the style target. Only a control that registers a handler but has no
        // implicit style (so style_target_type_ stays unset) calls this in its constructor. Stored
        // separately from the style target so the two stay semantically distinct.
        template <class TControl> void set_handler_type_tag()
        {
            handler_type_tag_ = maui::core::type_tag::of<TControl>();
        }

        // Apply the merged (implicit + class) style to this element (merged_style.apply). Called by the view
        // when an explicit style is set/cleared and by the resource/parent machinery on a relevant change.
        // The explicit (local) style is owned by view<>; merged_style only handles implicit + class styles.
        void refresh_merged_style();
        // Tell the merged_style which class names this control selects (VisualElement.StyleClass) and which
        // local style is set, so it can layer implicit < class < local correctly. Called by view<>.
        void set_merged_style_classes(std::vector<std::string> classes);

        // The element's resource chain changed (a resource value changed, or it reparented). view<> overrides
        // this to re-resolve a LOCAL style's base_resource_key against the new chain (the local style is owned
        // by view<>, so element can't touch it directly). Called AFTER the merged-style refresh. Default no-op.
        virtual void on_resource_chain_changed()
        {
        }

        merged_style merged_style_{*this}; // the implicit + class style resolver (MergedStyle)

    private:
        // Re-resolve every bound DynamicResource on THIS element against the current resource chain (used on
        // a reparent — the new ancestors may hold the key) and re-resolve the merged style, then recurse into
        // logical children (their chain changed too). Mirrors Element.OnParentResourcesChanged.
        void reapply_resources_from_chain();
        // THIS element's own resource dictionary changed: re-apply the affected DynamicResources + refresh
        // the merged style, then propagate the changed keys DOWN to logical children (Element
        // .OnResourcesChanged → each child). Down-propagation is why an element only subscribes to its OWN
        // dictionary — a change up the tree reaches a descendant by recursing through children.
        void on_resources_changed(const std::vector<resource_change>& values);
        // Re-apply the DynamicResources whose key is among `keys`, or ALL of them when `keys` is null,
        // resolving each against the current chain. Used by both paths above.
        void apply_dynamic_resources(const std::vector<resource_change>* keys);

        // INameScope.FindByName narrowed to a control (the shared_ptr<bindable_object> case): the stored
        // object as a bindable_object*, or null on a miss / a non-control payload. Shared by find_by_name +
        // find_target_by_name's parent walk.
        [[nodiscard]] static maui::core::bindable_object* resolve_in_scope(const maui::xaml::name_scope& scope,
                                                                           std::string_view name);

        window* window_ = nullptr;          // non-owning back-ref to the hosting window (VisualElement.Window)
        element* logical_parent_ = nullptr; // non-owning back-ref to the logical parent (Element.Parent)
        // The attached x:Name registry (NameScope.NameScopeProperty), null until set_name_scope. Held as a
        // shared_ptr because a XAML load shares one scope across the nodes of a scope (name_scope_ref); a
        // code-first control simply owns its own. Entries co-own the registered controls (see set_name_scope).
        std::shared_ptr<maui::xaml::name_scope> name_scope_;
        std::unique_ptr<resource_dictionary> resources_; // lazily created (IsResourcesCreated)
        maui::core::scoped_connection resources_token_;  // own-dictionary values_changed subscription
        // DynamicResource bindings: property-name → resource-key (Element.DynamicResources). Both strings are
        // owned (a name or key may be a built string).
        std::unordered_map<std::string, std::string> dynamic_resources_;
        std::optional<maui::core::type_tag> style_target_type_; // this control's style TargetType (if any)
        // The concrete handler-registry key for the generic mount driver (set_handler_type_tag). Distinct
        // from style_target_type_: a control with an implicit style needs only the style target; a control
        // that registers a handler but has no implicit style sets this. handler_type_tag() prefers it.
        std::optional<maui::core::type_tag> handler_type_tag_;

        // --- templates (W1-09) --------------------------------------------------------------------------
        // The Element-side templated-parent surface: Element.IsTemplateRoot, the synchronous
        // TemplateUtilities.FindTemplatedParentAsync walk, and the storage/re-application of
        // template_bindings (SetBinding(..., new TemplateBinding(...))). Bodies that need the complete
        // template_binding type live in src/controls/templates/element_templates.cpp; the destructor is
        // out-of-line for the same reason (the vector member of the forward-declared type).
    public:
        ~element() override; // out-of-line in element.cpp: unapplies the runtime bindings (W1-02)
                             // and needs template_binding complete for template_bindings_'s teardown

        // Element.IsTemplateRoot — set by element_template::create_content on the created root.
        [[nodiscard]] bool is_template_root() const
        {
            return is_template_root_;
        }
        void set_is_template_root(bool value)
        {
            is_template_root_ = value;
        }

        // TemplateUtilities.FindTemplatedParentAsync, synchronous: walk the logical-parent chain for the
        // nearest i_control_templated ancestor with a non-null ControlTemplate; each content_presenter
        // passed on the way up skips one templated ancestor (a presenter's content belongs to the OUTER
        // scope). C# awaits a pending ParentSet instead — the port re-resolves on every reparent (the
        // reapply_template_bindings hook in reapply_resources_from_chain). Null when out of scope.
        [[nodiscard]] element* find_templated_parent() const;

        // Store (replacing any binding on the same target property) and immediately apply a
        // template_binding — BindableObject.SetBinding(property, new TemplateBinding(...)). Re-applied
        // automatically whenever this element's ancestor chain changes.
        void set_template_binding(template_binding binding);
        void clear_template_bindings();

    protected:
        // C# Element.SetChildInheritedBindingContext — the seam the templated controls override:
        // TemplatedView/TemplatedPage SUPPRESS inheritance into template-created children while a
        // ControlTemplate is set; content_presenter never propagates (its content gets the context
        // pushed by the templated parent instead). Default: plain inheritance.
        virtual void set_child_inherited_binding_context(
            element& child, const maui::core::bindable_object::binding_context_box& context)
        {
            child.set_inherited_binding_context(context);
        }

    private:
        // Re-resolve the templated parent and re-apply every stored template_binding against it (or
        // un-apply when the element left its template scope). Called from reapply_resources_from_chain.
        void reapply_template_bindings();

        std::vector<template_binding> template_bindings_; // the element's TemplateBindings (target-keyed)
        bool is_template_root_ = false;                   // Element.IsTemplateRoot

        friend class template_utilities; // walks logical children for the ControlTemplate machinery
        // --- end templates (W1-09) ----------------------------------------------------------------------

        // --- runtime bindings (W1-02) -------------------------------------------------------------
        // BindableObject.SetBinding/RemoveBinding + ApplyBindings, hosted on element (binding_base is
        // a controls-layer type). One binding per property name (C#'s per-specificity binding
        // LAYERING — style-sourced bindings — is not ported; styles use the setter channel instead).
    public:
        // Raised after this element's logical parent changes (Element.ParentSet) — relative-source
        // ancestor bindings re-resolve on it.
        maui::core::event<> parent_set;

        // Assign `binding` to the property named `property_name` and apply it now against the current
        // binding context (re-applied automatically when the context changes). A realized-TwoWay
        // binding applies its values at from_handler specificity, others at from_binding — and any
        // value currently above from_binding is silently demoted so the first apply replaces it
        // (BindableObject.SetBinding, incl. the dotnet/maui#16849 two-way rules).
        void set_binding(const std::string& property_name, std::shared_ptr<binding_base> binding);
        // Convenience (the C# string-path SetBinding extension): create + set a path binding.
        void set_binding(const std::string& property_name, std::string path,
                         maui::core::binding_mode mode = maui::core::binding_mode::default_mode);
        // Unapply + drop the binding on `property_name` (BindableObject.RemoveBinding). The last
        // value the binding applied is kept, like C#.
        void remove_binding(std::string_view property_name);
        [[nodiscard]] std::shared_ptr<binding_base> binding_for(std::string_view property_name) const;

        // Bindings are torn down (unapplied) by the destructor (declared in the templates block
        // above — one declaration, defined in element.cpp) so source subscriptions never dangle (§8).
        element(const element&) = delete;
        element(element&&) = delete;
        element& operator=(const element&) = delete;
        element& operator=(element&&) = delete;

    private:
        // BindableObject.ApplyBindings: unapply + re-apply every binding against the (new) context.
        void reapply_bindings(bool from_binding_context_changed);

        struct bound_property
        {
            std::shared_ptr<binding_base> binding;
            maui::core::setter_specificity specificity;
        };
        std::unordered_map<std::string, bound_property> bindings_;
        // --- end runtime bindings (W1-02) ---------------------------------------------------------
        // --- animations (W1-14) ---
        // shared_ptr (not unique_ptr) so the deleter is type-erased at make_shared time: element's
        // inline defaulted constructors never need the (header-incomplete) detail type. Destroying an
        // element silently detaches its running animations from their manager (the deterministic-
        // teardown analog of C#'s weak-keyed static tables; see element_animations). Never shared.
        std::shared_ptr<detail::element_animations> animation_state_; // lazily created (see accessor)
        int batched_ = 0;                                             // VisualElement._batched
        // --- end animations (W1-14) ---

        // --- styles tail (W1-15) ------------------------------------------------------------------------
        // 1) Style.ApplyToDerivedTypes needs the control's BASE-TYPE CHAIN: C# walks Type.BaseType
        //    (MergedStyle.RegisterImplicitStyles registers one implicit-style slot per ancestor type); the
        //    reflection-free substitute has a derived control DECLARE its chain, most-derived first —
        //    `set_style_target_type<my_button, button>()`. The single-type overload above stays the common
        //    case (chain of one). merged_style walks the chain: the exact type always matches; a base-type
        //    implicit/class style matches only when it sets apply_to_derived_types (Style.CanBeAppliedTo).
    protected:
        template <class TControl, class TBase, class... TRest> void set_style_target_type()
        {
            style_target_type_ = maui::core::type_tag::of<TControl>();
            merged_style_.set_target_chain({maui::core::type_tag::of<TControl>(), maui::core::type_tag::of<TBase>(),
                                            maui::core::type_tag::of<TRest>()...});
        }

        // 2) The NAMED-EVENT registrar (EventTrigger's reflection-free seam — the event analog of the
        //    property-name routing through apply_setter): a control registers each public event channel by
        //    name in its constructor, supplying a subscribe function that connects a handler to the typed
        //    event member and returns the RAII connection. Re-registering a name replaces the channel.
        void register_named_event(std::string name,
                                  std::function<maui::core::scoped_connection(std::function<void()>)> subscribe);

    public:
        // Subscribe `handler` to the named event channel (EventTrigger.AttachHandlerTo). An unknown or
        // never-registered name returns an EMPTY connection — C# logs a warning and attaches nothing.
        [[nodiscard]] maui::core::scoped_connection connect_named_event(std::string_view name,
                                                                        std::function<void()> handler);

    private:
        // name → subscribe-function (the registered channels). Names are owned strings (built names ok).
        std::unordered_map<std::string, std::function<maui::core::scoped_connection(std::function<void()>)>>
            named_events_;
        // --- end styles tail (W1-15) --------------------------------------------------------------------

        // --- platform configuration (W2-24) ---------------------------------------------------------------
        // IElementConfiguration<TElement>.On<T>() + the attached-BindableProperty store the *Specific knob
        // sets (include/maui/controls/platform_configuration/) write through. Regular properties keep their
        // typed property<T> members; this side store exists ONLY for the platform-specific attached knobs,
        // whose keys are namespaced "<platform>.<DeclaringClass>.<PropertyName>" (the C# attached property's
        // declaring-type identity) so they can never collide with a property/mapper name.
    public:
        // C# IElementConfiguration<TElement>.On<T>(): the typed platform-config accessor over this element.
        // Deducing-this supplies C#'s self-typing — the static type at the call site becomes TElement, so
        // `entry.on<platform_configuration::ios>()` is a config<ios, entry> the entry knob set chains on.
        // Lazily minted per call (see configuration.hpp on the dropped registry cache).
        template <class TPlatform, class TSelf> [[nodiscard]] auto on(this TSelf& self)
        {
            static_assert(std::is_base_of_v<platform_configuration::i_config_platform, TPlatform>,
                          "TPlatform must be a platform marker tag (i_config_platform)");
            return platform_configuration::config<TPlatform, TSelf>(self);
        }

        // BindableObject.SetValue for a platform-spec attached property: equal value short-circuits (C#
        // SetValueActual), otherwise property_changing fires, the value is stored, and property_changed
        // fires — so a view<> forwards the (namespaced) name to its handler like any property change, and
        // a wired-real knob's mapper key re-runs.
        template <class T> void set_platform_spec(std::string_view name, T value)
        {
            std::string key{name};
            if (const auto it = platform_specs_.find(key); it != platform_specs_.end())
            {
                if (const T* current = std::any_cast<T>(&it->second); current != nullptr && *current == value)
                {
                    return;
                }
            }
            on_property_changing(name);
            platform_specs_.insert_or_assign(std::move(key), std::move(value));
            on_property_changed(name);
        }

        // BindableObject.GetValue: the stored value, or the knob's declared default when never set (the
        // knob header supplies the default — C# keeps it on the BindableProperty descriptor).
        template <class T> [[nodiscard]] T platform_spec(std::string_view name, T default_value) const
        {
            if (const auto it = platform_specs_.find(std::string{name}); it != platform_specs_.end())
            {
                if (const T* current = std::any_cast<T>(&it->second); current != nullptr)
                {
                    return *current;
                }
            }
            return default_value;
        }

        // BindableObject.IsSet for a platform-spec attached property (the guard C# consumers apply, e.g.
        // TextExtensions.UpdateCursorColor only acts when Entry.CursorColorProperty IsSet).
        [[nodiscard]] bool has_platform_spec(std::string_view name) const
        {
            return platform_specs_.find(std::string{name}) != platform_specs_.end();
        }

    private:
        std::unordered_map<std::string, std::any> platform_specs_; // namespaced key → boxed value
        // --- end platform configuration (W2-24) -----------------------------------------------------------

        // --- effects (G3) ---------------------------------------------------------------------------------
        // Element.Effects + the attach/detach lifecycle (Element.cs): the owning effect collection, the
        // settable IEffectControlProvider (the IElementController/IVisualElementController role), and the
        // AttachEffect / EffectControlProvider-change / property-changed routing. The bodies live in
        // element_effects.cpp; effect_collection.hpp is included above (alongside resource_dictionary.hpp,
        // for the same unique_ptr<...>-member reason — the destructor needs the complete type), while
        // effect / i_effect_control_provider stay forward-declared (only pointers/refs to them here).
    public:
        // Element.Effects: the collection of effects applied to this element, lazily created on first access
        // (matching C#'s lazy _effects). Adding / removing an effect triggers attach / detach against the
        // current effect-control-provider; clearing detaches each.
        [[nodiscard]] maui::controls::effect_collection& effects();
        [[nodiscard]] bool has_effects() const
        {
            return effects_ != nullptr;
        }

        // IVisualElementController.EffectControlProvider (the setter the renderer/handler — or a test stub —
        // assigns). On change: detach every effect from the old provider, swap, then attach every effect to
        // the new one (Element.EffectControlProvider's setter). NON-owning (the provider outlives the element
        // in tests; a handler-backed provider is owned elsewhere).
        void set_effect_control_provider(maui::controls::i_effect_control_provider* value);
        [[nodiscard]] maui::controls::i_effect_control_provider* effect_control_provider() const
        {
            return effect_control_provider_;
        }

        // Element.EffectIsAttached(name): whether any attached effect resolves to `name`.
        [[nodiscard]] bool effect_is_attached(std::string_view name);

    protected:
        // Element.OnPropertyChanged: after the base notification, fan SendOnElementPropertyChanged out to
        // every effect (C# raises args once and forwards to each effect). Derived view<> / window route
        // their on_property_changed through this (instead of straight to bindable_object) so attached
        // effects observe the change. Defined in element_effects.cpp.
        void on_property_changed(std::string_view name) override;

    private:
        // Element.AttachEffect: no-op without a provider; throws if the effect is already attached; for a
        // routing_effect with an inner effect, registers the INNER effect; sets the effect's Element, then
        // send_attached. The collection's add/insert hook.
        void attach_effect(maui::controls::effect& target);
        // The collection's clear/remove hook (Effect.ClearEffect): detach + drop the element back-ref.
        static void clear_effect(maui::controls::effect& target);

        std::unique_ptr<maui::controls::effect_collection> effects_; // Element._effects (lazy, owning)
        // Element._effectControlProvider — non-owning back-ref to the provider.
        maui::controls::i_effect_control_provider* effect_control_provider_ = nullptr;
        // --- end effects (G3) -----------------------------------------------------------------------------
    };
} // namespace maui::controls
