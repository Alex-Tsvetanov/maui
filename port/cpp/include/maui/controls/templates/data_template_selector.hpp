#pragma once
// maui::controls::data_template_selector  <=  Microsoft.Maui.Controls.DataTemplateSelector
//
// Selects a data_template per item: subclasses override on_select_template(item, container); callers
// go through select_template, which adds the C# validation + recycling semantics:
//   - a selector returning another selector throws std::runtime_error (C# NotSupportedException);
//   - when the container recycles templates (C#: a ListView whose CachingStrategy includes
//     RecycleElementAndDataTemplate), the selection is CACHED per item type — a non-recyclable
//     template (not type-activated, !can_recycle) throws std::runtime_error there.
//
// The data "item" is the port's universal object stand-in, bindable_object::binding_context_box
// (a shared_ptr<void> + type_tag) — the same box that becomes the templated content's
// BindingContext downstream; its type_tag is the cache key (C# item.GetType()).
//
// i_template_recycling_container is the port seam for the C# container check `container is ListView
// && (CachingStrategy & RecycleElementAndDataTemplate)`: ListView is not ported yet (this unit gates
// the collections layer), so a recycling container declares itself through this interface instead of
// a hard type test. Behavior is unchanged: no container, or one that does not implement/enable it,
// never recycles.

#include <memory>
#include <unordered_map>

#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    // The port seam for ListViewCachingStrategy.RecycleElementAndDataTemplate (see header comment).
    class i_template_recycling_container
    {
    public:
        virtual ~i_template_recycling_container() = default;

        // True when selected templates may be cached per item type and reused.
        [[nodiscard]] virtual bool recycles_data_templates() const = 0;

    protected:
        i_template_recycling_container() = default;
        i_template_recycling_container(const i_template_recycling_container&) = default;
        i_template_recycling_container(i_template_recycling_container&&) = default;
        i_template_recycling_container& operator=(const i_template_recycling_container&) = default;
        i_template_recycling_container& operator=(i_template_recycling_container&&) = default;
    };

    class data_template_selector : public data_template
    {
    public:
        using item_box = maui::core::bindable_object::binding_context_box;

        // DataTemplateSelector.SelectTemplate — validation + per-item-type recycling (header comment).
        [[nodiscard]] std::shared_ptr<data_template> select_template(const item_box& item,
                                                                     maui::core::bindable_object* container);

    protected:
        data_template_selector() = default;

        // DataTemplateSelector.OnSelectTemplate — the subclass's selection logic.
        [[nodiscard]] virtual std::shared_ptr<data_template> on_select_template(
            const item_box& item, maui::core::bindable_object* container) = 0;

    private:
        // DataTemplateSelector._dataTemplates — the per-item-type recycle cache.
        std::unordered_map<maui::core::type_tag, std::shared_ptr<data_template>> recycled_templates_;
    };
} // namespace maui::controls
