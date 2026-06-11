#pragma once
// maui::application_model::app_actions   <=  Microsoft.Maui.ApplicationModel.AppActions (static facade)
// maui::application_model::i_app_actions <=  Microsoft.Maui.ApplicationModel.IAppActions
// maui::application_model::app_action    <=  Microsoft.Maui.ApplicationModel.AppAction
//
// App shortcuts on the app icon. The C# Task surface becomes the library's conventions:
// get_async delivers the current actions through a callback (inline on every ported backend),
// set_async completes inline; the AppActionActivated event maps to explicit add_/remove_
// accessors (the battery pattern). AppActionEventArgs collapses to its app_action payload.
// AppAction.Icon is internal in C# (settable only through the ctor) - the port exposes a
// read-only accessor (no internal visibility here).
//
// Backends (suffix oracle): ios REAL (AppActions.ios.cs - UIApplication.shortcutItems; items
// round-trip through a "XE_APP_ACTION_TYPE" UIApplicationShortcutItem whose userInfo carries
// id/icon; IsSupported is true; the activation seam is PerformActionForShortcutItem, surfaced on
// the ios implementation). SIMULATOR-TESTABILITY NOTE: the spawned gtest process has no
// UIApplication instance, so shortcutItems reads as empty and stores no-op - the on-simulator
// suite asserts is_supported + the no-app surface; the set/get round-trip needs a real app.
// apple/macOS NOT SUPPORTED (AppActions.netstandard.tvos.watchos.macos.tizen.cs - EVERY member
// including is_supported throws, exactly like the netstandard partial). Headless mirrors
// netstandard until faked (settable supported flag + stored actions + simulate_activated).

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::application_model
{
    // An app shortcut: id + title (+ optional subtitle/icon).
    class app_action
    {
    public:
        app_action(std::string id, std::string title, std::optional<std::string> subtitle = std::nullopt,
                   std::optional<std::string> icon = std::nullopt)
            : id_(std::move(id)), title_(std::move(title)), subtitle_(std::move(subtitle)), icon_(std::move(icon))
        {
        }

        [[nodiscard]] const std::string& id() const
        {
            return id_;
        }
        [[nodiscard]] const std::string& title() const
        {
            return title_;
        }
        [[nodiscard]] const std::optional<std::string>& subtitle() const
        {
            return subtitle_;
        }
        [[nodiscard]] const std::optional<std::string>& icon() const
        {
            return icon_;
        }

        void set_id(std::string value)
        {
            id_ = std::move(value);
        }
        void set_title(std::string value)
        {
            title_ = std::move(value);
        }
        void set_subtitle(std::optional<std::string> value)
        {
            subtitle_ = std::move(value);
        }

        friend bool operator==(const app_action& a, const app_action& b) = default;

    private:
        std::string id_;
        std::string title_;
        std::optional<std::string> subtitle_;
        std::optional<std::string> icon_;
    };

    // Receives the currently registered actions.
    using app_actions_callback = maui::core::move_only_function<void(const std::vector<app_action>&)>;

    class i_app_actions
    {
    public:
        virtual ~i_app_actions() = default;

        // IsSupported (throws feature_not_supported on platforms whose partial is netstandard).
        [[nodiscard]] virtual bool is_supported() const = 0;

        // GetAsync: deliver the currently available actions.
        virtual void get_async(app_actions_callback on_complete) = 0;
        // SetAsync: replace the registered actions.
        virtual void set_async(const std::vector<app_action>& actions) = 0;

        // AppActionActivated event accessors (raised when a shortcut launches/resumes the app).
        virtual maui::core::connection_token add_app_action_activated(
            maui::core::move_only_function<void(const app_action&)> handler) = 0;
        virtual bool remove_app_action_activated(maui::core::connection_token token) = 0;

    protected:
        i_app_actions() = default;
        i_app_actions(const i_app_actions&) = default;
        i_app_actions(i_app_actions&&) = default;
        i_app_actions& operator=(const i_app_actions&) = default;
        i_app_actions& operator=(i_app_actions&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (AppActionsImplementation), one per backend under
        // src/platform/<backend>/essentials_app_actions.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_app_actions> make_app_actions();
    } // namespace detail

    // The static facade. on_app_action add/remove mirror the C# OnAppAction event accessors.
    class app_actions final
    {
    public:
        app_actions() = delete;

        [[nodiscard]] static bool is_supported()
        {
            return current().is_supported();
        }
        static void get_async(app_actions_callback on_complete)
        {
            current().get_async(std::move(on_complete));
        }
        static void set_async(const std::vector<app_action>& actions)
        {
            current().set_async(actions);
        }
        static maui::core::connection_token add_on_app_action(
            maui::core::move_only_function<void(const app_action&)> handler)
        {
            return current().add_app_action_activated(std::move(handler));
        }
        static bool remove_on_app_action(maui::core::connection_token token)
        {
            return current().remove_app_action_activated(token);
        }

        // AppActions.Current (lazy platform default) + SetCurrent (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_app_actions& current();
        static void set_current(std::shared_ptr<i_app_actions> implementation);
    };
} // namespace maui::application_model
