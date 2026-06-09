#pragma once
// maui::controls::window  <=  Microsoft.Maui.Controls.Window (+ Microsoft.Maui.IWindow lifecycle)
//
// Hosts a single root page and carries the window lifecycle: Created / Activated / Deactivated /
// Destroying (+ Resumed / Stopped / Backgrounding). A window is an element, so the hosted page is its one
// logical child — activating the window flows the window reference down the page's subtree (firing each
// element's Loaded) and drives the page's Appearing; deactivating reverses it (Disappearing + Unloaded).
// The window's own BindingContext also inherits down to the page. Ported from Window.cs (the
// IWindow.Created/Activated/Deactivated/Destroying drive + SendWindowAppearing / SendWindowDisppearing).
//
// A window is ALSO an i_element (C# Window : IWindow : ITitledElement : IElement), so it has the same
// handler seam as a control: set_handler wires the native host (the window_handler over an NSWindow),
// which on connect sets the NSWindow's contentView to the root page's native view and its title. The
// window_handler is a LIGHTWEIGHT i_element_handler (a window is not an i_view, so it is NOT a view<>) —
// its Title / Content / X / Y / Width / Height flow to the handler through the standard property mapper.
// Window's bindable X/Y/Width/Height are set at the handler specificity by frame_changed (so a developer's
// manual set still wins), matching Window.cs. The native NSWindow notifications map back via
// send_activated / send_destroying.
//
// The drive methods are send_* (the inbound channel — the platform's native window calls these, exactly
// like a control's send_clicked), so the matching observable events can keep the plain names. C# throws on
// a double Created/Activated; the port no-ops idempotently (gentler, and there is no platform contract to
// enforce here). Overlays, the modal stack, visual diagnostics, multi-content, MinimumWidth/MaximumWidth,
// the title bar / display density, and IPersistedState are out of scope (STATUS.md).

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_window.hpp"
#include "maui/core/property.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::controls
{
    class content_page; // forward — the convenience ctor hosts one; appearing is driven by dynamic_cast in .cpp

    // Shared bindable-property descriptors for the window geometry (Window.X/Y/Width/Height). NON-template
    // free functions so the descriptor identity (and thus the property name the window_handler keys on) is
    // a single instance program-wide. Defaults to Dimension.Unset (NaN) like Window.cs. Defined in
    // window.cpp.
    const maui::core::bindable_property<double>& window_x_property();
    const maui::core::bindable_property<double>& window_y_property();
    const maui::core::bindable_property<double>& window_width_property();
    const maui::core::bindable_property<double>& window_height_property();

    class window : public element, public maui::core::i_window
    {
    public:
        window();
        explicit window(content_page& page);

        // ---- i_element (the handler seam — a window is an IElement, like every control) ----
        [[nodiscard]] const std::shared_ptr<maui::core::i_element_handler>& handler() const override
        {
            return handler_;
        }
        // Setting the handler wires the seam: the incoming handler binds to this window (creating the native
        // NSWindow + running the mapper, so Title/Content/geometry reach it), then the previous handler is
        // disconnected. Mirrors Element.Handler's setter / view<>::set_handler.
        void set_handler(std::shared_ptr<maui::core::i_element_handler> value) override;
        // A window is a root — the port keeps the i_element parent null (the Application owns the window via
        // its windows() list, not through this child→parent link).
        [[nodiscard]] std::shared_ptr<maui::core::i_element> parent() const override
        {
            return nullptr;
        }

        // The hosted root page (Window.Page / IWindow.Content). NON-owning — the caller owns the page. The
        // page is both an element (the controls base, for the logical tree) and an i_element (via view<>, so
        // the handler can reach its native view); content() returns the i_element face (the i_window
        // contract). content_element() returns the controls-side element face for in-tree operations.
        [[nodiscard]] maui::core::i_element* content() const override
        {
            return dynamic_cast<maui::core::i_element*>(content_);
        }
        [[nodiscard]] element* content_element() const
        {
            return content_;
        }
        // Host (or replace) the root page. The page inherits the window's BindingContext immediately; if the
        // window is already activated it is also attached to the window (Loaded + Appearing) at once. The
        // handler is told to re-host the page's native view (Window.OnPropertyChanged(Page) → MapContent).
        void set_content(element& page);

        [[nodiscard]] std::string_view title() const override
        {
            return title_;
        }
        // Setting the title pushes it to the handler (Window.OnPropertyChanged(Title) → MapTitle).
        void set_title(std::string value);

        // ---- geometry (Window.X / Y / Width / Height) — bindable, pushed to the handler ----
        [[nodiscard]] double x() const override
        {
            return x_.get();
        }
        [[nodiscard]] double y() const override
        {
            return y_.get();
        }
        [[nodiscard]] double width() const override
        {
            return width_.get();
        }
        [[nodiscard]] double height() const override
        {
            return height_.get();
        }
        // Developer-set geometry (Window.X = …): a manual set, pushed to the handler, which sizes/moves the
        // NSWindow. Each is independent, like the C# bindables.
        void set_x(double value)
        {
            x_.set(value);
        }
        void set_y(double value)
        {
            y_.set(value);
        }
        void set_width(double value)
        {
            width_.set(value);
        }
        void set_height(double value)
        {
            height_.set(value);
        }
        // The current frame as a rect (X, Y, Width, Height) — a convenience over the four getters.
        [[nodiscard]] maui::graphics::rect frame() const
        {
            return {x_.get(), y_.get(), width_.get(), height_.get()};
        }

        // IWindow.FrameChanged: the native window reports its new frame. A no-op when the frame is unchanged;
        // otherwise sets the four geometry properties at the HANDLER specificity (Window.cs's
        // SetterSpecificity.FromHandler), suppresses the re-push to the handler during the batch
        // (Window._batchFrameUpdate), and raises `size_changed` only when the size actually changed. A later
        // developer set (manual specificity) overrides the handler value, so the developer can still pin the
        // geometry after a platform frame report.
        void frame_changed(const maui::graphics::rect& frame) override;

        // Window.SizeChanged — raised by frame_changed when the width or height actually changed.
        maui::core::event<> size_changed;

        // ---- IWindow lifecycle — driven by the host (the application or the native window_handler) ----
        // send_created is NOT part of i_window: in C# the platform window does not raise Created — the
        // application drives it on OpenWindow (Application?.SendStart). The other six ARE the i_window
        // inbound channel the native window's notification trampoline calls.
        void send_created();
        void send_activated() override;
        void send_deactivated() override;
        void send_destroying() override;
        // Resume / sleep (IWindow.Resumed / Stopped / Backgrounding). Each raises its event then routes to
        // the owning application's resume/sleep hook (set by application::open_window). Backgrounding has no
        // persisted-state payload in the port (IPersistedState is out of scope) — it is a bare notification.
        void send_resumed() override;
        void send_stopped() override;
        void send_backgrounding() override;

        [[nodiscard]] bool is_created() const
        {
            return is_created_;
        }
        [[nodiscard]] bool is_activated() const
        {
            return is_activated_;
        }

        // Observable lifecycle (Window.Created/Activated/Deactivated/Destroying + Resumed/Stopped/
        // Backgrounding — EventHandler, no args; Backgrounding's IPersistedState payload is omitted).
        maui::core::event<> created;
        maui::core::event<> activated;
        maui::core::event<> deactivated;
        maui::core::event<> destroying;
        maui::core::event<> resumed;
        maui::core::event<> stopped;
        maui::core::event<> backgrounding;

        // ---- application back-reference (set by application::open_window / cleared by close_window) ----
        // The resume/sleep drive routes through these so a window started by an application resumes/sleeps it
        // (Window.cs Application?.SendResume / SendSleep). Non-owning; both are reset on close.
        void set_resume_hook(std::function<void()> on_resume, std::function<void()> on_sleep)
        {
            on_resume_ = std::move(on_resume);
            on_sleep_ = std::move(on_sleep);
        }

    protected:
        // The hosted page is the window's one logical child (so the window's BindingContext inherits to it).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (content_ != nullptr)
            {
                visit(*content_);
            }
        }

        // The virtual→native seam (like view<>::on_property_changed): a bindable geometry change (X/Y/Width/
        // Height) notifies the handler, which re-runs that property's mapper — unless suppressed during a
        // frame_changed batch (Window.UpdateHandlerValue's _batchFrameUpdate guard).
        void on_property_changed(std::string_view name) override;

    private:
        // Attach / detach the page to this window: set its containing window (firing Loaded/Unloaded down the
        // subtree) and drive Appearing/Disappearing when the page is a content_page (Window.SendWindow*).
        void attach_page();
        void detach_page();
        // Push a geometry/title/content change to the handler (Window.OnPropertyChanged → Handler.UpdateValue),
        // unless suppressed during a frame_changed batch (matching Window.UpdateHandlerValue).
        void update_handler_value(std::string_view property);

        element* content_ = nullptr;                             // NON-owning root page (Window.Page)
        std::shared_ptr<maui::core::i_element_handler> handler_; // the window owns its handler (PROFILE §8)
        std::string title_;
        // Geometry (Window.X/Y/Width/Height). Default Dimension.Unset; frame_changed sets at handler
        // specificity, a developer set at manual specificity (a later manual set then overrides the handler
        // value, per BindableObject precedence + Window.cs).
        maui::core::property<double> x_{*this, window_x_property()};
        maui::core::property<double> y_{*this, window_y_property()};
        maui::core::property<double> width_{*this, window_width_property()};
        maui::core::property<double> height_{*this, window_height_property()};
        std::function<void()> on_resume_; // Application.SendResume hook (set by open_window)
        std::function<void()> on_sleep_;  // Application.SendSleep hook (set by open_window)
        int batch_frame_update_ = 0;      // Window._batchFrameUpdate — suppresses the handler re-push
        bool is_created_ = false;
        bool is_activated_ = false;
    };
} // namespace maui::controls
