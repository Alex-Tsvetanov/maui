// Tests for the handler seam (#22): the CRTP view_handler base, the property/command mappers, and the
// type-keyed handler/service registries. There is no concrete control yet (that is M2), so a mock
// handler over a fake platform view exercises the infrastructure: connect creates + connects the
// platform view and runs the mapper; update_value re-runs one mapper; invoke runs the command mapper;
// disconnect tears down. Plus direct tests of mapper chaining/override, type_tag identity, and the
// registries.
#include "maui/core/flow_direction.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/service_registry.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/core/view_handler.hpp"

#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::core::command_mapper;
    using maui::core::handler_registry;
    using maui::core::handler_state;
    using maui::core::i_element;
    using maui::core::i_element_handler;
    using maui::core::i_view;
    using maui::core::i_view_handler;
    using maui::core::layout_alignment;
    using maui::core::property_mapper;
    using maui::core::service_registry;
    using maui::core::thickness;
    using maui::core::type_tag;
    using maui::core::view_handler;
    using maui::graphics::rect;
    using maui::graphics::size;

    // A minimal concrete i_view: only width is interesting (it is what the mock handler maps); every
    // other member returns a trivial default.
    struct stub_view : i_view
    {
        // i_element
        [[nodiscard]] const std::shared_ptr<i_element_handler>& handler() const override
        {
            return handler_;
        }
        void set_handler(std::shared_ptr<i_element_handler> value) override
        {
            handler_ = std::move(value);
        }
        [[nodiscard]] std::shared_ptr<i_element> parent() const override
        {
            return nullptr;
        }
        // i_transform
        [[nodiscard]] double translation_x() const override
        {
            return 0;
        }
        [[nodiscard]] double translation_y() const override
        {
            return 0;
        }
        [[nodiscard]] double scale() const override
        {
            return 1;
        }
        [[nodiscard]] double scale_x() const override
        {
            return 1;
        }
        [[nodiscard]] double scale_y() const override
        {
            return 1;
        }
        [[nodiscard]] double rotation() const override
        {
            return 0;
        }
        [[nodiscard]] double rotation_x() const override
        {
            return 0;
        }
        [[nodiscard]] double rotation_y() const override
        {
            return 0;
        }
        [[nodiscard]] double anchor_x() const override
        {
            return 0.5;
        }
        [[nodiscard]] double anchor_y() const override
        {
            return 0.5;
        }
        // i_view
        [[nodiscard]] std::string_view automation_id() const override
        {
            return "";
        }
        [[nodiscard]] maui::core::flow_direction flow_direction() const override
        {
            return maui::core::flow_direction::match_parent;
        }
        [[nodiscard]] layout_alignment horizontal_layout_alignment() const override
        {
            return layout_alignment::fill;
        }
        [[nodiscard]] layout_alignment vertical_layout_alignment() const override
        {
            return layout_alignment::fill;
        }
        [[nodiscard]] maui::core::semantics* semantics() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::graphics::i_shape* clip() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::i_shadow* shadow() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::graphics::paint* background() const override
        {
            return nullptr;
        }
        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return maui::core::visibility::visible;
        }
        [[nodiscard]] double opacity() const override
        {
            return 1.0;
        }
        [[nodiscard]] bool is_enabled() const override
        {
            return true;
        }
        [[nodiscard]] bool is_focused() const override
        {
            return false;
        }
        void set_is_focused(bool /*value*/) override
        {
        }
        [[nodiscard]] bool input_transparent() const override
        {
            return false;
        }
        [[nodiscard]] rect frame() const override
        {
            return frame_;
        }
        void set_frame(rect value) override
        {
            frame_ = value;
        }
        [[nodiscard]] double width() const override
        {
            return width_;
        }
        [[nodiscard]] double minimum_width() const override
        {
            return 0;
        }
        [[nodiscard]] double maximum_width() const override
        {
            return std::numeric_limits<double>::infinity();
        }
        [[nodiscard]] double height() const override
        {
            return 50;
        }
        [[nodiscard]] double minimum_height() const override
        {
            return 0;
        }
        [[nodiscard]] double maximum_height() const override
        {
            return std::numeric_limits<double>::infinity();
        }
        [[nodiscard]] thickness margin() const override
        {
            return {};
        }
        [[nodiscard]] size desired_size() const override
        {
            return {};
        }
        [[nodiscard]] int z_index() const override
        {
            return 0;
        }
        size arrange(const rect& bounds) override
        {
            frame_ = bounds;
            return {bounds.width, bounds.height};
        }
        size measure(double width_constraint, double height_constraint) override
        {
            return {width_constraint, height_constraint};
        }
        void invalidate_measure() override
        {
        }
        void invalidate_arrange() override
        {
        }
        bool focus() override
        {
            return true;
        }
        void unfocus() override
        {
        }

        double width_ = 100;
        rect frame_;
        std::shared_ptr<i_element_handler> handler_;
    };

    // The fake "native" view the headless handler manages.
    struct fake_platform
    {
        int width = 0;
        bool connected = false;
    };

    // A control key (the registry only uses its type identity).
    struct fake_control
    {
    };

    // The mock handler under test. Maps IView.Width onto fake_platform.width, and a "ping" command.
    struct test_handler : view_handler<test_handler, i_view, fake_platform>
    {
        test_handler() : view_handler(&property_map(), &command_map())
        {
        }

        // Mapper tables (defined out-of-class so the lambdas see a complete test_handler).
        static property_mapper<i_view, test_handler>& property_map();
        static command_mapper<i_view, test_handler>& command_map();

        static std::unique_ptr<fake_platform> create_platform_view()
        {
            return std::make_unique<fake_platform>();
        }
        void on_connect_handler(fake_platform& platform)
        {
            platform.connected = true;
            ++connects;
        }
        void on_disconnect_handler(fake_platform& platform)
        {
            platform.connected = false;
            ++disconnects;
        }

        [[nodiscard]] size get_desired_size(double width_constraint, double height_constraint) const override
        {
            return {width_constraint, height_constraint};
        }
        void platform_arrange(const rect& frame) override
        {
            arranged = frame;
        }

        int connects = 0;
        int disconnects = 0;
        int width_maps = 0;
        int pings = 0;
        rect arranged;
    };

    property_mapper<i_view, test_handler>& test_handler::property_map()
    {
        static property_mapper<i_view, test_handler> map{
            {"width",
             [](test_handler& handler, i_view& view) {
                 ++handler.width_maps;
                 if (auto* platform = handler.typed_platform_view())
                 {
                     platform->width = static_cast<int>(view.width());
                 }
             }},
        };
        return map;
    }

    command_mapper<i_view, test_handler>& test_handler::command_map()
    {
        static command_mapper<i_view, test_handler> map{
            {"ping", [](test_handler& handler, i_view& /*view*/, const std::any& /*args*/) { ++handler.pings; }},
        };
        return map;
    }

    TEST(handler, connect_creates_and_connects_platform_view)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);

        EXPECT_NE(handler.platform_view(), nullptr);
        EXPECT_EQ(handler.typed_platform_view()->connected, true);
        EXPECT_EQ(handler.connects, 1);
        EXPECT_EQ(handler.state(), handler_state::connected);
        EXPECT_EQ(handler.virtual_view(), &view);
        // The full mapper ran on connect, pushing width onto the platform view.
        EXPECT_GE(handler.width_maps, 1);
        EXPECT_EQ(handler.typed_platform_view()->width, 100);
    }

    TEST(handler, update_value_runs_one_mapper)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);
        const int maps_after_connect = handler.width_maps;

        view.width_ = 250;
        handler.update_value("width");

        EXPECT_EQ(handler.width_maps, maps_after_connect + 1);
        EXPECT_EQ(handler.typed_platform_view()->width, 250);
    }

    TEST(handler, update_value_unknown_property_is_noop)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);
        const int maps_after_connect = handler.width_maps;

        handler.update_value("does_not_exist");

        EXPECT_EQ(handler.width_maps, maps_after_connect);
    }

    TEST(handler, update_value_before_connect_is_noop)
    {
        test_handler handler;
        handler.update_value("width"); // no virtual view yet
        EXPECT_EQ(handler.width_maps, 0);
    }

    TEST(handler, invoke_runs_command_mapper)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);

        handler.invoke("ping");
        EXPECT_EQ(handler.pings, 1);
        handler.invoke("ping", std::any{42});
        EXPECT_EQ(handler.pings, 2);
    }

    TEST(handler, invoke_unknown_command_is_noop)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);

        handler.invoke("nope");
        EXPECT_EQ(handler.pings, 0);
    }

    TEST(handler, disconnect_clears_state)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);

        handler.disconnect_handler();

        EXPECT_EQ(handler.platform_view(), nullptr);
        EXPECT_EQ(handler.virtual_view(), nullptr);
        EXPECT_EQ(handler.disconnects, 1);
        EXPECT_EQ(handler.state(), handler_state::disconnected);
    }

    TEST(handler, set_virtual_view_is_idempotent)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);
        handler.set_virtual_view(view); // same view → no second connect

        EXPECT_EQ(handler.connects, 1);
    }

    TEST(handler, reconnect_after_disconnect_creates_new_platform_view)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);
        handler.disconnect_handler();
        handler.set_virtual_view(view); // reconnect

        EXPECT_NE(handler.platform_view(), nullptr);
        EXPECT_EQ(handler.connects, 2);
        EXPECT_EQ(handler.state(), handler_state::connected);
    }

    TEST(handler, virtual_view_is_covariant_through_interfaces)
    {
        test_handler handler;
        stub_view view;
        handler.set_virtual_view(view);

        i_view_handler& as_view_handler = handler;
        i_element_handler& as_element_handler = handler;
        EXPECT_EQ(as_view_handler.virtual_view(), &view);    // i_view*
        EXPECT_EQ(as_element_handler.virtual_view(), &view); // i_element* (same object)
    }

    TEST(property_mapper_test, chaining_runs_chained_then_own_in_order)
    {
        property_mapper<i_view, test_handler> base_map{
            {"a", [](test_handler& handler, i_view& /*view*/) { ++handler.connects; }},
        };
        property_mapper<i_view, test_handler> const derived_map{
            base_map,
            {{"b", [](test_handler& handler, i_view& /*view*/) { ++handler.disconnects; }}},
        };

        // Keys: chained ("a") first, then own ("b").
        const std::vector<std::string> expected{"a", "b"};
        EXPECT_EQ(derived_map.keys(), expected);

        test_handler handler;
        stub_view view;
        derived_map.update_properties(handler, view);
        EXPECT_EQ(handler.connects, 1);    // "a" (from the chained mapper)
        EXPECT_EQ(handler.disconnects, 1); // "b" (own)
    }

    TEST(property_mapper_test, own_key_overrides_chained)
    {
        property_mapper<i_view, test_handler> base_map{
            {"a", [](test_handler& handler, i_view& /*view*/) { ++handler.connects; }},
        };
        property_mapper<i_view, test_handler> const derived_map{
            base_map,
            {{"a", [](test_handler& handler, i_view& /*view*/) { ++handler.pings; }}},
        };

        test_handler handler;
        stub_view view;
        derived_map.update_property(handler, view, "a");
        EXPECT_EQ(handler.pings, 1);    // own "a" won
        EXPECT_EQ(handler.connects, 0); // chained "a" did not run
    }

    TEST(type_tag_test, identity_is_per_type_and_hashable)
    {
        EXPECT_EQ(type_tag::of<int>(), type_tag::of<int>());
        EXPECT_NE(type_tag::of<int>(), type_tag::of<double>());
        EXPECT_NE(type_tag::of<stub_view>(), type_tag::of<fake_control>());

        std::unordered_map<type_tag, int> by_type;
        by_type[type_tag::of<int>()] = 1;
        by_type[type_tag::of<double>()] = 2;
        EXPECT_EQ(by_type.at(type_tag::of<int>()), 1);
        EXPECT_EQ(by_type.at(type_tag::of<double>()), 2);
        EXPECT_EQ(by_type.size(), 2U);
    }

    TEST(handler_registry_test, register_and_create)
    {
        handler_registry registry;
        EXPECT_FALSE(registry.is_registered(type_tag::of<fake_control>()));

        maui::core::register_handler<fake_control, test_handler>(registry);
        EXPECT_TRUE(registry.is_registered(type_tag::of<fake_control>()));

        std::unique_ptr<i_element_handler> const created = registry.create_handler<fake_control>();
        ASSERT_NE(created, nullptr);
        EXPECT_NE(dynamic_cast<test_handler*>(created.get()), nullptr);
    }

    TEST(handler_registry_test, create_unregistered_returns_null)
    {
        const handler_registry registry;
        EXPECT_EQ(registry.create_handler<fake_control>(), nullptr);
    }

    TEST(handler_registry_test, self_registration_populates_default_registry)
    {
        // fake_control -> test_handler is self-registered at file scope (MAUI_REGISTER_HANDLER, below).
        auto created = maui::core::default_handler_registry().create_handler<fake_control>();
        ASSERT_NE(created, nullptr);
        EXPECT_NE(dynamic_cast<test_handler*>(created.get()), nullptr);
    }

    struct fake_service
    {
        int value = 42;
    };
    struct other_service
    {
    };

    TEST(service_registry_test, add_and_resolve)
    {
        service_registry registry;
        registry.add_singleton<fake_service>(std::make_shared<fake_service>());

        const auto resolved = registry.get_service<fake_service>();
        ASSERT_NE(resolved, nullptr);
        EXPECT_EQ(resolved->value, 42);
        EXPECT_EQ(registry.get_service<other_service>(), nullptr);
    }

    TEST(service_registry_test, get_required_throws_when_missing)
    {
        const service_registry registry;
        EXPECT_THROW((void)registry.get_required_service<other_service>(), std::runtime_error);
    }
} // namespace

// Opt-in self-registration (exercised above). Anonymous-namespace types are visible here in-TU.
MAUI_REGISTER_HANDLER(fake_control, test_handler)
