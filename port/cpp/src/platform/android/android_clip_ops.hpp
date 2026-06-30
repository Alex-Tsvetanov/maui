#pragma once
// Shared Android (JNI) operation for a GENERIC view's VisualElement.Clip — the platform side of the
// shared view_mapper's map_clip (view_mapper.cpp) for every non-image control. The JNI twin of
// apple_visual_ops.hpp's apply_clip (which installs a CAShapeLayer mask on the view's layer). Include only
// from the android partials. VM-less safe (returns when no JavaVM / no widget exists — the headless-mirror
// degradation the android partials document).
//
// apply_outline_clip ports the EFFECT of Microsoft.Maui.Platform.ViewExtensions.UpdateClip (Android): C#
// clips a WrapperView by setting WrapperView.Clip, which the WrapperView's own draw honours. This AAR-less
// backend has no per-control WrapperView, so — instead of leaving every non-image control's clip as a
// headless mirror only (the old android_visual_ops note) — it installs a ViewOutlineProvider +
// setClipToOutline(true) on the stock native view. The framework then clips the whole view (background,
// content, and a ViewGroup's children) to the outline, for ANY view type without subclassing it.
//
// CONVEX-ONLY — the honest constraint (documented in MauiClipOutlineProvider.java): outline clipping only
// takes effect for a CONVEX outline (Outline.canClip()). The clip family's convex shapes — EllipseGeometry,
// RectangleGeometry, RoundRectangleGeometry — clip exactly (this is what the clip_views gallery page
// exercises: one shared EllipseGeometry on a Button/DatePicker/Entry/Editor/Grid/SearchBar/TimePicker). A
// non-convex PathGeometry or multi-region GeometryGroup yields canClip()==false, so setClipToOutline is a
// silent no-op and the view renders UNCLIPPED — graceful degradation, no crash. apply_outline_clip reads
// the provider's hasClip() back and returns it, so the caller knows whether the convex clip actually landed
// (true) or fell back to the headless mirror (false) and can log/track the deferral. The arbitrary-path
// case (the iOS CAShapeLayer-mask generality) needs a custom WrapperView ViewGroup overriding dispatchDraw
// to canvas.clipPath() (the MauiImageView.onDraw / MauiShapeView pattern); that deeper port is deferred.
//
// The Path is built here in the view's PIXEL coordinate space: build_clip_path resolves the clip geometry
// against the view's live bounds (in points) and scales every coordinate by the display density, exactly as
// image_handler's build_clip_path does for the MauiImageView mask (the walk is duplicated rather than
// shared with the image partial, per the box/border corner_radii_of doctrine, so each partial stays
// independently buildable). The caller resolves path_for_bounds against rect{0,0,w,h} in POINTS (the
// WrapperView.SetClip convention — the apple apply_clip twin) before passing the path_f in.

#include <jni.h>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "maui/graphics/i_shape.hpp"
#include "maui/graphics/path_f.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/rect.hpp"

namespace maui::platform::android
{
    namespace detail
    {
        inline constexpr const char* k_clip_view_class = "android/view/View";
        inline constexpr const char* k_clip_path_class = "android/graphics/Path";
        inline constexpr const char* k_clip_provider_class = "dev/mauicpp/MauiClipOutlineProvider";

