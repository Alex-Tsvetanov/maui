// shape_view_handler — Android (JNI) platform recipe, the box_view slice of the M-android per-control
// fan-out. box_view is NOT a handler of its own: BoxView is its own IShapeView AND its own IShape and
// renders through the shared shape_view_handler (box_view.cpp self-registers
// MAUI_REGISTER_HANDLER(box_view, shape_view_handler); shape_view_handler.cpp/.hpp hold the
// cross-platform mapper + the shape_view_platform struct). This file is the Android half of that
// platform recipe — the JNI twin of src/platform/apple/shape_view_handler.mm and the
// src/platform/headless/shape_view_handler.cpp mirror.
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// THE C# ORACLE AND WHY THIS PARTIAL DEVIATES (documented, not stubbed)
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// In .NET MAUI, Android BoxView is Microsoft.Maui.Platform.MauiBoxView : PlatformGraphicsView
// (src/Core/src/Platform/Android/MauiBoxView.cs) — a CANVAS-DRAWING view, the exact twin of iOS's
// MauiShapeView : PlatformGraphicsView. The box is painted by ShapeDrawable.Draw(ICanvas) inside the
// view's onDraw — there is NO android.graphics.drawable in MAUI's BoxView render path; the rounded
// rectangle is stroked/filled through the Microsoft.Maui.Graphics canvas stack. The port's apple/iOS
// twins reproduce this faithfully: create_drawable_host() makes an NSView/UIView whose drawRect calls
// shape_view_platform::replay → drawable.draw(canvas, dirty_rect).
//
// The fidelity-perfect Android port would therefore be a custom Java View subclass overriding
// onDraw(android.graphics.Canvas), backed by an i_canvas → android.graphics.Canvas bridge so the
// SAME shape_drawable.draw() renders the box. That bridge DOES NOT EXIST in the Android backend yet
// (there is no Skia/i_canvas Android implementation, and the only custom-View Java class shipped is
// MauiLayout, a no-op ViewGroup). Building the PlatformGraphicsView twin would require a new Java
// class + a native canvas bridge + a CMake/Java fan-out — out of scope for this two-file slice.
//
// So this partial uses the SAME deviation the android button partial established
// (src/platform/android/button_handler.cpp) and the shared android background op documents
// (android_visual_ops.hpp): ONE android.graphics.drawable.GradientDrawable installed as a plain
// android.view.View's background, with the box Color → GradientDrawable.setColor(argb) and the box
// CornerRadius → setCornerRadius / setCornerRadii. For BoxView's geometry — a solid-filled rounded
// rectangle with no stroke (i_stroke is all empty/zero in box_view.cpp) — GradientDrawable is a
// FAITHFUL expression, not a lossy stand-in: a uniform corner radius maps to setCornerRadius(px), the
// four independent radii map to setCornerRadii(float[8]) (BoxView's per-corner support, which the
// button's single CornerRadius could not express). It is lossy only for the shape family at large
// (arbitrary paths, strokes, gradient fills via Fill) — none of which a BoxView carries. The plain
// View ctor `new android.view.View(Context)` is theme-independent (no defStyleAttr resolution), so it
// constructs in the bare app_process testhost (LESSON 2 in docs/MACOS_ANDROID_RESUME.md — unlike the
// horizontal-ProgressBar / SeekBar / EditText ctors that resolve a theme style attr).
//
// THE HEADLESS MIRROR STAYS LIVE: every shared write the headless/apple twins make
// (drawable.update_shape_view, refresh_drawable_state's winding/render-transform pushes, the
// invalidations counter) is preserved here, so the android preset's PURE-NATIVE cross-platform suite
// (tools/android-emu-run.sh, no JavaVM) observes exactly the headless partial's behavior. The JNI
// GradientDrawable push is layered ON TOP, behind scoped_env/app_context guards, and runs only when a
// VM + a native View exist (the widget testhost + the app host).
//
// Maps actually pushed to the View (the others — Aspect/Stroke*/DashPattern/LineCap/LineJoin/
// MiterLimit/FlowDirection — funnel into invalidate_shape, which keeps the drawable mirror current but
// has no GradientDrawable analog for a strokeless solid box, so they no-op on the native side, exactly
// as they would draw nothing extra on a BoxView):
//   - i_shape_view::fill() solid color → GradientDrawable.setColor(argb)   (BoxView.Color → Fill)
//   - box_view CornerRadius (read off the i_shape's PathForBounds geometry) → setCornerRadius[i]
// CornerRadius does not reach the handler through i_shape_view (the union mapper carries a
// "corner_radius" KEY that funnels to invalidate_shape, but the VALUE lives on box_view, reached via
// the i_shape face). The push therefore reads the radii by asking the shape for its bounds path corner
// metrics through the i_shape_view's shape() → see corner_radii_of() below.

