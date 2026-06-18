// Tests for maui::controls::binding — the string-path runtime binding (W1-02). Ported from
// src/Controls/tests/Core.UnitTests/BindingUnitTests.cs + BindingBaseUnitTests.cs (the engine-level
// suites; XAML-only, reflection-only — private setters / POCO walking / ValueTuple — GC-weakness and
// CollectionSynchronization cases are skipped, see STATUS). The C# MockViewModel (a hand-rolled INPC
// poco) becomes a bindable_object with property<T> members; MockApplication.MockLogger becomes the
// binding_diagnostics failure handler.
#include "maui/controls/bindings/binding.hpp"

#include <any>
#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/bindings/binding_base.hpp"
#include "maui/controls/bindings/binding_diagnostics.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/i_indexable.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::binding;
    using maui::controls::binding_base;
    using maui::controls::element;
    using maui::controls::i_value_converter;
    using maui::controls::set_binding_failure_handler;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::binding_mode;
    using maui::core::i_indexable;
    using maui::core::property;
    using maui::core::setter_specificity;
    using maui::core::type_tag;

    // ---- shared mocks (MockViewModel / ComplexMockViewModel / MockBindable) ----

    const bindable_property<std::string>& vm_text_prop()
    {
        static const bindable_property<std::string> descriptor{"text"};
        return descriptor;
    }

    struct mock_view_model : bindable_object
    {
        property<std::string> text{*this, vm_text_prop()};
    };

    struct complex_mock_view_model;

    const bindable_property<std::shared_ptr<complex_mock_view_model>>& vm_model_prop()
    {
        static const bindable_property<std::shared_ptr<complex_mock_view_model>> descriptor{"model"};
        return descriptor;
    }

    // ComplexMockViewModel: Model (a nested view-model) + an [IndexerName("Indexer")] indexer.
    struct complex_mock_view_model : mock_view_model, i_indexable
    {
        property<std::shared_ptr<complex_mock_view_model>> model{*this, vm_model_prop()};

        [[nodiscard]] std::string_view indexer_name() const override
        {
            return "Indexer";
        }
        [[nodiscard]] std::optional<std::any> try_get_item(std::string_view index) const override
        {
            if (const auto slot = parse_index(index))
            {
                return std::any{values_.at(*slot)};
            }
            return std::nullopt;
        }
        bool try_set_item(std::string_view index, const std::any& value) override
        {
            const auto slot = parse_index(index);
            const auto* text = std::any_cast<std::string>(&value);
            if (!slot || text == nullptr)
            {
                return false;
            }
            if (values_.at(*slot) == *text)
            {
                return true;
            }
            values_.at(*slot) = *text;
            property_changed.raise("Indexer[" + std::string{index} + "]");
            return true;
        }
        [[nodiscard]] std::shared_ptr<maui::core::bindable_object> try_get_item_object(
            std::string_view index) const override
        {
            return i_indexable::try_get_item_object(index); // the default: items are leaves
        }
        void set_item(std::size_t index, std::string value)
        {
            (void)try_set_item(std::to_string(index), std::any{std::move(value)});
        }
        [[nodiscard]] const std::string& item(std::size_t index) const
        {
            return values_.at(index);
        }

    private:
        [[nodiscard]] static std::optional<std::size_t> parse_index(std::string_view index)
        {
            if (index.empty() || index.front() < '0' || index.front() > '9')
            {
                return std::nullopt;
            }
            const auto slot = static_cast<std::size_t>(index.front() - '0');
            return slot < 5 ? std::optional<std::size_t>{slot} : std::nullopt;
        }
        std::array<std::string, 5> values_;
    };

    // The bindable target (MockBindable): text (TwoWay default, like C#'s MockBindable.TextProperty),
    // a OneWay-default twin, an int slot, and a OneWayToSource-default slot.
    const bindable_property<std::string>& target_text_prop()
    {
        static const bindable_property<std::string> descriptor{
            "text", std::string{}, {.default_binding_mode = binding_mode::two_way}};
        return descriptor;
    }
    const bindable_property<std::string>& target_foo_prop()
    {
        static const bindable_property<std::string> descriptor{"foo", std::string{"foo-default"}};
        return descriptor;
    }
    const bindable_property<int>& target_int_prop()
    {
        static const bindable_property<int> descriptor{"target_int"};
        return descriptor;
    }
    const bindable_property<double>& target_value_prop()
    {
        static const bindable_property<double> descriptor{
            "value", 0.0, {.default_binding_mode = binding_mode::two_way}};
        return descriptor;
    }
    const bindable_property<std::string>& target_to_source_prop()
    {
        static const bindable_property<std::string> descriptor{
            "to_source", std::string{"ows-default"}, {.default_binding_mode = binding_mode::one_way_to_source}};
        return descriptor;
    }

    struct mock_bindable : element
    {
        property<std::string> text{*this, target_text_prop()};
        property<std::string> foo{*this, target_foo_prop()};
        property<int> target_int{*this, target_int_prop()};
        property<double> value{*this, target_value_prop()};
        property<std::string> to_source{*this, target_to_source_prop()};
    };

    // A target carrying a NAMED event channel ("completed") — the reflection-free analog of C#'s
    // MockPlatformView with a public event named in Binding.UpdateSourceEventName (PlatformBindingTests'
    // MockPlatformView.BazChanged / FireBazChanged). register_named_event mirrors C#'s GetRuntimeEvent
    // resolution; fire_completed() is the FireBazChanged() trigger.
    struct mock_event_bindable : mock_bindable
    {
        maui::core::event<> completed;

        mock_event_bindable()
        {
            this->register_named_event("completed", [this](std::function<void()> handler) {
                return maui::core::connect_scoped(completed, std::move(handler));
            });
        }

        void fire_completed()
        {
            completed.raise();
        }
    };

    // A container element exposing logical children (the StackLayout stand-in for inheritance tests).
    struct mock_container : mock_bindable
    {
        void add_child(element& child)
        {
            children_.push_back(&child);
            attach_logical_child(child);
        }
        void clear_children()
        {
            for (element* child : children_)
            {
                detach_logical_child(*child);
            }
            children_.clear();
        }

    protected:
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (element* child : children_)
            {
                visit(*child);
            }
        }

    private:
        std::vector<element*> children_;
    };

    // Counts binding failures through the diagnostics seam (MockApplication.MockLogger).
    class failure_log
    {
    public:
        failure_log()
        {
            set_binding_failure_handler([this](const std::string& message) { messages_.push_back(message); });
        }
        failure_log(const failure_log&) = delete;
        failure_log(failure_log&&) = delete;
        failure_log& operator=(const failure_log&) = delete;
        failure_log& operator=(failure_log&&) = delete;
        ~failure_log()
        {
            set_binding_failure_handler({});
        }
        [[nodiscard]] std::size_t count() const
        {
            return messages_.size();
        }

    private:
        std::vector<std::string> messages_;
    };

    // The C# test converters.
    struct string_int_converter : i_value_converter // TestConverter<string,int>
    {
        std::any convert(const std::any& value, type_tag target_type, const std::any& /*parameter*/) override
        {
            EXPECT_TRUE(target_type == type_tag::of<int>());
            return std::any{std::stoi(std::any_cast<std::string>(value))};
        }
        std::any convert_back(const std::any& value, type_tag target_type, const std::any& /*parameter*/) override
        {
            EXPECT_TRUE(target_type == type_tag::of<std::string>());
            return std::any{std::to_string(std::any_cast<int>(value))};
        }
    };

    struct parameter_converter : i_value_converter // TestConverterParameter
    {
        std::any convert(const std::any& /*value*/, type_tag /*target*/, const std::any& parameter) override
        {
            return parameter;
        }
        std::any convert_back(const std::any& /*value*/, type_tag /*target*/, const std::any& parameter) override
        {
            return parameter;
        }
    };

    struct identity_logger_converter : i_value_converter // IdentityLoggerConverter
    {
        identity_logger_converter(std::string* log, int id) : log_(log), id_(id)
        {
        }
        std::any convert(const std::any& value, type_tag /*target*/, const std::any& /*parameter*/) override
        {
            *log_ += std::to_string(id_);
            return value;
        }
        std::any convert_back(const std::any& /*v*/, type_tag /*t*/, const std::any& /*p*/) override
        {
            ADD_FAILURE() << "ConvertBack should not run";
            return {};
        }

    private:
        std::string* log_;
        int id_;
    };

    [[nodiscard]] std::shared_ptr<complex_mock_view_model> make_complex_chain(std::string leaf_text)
    {
        auto root = std::make_shared<complex_mock_view_model>();
        auto mid = std::make_shared<complex_mock_view_model>();
        auto leaf = std::make_shared<complex_mock_view_model>();
        leaf->text.set(std::move(leaf_text));
        mid->model.set(leaf);
        root->model.set(mid);
        return root;
    }

    // ================= Binding ctor / mutation rules =================

    TEST(binding_engine, ctor_sets_path_and_mode)
    {
        const binding b{"Foo", binding_mode::one_way_to_source};
        EXPECT_EQ(b.path(), "Foo");
        EXPECT_EQ(b.mode(), binding_mode::one_way_to_source);
    }

    TEST(binding_engine, invalid_ctor_throws)
    {
        EXPECT_THROW(binding{""}, std::invalid_argument);
        EXPECT_THROW(binding{"   "}, std::invalid_argument);
    }

    TEST(binding_engine, change_binding_after_apply_throws)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Bar");
        mock_bindable target;
        target.set_binding_context(vm);
        auto b = std::make_shared<binding>("text");
        target.set_binding("foo", b);

        EXPECT_THROW(b->set_path("other"), std::runtime_error);
        EXPECT_THROW(b->set_converter(nullptr), std::runtime_error);
        EXPECT_THROW(b->set_converter_parameter(std::any{1}), std::runtime_error);
        EXPECT_THROW(b->set_mode(binding_mode::one_way_to_source), std::runtime_error);
        EXPECT_THROW(b->set_string_format("{0}"), std::runtime_error);
    }

    TEST(binding_engine, null_path_is_self)
    {
        // C# NullPathIsSelf: a default Binding() binds the context itself.
        mock_bindable target;
        target.set_binding_context(std::make_shared<std::string>("Foo"));
        target.set_binding("foo", std::make_shared<binding>());
        EXPECT_EQ(target.foo.get(), "Foo");
    }

    TEST(binding_engine, reuse_binding_instance_throws)
    {
        auto b = std::make_shared<binding>("text");
        mock_bindable first;
        first.set_binding_context(std::make_shared<mock_view_model>());
        first.set_binding("foo", b);

        mock_bindable second;
        second.set_binding_context(std::make_shared<mock_view_model>());
        EXPECT_THROW(second.set_binding("foo", b), std::runtime_error);
    }

    TEST(binding_engine, clone_copies_the_policy)
    {
        auto converter = std::make_shared<parameter_converter>();
        auto b = std::make_shared<binding>(".", binding_mode::two_way, converter, std::any{42}, "{0}");
        const auto clone = std::static_pointer_cast<binding>(b->clone());
        EXPECT_EQ(clone->converter(), converter);
        EXPECT_EQ(std::any_cast<int>(clone->converter_parameter()), 42);
        EXPECT_EQ(clone->mode(), b->mode());
        EXPECT_EQ(clone->path(), b->path());
        EXPECT_EQ(clone->string_format(), b->string_format());
    }

    // ================= Simple paths: set / update / old-context =================

    TEST(binding_engine, value_set_on_one_way)
    {
        // C# ValueSetOnOneWay (set-context-first x explicit/default mode).
        for (const bool context_first : {true, false})
        {
            for (const bool use_default : {true, false})
            {
                failure_log const log;
                auto vm = std::make_shared<mock_view_model>();
                vm->text.set("Foo");
                mock_bindable target;
                auto b =
                    std::make_shared<binding>("text", use_default ? binding_mode::default_mode : binding_mode::one_way);
                if (context_first)
                {
                    target.set_binding_context(vm);
                    target.set_binding("foo", b); // "foo" defaults OneWay
                }
                else
                {
                    target.set_binding("foo", b);
                    target.set_binding_context(vm);
                }
                EXPECT_EQ(vm->text.get(), "Foo");
                EXPECT_EQ(target.foo.get(), "Foo");
                EXPECT_EQ(log.count(), 0U);
            }
        }
    }

    TEST(binding_engine, value_set_on_one_way_to_source)
    {
        for (const bool context_first : {true, false})
        {
            for (const bool use_default : {true, false})
            {
                failure_log const log;
                auto vm = std::make_shared<mock_view_model>();
                mock_bindable target;
                auto b = std::make_shared<binding>("text", use_default ? binding_mode::default_mode
                                                                       : binding_mode::one_way_to_source);
                if (context_first)
                {
                    target.set_binding_context(vm);
                    target.set_binding("to_source", b); // "to_source" defaults OneWayToSource
                }
                else
                {
                    target.set_binding("to_source", b);
                    target.set_binding_context(vm);
                }
                EXPECT_EQ(target.to_source.get(), "ows-default"); // target unchanged
                EXPECT_EQ(vm->text.get(), "ows-default");         // source received the target value
                EXPECT_EQ(log.count(), 0U);
            }
        }
    }

    TEST(binding_engine, value_set_on_two_way)
    {
        for (const bool context_first : {true, false})
        {
            for (const bool use_default : {true, false})
            {
                failure_log const log;
                auto vm = std::make_shared<mock_view_model>();
                vm->text.set("Foo");
                mock_bindable target;
                auto b =
                    std::make_shared<binding>("text", use_default ? binding_mode::default_mode : binding_mode::two_way);
                if (context_first)
                {
                    target.set_binding_context(vm);
                    target.set_binding("text", b); // "text" defaults TwoWay
                }
                else
                {
                    target.set_binding("text", b);
                    target.set_binding_context(vm);
                }
                EXPECT_EQ(vm->text.get(), "Foo");
                EXPECT_EQ(target.text.get(), "Foo");
                EXPECT_EQ(log.count(), 0U);
            }
        }
    }

    TEST(binding_engine, value_updated_with_simple_path_on_one_way)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way));

        vm->text.set("New Value");
        EXPECT_EQ(target.foo.get(), "New Value");
        EXPECT_EQ(vm->text.get(), "New Value");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_updated_with_simple_path_on_one_way_to_source)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("to_source", std::make_shared<binding>("text", binding_mode::one_way_to_source));

        const std::string original = target.to_source.get();
        vm->text.set("value");
        EXPECT_EQ(target.to_source.get(), original); // source change must not flow to target

        target.to_source.set("New Value");
        EXPECT_EQ(target.to_source.get(), "New Value");
        EXPECT_EQ(vm->text.get(), "New Value");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_updated_with_simple_path_on_two_way)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("text", std::make_shared<binding>("text", binding_mode::two_way));

        vm->text.set("New Value");
        EXPECT_EQ(target.text.get(), "New Value");
        EXPECT_EQ(vm->text.get(), "New Value");

        target.text.set("New Value in the other direction");
        EXPECT_EQ(vm->text.get(), "New Value in the other direction");
        EXPECT_EQ(target.text.get(), "New Value in the other direction");
        EXPECT_EQ(log.count(), 0U);
    }

    // ---- UpdateSourceEventName (Binding.UpdateSourceEventName) ----
    // Ported from PlatformBindingTests.Set2WayBindingsWithUpdateSourceEvent{,InBindingObject} (which are
    // [Fact(Skip="PlatformBindings aren't used")] upstream — native-view binding). The port has no native
    // binding, so the faithful analog binds an `element` target carrying a registered named event: firing
    // that event pushes target -> source, exactly like C#'s EventWrapper raising INPC(targetProperty).

    TEST(binding_engine, update_source_event_drives_source_update)
    {
        // OneWayToSource so a source-side write does NOT flow back to the target (no auto-resync). That
        // lets the two sides genuinely DIFFER at fire time, making the event-driven push observable: the
        // fire re-pushes the target's value over the externally-changed source. C# FireBazChanged() =>
        // vm.FFoo == platformView.Baz, the OWTS/TwoWay native-binding case.
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        mock_event_bindable target;
        target.set_binding_context(vm);

        auto b = std::make_shared<binding>("text", binding_mode::one_way_to_source);
        b->set_update_source_event_name("completed");
        target.set_binding("to_source", b);

        // Seed the target value; the OWTS auto-push (target property_changed) syncs it to the source.
        target.to_source.set("oof");
        EXPECT_EQ(vm->text.get(), "oof");

        // Desync: write the source directly. OWTS doesn't pull, so the target keeps "oof" and no auto-push
        // fires (the target property didn't change). The two sides now differ.
        vm->text.set("source-changed-externally");
        EXPECT_EQ(target.to_source.get(), "oof");
        EXPECT_EQ(vm->text.get(), "source-changed-externally");

        // Fire the named event: the ONLY thing that can re-push here. It overwrites the source with the
        // target's current value.
        target.fire_completed();
        EXPECT_EQ(vm->text.get(), "oof");
        EXPECT_EQ(target.to_source.get(), "oof");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, update_source_event_name_is_copied_by_clone)
    {
        // C# Binding.Clone() copies UpdateSourceEventName.
        auto b = std::make_shared<binding>("text", binding_mode::two_way);
        b->set_update_source_event_name("completed");
        const auto cloned = std::dynamic_pointer_cast<binding>(b->clone());
        ASSERT_NE(cloned, nullptr);
        EXPECT_EQ(cloned->update_source_event_name(), "completed");
    }

    TEST(binding_engine, update_source_event_name_set_after_apply_throws)
    {
        // C# UpdateSourceEventName setter calls ThrowIfApplied().
        auto b = std::make_shared<binding>("text", binding_mode::two_way);
        mock_event_bindable target;
        target.set_binding_context(std::make_shared<mock_view_model>());
        target.set_binding("text", b);
        EXPECT_THROW(b->set_update_source_event_name("completed"), std::runtime_error);
    }

    TEST(binding_engine, update_source_event_unregistered_name_pushes_nothing)
    {
        // connect_named_event returns an empty connection for an unregistered name (C# logs a warning and
        // attaches nothing): the binding never event-pushes, and firing the registered event does nothing
        // for this binding. No failure is logged for the name miss.
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        mock_event_bindable target;
        target.set_binding_context(vm);

        auto b = std::make_shared<binding>("text", binding_mode::one_way_to_source);
        b->set_update_source_event_name("no_such_event");
        target.set_binding("to_source", b);

        target.to_source.set("oof");               // OWTS auto-push syncs source to "oof"
        vm->text.set("still-source");              // desync; OWTS doesn't pull
        target.fire_completed();                   // "completed" exists but this binding subscribed to "no_such_event"
        EXPECT_EQ(vm->text.get(), "still-source"); // unchanged: no event push happened
    }

    TEST(binding_engine, value_updated_with_old_context_does_not_update)
    {
        // C# ValueUpdatedWithOldContextDoesNotUpdateWith{OneWay,TwoWay}Binding.
        for (const binding_mode mode : {binding_mode::one_way, binding_mode::two_way})
        {
            failure_log const log;
            auto vm = std::make_shared<mock_view_model>();
            vm->text.set("Foo");
            mock_bindable target;
            target.set_binding_context(vm);
            target.set_binding("foo", std::make_shared<binding>("text", mode));
            EXPECT_EQ(target.foo.get(), "Foo");

            auto replacement = std::make_shared<mock_view_model>();
            target.set_binding_context(replacement);
            EXPECT_EQ(target.foo.get(), ""); // the new context's (default) value

            vm->text.set("New Value");
            EXPECT_EQ(target.foo.get(), ""); // the old context is disconnected

            if (mode == binding_mode::two_way)
            {
                const std::string original = vm->text.get();
                target.foo.set("target change");
                EXPECT_EQ(vm->text.get(), original); // old source must not see target changes
            }
            EXPECT_EQ(log.count(), 0U);
        }
    }

    TEST(binding_engine, binding_stays_on_update_value_from_binding)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("text"));

        vm->text.set("New Value");
        EXPECT_EQ(target.foo.get(), "New Value");
        vm->text.set("new value 2");
        EXPECT_EQ(target.foo.get(), "new value 2");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, one_way_to_source_context_set_to_null)
    {
        mock_bindable target;
        target.set_binding_context(std::make_shared<mock_view_model>());
        target.set_binding("text", std::make_shared<binding>("text", binding_mode::one_way_to_source));
        target.set_binding_context(std::shared_ptr<mock_view_model>{}); // must not crash
    }

    // ================= StringFormat (BindingBaseUnitTests) =================

    TEST(binding_engine, string_format)
    {
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Bar");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding(
            "foo", std::make_shared<binding>("text", binding_mode::default_mode, nullptr, std::any{}, "Foo {0}"));
        EXPECT_EQ(target.foo.get(), "Foo Bar");
    }

    TEST(binding_engine, string_format_on_update)
    {
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Bar");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding(
            "foo", std::make_shared<binding>("text", binding_mode::default_mode, nullptr, std::any{}, "Foo {0}"));
        vm->text.set("Baz");
        EXPECT_EQ(target.foo.get(), "Foo Baz");
    }

    TEST(binding_engine, string_format_not_applied_to_one_way_to_source)
    {
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Bar");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding(
            "foo", std::make_shared<binding>("text", binding_mode::one_way_to_source, nullptr, std::any{}, "Foo {0}"));
        target.foo.set("Bar");
        EXPECT_EQ(vm->text.get(), "Bar"); // not "Foo Bar"
    }

    TEST(binding_engine, string_format_two_way_applies_only_from_source)
    {
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Bar");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo",
                           std::make_shared<binding>("text", binding_mode::two_way, nullptr, std::any{}, "Foo {0}"));
        target.foo.set("Baz", setter_specificity::from_handler);
        EXPECT_EQ(vm->text.get(), "Baz");
        EXPECT_EQ(target.foo.get(), "Foo Baz"); // the source push reformats the target
    }

    TEST(binding_engine, string_format_non_string_source)
    {
        // C# StringFormatNonStringType ({0:P2} percent formatting is culture machinery the port does
        // not carry; the invariant zero-pad spec is covered in the multi-binding tests instead).
        auto vm = std::make_shared<mock_bindable>();
        vm->value.set(0.95);
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding(
            "foo", std::make_shared<binding>("value", binding_mode::default_mode, nullptr, std::any{}, "Value: {0}"));
        EXPECT_EQ(target.foo.get(), "Value: 0.95");
    }

    // ================= Complex paths =================

    TEST(binding_engine, value_set_with_complex_path_one_way)
    {
        for (const bool context_first : {true, false})
        {
            failure_log const log;
            auto vm = make_complex_chain("Foo");
            mock_bindable target;
            auto b = std::make_shared<binding>("model.model.text", binding_mode::one_way);
            if (context_first)
            {
                target.set_binding_context(vm);
                target.set_binding("foo", b);
            }
            else
            {
                target.set_binding("foo", b);
                target.set_binding_context(vm);
            }
            EXPECT_EQ(target.foo.get(), "Foo");
            EXPECT_EQ(log.count(), 0U);
        }
    }

    TEST(binding_engine, value_set_with_complex_path_one_way_to_source)
    {
        failure_log const log;
        auto vm = make_complex_chain("");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("to_source", std::make_shared<binding>("model.model.text", binding_mode::one_way_to_source));
        EXPECT_EQ(target.to_source.get(), "ows-default");
        EXPECT_EQ(vm->model.get()->model.get()->text.get(), "ows-default");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_set_with_complex_path_two_way)
    {
        failure_log const log;
        auto vm = make_complex_chain("Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("text", std::make_shared<binding>("model.model.text", binding_mode::two_way));
        EXPECT_EQ(target.text.get(), "Foo");

        vm->model.get()->model.get()->text.set("New Value");
        EXPECT_EQ(target.text.get(), "New Value");

        target.text.set("other direction");
        EXPECT_EQ(vm->model.get()->model.get()->text.get(), "other direction");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_updated_with_complex_path_intermediate_change)
    {
        // The heart of BindingExpression: an INTERMEDIATE hop swap re-resolves the chain.
        failure_log const log;
        auto vm = make_complex_chain("Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("model.model.text", binding_mode::one_way));
        EXPECT_EQ(target.foo.get(), "Foo");

        auto replacement = std::make_shared<complex_mock_view_model>();
        replacement->text.set("Swapped");
        vm->model.get()->model.set(replacement);
        EXPECT_EQ(target.foo.get(), "Swapped");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, null_in_path_uses_default_value)
    {
        failure_log const log;
        auto vm = std::make_shared<complex_mock_view_model>();
        auto mid = std::make_shared<complex_mock_view_model>();
        mid->text.set("mid");
        vm->model.set(mid);

        mock_bindable target;
        target.set_binding("foo", std::make_shared<binding>("model.text", binding_mode::one_way));
        target.set_binding_context(vm);
        EXPECT_EQ(target.foo.get(), "mid");

        vm->model.set(nullptr);
        EXPECT_EQ(target.foo.get(), "foo-default");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, null_context_uses_default_value)
    {
        failure_log const log;
        auto vm = std::make_shared<complex_mock_view_model>();
        auto mid = std::make_shared<complex_mock_view_model>();
        mid->text.set("vm value");
        vm->model.set(mid);

        mock_bindable target;
        target.set_binding("foo", std::make_shared<binding>("model.text", binding_mode::one_way));
        target.set_binding_context(vm);
        EXPECT_EQ(target.foo.get(), "vm value");

        target.set_binding_context(std::shared_ptr<complex_mock_view_model>{});
        EXPECT_EQ(target.foo.get(), "foo-default");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, chained_part_null_logs_nothing)
    {
        failure_log const log;
        mock_bindable target;
        target.set_binding_context(std::make_shared<complex_mock_view_model>()); // model is null
        target.set_binding("foo", std::make_shared<binding>("model.text"));
        EXPECT_EQ(log.count(), 0U);
    }

    // ================= Indexed paths =================

    TEST(binding_engine, value_set_on_one_way_with_indexed_path)
    {
        failure_log const log;
        auto vm = make_complex_chain("");
        vm->model.get()->model.get()->set_item(1, "Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("model.model[1]", binding_mode::one_way));
        EXPECT_EQ(target.foo.get(), "Foo");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_set_on_one_way_with_self_indexed_path)
    {
        failure_log const log;
        auto vm = std::make_shared<complex_mock_view_model>();
        vm->set_item(1, "Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>(".[1]", binding_mode::one_way));
        EXPECT_EQ(target.foo.get(), "Foo");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_updated_with_indexed_path_on_one_way)
    {
        failure_log const log;
        auto vm = make_complex_chain("");
        vm->model.get()->model.get()->set_item(1, "Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("model.model[1]", binding_mode::one_way));

        vm->model.get()->model.get()->set_item(1, "New Value");
        EXPECT_EQ(target.foo.get(), "New Value");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_updated_with_indexed_path_on_two_way)
    {
        failure_log const log;
        auto vm = make_complex_chain("");
        vm->model.get()->model.get()->set_item(1, "Foo");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("text", std::make_shared<binding>("model.model[1]", binding_mode::two_way));
        EXPECT_EQ(target.text.get(), "Foo");

        vm->model.get()->model.get()->set_item(1, "New Value");
        EXPECT_EQ(target.text.get(), "New Value");

        target.text.set("other direction");
        EXPECT_EQ(vm->model.get()->model.get()->item(1), "other direction");
        EXPECT_EQ(log.count(), 0U);
    }

    // A string-keyed indexable (C# IndexedViewModel: dict + "Item[Foo]" notifications).
    struct indexed_view_model : bindable_object, i_indexable
    {
        [[nodiscard]] std::optional<std::any> try_get_item(std::string_view index) const override
        {
            if (const auto it = values_.find(std::string{index}); it != values_.end())
            {
                return std::any{it->second};
            }
            return std::nullopt;
        }
        bool try_set_item(std::string_view index, const std::any& value) override
        {
            const auto* text = std::any_cast<std::string>(&value);
            if (text == nullptr)
            {
                return false;
            }
            values_[std::string{index}] = *text;
            property_changed.raise("Item[" + std::string{index} + "]");
            return true;
        }
        [[nodiscard]] std::string_view indexer_name() const override
        {
            return i_indexable::indexer_name(); // the default "Item"
        }
        [[nodiscard]] std::shared_ptr<maui::core::bindable_object> try_get_item_object(
            std::string_view index) const override
        {
            return i_indexable::try_get_item_object(index); // the default: items are leaves
        }
        void set_item(const std::string& index, std::string value)
        {
            (void)try_set_item(index, std::any{std::move(value)});
        }

    private:
        std::map<std::string, std::string> values_;
    };

    const bindable_property<std::shared_ptr<indexed_view_model>>& vm_data_prop()
    {
        static const bindable_property<std::shared_ptr<indexed_view_model>> descriptor{"data"};
        return descriptor;
    }
    struct data_view_model : bindable_object
    {
        property<std::shared_ptr<indexed_view_model>> data{*this, vm_data_prop()};
    };

    TEST(binding_engine, indexed_view_model_property_changed)
    {
        // C# IndexedViewModelPropertyChanged: "Data[Foo]" resolves later when the key appears.
        auto inner = std::make_shared<indexed_view_model>();
        auto vm = std::make_shared<data_view_model>();
        vm->data.set(inner);

        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("data[Foo]"));
        EXPECT_EQ(target.foo.get(), "foo-default"); // key missing -> unresolved -> default

        inner->set_item("Foo", "Baz");
        EXPECT_EQ(target.foo.get(), "Baz");
    }

    // ================= Self paths =================

    TEST(binding_engine, value_set_on_one_way_with_self_path)
    {
        for (const bool context_first : {true, false})
        {
            failure_log const log;
            mock_bindable target;
            auto b = std::make_shared<binding>(".", binding_mode::one_way);
            auto context = std::make_shared<std::string>("value");
            if (context_first)
            {
                target.set_binding_context(context);
                target.set_binding("foo", b);
            }
            else
            {
                target.set_binding("foo", b);
                target.set_binding_context(context);
            }
            EXPECT_EQ(target.foo.get(), "value");
            EXPECT_EQ(log.count(), 0U);
        }
    }

    TEST(binding_engine, value_not_set_on_one_way_to_source_with_self_path)
    {
        failure_log const log;
        mock_bindable target;
        target.set_binding("to_source", std::make_shared<binding>(".", binding_mode::one_way_to_source));
        EXPECT_EQ(target.to_source.get(), "ows-default"); // target unchanged
        EXPECT_FALSE(target.has_binding_context());       // context untouched (a self path has no setter)
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_updated_with_self_path_on_two_way)
    {
        failure_log const log;
        mock_bindable target;
        target.set_binding_context(std::make_shared<std::string>("value"));
        target.set_binding("text", std::make_shared<binding>(".", binding_mode::two_way));

        auto next = std::make_shared<std::string>("New Value");
        target.set_binding_context(next);
        EXPECT_EQ(target.text.get(), "New Value");

        target.text.set("other direction");
        // a self path cannot write back: the context value is unchanged
        EXPECT_EQ(*target.binding_context<std::string>(), "New Value");
        EXPECT_EQ(target.text.get(), "other direction");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, self_binding_converter)
    {
        struct int_to_string_converter : i_value_converter
        {
            std::any convert(const std::any& value, type_tag /*t*/, const std::any& /*p*/) override
            {
                return std::any{std::to_string(**std::any_cast<std::shared_ptr<int>>(&value))};
            }
            std::any convert_back(const std::any& /*v*/, type_tag /*t*/, const std::any& /*p*/) override
            {
                return {};
            }
        };
        failure_log const log;
        mock_bindable target;
        target.set_binding_context(std::make_shared<int>(1));
        target.set_binding("foo", std::make_shared<binding>(".", binding_mode::default_mode,
                                                            std::make_shared<int_to_string_converter>()));
        EXPECT_EQ(target.foo.get(), "1");
        EXPECT_EQ(log.count(), 0U);
    }

    // ================= Converters =================

    TEST(binding_engine, value_converter)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("1");
        mock_bindable target;
        target.set_binding("target_int", std::make_shared<binding>("text", binding_mode::default_mode,
                                                                   std::make_shared<string_int_converter>()));
        target.set_binding_context(vm);
        EXPECT_EQ(target.target_int.get(), 1);
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_converter_back)
    {
        struct int_to_string_back : i_value_converter
        {
            std::any convert(const std::any& value, type_tag /*t*/, const std::any& /*p*/) override
            {
                return value;
            }
            std::any convert_back(const std::any& value, type_tag target_type, const std::any& /*p*/) override
            {
                EXPECT_TRUE(target_type == type_tag::of<std::string>());
                return std::any{std::to_string(std::any_cast<int>(value))};
            }
        };
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        mock_bindable target;
        target.target_int.set(1);
        target.set_binding("target_int", std::make_shared<binding>("text", binding_mode::one_way_to_source,
                                                                   std::make_shared<int_to_string_back>()));
        target.set_binding_context(vm);
        EXPECT_EQ(vm->text.get(), "1");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, value_converter_parameter)
    {
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        mock_bindable target;
        target.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way_to_source,
                                                            std::make_shared<parameter_converter>(),
                                                            std::any{std::string{"Foo"}}));
        target.set_binding_context(vm);
        EXPECT_EQ(vm->text.get(), "Foo");
        EXPECT_EQ(log.count(), 0U);
    }

    // ================= Apply-once + isolation =================

    TEST(binding_engine, bindings_apply_only_once_on_binding_context_inheritance)
    {
        std::string log;
        auto context = std::make_shared<mock_view_model>();
        context->text.set("a binding context");

        mock_container root;
        mock_bindable const level1;
        mock_container level1c; // need a container to host level2
        mock_bindable level2;
        level1c.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way,
                                                             std::make_shared<identity_logger_converter>(&log, 1)));
        level2.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way,
                                                            std::make_shared<identity_logger_converter>(&log, 2)));
        root.add_child(level1c);
        level1c.add_child(level2);
        log.clear(); // ignore the unresolved applies made before the context exists

        root.set_binding_context(context);
        EXPECT_EQ(log, "12");
    }

    TEST(binding_engine, bindings_apply_only_once_on_parent_set)
    {
        std::string log;
        auto context = std::make_shared<mock_view_model>();
        context->text.set("a binding context");

        mock_container root;
        mock_container level1;
        mock_bindable level2;
        level1.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way,
                                                            std::make_shared<identity_logger_converter>(&log, 1)));
        level2.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way,
                                                            std::make_shared<identity_logger_converter>(&log, 2)));
        level1.add_child(level2);
        root.set_binding_context(context);
        log.clear();

        root.add_child(level1);
        EXPECT_EQ(log, "12");
    }

    TEST(binding_engine, bindings_should_not_trigger_other_bindings)
    {
        // C# BindingsShouldNotTriggerOtherBindings (adapted: the converter counts evaluations).
        std::string log;
        auto vm = std::make_shared<complex_mock_view_model>();
        mock_bindable target;
        target.set_binding("foo", std::make_shared<binding>("model", binding_mode::one_way,
                                                            std::make_shared<identity_logger_converter>(&log, 1)));
        target.set_binding("text", std::make_shared<binding>("text", binding_mode::one_way));
        target.set_binding_context(vm);
        const std::string after_context = log;

        vm->text.set("update"); // must not re-evaluate the "model" binding
        EXPECT_EQ(log, after_context);
    }

    TEST(binding_engine, binding_creates_single_subscription)
    {
        auto vm = std::make_shared<mock_view_model>();
        mock_bindable target;
        target.set_binding("foo", std::make_shared<binding>("text"));
        target.set_binding_context(vm);
        EXPECT_EQ(vm->property_changed.handler_count(), 1U);
    }

    TEST(binding_engine, binding_unsubscribes_for_dead_target)
    {
        // C# BindingUnsubscribesForDeadTarget (GC -> deterministic destruction per §8).
        auto vm = std::make_shared<mock_view_model>();
        {
            mock_bindable target;
            target.set_binding("foo", std::make_shared<binding>("text"));
            target.set_binding_context(vm);
            EXPECT_EQ(vm->property_changed.handler_count(), 1U);
        }
        EXPECT_EQ(vm->property_changed.handler_count(), 0U);
        vm->text.set("no crash");
    }

    TEST(binding_engine, source_death_before_target_is_safe)
    {
        // The reverse teardown order: the source dies first; the target's later teardown and
        // re-application must not touch the dead source (ASan validates).
        mock_bindable target;
        {
            auto vm = std::make_shared<mock_view_model>();
            vm->text.set("alive");
            target.set_binding_context(vm);
            target.set_binding("foo", std::make_shared<binding>("text"));
            EXPECT_EQ(target.foo.get(), "alive");
        }
        target.remove_binding("foo"); // guarded disconnect against the dead source
        EXPECT_EQ(target.foo.get(), "alive");
    }

    TEST(binding_engine, different_context_type_accessed_correctly_with_same_path)
    {
        // C# DifferentContextTypeAccessedCorrectlyWithSamePath: paths don't care about types.
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("text");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("text"));
        EXPECT_EQ(target.foo.get(), "text");

        auto other = std::make_shared<complex_mock_view_model>(); // a different type with "text"
        other->text.set("foo");
        target.set_binding_context(other);
        EXPECT_EQ(target.foo.get(), "foo");
    }

    // ================= Missing properties / failures =================

    TEST(binding_engine, property_missing_logs_failure)
    {
        for (const binding_mode mode : {binding_mode::one_way, binding_mode::one_way_to_source, binding_mode::two_way})
        {
            failure_log const log;
            mock_bindable target;
            target.set_binding_context(std::make_shared<mock_view_model>());
            target.text.set("foo");
            target.set_binding("text", std::make_shared<binding>("Monkeys", mode));
            EXPECT_GE(log.count(), 1U) << static_cast<int>(mode);
        }
    }

    TEST(binding_engine, binding_applies_after_getter_previously_missing)
    {
        struct empty_view_model : bindable_object
        {
        };
        failure_log const log;
        mock_bindable target;
        target.set_binding_context(std::make_shared<empty_view_model>());
        target.set_binding("foo", std::make_shared<binding>("text"));
        EXPECT_EQ(log.count(), 1U);

        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("Foo");
        target.set_binding_context(vm);
        EXPECT_EQ(target.foo.get(), "Foo");
        EXPECT_EQ(log.count(), 1U); // no new failures
    }

    TEST(binding_engine, property_not_found_chained)
    {
        failure_log const log;
        auto vm = std::make_shared<complex_mock_view_model>();
        vm->model.set(std::make_shared<complex_mock_view_model>());
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("model.MissingProperty"));
        EXPECT_EQ(log.count(), 1U);
        EXPECT_EQ(target.foo.get(), "foo-default");
    }

    TEST(binding_engine, fail_to_convert_keeps_default)
    {
        // C# FailToConvert: a model object cannot convert to double — the target keeps its default.
        failure_log const log;
        auto vm = std::make_shared<complex_mock_view_model>();
        vm->model.set(std::make_shared<complex_mock_view_model>());
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("value", std::make_shared<binding>("model"));
        EXPECT_EQ(target.value.get(), 0.0);
        EXPECT_EQ(log.count(), 1U); // TypeConversionExceptionIsCaughtAndLogged
    }

    TEST(binding_engine, null_ref_with_default_ctor)
    {
        mock_bindable target;
        target.set_binding("foo", std::make_shared<binding>()); // no context — must not crash
    }

    // ================= One-time =================

    TEST(binding_engine, one_time_binding_doesnt_update_on_property_changed)
    {
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("foobar");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_time));
        target.set_binding("text", std::make_shared<binding>("text", binding_mode::one_way));
        EXPECT_EQ(target.foo.get(), "foobar");
        EXPECT_EQ(target.text.get(), "foobar");

        vm->text.set("qux");
        EXPECT_EQ(target.text.get(), "qux");
        EXPECT_EQ(target.foo.get(), "foobar"); // one-time: frozen
    }

    TEST(binding_engine, one_time_binding_updates_on_binding_context_changed)
    {
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("foobar");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_time));
        EXPECT_EQ(target.foo.get(), "foobar");

        auto next = std::make_shared<mock_view_model>();
        next->text.set("qux");
        target.set_binding_context(next);
        EXPECT_EQ(target.foo.get(), "qux");
    }

    // ================= FallbackValue / TargetNullValue =================

    TEST(binding_engine, fallback_value_when_source_is_null)
    {
        mock_bindable target;
        auto b = std::make_shared<binding>("Foo.Bar");
        b->set_fallback_value(std::any{std::string{"fallback"}});
        target.set_binding("foo", b);
        EXPECT_EQ(target.foo.get(), "fallback");
    }

    // A view-model with a NULLABLE text (C# string null), for TargetNullValue.
    const bindable_property<std::shared_ptr<std::string>>& vm_opt_text_prop()
    {
        static const bindable_property<std::shared_ptr<std::string>> descriptor{"opt_text"};
        return descriptor;
    }
    struct nullable_view_model : bindable_object
    {
        property<std::shared_ptr<std::string>> opt_text{*this, vm_opt_text_prop()};
    };

    TEST(binding_engine, target_null_value_ignored_when_binding_is_resolved)
    {
        mock_bindable target;
        auto b = std::make_shared<binding>("opt_text");
        b->set_target_null_value(std::any{std::string{"fallback"}});
        target.set_binding("foo", b);
        EXPECT_EQ(target.foo.get(), "foo-default");

        auto vm = std::make_shared<nullable_view_model>();
        vm->opt_text.set(std::make_shared<std::string>("Foo"));
        target.set_binding_context(vm);
        EXPECT_EQ(target.foo.get(), "Foo");
    }

    TEST(binding_engine, target_null_value_fallback)
    {
        mock_bindable target;
        auto b = std::make_shared<binding>("opt_text");
        b->set_target_null_value(std::any{std::string{"fallback"}});
        target.set_binding("foo", b);
        EXPECT_EQ(target.foo.get(), "foo-default");

        target.set_binding_context(std::make_shared<nullable_view_model>()); // opt_text is null
        EXPECT_EQ(target.foo.get(), "fallback");
    }

    // ================= Source =================

    TEST(binding_engine, binding_source_over_context)
    {
        mock_bindable target;
        target.set_binding_context(std::make_shared<std::string>("bindingcontext"));
        target.set_binding("foo", std::make_shared<binding>("."));
        EXPECT_EQ(target.foo.get(), "bindingcontext");

        auto pinned = std::make_shared<binding>(".");
        pinned->set_source(std::make_shared<std::string>("bindingsource"));
        target.set_binding("foo", pinned);
        EXPECT_EQ(target.foo.get(), "bindingsource");
    }

    TEST(binding_engine, binding_with_source_not_reapplied_when_binding_context_changed)
    {
        // C# VM57081 (adapted: the converter counts applies instead of a counting getter).
        std::string log;
        auto model = std::make_shared<mock_view_model>();
        mock_bindable target;
        auto b = std::make_shared<binding>("text", binding_mode::one_way,
                                           std::make_shared<identity_logger_converter>(&log, 1));
        b->set_source(model);
        target.set_binding("foo", b);
        EXPECT_EQ(log, "1");

        target.set_binding_context(std::make_shared<mock_view_model>());
        EXPECT_EQ(log, "1"); // pinned source: not re-applied
    }

    TEST(binding_engine, binding_with_source_not_reapplied_when_parented)
    {
        std::string log;
        auto model = std::make_shared<mock_view_model>();
        mock_container parent;
        parent.set_binding_context(std::make_shared<mock_view_model>());
        mock_bindable child;
        auto b = std::make_shared<binding>("text", binding_mode::one_way,
                                           std::make_shared<identity_logger_converter>(&log, 1));
        b->set_source(model);
        child.set_binding("foo", b);
        EXPECT_EQ(log, "1");

        parent.add_child(child);
        EXPECT_EQ(log, "1");
    }

    TEST(binding_engine, inpc_on_binding_with_source)
    {
        // C# INPCOnBindingWithSource: path "binding_context.text" against a pinned element source.
        auto page = std::make_shared<mock_bindable>();
        auto title_vm = std::make_shared<mock_view_model>();
        title_vm->text.set("Foo");
        page->set_binding_context(title_vm);

        mock_bindable label;
        auto b = std::make_shared<binding>("binding_context.text");
        b->set_source(page);
        label.set_binding("foo", b);
        EXPECT_EQ(label.foo.get(), "Foo");

        title_vm->text.set("Bar");
        EXPECT_EQ(label.foo.get(), "Bar");

        auto other_vm = std::make_shared<mock_view_model>();
        other_vm->text.set("Baz");
        page->set_binding_context(other_vm); // the binding_context hop re-resolves
        EXPECT_EQ(label.foo.get(), "Baz");
    }

    // ================= TwoWay specificity rules (dotnet/maui#16849 / #17776) =================

    TEST(binding_engine, value_from_handler_doesnt_clear_two_way_binding)
    {
        auto vm = std::make_shared<mock_view_model>();
        mock_bindable entry;
        entry.set_binding_context(vm);
        entry.set_binding("text", std::make_shared<binding>("text", binding_mode::two_way));
        entry.text.set("foo", setter_specificity::from_handler);
        EXPECT_EQ(vm->text.get(), "foo");
        vm->text.set("bar");
        EXPECT_EQ(entry.text.get(), "bar");
        vm->text.set("");
        EXPECT_EQ(entry.text.get(), "");
    }

    TEST(binding_engine, manual_value_doesnt_clear_two_way_binding)
    {
        auto vm = std::make_shared<mock_view_model>();
        mock_bindable entry;
        entry.set_binding_context(vm);
        entry.set_binding("text", std::make_shared<binding>("text", binding_mode::two_way));
        entry.text.set("foo");
        EXPECT_EQ(vm->text.get(), "foo");
        vm->text.set("bar");
        EXPECT_EQ(entry.text.get(), "bar");
        vm->text.set("");
        EXPECT_EQ(entry.text.get(), "");
    }

    TEST(binding_engine, double_set_binding)
    {
        // C# DoubleSetBinding (#17776): manual value + rebinding twice must not throw.
        auto source = std::make_shared<mock_view_model>();
        source->text.set("HotPink");
        mock_bindable button;
        button.foo.set("Coral");
        auto first = std::make_shared<binding>("text");
        first->set_source(source);
        button.set_binding("foo", first);
        EXPECT_EQ(button.foo.get(), "HotPink");
        button.foo.set("Coral");
        auto second = std::make_shared<binding>("text");
        second->set_source(source);
        button.set_binding("foo", second);
        EXPECT_EQ(button.foo.get(), "HotPink"); // the rebind replaces the demoted manual value
    }

    TEST(binding_engine, convert_two_way_string_double)
    {
        // C# Convert: slider.Value (double, TwoWay) bound to vm.Text (string), invariant culture.
        failure_log const log;
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("0.5");
        mock_bindable slider;
        slider.set_binding_context(vm);
        slider.set_binding("value", std::make_shared<binding>("text", binding_mode::two_way));
        EXPECT_EQ(slider.value.get(), 0.5);

        slider.value.set(0.9);
        EXPECT_EQ(vm->text.get(), "0.9");
        EXPECT_EQ(log.count(), 0U);
    }

    TEST(binding_engine, multiple_property_updates)
    {
        // C# MultiplePropertyUpdates (adapted): one INPC raise refreshes the dependent binding.
        auto vm = std::make_shared<mock_view_model>();
        vm->text.set("initial");
        mock_bindable target;
        target.set_binding_context(vm);
        target.set_binding("foo", std::make_shared<binding>("text", binding_mode::one_way));

        // C# NullPropertyUpdatesAllBindings: an empty property name refreshes every binding.
        vm->text.set("Foo");
        EXPECT_EQ(target.foo.get(), "Foo");
        target.foo.set("stale", setter_specificity::from_handler);
        vm->property_changed.raise("");
        EXPECT_EQ(target.foo.get(), "Foo");
    }
} // namespace
