#pragma once
// maui::samples::hybrid_web_view_page — ports HybridWebViewPage.xaml (+ HybridWebViewPage.xaml.cs).
//
// The C# sample hosts a HybridWebView (HybridRoot="HybridSamplePage") next to a read-only status Editor
// and a column of buttons that exercise the host<->JS bridge: "Send message to JS" (SendRawMessage),
// "Invoke JS" / "Invoke Async JS" (typed InvokeJavaScriptAsync<T>), and two "Test JS [Async] Exception"
// buttons. Inbound `RawMessageReceived` appends the message to the status editor; the code-behind also
// installs a .NET method target via SetInvokeJavaScriptTarget<DotNetMethods> so JS can call back into C#.
//
// PORT SCOPE (faithful to what the cross-platform surface exposes — see hybrid_web_view.hpp):
//   - send_raw_message / raw_message_received_event are fully wired (the host->page and page->host raw
//     channel), exactly as the C# "Send message to JS" button + hwv_RawMessageReceived handler;
//   - InvokeJavaScriptAsync<T> is ported as invoke_js(method, raw-json params, on_result) — the callback
//     form returning the RAW json (the port has no reflection-driven System.Text.Json, so the caller
//     owns (de)serialization). The four invoke/exception buttons drive invoke_js and write the raw result
//     (or "<null>") into the status editor — the same shape as the C# buttons minus typed deserialization.
//   note: SetInvokeJavaScriptTarget + InvokeDotNet (JS -> .NET method invocation by reflection) and the
//   typed InvokeJavaScriptAsync<T> JSON deserialization are DEFERRED in the port (PROFILE §6, no
//   reflection); DotNetMethods has no analog here. The page wires everything the cross-platform surface
//   allows and notes the rest.
//
// The page OWNS its whole element tree (the web_view_page pattern). It is backend-agnostic — a sample
// main attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/
// apple/ios test trees exercise the same controls directly.
//
// Demonstrated:
//   - a HybridWebView with HybridRoot set, hosted in a 2-column / 2-row grid;
//   - SendRawMessage from a button + RawMessageReceived feeding a read-only status editor;
//   - invoke_js (the InvokeJavaScriptAsync callback port) with a raw result written back to the status.

