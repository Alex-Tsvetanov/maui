// Tests for maui::controls::multi_binding (W1-02). Ported from
// src/Controls/tests/Core.UnitTests/MultiBindingTests.cs: the PersonViewModel/GroupViewModel pair,
// the StringConcatenation/AllTrue/AnyTrue converters with their DoNothing/UnsetValue/null sentinel
// matrix, nested multi-bindings with relative sources, the per-mode behavior table, and composite
// StringFormat. Skips (deviations, see STATUS): the TemplatedParent/ControlTemplate scenarios (no
// templates in the port) and the binding-the-BindingContext label trick in TestBindingModes (context
// bindings are out of scope for W1-02 — the test re-contexts the label directly instead).
#include "maui/controls/bindings/multi_binding.hpp"

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/bindings/binding.hpp"
#include "maui/controls/bindings/binding_base.hpp"
#include "maui/controls/bindings/i_multi_value_converter.hpp"
#include "maui/controls/bindings/i_value_converter.hpp"
#include "maui/controls/bindings/relative_binding_source.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/boxed_value.hpp"
#include "maui/core/property.hpp"
#include "maui/core/type_tag.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::binding;
    using maui::controls::binding_base;
    using maui::controls::do_nothing_value;
    using maui::controls::element;
    using maui::controls::i_multi_value_converter;
    using maui::controls::i_value_converter;
    using maui::controls::multi_binding;
    using maui::controls::relative_binding_source;
    using maui::controls::unset_value;
    using maui::core::bindable_object;
    using maui::core::bindable_property;
    using maui::core::binding_mode;
    using maui::core::property;
    using maui::core::type_tag;

    constexpr std::string_view c_fallback = "First Middle Last";
    constexpr std::string_view c_target_null = "No Name Given";

    // ---- PersonViewModel / GroupViewModel ----

    const bindable_property<std::string>& first_name_prop()
    {
        static const bindable_property<std::string> d{"first_name"};
        return d;
    }
    const bindable_property<std::string>& middle_name_prop()
    {
        static const bindable_property<std::string> d{"middle_name"};
        return d;
    }
    const bindable_property<std::string>& last_name_prop()
    {
        static const bindable_property<std::string> d{"last_name"};
        return d;
    }
    const bindable_property<bool>& is_over16_prop()
    {
        static const bindable_property<bool> d{"is_over16"};
        return d;
    }
    const bindable_property<bool>& has_passed_test_prop()
    {
        static const bindable_property<bool> d{"has_passed_test"};
        return d;
    }
    const bindable_property<bool>& is_suspended_prop()
    {
        static const bindable_property<bool> d{"is_suspended"};
        return d;
    }
    const bindable_property<bool>& is_monarch_prop()
    {
        static const bindable_property<bool> d{"is_monarch"};
        return d;
    }

    struct person_view_model : bindable_object
    {
        property<std::string> first_name{*this, first_name_prop()};
        property<std::string> middle_name{*this, middle_name_prop()};
        property<std::string> last_name{*this, last_name_prop()};
        property<bool> is_over16{*this, is_over16_prop()};
        property<bool> has_passed_test{*this, has_passed_test_prop()};
        property<bool> is_suspended{*this, is_suspended_prop()};
        property<bool> is_monarch{*this, is_monarch_prop()};

        [[nodiscard]] std::string full_name() const
        {
            return first_name.get() + " " + middle_name.get() + " " + last_name.get();
        }
    };

    const bindable_property<bool>& pardon_prop()
    {
        static const bindable_property<bool> d{"pardon_all_suspensions"};
        return d;
    }
    const bindable_property<bool>& suspend_all_prop()
    {
        static const bindable_property<bool> d{"suspend_all"};
        return d;
    }

    struct group_view_model : bindable_object
    {
        property<bool> pardon_all_suspensions{*this, pardon_prop()};
        property<bool> suspend_all{*this, suspend_all_prop()};

        std::shared_ptr<person_view_model> person1 =
            make_person("Gaius", "Julius", "Caesar", true, false, false, false);
        std::shared_ptr<person_view_model> person2 = make_person("William", "Henry", "Gates", true, true, false, false);
        std::shared_ptr<person_view_model> person3 =
            make_person("John", "Fitzgerald", "Kennedy", true, true, true, false);
        std::shared_ptr<person_view_model> person4 = make_person("Harry", "James", "Potter", false, true, false, false);
        std::shared_ptr<person_view_model> person5 = make_person("Queen", "Elizabeth", "II", true, false, false, true);

    private:
        static std::shared_ptr<person_view_model> make_person(std::string first, std::string middle, std::string last,
                                                              bool over16, bool passed, bool suspended, bool monarch)
        {
            auto person = std::make_shared<person_view_model>();
            person->first_name.set(std::move(first));
            person->middle_name.set(std::move(middle));
            person->last_name.set(std::move(last));
            person->is_over16.set(over16);
            person->has_passed_test.set(passed);
            person->is_suspended.set(suspended);
            person->is_monarch.set(monarch);
            return person;
        }
    };

    // ---- the bindable target mocks ----

    const bindable_property<std::string>& label_text_prop()
    {
        static const bindable_property<std::string> d{"text"};
        return d;
    }
    struct mock_label : element
    {
        property<std::string> text{*this, label_text_prop()};
    };

    const bindable_property<bool>& checked_prop()
    {
        static const bindable_property<bool> d{"is_checked"};
        return d;
    }
    struct mock_check_box : element
    {
        property<bool> is_checked{*this, checked_prop()};
    };

    struct mock_container : element
    {
        void add_child(element& child)
        {
            children_.push_back(&child);
            attach_logical_child(child);
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

    // ---- the C# test converters ----

    struct inverter : i_value_converter // Inverter
    {
        std::any convert(const std::any& value, type_tag /*t*/, const std::any& /*p*/) override
        {
            const bool* b = std::any_cast<bool>(&value);
            return std::any{b != nullptr && !*b};
        }
        std::any convert_back(const std::any& value, type_tag target_type, const std::any& parameter) override
        {
            return convert(value, target_type, parameter);
        }
    };

    struct all_true_multi_converter : i_multi_value_converter // AllTrueMultiConverter
    {
        std::any convert(const std::vector<std::any>& values, type_tag /*t*/, const std::any& /*p*/) override
        {
            for (const std::any& value : values)
            {
                const bool* b = std::any_cast<bool>(&value);
                if (b == nullptr)
                {
                    return std::any{unset_value{}}; // use the binding FallbackValue
                }
                if (!*b)
                {
                    return std::any{false};
                }
            }
            return std::any{true};
        }
        std::optional<std::vector<std::any>> convert_back(const std::any& value,
                                                          const std::vector<type_tag>& target_types,
                                                          const std::any& /*p*/) override
        {
            const bool* b = std::any_cast<bool>(&value);
            if (b == nullptr || !*b)
            {
                return std::nullopt; // can't convert back from false (ambiguous)
            }
            return std::vector<std::any>(target_types.size(), std::any{true});
        }
    };

    struct any_true_multi_converter : i_multi_value_converter // AnyTrueMultiConverter
    {
        std::any convert(const std::vector<std::any>& values, type_tag /*t*/, const std::any& /*p*/) override
        {
            for (const std::any& value : values)
            {
                const bool* b = std::any_cast<bool>(&value);
                if (b == nullptr)
                {
                    return std::any{unset_value{}};
                }
                if (*b)
                {
                    return std::any{true};
                }
            }
            return std::any{false};
        }
        std::optional<std::vector<std::any>> convert_back(const std::any& value,
                                                          const std::vector<type_tag>& target_types,
                                                          const std::any& /*p*/) override
        {
            const bool* b = std::any_cast<bool>(&value);
            if (b == nullptr || *b)
            {
                return std::nullopt;
            }
            return std::vector<std::any>(target_types.size(), std::any{false});
        }
    };

    // StringConcatenationConverter, sentinel strings and all.
    struct string_concatenation_converter : i_multi_value_converter
    {
        int converts = 0;
        int convert_backs = 0;

        std::any convert(const std::vector<std::any>& values, type_tag /*t*/, const std::any& parameter) override
        {
            ++converts;
            const std::string separator = std::any_cast<std::string>(&parameter) != nullptr
                                              ? std::any_cast<std::string>(parameter)
                                              : std::string{" "};
            bool all_empty = true;
            for (const std::any& value : values)
            {
                const std::string* text = std::any_cast<std::string>(&value);
                if (text != nullptr && !text->empty())
                {
                    all_empty = false;
                }
            }
            if (all_empty)
            {
                return std::any{unset_value{}};
            }
            std::string result;
            int i = 0;
            for (const std::any& value : values)
            {
                const std::string* text = std::any_cast<std::string>(&value);
                if (text != nullptr && *text == "DoNothing")
                {
                    return std::any{do_nothing_value{}};
                }
                if (text != nullptr && *text == "UnsetValue")
                {
                    return std::any{unset_value{}};
                }
                if (text != nullptr && *text == "null")
                {
                    return std::any{};
                }
                if (i != 0)
                {
                    result += separator;
                }
                if (text != nullptr)
                {
                    result += *text;
                }
                else if (auto rendered = maui::core::boxed_to_string(value))
                {
                    result += *rendered;
                }
                ++i;
            }
            return std::any{result};
        }

        std::optional<std::vector<std::any>> convert_back(const std::any& value,
                                                          const std::vector<type_tag>& target_types,
                                                          const std::any& parameter) override
        {
            ++convert_backs;
            const std::string* s = std::any_cast<std::string>(&value);
            if (s == nullptr || s->empty() || *s == "null")
            {
                return std::nullopt;
            }
            const std::string separator = std::any_cast<std::string>(&parameter) != nullptr
                                              ? std::any_cast<std::string>(parameter)
                                              : std::string{" "};
            for (const type_tag& tag : target_types)
            {
                // "Normally we'd return null but throw just for the unit test to catch" (C#).
                if (!(tag == type_tag::of<std::string>() || tag == type_tag::of<void>()))
                {
                    throw std::runtime_error("Invalid targetTypes");
                }
            }
            std::vector<std::any> parts;
            std::size_t start = 0;
            while (start <= s->size())
            {
                const std::size_t pos = s->find(separator, start);
                std::string piece = pos == std::string::npos ? s->substr(start) : s->substr(start, pos - start);
                if (!piece.empty())
                {
                    if (piece == "null")
                    {
                        parts.emplace_back();
                    }
                    else if (piece == "UnsetValue")
                    {
                        parts.emplace_back(unset_value{});
                    }
                    else if (piece == "DoNothing")
                    {
                        parts.emplace_back(do_nothing_value{});
                    }
                    else
                    {
                        parts.emplace_back(std::move(piece));
                    }
                }
                if (pos == std::string::npos)
                {
                    break;
                }
                start = pos + separator.size();
            }
            return parts;
        }
    };

    [[nodiscard]] std::shared_ptr<multi_binding> make_name_multi(binding_mode mode,
                                                                 std::shared_ptr<i_multi_value_converter> converter)
    {
        auto multi = std::make_shared<multi_binding>();
        multi->add_binding(std::make_shared<binding>("first_name"));
        multi->add_binding(std::make_shared<binding>("middle_name"));
        multi->add_binding(std::make_shared<binding>("last_name"));
        multi->set_converter(std::move(converter));
        multi->set_mode(mode);
        multi->set_fallback_value(std::any{std::string{c_fallback}});
        multi->set_target_null_value(std::any{std::string{c_target_null}});
        return multi;
    }

    // ================= the suites =================

    TEST(multi_binding, throws_without_converter_or_format_or_bindings)
    {
        mock_label label;
        auto empty = std::make_shared<multi_binding>();
        empty->set_converter(std::make_shared<string_concatenation_converter>());
        EXPECT_THROW(label.set_binding("text", empty), std::runtime_error); // Bindings empty

        auto no_policy = std::make_shared<multi_binding>();
        no_policy->add_binding(std::make_shared<binding>("first_name"));
        EXPECT_THROW(label.set_binding("text", no_policy), std::runtime_error); // converter+format null
    }

    TEST(multi_binding, test_child_one_way_on_multi_two_way)
    {
        auto group = std::make_shared<group_view_model>();
        mock_container stack;
        stack.set_binding_context(group->person1);

        const std::string old_name = group->person1->full_name();
        const std::string old_first = group->person1->first_name.get();
        const std::string old_middle = group->person1->middle_name.get();

        mock_label label;
        auto multi = std::make_shared<multi_binding>();
        multi->add_binding(std::make_shared<binding>("first_name", binding_mode::one_way));
        multi->add_binding(std::make_shared<binding>("middle_name"));
        multi->add_binding(std::make_shared<binding>("last_name"));
        multi->set_converter(std::make_shared<string_concatenation_converter>());
        multi->set_mode(binding_mode::two_way);
        label.set_binding("text", multi);
        stack.add_child(label);

        EXPECT_EQ(label.text.get(), old_name);
        EXPECT_EQ(group->person1->full_name(), old_name);

        const std::string upper_first = "GAIUS";
        const std::string upper_last = "CAESAR";
        label.text.set(upper_first + " " + old_middle + " " + upper_last);
        // the first-name child binding is OneWay: the source keeps the old first name
        EXPECT_EQ(group->person1->full_name(), old_first + " " + old_middle + " " + upper_last);
    }

    TEST(multi_binding, test_multi_binding_continues_updating_after_convert_back)
    {
        auto group = std::make_shared<group_view_model>();
        mock_container stack;
        stack.set_binding_context(group->person1);

        mock_label label;
        label.set_binding("text",
                          make_name_multi(binding_mode::two_way, std::make_shared<string_concatenation_converter>()));
        stack.add_child(label);

        EXPECT_EQ(label.text.get(), "Gaius Julius Caesar");

        label.text.set("Marcus Tullius Cicero");
        EXPECT_EQ(group->person1->first_name.get(), "Marcus");
        EXPECT_EQ(group->person1->middle_name.get(), "Tullius");
        EXPECT_EQ(group->person1->last_name.get(), "Cicero");
        EXPECT_EQ(label.text.get(), "Marcus Tullius Cicero");

        group->person1->first_name.set("Julius");
        EXPECT_EQ(label.text.get(), "Julius Tullius Cicero");
        EXPECT_EQ(group->person1->full_name(), "Julius Tullius Cicero");

        group->person1->last_name.set("Augustus");
        EXPECT_EQ(label.text.get(), "Julius Tullius Augustus");
        EXPECT_EQ(group->person1->full_name(), "Julius Tullius Augustus");
    }

    TEST(multi_binding, test_nested_multi_bindings)
    {
        auto group = std::make_shared<group_view_model>();
        mock_container stack;
        stack.set_binding_context(group);

        auto inner_multi = std::make_shared<multi_binding>();
        inner_multi->add_binding(std::make_shared<binding>("is_over16"));
        inner_multi->add_binding(std::make_shared<binding>("has_passed_test"));
        inner_multi->add_binding(
            std::make_shared<binding>("is_suspended", binding_mode::default_mode, std::make_shared<inverter>()));
        auto suspend_all =
            std::make_shared<binding>("suspend_all", binding_mode::default_mode, std::make_shared<inverter>());
        suspend_all->set_source(relative_binding_source::find_ancestor_binding_context<group_view_model>());
        inner_multi->add_binding(suspend_all);
        inner_multi->set_converter(std::make_shared<all_true_multi_converter>());

        auto pardon = std::make_shared<binding>("binding_context.pardon_all_suspensions");
        pardon->set_source(relative_binding_source::find_ancestor<mock_container>());

        auto outer = std::make_shared<multi_binding>();
        outer->add_binding(inner_multi);
        outer->add_binding(std::make_shared<binding>("is_monarch"));
        outer->add_binding(pardon);
        outer->set_converter(std::make_shared<any_true_multi_converter>());
        outer->set_fallback_value(std::any{false});

        // CanDrive = (IsOver16 && HasPassedTest && !IsSuspended && !Group.SuspendAll) || IsMonarch
        //            || Group.PardonAllSuspensions
        mock_check_box check_box;
        check_box.set_binding("is_checked", outer);
        check_box.set_binding_context(group->person5);
        stack.add_child(check_box);

        // Monarch can do whatever she wants
        EXPECT_TRUE(check_box.is_checked.get());
        // ... until being deposed after a coup
        group->person5->is_monarch.set(false);
        EXPECT_FALSE(check_box.is_checked.get());
        // After passing the test she can drive again
        group->person5->has_passed_test.set(true);
        EXPECT_TRUE(check_box.is_checked.get());
        // Martial law declared; no one can drive
        group->suspend_all.set(true);
        EXPECT_FALSE(check_box.is_checked.get());
        // Martial law is over
        group->suspend_all.set(false);
        EXPECT_TRUE(check_box.is_checked.get());
        // But she got in an accident and now can't drive again
        group->person5->is_suspended.set(true);
        EXPECT_FALSE(check_box.is_checked.get());
        // The new PM has pardoned everyone
        group->pardon_all_suspensions.set(true);
        EXPECT_TRUE(check_box.is_checked.get());
    }

    TEST(multi_binding, test_converter_return_values)
    {
        auto group = std::make_shared<group_view_model>();
        mock_container stack;
        stack.set_binding_context(group);

        // "Convert" return values
        std::string old_name = group->person1->full_name();
        mock_label label1;
        label1.set_binding("text",
                           make_name_multi(binding_mode::two_way, std::make_shared<string_concatenation_converter>()));
        label1.set_binding_context(group->person1);
        stack.add_child(label1);

        group->person1->first_name.set("DoNothing");
        EXPECT_EQ(label1.text.get(), old_name);
        EXPECT_EQ(group->person1->first_name.get(), "DoNothing");

        group->person1->first_name.set("UnsetValue");
        EXPECT_EQ(label1.text.get(), c_fallback);
        EXPECT_EQ(group->person1->first_name.get(), "UnsetValue");

        group->person1->first_name.set("null");
        EXPECT_EQ(label1.text.get(), c_target_null);
        EXPECT_EQ(group->person1->first_name.get(), "null");

        // "ConvertBack" return values
        old_name = group->person2->full_name();
        const std::string old_first = group->person2->first_name.get();
        const std::string old_middle = group->person2->middle_name.get();
        const std::string old_last = group->person2->last_name.get();

        mock_label label2;
        label2.set_binding("text",
                           make_name_multi(binding_mode::two_way, std::make_shared<string_concatenation_converter>()));
        label2.set_binding_context(group->person2);
        stack.add_child(label2);

        const std::string upper_last = "GATES";
        label2.text.set("DoNothing " + old_middle + " " + upper_last);
        EXPECT_EQ(group->person2->full_name(), old_first + " " + old_middle + " " + upper_last);
        EXPECT_EQ(label2.text.get(), "DoNothing " + old_middle + " " + upper_last);

        label2.text.set(old_name);
        EXPECT_EQ(group->person2->full_name(), old_name);
        // Any UnsetValue prevents that source update but the target accepts the value
        const std::string upper_first = "WILLIAM";
        label2.text.set(upper_first + " UnsetValue " + old_last);
        EXPECT_EQ(group->person2->full_name(), upper_first + " " + old_middle + " " + old_last);
        EXPECT_EQ(label2.text.get(), upper_first + " UnsetValue " + old_last);

        label2.text.set(old_name);
        EXPECT_EQ(group->person2->full_name(), old_name);
        // Returning null prevents changes to the source but the target accepts the value
        label2.text.set("null");
        EXPECT_EQ(group->person2->full_name(), old_name);
        EXPECT_EQ(label2.text.get(), "null");

        // Insufficient members in the ConvertBack array don't affect the remaining
        label2.text.set(old_name);
        EXPECT_EQ(group->person2->full_name(), old_name);
        label2.text.set("Duck Duck");
        EXPECT_EQ(group->person2->full_name(), "Duck Duck " + old_last);
        EXPECT_EQ(label2.text.get(), "Duck Duck");

        // Too many members are no problem either
        label2.text.set(old_name);
        EXPECT_EQ(group->person2->full_name(), old_name);
        label2.text.set(old_name + " Extra");
        EXPECT_EQ(group->person2->full_name(), old_name);
        EXPECT_EQ(label2.text.get(), old_name + " Extra");
    }

    TEST(multi_binding, test_binding_modes)
    {
        auto group = std::make_shared<group_view_model>();
        mock_container stack;
        stack.set_binding_context(group);

        std::string old_name = group->person1->full_name();
        mock_label label_1w;
        label_1w.set_binding(
            "text", make_name_multi(binding_mode::one_way, std::make_shared<string_concatenation_converter>()));
        label_1w.set_binding_context(group->person1);
        stack.add_child(label_1w);
        EXPECT_EQ(label_1w.text.get(), group->person1->full_name());
        label_1w.text.set("don't change source");
        EXPECT_EQ(group->person1->full_name(), old_name);

        mock_label label_2w;
        label_2w.set_binding(
            "text", make_name_multi(binding_mode::two_way, std::make_shared<string_concatenation_converter>()));
        label_2w.set_binding_context(group->person2);
        stack.add_child(label_2w);
        EXPECT_EQ(label_2w.text.get(), group->person2->full_name());
        const std::string upper2 = "WILLIAM HENRY GATES";
        label_2w.text.set(upper2);
        EXPECT_EQ(label_2w.text.get(), upper2);

        old_name = group->person3->full_name();
        mock_label label_1wts;
        label_1wts.set_binding("text", make_name_multi(binding_mode::one_way_to_source,
                                                       std::make_shared<string_concatenation_converter>()));
        label_1wts.set_binding_context(group->person3);
        stack.add_child(label_1wts);
        EXPECT_EQ(label_1wts.text.get(), ""); // the target keeps its default
        label_1wts.text.set(old_name);
        EXPECT_EQ(label_1wts.text.get(), old_name);
        EXPECT_EQ(group->person3->full_name(), old_name);

        old_name = group->person4->full_name();
        mock_label label_1t;
        label_1t.set_binding(
            "text", make_name_multi(binding_mode::one_time, std::make_shared<string_concatenation_converter>()));
        label_1t.set_binding_context(group->person4);
        stack.add_child(label_1t);
        EXPECT_EQ(label_1t.text.get(), group->person4->full_name());
        group->person4->first_name.set("Do");
        group->person4->middle_name.set("Not");
        group->person4->last_name.set("Update");
        // changing source values must not trigger an update
        EXPECT_EQ(label_1t.text.get(), old_name);
        EXPECT_EQ(group->person4->full_name(), "Do Not Update");
        // changing the binding context must (C# re-contexts via a BindingContext binding; context
        // bindings are out of W1-02 scope, so the port re-contexts the label directly)
        label_1t.set_binding_context(group->person1);
        EXPECT_EQ(label_1t.text.get(), group->person1->full_name());
    }

    TEST(multi_binding, test_string_format)
    {
        auto vm = std::make_shared<group_view_model>();
        vm->person1->first_name.set("FOO");
        vm->person1->middle_name.set("42");
        vm->person1->last_name.set("BAZ");

        mock_label target;
        auto multi = std::make_shared<multi_binding>();
        multi->add_binding(std::make_shared<binding>("first_name"));
        multi->add_binding(std::make_shared<binding>("middle_name"));
        multi->add_binding(std::make_shared<binding>("last_name"));
        multi->set_string_format("{0} - {1} - {2}");
        target.set_binding("text", multi);
        target.set_binding_context(vm->person1);
        EXPECT_EQ(target.text.get(), "FOO - 42 - BAZ");
    }

    TEST(multi_binding, test_converter_with_string_format)
    {
        // Inner "{0:000}" zero-pads; the outer format wraps the converted result.
        auto vm = std::make_shared<person_view_model>();
        vm->first_name.set("FOO");
        vm->middle_name.set("42");
        vm->last_name.set("BAZ");

        mock_label target;
        auto multi = std::make_shared<multi_binding>();
        multi->add_binding(std::make_shared<binding>("first_name"));
        multi->add_binding(
            std::make_shared<binding>("middle_name", binding_mode::default_mode, nullptr, std::any{}, "{0:000}"));
        multi->add_binding(std::make_shared<binding>("last_name"));
        multi->set_converter(std::make_shared<string_concatenation_converter>());
        multi->set_string_format("Hello {0}");
        target.set_binding("text", multi);
        target.set_binding_context(vm);
        EXPECT_EQ(target.text.get(), "Hello FOO 042 BAZ");
    }

    TEST(multi_binding, test_relative_source_self)
    {
        // The Self leg of C# TestRelativeSources: three self-sourced inner bindings concatenated.
        const bindable_property<std::string>& family_prop = []() -> const bindable_property<std::string>& {
            static const bindable_property<std::string> d{"font_family"};
            return d;
        }();
        const bindable_property<double>& size_prop = []() -> const bindable_property<double>& {
            static const bindable_property<double> d{"font_size"};
            return d;
        }();
        struct mock_entry : element
        {
            explicit mock_entry(const bindable_property<std::string>& family_descriptor,
                                const bindable_property<double>& size_descriptor)
                : font_family{*this, family_descriptor}, font_size{*this, size_descriptor}
            {
            }
            property<std::string> font_family;
            property<double> font_size;
            property<std::string> text{*this, label_text_prop()};
        };

        mock_entry entry{family_prop, size_prop};
        entry.font_family.set("Courier New");
        entry.font_size.set(12.0);

        auto family = std::make_shared<binding>("font_family");
        family->set_source(relative_binding_source::self());
        auto size = std::make_shared<binding>("font_size");
        size->set_source(relative_binding_source::self());

        auto multi = std::make_shared<multi_binding>();
        multi->add_binding(family);
        multi->add_binding(size);
        multi->set_converter(std::make_shared<string_concatenation_converter>());
        multi->set_mode(binding_mode::two_way);
        entry.set_binding("text", multi);
        EXPECT_EQ(entry.text.get(), "Courier New 12");

        // ConvertBack must throw: the targets aren't all strings (C# asserts the throw).
        EXPECT_THROW(entry.text.set("Arial 12"), std::runtime_error);
    }

    TEST(multi_binding, clone_copies_inner_bindings)
    {
        auto multi = std::make_shared<multi_binding>();
        multi->add_binding(std::make_shared<binding>("first_name"));
        multi->add_binding(std::make_shared<binding>("last_name"));
        auto converter = std::make_shared<string_concatenation_converter>();
        multi->set_converter(converter);
        multi->set_mode(binding_mode::two_way);
        multi->set_string_format("{0}");
        multi->set_fallback_value(std::any{std::string{c_fallback}});

        const auto clone = std::static_pointer_cast<multi_binding>(multi->clone());
        EXPECT_EQ(clone->bindings().size(), 2U);
        EXPECT_NE(clone->bindings()[0], multi->bindings()[0]); // inner bindings are cloned, not shared
        EXPECT_EQ(clone->converter(), converter);
        EXPECT_EQ(clone->mode(), binding_mode::two_way);
        EXPECT_EQ(clone->string_format(), "{0}");
    }
} // namespace
