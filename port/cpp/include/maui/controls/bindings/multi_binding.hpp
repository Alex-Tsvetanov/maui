#pragma once
// maui::controls::multi_binding  <=  Microsoft.Maui.Controls.MultiBinding
//
// Combines several inner bindings into one target value through an i_multi_value_converter (or a
// composite StringFormat). The implementation mirrors C# exactly: a hidden PROXY element carries one
// dynamically-created property per inner binding ("mb-proxy0"...), each inner binding is set on the
// proxy through the normal element::set_binding machinery (so relative sources, modes, and converters
// all behave), the proxy's binding context tracks the target's, and any proxy-property change
// recomputes the combined value (guarded by the C# `_applying` flag). ConvertBack distributes a
// target change across the proxies, which the inner two-way bindings push to their sources.
//
// Sentinels (see i_multi_value_converter.hpp): convert may answer do_nothing (no update) or
// unset_value (use FallbackValue); convert_back may answer nullopt (no source updates) or per-element
// sentinels (skip that source).

#include <any>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/bindings/binding_base.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class i_multi_value_converter;

    class multi_binding final : public binding_base
    {
    public:
        multi_binding();
        ~multi_binding() override;
        multi_binding(const multi_binding&) = delete;
        multi_binding(multi_binding&&) = delete;
        multi_binding& operator=(const multi_binding&) = delete;
        multi_binding& operator=(multi_binding&&) = delete;

        [[nodiscard]] const std::shared_ptr<i_multi_value_converter>& converter() const
        {
            return converter_;
        }
        void set_converter(std::shared_ptr<i_multi_value_converter> value);

        [[nodiscard]] const std::any& converter_parameter() const
        {
            return converter_parameter_;
        }
        void set_converter_parameter(std::any value);

        // The inner bindings, combined in order (C# MultiBinding.Bindings).
        [[nodiscard]] const std::vector<std::shared_ptr<binding_base>>& bindings() const
        {
            return bindings_;
        }
        void add_binding(std::shared_ptr<binding_base> value);
        void set_bindings(std::vector<std::shared_ptr<binding_base>> value);

        [[nodiscard]] std::shared_ptr<binding_base> clone() const override;

        // ---- the internal seam ----
        void apply(const maui::core::bindable_object::binding_context_box& context, maui::core::bindable_object& target,
                   std::string_view target_property, bool from_binding_context_changed,
                   maui::core::setter_specificity specificity) override;
        void apply(bool from_target) override;
        void unapply(bool from_binding_context_changed = false) override;

    private:
        class proxy_element; // the hidden per-binding property host (defined in the .cpp)

        [[nodiscard]] maui::core::binding_mode realized_mode() const;
        // MultiBinding.GetSourceValue over the value array: converter / composite StringFormat,
        // unset_value -> FallbackValue, then the base (TargetNullValue + StringFormat-on-result).
        [[nodiscard]] std::any compute_source_value(maui::core::type_tag target_type) const;
        void push_to_target(maui::core::setter_specificity specificity);
        void apply_back_to_proxies();
        void on_proxy_changed();

        std::shared_ptr<i_multi_value_converter> converter_;
        std::any converter_parameter_;
        std::vector<std::shared_ptr<binding_base>> bindings_;

        std::unique_ptr<proxy_element> proxy_;
        maui::core::bindable_object* target_ = nullptr;
        std::weak_ptr<void> target_alive_;
        std::string target_property_;
        maui::core::setter_specificity specificity_;
        bool applying_ = false;
    };
} // namespace maui::controls
