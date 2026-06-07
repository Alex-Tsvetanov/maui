#pragma once
// maui::core::i_element  <=  Microsoft.Maui.IElement
//
// The root virtual-view contract: a handler attachment point and a parent back-reference. Ported
// from src/Core/src/Core/IElement.cs. An abstract class (runtime-polymorphic ⇒ i_* class, PROFILE
// §11). Per §8: the view owns its handler (shared_ptr); the parent link is a weak back-reference,
// surfaced here as a locked shared_ptr (null once the parent is gone). i_element_handler is defined
// with the handler layer (#22) — forward-declared here.

#include <memory>

namespace maui::core
{
    class i_element_handler;

    class i_element
    {
    public:
        virtual ~i_element() = default;

        [[nodiscard]] virtual const std::shared_ptr<i_element_handler>& handler() const = 0;
        virtual void set_handler(std::shared_ptr<i_element_handler> value) = 0;
        [[nodiscard]] virtual std::shared_ptr<i_element> parent() const = 0;

    protected:
        i_element() = default;
        i_element(const i_element&) = default;
        i_element(i_element&&) = default;
        i_element& operator=(const i_element&) = default;
        i_element& operator=(i_element&&) = default;
    };
} // namespace maui::core
