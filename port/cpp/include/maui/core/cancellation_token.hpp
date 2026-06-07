#pragma once
// maui::core::cancellation_token  <=  System.Threading.CancellationToken / CancellationTokenSource
//
// A minimal cooperative cancellation primitive for the async image loader (image_source_loader.hpp),
// the C++ stand-in for the CTS/token pair C#'s ImageSourceServiceResultManager.BeginLoad() cancels +
// re-issues. Modeled as a shared flag: a single std::shared_ptr<std::atomic<bool>> is shared between the
// token handed to the worker and the loader that may cancel it. cancel() sets the flag;
// is_cancelled() reads it. A default-constructed token (no shared state) is never cancelled.
//
// Thread-safety: the flag is a std::atomic<bool>, so cancel() on one thread and is_cancelled() on
// another are race-free (relaxed ordering suffices — it is an advisory hint, not a lock). The loader's
// apply still runs on the dispatcher thread, so cancellation only needs to be visible, not ordered.

#include <atomic>
#include <memory>

namespace maui::core
{
    class cancellation_token
    {
    public:
        // A token with no shared state — permanently non-cancelled (mirrors `default(CancellationToken)`).
        cancellation_token() = default;

        // Bind to an existing shared flag (the loader mints one per in-flight load and keeps a copy).
        explicit cancellation_token(std::shared_ptr<std::atomic<bool>> flag) : flag_(std::move(flag))
        {
        }

        // True once cancel() has been called on any copy sharing this flag.
        [[nodiscard]] bool is_cancelled() const noexcept
        {
            return flag_ != nullptr && flag_->load(std::memory_order_relaxed);
        }

        // Request cancellation (idempotent). Visible to every copy sharing the flag.
        void cancel() const noexcept
        {
            if (flag_ != nullptr)
            {
                flag_->store(true, std::memory_order_relaxed);
            }
        }

    private:
        std::shared_ptr<std::atomic<bool>> flag_;
    };
} // namespace maui::core
