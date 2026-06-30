// maui::platform::android::android_canvas — the android.graphics.Canvas i_canvas. See
// android_canvas.hpp. Ported from src/Graphics/src/Graphics/Platforms/Android/PlatformCanvas.cs
// (+ PathExtensions.cs's PathF→android.graphics.Path walk and CanvasExtensions.cs). The op→native
// mapping mirrors the apple_shared coregraphics_canvas (the same shape_drawable drives both): every
// DrawPath strokes, every FillPath fills, every Clip* intersects the canvas clip, transforms push onto
// the Canvas matrix. android.graphics.Paint carries the style (FILL/STROKE) + color + stroke geometry,
// android.graphics.Path carries the geometry, both reused across ops.

#include "android_canvas.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "maui/graphics/colors.hpp"
#include "maui/graphics/gradient_paint.hpp"
#include "maui/graphics/gradient_stop.hpp"
#include "maui/graphics/linear_gradient_paint.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/path_operation.hpp"
#include "maui/graphics/point_f.hpp"
#include "maui/graphics/radial_gradient_paint.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::platform::android
{
    namespace
    {
        constexpr const char* k_canvas_class = "android/graphics/Canvas";
        constexpr const char* k_paint_class = "android/graphics/Paint";
        constexpr const char* k_path_class = "android/graphics/Path";
        constexpr const char* k_rectf_class = "android/graphics/RectF";
        constexpr const char* k_paint_style_class = "android/graphics/Paint$Style";
        constexpr const char* k_paint_cap_class = "android/graphics/Paint$Cap";
        constexpr const char* k_paint_join_class = "android/graphics/Paint$Join";
        constexpr const char* k_path_filltype_class = "android/graphics/Path$FillType";
        constexpr const char* k_dash_path_effect_class = "android/graphics/DashPathEffect";
        constexpr const char* k_region_op_class = "android/graphics/Region$Op";
        constexpr const char* k_linear_gradient_class = "android/graphics/LinearGradient";
        constexpr const char* k_radial_gradient_class = "android/graphics/RadialGradient";
        constexpr const char* k_shader_tilemode_class = "android/graphics/Shader$TileMode";

        // Clears any pending Java exception (the bridge must never leak JNI pending-exception state); true
        // when one was pending. Mirrors the box/button partials' clear_pending.
        bool clear_pending(JNIEnv* env)
        {
            if (env == nullptr || env->ExceptionCheck() == JNI_FALSE)
            {
                return false;
            }
            env->ExceptionDescribe();
            env->ExceptionClear();
            return true;
        }

        // Read a named static enum constant (Paint$Style.FILL etc.) as a fresh local ref, or empty. The
        // enum classes' constants are public static final fields of the enum's own type (LType;).
        local_ref<jobject> static_enum(JNIEnv* env, const char* class_name, const char* field, const char* type)
        {
            jclass owner = default_jni_cache().find_class(env, class_name);
            if (owner == nullptr)
            {
                return {};
            }
            jfieldID id = env->GetStaticFieldID(owner, field, type);
            if (id == nullptr || clear_pending(env))
            {
                return {};
            }
            local_ref<jobject> value{env, env->GetStaticObjectField(owner, id)};
            clear_pending(env);
            return value;
        }

        // android.graphics.Color.argb packs an int the same way the port's color::to_int() does
        // (0xAARRGGBB), so Paint.setColor(int) takes color.to_int() directly — no per-channel build.
        jint to_argb(const maui::graphics::color& value)
        {
            return static_cast<jint>(value.to_int());
        }

        // android.graphics.Shader.TileMode.CLAMP (the C# Shader.TileMode.Clamp both gradients use). The
        // enum's constants are public static final fields of the enum's own type.
        local_ref<jobject> clamp_tile_mode(JNIEnv* env)
        {
            return static_enum(env, k_shader_tilemode_class, "CLAMP", "Landroid/graphics/Shader$TileMode;");
        }

        // Build the parallel int[] colors / float[] stops a LinearGradient/RadialGradient ctor takes from a
        // gradient paint's SORTED stops (C# SetFillPaint: `GetSortedStops()` → colors[i]/stops[i]). The two
        // arrays are returned as fresh local refs; both empty on a JNI failure. The shader ctors require at
        // least two colors, so a <2-stop paint is widened to a flat two-color ramp of the lone/first color
        // (the visible result that gradient yields) — matching android_visual_ops::stop_color_array.
        //
        // DEVIATION from the C# oracle: C# multiplies each stop color by CurrentState.Alpha
        // (Color.MultiplyAlpha(Alpha).ToInt()). This bridge does not track a separate state alpha — the
        // global alpha is pushed straight onto the Paint (set_alpha → Paint.setAlpha), which still modulates
        // a shader's output, so the stop colors are used un-pre-multiplied (color.to_int()). No gallery
        // gradient page sets a non-1 fill alpha, so the rendered result is identical; documented as a port
        // decision (the Paint-level alpha covers the common case the state-alpha multiply would).
        struct gradient_arrays
        {
            local_ref<jintArray> colors;
            local_ref<jfloatArray> stops;
        };

        gradient_arrays build_gradient_arrays(JNIEnv* env, const maui::graphics::gradient_paint& gradient)
        {
            const std::vector<maui::graphics::gradient_stop> sorted = gradient.get_sorted_stops();
            const auto count = static_cast<jsize>(sorted.size() < 2 ? 2 : sorted.size());
            local_ref<jintArray> colors{env, env->NewIntArray(count)};
            local_ref<jfloatArray> stops{env, env->NewFloatArray(count)};
            if (!colors || !stops)
            {
                env->ExceptionClear();
                return {};
            }
            std::vector<jint> argb(static_cast<std::size_t>(count));
            std::vector<jfloat> offsets(static_cast<std::size_t>(count));
            if (sorted.empty())
            {
                // No stops: a transparent two-color ramp at 0..1 (a deliberately-emptied paint — render it
                // transparent rather than crash; GradientPaint defaults to white-to-white, so this is rare).
                argb.assign(static_cast<std::size_t>(count), 0);
                offsets[0] = 0.0F;
                offsets[1] = 1.0F;
            }
            else if (sorted.size() == 1)
            {
                argb[0] = static_cast<jint>(sorted[0].color().to_int());
                argb[1] = argb[0];
                offsets[0] = 0.0F;
                offsets[1] = 1.0F;
            }
            else
            {
                for (std::size_t i = 0; i < sorted.size(); ++i)
                {
                    argb[i] = static_cast<jint>(sorted[i].color().to_int());
                    offsets[i] = static_cast<jfloat>(sorted[i].offset());
                }
            }
            env->SetIntArrayRegion(colors.get(), 0, count, argb.data());
            env->SetFloatArrayRegion(stops.get(), 0, count, offsets.data());
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
                return {};
            }
            return {.colors = std::move(colors), .stops = std::move(stops)};
        }
    } // namespace

    android_canvas::android_canvas(JNIEnv* env, jobject canvas) : env_(env), canvas_(canvas)
    {
        if (env_ == nullptr || canvas_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jclass paint_class = cache.find_class(env_, k_paint_class);
        jmethodID paint_ctor = cache.method(env_, k_paint_class, "<init>", "(I)V");
        jclass path_class = cache.find_class(env_, k_path_class);
        jmethodID path_ctor = cache.method(env_, k_path_class, "<init>", "()V");
        jclass rectf_class = cache.find_class(env_, k_rectf_class);
        jmethodID rectf_ctor = cache.method(env_, k_rectf_class, "<init>", "()V");
        if (paint_class == nullptr || paint_ctor == nullptr || path_class == nullptr || path_ctor == nullptr ||
            rectf_class == nullptr || rectf_ctor == nullptr)
        {
            return;
        }
        // Paint(Paint.ANTI_ALIAS_FLAG) — antialiased by default, like C# PlatformCanvas (and i_canvas's
        // Antialias default true). Two paints: one styled FILL, one STROKE, so a draw never re-styles.
        constexpr jint k_anti_alias_flag = 1; // android.graphics.Paint.ANTI_ALIAS_FLAG
        const local_ref<jobject> fill{env_, env_->NewObject(paint_class, paint_ctor, k_anti_alias_flag)};
        const local_ref<jobject> stroke{env_, env_->NewObject(paint_class, paint_ctor, k_anti_alias_flag)};
        const local_ref<jobject> path{env_, env_->NewObject(path_class, path_ctor)};
        const local_ref<jobject> rectf{env_, env_->NewObject(rectf_class, rectf_ctor)};
        if (clear_pending(env_) || !fill || !stroke || !path || !rectf)
        {
            return;
        }
        fill_paint_ = env_->NewGlobalRef(fill.get());
        stroke_paint_ = env_->NewGlobalRef(stroke.get());
        path_ = env_->NewGlobalRef(path.get());
        rectf_ = env_->NewGlobalRef(rectf.get());

        // Style the two paints once: FILL / STROKE.
        const local_ref<jobject> fill_style =
            static_enum(env_, k_paint_style_class, "FILL", "Landroid/graphics/Paint$Style;");
        const local_ref<jobject> stroke_style =
            static_enum(env_, k_paint_style_class, "STROKE", "Landroid/graphics/Paint$Style;");
        jmethodID set_style = cache.method(env_, k_paint_class, "setStyle", "(Landroid/graphics/Paint$Style;)V");
        if (set_style != nullptr)
        {
            if (fill_style)
            {
                env_->CallVoidMethod(fill_paint_, set_style, fill_style.get());
                clear_pending(env_);
            }
            if (stroke_style)
            {
                env_->CallVoidMethod(stroke_paint_, set_style, stroke_style.get());
                clear_pending(env_);
            }
        }
    }

    android_canvas::~android_canvas()
    {
        if (env_ == nullptr)
        {
            return;
        }
        // The Paint/Path/RectF globals are owned; the Canvas is borrowed (the Java onDraw owns it).
        if (fill_paint_ != nullptr)
        {
            env_->DeleteGlobalRef(fill_paint_);
        }
        if (stroke_paint_ != nullptr)
        {
            env_->DeleteGlobalRef(stroke_paint_);
        }
        if (path_ != nullptr)
        {
            env_->DeleteGlobalRef(path_);
        }
        if (rectf_ != nullptr)
        {
            env_->DeleteGlobalRef(rectf_);
        }
    }

    void android_canvas::set_display_scale(float value)
    {
        abstract_canvas::set_display_scale(value);
        // The framework draws in POINTS; the Canvas is in PIXELS. Scale once so every point-coordinate op
        // lands at the right pixel (header note). Applied to the live Canvas matrix (not tracked on the
        // abstract state — it is a fixed device transform, not part of save/restore the drawable manages).
        if (env_ == nullptr || canvas_ == nullptr || !(value > 0))
        {
            return;
        }
        jmethodID scale = default_jni_cache().method(env_, k_canvas_class, "scale", "(FF)V");
        if (scale != nullptr)
        {
            env_->CallVoidMethod(canvas_, scale, static_cast<jfloat>(value), static_cast<jfloat>(value));
            clear_pending(env_);
        }
    }

    void android_canvas::set_miter_limit(float value)
    {
        if (stroke_paint_ == nullptr)
        {
            return;
        }
        jmethodID set_miter = default_jni_cache().method(env_, k_paint_class, "setStrokeMiter", "(F)V");
        if (set_miter != nullptr)
        {
            env_->CallVoidMethod(stroke_paint_, set_miter, static_cast<jfloat>(value));
            clear_pending(env_);
        }
    }

    void android_canvas::set_stroke_color(const maui::graphics::color& value)
    {
        if (stroke_paint_ == nullptr)
        {
            return;
        }
        jmethodID set_color = default_jni_cache().method(env_, k_paint_class, "setColor", "(I)V");
        if (set_color != nullptr)
        {
            env_->CallVoidMethod(stroke_paint_, set_color, to_argb(value));
            clear_pending(env_);
        }
    }

    void android_canvas::set_stroke_line_cap(maui::graphics::line_cap value)
    {
        if (stroke_paint_ == nullptr)
        {
            return;
        }
        const char* name = "BUTT";
        if (value == maui::graphics::line_cap::round)
        {
            name = "ROUND";
        }
        else if (value == maui::graphics::line_cap::square)
        {
            name = "SQUARE";
        }
        const local_ref<jobject> cap = static_enum(env_, k_paint_cap_class, name, "Landroid/graphics/Paint$Cap;");
        jmethodID set_cap =
            default_jni_cache().method(env_, k_paint_class, "setStrokeCap", "(Landroid/graphics/Paint$Cap;)V");
        if (set_cap != nullptr && cap)
        {
            env_->CallVoidMethod(stroke_paint_, set_cap, cap.get());
            clear_pending(env_);
        }
    }

    void android_canvas::set_stroke_line_join(maui::graphics::line_join value)
    {
        if (stroke_paint_ == nullptr)
        {
            return;
        }
        const char* name = "MITER";
        if (value == maui::graphics::line_join::round)
        {
            name = "ROUND";
        }
        else if (value == maui::graphics::line_join::bevel)
        {
            name = "BEVEL";
        }
        const local_ref<jobject> join = static_enum(env_, k_paint_join_class, name, "Landroid/graphics/Paint$Join;");
        jmethodID set_join =
            default_jni_cache().method(env_, k_paint_class, "setStrokeJoin", "(Landroid/graphics/Paint$Join;)V");
        if (set_join != nullptr && join)
        {
            env_->CallVoidMethod(stroke_paint_, set_join, join.get());
            clear_pending(env_);
        }
    }

    void android_canvas::apply_fill_color(const maui::graphics::color& value)
    {
        if (fill_paint_ == nullptr)
        {
            return;
        }
        // A solid color must drop any gradient shader staged by a prior set_fill_paint (Android draws the
        // shader IN PREFERENCE to the color while one is set), so clear it before pushing the color.
        set_fill_shader(nullptr);
        jmethodID set_color = default_jni_cache().method(env_, k_paint_class, "setColor", "(I)V");
        if (set_color != nullptr)
        {
            env_->CallVoidMethod(fill_paint_, set_color, to_argb(value));
            clear_pending(env_);
        }
    }

    void android_canvas::set_fill_shader(jobject shader)
    {
        if (fill_paint_ == nullptr)
        {
            return;
        }
        // Paint.setShader(Shader) returns the passed-in shader; wrap the returned local ref so it is freed
        // (it is the same object, but JNI still mints a local ref per ObjectMethod call). A null clears it.
        jmethodID set_shader = default_jni_cache().method(env_, k_paint_class, "setShader",
                                                          "(Landroid/graphics/Shader;)Landroid/graphics/Shader;");
        if (set_shader != nullptr)
        {
            const local_ref<jobject> prior{env_, env_->CallObjectMethod(fill_paint_, set_shader, shader)};
            clear_pending(env_);
        }
    }

    void android_canvas::set_fill_color(const maui::graphics::color& value)
    {
        apply_fill_color(value);
    }

    void android_canvas::set_font_color(const maui::graphics::color& value)
    {
        font_color_ = value;
    }

    void android_canvas::set_font(const maui::graphics::font& value)
    {
        font_ = value;
    }

    void android_canvas::set_font_size(float value)
    {
        font_size_ = value;
    }

    void android_canvas::set_alpha(float value)
    {
        // Paint.setAlpha takes 0..255; apply to both paints so fills + strokes honour the global alpha.
        const auto alpha = static_cast<jint>(std::lround(std::clamp(value, 0.0F, 1.0F) * 255.0F));
        jmethodID set_alpha = default_jni_cache().method(env_, k_paint_class, "setAlpha", "(I)V");
        if (set_alpha == nullptr)
        {
            return;
        }
        if (fill_paint_ != nullptr)
        {
            env_->CallVoidMethod(fill_paint_, set_alpha, alpha);
            clear_pending(env_);
        }
        if (stroke_paint_ != nullptr)
        {
            env_->CallVoidMethod(stroke_paint_, set_alpha, alpha);
            clear_pending(env_);
        }
    }

    void android_canvas::set_antialias(bool value)
    {
        jmethodID set_aa = default_jni_cache().method(env_, k_paint_class, "setAntiAlias", "(Z)V");
        if (set_aa == nullptr)
        {
            return;
        }
        if (fill_paint_ != nullptr)
        {
            env_->CallVoidMethod(fill_paint_, set_aa, static_cast<jboolean>(value));
            clear_pending(env_);
        }
        if (stroke_paint_ != nullptr)
        {
            env_->CallVoidMethod(stroke_paint_, set_aa, static_cast<jboolean>(value));
            clear_pending(env_);
        }
    }

    void android_canvas::set_blend_mode(maui::graphics::blend_mode /*value*/)
    {
        // android.graphics.Paint.setBlendMode is API 29+ and no shape gallery page sets a blend mode; the
        // common case (normal) is the Paint default. // TODO: verify against PlatformCanvas.Android
        // (Xfermode/BlendMode).
    }

    // ---- the path conversion seam ----
    jobject android_canvas::build_path(const maui::graphics::path_f& path)
    {
        if (path_ == nullptr)
        {
            return nullptr;
        }
        auto& cache = default_jni_cache();
        jmethodID rewind = cache.method(env_, k_path_class, "rewind", "()V");
        jmethodID move_to = cache.method(env_, k_path_class, "moveTo", "(FF)V");
        jmethodID line_to = cache.method(env_, k_path_class, "lineTo", "(FF)V");
        jmethodID quad_to = cache.method(env_, k_path_class, "quadTo", "(FFFF)V");
        jmethodID cubic_to = cache.method(env_, k_path_class, "cubicTo", "(FFFFFF)V");
        jmethodID close = cache.method(env_, k_path_class, "close", "()V");
        jmethodID arc_to = cache.method(env_, k_path_class, "arcTo", "(Landroid/graphics/RectF;FFZ)V");
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        if (rewind == nullptr || move_to == nullptr || line_to == nullptr || quad_to == nullptr ||
            cubic_to == nullptr || close == nullptr)
        {
            return nullptr;
        }
        env_->CallVoidMethod(path_, rewind);
        clear_pending(env_);

        // The walk mirrors apple_shared coregraphics_canvas's path_to_cg_path (already a 1:1 of
        // GraphicsExtensions.AsCGPath / the android PathExtensions): operations + a running point index.
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
                    env_->CallVoidMethod(path_, move_to, static_cast<jfloat>(p.x), static_cast<jfloat>(p.y));
                    break;
                }
                case maui::graphics::path_operation::line: {
                    const maui::graphics::point_f p = path[point_index++];
                    env_->CallVoidMethod(path_, line_to, static_cast<jfloat>(p.x), static_cast<jfloat>(p.y));
                    break;
                }
                case maui::graphics::path_operation::quad: {
                    const maui::graphics::point_f control = path[point_index++];
                    const maui::graphics::point_f end = path[point_index++];
                    env_->CallVoidMethod(path_, quad_to, static_cast<jfloat>(control.x), static_cast<jfloat>(control.y),
                                         static_cast<jfloat>(end.x), static_cast<jfloat>(end.y));
                    break;
                }
                case maui::graphics::path_operation::cubic: {
                    const maui::graphics::point_f c1 = path[point_index++];
                    const maui::graphics::point_f c2 = path[point_index++];
                    const maui::graphics::point_f end = path[point_index++];
                    env_->CallVoidMethod(path_, cubic_to, static_cast<jfloat>(c1.x), static_cast<jfloat>(c1.y),
                                         static_cast<jfloat>(c2.x), static_cast<jfloat>(c2.y),
                                         static_cast<jfloat>(end.x), static_cast<jfloat>(end.y));
                    break;
                }
                case maui::graphics::path_operation::arc: {
                    const maui::graphics::point_f top_left = path[point_index++];
                    const maui::graphics::point_f bottom_right = path[point_index++];
                    const float start_angle = path.get_arc_angle(arc_angle_index++);
                    const float end_angle = path.get_arc_angle(arc_angle_index++);
                    const bool clockwise = path.get_arc_clockwise(arc_clockwise_index++);
                    if (arc_to != nullptr && set_rectf != nullptr && rectf_ != nullptr)
                    {
                        // android Path.arcTo(oval, startAngle, sweepAngle): the oval is the arc's bounding
                        // box; android angles are clockwise from +x (degrees), the framework's are
                        // counter-clockwise, so negate (matching the CoreGraphics -start/-end mapping).
                        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(top_left.x),
                                             static_cast<jfloat>(top_left.y), static_cast<jfloat>(bottom_right.x),
                                             static_cast<jfloat>(bottom_right.y));
                        clear_pending(env_);
                        float sweep = -(end_angle - start_angle);
                        if (!clockwise && sweep > 0)
                        {
                            sweep -= 360.0F;
                        }
                        else if (clockwise && sweep < 0)
                        {
                            sweep += 360.0F;
                        }
                        env_->CallVoidMethod(path_, arc_to, rectf_, static_cast<jfloat>(-start_angle),
                                             static_cast<jfloat>(sweep), static_cast<jboolean>(false));
                    }
                    break;
                }
                case maui::graphics::path_operation::close:
                    env_->CallVoidMethod(path_, close);
                    break;
            }
            clear_pending(env_);
        }
        return path_;
    }

    // ---- stroked draws (the abstract_canvas platform hooks) ----
    void android_canvas::platform_set_stroke_size(float value)
    {
        if (stroke_paint_ == nullptr)
        {
            return;
        }
        jmethodID set_width = default_jni_cache().method(env_, k_paint_class, "setStrokeWidth", "(F)V");
        if (set_width != nullptr)
        {
            env_->CallVoidMethod(stroke_paint_, set_width, static_cast<jfloat>(value));
            clear_pending(env_);
        }
    }

    void android_canvas::platform_set_stroke_dash_pattern(const std::vector<float>& pattern, float stroke_dash_offset,
                                                          float stroke_size)
    {
        if (stroke_paint_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_effect = cache.method(env_, k_paint_class, "setPathEffect",
                                            "(Landroid/graphics/PathEffect;)Landroid/graphics/PathEffect;");
        if (set_effect == nullptr)
        {
            return;
        }
        // C# PlatformSetStrokeDashPattern: a null/empty pattern clears the effect; otherwise the dash
        // lengths scale by the stroke size (matching the apple backend + DashPathEffect's pixel lengths).
        if (pattern.empty())
        {
            env_->CallObjectMethod(stroke_paint_, set_effect, static_cast<jobject>(nullptr));
            clear_pending(env_);
            return;
        }
        // DashPathEffect needs an even-length interval array; duplicate a lone entry (on==off).
        std::vector<jfloat> intervals;
        intervals.reserve(pattern.size() * 2);
        for (const float dash : pattern)
        {
            intervals.push_back(static_cast<jfloat>(dash * stroke_size));
        }
        if ((intervals.size() % 2) != 0)
        {
            intervals.insert(intervals.end(), intervals.begin(), intervals.end());
        }
        const local_ref<jfloatArray> array{env_, env_->NewFloatArray(static_cast<jsize>(intervals.size()))};
        if (clear_pending(env_) || !array)
        {
            return;
        }
        env_->SetFloatArrayRegion(array.get(), 0, static_cast<jsize>(intervals.size()), intervals.data());
        clear_pending(env_);
        jclass effect_class = cache.find_class(env_, k_dash_path_effect_class);
        jmethodID effect_ctor = cache.method(env_, k_dash_path_effect_class, "<init>", "([FF)V");
        if (effect_class == nullptr || effect_ctor == nullptr)
        {
            return;
        }
        const local_ref<jobject> effect{env_, env_->NewObject(effect_class, effect_ctor, array.get(),
                                                              static_cast<jfloat>(stroke_dash_offset * stroke_size))};
        if (clear_pending(env_) || !effect)
        {
            return;
        }
        env_->CallObjectMethod(stroke_paint_, set_effect, effect.get());
        clear_pending(env_);
    }

    void android_canvas::platform_draw_line(float x1, float y1, float x2, float y2)
    {
        if (canvas_ == nullptr || stroke_paint_ == nullptr)
        {
            return;
        }
        jmethodID draw_line =
            default_jni_cache().method(env_, k_canvas_class, "drawLine", "(FFFFLandroid/graphics/Paint;)V");
        if (draw_line != nullptr)
        {
            env_->CallVoidMethod(canvas_, draw_line, static_cast<jfloat>(x1), static_cast<jfloat>(y1),
                                 static_cast<jfloat>(x2), static_cast<jfloat>(y2), stroke_paint_);
            clear_pending(env_);
        }
    }

    void android_canvas::platform_draw_arc(float x, float y, float width, float height, float start_angle,
                                           float end_angle, bool clockwise, bool closed)
    {
        if (canvas_ == nullptr || stroke_paint_ == nullptr || rectf_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        jmethodID draw_arc =
            cache.method(env_, k_canvas_class, "drawArc", "(Landroid/graphics/RectF;FFZLandroid/graphics/Paint;)V");
        if (set_rectf == nullptr || draw_arc == nullptr)
        {
            return;
        }
        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(x), static_cast<jfloat>(y),
                             static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
        clear_pending(env_);
        // Framework angles are counter-clockwise from +x; android's drawArc is clockwise — negate start,
        // and compute the signed sweep (the apple backend negates both angles likewise).
        float sweep = -(end_angle - start_angle);
        if (!clockwise && sweep > 0)
        {
            sweep -= 360.0F;
        }
        else if (clockwise && sweep < 0)
        {
            sweep += 360.0F;
        }
        env_->CallVoidMethod(canvas_, draw_arc, rectf_, static_cast<jfloat>(-start_angle), static_cast<jfloat>(sweep),
                             static_cast<jboolean>(closed), stroke_paint_);
        clear_pending(env_);
    }

    void android_canvas::platform_draw_rectangle(float x, float y, float width, float height)
    {
        if (canvas_ == nullptr || stroke_paint_ == nullptr)
        {
            return;
        }
        jmethodID draw_rect =
            default_jni_cache().method(env_, k_canvas_class, "drawRect", "(FFFFLandroid/graphics/Paint;)V");
        if (draw_rect != nullptr)
        {
            env_->CallVoidMethod(canvas_, draw_rect, static_cast<jfloat>(x), static_cast<jfloat>(y),
                                 static_cast<jfloat>(x + width), static_cast<jfloat>(y + height), stroke_paint_);
            clear_pending(env_);
        }
    }

    void android_canvas::platform_draw_rounded_rectangle(float x, float y, float width, float height,
                                                         float corner_radius)
    {
        if (canvas_ == nullptr || stroke_paint_ == nullptr || rectf_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        jmethodID draw_round = cache.method(env_, k_canvas_class, "drawRoundRect",
                                            "(Landroid/graphics/RectF;FFLandroid/graphics/Paint;)V");
        if (set_rectf == nullptr || draw_round == nullptr)
        {
            return;
        }
        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(x), static_cast<jfloat>(y),
                             static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
        clear_pending(env_);
        env_->CallVoidMethod(canvas_, draw_round, rectf_, static_cast<jfloat>(corner_radius),
                             static_cast<jfloat>(corner_radius), stroke_paint_);
        clear_pending(env_);
    }

    void android_canvas::platform_draw_ellipse(float x, float y, float width, float height)
    {
        if (canvas_ == nullptr || stroke_paint_ == nullptr || rectf_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        jmethodID draw_oval =
            cache.method(env_, k_canvas_class, "drawOval", "(Landroid/graphics/RectF;Landroid/graphics/Paint;)V");
        if (set_rectf == nullptr || draw_oval == nullptr)
        {
            return;
        }
        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(x), static_cast<jfloat>(y),
                             static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
        clear_pending(env_);
        env_->CallVoidMethod(canvas_, draw_oval, rectf_, stroke_paint_);
        clear_pending(env_);
    }

    void android_canvas::platform_draw_path(const maui::graphics::path_f& path)
    {
        if (canvas_ == nullptr || stroke_paint_ == nullptr)
        {
            return;
        }
        jobject path_obj = build_path(path);
        if (path_obj == nullptr)
        {
            return;
        }
        jmethodID draw_path = default_jni_cache().method(env_, k_canvas_class, "drawPath",
                                                         "(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
        if (draw_path != nullptr)
        {
            env_->CallVoidMethod(canvas_, draw_path, path_obj, stroke_paint_);
            clear_pending(env_);
        }
    }

    void android_canvas::platform_draw_image(const maui::graphics::i_graphics_image& /*image*/, float /*x*/,
                                             float /*y*/, float /*width*/, float /*height*/)
    {
        // Deferred — no shape page blits an image. // TODO: android.graphics.Canvas.drawBitmap.
    }

    // ---- transforms: the live Canvas matrix mirrors abstract_canvas's tracked transform ----
    void android_canvas::platform_rotate(float degrees, float /*radians*/, float x, float y)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        jmethodID rotate = default_jni_cache().method(env_, k_canvas_class, "rotate", "(FFF)V");
        if (rotate != nullptr)
        {
            env_->CallVoidMethod(canvas_, rotate, static_cast<jfloat>(degrees), static_cast<jfloat>(x),
                                 static_cast<jfloat>(y));
            clear_pending(env_);
        }
    }

    void android_canvas::platform_rotate(float degrees, float /*radians*/)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        jmethodID rotate = default_jni_cache().method(env_, k_canvas_class, "rotate", "(F)V");
        if (rotate != nullptr)
        {
            env_->CallVoidMethod(canvas_, rotate, static_cast<jfloat>(degrees));
            clear_pending(env_);
        }
    }

    void android_canvas::platform_scale(float sx, float sy)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        jmethodID scale = default_jni_cache().method(env_, k_canvas_class, "scale", "(FF)V");
        if (scale != nullptr)
        {
            env_->CallVoidMethod(canvas_, scale, static_cast<jfloat>(sx), static_cast<jfloat>(sy));
            clear_pending(env_);
        }
    }

    void android_canvas::platform_translate(float tx, float ty)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        jmethodID translate = default_jni_cache().method(env_, k_canvas_class, "translate", "(FF)V");
        if (translate != nullptr)
        {
            env_->CallVoidMethod(canvas_, translate, static_cast<jfloat>(tx), static_cast<jfloat>(ty));
            clear_pending(env_);
        }
    }

    void android_canvas::platform_concatenate_transform(const maui::graphics::matrix3x2& transform)
    {
        // android.graphics.Canvas.concat(Matrix); the 3x3 Matrix's 9 values are row-major
        // [scaleX skewX transX, skewY scaleY transY, 0 0 1] = [m11 m21 m31, m12 m22 m32, 0 0 1].
        if (canvas_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jclass matrix_class = cache.find_class(env_, "android/graphics/Matrix");
        jmethodID matrix_ctor = cache.method(env_, "android/graphics/Matrix", "<init>", "()V");
        jmethodID set_values = cache.method(env_, "android/graphics/Matrix", "setValues", "([F)V");
        jmethodID concat = cache.method(env_, k_canvas_class, "concat", "(Landroid/graphics/Matrix;)V");
        if (matrix_class == nullptr || matrix_ctor == nullptr || set_values == nullptr || concat == nullptr)
        {
            return;
        }
        const std::array<jfloat, 9> values{transform.m11, transform.m21, transform.m31, transform.m12, transform.m22,
                                           transform.m32, 0.0F,          0.0F,          1.0F};
        const local_ref<jfloatArray> array{env_, env_->NewFloatArray(9)};
        if (clear_pending(env_) || !array)
        {
            return;
        }
        env_->SetFloatArrayRegion(array.get(), 0, 9, values.data());
        const local_ref<jobject> matrix{env_, env_->NewObject(matrix_class, matrix_ctor)};
        if (clear_pending(env_) || !matrix)
        {
            return;
        }
        env_->CallVoidMethod(matrix.get(), set_values, array.get());
        clear_pending(env_);
        env_->CallVoidMethod(canvas_, concat, matrix.get());
        clear_pending(env_);
    }

    // ---- fills ----
    void android_canvas::fill_built_path(jobject path_obj, maui::graphics::winding_mode winding)
    {
        if (canvas_ == nullptr || fill_paint_ == nullptr || path_obj == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        // Path.setFillType(WINDING|EVEN_ODD) carries the fill rule (the apple backend uses EOFill; android
        // expresses the same on the Path itself).
        const char* fill_type = winding == maui::graphics::winding_mode::even_odd ? "EVEN_ODD" : "WINDING";
        const local_ref<jobject> type =
            static_enum(env_, k_path_filltype_class, fill_type, "Landroid/graphics/Path$FillType;");
        jmethodID set_fill_type =
            cache.method(env_, k_path_class, "setFillType", "(Landroid/graphics/Path$FillType;)V");
        if (set_fill_type != nullptr && type)
        {
            env_->CallVoidMethod(path_obj, set_fill_type, type.get());
            clear_pending(env_);
        }
        jmethodID draw_path =
            cache.method(env_, k_canvas_class, "drawPath", "(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
        if (draw_path != nullptr)
        {
            env_->CallVoidMethod(canvas_, draw_path, path_obj, fill_paint_);
            clear_pending(env_);
        }
    }

    void android_canvas::fill_path(const maui::graphics::path_f& path, maui::graphics::winding_mode winding)
    {
        jobject path_obj = build_path(path);
        fill_built_path(path_obj, winding);
    }

    void android_canvas::fill_rectangle(float x, float y, float width, float height)
    {
        if (canvas_ == nullptr || fill_paint_ == nullptr)
        {
            return;
        }
        jmethodID draw_rect =
            default_jni_cache().method(env_, k_canvas_class, "drawRect", "(FFFFLandroid/graphics/Paint;)V");
        if (draw_rect != nullptr)
        {
            env_->CallVoidMethod(canvas_, draw_rect, static_cast<jfloat>(x), static_cast<jfloat>(y),
                                 static_cast<jfloat>(x + width), static_cast<jfloat>(y + height), fill_paint_);
            clear_pending(env_);
        }
    }

    void android_canvas::fill_rounded_rectangle(float x, float y, float width, float height, float corner_radius)
    {
        if (canvas_ == nullptr || fill_paint_ == nullptr || rectf_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        jmethodID draw_round = cache.method(env_, k_canvas_class, "drawRoundRect",
                                            "(Landroid/graphics/RectF;FFLandroid/graphics/Paint;)V");
        if (set_rectf == nullptr || draw_round == nullptr)
        {
            return;
        }
        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(x), static_cast<jfloat>(y),
                             static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
        clear_pending(env_);
        env_->CallVoidMethod(canvas_, draw_round, rectf_, static_cast<jfloat>(corner_radius),
                             static_cast<jfloat>(corner_radius), fill_paint_);
        clear_pending(env_);
    }

    void android_canvas::fill_ellipse(float x, float y, float width, float height)
    {
        if (canvas_ == nullptr || fill_paint_ == nullptr || rectf_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        jmethodID draw_oval =
            cache.method(env_, k_canvas_class, "drawOval", "(Landroid/graphics/RectF;Landroid/graphics/Paint;)V");
        if (set_rectf == nullptr || draw_oval == nullptr)
        {
            return;
        }
        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(x), static_cast<jfloat>(y),
                             static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
        clear_pending(env_);
        env_->CallVoidMethod(canvas_, draw_oval, rectf_, fill_paint_);
        clear_pending(env_);
    }

    void android_canvas::fill_arc(float x, float y, float width, float height, float start_angle, float end_angle,
                                  bool clockwise)
    {
        if (canvas_ == nullptr || fill_paint_ == nullptr || rectf_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        jmethodID set_rectf = cache.method(env_, k_rectf_class, "set", "(FFFF)V");
        jmethodID draw_arc =
            cache.method(env_, k_canvas_class, "drawArc", "(Landroid/graphics/RectF;FFZLandroid/graphics/Paint;)V");
        if (set_rectf == nullptr || draw_arc == nullptr)
        {
            return;
        }
        env_->CallVoidMethod(rectf_, set_rectf, static_cast<jfloat>(x), static_cast<jfloat>(y),
                             static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
        clear_pending(env_);
        float sweep = -(end_angle - start_angle);
        if (!clockwise && sweep > 0)
        {
            sweep -= 360.0F;
        }
        else if (clockwise && sweep < 0)
        {
            sweep += 360.0F;
        }
        // A filled arc is a pie slice (useCenter = true).
        env_->CallVoidMethod(canvas_, draw_arc, rectf_, static_cast<jfloat>(-start_angle), static_cast<jfloat>(sweep),
                             static_cast<jboolean>(true), fill_paint_);
        clear_pending(env_);
    }

    // ---- clipping ----
    void android_canvas::clip_path(const maui::graphics::path_f& path, maui::graphics::winding_mode winding)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        jobject path_obj = build_path(path);
        if (path_obj == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        const char* fill_type = winding == maui::graphics::winding_mode::even_odd ? "EVEN_ODD" : "WINDING";
        const local_ref<jobject> type =
            static_enum(env_, k_path_filltype_class, fill_type, "Landroid/graphics/Path$FillType;");
        jmethodID set_fill_type =
            cache.method(env_, k_path_class, "setFillType", "(Landroid/graphics/Path$FillType;)V");
        if (set_fill_type != nullptr && type)
        {
            env_->CallVoidMethod(path_obj, set_fill_type, type.get());
            clear_pending(env_);
        }
        jmethodID clip = cache.method(env_, k_canvas_class, "clipPath", "(Landroid/graphics/Path;)Z");
        if (clip != nullptr)
        {
            env_->CallBooleanMethod(canvas_, clip, path_obj);
            clear_pending(env_);
        }
    }

    void android_canvas::clip_rectangle(float x, float y, float width, float height)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        jmethodID clip = default_jni_cache().method(env_, k_canvas_class, "clipRect", "(FFFF)Z");
        if (clip != nullptr)
        {
            env_->CallBooleanMethod(canvas_, clip, static_cast<jfloat>(x), static_cast<jfloat>(y),
                                    static_cast<jfloat>(x + width), static_cast<jfloat>(y + height));
            clear_pending(env_);
        }
    }

    void android_canvas::subtract_from_clip(float x, float y, float width, float height)
    {
        if (canvas_ == nullptr)
        {
            return;
        }
        auto& cache = default_jni_cache();
        // C# SubtractFromClip removes the inner rect from the current clip. android's clipRect(rect, Op)
        // with Region.Op.DIFFERENCE expresses exactly that (the Op overload is deprecated on API 26+ but
        // present + functional; no shape page exercises this, so it stays the faithful mapping).
        const local_ref<jobject> op =
            static_enum(env_, k_region_op_class, "DIFFERENCE", "Landroid/graphics/Region$Op;");
        jmethodID clip = cache.method(env_, k_canvas_class, "clipRect", "(FFFFLandroid/graphics/Region$Op;)Z");
        if (clip != nullptr && op)
        {
            env_->CallBooleanMethod(canvas_, clip, static_cast<jfloat>(x), static_cast<jfloat>(y),
                                    static_cast<jfloat>(x + width), static_cast<jfloat>(y + height), op.get());
            clear_pending(env_);
        }
    }

    // ---- graphics state: the Canvas's own save/restore mirrors abstract_canvas's stack ----
    void android_canvas::save_state()
    {
        abstract_canvas::save_state();
        if (canvas_ == nullptr)
        {
            return;
        }
        jmethodID save = default_jni_cache().method(env_, k_canvas_class, "save", "()I");
        if (save != nullptr)
        {
            env_->CallIntMethod(canvas_, save);
            clear_pending(env_);
        }
    }

    bool android_canvas::restore_state()
    {
        const bool restored = abstract_canvas::restore_state();
        if (canvas_ != nullptr)
        {
            jmethodID restore = default_jni_cache().method(env_, k_canvas_class, "restore", "()V");
            if (restore != nullptr)
            {
                env_->CallVoidMethod(canvas_, restore);
                clear_pending(env_);
            }
        }
        return restored;
    }

    void android_canvas::reset_state()
    {
        abstract_canvas::reset_state();
        if (canvas_ == nullptr)
        {
            return;
        }
        // Unwind the Canvas's own stack to the base save count (Canvas.restoreToCount(1) keeps the device
        // scale established at construction). Best-effort: a failure leaves the matrix as-is.
        jmethodID restore_to_count = default_jni_cache().method(env_, k_canvas_class, "restoreToCount", "(I)V");
        if (restore_to_count != nullptr)
        {
            env_->CallVoidMethod(canvas_, restore_to_count, static_cast<jint>(1));
            clear_pending(env_);
        }
    }

    void android_canvas::set_shadow(const maui::graphics::size_f& /*offset*/, float /*blur*/,
                                    const maui::graphics::color& /*shadow_color*/)
    {
        // Paint.setShadowLayer is the analog; no shape gallery page casts a canvas shadow (shadows are an
        // IView property routed elsewhere). // TODO: verify against PlatformCanvas.Android (setShadowLayer).
    }

    void android_canvas::set_fill_paint(const maui::graphics::paint* fill_paint, const maui::graphics::rect_f& rect)
    {
        // Ports PlatformCanvas.Android.SetFillPaint (src/Graphics/.../Platforms/Android/PlatformCanvas.cs).
        // C# SetFillPaint(null) -> a solid white fill. A solid paint maps to its color (apply_fill_color
        // clears any prior shader). A linear/radial gradient mints a matching Shader on the fill Paint; a
        // shader-ctor / JNI failure falls to the paint's blended start/end color (C#'s catch → FillColor =
        // BlendStartAndEndColors). Pattern/image remain a flat background_color fall-back (header note).
        if (fill_paint == nullptr)
        {
            apply_fill_color(maui::graphics::color(1, 1, 1, 1));
            return;
        }
        if (const auto* const solid = dynamic_cast<const maui::graphics::solid_paint*>(fill_paint))
        {
            apply_fill_color(solid->color());
            return;
        }

        if (const auto* const linear = dynamic_cast<const maui::graphics::linear_gradient_paint*>(fill_paint))
        {
            // C#: x1 = StartPoint.X * rect.Width + rect.X (etc.) — the relative endpoints scaled into the
            // fill rectangle's POINT space (the canvas CTM maps points→pixels, like the path coords).
            const auto x1 = static_cast<jfloat>((linear->start_point().x * rect.width) + rect.x);
            const auto y1 = static_cast<jfloat>((linear->start_point().y * rect.height) + rect.y);
            const auto x2 = static_cast<jfloat>((linear->end_point().x * rect.width) + rect.x);
            const auto y2 = static_cast<jfloat>((linear->end_point().y * rect.height) + rect.y);
            const gradient_arrays arrays = build_gradient_arrays(env_, *linear);
            const local_ref<jobject> clamp = clamp_tile_mode(env_);
            jclass shader_class = default_jni_cache().find_class(env_, k_linear_gradient_class);
            jmethodID shader_ctor = default_jni_cache().method(env_, k_linear_gradient_class, "<init>",
                                                               "(FFFF[I[FLandroid/graphics/Shader$TileMode;)V");
            if (arrays.colors && arrays.stops && clamp && shader_class != nullptr && shader_ctor != nullptr)
            {
                const local_ref<jobject> shader{env_,
                                                env_->NewObject(shader_class, shader_ctor, x1, y1, x2, y2,
                                                                arrays.colors.get(), arrays.stops.get(), clamp.get())};
                if (!clear_pending(env_) && shader)
                {
                    // C# sets FillColor = White before staging the shader; a solid color also clears any
                    // prior shader, so set the white base first, THEN install the gradient over it.
                    apply_fill_color(maui::graphics::colors::white);
                    set_fill_shader(shader.get());
                    return;
                }
            }
            apply_fill_color(linear->blend_start_and_end_colors()); // C# catch fall-back
            return;
        }

        if (const auto* const radial = dynamic_cast<const maui::graphics::radial_gradient_paint*>(fill_paint))
        {
            // C#: center = Center.{X,Y} * rect.{Width,Height} + rect.{X,Y}; radius = Radius * max(W,H), or
            // the rect diagonal when the radius is non-positive (GeometryUtil.GetDistance corner-to-corner).
            const auto cx = static_cast<jfloat>((radial->center().x * rect.width) + rect.x);
            const auto cy = static_cast<jfloat>((radial->center().y * rect.height) + rect.y);
            auto radius =
                static_cast<jfloat>(radial->radius() * static_cast<double>(std::max(rect.height, rect.width)));
            if (!(radius > 0.0F))
            {
                radius = std::hypot(rect.width, rect.height);
            }
            const gradient_arrays arrays = build_gradient_arrays(env_, *radial);
            const local_ref<jobject> clamp = clamp_tile_mode(env_);
            jclass shader_class = default_jni_cache().find_class(env_, k_radial_gradient_class);
            jmethodID shader_ctor = default_jni_cache().method(env_, k_radial_gradient_class, "<init>",
                                                               "(FFF[I[FLandroid/graphics/Shader$TileMode;)V");
            if (arrays.colors && arrays.stops && clamp && shader_class != nullptr && shader_ctor != nullptr)
            {
                const local_ref<jobject> shader{env_,
                                                env_->NewObject(shader_class, shader_ctor, cx, cy, radius,
                                                                arrays.colors.get(), arrays.stops.get(), clamp.get())};
                if (!clear_pending(env_) && shader)
                {
                    apply_fill_color(maui::graphics::colors::white);
                    set_fill_shader(shader.get());
                    return;
                }
            }
            apply_fill_color(radial->blend_start_and_end_colors()); // C# catch fall-back
            return;
        }

        // pattern / image — flat fallback. // TODO: image/pattern fills via android BitmapShader.
        apply_fill_color(fill_paint->background_color());
    }

    // ---- text (deferred — no shape renders text; header note) ----
    void android_canvas::draw_string(std::string_view /*value*/, float /*x*/, float /*y*/,
                                     maui::graphics::horizontal_alignment /*h_align*/)
    {
    }

    void android_canvas::draw_string(std::string_view /*value*/, float /*x*/, float /*y*/, float /*width*/,
                                     float /*height*/, maui::graphics::horizontal_alignment /*h_align*/,
                                     maui::graphics::vertical_alignment /*v_align*/, maui::graphics::text_flow /*flow*/,
                                     float /*line_spacing_adjustment*/)
    {
    }

    void android_canvas::draw_text(const maui::graphics::text::i_attributed_text& /*value*/, float /*x*/, float /*y*/,
                                   float /*width*/, float /*height*/)
    {
    }

    maui::graphics::size_f android_canvas::get_string_size(std::string_view /*value*/,
                                                           const maui::graphics::font& /*font*/,
                                                           float /*font_size*/) const
    {
        return {0, 0};
    }

    maui::graphics::size_f android_canvas::get_string_size(std::string_view /*value*/,
                                                           const maui::graphics::font& /*font*/, float /*font_size*/,
                                                           maui::graphics::horizontal_alignment /*h_align*/,
                                                           maui::graphics::vertical_alignment /*v_align*/) const
    {
        return {0, 0};
    }
} // namespace maui::platform::android
