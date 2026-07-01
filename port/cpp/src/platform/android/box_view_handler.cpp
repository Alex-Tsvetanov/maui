// shape_view_handler — Android (JNI) platform recipe, the box_view slice of the M-android per-control
// fan-out. box_view is NOT a handler of its own: BoxView is its own IShapeView AND its own IShape and
// renders through the shared shape_view_handler (box_view.cpp self-registers
// MAUI_REGISTER_HANDLER(box_view, shape_view_handler); shape_view_handler.cpp/.hpp hold the
// cross-platform mapper + the shape_view_platform struct). This file is the Android half of that
// platform recipe — the JNI twin of src/platform/apple/shape_view_handler.mm and the
// src/platform/headless/shape_view_handler.cpp mirror.
//
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// WAVE 7: the canvas bridge (the faithful render) replaces the GradientDrawable shortcut
// ──────────────────────────────────────────────────────────────────────────────────────────────────
// In .NET MAUI, Android shapes render through Microsoft.Maui.Platform.MauiShapeView : PlatformGraphicsView
// (src/Core/src/Platform/Android/MauiShapeView.cs) — a View whose OnDraw(Canvas) wraps the
// android.graphics.Canvas in Microsoft.Maui.Graphics.Platform.PlatformCanvas and calls the drawable's
// Draw(ICanvas, RectF). The port now reproduces this faithfully: create_platform_view builds a
// dev.mauicpp.MauiShapeView (java/MauiShapeView.java), whose onDraw crosses back into native_draw here,
// which constructs an android_canvas (android_canvas.{hpp,cpp}) over the incoming Canvas and calls
// shape_view_platform::replay → shape_drawable.draw — the SAME drawable the apple/ios drawRect hosts
// render. This unblocks the WHOLE shapes family on Android (ellipse/line/polyline/polygon/path/rectangle/
// rounded rectangle), where the prior cut rendered nothing.
//
// PRIOR DEVIATION (now retired): the first box_view cut installed an android.graphics.drawable.
// GradientDrawable as a plain View's background to express BoxView's solid rounded rectangle. That was a
// faithful expression of a strokeless solid box but had NO analog for arbitrary paths, strokes, or
// non-box shapes — so every non-box shape page rendered blank. The canvas render subsumes it: BoxView's
// own ShapeDrawable already fills its rounded rectangle through the same canvas, so box_view + border stay
// rendered (now via the real fill/stroke path, not a drawable stand-in) and the rest of the family fills
// in. No GradientDrawable remains in this file.
//
// THE HEADLESS MIRROR STAYS LIVE: every shared write the headless/apple twins make
// (drawable.update_shape_view, refresh_drawable_state's winding/render-transform pushes, the
// invalidations counter) is preserved, so the android preset's PURE-NATIVE cross-platform suite
// (no JavaVM) observes exactly the headless partial's behavior. The native redraw (MauiShapeView
// .invalidate()) is layered ON TOP, behind scoped_env/app_context guards, and runs only when a VM + a
// native View exist (the widget testhost + the app host).
//
// COORDINATE CONVENTION: arrange_native frames the View in PIXELS (to_pixels), and native_draw hands the
// drawable a dirty_rect in POINTS (the pixel width/height ÷ density) after setting the canvas's
// display_scale to the density — the android_canvas applies one canvas.scale(density,density) so the
// point-coordinate ops land at the right pixel. This matches the framework laying out in points and the
// seam scaling at the boundary, exactly as the headless/apple canvases carry display_scale.
//
// Shadow / Clip / InputTransparent have no plain-View analog (C#'s ViewExtensions early-returns when the
// platformView is not a WrapperView), so they keep ONLY the headless mirror (the base bodies) — no
// android override is declared for them in shape_view_handler.hpp.

#include "maui/core/shape_view_handler.hpp"

#include <jni.h>

#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <string_view>

#include "android_canvas.hpp"
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
#include "maui/graphics/rect.hpp"
#include "maui/graphics/rect_f.hpp"
#include "maui/graphics/size.hpp"

namespace
{
    using maui::platform::android::android_canvas;
    using maui::platform::android::app_context;
    using maui::platform::android::default_jni_cache;
    using maui::platform::android::local_ref;
    using maui::platform::android::scoped_env;

    // The custom drawing View hosting the shape (dev.mauicpp.MauiShapeView — the PlatformGraphicsView
    // twin). GetMethodID walks superclasses, so View surface methods resolve through this class too.
    constexpr const char* k_shape_view_class = "dev/mauicpp/MauiShapeView";
    constexpr const char* k_view_class = "android/view/View";
    constexpr const char* k_measure_spec_class = "android/view/View$MeasureSpec";

    // GeometryUtil.Epsilon — ContextExtensions.ToPixels subtracts it before ceiling (see to_pixels).
    constexpr double k_to_pixels_epsilon = 0.0000000001;

    // android.view.View visibility states (ViewExtensions.ToPlatformVisibility's targets).
    constexpr jint k_view_visible = 0;
    constexpr jint k_view_invisible = 4;
    constexpr jint k_view_gone = 8;

