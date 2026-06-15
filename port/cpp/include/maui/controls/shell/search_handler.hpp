#pragma once
// maui::controls::search_handler  <=  Microsoft.Maui.Controls.SearchHandler (+ ISearchHandlerController)
//
// The search surface of a shell application: the model behind the shell chrome's search box. Ported from
// src/Controls/src/Core/Shell/SearchHandler.cs + ISearchHandlerController.cs. Installed onto a page/shell
// via the Shell.SearchHandler attached property (shell::set_search_handler / get_search_handler); the
// shell handler reads it to build the native search box (UISearchBar on iOS, NSSearchField on AppKit) and
// routes native text + selection back through this model's inbound seam.
//
// search_handler is a bindable_object (NOT a visual_element — exactly like the C# SearchHandler, which is a
// plain BindableObject), so it carries the property system + BindingContext but is not laid out itself.
//
// ---- The behavior seam (the heart of the port) ----
//   set_query(text)        → OnQueryChanged(old, new): the Query property's two-way propertyChanged. A
//                            programmatic set AND a native edit both funnel through here (the native edit
//                            arrives via send_query_changed, which sets the property so the virtual fires).
//   query_confirmed()      → OnQueryConfirmed(): the user submitted (return/search button). Runs Command,
//                            then the queried event (command-then-event order, like button/search_bar).
//   item_selected(obj)     → OnItemSelected(obj): the user picked a results row. Records SelectedItem (at
//                            from_handler specificity — the C# read-only OneWayToSource key), then (on
//                            every non-WinUI platform, which is all the port targets) calls OnQueryConfirmed
//                            so the confirm Command fires for a selection too (SearchHandler.ItemSelected).
//   clear_placeholder_clicked() → OnClearPlaceholderClicked(): runs ClearPlaceholderCommand.
//
// OnQueryChanged / OnItemSelected / OnQueryConfirmed / OnClearPlaceholderClicked are the C# protected
// virtuals — overridable here too (a derived search_handler customizes filtering). The base
// on_query_confirmed runs the command; the base on_item_selected is a no-op; the base on_query_changed is a
// no-op (the developer overrides it to filter ItemsSource). The non-virtual public seam methods
// (query_confirmed / item_selected / clear_placeholder_clicked) ARE the ISearchHandlerController surface
// the chrome calls.
//
// ---- ICommand deviation (documented, consistent with search_bar.hpp + menu_item.hpp) ----
// The port has no ICommand subsystem, so Command / ClearPlaceholderCommand collapse to
// move_only_function<void()> action channels (the codebase's standing convention). Consequently the C#
// "Command.CanExecute → IsSearchEnabledCore" / "ClearPlaceholderCommand.CanExecute →
// ClearPlaceholderEnabledCore" auto-coupling is NOT ported (no CanExecute predicate); is_search_enabled /
// clear_placeholder_enabled remain plain bindable properties the developer sets. command_parameter /
// clear_placeholder_command_parameter are kept as bindable std::any values for surface fidelity (they ride
// along but, absent ICommand, are not forwarded to the parameter-less action) — documented, not stubbed.
//
// ---- DisplayMemberName + results ----
// items_source is a materialized vector of shared_ptr<bindable_object> (C#'s IEnumerable<object>); a
// non-null set wraps it in a list_proxy (the C# ListProxy) exposed to the chrome via results(). The C#
// per-source ListProxy windowing/INotifyCollectionChanged is collapsed (see list_proxy.hpp). display_member
// _name records which property the chrome shows per row (the obsolete-in-C# DisplayMemberName); the actual
// per-row text resolution lives in the chrome (it reads the row's display_member_name property by name).
//
// ---- Deferred (OUT OF SCOPE this unit, documented not stubbed) ----
//   - Font / text styling (FontFamily/FontSize/FontAttributes/FontAutoScalingEnabled/CharacterSpacing/
//     TextColor/Horizontal+VerticalTextAlignment/TextTransform/Keyboard): the IFontElement / ITextElement /
//     ITextAlignmentElement / IPlaceholderElement mixin surface. The port has no text_transform type yet
//     (search_bar deferred it too) and these are pure pass-through styling with no behavior; query /
//     placeholder / keyboard / colors that the chrome actually needs ARE ported.
//   - IsFocused / Focus / Unfocus / Focused+Unfocused events / ShowSoftInputAsync / HideSoftInputAsync:
//     the focus + soft-input request channel (needs a live native first-responder the headless model
//     cannot host; the chrome owns focus).
//   - The automation-properties seam (QueryIconName/HelpText etc. → AutomationProperties.Set* on the icon):
//     the accessibility name/help-text strings are kept as bindable properties for surface fidelity, but
//     applying them to the icon ImageSource (C# UpdateAutomationProperties) is out of scope.

