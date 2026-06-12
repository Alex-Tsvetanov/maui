#pragma once
// maui::controls::groupable_items_view  <=  Microsoft.Maui.Controls.GroupableItemsView
//
// Adds grouping over selectable_items_view: IsGrouped (the source is then read as groups — the
// item_collection<grouping_ptr> flavor) plus the group header/footer templates (their binding
// context is the group's key object; see item_collection.hpp).

#include <memory>
#include <utility>

#include "maui/controls/items/selectable_items_view.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class groupable_items_view : public selectable_items_view
    {
    public:
        groupable_items_view() = default;

        static const maui::core::bindable_property<bool>& is_grouped_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& group_header_template_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& group_footer_template_property();

        [[nodiscard]] bool is_grouped() const
        {
            return is_grouped_.get();
        }
        void set_is_grouped(bool value)
        {
            is_grouped_.set(value);
        }

        [[nodiscard]] const std::shared_ptr<data_template>& group_header_template() const
        {
            return group_header_template_.get();
        }
        void set_group_header_template(std::shared_ptr<data_template> value)
        {
            group_header_template_.set(std::move(value));
        }

        [[nodiscard]] const std::shared_ptr<data_template>& group_footer_template() const
        {
            return group_footer_template_.get();
        }
        void set_group_footer_template(std::shared_ptr<data_template> value)
        {
            group_footer_template_.set(std::move(value));
        }

    private:
        maui::core::property<bool> is_grouped_{*this, is_grouped_property()};
        maui::core::property<std::shared_ptr<data_template>> group_header_template_{*this,
                                                                                    group_header_template_property()};
        maui::core::property<std::shared_ptr<data_template>> group_footer_template_{*this,
                                                                                    group_footer_template_property()};
    };
} // namespace maui::controls
