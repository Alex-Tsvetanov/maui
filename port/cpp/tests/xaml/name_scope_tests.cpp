// Tests for maui::xaml::name_scope / name_scope_ref (M7 wave 2) — the x:Name registry of one XAML
// namescope. Behavior derived from src/Controls/src/Core/Internals/NameScope.cs (RegisterName /
// FindByName / UnregisterName semantics — there is no direct C# unit suite for the class itself, so
// these capture the source's behavior per the operating manual) plus the loader-level halves of
// src/Controls/tests/Xaml.UnitTests/NameScopeTests.cs, which land with the xaml_loader e2e tests
// (loader_tests.cpp) — the port's scopes hang off the XAML nodes/load result, not the elements
// (the placement deviation documented in name_scope.hpp).
#include "maui/xaml/name_scope.hpp"

#include <any>
#include <memory>
#include <stdexcept>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/label.hpp"
#include "maui/core/bindable_object.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::xaml::name_scope;
    using maui::xaml::name_scope_ref;

    // ---- INameScope.RegisterName / FindByName (NameScope.cs) ----

    TEST(name_scope, find_by_name_returns_the_registered_object)
    {
        name_scope scope;
        const auto label = std::make_shared<maui::controls::label>();
        scope.register_name("title", std::any{std::shared_ptr<maui::core::bindable_object>{label}});

        const std::any* found = scope.find_by_name("title");
        ASSERT_NE(found, nullptr);
        EXPECT_EQ(std::any_cast<std::shared_ptr<maui::core::bindable_object>>(*found), label);
    }

    TEST(name_scope, find_by_name_misses_with_null)
    {
        // C# FindByName returns null for an unregistered name.
        const name_scope scope;
        EXPECT_EQ(scope.find_by_name("nope"), nullptr);
    }

    TEST(name_scope, names_compare_ordinal)
    {
        // _names uses StringComparer.Ordinal — "Title" and "title" are distinct.
        name_scope scope;
        scope.register_name("Title", std::any{std::string{"upper"}});
        EXPECT_EQ(scope.find_by_name("title"), nullptr);
        ASSERT_NE(scope.find_by_name("Title"), nullptr);
    }

    TEST(name_scope, duplicate_registration_throws_argument_exception)
    {
        // C#: ArgumentException($"An element with the key '{name}' already exists in NameScope").
        name_scope scope;
        scope.register_name("dupe", std::any{1});
        try
        {
            scope.register_name("dupe", std::any{2});
            FAIL() << "expected std::invalid_argument";
        }
        catch (const std::invalid_argument& error)
        {
            EXPECT_STREQ(error.what(), "An element with the key 'dupe' already exists in NameScope");
        }
        // The first registration survives the failed second one.
        ASSERT_NE(scope.find_by_name("dupe"), nullptr);
        EXPECT_EQ(std::any_cast<int>(*scope.find_by_name("dupe")), 1);
    }

    // ---- the typed lookup (Element.FindByName<T>'s unboxing role) ----

    TEST(name_scope, find_by_name_as_unboxes_the_control)
    {
        name_scope scope;
        const auto button = std::make_shared<maui::controls::button>();
        scope.register_name("ok", std::any{std::shared_ptr<maui::core::bindable_object>{button}});

        EXPECT_EQ(scope.find_by_name_as<maui::controls::button>("ok"), button);
        // A different control type, an unregistered name, and a non-control payload all miss.
        EXPECT_EQ(scope.find_by_name_as<maui::controls::label>("ok"), nullptr);
        EXPECT_EQ(scope.find_by_name_as<maui::controls::button>("nope"), nullptr);
        scope.register_name("text", std::any{std::string{"not a control"}});
        EXPECT_EQ(scope.find_by_name_as<maui::controls::button>("text"), nullptr);
    }

    // ---- INameScope.UnregisterName (NameScope.cs) ----

    TEST(name_scope, unregister_name_removes_the_registration)
    {
        name_scope scope;
        scope.register_name("gone", std::any{1});
        scope.unregister_name("gone");
        EXPECT_EQ(scope.find_by_name("gone"), nullptr);
        // And the name is free for re-registration afterwards.
        scope.register_name("gone", std::any{2});
        EXPECT_EQ(std::any_cast<int>(*scope.find_by_name("gone")), 2);
    }

    TEST(name_scope, unregister_name_guards_match_csharp)
    {
        name_scope scope;
        try
        {
            scope.unregister_name("");
            FAIL() << "expected std::invalid_argument";
        }
        catch (const std::invalid_argument& error)
        {
            EXPECT_STREQ(error.what(), "name was provided as empty string.");
        }
        try
        {
            scope.unregister_name("never-registered");
            FAIL() << "expected std::invalid_argument";
        }
        catch (const std::invalid_argument& error)
        {
            EXPECT_STREQ(error.what(), "name provided had not been registered.");
        }
    }

    // ---- name_scope_ref (XamlNode.cs NameScopeRef) ----

    TEST(name_scope_ref, retargeting_the_ref_retargets_every_holder)
    {
        // The C# two-level indirection: nodes of one scope share ONE ref object, so replacing
        // ref->scope (CreateValuesVisitor does this for a load_into root with an existing scope)
        // switches them all at once.
        const auto ref = std::make_shared<name_scope_ref>();
        ref->scope = std::make_shared<name_scope>();
        const std::shared_ptr<name_scope_ref>& alias = ref; // a second node holding the same ref

        const auto replacement = std::make_shared<name_scope>();
        replacement->register_name("only-here", std::any{1});
        ref->scope = replacement;

        EXPECT_EQ(alias->scope, replacement);
        EXPECT_NE(alias->scope->find_by_name("only-here"), nullptr);
    }
} // namespace
