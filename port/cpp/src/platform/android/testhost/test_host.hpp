#pragma once
// maui::platform::android::testhost — the widget test-host seam. Internal test infrastructure for
// the Android backend, NOT a ported MAUI type (the C# counterpart is the device-test runner MAUI
// hosts its Android DeviceTests in; this host replaces it with app_process + JNI, see
// tools/android-testhost-run.sh).
//
// test_host.cpp implements Bootstrap.nativeRun: it pins the process JavaVM (jni_env.hpp) and the
// bootstrap's themed android.content.Context, then runs the embedded gtest suite. Widget test cases
// reach that Context through host_context() to construct real android.widget views.

#include <jni.h>

namespace maui::platform::android::testhost
{
    // The themed android.content.Context minted by testhost/Bootstrap.java (a process-lifetime
    // global reference, valid from any attached thread — deliberately leaked at exit like the
    // jni_cache pins). nullptr when the process is not the widget test host.
    [[nodiscard]] jobject host_context() noexcept;
} // namespace maui::platform::android::testhost
