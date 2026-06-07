// maui::core::manual_dispatcher — headless virtual-clock dispatcher (manual_dispatcher.hpp).
#include "maui/core/manual_dispatcher.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "maui/core/dispatcher.hpp"
#include "maui/core/event.hpp"

namespace maui::core
{
    namespace
    {
        // Concrete headless timer: reschedules itself through the dispatcher's delayed queue. A
        // generation counter invalidates pending ticks across stop()/restart(); a shared "life"
        // token lets a still-queued tick detect that the timer was destroyed before it fired.
        class manual_dispatcher_timer final : public i_dispatcher_timer
        {
        public:
            explicit manual_dispatcher_timer(manual_dispatcher &owner) : owner_(&owner)
            {
            }
            manual_dispatcher_timer(const manual_dispatcher_timer &) = delete;
            manual_dispatcher_timer(manual_dispatcher_timer &&) = delete;
            manual_dispatcher_timer &operator=(const manual_dispatcher_timer &) = delete;
            manual_dispatcher_timer &operator=(manual_dispatcher_timer &&) = delete;
            ~manual_dispatcher_timer() override = default;

            [[nodiscard]] std::chrono::milliseconds interval() const override
            {
                return interval_;
            }
            void set_interval(std::chrono::milliseconds value) override
            {
                interval_ = value;
            }
            [[nodiscard]] bool is_repeating() const override
            {
                return is_repeating_;
            }
            void set_is_repeating(bool value) override
            {
                is_repeating_ = value;
            }
            [[nodiscard]] bool is_running() const override
            {
                return running_;
            }
            event<> &tick() override
            {
                return tick_;
            }

            void start() override
            {
                if (running_)
                {
                    return;
                }
                running_ = true;
                ++generation_;
                schedule();
            }
            void stop() override
            {
                running_ = false;
                ++generation_; // any tick already queued is now stale
            }

        private:
            void schedule()
            {
                std::uint64_t const gen = generation_;
                std::weak_ptr<int> const life = life_;
                owner_->dispatch_delayed(interval_, [this, gen, life] {
                    if (life.expired())
                    {
                        return; // timer was destroyed before this tick fired
                    }
                    if (!running_ || gen != generation_)
                    {
                        return; // stopped or restarted since this tick was scheduled
                    }
                    tick_.raise();
                    if (running_ && is_repeating_)
                    {
                        schedule();
                    }
                    else
                    {
                        running_ = false;
                    }
                });
            }

            manual_dispatcher *owner_;
            std::shared_ptr<int> life_ = std::make_shared<int>(0);
            event<> tick_;
            std::chrono::milliseconds interval_{std::chrono::milliseconds::zero()};
            bool is_repeating_ = false; // matches the C# bool default
            bool running_ = false;
            std::uint64_t generation_ = 0;
        };
    } // namespace

    manual_dispatcher::manual_dispatcher() : owner_thread_(std::this_thread::get_id())
    {
    }

    bool manual_dispatcher::is_dispatch_required() const
    {
        return std::this_thread::get_id() != owner_thread_;
    }

    bool manual_dispatcher::dispatch(dispatcher_action action)
    {
        queue_.emplace_back(now_, next_seq_++, std::move(action));
        return true;
    }

    bool manual_dispatcher::dispatch_delayed(std::chrono::milliseconds delay, dispatcher_action action)
    {
        std::chrono::milliseconds const due = now_ + delay;
        queue_.emplace_back(due, next_seq_++, std::move(action));
        return true;
    }

    std::unique_ptr<i_dispatcher_timer> manual_dispatcher::create_timer()
    {
        return std::make_unique<manual_dispatcher_timer>(*this);
    }

    std::size_t manual_dispatcher::run_due()
    {
        std::size_t count = 0;
        while (true)
        {
            auto best = queue_.end();
            for (auto it = queue_.begin(); it != queue_.end(); ++it)
            {
                if (it->due <= now_ &&
                    (best == queue_.end() || it->due < best->due || (it->due == best->due && it->seq < best->seq)))
                {
                    best = it;
                }
            }
            if (best == queue_.end())
            {
                break;
            }
            dispatcher_action const action = std::move(best->action);
            queue_.erase(best);
            action(); // may enqueue more work (a timer reschedule, a nested dispatch)
            ++count;
        }
        return count;
    }

    std::size_t manual_dispatcher::run_pending()
    {
        return run_due();
    }

    void manual_dispatcher::advance(std::chrono::milliseconds delta)
    {
        std::chrono::milliseconds const target = now_ + delta;
        run_due();
        while (true)
        {
            std::optional<std::chrono::milliseconds> next;
            for (const auto &e : queue_)
            {
                if (e.due > now_ && e.due <= target && (!next || e.due < *next))
                {
                    next = e.due;
                }
            }
            if (!next)
            {
                break;
            }
            now_ = *next;
            run_due();
        }
        now_ = target;
    }

    std::chrono::milliseconds manual_dispatcher::now() const
    {
        return now_;
    }

    std::size_t manual_dispatcher::pending_count() const
    {
        return queue_.size();
    }
} // namespace maui::core
