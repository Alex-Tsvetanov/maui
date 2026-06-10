// maui::controls::relative_binding_source — the Self/TemplatedParent singletons
// (relative_binding_source.hpp). Ported from src/Controls/src/Core/RelativeBindingSource.cs.
#include "maui/controls/bindings/relative_binding_source.hpp"

#include <memory>

namespace maui::controls
{
    std::shared_ptr<relative_binding_source> relative_binding_source::self()
    {
        static const std::shared_ptr<relative_binding_source> instance = std::make_shared<relative_binding_source>(
            relative_binding_source_mode::self, ancestor_predicate{}, context_predicate{}, 1);
        return instance;
    }

    std::shared_ptr<relative_binding_source> relative_binding_source::templated_parent()
    {
        static const std::shared_ptr<relative_binding_source> instance = std::make_shared<relative_binding_source>(
            relative_binding_source_mode::templated_parent, ancestor_predicate{}, context_predicate{}, 1);
        return instance;
    }
} // namespace maui::controls
