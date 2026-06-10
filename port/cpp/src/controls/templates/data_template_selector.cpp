// maui::controls::data_template_selector — SelectTemplate's validation + recycling cache
// (data_template_selector.hpp). Ported from DataTemplateSelector.cs.
#include "maui/controls/templates/data_template_selector.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    std::shared_ptr<data_template> data_template_selector::select_template(const item_box& item,
                                                                           maui::core::bindable_object* container)
    {
        // C#: container is ListView && (CachingStrategy & RecycleElementAndDataTemplate) — the port
        // seam (header comment). An untyped item (no type_tag) cannot key the cache.
        const auto* recycler = dynamic_cast<const i_template_recycling_container*>(container);
        const bool recycle = recycler != nullptr && recycler->recycles_data_templates() && item.type.has_value();

        if (recycle)
        {
            if (const auto found = recycled_templates_.find(*item.type); found != recycled_templates_.end())
            {
                return found->second;
            }
        }

        std::shared_ptr<data_template> selected = on_select_template(item, container);
        if (dynamic_cast<data_template_selector*>(selected.get()) != nullptr)
        {
            // C# NotSupportedException.
            throw std::runtime_error(
                "data_template_selector.on_select_template must not return another data_template_selector");
        }

        if (recycle)
        {
            if (selected == nullptr || !selected->can_recycle())
            {
                // C# NotSupportedException.
                throw std::runtime_error(
                    "RecycleElementAndDataTemplate requires a data_template activated with the type factory");
            }
            recycled_templates_[*item.type] = selected;
        }

        return selected;
    }
} // namespace maui::controls
