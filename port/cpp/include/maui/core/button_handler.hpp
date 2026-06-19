#pragma once
// maui::core::button_handler  <=  Microsoft.Maui.Handlers.ButtonHandler
//
// The first concrete handler — the Rosetta Stone (PROJECT.md M2). It maps the cross-platform i_button
// virtual view to a native button: Text flows virtual→native through the property mapper, and a native
// tap flows native→virtual by calling i_button::send_clicked() (which the control turns into its
// `clicked` event). Ported from ButtonHandler.cs (cross-platform) + ButtonHandler.iOS.cs (the platform
// recipe, translated to each backend).
//
// Partial-class split (PROFILE §5): the mapper TABLES and ctor are cross-platform (button_handler.cpp);
// the platform recipe — create_platform_view / connect / disconnect / map_text / measure — is defined
// per backend under src/platform/<backend>/button_handler.{cpp,mm}. Only one backend is linked.
//
// button_platform is the managed platform view. It is a single cross-platform struct (so the CRTP
// Platform type stays complete everywhere — no incomplete-type pimpl dance): the `native` slot holds
// the real backend view (an NSButton* on Apple / a UIButton* on iOS, retained in the .mm; unused
// headless), `title` is the headless text mirror, and the callbacks are the inbound event hooks the
// platform partial wires up.

#include <memory>
#include <string>
#include <string_view>