#include "maui/core/shape_view_handler.hpp"

#include <jni.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <string_view>

#include "android_semantics_ops.hpp"
#include "android_view_ops.hpp"
#include "android_visual_ops.hpp"
#include "jni/app_context.hpp"
#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/i_shape_view.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace
{
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    // The plain android.view.View hosting the box (the GradientDrawable stand-in for MauiBoxView's
    // PlatformGraphicsView — see the header deviations). GetMethodID walks superclasses, so the View
    // surface resolves through this class directly.
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_gradient_drawable_class = "android/graphics/drawable/GradientDrawable";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.MeasureSpec modes (ViewHandlerExtensions.GetDesiredSizeFromHandler /
    // PlatformArrange) — shared with the button partial.
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject view_of(const maui::core::shape_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the partial must never leak JNI pending-exception state into
    // the cross-platform layer); true when one was pending — call sites skip the read-back. Mirrors the
    // button partial's clear_pending.
    bool clear_pending(JNIEnv* env)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe(); // logcat/stderr breadcrumb, same channel the test host uses
        env->ExceptionClear();
        return true;
    }

    void call_void_int(JNIEnv* env, jobject view, const char* name, jint value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(view, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject view, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(F)V"))
        {
            env->CallVoidMethod(view, method, value);
            clear_pending(env);
        }
    }

    void call_void_bool(JNIEnv* env, jobject view, const char* name, jboolean value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_view_class, name, "(Z)V"))
        {
            env->CallVoidMethod(view, method, value);
            clear_pending(env);
        }
    }

    // ContextExtensions.ToPixels: ceil(dp * density - Epsilon), then C#'s (int) truncation at the call
    // sites — the ceil already produced an integral value, so truncation is exact (button partial).
    [[nodiscard]] jint to_pixels(double dp, float density)
    {
        return static_cast<jint>(std::ceil((dp * static_cast<double>(density)) - k_to_pixels_epsilon));
    }

    // The widget's display density (Context.getResources().getDisplayMetrics().density). Memoized
    // process-wide after the first successful read — exactly the button partial's display_density, kept
    // standalone so the two partials stay independently buildable. 1.0 when any step fails (failures
    // are not memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject view)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_view_class, "getContext", "()Landroid/content/Context;");
        jmethodID get_resources =
            cache.method(env, "android/content/Context", "getResources", "()Landroid/content/res/Resources;");
        jmethodID get_display_metrics =
            cache.method(env, "android/content/res/Resources", "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");
        jfieldID density_field = cache.field(env, "android/util/DisplayMetrics", "density", "F");
        if (get_context == nullptr || get_resources == nullptr || get_display_metrics == nullptr ||
            density_field == nullptr)
        {
            return 1.0F;
        }
        const local_ref<jobject> context{env, env->CallObjectMethod(view, get_context)};
        if (clear_pending(env) || !context)
        {
            return 1.0F;
        }
        const local_ref<jobject> resources{env, env->CallObjectMethod(context.get(), get_resources)};
        if (clear_pending(env) || !resources)
        {
            return 1.0F;
        }
        const local_ref<jobject> metrics{env, env->CallObjectMethod(resources.get(), get_display_metrics)};
        if (clear_pending(env) || !metrics)
        {
            return 1.0F;
        }
        const jfloat density = env->GetFloatField(metrics.get(), density_field);
        if (clear_pending(env) || density == 0.0F)
        {
            return 1.0F;
        }
        memoized.store(density, std::memory_order_relaxed);
        return density;
    }

    // The maui-managed GradientDrawable carrying the box fill color + corner radius (the plain-View
    // stand-in for MauiBoxView's canvas paint — see the header deviations). Returns the installed one
    // (View.getBackground() instanceof GradientDrawable identifies ours), installing a fresh one only
    // when `install` is set. An empty ref means "not installed and not asked to install" (or a JNI
    // failure). The button partial's maui_background_drawable, kept standalone here.
    [[nodiscard]] local_ref<jobject> maui_box_drawable(JNIEnv* env, jobject view, bool install)
    {
        auto& cache = default_jni_cache();
        jclass gradient_class = cache.find_class(env, k_gradient_drawable_class);
        jmethodID get_background =
            cache.method(env, k_view_class, "getBackground", "()Landroid/graphics/drawable/Drawable;");
        if (gradient_class == nullptr || get_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> current{env, env->CallObjectMethod(view, get_background)};
        if (clear_pending(env))
        {
            return {};
        }
        if (current && env->IsInstanceOf(current.get(), gradient_class) == JNI_TRUE)
        {
            return current;
        }
        if (!install)
        {
            return {};
        }
        jmethodID ctor = cache.method(env, k_gradient_drawable_class, "<init>", "()V");
        jmethodID set_background =
            cache.method(env, k_view_class, "setBackground", "(Landroid/graphics/drawable/Drawable;)V");
        if (ctor == nullptr || set_background == nullptr)
        {
            return {};
        }
        local_ref<jobject> fresh{env, env->NewObject(gradient_class, ctor)};
        if (clear_pending(env) || !fresh)
        {
            return {};
        }
        env->CallVoidMethod(view, set_background, fresh.get());
        if (clear_pending(env))
        {
            return {};
        }
        return fresh;
    }

    // The box's corner radius (dp), recovered off the i_shape bounds path. CornerRadius does NOT reach
    // this seam as a value: the handler only sees i_shape_view, whose shape() is box_view's own i_shape
    // face, and the union mapper's "corner_radius" KEY funnels to invalidate_shape with no value. So the
    // radius is read back out of the geometry box_view.cpp builds: PathForBounds lays
    // append_rounded_rectangle(x,y,w,h, tl,tr,bl,br) whose VERY FIRST point is move_to(x, y + tl) — i.e.
    // the top-left radius is exactly first_point().y - bounds.y. This is exact for the uniform
    // CornerRadius BoxView almost always carries (one value → all four corners equal). Per-corner radii
    // are not generically recoverable from later path points without index assumptions against path_f's
    // representation, so the push is uniform: faithful for uniform (the common case), and a conservative
    // top-left approximation for the rare four-distinct case (still a rounded box, just symmetric).
    // Returns {0} when the shape is absent, square-cornered, or its geometry cannot be inspected — in
    // which case the box still fills correctly (the color push is independent).
    [[nodiscard]] maui::graphics::corner_radius corner_radii_of(const maui::core::i_shape_view& view)
    {
        const maui::graphics::i_shape* shape = view.shape();
        if (shape == nullptr)
        {
            return {};
        }
        constexpr float k_ref = 100.0F; // a reference square; the recovered radius is bounds-independent
        const maui::graphics::path_f path = shape->path_for_bounds(maui::graphics::rect{0, 0, k_ref, k_ref});
        if (path.count() == 0)
        {
            return {};
        }
        const maui::graphics::rect_f bounds = path.bounds();
        if (bounds.width <= 0 || bounds.height <= 0)
        {
            return {}; // a collapsed path → no rounding
        }
        // append_rounded_rectangle's first emitted point is (min_x, min_y + tl); recover tl as the
        // vertical inset of that first point from the path's top edge.
        const maui::graphics::point_f first = path.first_point();
        const double tl = static_cast<double>(first.y - bounds.y);
        if (!(tl > 0))
        {
            return {}; // square corners (a tl of 0 leaves the box sharp)
        }
        return maui::graphics::corner_radius(tl);
    }

    // Push the box's uniform corner radius (dp) onto the GradientDrawable via setCornerRadius(px).
    // corner_radii_of recovers a single uniform radius (the BoxView CornerRadius case — see its note),
    // so the GradientDrawable's uniform setter is the exact expression; a 0 radius keeps the box sharp.
    void set_box_corner(JNIEnv* env, jobject drawable, const maui::graphics::corner_radius& radius, float density)
    {
        jmethodID set_corner_radius =
            default_jni_cache().method(env, k_gradient_drawable_class, "setCornerRadius", "(F)V");
        if (set_corner_radius != nullptr)
        {
            env->CallVoidMethod(drawable, set_corner_radius, static_cast<jfloat>(to_pixels(radius.top_left, density)));
            clear_pending(env);
        }
    }

    // The native side of update_shape / invalidate_shape: push the box's solid fill color + corner
    // radius onto the maui GradientDrawable. Installs the drawable only when the box actually carries a
    // visible fill (an unset BoxView.Color = null Fill leaves the View's default background, mirroring
    // the button partial's lazy install and the headless "host stays transparent when Fill is null").
    void push_box_to_view(maui::core::shape_view_platform& platform, const maui::core::i_shape_view& view)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject view_obj = view_of(platform);
        // BoxView's interior paint: i_shape_view::fill() is Color?.AsPaint() — null while Color is
        // unset. The shape family's stroke is all empty/zero for a BoxView, so the fill IS the box.
        const auto* fill = dynamic_cast<const maui::graphics::solid_paint*>(view.fill());
        const bool has_fill = fill != nullptr;
        const local_ref<jobject> drawable = maui_box_drawable(env.get(), view_obj, /*install=*/has_fill);
        if (!drawable)
        {
            return; // unset color and no drawable installed → keep the default background (no guess)
        }
        auto& cache = default_jni_cache();
        if (has_fill)
        {
            jmethodID set_color = cache.method(env.get(), k_gradient_drawable_class, "setColor", "(I)V");
            if (set_color != nullptr)
            {
                env->CallVoidMethod(drawable.get(), set_color, static_cast<jint>(fill->color().to_int()));
                clear_pending(env.get());
            }
        }
        const float density = display_density(env.get(), view_obj);
        set_box_corner(env.get(), drawable.get(), corner_radii_of(view), density);
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the android.view.View (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSView host here).
    shape_view_platform::~shape_view_platform()
    {
        if (native != nullptr)
        {
            const scoped_env env; // any-thread teardown, exactly like global_ref::reset
            if (env)
            {
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            native = nullptr;
        }
    }

    // The portable replay seat (the headless twin; on Apple/iOS the real draw runs through drawRect, on
    // Android it would run through onDraw once the canvas bridge exists — header deviations). Kept live
    // so any host that does have a recording canvas observes the same shape ops.
    void shape_view_platform::replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        drawable.draw(canvas, dirty_rect);
    }

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Each calls the base body FIRST — the headless mirrors must stay live for the
    // VM-less cross-platform suite — then pushes to the real View when one exists. Mirrors the apple
    // twin's override set + the button partial's structure. ----

    void shape_view_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        // ViewExtensions.ToPlatformVisibility: Visible/Hidden/Collapsed → View.VISIBLE/INVISIBLE/GONE.
        jint state = k_view_visible;
        if (value == maui::core::visibility::hidden)
        {
            state = k_view_invisible;
        }
        else if (value == maui::core::visibility::collapsed)
        {
            state = k_view_gone;
        }
        call_void_int(env.get(), view_of(*this), "setVisibility", state);
    }

    void shape_view_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (env)
        {
            // ViewExtensions.UpdateOpacity: platformView.Alpha = (float)opacity.
            call_void_float(env.get(), view_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void shape_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId's IsNullOrWhiteSpace gate (a blank id is never pushed).
        if (native == nullptr || value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject view_obj = view_of(*this);
        auto& cache = default_jni_cache();
        jmethodID set_description =
            cache.method(env.get(), k_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
        if (set_description == nullptr)
        {
            return;
        }
        const local_ref<jstring> description = maui::platform::android::to_jstring(env.get(), value);
        env->CallVoidMethod(view_obj, set_description, description.get());
        clear_pending(env.get());
    }

    void shape_view_platform::update_transform(const maui::core::transform_spec& value)
    {
        view_platform_base::update_transform(value);
        maui::platform::android::apply_transform(native, value);
    }

    void shape_view_platform::update_flow_direction(maui::core::flow_direction value)
    {
        view_platform_base::update_flow_direction(value);
        maui::platform::android::apply_flow_direction(native, value);
    }

    // C# ShapeViewHandler.MapBackground only pushes the platform background to a CONTAINER when BOTH
    // Background and Fill are set (the shared shape_view_handler.cpp::map_background handles that
    // branch + the invalidate). A BoxView normally carries Fill (its Color) and no separate Background,
    // so this generic-IView push is the IView.Background path — apply it to the host View directly,
    // matching the port's no-container shape model (shape_view_handler.cpp header note).
    void shape_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // NOTE: do NOT clobber the maui GradientDrawable that push_box_to_view installs for the Fill.
        // Only a non-null IView.Background should reach the View here; a null leaves the box fill alone.
        if (native == nullptr || value == nullptr)
        {
            return;
        }
        maui::platform::android::apply_background(native, value);
    }

    void shape_view_platform::update_semantics(const maui::core::semantics* value)
    {
        view_platform_base::update_semantics(value);
        maui::platform::android::apply_semantics(native, value);
    }

    // Shadow / Clip / InputTransparent have no plain-android.view.View analog (android_visual_ops.hpp
    // deviation: C#'s ViewExtensions early-returns when the platformView is not a WrapperView, so an
    // unwrapped View receives no shadow/clip update in C# either). Keep ONLY the headless mirror — the
    // base bodies — exactly matching that unwrapped-View behavior. (No overrides declared for them in
    // the android block of shape_view_handler.hpp; the base view_platform_base bodies run.)

    std::unique_ptr<shape_view_platform> shape_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<shape_view_platform>();
        const scoped_env env;
        jobject context = app_context();
        if (!env || context == nullptr)
        {
            return platform; // VM-less / context-less: the headless-mirror degradation (header note)
        }
        auto& cache = default_jni_cache();
        // MauiBoxView : PlatformGraphicsView → the port's plain android.view.View(Context) host (the
        // GradientDrawable carries the paint; header deviations). The plain View ctor is
        // theme-independent, so it constructs in the bare app_process testhost.
        jclass view_class = cache.find_class(env.get(), k_view_class);
        jmethodID ctor = cache.method(env.get(), k_view_class, "<init>", "(Landroid/content/Context;)V");
        if (view_class == nullptr || ctor == nullptr)
        {
            return platform;
        }
        const local_ref<jobject> view{env.get(), env->NewObject(view_class, ctor, context)};
        if (clear_pending(env.get()) || !view)
        {
            return platform;
        }
        // Wrap-content LayoutParams up front (the button partial's rationale: a parentless View with
        // null LayoutParams trips relayout paths; the android container fan-out supplies them in C#).
        jclass layout_params_class = cache.find_class(env.get(), "android/view/ViewGroup$LayoutParams");
        jmethodID layout_params_ctor =
            cache.method(env.get(), "android/view/ViewGroup$LayoutParams", "<init>", "(II)V");
        jmethodID set_layout_params =
            cache.method(env.get(), k_view_class, "setLayoutParams", "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (layout_params_class != nullptr && layout_params_ctor != nullptr && set_layout_params != nullptr)
        {
            constexpr jint k_wrap_content = -2; // ViewGroup.LayoutParams.WRAP_CONTENT
            const local_ref<jobject> params{
                env.get(), env->NewObject(layout_params_class, layout_params_ctor, k_wrap_content, k_wrap_content)};
            if (!clear_pending(env.get()) && params)
            {
                env->CallVoidMethod(view.get(), set_layout_params, params.get());
                clear_pending(env.get());
            }
        }
        platform->native = env->NewGlobalRef(view.get()); // released in ~shape_view_platform
        return platform;
    }

    // C# UpdateShape (ShapeViewExtensions): re-point the host's drawable at the virtual view + redraw.
    // The shared headless mirror (drawable + winding/render-transform + invalidations) is kept live;
    // the native box fill + corner radius is pushed on top.
    void shape_view_handler::update_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->drawable.update_shape_view(virtual_view());
        refresh_drawable_state();
        platform->invalidations++;
        if (auto* view = virtual_view())
        {
            push_box_to_view(*platform, *view);
        }
    }

    // C# InvalidateShape: refresh the drawable pushes + count the redraw request, then re-push the box
    // to the native View (a fill/corner change funnels here through the union mapper).
    void shape_view_handler::invalidate_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        refresh_drawable_state();
        platform->invalidations++;
        if (auto* view = virtual_view())
        {
            push_box_to_view(*platform, *view);
        }
    }

    void shape_view_handler::arrange_native(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject view_obj = view_of(*platform);
        auto& cache = default_jni_cache();
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the View measures Exactly at the
        // final size (Android requires a measure pass before layout) and lays out (button partial).
        jmethodID make_measure_spec = cache.static_method(env.get(), k_measure_spec_class, "makeMeasureSpec", "(II)I");
        jmethodID measure = cache.method(env.get(), k_view_class, "measure", "(II)V");
        jmethodID layout = cache.method(env.get(), k_view_class, "layout", "(IIII)V");
        jclass measure_spec_class = cache.find_class(env.get(), k_measure_spec_class);
        if (make_measure_spec == nullptr || measure == nullptr || layout == nullptr || measure_spec_class == nullptr)
        {
            return;
        }
        const float density = display_density(env.get(), view_obj);
        const jint left = to_pixels(frame.x, density);
        const jint top = to_pixels(frame.y, density);
        const jint width = to_pixels(frame.width, density);
        const jint height = to_pixels(frame.height, density);
        const jint width_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, width, k_measure_spec_exactly);
        const jint height_spec =
            env->CallStaticIntMethod(measure_spec_class, make_measure_spec, height, k_measure_spec_exactly);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(view_obj, measure, width_spec, height_spec);
        if (clear_pending(env.get()))
        {
            return;
        }
        env->CallVoidMethod(view_obj, layout, left, top, left + width, top + height);
        clear_pending(env.get());
    }
} // namespace maui::core
