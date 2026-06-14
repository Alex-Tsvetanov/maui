#pragma once
// Shared Android (JNI) operations for the generic-IView accessibility metadata (Semantics) — the
// platform side of the shared view_mapper's map_semantics (view_mapper.cpp). The JNI twin of
// apple_semantics_ops.hpp's apply_semantics. Include only from the android partials. VM-less safe
// (acquires a scoped_env and returns when no JavaVM / no widget exists — the headless-mirror
// degradation the android partials document).
//
// apply_semantics ports Microsoft.Maui.Platform.SemanticExtensions (Android,
// src/Core/src/Platform/Android/SemanticExtensions.cs) + ViewHandler.Android.cs MappingSemantics +
// SemanticExtensions.UpdateSemanticNodeInfo, distilled to the plain-android.view.View surface:
//   - a null Semantics is a NO-OP (C# MappingSemantics / UpdateSemantics return early on a null
//     Semantics — they do not clear the description/heading);
//   - Description maps to View.setContentDescription (UpdateSemanticNodeInfo sets
//     info.ContentDescription = desc for a non-EditText view; the View's contentDescription is the
//     plain-View carrier of that node-info field, and TalkBack reads it as the accessible name);
//   - IsHeading maps to View.setAccessibilityHeading(boolean) (the framework API the C#
//     ViewCompat.SetAccessibilityHeading delegates to on API 28+, exactly what UpdateSemantics does);
//   - setting a non-blank ContentDescription flips ImportantForAccessibility to YES (so the description
//     is actually surfaced), then we DO NOT restore AUTO — C# leaves it YES whenever a desc/hint is set
//     (MappingSemantics sets ImportantForAccessibility.Yes in that branch). A blank description clears
//     the contentDescription back to null (the AutomationId-vs-Semantics interplay below).
//
// DOCUMENTED DEVIATIONS from the C# Android oracle (each an infrastructure gap, not a behavior guess):
//   - Hint has NO plain-android.view.View setter: C# surfaces it via info.HintText on the
//     AccessibilityNodeInfo, filled by a MauiAccessibilityDelegateCompat installed on the View during
//     the accessibility traversal (a host-provided Java subclass, like NativeOnClickListener). This
//     APK-less backend carries no such delegate class, so Hint is kept ONLY in the headless mirror (the
//     base body) — matching exactly what a View shows when no delegate is installed. (Wiring a hint
//     delegate is the accessibility-delegate fan-out, paralleling the click-listener trampoline.)
//   - C#'s AccessibilityHeading goes through ViewCompat (AndroidX), which back-ports the flag below
//     API 28. The framework View.setAccessibilityHeading is API 28+; the test emulator is android-34,
//     so the framework setter is used directly (no AndroidX), the same flag ViewCompat sets.
//   - ImportantForAccessibility is left at YES after a description (no restore-to-AUTO), unlike the
//     AutomationId path (PlatformInterop.setContentDescriptionForAutomationId restores AUTO): C#'s
//     MappingSemantics deliberately keeps YES so the Semantics description is announced.

#include <jni.h>

#include <string_view>

#include "jni/jni_cache.hpp"
#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"
#include "maui/core/semantics.hpp"

namespace maui::platform::android
{
    namespace detail
    {
        inline constexpr const char* k_semantics_view_class = "android/view/View";

        // android.view.View.IMPORTANT_FOR_ACCESSIBILITY_YES (MappingSemantics' target when a desc/hint
        // is present, so the description is announced).
        inline constexpr jint k_important_for_accessibility_yes = 1;

        // True when a string is blank (empty or only whitespace) — C#'s string.IsNullOrWhiteSpace gate.
        [[nodiscard]] inline bool is_blank(std::string_view value)
        {
            return value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos;
        }
    } // namespace detail

    // Push a Semantics borrow (null = leave as-is) onto the native View's accessibility surface.
    inline void apply_semantics(void* native, const maui::core::semantics* value)
    {
        if (native == nullptr || value == nullptr)
        {
            return; // C#: a null Semantics leaves the platform accessibility properties untouched
        }
        const scoped_env env;
        if (!env)
        {
            return;
        }
        auto* const view = static_cast<jobject>(native);
        auto& cache = default_jni_cache();

        // Description → contentDescription (UpdateSemanticNodeInfo's info.ContentDescription = desc).
        const std::string& description = value->description();
        jmethodID set_description = cache.method(env.get(), detail::k_semantics_view_class, "setContentDescription",
                                                 "(Ljava/lang/CharSequence;)V");
        if (set_description != nullptr)
        {
            if (detail::is_blank(description))
            {
                // A blank description clears the carrier (the AutomationId-vs-Semantics interplay:
                // UpdateSemanticNodeInfo nulls the contentDescription when no desc remains).
                env->CallVoidMethod(view, set_description, static_cast<jobject>(nullptr));
                env->ExceptionClear();
            }
            else
            {
                const local_ref<jstring> desc = to_jstring(env.get(), description);
                env->CallVoidMethod(view, set_description, desc.get());
                env->ExceptionClear();
                // MappingSemantics: a present desc/hint flips ImportantForAccessibility to YES (and
                // leaves it there, unlike the AutomationId path) so the description is announced.
                if (jmethodID set_important =
                        cache.method(env.get(), detail::k_semantics_view_class, "setImportantForAccessibility", "(I)V"))
                {
                    env->CallVoidMethod(view, set_important, detail::k_important_for_accessibility_yes);
                    env->ExceptionClear();
                }
            }
        }

        // IsHeading → View.setAccessibilityHeading(boolean) (UpdateSemantics →
        // ViewCompat.SetAccessibilityHeading; the framework setter is API 28+, available on android-34).
        jmethodID set_heading =
            cache.method(env.get(), detail::k_semantics_view_class, "setAccessibilityHeading", "(Z)V");
        if (set_heading != nullptr)
        {
            env->CallVoidMethod(view, set_heading, static_cast<jboolean>(value->is_heading()));
            env->ExceptionClear();
        }
    }
} // namespace maui::platform::android