#include "maui/core/command_mapper.hpp"
#include "maui/core/i_button.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/i_text_button.hpp"
#include "maui/core/image_source_loader.hpp"
#include "maui/core/image_source_result.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/property_mapper.hpp"
#include "maui/core/thickness.hpp"
#include "maui/core/view_handler.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::core
{
    // Derives view_platform_base so the shared view_mapper can push the generic IView properties
    // (Visibility/Opacity/IsEnabled/AutomationId) onto it: the headless build keeps the base's mirrors,
    // the Apple build overrides update_* to push to the NSButton.
    struct button_platform : view_platform_base
    {
        button_platform() = default;
        // Destruction releases `native` and is therefore backend-defined (the headless build defaults
        // it; the Apple/iOS builds CFRelease the retained NSButton/UIButton). Non-copyable/non-movable:
        // it is owned solely by the handler's unique_ptr and never copied or moved.
        ~button_platform() override;
        button_platform(const button_platform&) = delete;
        button_platform(button_platform&&) = delete;
        button_platform& operator=(const button_platform&) = delete;
        button_platform& operator=(button_platform&&) = delete;

        void* native = nullptr;
        // Headless mirror of every mapped property (the Apple build pushes to `native` instead; these
        // let the headless tests observe that each mapper ran with the right value). The generic IView
        // mirrors (hidden/alpha/enabled/automation_id) come from view_platform_base.
        std::string title;
        maui::graphics::color text_color;
        font text_font;
        double character_spacing = 0;
        thickness padding;
        maui::graphics::color stroke_color;
        double stroke_thickness = 0;
        int corner_radius = 0;
        // Image-source mirrors (the image_platform convention copied here so the headless tests observe a
        // load): kind/file/loaded after map_image_source pushes the source. content_layout_push_count
        // counts map_content_layout invocations — the port stores + pushes ContentLayout but defers the
        // text+image composition (no container infra), so the count is all a test can observe.
        std::string source_kind;
        std::string source_file;
        bool source_loaded = false;
        int content_layout_push_count = 0;
        move_only_function<void()> on_click;
        move_only_function<void()> on_press;
        move_only_function<void()> on_release;

#ifdef MAUI_PLATFORM_APPLE
        // Apple backend: push the generic IView properties to the NSButton (defined in
        // src/platform/apple/button_handler.mm). The headless build omits these and keeps the
        // view_platform_base mirrors — the class layout is identical (same four virtual slots), and a
        // given build only ever sees one backend's definition, so there is no ODR mismatch.
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_shadow(const maui::core::i_shadow* value) override;
        void update_clip(const maui::graphics::i_shape* value) override;
        // Accessibility metadata + the input-transparent flag pushed to the NSButton (M5d native a11y /
        // hit-test): semantics → accessibilityLabel/Help/heading role, input_transparent → -hitTest: gate.
        void update_semantics(const maui::core::semantics* value) override;
        void update_input_transparent(bool value) override;
#endif

#ifdef MAUI_PLATFORM_IOS
        // iOS backend (M6 scaffold): push the four fundamental IView properties to the UIButton
        // (defined in src/platform/ios/button_handler.mm). The remaining generic-IView pushes —
        // transform / flow_direction / shadow / clip / semantics / input_transparent — deliberately keep
        // the view_platform_base mirrors for now; the M6 fan-out units port the shared ios view/visual/
        // semantics op helpers and override them here (see port/STATUS.md). background IS overridden: a
        // UIButton's BackgroundColor must be drawn as a per-state backgroundImage (ButtonHandler.MapBackground).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
#endif

#ifdef MAUI_PLATFORM_ANDROID
        // Android backend (M-android fan-out, unit 28): push the four fundamental IView properties
        // plus Background to the real android.widget.Button over JNI (defined in
        // src/platform/android/button_handler.cpp). Background joins the four here because the
        // android button's stroke/corner-radius land in the same GradientDrawable the background
        // color fills (ButtonExtensions.UpdateButtonBackground). Each override calls the
        // view_platform_base body FIRST — the android preset also runs the pure-native
        // cross-platform suite on the emulator WITHOUT a Java VM, and that suite observes the
        // headless mirrors — then pushes to the widget when one exists. The render transform, flow
        // direction, and semantics push through the shared android view/semantics ops (W4-34e). Shadow,
        // Clip, and InputTransparent keep ONLY the base mirror: on Android those are WrapperView-only
        // (no plain-android.view.View analog), so an unwrapped View receives no update in C# either
        // (see src/platform/android/android_visual_ops.hpp / android_semantics_ops.hpp deviations).
        void update_visibility(maui::core::visibility value) override;
        void update_opacity(double value) override;
        void update_is_enabled(bool value) override;
        void update_automation_id(std::string_view value) override;
        void update_background(const maui::graphics::paint* value) override;
        void update_transform(const maui::core::transform_spec& value) override;
        void update_flow_direction(maui::core::flow_direction value) override;
        void update_semantics(const maui::core::semantics* value) override;
#endif
    };

    class button_handler : public view_handler<button_handler, i_button, button_platform>
    {
    public:
        button_handler();

        // Shared mapper tables (cross-platform — defined in button_handler.cpp). `mapper` chains the
        // text mapper + the image mapper, mirroring ButtonHandler.Mapper chaining TextButtonMapper +
        // ImageButtonMapper.
        static property_mapper<i_button, button_handler>& mapper();
        static property_mapper<i_text_button, button_handler>& text_mapper();
        // C# ButtonHandler.ImageButtonMapper — PropertyMapper<IImage, IButtonHandler> keyed on the image
        // source ([Source] → MapImageSource) plus the controls-side ContentLayout remap (Button.Mapper.cs's
        // MapContentLayout). Keyed on i_text_button (Button's virtual view, which carries image_source()):
        // the diamond-free narrow seam (Button is not an i_image — see i_text_button.hpp).
        static property_mapper<i_text_button, button_handler>& image_mapper();
        static command_mapper<i_button, button_handler>& command_mapper();

        // Platform recipe (defined per backend: src/platform/<backend>/button_handler.{cpp,mm}).
        // create + disconnect need no handler state (static); connect captures `this` to route the
        // native control's events back to the virtual view.
        static std::unique_ptr<button_platform> create_platform_view();
        void on_connect_handler(button_platform& platform);
        static void on_disconnect_handler(button_platform& platform);

        // i_view_handler measure/arrange seam (platform-specific sizing).
        [[nodiscard]] maui::graphics::size get_desired_size(double width_constraint,
                                                            double height_constraint) const override;
        void platform_arrange(const maui::graphics::rect& frame) override;

        // Property map functions (platform recipe), each pushing one virtual-view property onto the
        // platform view. Text/appearance are keyed by i_text_button (C#'s TextButtonMapper<ITextButton>);
        // padding + stroke are keyed by i_button (the button's own mapper).
        static void map_text(button_handler& handler, i_text_button& view);
        static void map_text_color(button_handler& handler, i_text_button& view);
        static void map_font(button_handler& handler, i_text_button& view);
        static void map_character_spacing(button_handler& handler, i_text_button& view);
        static void map_padding(button_handler& handler, i_button& view);
        static void map_stroke_color(button_handler& handler, i_button& view);
        static void map_stroke_thickness(button_handler& handler, i_button& view);
        static void map_corner_radius(button_handler& handler, i_button& view);

        // Cross-platform image-source routing (ButtonHandler.MapImageSource → MapImageSourceAsync →
        // ImageSourceLoader.UpdateImageSourceAsync). Defined in button_handler.cpp: the file fast-path is
        // synchronous; every other source goes through the handler-owned loader (async, with the
        // source-identity recheck); a null/empty source cancels + clears — image_handler::map_source's twin.
        static void map_image_source(button_handler& handler, i_text_button& view);
        // ContentLayout (Button.Mapper.cs MapContentLayout → UpdateContentLayout): stored + pushed only —
        // the text+image composition is deferred (no container infra). Records a push on the platform.
        static void map_content_layout(button_handler& handler, i_text_button& view);

        // The handler-owned async image-source loader (C#'s ButtonHandler.ImageSourceLoader /
        // ImageSourcePartLoader). Per-backend wiring happens in the platform partial's configure_loader.
        [[nodiscard]] image_source_loader& image_source_loader_ref()
        {
            return image_source_loader_;
        }

    private:
        // Per-backend source primitives (the image_handler convention; defined in the platform partial).
        // The file fast-path loads synchronously; apply_loaded_result copies a delivered async result onto
        // the platform; clear_source_native removes the image; configure_loader wires per-backend seams.
        static void load_file_source_sync(button_platform& platform, const i_file_image_source& file_src);
        static void apply_loaded_result(button_platform& platform, const image_source_result& result);
        static void clear_source_native(button_platform& platform);
        static void configure_loader(maui::core::image_source_loader& loader);

        maui::core::image_source_loader image_source_loader_;
    };
} // namespace maui::core
