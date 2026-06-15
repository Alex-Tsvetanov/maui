// font_manager — HEADLESS backend partial: the deterministic fake. There is no native font tree, so
// create_font hands back a small heap-allocated SENTINEL (a distinct non-null handle per resolved font)
// that the manager owns + frees via its disposer; default_font / default_font_size mirror C#
// FontManager.Standard (DefaultFontSize = -1, no real default font). The apple/ios twins build a real
// UIFont/NSFont. See font_manager.hpp.

#include "maui/core/font_manager.hpp"

#include <memory>
#include <string>
#include <utility>

#include "maui/core/font.hpp"

namespace maui::core
{
    // C# FontManager.Standard.DefaultFontSize => -1. (The "no OS default size" sentinel; resolve_font_size
    // floors a non-positive backend default to its own fixed fallback, so size resolution stays positive.)
    double font_manager::default_font_size() const
    {
        return -1.0;
    }

    // No real native default font headless: a single owned sentinel (a heap int held by a shared_ptr that
    // the disposer keeps alive, freed in the dtor) stands in for C#'s _defaultFont so default_font() is
    // stably non-null. The void* is the int's address; the disposer releases the owning shared_ptr.
    void* font_manager::default_font()
    {
        if (default_font_ == nullptr)
        {
            auto sentinel = std::make_shared<int>(0);
            default_font_ = sentinel.get();
            default_dispose_ = [sentinel = std::move(sentinel)] {}; // releases the owner on the dtor's call
        }
        return default_font_;
    }

    // A distinct non-null sentinel per resolved font (the cache keys on the font, so each new font gets its
    // own handle; a repeat get_font returns the cached one). Like the apple/ios twins, the family is first
    // resolved through the registrar (cleanse_font_name) — headless builds no native font, but still
    // exercises the same alias/embedded resolution path, so a misconfigured registrar surfaces in tests.
    // The shared_ptr owner is captured by the disposer the cache fires on teardown — so the handle stays
    // valid for the manager's lifetime.
    font_manager::created_font font_manager::create_font(const font& value) const
    {
        if (!value.family().empty())
        {
            (void)cleanse_font_name(value.family()); // resolve through the registrar (no native font headless)
        }
        auto sentinel = std::make_shared<int>(0);
        void* const handle = sentinel.get();
        return created_font{.handle = handle, .dispose = [sentinel = std::move(sentinel)] {}};
    }
} // namespace maui::core
