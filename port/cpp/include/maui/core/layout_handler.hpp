#pragma once
// maui::core::layout_handler  <=  Microsoft.Maui.Handlers.LayoutHandler
//
// The handler for a layout (ILayout) — the native host panel behind a stack/grid control. Unlike a leaf
// control, a layout computes its own geometry (its layout_manager does measure/arrange); the handler's
// job is to host the children: it owns a native container view and keeps that view's subview list in
// sync with the control's logical children via the i_layout_handler seam (add/remove/clear/insert/
// update/update_z_index). Ported from LayoutHandler.cs + LayoutHandler.iOS.cs (the AppKit panel is a
// plain NSView container).
//
// Same partial-class split + single cross-platform layout_platform struct as the other handlers: the
// mapper TABLES + ctor are cross-platform (layout_handler.cpp); the platform recipe (create + the
// add/remove/… subview wiring) lives per backend under src/platform/<backend>/layout_handler.{cpp,mm}.
//
// The control invokes the child-management commands through the command_mapper, carrying a
// layout_handler_update (index + view) payload — mirroring C#'s LayoutHandlerUpdate. MapAdd/… unwrap the
// payload and forward to the typed i_layout_handler methods.

#include <any>
#include <memory>
#include <string_view>
#include <vector>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_layout_handler.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    class i_view;

    // The command payload for the child-management commands (C# LayoutHandlerUpdate record): the logical
    // index and the (non-owning) child view. `view` is borrowed — the layout control owns the children.
    struct layout_handler_update
    {
        int index = 0;
        i_view* view = nullptr;
    };

    // Derives view_platform_base so the shared view_mapper can push the generic IView properties onto it
    // (headless keeps the base mirrors; Apple overrides update_* to push to the NSView panel).
    struct layout_platform : view_platform_base
    {
        layout_platform() = default;
        ~layout_platform() override; // backend-defined: releases the retained native panel on Apple
        layout_platform(const layout_platform&) = delete;
        layout_platform(layout_platform&&) = delete;
        layout_platform& operator=(const layout_platform&) = delete;
        layout_platform& operator=(layout_platform&&) = delete;

        void* native = nullptr;
        // The hosted children, in subview order — the panel's mirror of the layout's logical children
        // (the Apple build ALSO adds/removes the matching real NSView subviews). children.size() is the
        // hosted child count the headless tests observe as the panel tracks the control's children.
        std::vector<i_view*> children;
        // Headless mirror of ILayout.ClipsToBounds (Apple overrides update_clips_to_bounds to push to the
        // panel's layer.masksToBounds). Lets the headless tests observe the map ran with the right value.
        bool clips_to_bounds = false;

        // ILayout.ClipsToBounds → the native panel's clip flag (Apple: layer.masksToBounds). Layout-specific
        // (not a view_platform_base property), so it lives here; the default body records the mirror and the
        // Apple override pushes to the layer. Defined in the per-backend layout_handler.{cpp,mm}.
        virtual void update_clips_to_bounds(bool value);

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSView panel (defined in
        // src/platform/apple/layout_handler.mm). is_enabled is intentionally NOT overridden — a plain
        // NSView container has no enabled state (unlike NSControl), so it keeps the base mirror.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the NSView panel (M5d native a11y /
        // hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate
        // (mirrors MAUI's LayoutView.HitTest dropping an InputTransparent layout from hit-testing).
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
        // update_clips_to_bounds is declared once above (it is NEW on layout_platform, not a
        // view_platform_base override); the Apple body lives in layout_handler.mm, the headless body in
        // layout_handler.cpp — selected by the backend build, like the other update_* definitions.
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend: push the generic IView properties to the UIView panel (defined in
        // src/platform/ios/layout_handler.mm). is_enabled is intentionally NOT overridden — a plain
        // UIView container has no enabled state (only UIControl has), so it keeps the base mirror.
        // transform IS pushed via the shared ios apply_transform helper (the generic-IView ViewMapper
        // widening); flow_direction still keeps the base mirror for now (see port/STATUS.md).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        // Render transform pushed to the native view via the shared ios apply_transform helper
        // (the generic-IView ViewMapper widening). `native` is this struct's UIView handle.
        void update_transform(const maui::core::transform_spec& value) override;
        // Background / shadow / clip pushed to the panel's layer (ios_visual_ops.hpp: the direct
        // PaintExtensions / ShadowExtensions / WrapperView.SetClip ports).
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the UIView panel
        // (ios_semantics_ops.hpp): semantics → accessibilityLabel/Hint + the Header trait,
        // input_transparent → userInteractionEnabled (UIKit's native flag).
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
        // update_clips_to_bounds (declared once above) pushes UIView.clipsToBounds — the REAL UIKit
        // property C#'s LayoutViewExtensions.UpdateClipsToBounds drives; body in ios/layout_handler.mm.
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (container fan-out): push the generic IView properties to the real
        // dev.mauicpp.MauiLayout ViewGroup over JNI (defined in src/platform/android/layout_handler.cpp).
        // is_enabled is intentionally NOT overridden — a plain ViewGroup container has no enabled state
        // (the leaf widgets carry their own), matching the apple/ios twins, so it keeps the base mirror.
        // Each override calls the view_platform_base body FIRST — the android preset also runs the
        // pure-native cross-platform suite on the emulator WITHOUT a Java VM, and that suite observes the
        // headless mirrors — then pushes to the ViewGroup when one exists. Visibility/opacity/automation_id
        // push directly; transform/flow_direction/background/semantics push through the shared android
        // view/visual/semantics ops (the same set the button partial widens). Shadow, Clip, and
        // InputTransparent keep ONLY the base mirror: on Android those are WrapperView-only (no plain
        // android.view.ViewGroup analog), exactly as the button partial documents. update_clips_to_bounds
        // (declared once above) pushes ViewGroup.setClipChildren/setClipToPadding via the JNI body.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class layout_handler : public view_handler<layout_handler, i_layout, layout_platform>, public i_layout_handler
    {
    public:
        layout_handler();

        static property_mapper<i_layout, layout_handler>& mapper();
        static command_mapper<i_layout, layout_handler>& command_mapper();

        static std::unique_ptr<layout_platform> create_platform_view();

        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // ---- property map functions (the layout's own properties; chained after the shared view_mapper) ----
        // ILayout.ClipsToBounds → push to the panel's clip flag (C# LayoutHandler chains ViewMapper, where
        // MapClipsToBounds → PlatformView.ClipsToBounds; the port pushes via layout_platform).
        static void map_clips_to_bounds(layout_handler& handler, i_layout& layout);

        // ---- i_layout_handler (the child-management seam; defined per backend) ----
        void add(i_view& child) override;
        void remove(i_view& child) override;
        void clear() override;
        void insert(int index, i_view& child) override;
        void update(int index, i_view& child) override;
        void update_z_index(i_view& child) override;

        // ---- command map functions: unwrap the std::any payload and call the typed methods above ----
        static void map_add(layout_handler& handler, i_layout& layout, const std::any& args);
        static void map_remove(layout_handler& handler, i_layout& layout, const std::any& args);
        static void map_clear(layout_handler& handler, i_layout& layout, const std::any& args);
        static void map_insert(layout_handler& handler, i_layout& layout, const std::any& args);
        static void map_update(layout_handler& handler, i_layout& layout, const std::any& args);
        static void map_update_z_index(layout_handler& handler, i_layout& layout, const std::any& args);

#ifdef MAUI_PLATFORM_ANDROID
        // Android container fan-out: wire the MauiLayout's onLayout callback to this handler so a system
        // layout traversal re-runs the cross-platform arrange (children re-positioned host-relative on every
        // pass — the nested-layout fix; see java/MauiLayout.java + src/Core's LayoutViewGroup.OnLayout). The
        // peer (this handler) is installed on connect and cleared on disconnect, before the handler dies.
        // Detected by the view_handler base via `requires` (so the Apple/iOS twins, which omit these,
        // compile unchanged — AppKit/UIKit never re-lay-out subviews, so they need no onLayout callback).
        void on_connect_handler(layout_platform& platform);
        void on_disconnect_handler(layout_platform& platform);
#endif
    };
} // namespace maui::core
