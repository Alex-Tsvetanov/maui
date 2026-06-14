#pragma once
// maui::application_model::clipboard    <=  Microsoft.Maui.ApplicationModel.DataTransfer.Clipboard (static facade)
// maui::application_model::i_clipboard  <=  Microsoft.Maui.ApplicationModel.DataTransfer.IClipboard
//
// Works with text on the device clipboard. The C# `Task SetTextAsync` / `Task<string?> GetTextAsync`
// become the library's synchronous-completion convention (every backend completes inline like the
// platform partials' Task.CompletedTask / Task.FromResult): set_text_async takes the text + an
// optional completion callback, get_text_async delivers the optional string through a callback
// (std::nullopt = no text). has_text is a synchronous bool.
//
// ClipboardContentChanged is the listener-starting add/remove pair (the C# add/remove accessors,
// mirrored exactly by the devices battery_base precedent): the first subscriber runs
// StartClipboardListeners BEFORE the handler is stored (so a throwing start - the macOS / netstandard
// mirror - leaves no subscription behind), and the last removal runs StopClipboardListeners. The
// shared gate lives in detail::clipboard_base; backends derive it and call on_clipboard_content_changed
// from their platform observer.
//
// The C# `SetTextAsync(string?)` coalesces null to string.Empty in the static facade; the port's
// set_text_async takes a string_view, so the caller's empty string is the null analog and the facade
// passes it straight through.
//
// Backends (suffix oracle): apple/macOS REAL (Clipboard.macos.cs - NSPasteboard.GeneralPasteboard
// string read/write; macOS has NO change notification, so the listener hooks throw
// feature_not_supported - subscribing to clipboard_content_changed there is unsupported, matching the
// C# StartClipboardListeners throw), ios REAL (Clipboard.ios.cs - UIPasteboard.General string +
// HasStrings; the listener observes UIPasteboard.ChangedNotification). Headless mirrors netstandard
// (throws) until the fake is configured (an in-memory string + a manual raise that drives the shared
// event listener path).

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/core/event.hpp"
#include "maui/core/move_only_function.hpp"

namespace maui::application_model
{
    // Receives the clipboard text (std::nullopt when there is none) - the Task<string?> result.
    using clipboard_text_callback = maui::core::move_only_function<void(std::optional<std::string>)>;
    // A bare completion signal for set_text_async (the Task with no result).
    using clipboard_completion_callback = maui::core::move_only_function<void()>;
    // The ClipboardContentChanged handler (EventArgs.Empty - no payload).
    using clipboard_changed_handler = maui::core::move_only_function<void()>;

    class i_clipboard
    {
    public:
        virtual ~i_clipboard() = default;

        // HasText: is there any text on the clipboard?
        [[nodiscard]] virtual bool has_text() const = 0;
        // SetTextAsync: replace the clipboard text (completion is signalled inline).
        virtual void set_text_async(std::string_view text, clipboard_completion_callback on_complete) = 0;
        // GetTextAsync: read the clipboard text (std::nullopt when empty).
        virtual void get_text_async(clipboard_text_callback on_complete) = 0;

        // ClipboardContentChanged add/remove (the C# event accessors): the first subscribe starts
        // the platform observer, the last unsubscribe stops it.
        virtual maui::core::connection_token add_clipboard_content_changed(clipboard_changed_handler handler) = 0;
        virtual bool remove_clipboard_content_changed(maui::core::connection_token token) = 0;

    protected:
        i_clipboard() = default;
        i_clipboard(const i_clipboard&) = default;
        i_clipboard(i_clipboard&&) = default;
        i_clipboard& operator=(const i_clipboard&) = default;
        i_clipboard& operator=(i_clipboard&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (ClipboardImplementation), one per backend under
        // src/platform/<backend>/essentials_clipboard.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_clipboard> make_clipboard();

        // The cross-platform half of the C# ClipboardImplementation add/remove accessors: the
        // subscriber-count gate that drives start/stop_clipboard_listeners. Backends derive this
        // instead of i_clipboard directly and supply the value members + the listener hooks.
        class clipboard_base : public i_clipboard
        {
        public:
            maui::core::connection_token add_clipboard_content_changed(clipboard_changed_handler handler) override
            {
                if (subscribers_ == 0)
                {
                    start_clipboard_listeners();
                }
                ++subscribers_;
                return clipboard_content_changed_.connect(std::move(handler));
            }

            bool remove_clipboard_content_changed(maui::core::connection_token token) override
            {
                if (!clipboard_content_changed_.disconnect(token))
                {
                    return false;
                }
                if (--subscribers_ == 0)
                {
                    stop_clipboard_listeners();
                }
                return true;
            }

        protected:
            clipboard_base() = default;

            // Start/StopClipboardListeners (the platform partial's listener hooks).
            virtual void start_clipboard_listeners() = 0;
            virtual void stop_clipboard_listeners() = 0;

            // OnClipboardContentChanged(): raise the shared event (the platform observer calls this).
            void on_clipboard_content_changed()
            {
                clipboard_content_changed_.raise();
            }

        private:
            maui::core::event<> clipboard_content_changed_;
            int subscribers_ = 0;
        };
    } // namespace detail

    // The static facade over clipboard::default_() (C# Clipboard).
    class clipboard final
    {
    public:
        clipboard() = delete;

        // HasText.
        [[nodiscard]] static bool has_text()
        {
            return default_().has_text();
        }
        // SetTextAsync(text) with no completion observer.
        static void set_text_async(std::string_view text)
        {
            default_().set_text_async(text, nullptr);
        }
        // SetTextAsync(text) signalling completion.
        static void set_text_async(std::string_view text, clipboard_completion_callback on_complete)
        {
            default_().set_text_async(text, std::move(on_complete));
        }
        // GetTextAsync.
        static void get_text_async(clipboard_text_callback on_complete)
        {
            default_().get_text_async(std::move(on_complete));
        }

        // ClipboardContentChanged add/remove.
        static maui::core::connection_token add_clipboard_content_changed(clipboard_changed_handler handler)
        {
            return default_().add_clipboard_content_changed(std::move(handler));
        }
        static bool remove_clipboard_content_changed(maui::core::connection_token token)
        {
            return default_().remove_clipboard_content_changed(token);
        }

        // Clipboard.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_clipboard& default_();
        static void set_default(std::shared_ptr<i_clipboard> implementation);
    };
} // namespace maui::application_model
