// Tests for Setter.TargetName resolution via the element-side NameScope (U-TN). Behavior derived from
// src/Controls/src/Core/Setter.cs (Apply/UnApply + FindTargetByName, lines ~70-130) and
// src/Controls/src/Core/Element/Element.cs (FindByName + GetNameScope walking RealParent, lines
// ~542-953) plus src/Controls/src/Core/Internals/NameScope.cs (the attached-NameScope accessors). The
// XAML oracle for the ControlTemplate-boundary parent-walk is Maui3793.xaml (a <Setter TargetName="Check">
// inside a VisualState whose target is the templated control, while "Check" lives in the inner
// ControlTemplate namescope).
//
// These exercise the code-first path: a control registers its named descendants in its own NameScope,
// and a named setter resolves the string at apply() time against that scope (and, for nested scopes,
// the ancestor chain).
#include "maui/controls/setter.hpp"

#include <memory>
#include <string>

#include "maui/controls/element.hpp"
#include "maui/controls/trigger.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/xaml/name_scope.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::element;
    using maui::controls::property_trigger;
    using maui::controls::setter;
    using maui::core::bindable_property;
    using maui::core::property;
    using maui::core::setter_specificity;
    using maui::xaml::name_scope;

    const bindable_property<bool>& flag_prop()
    {
        static const bindable_property<bool> descriptor{"flag", false};
        return descriptor;
    }
    const bindable_property<std::string>& out_prop()
    {
        static const bindable_property<std::string> descriptor{"out"};
        return descriptor;
    }

    // A leaf element with a watched flag + a settable "out" — stands in for a named child control.
    struct mock_element : element
    {
        property<bool> flag{*this, flag_prop()};
        property<std::string> out{*this, out_prop()};
    };

    // A container element exposing one logical child, so the parent chain + per-element NameScope can be
    // wired (attach_logical_child / for_each_logical_child are protected on element).
    struct mock_container : element
    {
        std::shared_ptr<mock_element> child = std::make_shared<mock_element>();
        property<bool> flag{*this, flag_prop()};
        property<std::string> out{*this, out_prop()};

        mock_container()
        {
            attach_logical_child(*child);
        }
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            visit(*child);
        }
        // Re-expose the protected attach hook for the nested-scope test (a presenter-like inner child).
        void add_child(element& grandchild)
        {
            attach_logical_child(grandchild);
        }
    };

    // ---- Element-side NameScope (Element.FindByName + GetNameScope) ----

    TEST(element_name_scope, find_by_name_resolves_against_own_scope)
    {
        mock_container root;
        auto scope = std::make_shared<name_scope>();
        scope->register_name("Child", std::any{std::shared_ptr<maui::core::bindable_object>{root.child}});
        root.set_name_scope(scope);

        // FindByName from the root resolves its own scope.
        EXPECT_EQ(root.find_by_name("Child"), root.child.get());
        // An unregistered name misses (C# returns null).
        EXPECT_EQ(root.find_by_name("Missing"), nullptr);
    }

    TEST(element_name_scope, get_name_scope_walks_parent_chain)
    {
        // C# Element.GetNameScope walks RealParent until it finds an attached scope: the child has no
        // scope of its own, so it inherits the root's (FindByName from the child resolves "Child").
        mock_container root;
        auto scope = std::make_shared<name_scope>();
        scope->register_name("Child", std::any{std::shared_ptr<maui::core::bindable_object>{root.child}});
        root.set_name_scope(scope);

        EXPECT_EQ(root.child->get_name_scope(), scope.get());
        EXPECT_EQ(root.child->find_by_name("Child"), root.child.get());
    }

    TEST(element_name_scope, no_scope_in_chain_returns_null)
    {
        // No scope anywhere up the chain: GetNameScope is null and FindByName misses (C# returns null
        // from FindByName once the transient/attached scope is absent — the port has no transient slot).
        mock_container root;
        EXPECT_EQ(root.get_name_scope(), nullptr);
        EXPECT_EQ(root.find_by_name("Child"), nullptr);
    }

    // ---- Setter.TargetName: single-scope resolution at apply time (Setter.Apply -> FindTargetByName) ----

    TEST(setter_target_name, resolves_named_target_from_apply_target_scope)
    {
        mock_container root;
        auto scope = std::make_shared<name_scope>();
        scope->register_name("Child", std::any{std::shared_ptr<maui::core::bindable_object>{root.child}});
        root.set_name_scope(scope);

        // A named setter applied to `root` writes to the element named "Child", not to root itself.
        const setter named = setter::of_named(out_prop(), std::string("ON"), "Child");
        named.apply(root, setter_specificity::trigger);
        EXPECT_EQ(root.child->out.get(), "ON");
        EXPECT_EQ(root.out.get(), ""); // root (the apply target) is untouched

        named.unapply(root, setter_specificity::trigger);
        EXPECT_EQ(root.child->out.get(), ""); // unapply clears on the resolved target
    }

    // ---- Setter.TargetName: ControlTemplate-boundary parent-walk (Maui3793: the named element lives in
    // an OUTER scope while the apply target's own scope misses) ----

    TEST(setter_target_name, walks_parent_chain_across_namescope_boundary)
    {
        // root owns a scope that registers "Banner". An inner subtree (presenter-like) carries its OWN
        // scope that does NOT have "Banner". A setter applied to the inner element must still resolve
        // "Banner" by walking up to root's scope (FindTargetByName's parent walk).
        mock_container root;
        auto outer = std::make_shared<name_scope>();
        outer->register_name("Banner", std::any{std::shared_ptr<maui::core::bindable_object>{root.child}});
        root.set_name_scope(outer);

        auto inner = std::make_shared<mock_element>();
        root.add_child(*inner);
        auto inner_scope = std::make_shared<name_scope>(); // a nested scope WITHOUT "Banner"
        inner->set_name_scope(inner_scope);

        const setter named = setter::of_named(out_prop(), std::string("HIT"), "Banner");
        named.apply(*inner, setter_specificity::trigger);
        EXPECT_EQ(root.child->out.get(), "HIT"); // resolved across the inner namescope boundary
    }

    // ---- Error behavior (Setter.Apply: unresolved name throws) ----

    TEST(setter_target_name, unresolved_name_throws)
    {
        mock_container root;
        root.set_name_scope(std::make_shared<name_scope>()); // empty scope

        const setter named = setter::of_named(out_prop(), std::string("X"), "Nope");
        EXPECT_THROW(named.apply(root, setter_specificity::trigger), maui::xaml::xaml_parse_exception);
    }

    TEST(setter_target_name, named_setter_on_non_element_throws)
    {
        // C# guards `target is Element`; a bare bindable_object cannot host a namescope, so a named
        // setter applied to one is unresolvable.
        maui::core::bindable_object plain;
        const setter named = setter::of_named(out_prop(), std::string("X"), "Nope");
        EXPECT_THROW(named.apply(plain, setter_specificity::trigger), maui::xaml::xaml_parse_exception);
    }

    // ---- End-to-end through a trigger (a named setter inside a property_trigger) ----

    TEST(setter_target_name, trigger_applies_named_setter_to_resolved_element)
    {
        mock_container root;
        auto scope = std::make_shared<name_scope>();
        scope->register_name("Child", std::any{std::shared_ptr<maui::core::bindable_object>{root.child}});
        root.set_name_scope(scope);

        property_trigger<bool> trigger{root.flag, true};
        trigger.add(setter::of_named(out_prop(), std::string("ON"), "Child"));

        auto handle = trigger.attach(root);
        EXPECT_EQ(root.child->out.get(), ""); // flag is false at attach

        root.flag.set(true);
        EXPECT_EQ(root.child->out.get(), "ON"); // condition true -> named setter writes to "Child"
        EXPECT_EQ(root.out.get(), "");          // the trigger's own target is untouched

        root.flag.set(false);
        EXPECT_EQ(root.child->out.get(), ""); // de-activation un-applies on the resolved target
    }
} // namespace