        // Build `path` (a path_f in POINT coordinates) into a fresh android.graphics.Path scaled to PIXELS by
        // `density`. Empty local ref on any JNI failure. The walk mirrors image_handler's build_clip_path /
        // android_canvas::build_path 1:1 (move/line/quad/cubic/arc/close). Arc mapping matches them exactly:
        // android's arcTo angles are clockwise from +x and the framework's are counter-clockwise, so the
        // start is negated and the sweep sign-adjusted.
        [[nodiscard]] inline local_ref<jobject> build_clip_path(JNIEnv* env, const maui::graphics::path_f& path,
                                                                float density)
        {
            auto& cache = default_jni_cache();
            jclass path_class = cache.find_class(env, k_clip_path_class);
            jmethodID path_ctor = cache.method(env, k_clip_path_class, "<init>", "()V");
            jmethodID move_to = cache.method(env, k_clip_path_class, "moveTo", "(FF)V");
            jmethodID line_to = cache.method(env, k_clip_path_class, "lineTo", "(FF)V");
            jmethodID quad_to = cache.method(env, k_clip_path_class, "quadTo", "(FFFF)V");
            jmethodID cubic_to = cache.method(env, k_clip_path_class, "cubicTo", "(FFFFFF)V");
            jmethodID close = cache.method(env, k_clip_path_class, "close", "()V");
            jmethodID arc_to = cache.method(env, k_clip_path_class, "arcTo", "(FFFFFFZ)V");
            if (path_class == nullptr || path_ctor == nullptr || move_to == nullptr || line_to == nullptr ||
                quad_to == nullptr || cubic_to == nullptr || close == nullptr || arc_to == nullptr)
            {
                return {};
            }
            local_ref<jobject> path_obj{env, env->NewObject(path_class, path_ctor)};
            if (env->ExceptionCheck() == JNI_TRUE || !path_obj)
            {
                env->ExceptionClear();
                return {};
            }
            const auto s = static_cast<jfloat>(density);
            int point_index = 0;
            int arc_angle_index = 0;
            int arc_clockwise_index = 0;
            const auto& operations = path.segment_types();
            for (const auto type : operations)
            {
                switch (type)
                {
                    case maui::graphics::path_operation::move: {
                        const maui::graphics::point_f p = path[point_index++];
                        env->CallVoidMethod(path_obj.get(), move_to, p.x * s, p.y * s);
                        break;
                    }
                    case maui::graphics::path_operation::line: {
                        const maui::graphics::point_f p = path[point_index++];
                        env->CallVoidMethod(path_obj.get(), line_to, p.x * s, p.y * s);
                        break;
                    }
                    case maui::graphics::path_operation::quad: {
                        const maui::graphics::point_f control = path[point_index++];
                        const maui::graphics::point_f end = path[point_index++];
                        env->CallVoidMethod(path_obj.get(), quad_to, control.x * s, control.y * s, end.x * s,
                                            end.y * s);
                        break;
                    }
                    case maui::graphics::path_operation::cubic: {
                        const maui::graphics::point_f c1 = path[point_index++];
                        const maui::graphics::point_f c2 = path[point_index++];
                        const maui::graphics::point_f end = path[point_index++];
                        env->CallVoidMethod(path_obj.get(), cubic_to, c1.x * s, c1.y * s, c2.x * s, c2.y * s, end.x * s,
                                            end.y * s);
                        break;
                    }
                    case maui::graphics::path_operation::arc: {
                        const maui::graphics::point_f top_left = path[point_index++];
                        const maui::graphics::point_f bottom_right = path[point_index++];
                        const float start_angle = path.get_arc_angle(arc_angle_index++);
                        const float end_angle = path.get_arc_angle(arc_angle_index++);
                        const bool clockwise = path.get_arc_clockwise(arc_clockwise_index++);
                        float sweep = -(end_angle - start_angle);
                        if (!clockwise && sweep > 0)
                        {
                            sweep -= 360.0F;
                        }
                        else if (clockwise && sweep < 0)
                        {
                            sweep += 360.0F;
                        }
                        env->CallVoidMethod(path_obj.get(), arc_to, top_left.x * s, top_left.y * s, bottom_right.x * s,
                                            bottom_right.y * s, static_cast<jfloat>(-start_angle),
                                            static_cast<jfloat>(sweep), static_cast<jboolean>(false));
                        break;
                    }
                    case maui::graphics::path_operation::close:
                        env->CallVoidMethod(path_obj.get(), close);
                        break;
                }
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    env->ExceptionClear();
                }
            }
            return path_obj;
        }
    } // namespace detail

    // Install (or clear, when shape is null or width/height is 0) the outline clip on `native`. Returns true
    // when a CONVEX clip actually landed (the framework accepted the path as clippable), false when the clip
    // is unset/cleared OR the geometry is non-convex (the headless-mirror fallback). `width`/`height` are the
    // view's CURRENT size in POINTS (pixels / density): the clip geometry is bounds-dependent, so the caller
    // re-invokes this from platform_arrange after the view has its final size.
    inline bool apply_outline_clip(void* native, const maui::graphics::i_shape* shape, float density, double width,
                                   double height)
    {
        if (native == nullptr)
        {
            return false;
        }
        const scoped_env env;
        if (!env)
        {
            return false;
        }
        auto* const view = static_cast<jobject>(native);
        auto& cache = default_jni_cache();
        jmethodID set_provider = cache.method(env.get(), detail::k_clip_view_class, "setOutlineProvider",
                                              "(Landroid/view/ViewOutlineProvider;)V");
        jmethodID set_clip_to_outline = cache.method(env.get(), detail::k_clip_view_class, "setClipToOutline", "(Z)V");
        jmethodID invalidate_outline = cache.method(env.get(), detail::k_clip_view_class, "invalidateOutline", "()V");
        if (set_provider == nullptr || set_clip_to_outline == nullptr || invalidate_outline == nullptr)
        {
            return false;
        }

        // Null shape (clip removed) OR a not-yet-laid-out 0-sized view: clear our outline clip. Restore the
        // framework's BACKGROUND outline provider (the default — it tracks the background drawable's shape so
        // shadows still cast correctly) and turn clipToOutline off, mirroring WrapperView.SetClip(null).
        if (shape == nullptr || width <= 0.0 || height <= 0.0)
        {
            jclass provider_base = cache.find_class(env.get(), "android/view/ViewOutlineProvider");
            // BACKGROUND is a STATIC field, so it needs GetStaticFieldID (cache.field resolves INSTANCE
            // fields — mirrors how android_visual_ops reads the GradientDrawable$Orientation static enum).
            jfieldID background_field =
                provider_base != nullptr
                    ? env->GetStaticFieldID(provider_base, "BACKGROUND", "Landroid/view/ViewOutlineProvider;")
                    : nullptr;
            if (background_field == nullptr)
            {
                env->ExceptionClear();
            }
            if (provider_base != nullptr && background_field != nullptr)
            {
                const local_ref<jobject> background{env.get(),
                                                    env->GetStaticObjectField(provider_base, background_field)};
                if (env->ExceptionCheck() == JNI_TRUE)
                {
                    env->ExceptionClear();
                }
                else
                {
                    env->CallVoidMethod(view, set_provider, background.get());
                }
            }
            env->CallVoidMethod(view, set_clip_to_outline, static_cast<jboolean>(false));
            env->CallVoidMethod(view, invalidate_outline);
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
            }
            return false;
        }

        // Resolve the clip geometry against the live bounds in POINTS (the WrapperView.SetClip convention —
        // RectF(0,0,width,height) in points; the apple apply_clip twin), then build the pixel-space Path.
        const maui::graphics::path_f path = shape->path_for_bounds(maui::graphics::rect{0.0, 0.0, width, height});
        const local_ref<jobject> path_obj = detail::build_clip_path(env.get(), path, density);
        if (!path_obj)
        {
            return false;
        }

        // new MauiClipOutlineProvider(path): the ViewOutlineProvider that clips to this convex path.
        jclass provider_class = cache.find_class(env.get(), detail::k_clip_provider_class);
        jmethodID provider_ctor = provider_class != nullptr ? cache.method(env.get(), detail::k_clip_provider_class,
                                                                           "<init>", "(Landroid/graphics/Path;)V")
                                                            : nullptr;
        if (provider_class == nullptr || provider_ctor == nullptr)
        {
            return false; // MauiClipOutlineProvider is host-provided (java/MauiClipOutlineProvider.java)
        }
        const local_ref<jobject> provider{env.get(), env->NewObject(provider_class, provider_ctor, path_obj.get())};
        if (env->ExceptionCheck() == JNI_TRUE || !provider)
        {
            env->ExceptionClear();
            return false;
        }

        // Install the provider, enable outline clipping, and force the framework to re-query getOutline. The
        // getOutline call (driven by invalidateOutline) sets the provider's hasClip flag, which we then read
        // back to report whether the convex clip actually landed.
        env->CallVoidMethod(view, set_provider, provider.get());
        env->CallVoidMethod(view, set_clip_to_outline, static_cast<jboolean>(true));
        env->CallVoidMethod(view, invalidate_outline);
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return false;
        }

        jmethodID has_clip = cache.method(env.get(), detail::k_clip_provider_class, "hasClip", "()Z");
        if (has_clip == nullptr)
        {
            return true; // installed; assume it took (the convex shapes this page uses do)
        }
        const jboolean landed = env->CallBooleanMethod(provider.get(), has_clip);
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            env->ExceptionClear();
            return true;
        }
        return landed == JNI_TRUE;
    }
} // namespace maui::platform::android
