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

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/gestures/gesture_platform_manager.hpp" // --- gestures (W1-12) ---
#include "maui/controls/style.hpp"
#include "maui/controls/visual_state_manager.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/flow_direction.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_layout.hpp"
#include "maui/core/i_shadow.hpp"
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

    template <class ViewInterface> class view : public maui::controls::element, public ViewInterface
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
        [[nodiscard]] maui::core::layout_alignment horizontal_layout_alignment() const override
        {
            return maui::core::layout_alignment::fill;
        }
        [[nodiscard]] maui::core::layout_alignment vertical_layout_alignment() const override
        {
            return maui::core::layout_alignment::fill;
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
            background_.set(std::move(value));
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
        [[nodiscard]] bool is_enabled() const override
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
        void set_is_focused(bool value) override
        {
            is_focused_ = value;
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
        [[nodiscard]] maui::core::thickness margin() const override
        {
            return {};
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
        // The measure/arrange seam delegates to the view handler (C# IViewHandler.GetDesiredSize /
        // PlatformArrange), the cross-platform layout calling into the platform view.
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override
        {
            frame_ = bounds;
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler_.get()))
            {
                view_handler->platform_arrange(bounds);
            }
            return {bounds.width, bounds.height};
        }
        // The leaf-control measure seam (C# View.MeasureOverride → the handler's GetDesiredSizeFromHandler):
        // ask the handler for the content size, then resolve it against this view's OWN size requests so the
        // reported desired size honors WidthRequest/Minimum*/Maximum* — the per-child clamp the layout
        // managers rely on (ViewHandlerExtensions.ResolveConstraints). With no handler the desired size is
        // zero, still resolved against the requests so an explicit Width/Height request is reported.
        maui::graphics::size measure(double width_constraint, double height_constraint) override
        {
            maui::graphics::size content{};
            if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler_.get()))
            {
                content = view_handler->get_desired_size(width_constraint, height_constraint);
            }
            desired_size_ = {resolve_size_request(content.width, width(), minimum_width(), maximum_width()),
                             resolve_size_request(content.height, height(), minimum_height(), maximum_height())};
            return desired_size_;
        }
        void invalidate_measure() override
        {
            // Layout invalidation is wired in M3 (the layout pass); no-op for the M2 seam.
        }
        void invalidate_arrange() override
        {
        }
        bool focus() override
        {
            is_focused_ = true;
            change_visual_state(); // VisualElement.Focus → ChangeVisualState (Focused)
            return true;
        }
        void unfocus() override
        {
            is_focused_ = false;
            change_visual_state();
        }

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
        void change_visual_state()
        {
            using common = maui::controls::common_states;
            const std::string_view target =
                !is_enabled_.get() ? common::disabled : (is_focused_ ? common::focused : common::normal);
            visual_states_.go_to_state(*this, target);
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

    protected:
        view() = default;

        // The virtual→native seam: any bindable property change notifies the handler, which re-runs
        // that property's mapper. Mirrors MAUI, where a BindableProperty change calls Handler.UpdateValue.
        // An IsEnabled change additionally drives the visual state (VisualElement.OnIsEnabledPropertyChanged
        // → ChangeVisualState), so a configured Disabled/Normal state applies automatically.
        void on_property_changed(std::string_view name) override
        {
            maui::core::bindable_object::on_property_changed(name);
            if (handler_)
            {
                handler_->update_value(name);
            }
            if (name == "is_enabled")
            {
                change_visual_state();
            }
            if (name == "z_index")
            {
                update_z_order();
            }
        }

        // --- gestures (W1-12): View.OnBindingContextChanged — after the base propagation to the
        // logical children, hand the (possibly inherited) context to every gesture recognizer too
        // (C#'s PropagateBindingContext(GestureRecognizers)) ---
        void on_binding_context_changed() override
        {
            maui::controls::element::on_binding_context_changed();
            const auto& context = this->raw_binding_context();
            for (const auto& recognizer : gesture_recognizers_.items())
            {
                recognizer->set_inherited_binding_context(context);
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
        // The visual-layer properties (Background / Shadow / Clip). The control owns each object; a set
        // re-runs the chained view_mapper's map_background / map_shadow / map_clip. Shared descriptors.
        maui::core::property<std::shared_ptr<maui::graphics::paint>> background_{*this, background_property()};
        maui::core::property<std::shared_ptr<maui::core::i_shadow>> shadow_{*this, shadow_property()};
        maui::core::property<std::shared_ptr<maui::graphics::i_shape>> clip_{*this, clip_property()};
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
    };
} // namespace maui::controls
