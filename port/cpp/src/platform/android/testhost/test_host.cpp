// maui::platform::android::testhost — the native side of the app_process widget test host
// (M-android milestone 3). testhost/Bootstrap.java System.load()s the test library and calls
// nativeRun(themedContext, gtestArgs): this entry pins the JavaVM for the JNI seam (jni_env.hpp),
// pins the Context for host_context(), rebuilds argv for InitGoogleTest (so --gtest_filter /
// --gtest_output flow through the lane like any gtest binary), and returns RUN_ALL_TESTS()'s exit
// code — which app_process surfaces to adb shell-v2 via System.exit.

#include "test_host.hpp"

#include <jni.h>

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "jni/jni_env.hpp"
#include "jni/jni_ref.hpp"
#include "jni/jni_string.hpp"

namespace maui::platform::android::testhost
{
    namespace detail
    {
        jobject& host_context_slot() noexcept
        {
            static jobject slot = nullptr;
            return slot;
        }
    } // namespace detail

    jobject host_context() noexcept
    {
        return detail::host_context_slot();
    }
} // namespace maui::platform::android::testhost

extern "C" JNIEXPORT jint JNICALL Java_dev_mauicpp_testhost_Bootstrap_nativeRun(JNIEnv* env, jclass /*bootstrap*/,
                                                                                jobject context, jobjectArray args)
{
    namespace android = maui::platform::android;

    JavaVM* vm = nullptr;
    if (env->GetJavaVM(&vm) == JNI_OK)
    {
        android::set_java_vm(vm); // global_ref teardown + scoped_env need it
    }
    // Pin the bootstrap's themed Context for the process lifetime (the host exits via System.exit
    // right after this returns, so the reference is deliberately never released).
    android::testhost::detail::host_context_slot() = env->NewGlobalRef(context);

    // Rebuild argc/argv: gtest expects a program name at argv[0]; the Java side forwards only the
    // real arguments. The storage outlives both InitGoogleTest and RUN_ALL_TESTS (same scope).
    std::vector<std::string> arg_storage;
    arg_storage.emplace_back("maui_android_widget_tests");
    const jsize count = args == nullptr ? 0 : env->GetArrayLength(args);
    for (jsize i = 0; i < count; ++i)
    {
        const android::local_ref<jstring> arg{env, static_cast<jstring>(env->GetObjectArrayElement(args, i))};
        arg_storage.push_back(android::to_utf8(env, arg.get()));
    }
    std::vector<char*> argv;
    argv.reserve(arg_storage.size() + 1);
    for (std::string& arg : arg_storage)
    {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);
    int argc = static_cast<int>(arg_storage.size());

    ::testing::InitGoogleTest(&argc, argv.data());
    return RUN_ALL_TESTS();
}