#include <cstdio>
#include <exception>
#include <optional>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/hybrid_web_view.hpp"
#include "maui/controls/hybrid_web_view_raw_message_received_event_args.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class hybrid_web_view_page
    {
    public:
        hybrid_web_view_page()
        {
            page_.set_title("HybridWebView");

            // <Grid ColumnDefinitions="2*,1*" RowDefinitions="Auto,1*">
            root_.add_column_definition(maui::core::grid_length(2.0, maui::core::grid_unit_type::star));
            root_.add_column_definition(maui::core::grid_length(1.0, maui::core::grid_unit_type::star));
            root_.add_row_definition(maui::core::grid_length::automatic());
            root_.add_row_definition(maui::core::grid_length(1.0, maui::core::grid_unit_type::star));

            // <Editor x:Name="statusText" Grid.Row="0" Grid.Column="0" Text="HybridWebView here"
            //         IsReadOnly="True" MinimumHeightRequest="200"/>
            status_.set_text("HybridWebView here");
            status_.set_is_read_only(true);
            status_.set_minimum_height_request(200);

            // <VerticalStackLayout Grid.Row="0" Grid.Column="1"> — the button column.
            send_msg_.set_text("Send message to JS");
            send_msg_.clicked.connect([this] {
                // C# SendMessageButton_Clicked: hwv.SendRawMessage($"Hello from C#! #{count++}").
                hwv_.send_raw_message("Hello from C#! #" + std::to_string(count_++));
            });

            invoke_js_.set_text("Invoke JS");
            invoke_js_.clicked.connect([this] {
                // C# InvokeJSMethodButton_Clicked: await hwv.InvokeJavaScriptAsync<ComputationResult>(
                //   "AddNumbers", …, [123, 321], …). Port: invoke_js with the already-json-encoded params;
                // on_result gets the raw json (no typed deserialization — PORT SCOPE note above).
                hwv_.invoke_js("AddNumbers", std::vector<std::string>{"123", "321"},
                               [this](const std::optional<std::string>& result) {
                                   append_status(result ? "AddNumbers -> " + *result
                                                        : std::string("Got no result for AddNumbers"));
                               });
            });

            invoke_async_js_.set_text("Invoke Async JS");
            invoke_async_js_.clicked.connect([this] {
                // C# InvokeAsyncJSMethodButton_Clicked: EvaluateMeWithParamsAndAsyncReturn("new_key",
                // "new_value"). The two string params are pre-json-encoded (quoted) for the raw channel.
                hwv_.invoke_js("EvaluateMeWithParamsAndAsyncReturn",
                               std::vector<std::string>{"\"new_key\"", "\"new_value\""},
                               [this](const std::optional<std::string>& result) {
                                   append_status(result ? "EvaluateMeWithParamsAndAsyncReturn -> " + *result
                                                        : std::string("Got no result from "
                                                                      "EvaluateMeWithParamsAndAsyncReturn"));
                               });
            });

            throw_js_.set_text("Test JS Exception");
            throw_js_.clicked.connect([this] {
                // C# InvokeJSExceptionButton_Clicked: "ThrowJavaScriptError". A failed/void call reports
                // nullopt through the callback channel (the port has no faulted Task — see invoke_js note).
                append_status("Calling JavaScript function that throws exception...");
                hwv_.invoke_js("ThrowJavaScriptError", [this](const std::optional<std::string>& result) {
                    append_status(result ? "Unexpected success: " + *result
                                         : std::string("ThrowJavaScriptError reported no result (error)"));
                });
            });

            throw_async_js_.set_text("Test JS Async Exception");
            throw_async_js_.clicked.connect([this] {
                // C# InvokeJSAsyncExceptionButton_Clicked: "ThrowJavaScriptErrorAsync".
                append_status("Calling async JavaScript function that throws exception...");
                hwv_.invoke_js("ThrowJavaScriptErrorAsync", [this](const std::optional<std::string>& result) {
                    append_status(result ? "Unexpected success: " + *result
                                         : std::string("ThrowJavaScriptErrorAsync reported no result (error)"));
                });
            });

            buttons_.set_spacing(8);
            buttons_.add(send_msg_);
            buttons_.add(invoke_js_);
            buttons_.add(invoke_async_js_);
            buttons_.add(throw_js_);
            buttons_.add(throw_async_js_);

            // <HybridWebView x:Name="hwv" Grid.Row="1" Grid.ColumnSpan="3" HybridRoot="HybridSamplePage"
            //                RawMessageReceived="hwv_RawMessageReceived"/>
            hwv_.set_hybrid_root("HybridSamplePage");
            hwv_.raw_message_received_event.connect(
                [this](const maui::controls::hybrid_web_view_raw_message_received_event_args& args) {
                    // C# hwv_RawMessageReceived: statusText.Text += Environment.NewLine + e.Message.
                    append_status(args.message.value_or("<null>"));
                });

            // Place children in the grid (status + button column on row 0; the web view spanning row 1).
            root_.add(status_);
            root_.set_row(status_, 0);
            root_.set_column(status_, 0);

            root_.add(buttons_);
            root_.set_row(buttons_, 0);
            root_.set_column(buttons_, 1);

            root_.add(hwv_);
            root_.set_row(hwv_, 1);
            root_.set_column(hwv_, 0);
            root_.set_column_span(hwv_, 2); // C# ColumnSpan="3" clamped to the 2 columns this grid defines

            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the leaf controls, then the button stack, then
        // the grid, then the page), then re-host the tree built in the ctor (gallery_attach.hpp). The
        // hybrid_web_view may have no headless/AppKit handler registered — gallery_attach_one logs + skips
        // it, the rest of the page still mounts.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& v, const char* n) {
                try
                {
                    app.attach_handler(v);
                }
                catch (const std::exception& e)
                {
                    std::fprintf(stderr, "[gallery] skip %s: %s\n", n, e.what());
                }
            };

            one(status_, "status_");
            one(send_msg_, "send_msg_");
            one(invoke_js_, "invoke_js_");
            one(invoke_async_js_, "invoke_async_js_");
            one(throw_js_, "throw_js_");
            one(throw_async_js_, "throw_async_js_");
            one(hwv_, "hwv_");
            one(buttons_, "buttons_");
            one(root_, "root_");
            one(page_, "page_");

            gallery_rehost_layout(buttons_); // the button stack hosts its five buttons
            gallery_rehost_layout(root_);    // the grid hosts status_ + buttons_ + hwv_
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::grid& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::hybrid_web_view& hwv()
        {
            return hwv_;
        }
        [[nodiscard]] maui::controls::editor& status()
        {
            return status_;
        }
        [[nodiscard]] maui::controls::button& send_message_button()
        {
            return send_msg_;
        }

    private:
        // C# statusText.Text += Environment.NewLine + line — append a line to the read-only status editor.
        void append_status(const std::string& line)
        {
            std::string text(status_.text());
            if (!text.empty())
            {
                text += '\n';
            }
            text += line;
            status_.set_text(std::move(text));
        }

        maui::controls::content_page page_;
        maui::controls::grid root_;
        maui::controls::editor status_;
        maui::controls::vertical_stack_layout buttons_;
        maui::controls::button send_msg_;
        maui::controls::button invoke_js_;
        maui::controls::button invoke_async_js_;
        maui::controls::button throw_js_;
        maui::controls::button throw_async_js_;
        maui::controls::hybrid_web_view hwv_;
        int count_ = 0; // C# `int count` in SendMessageButton_Clicked
    };
} // namespace maui::samples
