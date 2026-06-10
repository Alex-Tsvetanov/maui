#pragma once
// maui::platform::android — JNI environment access. Internal seam infrastructure for the Android
// backend, NOT a ported MAUI type (the C# counterpart is .NET-for-Android's JNIEnv/JniRuntime
// plumbing, which the C++ port replaces with explicit registration + RAII).
//
// Ownership doctrine (PROFILE §8) applied to JNI: the process-wide JavaVM* is a non-owning borrow
// registered exactly once (from JNI_OnLoad, or from the test host's first native entry); per-thread
// JNIEnv* pointers are NEVER cached across threads — scoped_env materializes one for the current
// thread on demand, attaching to the VM if the thread is detached and detaching on scope exit ONLY
// when it performed the attach (a thread that was already attached stays attached).

#include <jni.h>

#include <atomic>

namespace maui::platform::android
{
    namespace detail
    {
        inline std::atomic<JavaVM*>& java_vm_slot() noexcept
        {
            static std::atomic<JavaVM*> slot{nullptr};
            return slot;
        }
    } // namespace detail

    // Register the process JavaVM. Call once, before any scoped_env/global_ref use.
    inline void set_java_vm(JavaVM* vm) noexcept
    {
        detail::java_vm_slot().store(vm, std::memory_order_release);
    }

    [[nodiscard]] inline JavaVM* java_vm() noexcept
    {
        return detail::java_vm_slot().load(std::memory_order_acquire);
    }

    // scoped_env — a JNIEnv* for the current thread, valid for this scope. Empty (get() == nullptr)
    // when no VM is registered or the attach fails; callers must check before dereferencing.
    class scoped_env
    {
    public:
        scoped_env() : scoped_env(java_vm())
        {
        }

        explicit scoped_env(JavaVM* vm)
        {
            if (vm == nullptr)
            {
                return;
            }
            void* current = nullptr;
            const jint status = vm->GetEnv(&current, JNI_VERSION_1_6);
            if (status == JNI_OK)
            {
                env_ = static_cast<JNIEnv*>(current);
            }
            else if (status == JNI_EDETACHED && vm->AttachCurrentThread(&env_, nullptr) == JNI_OK)
            {
                owner_vm_ = vm; // this object attached the thread, so it detaches on destruction
            }
        }

        ~scoped_env()
        {
            if (owner_vm_ != nullptr)
            {
                owner_vm_->DetachCurrentThread();
            }
        }

        scoped_env(const scoped_env&) = delete;
        scoped_env& operator=(const scoped_env&) = delete;
        scoped_env(scoped_env&&) = delete;
        scoped_env& operator=(scoped_env&&) = delete;

        [[nodiscard]] JNIEnv* get() const noexcept
        {
            return env_;
        }
        [[nodiscard]] JNIEnv* operator->() const noexcept
        {
            return env_;
        }
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return env_ != nullptr;
        }

    private:
        JNIEnv* env_ = nullptr;
        JavaVM* owner_vm_ = nullptr; // non-null only when this scope performed the attach
    };
} // namespace maui::platform::android
