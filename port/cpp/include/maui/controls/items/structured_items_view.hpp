#pragma once
// maui::controls::structured_items_view  <=  Microsoft.Maui.Controls.StructuredItemsView
//
// Adds the structural chrome over items_view: Header/Footer (boxed objects, like EmptyView) with
// their templates, the public ItemsLayout face over the internal slot, and ItemSizingStrategy.
// First CONCRETE level of the hierarchy (C# news it up in tests).

#include <memory>
#include <utility>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/items/items_layout.hpp"
#include "maui/controls/items/items_view.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class structured_items_view : public items_view
    {
    public:
        structured_items_view() = default;

        static const maui::core::bindable_property<boxed_item>& header_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& header_template_property();
        static const maui::core::bindable_property<boxed_item>& footer_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& footer_template_property();
        static const maui::core::bindable_property<controls::item_sizing_strategy>& item_sizing_strategy_property();

        [[nodiscard]] const boxed_item& header() const
        {
            return header_.get();
        }
        void set_header(boxed_item value)
        {
            header_.set(std::move(value));
        }

        [[nodiscard]] const std::shared_ptr<data_template>& header_template() const
        {
            return header_template_.get();
        }
        void set_header_template(std::shared_ptr<data_template> value)
        {
            header_template_.set(std::move(value));
        }

        [[nodiscard]] const boxed_item& footer() const
        {
            return footer_.get();
        }
        void set_footer(boxed_item value)
        {
            footer_.set(std::move(value));
        }

        [[nodiscard]] const std::shared_ptr<data_template>& footer_template() const
        {
            return footer_template_.get();
        }
        void set_footer_template(std::shared_ptr<data_template> value)
        {
            footer_template_.set(std::move(value));
        }

        // StructuredItemsView.ItemsLayout — the public face of the internal slot (never null).
        [[nodiscard]] const std::shared_ptr<controls::items_layout>& items_layout() const
        {
            return internal_items_layout();
        }
        void set_items_layout(std::shared_ptr<controls::items_layout> value)
        {
            set_internal_items_layout(std::move(value));
        }

        [[nodiscard]] controls::item_sizing_strategy item_sizing_strategy() const
        {
            return item_sizing_strategy_.get();
        }
        void set_item_sizing_strategy(controls::item_sizing_strategy value)
        {
            item_sizing_strategy_.set(value);
        }

    private:
        maui::core::property<boxed_item> header_{*this, header_property()};
        maui::core::property<std::shared_ptr<data_template>> header_template_{*this, header_template_property()};
        maui::core::property<boxed_item> footer_{*this, footer_property()};
        maui::core::property<std::shared_ptr<data_template>> footer_template_{*this, footer_template_property()};
        maui::core::property<controls::item_sizing_strategy> item_sizing_strategy_{*this,
                                                                                   item_sizing_strategy_property()};
    };
} // namespace maui::controls