    // android.view.View.MeasureSpec EXACTLY mode (PlatformArrange).
    constexpr auto k_measure_spec_exactly = static_cast<jint>(0x40000000U);

    [[nodiscard]] jobject view_of(const maui::core::shape_view_platform& platform) noexcept
    {
        return static_cast<jobject>(platform.native);
    }

    // Clears any pending Java exception (the partial must never leak JNI pending-exception state); true
    // when one was pending — call sites skip the read-back.
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
        if (jmethodID method = default_jni_cache().method(env, k_shape_view_class, name, "(I)V"))
        {
            env->CallVoidMethod(view, method, value);
            clear_pending(env);
        }
    }

    void call_void_float(JNIEnv* env, jobject view, const char* name, jfloat value)
    {
        if (jmethodID method = default_jni_cache().method(env, k_shape_view_class, name, "(F)V"))
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
    // process-wide after the first successful read; 1.0 when any step fails (failures are not memoized).
    [[nodiscard]] float display_density(JNIEnv* env, jobject view)
    {
        static std::atomic<float> memoized{0.0F}; // 0 = not read yet (a real density is never 0)
        if (const float cached = memoized.load(std::memory_order_relaxed); cached != 0.0F)
        {
            return cached;
        }
        auto& cache = default_jni_cache();
        jmethodID get_context = cache.method(env, k_shape_view_class, "getContext", "()Landroid/content/Context;");
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

    // MauiShapeView.nativeDraw(long peer, Canvas, int width, int height): the JNI callback the View's
    // onDraw crosses into. The peer is the handler's shape_view_platform; build an android_canvas over the
    // Canvas and replay the shared shape drawable into it — the SAME drawable the apple/ios drawRect hosts
    // render. width/height are PIXELS; the drawable draws in POINTS, so divide by density for the dirty
    // rect and set the canvas's display_scale to the density (android_canvas applies the matching scale).
    void JNICALL native_draw(JNIEnv* env, jclass /*view_class*/, jlong peer, jobject canvas, jint width, jint height)
    {
        auto* platform = reinterpret_cast<maui::core::shape_view_platform*>(peer);
        if (platform == nullptr || canvas == nullptr || env == nullptr)
        {
            return;
        }
        const float density = display_density(env, view_of(*platform));
        const float scale = density > 0 ? density : 1.0F;
        android_canvas bridge(env, canvas);
        bridge.set_display_scale(scale); // applies canvas.scale(density,density) before any op
        const auto w = static_cast<float>(width) / scale;
        const auto h = static_cast<float>(height) / scale;
        platform->replay(bridge, maui::graphics::rect_f(0.0F, 0.0F, w, h));
    }

    // Binds nativeDraw to MauiShapeView (RegisterNatives — no Java_* export needed). Idempotent
    // (RegisterNatives replaces an existing binding), so it is safe to call on every view creation.
    [[nodiscard]] bool register_draw_natives(JNIEnv* env, jclass view_class)
    {
        static const std::array<JNINativeMethod, 1> k_methods{
            JNINativeMethod{.name = const_cast<char*>("nativeDraw"),
                            .signature = const_cast<char*>("(JLandroid/graphics/Canvas;II)V"),
                            .fnPtr = reinterpret_cast<void*>(&native_draw)},
        };
        const jint status = env->RegisterNatives(view_class, k_methods.data(), static_cast<jint>(k_methods.size()));
        if (status != JNI_OK)
        {
            clear_pending(env);
            return false;
        }
        return true;
    }

    // Schedule a redraw of the shape View (the C# Drawable setter / InvalidateShape invalidate). A plain
    // View.invalidate() — the handler calls this on every update_shape/invalidate_shape so onDraw re-runs.
    void invalidate_view(maui::core::shape_view_platform& platform)
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
        jmethodID invalidate = default_jni_cache().method(env.get(), k_shape_view_class, "invalidate", "()V");
        if (invalidate != nullptr)
        {
            env->CallVoidMethod(view_of(platform), invalidate);
            clear_pending(env.get());
        }
    }
} // namespace

namespace maui::core
{
    // Releases the global reference pinning the MauiShapeView (the JNI shape of the
    // pimpl-owned-native-view doctrine; the apple twin CFReleases its NSView host here). Clears the
    // view's nativePtr FIRST so a late onDraw cannot dereference this struct mid-teardown.
    shape_view_platform::~shape_view_platform()
    {
        if (native != nullptr)
        {
            const scoped_env env; // any-thread teardown, exactly like global_ref::reset
            if (env)
            {
                if (jmethodID set_ptr =
                        default_jni_cache().method(env.get(), k_shape_view_class, "setNativePtr", "(J)V"))
                {
                    env->CallVoidMethod(static_cast<jobject>(native), set_ptr, static_cast<jlong>(0));
                    clear_pending(env.get());
                }
                env->DeleteGlobalRef(static_cast<jobject>(native));
            }
            native = nullptr;
        }
    }

