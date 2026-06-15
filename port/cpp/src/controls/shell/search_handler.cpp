// maui::controls::search_handler — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like SearchHandler.*Property), the ItemsSource → list_proxy wiring, and the behavior
// seam (OnQueryChanged / OnQueryConfirmed / OnItemSelected / OnClearPlaceholderClicked). See
// search_handler.hpp. Behavior derived from src/Controls/src/Core/Shell/SearchHandler.cs.
//
// Defaults mirror SearchHandler.cs: empty Query/Placeholder; SearchBoxVisibility.Expanded;
// ShowsResults false; IsSearchEnabled TRUE (IsSearchEnabledProperty default true);
// ClearPlaceholderEnabled false; default-color Background/Placeholder/Text/CancelButton (C# null —
// platform default); Keyboard.Default; null icons / item template / selected item.
//
// search_handler is NOT a view — it has no native handler of its own (the shell chrome installs the search
// box and drives this model), so there is NO MAUI_REGISTER_HANDLER here.

#include "maui/controls/shell/search_handler.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/shell/list_proxy.hpp"
#include "maui/controls/shell/search_box_visibility.hpp"
#include "maui/controls/templates/data_template.hpp" // complete type for property<shared_ptr<data_template>>
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    // ---- the shared descriptors ----

    const maui::core::bindable_property<std::string>& search_handler::query_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"query", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::placeholder_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"placeholder", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<search_box_visibility>& search_handler::search_box_visibility_property()
    {
        // C# SearchBoxVisibilityProperty default: SearchBoxVisibility.Expanded.
        static const maui::core::bindable_property<search_box_visibility> descriptor{"search_box_visibility",
                                                                                     search_box_visibility::expanded};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& search_handler::shows_results_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"shows_results", false};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& search_handler::is_search_enabled_property()
    {
        // C# IsSearchEnabledProperty default: true.
        static const maui::core::bindable_property<bool> descriptor{"is_search_enabled", true};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& search_handler::clear_placeholder_enabled_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"clear_placeholder_enabled", false};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::display_member_name_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"display_member_name", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::automation_id_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"automation_id", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_handler::background_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"background_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_handler::placeholder_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"placeholder_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_handler::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& search_handler::cancel_button_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"cancel_button_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::keyboard>& search_handler::keyboard_property()
    {
        // C# KeyboardProperty default: Keyboard.Default (coerced from null).
        static const maui::core::bindable_property<maui::core::keyboard> descriptor{
            "keyboard", maui::core::keyboard::default_keyboard()};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& search_handler::
        query_icon_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "query_icon"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& search_handler::
        clear_icon_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "clear_icon"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& search_handler::
        clear_placeholder_icon_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>> descriptor{
            "clear_placeholder_icon"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<data_template>>& search_handler::item_template_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<data_template>> descriptor{"item_template"};
        return descriptor;
    }

    const maui::core::bindable_property<std::shared_ptr<maui::core::bindable_object>>& search_handler::
        selected_item_property()
    {
        // C# SelectedItemProperty: read-only, OneWayToSource; default null. Written only at from_handler
        // specificity via item_selected.
        static const maui::core::bindable_property<std::shared_ptr<maui::core::bindable_object>> descriptor{
            "selected_item"};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::query_icon_name_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"query_icon_name", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::query_icon_help_text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"query_icon_help_text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::clear_icon_name_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"clear_icon_name", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::clear_icon_help_text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"clear_icon_help_text", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::clear_placeholder_name_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"clear_placeholder_name", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<std::string>& search_handler::clear_placeholder_help_text_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"clear_placeholder_help_text",
                                                                           std::string{}};
        return descriptor;
    }

    // ---- ctor/dtor ----

    search_handler::search_handler() = default;
    search_handler::~search_handler() = default;

    // ---- Query: route through the store, then fire OnQueryChanged + the query_changed event ----

    void search_handler::set_query(std::string value)
    {
        const std::string old{query_.get()};
        query_.set(std::move(value));
        const std::string current{query_.get()};
        if (current != old)
        {
            on_query_changed(old, current);
            query_changed.raise(old, current);
        }
    }

    // ---- ItemsSource → list_proxy (the C# OnItemsSourceChanged) ----

    void search_handler::set_items_source(std::vector<std::shared_ptr<maui::core::bindable_object>> items)
    {
        // C# fires ListProxyChanged with old + new on every ListProxy reassignment. Build the new proxy
        // FIRST, then keep the old one alive across the raise (a heap object at its OWN address) so old and
        // new are distinct, live pointers during the notification.
        auto new_proxy = std::make_unique<list_proxy>(std::move(items));
        const list_proxy* const old_ptr = results_.get();
        const list_proxy* const new_ptr = new_proxy.get();
        const std::unique_ptr<list_proxy> old_keep = std::move(results_); // keep the old alive for the raise
        results_ = std::move(new_proxy);
        list_proxy_changed.raise(old_ptr, new_ptr);
    }

    void search_handler::clear_items_source()
    {
        if (results_ == nullptr)
        {
            return;
        }
        // The C# OnItemsSourceChanged sets ListProxy = null when the source is null (fires (old, null)).
        const std::unique_ptr<list_proxy> old_keep = std::move(results_); // alive through the raise
        list_proxy_changed.raise(old_keep.get(), nullptr);
    }

    // ---- the inbound controller seam ----

    void search_handler::item_selected(std::shared_ptr<maui::core::bindable_object> obj)
    {
        maui::core::bindable_object* const raw = obj.get();
        on_item_selected(raw);
        // SearchHandler.ISearchHandlerController.ItemSelected: SetValue(SelectedItem, obj, FromHandler).
        selected_item_.set(std::move(obj), maui::core::setter_specificity::from_handler);
        // Non-WinUI (every port target): confirm the query for the selection too.
        on_query_confirmed();
    }

    // ---- the protected virtuals (the base behavior) ----

    void search_handler::on_query_changed(std::string_view /*old_value*/, std::string_view /*new_value*/)
    {
        // SearchHandler.OnQueryChanged base: empty (the developer overrides to filter ItemsSource).
    }

    void search_handler::on_query_confirmed()
    {
        // SearchHandler.OnQueryConfirmed base: run the confirm Command (the port's action stand-in — no
        // ICommand CanExecute gate), then notify subscribers via the queried event.
        if (command)
        {
            command();
        }
        queried.raise(query_.get());
    }

    void search_handler::on_item_selected(maui::core::bindable_object* /*item*/)
    {
        // SearchHandler.OnItemSelected base: empty (the developer overrides to act on a picked row).
    }

    void search_handler::on_clear_placeholder_clicked()
    {
        // SearchHandler.OnClearPlaceholderClicked base: run the ClearPlaceholderCommand (action stand-in).
        if (clear_placeholder_command)
        {
            clear_placeholder_command();
        }
    }
} // namespace maui::controls
