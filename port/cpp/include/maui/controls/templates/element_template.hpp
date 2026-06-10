#pragma once
// maui::controls::element_template  <=  Microsoft.Maui.Controls.ElementTemplate
//
// The base of data_template and control_template: it wraps the content-creator function (C#'s
// Func<object> LoadTemplate — the port's stand-in for both the Func<object> ctor and the
// reflection-driven Type ctor) and implements CreateContent. The created content is returned as an
// OWNING shared_ptr<bindable_object> (the C# "object"): the template mints the subtree root, so
// ownership flows to the caller (a templated control stores it as its template root, PROFILE §8).
//
// The reflection Type ctor maps onto the typed factory each subclass exposes
// (`data_template::of<TControl>()` / `control_template::of<TControl>()`): it loads via
// make_shared<TControl> and marks the template recyclable (C# _canRecycle, aka IsDeclarative — only
// type-activated templates may be recycled by RecycleElementAndDataTemplate).
//
// Out of scope (documented): the IElementDefinition.Parent / resources-changed listener plumbing —
// the port's resource changes flow through the element tree (element.hpp), templates hold no
// resource scope of their own.

#include <memory>
#include <string>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::controls
{
    class element_template
    {
    public:
        // C# Func<object> LoadTemplate — returns the freshly created content (an owning pointer).
        using loader = maui::core::move_only_function<std::shared_ptr<maui::core::bindable_object>()>;

        element_template(const element_template&) = delete;
        element_template(element_template&&) = delete;
        element_template& operator=(const element_template&) = delete;
        element_template& operator=(element_template&&) = delete;
        virtual ~element_template() = default;

        // C# LoadTemplate { get; set; } (null = unset).
        [[nodiscard]] bool has_load_template() const
        {
            return static_cast<bool>(load_template_);
        }
        void set_load_template(loader load_template)
        {
            load_template_ = std::move(load_template);
        }

        // ElementTemplate.CreateContent: run the loader, stage the content (data_template applies its
        // Values/Bindings via setup_content), and mark an element root as IsTemplateRoot. With no
        // loader, returns a default label instead of throwing (the C# HotReload-transition guard).
        // Throws std::runtime_error (C# InvalidOperationException) when called on a
        // data_template_selector — select a concrete template first.
        [[nodiscard]] std::shared_ptr<maui::core::bindable_object> create_content() const;

        // ElementTemplate.CanRecycle (aka IsDeclarative): true only for type-activated templates
        // (the of<TControl>() factories) — the RecycleElementAndDataTemplate gate.
        [[nodiscard]] bool can_recycle() const
        {
            return can_recycle_;
        }

    protected:
        element_template() = default;
        // The Func<object> ctor (throwing std::invalid_argument on a null loader — C#
        // ArgumentNullException) and the type-activated form (can_recycle = true) — both reached
        // through the subclasses' public ctors/factories.
        explicit element_template(loader load_template, bool can_recycle = false);

        // ElementTemplate.SetupContent — data_template overrides to apply its Bindings + Values.
        virtual void setup_content(maui::core::bindable_object& item) const
        {
            (void)item;
        }

    private:
        loader load_template_;
        bool can_recycle_ = false;
    };
} // namespace maui::controls