    // The portable replay seat (shared with headless; on Android the REAL draw runs through native_draw
    // above, which calls THIS, then onto drawable.draw). Kept live so any recording-canvas host observes
    // the same shape ops.
    void shape_view_platform::replay(maui::graphics::i_canvas& canvas, const maui::graphics::rect_f& dirty_rect)
    {
        drawable.draw(canvas, dirty_rect);
    }

    // ---- the generic-IView property pushes (the shared view_mapper calls these through
    // view_platform_base). Each calls the base body FIRST — the headless mirrors must stay live for the
    // VM-less cross-platform suite — then pushes to the real View when one exists. ----

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
            call_void_float(env.get(), view_of(*this), "setAlpha", static_cast<jfloat>(value));
        }
    }

    void shape_view_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
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
            cache.method(env.get(), k_shape_view_class, "setContentDescription", "(Ljava/lang/CharSequence;)V");
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

    void shape_view_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        // The shape's Fill renders through the canvas (drawable). A non-null IView.Background still maps to
        // the host View's background (the no-container shape model); a null leaves the transparent drawing
        // View alone so the shape paints over a clear backdrop.
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

    // IView.Shadow on a shape view → the native colored elevation shadow (android_visual_ops apply_shadow).
    // Base mirror FIRST (the VM-less cross-platform suite observes the borrow), then the widget push. The
    // view is 0×0 at map time, so apply_shadow clears here and arrange_native re-invokes it at the live size
    // (the shadow outline is bounds-dependent, exactly like update_clip's outline). The shape's silhouette is
    // approximated by a plain rounded-rect outline of corner radius 0 (a rect glow) — the exact per-shape
    // shadow silhouette needs the software WrapperView port (documented on apply_shadow).
    void shape_view_platform::update_shadow(const maui::core::i_shadow* value)
    {
        view_platform_base::update_shadow(value);
        if (native == nullptr)
        {
            return;
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        jobject view_obj = view_of(*this);
        const float density = display_density(env.get(), view_obj);
        const double width = maui::platform::android::detail::view_width_dp(env.get(), view_obj, density);
        const double height = maui::platform::android::detail::view_height_dp(env.get(), view_obj, density);
        maui::platform::android::apply_shadow(native, value, density, width, height, 0.0);
    }

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
        // MauiShapeView : PlatformGraphicsView → dev.mauicpp.MauiShapeView(Context). The plain View ctor is
        // theme-independent, so it constructs in the bare app_process testhost.
        jclass view_class = cache.find_class(env.get(), k_shape_view_class);
        jmethodID ctor = cache.method(env.get(), k_shape_view_class, "<init>", "(Landroid/content/Context;)V");
        if (view_class == nullptr || ctor == nullptr || !register_draw_natives(env.get(), view_class))
        {
            return platform; // the MauiShapeView class is host-provided (java/MauiShapeView.java); without
                             // it the shape renders nothing on the native side (headless mirror stays live)
        }
        const local_ref<jobject> view{env.get(), env->NewObject(view_class, ctor, context)};
        if (clear_pending(env.get()) || !view)
        {
            return platform;
        }
        // Wrap-content LayoutParams up front (the button partial's rationale: a parentless View with null
        // LayoutParams trips relayout paths).
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
        // Seed the view's peer with this platform struct's address so onDraw → native_draw reaches it; the
        // dtor clears it (setNativePtr(0)) before the struct dies.
        if (jmethodID set_ptr = cache.method(env.get(), k_shape_view_class, "setNativePtr", "(J)V"))
        {
            env->CallVoidMethod(view.get(), set_ptr, reinterpret_cast<jlong>(platform.get()));
            clear_pending(env.get());
        }
        return platform;
    }

    // C# UpdateShape (ShapeViewExtensions): re-point the host's drawable at the virtual view + redraw. The
    // shared headless mirror (drawable + winding/render-transform + invalidations) is kept live; the native
    // View is invalidated so onDraw re-runs the canvas render.
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
        invalidate_view(*platform);
    }

    // C# InvalidateShape: refresh the drawable pushes + count the redraw request, then invalidate the View.
    void shape_view_handler::invalidate_shape()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        refresh_drawable_state();
        platform->invalidations++;
        invalidate_view(*platform);
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
        // ViewHandler.PlatformArrange: the dp frame becomes pixels, the View measures Exactly at the final
        // size (Android requires a measure pass before layout) and lays out.
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

        // Re-resolve the (bounds-dependent) native shadow outline against the just-laid-out size — update_shadow
        // may have run before layout when the view was 0×0 (apply_shadow cleared the elevation then), and a
        // resize must rebuild the caster rect. Mirrors the clip reapply pattern (button_handler platform_arrange).
        if (platform->shadow != nullptr)
        {
            maui::platform::android::apply_shadow(view_obj, platform->shadow, density, frame.width, frame.height, 0.0);
        }
    }
} // namespace maui::core
