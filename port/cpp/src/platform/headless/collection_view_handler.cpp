// collection_view_handler — headless platform partial. The ENTIRE fake-viewport virtualization
// simulator is cross-platform (src/controls/items/collection_view_handler.cpp); the headless backend
// only mints the bare platform struct (no native view behind `native`).

#include "maui/controls/items/collection_view_handler.hpp"

#include <memory>

namespace maui::controls
{
    collection_view_platform::~collection_view_platform() = default;

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        return std::make_unique<collection_view_platform>();
    }
} // namespace maui::controls