#include <any>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/shell/list_proxy.hpp"
#include "maui/controls/shell/search_box_visibility.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/keyboard.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class data_template;

    class search_handler : public maui::core::bindable_object
    {
    public:
        search_handler();
        ~search_handler() override;
        search_handler(const search_handler&) = delete;
        search_handler(search_handler&&) = delete;
        search_handler& operator=(const search_handler&) = delete;
        search_handler& operator=(search_handler&&) = delete;

        // ---- Shared bindable-property descriptors (one per type, like SearchHandler.*Property) ----
        static const maui::core::bindable_property<std::string>& query_property();
        static const maui::core::bindable_property<std::string>& placeholder_property();
        static const maui::core::bindable_property<search_box_visibility>& search_box_visibility_property();
        static const maui::core::bindable_property<bool>& shows_results_property();
        static const maui::core::bindable_property<bool>& is_search_enabled_property();
        static const maui::core::bindable_property<bool>& clear_placeholder_enabled_property();
        static const maui::core::bindable_property<std::string>& display_member_name_property();
        static const maui::core::bindable_property<std::string>& automation_id_property();
        static const maui::core::bindable_property<maui::graphics::color>& background_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& placeholder_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& cancel_button_color_property();
        static const maui::core::bindable_property<maui::core::keyboard>& keyboard_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& query_icon_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& clear_icon_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>&
        clear_placeholder_icon_property();
        static const maui::core::bindable_property<std::shared_ptr<data_template>>& item_template_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::bindable_object>>&
        selected_item_property();
        // The accessibility name/help-text strings (kept for surface fidelity — see header note).
        static const maui::core::bindable_property<std::string>& query_icon_name_property();
        static const maui::core::bindable_property<std::string>& query_icon_help_text_property();
        static const maui::core::bindable_property<std::string>& clear_icon_name_property();
        static const maui::core::bindable_property<std::string>& clear_icon_help_text_property();
        static const maui::core::bindable_property<std::string>& clear_placeholder_name_property();
        static const maui::core::bindable_property<std::string>& clear_placeholder_help_text_property();

        // ---- getters ----
        [[nodiscard]] std::string_view query() const
        {
            return query_.get();
        }
        [[nodiscard]] std::string_view placeholder() const
        {
            return placeholder_.get();
        }
        [[nodiscard]] search_box_visibility get_search_box_visibility() const
        {
            return search_box_visibility_.get();
        }
        [[nodiscard]] bool shows_results() const
        {
            return shows_results_.get();
        }
        [[nodiscard]] bool is_search_enabled() const
        {
            return is_search_enabled_.get();
        }
        [[nodiscard]] bool clear_placeholder_enabled() const
        {
            return clear_placeholder_enabled_.get();
        }
        [[nodiscard]] std::string_view display_member_name() const
        {
            return display_member_name_.get();
        }
        [[nodiscard]] std::string_view automation_id() const
        {
            return automation_id_.get();
        }
        [[nodiscard]] maui::graphics::color background_color() const
        {
            return background_color_.get();
        }
        [[nodiscard]] maui::graphics::color placeholder_color() const
        {
            return placeholder_color_.get();
        }
        [[nodiscard]] maui::graphics::color text_color() const
        {
            return text_color_.get();
        }
        [[nodiscard]] maui::graphics::color cancel_button_color() const
        {
            return cancel_button_color_.get();
        }
        [[nodiscard]] maui::core::keyboard keyboard() const
        {
            return keyboard_.get();
        }
        [[nodiscard]] maui::core::i_image_source* query_icon() const
        {
            return query_icon_.get().get();
        }
        [[nodiscard]] maui::core::i_image_source* clear_icon() const
        {
            return clear_icon_.get().get();
        }
        [[nodiscard]] maui::core::i_image_source* clear_placeholder_icon() const
        {
            return clear_placeholder_icon_.get().get();
        }
        [[nodiscard]] data_template* item_template() const
        {
            return item_template_.get().get();
        }
        // SelectedItem (read-only in C#) — the row last handed to item_selected (or null).
        [[nodiscard]] maui::core::bindable_object* selected_item() const
        {
            return selected_item_.get().get();
        }
        [[nodiscard]] std::string_view query_icon_name() const
        {
            return query_icon_name_.get();
        }
        [[nodiscard]] std::string_view query_icon_help_text() const
        {
            return query_icon_help_text_.get();
        }
        [[nodiscard]] std::string_view clear_icon_name() const
        {
            return clear_icon_name_.get();
        }
        [[nodiscard]] std::string_view clear_icon_help_text() const
        {
            return clear_icon_help_text_.get();
        }
        [[nodiscard]] std::string_view clear_placeholder_name() const
        {
            return clear_placeholder_name_.get();
        }
        [[nodiscard]] std::string_view clear_placeholder_help_text() const
        {
            return clear_placeholder_help_text_.get();
        }

        // ---- items source + results ----
        // ItemsSource setter: a non-null set wraps the items in a list_proxy (the C# ListProxy) and fires
        // list_proxy_changed(old, new); a clear/null set drops the proxy and fires (old, null).
        void set_items_source(std::vector<std::shared_ptr<maui::core::bindable_object>> items);
        void clear_items_source();
        // The results list the chrome reads (ISearchHandlerController.ListProxy), or null when no source.
        [[nodiscard]] const list_proxy* results() const
        {
            return results_.get();
        }
        // ISearchHandlerController.ListProxyChanged: (old, new) — the chrome re-binds its rows.
        maui::core::event<const list_proxy*, const list_proxy*> list_proxy_changed;

        // ---- setters (drive on_property_changed → the chrome via the shell handler) ----
        // Query: set_query routes through the property store so OnQueryChanged(old, new) fires (the C#
        // two-way QueryProperty propertyChanged).
        void set_query(std::string value);
        void set_placeholder(std::string value)
        {
            placeholder_.set(std::move(value));
        }
        void set_search_box_visibility(search_box_visibility value)
        {
            search_box_visibility_.set(value);
        }
        void set_shows_results(bool value)
        {
            shows_results_.set(value);
        }
        void set_is_search_enabled(bool value)
        {
            is_search_enabled_.set(value);
        }
        void set_clear_placeholder_enabled(bool value)
        {
            clear_placeholder_enabled_.set(value);
        }
        void set_display_member_name(std::string value)
        {
            display_member_name_.set(std::move(value));
        }
        void set_automation_id(std::string value)
        {
            automation_id_.set(std::move(value));
        }
        void set_background_color(maui::graphics::color value)
        {
            background_color_.set(value);
        }
        void set_placeholder_color(maui::graphics::color value)
        {
            placeholder_color_.set(value);
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        void set_cancel_button_color(maui::graphics::color value)
        {
            cancel_button_color_.set(value);
        }
        void set_keyboard(maui::core::keyboard value)
        {
            keyboard_.set(value);
        }
        void set_query_icon(std::shared_ptr<maui::core::i_image_source> value)
        {
            query_icon_.set(std::move(value));
        }
        void set_clear_icon(std::shared_ptr<maui::core::i_image_source> value)
        {
            clear_icon_.set(std::move(value));
        }
        void set_clear_placeholder_icon(std::shared_ptr<maui::core::i_image_source> value)
        {
            clear_placeholder_icon_.set(std::move(value));
        }
        void set_item_template(std::shared_ptr<data_template> value)
        {
            item_template_.set(std::move(value));
        }
        void set_query_icon_name(std::string value)
        {
            query_icon_name_.set(std::move(value));
        }
        void set_query_icon_help_text(std::string value)
        {
            query_icon_help_text_.set(std::move(value));
        }
        void set_clear_icon_name(std::string value)
        {
            clear_icon_name_.set(std::move(value));
        }
        void set_clear_icon_help_text(std::string value)
        {
            clear_icon_help_text_.set(std::move(value));
        }
        void set_clear_placeholder_name(std::string value)
        {
            clear_placeholder_name_.set(std::move(value));
        }
        void set_clear_placeholder_help_text(std::string value)
        {
            clear_placeholder_help_text_.set(std::move(value));
        }

        // ---- command parameters (surface fidelity — see ICommand deviation note) ----
        void set_command_parameter(std::any value)
        {
            command_parameter_ = std::move(value);
        }
        [[nodiscard]] const std::any& command_parameter() const
        {
            return command_parameter_;
        }
        void set_clear_placeholder_command_parameter(std::any value)
        {
            clear_placeholder_command_parameter_ = std::move(value);
        }
        [[nodiscard]] const std::any& clear_placeholder_command_parameter() const
        {
            return clear_placeholder_command_parameter_;
        }

        // ---- the ISearchHandlerController INBOUND seam (the chrome calls these on native events) ----
        // ISearchHandlerController.QueryConfirmed: the user submitted.
        void query_confirmed()
        {
            on_query_confirmed();
        }
        // ISearchHandlerController.ItemSelected: the user picked a results row. Records SelectedItem (at
        // from_handler specificity, the C# OneWayToSource read-only key), then confirms (all port platforms
        // are non-WinUI, so the C# "skip OnQueryConfirmed on WinUI" branch never applies).
        void item_selected(std::shared_ptr<maui::core::bindable_object> obj);
        // ISearchHandlerController.ClearPlaceholderClicked.
        void clear_placeholder_clicked()
        {
            on_clear_placeholder_clicked();
        }
        // A native text edit: set the Query property (so OnQueryChanged fires) — SearchHandler's native
        // search-box delegate path. The chrome calls this; programmatic code calls set_query.
        void send_query_changed(std::string value)
        {
            set_query(std::move(value));
        }

        // ---- developer-facing events + command channels (the outbound channel) ----
        // queried: raised after OnQueryConfirmed (the port's stand-in for the confirm Command having run).
        maui::core::event<std::string_view> queried;
        // query_changed: raised on every Query change with (old, new) — mirrors the C# OnQueryChanged hook
        // for subscribers that don't subclass.
        maui::core::event<std::string, std::string> query_changed;
        // The ICommand stand-ins (see deviation note): run before their event, on confirm / clear-click.
        maui::core::move_only_function<void()> command;
        maui::core::move_only_function<void()> clear_placeholder_command;

    protected:
        // The C# protected virtuals — a derived search_handler overrides these to customize behavior.
        virtual void on_query_changed(std::string_view old_value, std::string_view new_value);
        virtual void on_query_confirmed();
        virtual void on_item_selected(maui::core::bindable_object* item);
        virtual void on_clear_placeholder_clicked();

    private:
        maui::core::property<std::string> query_{*this, query_property()};
        maui::core::property<std::string> placeholder_{*this, placeholder_property()};
        maui::core::property<search_box_visibility> search_box_visibility_{*this, search_box_visibility_property()};
        maui::core::property<bool> shows_results_{*this, shows_results_property()};
        maui::core::property<bool> is_search_enabled_{*this, is_search_enabled_property()};
        maui::core::property<bool> clear_placeholder_enabled_{*this, clear_placeholder_enabled_property()};
        maui::core::property<std::string> display_member_name_{*this, display_member_name_property()};
        maui::core::property<std::string> automation_id_{*this, automation_id_property()};
        maui::core::property<maui::graphics::color> background_color_{*this, background_color_property()};
        maui::core::property<maui::graphics::color> placeholder_color_{*this, placeholder_color_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::graphics::color> cancel_button_color_{*this, cancel_button_color_property()};
        maui::core::property<maui::core::keyboard> keyboard_{*this, keyboard_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> query_icon_{*this, query_icon_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> clear_icon_{*this, clear_icon_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> clear_placeholder_icon_{
            *this, clear_placeholder_icon_property()};
        maui::core::property<std::shared_ptr<data_template>> item_template_{*this, item_template_property()};
        maui::core::property<std::shared_ptr<maui::core::bindable_object>> selected_item_{*this,
                                                                                          selected_item_property()};
        maui::core::property<std::string> query_icon_name_{*this, query_icon_name_property()};
        maui::core::property<std::string> query_icon_help_text_{*this, query_icon_help_text_property()};
        maui::core::property<std::string> clear_icon_name_{*this, clear_icon_name_property()};
        maui::core::property<std::string> clear_icon_help_text_{*this, clear_icon_help_text_property()};
        maui::core::property<std::string> clear_placeholder_name_{*this, clear_placeholder_name_property()};
        maui::core::property<std::string> clear_placeholder_help_text_{*this, clear_placeholder_help_text_property()};

        // The ListProxy over ItemsSource (null when no source). Heap-owned (unique_ptr, not optional) so a
        // reassignment keeps the OLD proxy alive at its OWN distinct address across the list_proxy_changed
        // (old, new) notification — std::optional::emplace would reuse the storage, making old == new.
        std::unique_ptr<list_proxy> results_;
        std::any command_parameter_;
        std::any clear_placeholder_command_parameter_;
    };
} // namespace maui::controls
