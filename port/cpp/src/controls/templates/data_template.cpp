// maui::controls::data_template — the id surface + SetupContent (data_template.hpp). Ported from
// DataTemplate.cs (the Values/Bindings application) and IDataTemplateController.cs.
#include "maui/controls/templates/data_template.hpp"

#include <atomic>
#include <stdexcept>
#include <string>
#include <utility>

#include "maui/controls/templates/element_template.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    namespace
    {
        constexpr int k_id_counter_start = 100; // C# DataTemplate.idCounter — first id is 101
    } // namespace

    int data_template::next_id()
    {
        static std::atomic<int> counter{k_id_counter_start}; // C# Interlocked.Increment(ref idCounter)
        return ++counter;
    }

    std::string data_template::make_type_id_string()
    {
        // The Type-ctor IdString (C# type.FullName): one stable string per TControl — minted once per
        // of<TControl>() instantiation (its function-local static), so every instance for the same
        // control type shares it, which is the property RecycleElementAndDataTemplate keys on.
        return "maui::controls::data_template/type#" + std::to_string(next_id());
    }

    data_template::data_template() : id_(next_id())
    {
        id_string_ = "maui::controls::data_template" + std::to_string(id_); // C# GetType().FullName + id
    }

    data_template::data_template(loader load_template) : element_template(std::move(load_template)), id_(next_id())
    {
        id_string_ = "maui::controls::data_template" + std::to_string(id_);
    }

    data_template::data_template(loader load_template, std::string id_string, maui::core::type_tag content_type)
        : element_template(std::move(load_template), /*can_recycle=*/true), id_(next_id()),
          id_string_(std::move(id_string)), content_type_(content_type)
    {
    }

    void data_template::setup_content(maui::core::bindable_object& item) const
    {
        // ApplyBindings then ApplyValues (DataTemplate.SetupContent order).
        for (const auto& [name, apply_binding] : bindings_)
        {
            if (values_.contains(name))
            {
                // C# InvalidOperationException("Binding and Value found for " + PropertyName).
                throw std::runtime_error("Binding and Value found for " + name);
            }
            apply_binding(item);
        }
        for (const auto& [name, value] : values_)
        {
            item.apply_setter(name, value, maui::core::setter_specificity::manual_value_setter);
        }
        // Setup actions (add_setup) run LAST — the C# lambda template body's per-instance configuration,
        // after the staged Values/Bindings so a setup can read or override them (data_template.hpp note).
        for (const auto& setup : setups_)
        {
            setup(item);
        }
    }
} // namespace maui::controls
