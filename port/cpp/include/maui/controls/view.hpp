#pragma once
// maui::controls::view<ViewInterface>  <=  Microsoft.Maui.Controls.View / VisualElement (minimal M2)
//
// The reusable base for concrete controls: a bindable_object that implements the i_view boilerplate
// (handler ownership + the virtual-view ⇄ handler wiring, geometry, the measure/arrange seam) so each
// control only adds its own interface members. This is the FIRST cut — only the members the handler
// seam needs are real; the full VisualElement property set (transforms, the bindable IsEnabled/Opacity/
// Visibility/WidthRequest/… and real layout) arrives in M3/M4. Documented gaps, not silent.
//
// Why a template parameter instead of `view : i_view`? A concrete control is e.g. `button : i_button,
// i_text`, and i_button already derives i_view. If `view` also derived i_view, the control would
// inherit i_view twice (a diamond). Parameterizing on the control's view-interface (`view<i_button>`)
// gives a single i_view subobject with zero virtual-inheritance overhead: view<ViewInterface> derives
// ViewInterface (which derives i_view) and supplies the i_view method bodies; the control supplies the
// interface-specific members.

#include <algorithm>
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/controls/behavior.hpp"               // --- styles tail (W1-15) ---
#include "maui/controls/brushes/brush.hpp"          // X1: VisualElement.Background accepts a Brush (bridged to paint)
#include "maui/controls/brushes/gradient_brush.hpp" // W7: re-derive the bridged paint when a gradient brush mutates
#include "maui/controls/element.hpp"
#include "maui/controls/gestures/gesture_platform_manager.hpp" // --- gestures (W1-12) ---
#include "maui/controls/style.hpp"
#include "maui/controls/trigger.hpp" // VisualElement.Triggers collection
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/event.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/focus_request.hpp"
#include "maui/core/i_context_flyout_element.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_flyout.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_layout.hpp"
#include "maui/core/i_shadow.hpp"
#include "maui/core/i_tool_tip_element.hpp" // --- chrome (W1-11) ---
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/property.hpp"
#include "maui/core/semantics.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    // X1 brush bridge: defined in src/controls/brushes/brush_paint_bridge.cpp. Declared here so the
    // set_background(brush) template overload can convert without pulling the full bridge header into this
    // central control header.
    [[nodiscard]] std::shared_ptr<maui::graphics::paint> to_paint(const std::shared_ptr<brush>& source);

    // The shared bindable-property descriptors for the four generic IView properties (VisualElement's
    // IsEnabled / Opacity / IsVisible(Visibility) + Element's AutomationId). They are NON-template free
    // functions — one descriptor per property, shared across EVERY view<ViewInterface> instantiation —
    // because the descriptor identity must match the view_mapper's keys regardless of the concrete
    // control type. (A static data member of the template would mint a distinct descriptor per
    // ViewInterface.) Defined out-of-line in src/controls/view.cpp. Names match the view_mapper keys.
    const maui::core::bindable_property<bool>& is_enabled_property();
    const maui::core::bindable_property<double>& opacity_property();
    const maui::core::bindable_property<maui::core::visibility>& visibility_property();
    const maui::core::bindable_property<std::string>& automation_id_property();

    // The render-transform descriptors (VisualElement's transform set) + FlowDirection — likewise
    // NON-template shared free-function descriptors, one per property, names matching the view_mapper
    // keys. Defaults are the identity transform: translations/rotations 0, scales 1, anchors 0.5
    // (VisualElement.cs). FlowDirection defaults to MatchParent (FlowDirection.cs).
    const maui::core::bindable_property<double>& translation_x_property();
    const maui::core::bindable_property<double>& translation_y_property();
    const maui::core::bindable_property<double>& scale_property();
    const maui::core::bindable_property<double>& scale_x_property();
    const maui::core::bindable_property<double>& scale_y_property();
    const maui::core::bindable_property<double>& rotation_property();
    const maui::core::bindable_property<double>& rotation_x_property();
    const maui::core::bindable_property<double>& rotation_y_property();
    const maui::core::bindable_property<double>& anchor_x_property();
    const maui::core::bindable_property<double>& anchor_y_property();
    const maui::core::bindable_property<maui::core::flow_direction>& flow_direction_property();

    // The three visual-layer descriptors (VisualElement's Background / Shadow / Clip). The control OWNS
    // each object via a property<shared_ptr<...>> (so a set flows through the same value engine +
    // on_property_changed → handler->update_value → the chained view_mapper as every other property);
    // i_view returns the raw .get() borrow. NON-template shared free-function descriptors, one per
    // property, names matching the view_mapper keys. Defaults are null (unset). Defined in view.cpp.
    const maui::core::bindable_property<std::shared_ptr<maui::graphics::paint>>& background_property();
    const maui::core::bindable_property<std::shared_ptr<maui::core::i_shadow>>& shadow_property();
    const maui::core::bindable_property<std::shared_ptr<maui::graphics::i_shape>>& clip_property();

    // Accessibility metadata (Semantics — the control owns the object) + InputTransparent (a bindable bool).
    // Shared NON-template descriptors, names matching the view_mapper keys.
    const maui::core::bindable_property<std::shared_ptr<maui::core::semantics>>& semantics_property();
    const maui::core::bindable_property<bool>& input_transparent_property();

    // The six size-request descriptors (VisualElement.WidthRequest / HeightRequest / MinimumWidthRequest /
    // MinimumHeightRequest / MaximumWidthRequest / MaximumHeightRequest). NON-template shared free-function
    // descriptors; defaults -1 (width/height/minimums = "size to content") and +inf (maximums = no cap),
    // matching VisualElement.cs. The names are the IView keys (width/height/minimum_*/maximum_*) so a
    // request change re-runs the matching mapper. i_view::width()/... derive their values from these
    // requests (NOT from the arranged frame), so the layout managers read the developer's request.
    const maui::core::bindable_property<double>& width_request_property();
    const maui::core::bindable_property<double>& height_request_property();
    const maui::core::bindable_property<double>& minimum_width_request_property();
    const maui::core::bindable_property<double>& minimum_height_request_property();
    const maui::core::bindable_property<double>& maximum_width_request_property();
    const maui::core::bindable_property<double>& maximum_height_request_property();

    // The front-to-back ordering within a layout (VisualElement.ZIndex). NON-template shared descriptor;
    // default 0. A change re-stacks the element among its siblings (see view<>::on_property_changed).
    const maui::core::bindable_property<int>& z_index_property();

    // chrome (W1-11): the attached ToolTipProperties.Text storage (the per-instance store of the C#
    // attached BindableProperty). Shared NON-template descriptor named "tool_tip" — the view_mapper key.
    const maui::core::bindable_property<std::string>& tool_tip_text_property();

    // The per-axis layout alignment (View.HorizontalOptions / VerticalOptions, both defaulting to
    // LayoutOptions.Fill). NON-template shared free-function descriptors, one per axis, defaulting to
    // layout_alignment::fill. i_view::horizontal_layout_alignment()/vertical_layout_alignment() return the
    // stored value (no longer a hardcoded fill), so a developer's Start/Center/End is honored by the
    // arrange-time ComputeFrame (view<>::arrange / layout::arrange) the same way C# View.HorizontalOptions
    // feeds IView.HorizontalLayoutAlignment → LayoutExtensions.ComputeFrame.
    const maui::core::bindable_property<maui::core::layout_alignment>& horizontal_layout_alignment_property();
    const maui::core::bindable_property<maui::core::layout_alignment>& vertical_layout_alignment_property();

    // View.Margin (a Thickness, default zero): the space reserved AROUND this view, outside its frame.
    // NON-template shared free-function descriptor named "margin". Margin is a LAYOUT-ONLY property —
    // there is no native handler mapper for it (C# ViewHandler has no MapMargin); it is consumed purely by
    // the measure/arrange seam (ComputeDesiredSize adds it to the reported size, ComputeFrame subtracts it
    // back out and offsets the frame), exactly like the two layout-alignment descriptors above. A change
    // re-runs no mapper (update_value is a harmless no-op for "margin") but invalidates measure so the
    // parent re-lays-out — see view<>::on_property_changed (C# View.MarginPropertyChanged →
    // InvalidateMeasureInternal(MarginChanged)). Default thickness{} mirrors C#'s default(Thickness).
    const maui::core::bindable_property<maui::core::thickness>& margin_property();

    // chrome (W1-11): every view is a tool-tip + context-flyout element, exactly as C# VisualElement
    // implements IToolTipElement + IContextFlyoutElement (the shared view_mapper discovers both by
    // dynamic_cast, like C#'s `view is IToolTipElement` checks in ViewHandler.MapToolTip/MapContextFlyout).
    template <class ViewInterface>
    class view : public maui::controls::element,
                 public ViewInterface,
                 public maui::core::i_tool_tip_element,
                 public maui::core::i_context_flyout_element
    {
        static_assert(std::is_base_of_v<maui::core::i_view, ViewInterface>,
                      "ViewInterface must derive maui::core::i_view");

    public:
        // ---- i_element ----
        [[nodiscard]] const std::shared_ptr<maui::core::i_element_handler>& handler() const override
        {
            return handler_;
        }
        // Setting the handler wires the seam: the incoming handler binds to this view (creating the
        // platform view + running the mapper), then any previous handler is disconnected. Mirrors
        // Element.Handler's setter (SetVirtualView(this) on the new one, DisconnectHandler() on the old).
        void set_handler(std::shared_ptr<maui::core::i_element_handler> value) override
        {
            if (handler_ == value)
            {
                return;
            }
            std::shared_ptr<maui::core::i_element_handler> const previous = handler_;
            handler_ = std::move(value);
            if (handler_)
            {
                handler_->set_virtual_view(*this);
            }
            if (previous && previous != handler_)
            {
                previous->disconnect_handler();
            }
            // --- gestures (W1-12): GestureManager.SetupGestureManager — re-seat the native gesture
            // recognizers on the new handler's platform view (detaching from the old one first) ---
            gesture_manager_.set_handler(dynamic_cast<maui::core::i_view_handler*>(handler_.get()), *this,
                                         gesture_recognizers_);
            // --- end gestures (W1-12) ---
        }
        [[nodiscard]] std::shared_ptr<maui::core::i_element> parent() const override
        {
            return parent_.lock();
        }

        // ---- i_transform (bindable; each change flows through the chained view_mapper's map_transform,
        // which rebuilds the whole render transform from all nine scalars) ----
        [[nodiscard]] double translation_x() const override
        {
            return translation_x_.get();
        }
        void set_translation_x(double value)
        {
            translation_x_.set(value);
        }
        [[nodiscard]] double translation_y() const override
        {
            return translation_y_.get();
        }
        void set_translation_y(double value)
        {
            translation_y_.set(value);
        }
        [[nodiscard]] double scale() const override
        {
            return scale_.get();
        }
        void set_scale(double value)
        {
            scale_.set(value);
        }
        [[nodiscard]] double scale_x() const override
        {
            return scale_x_.get();
        }
        void set_scale_x(double value)
        {
            scale_x_.set(value);
        }
        [[nodiscard]] double scale_y() const override
        {
            return scale_y_.get();
        }
        void set_scale_y(double value)
        {
            scale_y_.set(value);
        }
        [[nodiscard]] double rotation() const override
        {
            return rotation_.get();
        }
        void set_rotation(double value)
        {
            rotation_.set(value);
        }
        [[nodiscard]] double rotation_x() const override
        {
            return rotation_x_.get();
        }
        void set_rotation_x(double value)
        {
            rotation_x_.set(value);
        }
        [[nodiscard]] double rotation_y() const override
        {
            return rotation_y_.get();
        }
        void set_rotation_y(double value)
        {
            rotation_y_.set(value);
        }
        [[nodiscard]] double anchor_x() const override
        {
            return anchor_x_.get();
        }
        void set_anchor_x(double value)
        {
            anchor_x_.set(value);
        }
        [[nodiscard]] double anchor_y() const override
        {
            return anchor_y_.get();
        }
        void set_anchor_y(double value)
        {
            anchor_y_.set(value);
        }

        // ---- i_view ----
        [[nodiscard]] std::string_view automation_id() const override
        {
            return automation_id_.get();
        }
        void set_automation_id(std::string value)
        {
            automation_id_.set(std::move(value));
        }
        [[nodiscard]] maui::core::flow_direction flow_direction() const override
        {
            return flow_direction_.get();
        }
        void set_flow_direction(maui::core::flow_direction value)
        {
            flow_direction_.set(value);
        }
        // View.HorizontalOptions / VerticalOptions (bindable; default Fill). C# exposes the LayoutOptions
        // struct (alignment + the legacy StackLayout-only Expands bit); the IView contract — and thus the
        // port — surfaces only the resolved Primitives.LayoutAlignment (HorizontalOptions.ToCore()). A set
        // flows through on_property_changed (so binding/styles/setters apply) and is consumed at arrange
        // time by compute_frame; there is no native mapper for it (alignment is layout-only, not a handler
        // property in C#), so update_value is a harmless no-op for the "horizontal_/vertical_layout_alignment"
        // keys.
        [[nodiscard]] maui::core::layout_alignment horizontal_layout_alignment() const override
        {
            return horizontal_layout_alignment_.get();
        }
        void set_horizontal_layout_alignment(maui::core::layout_alignment value)
        {
            horizontal_layout_alignment_.set(value);
        }
        [[nodiscard]] maui::core::layout_alignment vertical_layout_alignment() const override
        {
            return vertical_layout_alignment_.get();
        }
        void set_vertical_layout_alignment(maui::core::layout_alignment value)
        {
            vertical_layout_alignment_.set(value);
        }
        // Accessibility metadata (bindable; the control owns the semantics object). i_view hands back the
        // raw borrow; the chained view_mapper's map_semantics pushes it to the platform base.
        [[nodiscard]] maui::core::semantics* semantics() const override
        {
            return semantics_.get().get();
        }
        // The control takes ownership of the semantics object. Passing a distinct instance fires the change.
        void set_semantics(std::shared_ptr<maui::core::semantics> value)
        {
            semantics_.set(std::move(value));
        }
        // The three visual-layer properties are bindable (each change flows through on_property_changed →
        // handler->update_value → the chained view_mapper's map_clip / map_shadow / map_background). The
        // control owns the object (property<shared_ptr<...>>); i_view hands back the raw .get() borrow.
        [[nodiscard]] maui::graphics::i_shape* clip() const override
        {
            return clip_.get().get();
        }
        // The control takes ownership of the clip shape. Passing a distinct instance fires the change.
        void set_clip(std::shared_ptr<maui::graphics::i_shape> value)
        {
            clip_.set(std::move(value));
        }
        [[nodiscard]] maui::core::i_shadow* shadow() const override
        {
            return shadow_.get().get();
        }
        // The control takes ownership of the shadow. Passing a distinct instance fires the change.
        void set_shadow(std::shared_ptr<maui::core::i_shadow> value)
        {
            shadow_.set(std::move(value));
        }
        [[nodiscard]] maui::graphics::paint* background() const override
        {
            return background_.get().get();
        }
        // The control takes ownership of the background paint. Passing a distinct instance fires the change.
        void set_background(std::shared_ptr<maui::graphics::paint> value)
        {
            background_brush_subscription_.reset(); // and its invalidate subscription (W7)
            background_brush_.reset();              // a raw-paint background clears any brush previously set
            background_.set(std::move(value));
        }

        // X1 — VisualElement.Background as a Brush (the developer-facing surface). The view owns the brush
        // for BindingContext inheritance (SetInheritedBindingContext on the brush, mirroring C#
        // VisualElement.SetInheritedBindingContext(Background, …)) and bridges it to the graphics::paint the
        // native layer already renders (Brush's implicit operator Paint). Setting null clears both. Named
        // set_background_brush (not an overload) so the existing set_background(nullptr)/(paint) calls stay
        // unambiguous — non-breaking: the paint-typed setter/getter above is untouched; handlers keep
        // consuming background().
        void set_background_brush(std::shared_ptr<brush> value)
        {
            background_brush_subscription_.reset(); // drop any subscription to the previous brush first
            background_brush_ = std::move(value);
            if (background_brush_)
            {
                background_brush_->set_inherited_binding_context(this->raw_binding_context());
                // A gradient brush's stops can change after assignment (the XAML loader adds them AFTER
                // setting Background; runtime mutation too). Subscribe to its invalidate event so the bridged
                // paint is re-derived then — without this the cached paint keeps the empty-stops snapshot
                // taken here and the gradient renders blank. (MAUI's VisualElement does the same via
                // InvalidateGradientBrushRequested.)
                if (auto* gradient = dynamic_cast<gradient_brush*>(background_brush_.get()))
                {
                    background_brush_subscription_ =
                        maui::core::connect_scoped(gradient->invalidate_gradient_brush_requested,
                                                   [this] { background_.set(to_paint(background_brush_)); });
                }
            }
            background_.set(to_paint(background_brush_));
        }
        // The owned background brush (null when the background was set as a raw paint, or never set).
        [[nodiscard]] const std::shared_ptr<brush>& background_brush() const
        {
            return background_brush_;
        }
        [[nodiscard]] maui::core::visibility visibility() const override
        {
            return visibility_.get();
        }
        void set_visibility(maui::core::visibility value)
        {
            visibility_.set(value);
        }
        [[nodiscard]] double opacity() const override
        {
            return opacity_.get();
        }
        void set_opacity(double value)
        {
            opacity_.set(value);
        }
        // VisualElement.IsEnabledCore / IView.IsEnabled — the EFFECTIVE (coerced) enabled state, which is
        // what the handler is pushed (the view_mapper's map_is_enabled reads THIS). An explicitly disabled
        // view is disabled no matter what (the C# `_isEnabledExplicit == false` short-circuit); otherwise it
        // inherits its parent's effective enabled (parent.IsEnabled, itself already coerced — so a single
        // immediate-parent check cascades the whole logical-parent chain). The raw developer-set value is
        // is_explicitly_enabled().
        [[nodiscard]] bool is_enabled() const override
        {
            if (!is_enabled_.get())
            {
                return false; // explicitly disabled — nothing else matters (C# IsEnabledCore short-circuit)
            }
            if (auto* parent_view = dynamic_cast<maui::core::i_view*>(this->logical_parent()))
            {
                return parent_view->is_enabled(); // the parent's coerced value (recurses up the chain)
            }
            return true;
        }
        // VisualElement.IsExplicitlyEnabled (_isEnabledExplicit): the developer-set IsEnabled, BEFORE the
        // parent-chain coercion is_enabled() applies. set_is_enabled writes this raw value; the cascade
        // (refresh_is_enabled_subtree) re-coerces it against the changed ancestor chain.
        [[nodiscard]] bool is_explicitly_enabled() const
        {
            return is_enabled_.get();
        }
        void set_is_enabled(bool value)
        {
            is_enabled_.set(value);
        }
        [[nodiscard]] bool is_focused() const override
        {
            return is_focused_;
        }
        // The IsFocused funnel (VisualElement.OnIsFocusedPropertyChanged): this is the INBOUND channel the
        // native focus/blur callback drives — and the focus/unfocus command mappers reflect the native
        // first-responder result through it. When the value actually changes it fires Focused (→ true) or
        // Unfocused (→ false) and runs ChangeVisualState, exactly like C#'s IsFocused bindable-property
        // changed callback. A redundant set (same value) is silent, matching the property no-op.
        void set_is_focused(bool value) override
        {
            if (is_focused_ == value)
            {
                return;
            }
            is_focused_ = value;
            if (is_focused_)
            {
                focused.raise(true); // VisualElement.OnFocused → Focused(FocusEventArgs(this, true))
            }
            else
            {
                unfocused.raise(false); // VisualElement.OnUnfocus → Unfocused(FocusEventArgs(this, false))
            }
            change_visual_state();
            // C# Element.OnPropertyChanged → UpdateHandlerValue: every property change forwards to the
            // handler's mapper. The IsFocused change drives the input handlers' MapIsFocused (Entry/Editor/
            // SearchBar.Mapper.cs AppendToMapping(nameof(IsFocused), InputView.MapIsFocused)), which arms /
            // disarms the HideSoftInputOnTapped tap gesture. A no-op for handlers without an is_focused key.
            if (handler_)
            {
                handler_->update_value("is_focused");
            }
        }
        // InputTransparent (bindable; flows through the chained view_mapper's map_input_transparent).
        [[nodiscard]] bool input_transparent() const override
        {
            return input_transparent_.get();
        }
        void set_input_transparent(bool value)
        {
            input_transparent_.set(value);
        }
        [[nodiscard]] maui::graphics::rect frame() const override
        {
            return frame_;
        }
        void set_frame(maui::graphics::rect value) override
        {
            frame_ = value;
        }
        // i_view::width()/height()/minimum_*/maximum_* derive from the SIZE REQUESTS (not the arranged
        // frame, which stays in frame_) — mirroring VisualElement's explicit IView.Width/... mapping so the
        // layout managers read the developer's request. Width/Height: unset OR an explicit -1 → Unset(NaN)
        // ("size to content"); otherwise EnsurePositive(request). Minimum*: unset → Unset(NaN) (no minimum);
        // else EnsurePositive(request). Maximum*: always EnsurePositive(request) (default +inf = no cap).
        [[nodiscard]] double width() const override
        {
            return resolve_request(width_request_, false);
        }
        [[nodiscard]] double minimum_width() const override
        {
            return resolve_minimum_request(minimum_width_request_);
        }
        [[nodiscard]] double maximum_width() const override
        {
            return ensure_positive(maximum_width_request_.get());
        }
        [[nodiscard]] double height() const override
        {
            return resolve_request(height_request_, false);
        }
        [[nodiscard]] double minimum_height() const override
        {
            return resolve_minimum_request(minimum_height_request_);
        }
        [[nodiscard]] double maximum_height() const override
        {
            return ensure_positive(maximum_height_request_.get());
        }
        // The developer-facing size requests (VisualElement.WidthRequest / HeightRequest / Minimum* /
        // Maximum*). Setting one re-runs the matching mapper (through on_property_changed) and changes the
        // size the next layout pass resolves; it does NOT immediately change the arranged frame.
        [[nodiscard]] double width_request() const
        {
            return width_request_.get();
        }
        void set_width_request(double value)
        {
            width_request_.set(value);
        }
        [[nodiscard]] double height_request() const
        {
            return height_request_.get();
        }
        void set_height_request(double value)
        {
            height_request_.set(value);
        }
        [[nodiscard]] double minimum_width_request() const
        {
            return minimum_width_request_.get();
        }
        void set_minimum_width_request(double value)
        {
            minimum_width_request_.set(value);
        }
        [[nodiscard]] double minimum_height_request() const
        {
            return minimum_height_request_.get();
        }
        void set_minimum_height_request(double value)
        {
            minimum_height_request_.set(value);
        }
        [[nodiscard]] double maximum_width_request() const
        {
            return maximum_width_request_.get();
        }
        void set_maximum_width_request(double value)
        {
            maximum_width_request_.set(value);
        }
        [[nodiscard]] double maximum_height_request() const
        {
            return maximum_height_request_.get();
        }
        void set_maximum_height_request(double value)
        {
            maximum_height_request_.set(value);
        }
        // View.Margin (bindable; default zero). Consumed by the measure/arrange seam: measure adds it to the
        // reported desired size (C# LayoutExtensions.ComputeDesiredSize) and compute_frame subtracts it back
        // out and offsets the frame by margin.left/top (ComputeFrame), so the margin is reserved space around
        // the view's frame. Setting it invalidates measure (on_property_changed), mirroring C#
        // View.MarginPropertyChanged → InvalidateMeasureInternal(MarginChanged).
        [[nodiscard]] maui::core::thickness margin() const override
        {
            return margin_.get();
        }
        void set_margin(maui::core::thickness value)
        {
            margin_.set(value);
        }
        [[nodiscard]] maui::graphics::size desired_size() const override
        {
            return desired_size_;
        }
        [[nodiscard]] int z_index() const override
        {
            return z_index_.get();
        }
        // The front-to-back order within the parent layout (VisualElement.ZIndex). Setting it re-stacks
        // this element among its siblings (on_property_changed routes the change to the parent layout).
        void set_z_index(int value)
        {
            z_index_.set(value);
        }
        // The measure/arrange seam (C# VisualElement.ArrangeOverride): resolve this view's FRAME within the
        // allotted `bounds` via compute_frame (the LayoutExtensions.ComputeFrame port — it honors this view's
        // HorizontalLayoutAlignment / VerticalLayoutAlignment and excludes the margin), store it, then push
        // that frame to the platform view (IViewHandler.PlatformArrange). The returned size is the frame's
        // size (C# `return Frame.Size`), not the raw bounds — so a Start/Center/End child reports its aligned
        // extent. A leaf control with no handler still computes + stores the aligned frame.
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            frame_ = compute_frame(bounds);
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler_.get()))
            {
                view_handler->platform_arrange(frame_);
            }
            return frame_.size();
        }
        // The leaf-control measure seam (C# View.MeasureOverride → LayoutExtensions.ComputeDesiredSize):
        // ask the handler for the content size, then resolve it against this view's OWN size requests so the
        // reported desired size honors WidthRequest/Minimum*/Maximum* — the per-child clamp the layout
        // managers rely on (ViewHandlerExtensions.ResolveConstraints). With no handler the content size is
        // zero, still resolved against the requests so an explicit Width/Height request is reported.
        //
        // ComputeDesiredSize folds the Margin in symmetrically with the arrange-side ComputeFrame
        // (compute_frame below): the margin is EXCLUDED from the content measurement (the constraints shrink
        // by it, so a wrapping child gets its true available space) and ADDED back into the reported desired
        // size, marking the margin as reserved space. compute_frame then subtracts it back out, so the frame
        // is the margin-inset extent — measure adds, arrange subtracts, and they balance to zero net at zero
        // margin (the prior behavior).
        maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            const maui::core::thickness view_margin = this->margin();
            maui::graphics::size content{};
            bool content_floor = false;
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler_.get()))
            {
                content = view_handler->get_desired_size(width_constraint - view_margin.horizontal_thickness(),
                                                         height_constraint - view_margin.vertical_thickness());
                content_floor = view_handler->content_is_minimum_size();
            }
            double resolved_width = resolve_size_request(content.width, width(), minimum_width(), maximum_width());
            double resolved_height = resolve_size_request(content.height, height(), minimum_height(), maximum_height());
            if (content_floor)
            {
                // The handler reports its measured content as a hard lower bound (a native iOS/macOS button
                // cannot render narrower/shorter than its title + insets — MAUI surfaces this via its
                // NeedsContainer WrapperView, which the port lacks). So an explicit WidthRequest/HeightRequest
                // smaller than the content GROWS to the content instead of truncating it, while Maximum* still
                // caps the result. Matches the maui-compare reference for ClippingPage Layout2 (the
                // WidthRequest=50 "Hey" buttons render at their full natural width).
                resolved_width = std::max(resolved_width, std::min(content.width, maximum_width()));
                resolved_height = std::max(resolved_height, std::min(content.height, maximum_height()));
            }
            desired_size_ = {resolved_width + view_margin.horizontal_thickness(),
                             resolved_height + view_margin.vertical_thickness()};
            return desired_size_;
        }
        void invalidate_measure() override
        {
            // Layout invalidation is wired in M3 (the layout pass); no-op for the M2 seam.
        }
        void invalidate_arrange() override
        {
        }
        // VisualElement.Focus → ViewExtensions.RequestFocus / MapFocus: already-focused returns true
        // (MapFocus's own early-out). With a handler attached, invoke its Focus command with a focus_request
        // payload (the chained view_command_mapper's map_focus asks the native view for first responder,
        // records the realized result on the request, and reflects it onto IsFocused — which fires Focused
        // here through set_is_focused); return the realized result. With NO handler there is no platform to
        // take focus, so — exactly like MapFocus's "nothing handled this" tail — nothing changes and the
        // result is false.
        bool focus() override
        {
            if (is_focused_)
            {
                return true;
            }
            if (handler_)
            {
                maui::core::focus_request request;
                handler_->invoke("focus", request);
                return request.result();
            }
            return false;
        }
        // VisualElement.Unfocus: if not focused, do nothing (its IsFocused guard); otherwise invoke the
        // handler's Unfocus command (the chained view_command_mapper resigns the native first responder and
        // clears IsFocused — firing Unfocused here). With no handler `Handler?.Invoke` is a no-op (C#
        // leaves IsFocused for the native callback) — and a view with no handler can never have become
        // focused, so the guard above has already returned.
        void unfocus() override
        {
            if (!is_focused_)
            {
                return;
            }
            if (handler_)
            {
                handler_->invoke("unfocus");
            }
        }

        // ---- focus events (VisualElement.Focused / VisualElement.Unfocused) ----
        // Each carries the IsFocused bool of FocusEventArgs (true for focused, false for unfocused).
        // Fired by set_is_focused when the focus state actually changes (OnFocused / OnUnfocus).
        maui::core::event<bool> focused;
        maui::core::event<bool> unfocused;

        // --- styles/resources (M5d) ---------------------------------------------------------------------
        // ---- style (VisualElement.Style / IStyleElement) ----
        // Setting a style applies its setters at the local-style specificity; replacing or clearing one
        // un-applies the previous style first (so its setter values are removed before the new ones land). A
        // base_resource_key on the style resolves against this element's resource chain (resource_resolver).
        // The type is qualified (maui::controls::style) because the accessor below is also named `style`.
        void set_style(std::shared_ptr<maui::controls::style> value)
        {
            if (style_ == value)
            {
                return;
            }
            const auto resolve = make_resource_resolver();
            if (style_)
            {
                style_->unapply(*this, maui::core::setter_specificity::style_local, resolve);
            }
            style_ = std::move(value);
            if (style_)
            {
                style_->apply(*this, maui::core::setter_specificity::style_local, resolve);
            }
        }
        [[nodiscard]] const std::shared_ptr<maui::controls::style>& style() const
        {
            return style_;
        }

        // The style classes this control selects (VisualElement.StyleClass): a control picks up the class
        // styles registered under each name in its resource chain (layered implicit < class < local). Setting
        // it re-resolves the merged style. set_dynamic_resource(name, key) (inherited from element) binds a
        // property to a resource key; the implicit style is resolved automatically when the control enters a
        // resource scope.
        void set_style_class(std::vector<std::string> classes)
        {
            style_class_ = classes;
            this->set_merged_style_classes(std::move(classes));
        }
        [[nodiscard]] const std::vector<std::string>& style_class() const
        {
            return style_class_;
        }
        // --- end styles/resources (M5d) -----------------------------------------------------------------

        // ---- Visual States (VisualStateManager.VisualStateGroups + VisualElement.ChangeVisualState) ----
        // The control's visual-state manager (configure groups/states on it, then change_visual_state()
        // drives the common states). Held by value — an empty manager makes change_visual_state() a no-op,
        // so this is free until a developer adds groups.
        [[nodiscard]] maui::controls::visual_state_manager& visual_states()
        {
            return visual_states_;
        }
        [[nodiscard]] const maui::controls::visual_state_manager& visual_states() const
        {
            return visual_states_;
        }
        // VisualElement.ChangeVisualState: go to Disabled when !IsEnabled, else Focused when focused, else
        // Normal. Driven automatically on an IsEnabled change (and on focus/unfocus); also call it once
        // after configuring the groups to apply the initial state. A no-op when no matching group exists.
        // Virtual, like C#'s `protected internal virtual ChangeVisualState` — stateful controls extend it
        // with their own states (Switch's On/Off, CheckBox's IsChecked).
        virtual void change_visual_state()
        {
            using common = maui::controls::common_states;
            // The disabled state reflects the EFFECTIVE (coerced) IsEnabled, so a child of a disabled parent
            // is visually disabled too (C# ChangeVisualState reads IsEnabled, which is IsEnabledCore).
            const std::string_view target =
                !is_enabled() ? common::disabled : (is_focused_ ? common::focused : common::normal);
            visual_states_.go_to_state(*this, target);
        }

        // C# IsEnabled cascade hook (element::refresh_is_enabled_subtree): an ancestor's IsEnabled changed,
        // so re-push THIS view's now-recomputed effective IsEnabled (map_is_enabled re-reads is_enabled())
        // and re-run ChangeVisualState, then recurse into the logical children so the whole subtree settles.
        // The walk is strictly top-down over the finite logical tree, so it always terminates (no infinite
        // recursion); a null handler is skipped. The re-push is unconditional and idempotent — C# prunes via
        // the IsEnabled bindable's own change-detection, but re-reading the coerced value here is
        // observationally identical (map_is_enabled and go_to_state are both idempotent) and avoids a stale
        // coerced-state cache.
        void refresh_is_enabled_subtree() override
        {
            if (handler_)
            {
                handler_->update_value("is_enabled");
            }
            change_visual_state();
            for_each_logical_child([](maui::controls::element& child) { child.refresh_is_enabled_subtree(); });
        }

        // --- gestures (W1-12) ---------------------------------------------------------------------------
        // View.GestureRecognizers: the recognizers attached to this view. add/remove parents the
        // recognizer to this view (Element.Parent + BindingContext/Window inheritance) and re-syncs the
        // native attachments through the gesture platform manager (the member-initializer hooks below —
        // the port's CollectionChanged subscription).
        [[nodiscard]] gesture_recognizer_collection& gesture_recognizers()
        {
            return gesture_recognizers_;
        }
        [[nodiscard]] const gesture_recognizer_collection& gesture_recognizers() const
        {
            return gesture_recognizers_;
        }
        // The per-view gesture platform manager (View._gestureManager). Exposed for attachment
        // observability + the synthetic dispatch (the headless stand-in for native gesture events);
        // real input flows from the native recognizers the manager attaches.
        [[nodiscard]] gesture_platform_manager& gesture_manager()
        {
            return gesture_manager_;
        }
        // --- end gestures (W1-12) -----------------------------------------------------------------------

        // --- chrome (W1-11): ToolTip + ContextFlyout ------------------------------------------------------
        // i_tool_tip_element: nullopt until the attached text is set (C# ToolTipProperties.GetToolTip's
        // IsSet probe). The setter is what ToolTipProperties.SetText delegates to; the property change
        // flows through on_property_changed → handler->update_value("tool_tip") → the chained
        // view_mapper's map_tool_tip (NSView.toolTip on AppKit; documented no-op on plain iOS).
        [[nodiscard]] std::optional<std::string> tool_tip() const override
        {
            return tool_tip_text_.is_set() ? std::optional<std::string>{tool_tip_text_.get()} : std::nullopt;
        }
        void set_tool_tip_text(std::string value)
        {
            tool_tip_text_.set(std::move(value));
        }

        // i_context_flyout_element: the attached right-click/long-press menu (C# FlyoutBase's attached
        // ContextFlyout). NON-owning — the caller owns the flyout (PROFILE §8). Setting it re-parents the
        // flyout into this view's logical tree (C# AddRemoveLogicalChildren — so the flyout and its items
        // inherit this view's BindingContext) and notifies the handler so the chained view_mapper's
        // map_context_flyout materializes the native menu (NSView.menu / UIContextMenuInteraction).
        [[nodiscard]] maui::core::i_flyout* context_flyout() const override
        {
            return context_flyout_;
        }
        void set_context_flyout(maui::core::i_flyout* value)
        {
            if (context_flyout_ == value)
            {
                return;
            }
            if (context_flyout_element_ != nullptr)
            {
                detach_logical_child(*context_flyout_element_);
            }
            context_flyout_ = value;
            context_flyout_element_ = dynamic_cast<element*>(context_flyout_);
            if (context_flyout_element_ != nullptr)
            {
                attach_logical_child(*context_flyout_element_);
            }
            if (handler_)
            {
                handler_->update_value("context_flyout");
            }
        }
        // --- end chrome (W1-11) ---------------------------------------------------------------------------

    protected:
        view() = default;

        // The virtual→native seam: any bindable property change notifies the handler, which re-runs
        // that property's mapper. Mirrors MAUI, where a BindableProperty change calls Handler.UpdateValue.
        // An IsEnabled change additionally drives the visual state (VisualElement.OnIsEnabledPropertyChanged
        // → ChangeVisualState), so a configured Disabled/Normal state applies automatically.
        void on_property_changed(std::string_view name) override
        {
            // Route through element (not straight to bindable_object) so attached effects observe the
            // change (Element.OnPropertyChanged fans SendOnElementPropertyChanged out — effects, G3).
            maui::controls::element::on_property_changed(name);
            if (handler_)
            {
                handler_->update_value(name);
            }
            if (name == "is_enabled")
            {
                change_visual_state();
                // C# OnIsEnabledPropertyChanged → PropagatePropertyChanged(IsEnabledProperty.PropertyName):
                // the new effective state cascades to every descendant, each re-coercing against this (now-
                // changed) ancestor and re-pushing to its handler. THIS element was already pushed by the
                // generic update_value(name) above, so the cascade only descends into the logical children.
                for_each_logical_child([](maui::controls::element& child) { child.refresh_is_enabled_subtree(); });
            }
            if (name == "z_index")
            {
                update_z_order();
            }
            // C# View.MarginPropertyChanged → InvalidateMeasureInternal(InvalidationTrigger.MarginChanged):
            // a margin change re-runs no native mapper (the update_value above is a harmless no-op — margin
            // is a layout-only property, like the layout alignments) but invalidates measure so the parent
            // re-lays-out with the new reserved space. invalidate_measure is the M3 no-op seam today (see
            // below), so the structure is faithful and future-proof — the moment real measure-invalidation
            // propagation lands, a margin change participates.
            if (name == "margin")
            {
                this->invalidate_measure();
            }
        }

        // --- gestures (W1-12): View.OnBindingContextChanged — after the base propagation to the
        // logical children, hand the (possibly inherited) context to every gesture recognizer too
        // (C#'s PropagateBindingContext(GestureRecognizers)). chrome (W1-11): the attached context
        // flyout inherits the context the same way (C# parents it via AddRemoveLogicalChildren; the
        // port's for_each_logical_child overrides don't enumerate it, so propagate here explicitly). ---
        void on_binding_context_changed() override
        {
            maui::controls::element::on_binding_context_changed();
            const auto& context = this->raw_binding_context();
            for (const auto& recognizer : gesture_recognizers_.items())
            {
                recognizer->set_inherited_binding_context(context);
            }
            if (context_flyout_element_ != nullptr)
            {
                context_flyout_element_->set_inherited_binding_context(context);
            }
            // X1 — VisualElement.OnBindingContextChanged: `if (Background != null) SetInheritedBindingContext
            // (Background, BindingContext)`. The background brush is NOT a logical child (so its Parent stays
            // null — BrushTypeConverterUnitTests.ImmutableBrushDoesntSetParent), only an inherited context.
            if (background_brush_)
            {
                background_brush_->set_inherited_binding_context(context);
            }
        }
        // --- end gestures (W1-12) ---

        // VisualElement.ZIndexProperty change → ViewHandler.MapZIndex: a z-index change re-stacks this view
        // among its siblings by asking the PARENT layout's handler to reorder it (C# walks
        // `view.Parent is ILayout` and invokes the layout handler's UpdateZIndex command with this view).
        // The logical parent (set when the layout attached this as a child) is the IElement parent here.
        void update_z_order()
        {
            if (auto* parent_layout = dynamic_cast<maui::core::i_layout*>(this->logical_parent()))
            {
                if (const auto& parent_handler = parent_layout->handler())
                {
                    parent_handler->invoke("update_z_index", static_cast<maui::core::i_view*>(this));
                }
            }
        }

        // The resource chain changed: if the LOCAL style resolves its base from a resource key, re-apply it
        // so the (newly-resolvable or changed) base style takes effect — the typed analog of C#'s
        // _basedOnResourceProperty DynamicResource on the style. A style without a base_resource_key is
        // chain-independent, so it is left untouched (no churn).
        void on_resource_chain_changed() override
        {
            if (style_ && !style_->base_resource_key().empty())
            {
                const auto resolve = make_resource_resolver();
                style_->unapply(*this, maui::core::setter_specificity::style_local, resolve);
                style_->apply(*this, maui::core::setter_specificity::style_local, resolve);
            }
        }

        // A based-on-by-key resolver bound to this element's resource chain — handed to style::apply/unapply
        // so a style's base_resource_key resolves from this element's resources (Style.GetBasedOnResource).
        [[nodiscard]] maui::controls::style::resource_resolver make_resource_resolver()
        {
            return [this](std::string_view key) -> std::shared_ptr<maui::controls::style> {
                if (const std::any* value = this->try_get_resource(key))
                {
                    if (const auto* found = std::any_cast<std::shared_ptr<maui::controls::style>>(value))
                    {
                        return *found;
                    }
                }
                return nullptr;
            };
        }

        // VisualElement.EnsurePositive: a negative request clamps to 0 (used by every IView size getter).
        [[nodiscard]] static double ensure_positive(double value)
        {
            return value < 0 ? 0.0 : value;
        }
        // IView.Width / IView.Height (is_minimum=false) and IView.MinimumWidth / IView.MinimumHeight
        // (is_minimum=true): an UNSET request → Unset(NaN); for Width/Height an explicit -1 is also Unset
        // ("size to content"); otherwise EnsurePositive(request). Mirrors VisualElement's IView mapping.
        [[nodiscard]] static double resolve_request(const maui::core::property<double>& request, bool is_minimum)
        {
            if (!request.is_set())
            {
                return maui::core::dimension::unset;
            }
            const double value = request.get();
            if (!is_minimum && value == -1.0)
            {
                return maui::core::dimension::unset;
            }
            return ensure_positive(value);
        }
        [[nodiscard]] static double resolve_minimum_request(const maui::core::property<double>& request)
        {
            return resolve_request(request, true);
        }
        // C# ViewHandlerExtensions.ResolveConstraints(measured, exact, min, max): the per-child size
        // resolution run in the leaf-control measure (above) — exact (if set) overrides measured, then max
        // caps and min (resolved to 0 when unset) floors. Kept inline here so view<> does not depend on the
        // layouts library; maui::layouts::layout_manager::resolve_size_request is the identical sibling the
        // managers use.
        [[nodiscard]] static double resolve_size_request(double measured, double exact, double min, double max)
        {
            double resolved = maui::core::dimension::is_explicit_set(exact) ? exact : measured;
            min = maui::core::dimension::resolve_minimum(min);
            if (resolved > max)
            {
                resolved = max;
            }
            if (resolved < min)
            {
                resolved = min;
            }
            return resolved;
        }

        // C# LayoutExtensions.ComputeFrame(this IView view, Rect bounds): resolve this view's final frame
        // within the parent-allotted `bounds`, honoring HorizontalLayoutAlignment / VerticalLayoutAlignment
        // and subtracting the margin. DesiredSize already INCLUDES the margin (ComputeDesiredSize adds it),
        // so the consumed extent starts from desired_size() and the frame size subtracts the margin back out;
        // a Fill view without an explicit width/height instead consumes min(bounds, maximum). The X/Y come
        // from align_in_axis (AlignHorizontal/AlignVertical). margin() is now a real bindable View.Margin
        // (measure adds it in ComputeDesiredSize), so this subtraction/offset balances it back out. Port of
        // the static ComputeFrame + AlignHorizontal + AlignVertical.
        [[nodiscard]] maui::graphics::rect compute_frame(const maui::graphics::rect& bounds) const
        {
            const maui::core::thickness view_margin = this->margin();
            const maui::graphics::size desired = this->desired_size();

            // consumedWidth: DesiredSize.Width, unless filling without an explicit width — then
            // min(bounds.Width, MaximumWidth) (MaximumWidth is +inf when the developer set no maximum).
            double consumed_width = desired.width;
            if (horizontal_layout_alignment() == maui::core::layout_alignment::fill &&
                !maui::core::dimension::is_explicit_set(this->width()))
            {
                consumed_width = std::min(bounds.width, this->maximum_width());
            }
            const double frame_width = std::max(0.0, consumed_width - view_margin.horizontal_thickness());

            double consumed_height = desired.height;
            if (vertical_layout_alignment() == maui::core::layout_alignment::fill &&
                !maui::core::dimension::is_explicit_set(this->height()))
            {
                consumed_height = std::min(bounds.height, this->maximum_height());
            }
            const double frame_height = std::max(0.0, consumed_height - view_margin.vertical_thickness());

            const double frame_x = align_horizontal(bounds, view_margin);
            const double frame_y = align_vertical(bounds, view_margin);

            return {frame_x, frame_y, frame_width, frame_height};
        }

        // C# LayoutExtensions.AlignHorizontal: the X edge for this view's frame. A Fill view with an
        // explicit width (or a finite MaximumWidth) is treated as Center over the space it "fills", with the
        // desired width clamped to min(bounds, maximum) when not explicitly set. Start = left, Center =
        // centered, End = right-aligned. desiredWidth is DesiredSize.Width (margin-inclusive), matching C#.
        [[nodiscard]] double align_horizontal(const maui::graphics::rect& bounds,
                                              const maui::core::thickness& view_margin) const
        {
            maui::core::layout_alignment alignment = horizontal_layout_alignment();
            double desired_width = this->desired_size().width;
            const double explicit_width = this->width();
            if (alignment == maui::core::layout_alignment::fill &&
                (maui::core::dimension::is_explicit_set(explicit_width) ||
                 maui::core::dimension::is_maximum_set(this->maximum_width())))
            {
                alignment = maui::core::layout_alignment::center;
                desired_width = maui::core::dimension::is_explicit_set(explicit_width)
                                    ? desired_width
                                    : std::min(bounds.width, this->maximum_width());
            }
            return align_in_axis(bounds.x, view_margin.left, bounds.width, desired_width, alignment);
        }

        // C# LayoutExtensions.AlignVertical — the Y twin of align_horizontal.
        [[nodiscard]] double align_vertical(const maui::graphics::rect& bounds,
                                            const maui::core::thickness& view_margin) const
        {
            maui::core::layout_alignment alignment = vertical_layout_alignment();
            double desired_height = this->desired_size().height;
            const double explicit_height = this->height();
            if (alignment == maui::core::layout_alignment::fill &&
                (maui::core::dimension::is_explicit_set(explicit_height) ||
                 maui::core::dimension::is_maximum_set(this->maximum_height())))
            {
                alignment = maui::core::layout_alignment::center;
                desired_height = maui::core::dimension::is_explicit_set(explicit_height)
                                     ? desired_height
                                     : std::min(bounds.height, this->maximum_height());
            }
            return align_in_axis(bounds.y, view_margin.top, bounds.height, desired_height, alignment);
        }

        // C# LayoutExtensions.AlignHorizontal(startX, startMargin, …) — the shared scalar positioner used
        // for both axes: Start keeps the leading edge (start + startMargin), Center adds half the slack,
        // End adds all the slack (bounds − desired). Fill resolves to Start here (its size already spans
        // the bounds), matching C#'s switch with no Fill case.
        [[nodiscard]] static double align_in_axis(double start, double start_margin, double bounds_length,
                                                  double desired_length, maui::core::layout_alignment alignment)
        {
            double edge = start + start_margin;
            switch (alignment)
            {
                case maui::core::layout_alignment::center:
                    edge += (bounds_length - desired_length) / 2.0;
                    break;
                case maui::core::layout_alignment::end:
                    edge += bounds_length - desired_length;
                    break;
                case maui::core::layout_alignment::fill:
                case maui::core::layout_alignment::start:
                    break;
            }
            return edge;
        }

        std::shared_ptr<maui::core::i_element_handler> handler_;
        std::weak_ptr<maui::core::i_element> parent_;
        maui::graphics::rect frame_;
        maui::graphics::size desired_size_;
        // The four generic IView properties are bindable (their change flows through
        // on_property_changed → handler->update_value → the chained view_mapper). Each references a
        // single shared descriptor (the non-template *_property() free functions above) so the
        // descriptor — and thus the property name the mapper keys on — is the same for every control.
        maui::core::property<bool> is_enabled_{*this, is_enabled_property()};
        maui::core::property<double> opacity_{*this, opacity_property()};
        maui::core::property<maui::core::visibility> visibility_{*this, visibility_property()};
        maui::core::property<std::string> automation_id_{*this, automation_id_property()};
        // The render-transform scalars + flow direction (each change re-runs the chained view_mapper's
        // map_transform / map_flow_direction). Shared NON-template descriptors, like the four above.
        maui::core::property<double> translation_x_{*this, translation_x_property()};
        maui::core::property<double> translation_y_{*this, translation_y_property()};
        maui::core::property<double> scale_{*this, scale_property()};
        maui::core::property<double> scale_x_{*this, scale_x_property()};
        maui::core::property<double> scale_y_{*this, scale_y_property()};
        maui::core::property<double> rotation_{*this, rotation_property()};
        maui::core::property<double> rotation_x_{*this, rotation_x_property()};
        maui::core::property<double> rotation_y_{*this, rotation_y_property()};
        maui::core::property<double> anchor_x_{*this, anchor_x_property()};
        maui::core::property<double> anchor_y_{*this, anchor_y_property()};
        maui::core::property<maui::core::flow_direction> flow_direction_{*this, flow_direction_property()};
        // The per-axis layout alignment (View.HorizontalOptions / VerticalOptions, default fill). Shared
        // NON-template descriptors; consumed at arrange time by compute_frame (no native mapper).
        maui::core::property<maui::core::layout_alignment> horizontal_layout_alignment_{
            *this, horizontal_layout_alignment_property()};
        maui::core::property<maui::core::layout_alignment> vertical_layout_alignment_{
            *this, vertical_layout_alignment_property()};
        // The visual-layer properties (Background / Shadow / Clip). The control owns each object; a set
        // re-runs the chained view_mapper's map_background / map_shadow / map_clip. Shared descriptors.
        maui::core::property<std::shared_ptr<maui::graphics::paint>> background_{*this, background_property()};
        maui::core::property<std::shared_ptr<maui::core::i_shadow>> shadow_{*this, shadow_property()};
        maui::core::property<std::shared_ptr<maui::graphics::i_shape>> clip_{*this, clip_property()};
        // X1 — the developer-facing Background brush (when set via the brush overload). Owned by the view so
        // it inherits the view's BindingContext; the bridged paint lives in background_ above for the handler.
        std::shared_ptr<brush> background_brush_;
        // W7 — a gradient brush mutates AFTER assignment (the XAML loader adds GradientStops after setting
        // Background; runtime code can too), so re-derive the bridged paint on its invalidate event. Declared
        // AFTER background_brush_ so it is destroyed FIRST — the disconnect runs while the brush (the event
        // publisher) is still alive.
        maui::core::scoped_connection background_brush_subscription_;
        // Accessibility metadata + the input-transparent flag (each re-runs the chained view_mapper's
        // map_semantics / map_input_transparent). Shared descriptors, like the visual-layer props.
        maui::core::property<std::shared_ptr<maui::core::semantics>> semantics_{*this, semantics_property()};
        maui::core::property<bool> input_transparent_{*this, input_transparent_property()};
        // The six size requests (VisualElement.WidthRequest / HeightRequest / Minimum* / Maximum*). Each
        // change re-runs the matching mapper (its key is the IView name) and feeds i_view::width()/...,
        // which the layout managers read; the arranged frame stays in frame_. Shared descriptors.
        maui::core::property<double> width_request_{*this, width_request_property()};
        maui::core::property<double> height_request_{*this, height_request_property()};
        maui::core::property<double> minimum_width_request_{*this, minimum_width_request_property()};
        maui::core::property<double> minimum_height_request_{*this, minimum_height_request_property()};
        maui::core::property<double> maximum_width_request_{*this, maximum_width_request_property()};
        maui::core::property<double> maximum_height_request_{*this, maximum_height_request_property()};
        // The z-order within the parent layout (VisualElement.ZIndex). A change routes to the parent
        // layout's handler so the native panel re-stacks this child (see on_property_changed).
        maui::core::property<int> z_index_{*this, z_index_property()};
        // View.Margin (the space reserved around this view; default zero). Layout-only — no native mapper;
        // consumed by measure (ComputeDesiredSize) + compute_frame (ComputeFrame). A change invalidates
        // measure so the parent re-lays-out (see on_property_changed). Shared NON-template descriptor.
        maui::core::property<maui::core::thickness> margin_{*this, margin_property()};
        // chrome (W1-11): the attached ToolTip text (bindable; is_set() distinguishes "never set") and
        // the attached ContextFlyout (non-owning; the element face is cached for tree/context plumbing).
        maui::core::property<std::string> tool_tip_text_{*this, tool_tip_text_property()};
        maui::core::i_flyout* context_flyout_ = nullptr;
        element* context_flyout_element_ = nullptr;
        bool is_focused_ = false;
        // --- gestures (W1-12) ---------------------------------------------------------------------------
        // Declaration order matters: the manager precedes the collection (the collection's hooks
        // reference it), so on destruction the collection goes first while the manager still holds its
        // own strong refs to the attached recognizers — the native detach in ~gesture_platform_manager
        // never touches a freed recognizer. The hooks are the C# View ctor's CollectionChanged handler:
        // attach/detach = the item.Parent writes (+ context/window inheritance), changed = the
        // GesturePlatformManager.LoadRecognizers re-sync.
        gesture_platform_manager gesture_manager_;
        gesture_recognizer_collection gesture_recognizers_{
            {.attach = [this](gesture_recognizer& recognizer) { this->attach_logical_child(recognizer); },
             .detach = [](gesture_recognizer& recognizer) { element::detach_logical_child(recognizer); },
             .changed = [this] { gesture_manager_.load_recognizers(); }}};
        // --- end gestures (W1-12) -----------------------------------------------------------------------
        // The applied style (VisualElement.Style). Held by shared_ptr so one style can be shared across
        // many controls; setting/replacing it routes through set_style (apply/unapply at style_local).
        std::shared_ptr<maui::controls::style> style_;
        std::vector<std::string> style_class_;               // the selected style classes (VisualElement.StyleClass)
        maui::controls::visual_state_manager visual_states_; // VisualStateManager.VisualStateGroups host

        // --- styles tail (W1-15) ------------------------------------------------------------------------
    public:
        // VisualStateManager.SetVisualStateGroups(element, groups): store `groups` as THIS control's
        // visual-state groups. The previous groups' current states are un-applied first (the
        // VisualStateGroupsPropertyChanged old-value branch), the new groups' state triggers are wired
        // (attaching/detaching with this element's loaded/unloaded — InvalidateStateTriggers), then
        // ChangeVisualState runs and the triggers are evaluated — so a Normal state and any
        // already-active trigger apply immediately. The member manager is mutated IN PLACE (its address
        // anchors the trigger hooks); visual_states() keeps exposing it.
        void set_visual_state_groups(maui::controls::visual_state_manager groups)
        {
            visual_states_.replace_from(std::move(groups), *this, [this] { this->change_visual_state(); });
        }

        // VisualElement.Behaviors: the behavior collection of this control, pre-attached to it (the
        // BehaviorsPropertyKey defaultValueCreator) — adding a behavior runs its OnAttachedTo here.
        [[nodiscard]] behavior_collection& behaviors()
        {
            return behaviors_;
        }
        [[nodiscard]] const behavior_collection& behaviors() const
        {
            return behaviors_;
        }

        // VisualElement.Triggers: the trigger collection of this control, pre-attached to it (the
        // TriggersPropertyKey defaultValueCreator) — adding a trigger attaches it to this view now.
        [[nodiscard]] triggers_collection& triggers()
        {
            return triggers_;
        }
        [[nodiscard]] const triggers_collection& triggers() const
        {
            return triggers_;
        }
        // The non-template reach for the XAML loader (element::triggers_or_null): a view HAS a Triggers
        // collection, so hand back the real one.
        [[nodiscard]] triggers_collection* triggers_or_null() override
        {
            return &triggers_;
        }

    private:
        behavior_collection behaviors_{*this};
        triggers_collection triggers_{*this};
        // --- end styles tail (W1-15) --------------------------------------------------------------------
    };
} // namespace maui::controls
