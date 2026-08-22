// view_size_ops — the DEFAULT (headless / apple / ios / windows) definition: a no-op.
//
// On those backends the leaf desired-size path already resolves Minimum*Request — apple/ios literally
// (ViewHandlerExtensions.iOS.cs:125-126 ResolveConstraints, applied unconditionally), headless and
// windows through the same cross-platform clamp in view<>::measure — so a second native push would be
// redundant, and there is no native floor for a headless view to carry anyway. Android is the backend
// where the minimum has NOWHERE ELSE to live and replaces this unit: src/platform/android/view_size_ops.cpp
// (see maui/core/view_size_ops.hpp for the C# derivation).

#include "maui/core/view_size_ops.hpp"

namespace maui::core
{
    void apply_native_minimum_size(void* /*native_view*/, double /*minimum_width*/, double /*minimum_height*/)
    {
    }
} // namespace maui::core
