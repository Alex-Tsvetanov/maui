// maui::controls::detail::element_animations — see element_animations.hpp.
#include "element_animations.hpp"

namespace maui::controls::detail
{
    element_animations::~element_animations()
    {
        for (const auto& [name, kinetic] : kinetics)
        {
            if (!kinetic)
            {
                continue;
            }
            if (const auto manager = kinetic->animation_manager())
            {
                manager->remove(*kinetic);
            }
        }
        // `animations` cleans itself up: each tweener's destructor detaches from the manager.
    }
} // namespace maui::controls::detail
