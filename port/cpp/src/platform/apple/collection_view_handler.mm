// collection_view_handler — Apple (AppKit / macOS) platform partial: the MINIMAL wave-2 stub. The
// cross-platform simulator (src/controls/items/collection_view_handler.cpp) runs unchanged on this
// backend as the state mirror; `native` holds a plain placeholder NSView so the handler composes
// into a real view tree. The REAL NSCollectionView host lands in wave 3 — per-backend members then
// join collection_view_platform inside its #ifdef MAUI_PLATFORM_APPLE block. Obj-C++ with ARC.

#import <AppKit/AppKit.h>

#include <memory>

#include "maui/controls/items/collection_view_handler.hpp"

namespace maui::controls
{
    collection_view_platform::~collection_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<collection_view_platform> collection_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<collection_view_platform>();
        NSView* placeholder = [[NSView alloc] initWithFrame:NSZeroRect];
        platform->native = (__bridge_retained void*)placeholder;
        return platform;
    }
} // namespace maui::controls
