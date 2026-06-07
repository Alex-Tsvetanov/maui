#pragma once
// maui::core::view_handler<Derived, Virtual, Platform>  <=  Microsoft.Maui.Handlers.ElementHandler +
// ViewHandler (the generic base).
//
// The reusable core of the handler recipe (PROFILE §5): a CRTP base owning the
// connect → create-platform-view → run-mapper → connected lifecycle, typed by the virtual-view and
// platform-view types, WITHOUT putting templates behind a vtable (the §5 "avoid vtable-through-
// templates" rule). Concrete handlers derive as
//   class button_handler : public view_handler<button_handler, i_button, native_button>
// supply the create/connect/disconnect hooks plus the get_desired_size/platform_arrange overrides, and
// pass their static property/command mappers to the base ctor. Ported from ElementHandler.cs +
// ViewHandler.cs.
//
// Derived must provide:
//   std::unique_ptr<Platform> create_platform_view();                       // C# OnCreatePlatformView
// and override i_view_handler::get_desired_size / platform_arrange. Derived MAY shadow these optional
// hooks (declare them PUBLIC so the base can call them through the CRTP self-reference):
//   void on_connect_handler(Platform&);  void on_disconnect_handler(Platform&);  // C# OnConnect/Disconnect
//   void on_setup_container();           void on_remove_container();
//
// Ownership (PROFILE §8): the handler OWNS its platform view (unique_ptr — headless has no native view
// tree to retain it; real backends override creation to hand ownership to the superview). The
// virtual-view back-reference is non-owning (the view owns the handler). The reciprocal
// `view.Handler = this` link is established by the hosting layer (M2), not here.

#include <any>
#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_maui_context;

    // C# ElementHandlerState — the connect/disconnect lifecycle phase (platform mappers read it to
    // skip default-valued work while connecting; see is_connecting()).
    enum class handler_state : std::uint8_t
    {
        disconnected = 0,
        connecting,
        connected,
        reconnecting,
    };

    template <class Derived, class Virtual, class Platform> class view_handler : public i_view_handler
    {
        static_assert(std::is_base_of_v<i_view, Virtual>, "Virtual must derive maui::core::i_view");

    public:
        // ---- i_element_handler ----
        void set_maui_context(i_maui_context* context) override
        {
            maui_context_ = context;
        }

        // C# ElementHandler.SetVirtualView: create the platform view on first connect, connect once,
        // then run the full property mapper. Re-running with the same view is a no-op.
        void set_virtual_view(i_element& view) override
        {
            auto* incoming = dynamic_cast<Virtual*>(&view);
            if (virtual_view_ == incoming)
            {
                return;
            }
            const bool first_setup = (virtual_view_ == nullptr);
            virtual_view_ = incoming;

            if (!platform_view_)
            {
                state_ = handler_state::connecting;
                platform_view_ = derived().create_platform_view();
            }
            else
            {
                state_ = handler_state::reconnecting;
            }

            if (first_setup && platform_view_)
            {
                derived().on_connect_handler(*platform_view_);
            }

            if (property_mapper_ != nullptr && virtual_view_ != nullptr)
            {
                property_mapper_->update_properties(*this, *virtual_view_);
            }

            state_ = handler_state::connected;
        }

        void update_value(std::string_view property) override
        {
            if (virtual_view_ == nullptr || property_mapper_ == nullptr)
            {
                return;
            }
            property_mapper_->update_property(*this, *virtual_view_, property);
        }

        void invoke(std::string_view command, const std::any& args = {}) override
        {
            if (virtual_view_ == nullptr || command_mapper_ == nullptr)
            {
                return;
            }
            command_mapper_->invoke(*this, *virtual_view_, command, args);
        }

        // C# ElementHandler.DisconnectHandler: tear down through the (now-isolated) platform view, then
        // clear both references. Idempotent — a no-op once disconnected.
        void disconnect_handler() override
        {
            if (platform_view_ && virtual_view_ != nullptr)
            {
                const std::unique_ptr<Platform> old = std::move(platform_view_);
                derived().on_disconnect_handler(*old);
                virtual_view_ = nullptr;
            }
            state_ = handler_state::disconnected;
        }

        [[nodiscard]] void* platform_view() const override
        {
            return platform_view_.get();
        }
        // Covariant narrowing all the way to Virtual (overrides i_view_handler's i_view* override,
        // which overrides i_element_handler's i_element* — a covariance chain).
        [[nodiscard]] Virtual* virtual_view() const override
        {
            return virtual_view_;
        }
        [[nodiscard]] i_maui_context* maui_context() const override
        {
            return maui_context_;
        }

        // ---- i_view_handler ----
        [[nodiscard]] bool has_container() const override
        {
            return has_container_;
        }
        void set_has_container(bool value) override
        {
            if (has_container_ == value)
            {
                return;
            }
            has_container_ = value;
            if (value)
            {
                derived().on_setup_container();
            }
            else
            {
                derived().on_remove_container();
            }
        }
        [[nodiscard]] void* container_view() const override
        {
            return container_view_;
        }

        // get_desired_size / platform_arrange stay pure here — Derived overrides them (mirrors C#'s
        // abstract GetDesiredSize / PlatformArrange).

        // ---- typed accessors for Derived's hooks/mappers ----
        [[nodiscard]] Platform* typed_platform_view() const
        {
            return platform_view_.get();
        }
        [[nodiscard]] handler_state state() const
        {
            return state_;
        }
        // C# IsConnectingHandler() — platform mappers use it to skip default-valued work on connect.
        [[nodiscard]] bool is_connecting() const
        {
            return state_ == handler_state::connecting;
        }

    protected:
        // Default optional hooks (headless: no native connect/teardown, no container). Derived may
        // shadow any of these with a PUBLIC method of the same name.
        void on_connect_handler(Platform& /*platform_view*/)
        {
        }
        void on_disconnect_handler(Platform& /*platform_view*/)
        {
        }
        void on_setup_container()
        {
        }
        void on_remove_container()
        {
        }

        void set_container_view(void* container)
        {
            container_view_ = container;
        }

    private:
        // CRTP safety (bugprone-crtp-constructor-accessibility): only Derived may construct the base
        // subobject, which prevents instantiating view_handler with the wrong Derived parameter.
        friend Derived;
        view_handler(property_mapper_base* property_mapper, command_mapper_base* command_mapper)
            : property_mapper_(property_mapper), command_mapper_(command_mapper)
        {
        }

        [[nodiscard]] Derived& derived() noexcept
        {
            return static_cast<Derived&>(*this);
        }

        property_mapper_base* property_mapper_;
        command_mapper_base* command_mapper_;
        i_maui_context* maui_context_ = nullptr;
        std::unique_ptr<Platform> platform_view_;
        Virtual* virtual_view_ = nullptr;
        void* container_view_ = nullptr;
        bool has_container_ = false;
        handler_state state_ = handler_state::disconnected;
    };
} // namespace maui::core
