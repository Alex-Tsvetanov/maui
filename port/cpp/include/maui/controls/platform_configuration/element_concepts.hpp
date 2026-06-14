#pragma once
// The shared element constraints the *Specific knob sets use for their config-chaining overloads
// (W2-24). C# types those extension methods on IPlatformElementConfiguration<TPlatform, FormsElement>
// and covariance admits derived elements; the port expresses the same surface with std::derived_from
// constraints — and, where C# targets an ABSTRACT base the port does not have, with a DECLARED set:
//   - Page (Microsoft.Maui.Controls.Page): the port's pages derive view<> directly with no shared page
//     base, so page_element enumerates them (the same declared-list substitute as
//     element::set_style_target_type — PROFILE-documented, reflection-free).
//   - VisualElement: any control is an element that implements the i_view contract.
//   - InputView (base of Entry/Editor/SearchBar): the declared trio.

#include <concepts>

#include "maui/controls/content_page.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/flyout_page.hpp"
#include "maui/controls/navigation_page.hpp"
#include "maui/controls/search_bar.hpp"
#include "maui/controls/tabbed_page.hpp"
#include "maui/core/i_view.hpp"

namespace maui::controls::platform_configuration
{
    // C# Microsoft.Maui.Controls.Page (the declared page set — see the header comment).
    template <class T>
    concept page_element = std::derived_from<T, content_page> || std::derived_from<T, navigation_page> ||
                           std::derived_from<T, tabbed_page> || std::derived_from<T, flyout_page>;

    // C# Microsoft.Maui.Controls.VisualElement.
    template <class T>
    concept visual_element = std::derived_from<T, element> && std::derived_from<T, maui::core::i_view>;

    // C# Microsoft.Maui.Controls.InputView (the declared trio — see the header comment).
    template <class T>
    concept input_view_element =
        std::derived_from<T, entry> || std::derived_from<T, editor> || std::derived_from<T, search_bar>;
} // namespace maui::controls::platform_configuration
