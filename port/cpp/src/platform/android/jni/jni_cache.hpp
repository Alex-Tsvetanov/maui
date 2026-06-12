#pragma once
// maui::platform::android — jclass/jmethodID cache. Internal seam infrastructure for the Android
// backend, NOT a ported MAUI type. FindClass walks the class loader and returns a fresh LOCAL
// reference on every call, and GetMethodID re-parses the signature — both are the canonical JNI
// hot-path costs, so the seam resolves each class once into a process-lifetime global reference and
// memoizes method ids per (class, name, signature). jmethodID values stay valid as long as their
// class is not unloaded; pinning the jclass globally here guarantees exactly that.
//
// Lookup failures (unknown class / unknown method) clear the pending Java exception and surface as
// nullptr so call sites keep explicit control flow — the JNI pending-exception state never leaks
// out of the cache.
//
// Thread safety: every lookup happens under one mutex (cold path only — the memoized hits are a
// hash probe). The cached global jclass references are deliberately leaked at process exit, the
// standard JNI practice (the VM dies with the process; tearing them down in static destructors
// would race VM shutdown).

#include <jni.h>

#include <mutex>
#include <string>
#include <unordered_map>

#include "jni_ref.hpp"

namespace maui::platform::android
{
    class jni_cache
    {
    public:
        // The pinned jclass for a slash-form name ("android/widget/Button"), or nullptr.
        [[nodiscard]] jclass find_class(JNIEnv* env, const char* name)
        {
            const std::scoped_lock lock(mutex_);
            const auto memoized = classes_.find(name);
            if (memoized != classes_.end())
            {
                return memoized->second.get();
            }
            const local_ref<jclass> found{env, env->FindClass(name)};
            if (!found)
            {
                env->ExceptionClear(); // ClassNotFoundError -> nullptr, no pending state
                return nullptr;
            }
            return classes_.emplace(name, global_ref<jclass>{env, found.get()}).first->second.get();
        }

        // Instance method id ("<init>" included), resolved through the pinned class; nullptr if the
        // class or the method is missing.
        [[nodiscard]] jmethodID method(JNIEnv* env, const char* class_name, const char* name, const char* signature)
        {
            return method_impl(env, class_name, name, signature, /*is_static=*/false);
        }

        [[nodiscard]] jmethodID static_method(JNIEnv* env, const char* class_name, const char* name,
                                              const char* signature)
        {
            return method_impl(env, class_name, name, signature, /*is_static=*/true);
        }

        // Instance field id ("F", "I", …), resolved through the pinned class; nullptr if the class
        // or the field is missing. jfieldID values share jmethodID's validity rule (valid while the
        // class is not unloaded — guaranteed by the pinned jclass), so the same memoization applies.
        [[nodiscard]] jfieldID field(JNIEnv* env, const char* class_name, const char* name, const char* signature)
        {
            jclass owner = find_class(env, class_name);
            if (owner == nullptr)
            {
                return nullptr;
            }
            std::string key{class_name};
            key += '.'; // distinct from the '#'/'$' method keys — fields live in the same map space
            key += name;
            key += signature;
            const std::scoped_lock lock(mutex_);
            const auto memoized = fields_.find(key);
            if (memoized != fields_.end())
            {
                return memoized->second;
            }
            jfieldID id = env->GetFieldID(owner, name, signature);
            if (id == nullptr)
            {
                env->ExceptionClear(); // NoSuchFieldError -> nullptr, no pending state
                return nullptr;
            }
            fields_.emplace(std::move(key), id);
            return id;
        }

    private:
        [[nodiscard]] jmethodID method_impl(JNIEnv* env, const char* class_name, const char* name,
                                            const char* signature, bool is_static)
        {
            jclass owner = find_class(env, class_name);
            if (owner == nullptr)
            {
                return nullptr;
            }
            std::string key{class_name};
            key += is_static ? '$' : '#';
            key += name;
            key += signature;
            const std::scoped_lock lock(mutex_);
            const auto memoized = methods_.find(key);
            if (memoized != methods_.end())
            {
                return memoized->second;
            }
            jmethodID id =
                is_static ? env->GetStaticMethodID(owner, name, signature) : env->GetMethodID(owner, name, signature);
            if (id == nullptr)
            {
                env->ExceptionClear(); // NoSuchMethodError -> nullptr, no pending state
                return nullptr;
            }
            methods_.emplace(std::move(key), id);
            return id;
        }

        std::mutex mutex_;
        std::unordered_map<std::string, global_ref<jclass>> classes_;
        std::unordered_map<std::string, jmethodID> methods_;
        std::unordered_map<std::string, jfieldID> fields_;
    };

    // The process-wide cache instance (the backend's handlers and the test host share it).
    // Heap-allocated and never destroyed ON PURPOSE: a static OBJECT's destructor would run at
    // process exit and DeleteGlobalRef every pinned jclass — the VM-shutdown race the class
    // comment rules out. The one-time allocation is the leak the doctrine documents.
    [[nodiscard]] inline jni_cache& default_jni_cache()
    {
        static jni_cache& cache = *new jni_cache();
        return cache;
    }
} // namespace maui::platform::android
