#pragma once
// maui::platform::android — JNI reference RAII. Internal seam infrastructure for the Android
// backend, NOT a ported MAUI type. The pimpl-owned-native-view doctrine (PROFILE §8) maps onto JNI
// as: every native widget handle a handler keeps is a global_ref released in the platform struct's
// destructor; everything created transiently inside a mapper call is a local_ref.
//
//   - local_ref<T>:  owns a JNI LOCAL reference (DeleteLocalRef on destruction). Bound to the
//                    JNIEnv* that minted it — never store one beyond the native call that owns
//                    that env, and never hand one to another thread.
//   - global_ref<T>: owns a JNI GLOBAL reference (NewGlobalRef on construction, DeleteGlobalRef on
//                    destruction). Valid across threads and calls; destruction obtains an env for
//                    the current thread via scoped_env, so it is safe from any thread once the
//                    process JavaVM is registered (jni_env.hpp).
// Both are move-only, mirroring unique_ptr semantics (T is a jobject-family pointer type).

#include <jni.h>

#include <utility>

#include "jni_env.hpp"

namespace maui::platform::android
{
    template <typename T = jobject> class local_ref
    {
    public:
        local_ref() = default;
        // Adopts an already-created local reference (e.g. the return of NewObject/CallObjectMethod).
        local_ref(JNIEnv* env, T object) noexcept : env_(env), object_(object)
        {
        }

        ~local_ref()
        {
            reset();
        }

        local_ref(local_ref&& other) noexcept : env_(other.env_), object_(other.release())
        {
        }
        local_ref& operator=(local_ref&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                env_ = other.env_;
                object_ = other.release();
            }
            return *this;
        }
        local_ref(const local_ref&) = delete;
        local_ref& operator=(const local_ref&) = delete;

        [[nodiscard]] T get() const noexcept
        {
            return object_;
        }
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return object_ != nullptr;
        }

        // Relinquishes ownership without deleting (e.g. to return the reference to Java).
        T release() noexcept
        {
            return std::exchange(object_, nullptr);
        }

        void reset() noexcept
        {
            if (object_ != nullptr && env_ != nullptr)
            {
                env_->DeleteLocalRef(object_);
            }
            object_ = nullptr;
        }

    private:
        JNIEnv* env_ = nullptr;
        T object_ = nullptr;
    };

    template <typename T = jobject> class global_ref
    {
    public:
        global_ref() = default;
        // Mints a NEW global reference from any reference (local or global); the source keeps its
        // own ownership (a local_ref argument still deletes its local on destruction).
        global_ref(JNIEnv* env, T object)
            : object_(object != nullptr ? static_cast<T>(env->NewGlobalRef(object)) : nullptr)
        {
        }

        ~global_ref()
        {
            reset();
        }

        global_ref(global_ref&& other) noexcept : object_(other.release())
        {
        }
        global_ref& operator=(global_ref&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                object_ = other.release();
            }
            return *this;
        }
        global_ref(const global_ref&) = delete;
        global_ref& operator=(const global_ref&) = delete;

        [[nodiscard]] T get() const noexcept
        {
            return object_;
        }
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return object_ != nullptr;
        }

        T release() noexcept
        {
            return std::exchange(object_, nullptr);
        }

        void reset() noexcept
        {
            if (object_ == nullptr)
            {
                return;
            }
            const scoped_env env; // any-thread teardown: attach on demand (see the class comment)
            if (env)
            {
                env->DeleteGlobalRef(object_);
            }
            object_ = nullptr;
        }

    private:
        T object_ = nullptr;
    };
} // namespace maui::platform::android
