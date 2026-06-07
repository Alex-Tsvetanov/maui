#pragma once
// maui::core::i_element_handler  <=  Microsoft.Maui.IElementHandler
//
// The core handler contract: the seam between a cross-platform virtual view (i_element) and its native
// platform view. Ported from src/Core/src/Handlers/IElementHandler.cs.
//
// Ownership (PROFILE §8): the view OWNS its handler (shared_ptr); the handler holds a NON-owning
// back-reference to its virtual view (a raw i_element*) — the view outlives the handler, so the
// back-ref never dangles. The native platform view is type-erased to `void*` here: this is the one
// legitimate cross-platform erasure boundary (Core cannot name UIKit/AppKit/etc. types); concrete
// handlers expose it typed via view_handler::typed_platform_view(). Command arguments are `object?`
// in C# → `std::any` here — a genuine heterogeneous dispatch payload (e.g. a focus request); the
// erasure is confined to this command-dispatch boundary, not the value system (cf. PROFILE §7).

#include <any>
#include <string_view>

namespace maui::core
{
    class i_element;
    class i_maui_context;

    class i_element_handler
    {
    public:
        virtual ~i_element_handler() = default;

        virtual void set_maui_context(i_maui_context* context) = 0;
        virtual void set_virtual_view(i_element& view) = 0;
        virtual void update_value(std::string_view property) = 0;
        virtual void invoke(std::string_view command, const std::any& args = {}) = 0;
        virtual void disconnect_handler() = 0;

        [[nodiscard]] virtual void* platform_view() const = 0;
        [[nodiscard]] virtual i_element* virtual_view() const = 0;
        [[nodiscard]] virtual i_maui_context* maui_context() const = 0;

    protected:
        i_element_handler() = default;
        i_element_handler(const i_element_handler&) = default;
        i_element_handler(i_element_handler&&) = default;
        i_element_handler& operator=(const i_element_handler&) = default;
        i_element_handler& operator=(i_element_handler&&) = default;
    };
} // namespace maui::core
