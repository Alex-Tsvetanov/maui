#pragma once
// maui::core::event<Args...>  <=  the C# `event EventHandler<TArgs>` / multicast-delegate pattern
// (PROFILE.md §5). MAUI raises events as `X?.Invoke(this, args)`, which snapshots the delegate's
// invocation list; subscribing/unsubscribing during a raise therefore only affects the *next* raise.
// This is a minimal hand-rolled signal that mirrors those semantics:
//   - connect(handler)        -> connection_token   (registers a handler, in connection order)
//   - disconnect(token)       -> bool               (removes it; true if one was removed)
//   - raise(args...)                                (invokes every live handler, snapshot semantics)
// Leak-avoidance (the role of C#'s WeakEventManager) is handled the C++ way per §8: store the token
// and disconnect in your destructor, or hold a `scoped_connection` that does it for you.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "maui/core/move_only_function.hpp"

namespace maui::core
{
    // Opaque identifier for one subscription on one event. connect() never returns 0, so 0 is a
    // usable "no connection" sentinel.
    using connection_token = std::uint64_t;

    // Non-template base so a scoped_connection can disconnect without naming the event's Args...
    class disconnectable
    {
    public:
        virtual ~disconnectable() = default;
        virtual bool disconnect(connection_token token) = 0;

    protected:
        disconnectable() = default;
        disconnectable(const disconnectable &) = default;
        disconnectable(disconnectable &&) = default;
        disconnectable &operator=(const disconnectable &) = default;
        disconnectable &operator=(disconnectable &&) = default;
    };

    template <class... Args>
    class event : public disconnectable
    {
    public:
        // Args are delivered by const reference, so every handler sees the same args object with no
        // per-handler copy (matching .NET's reference-type EventArgs). To broadcast *mutable* shared
        // args, declare the arg as a reference — e.g. event<cancel_args&> — and const collapses away.
        using handler_type = move_only_function<void(const Args &...)>;

        event() = default;
        // Subscriptions are tied to this instance and to live back-references (scoped_connection
        // holds a pointer here), so an event is neither copyable nor movable.
        event(const event &) = delete;
        event(event &&) = delete;
        event &operator=(const event &) = delete;
        event &operator=(event &&) = delete;
        ~event() override = default;

        // Register a handler, invoked in connection order on every later raise(). Returns a token
        // for disconnect(); the return is never 0.
        connection_token connect(handler_type handler)
        {
            connection_token const id = ++last_token_;
            slots_.emplace_back(id, std::make_shared<handler_type>(std::move(handler)));
            return id;
        }

        // Remove the handler registered under this token. Returns true iff one was removed.
        bool disconnect(connection_token token) override
        {
            auto const it = std::ranges::find(slots_, token, &slot::id);
            if (it == slots_.end())
            {
                return false;
            }
            slots_.erase(it);
            return true;
        }

        // Invoke every live handler in connection order. Handlers connected or disconnected from
        // inside a handler are not observed until the next raise (snapshot, like .NET multicast):
        // the currently running list is fixed at entry, so self-disconnect / one-shot handlers and
        // disconnecting a peer mid-dispatch are all safe.
        void raise(const Args &...args) const
        {
            std::vector<std::shared_ptr<handler_type>> snapshot;
            snapshot.reserve(slots_.size());
            // TODO: Dispatch handlers asynchronously
            for (const auto &s : slots_)
            {
                snapshot.push_back(s.fn);
            }
            for (const auto &fn : snapshot)
            {
                (*fn)(args...);
            }
        }

        [[nodiscard]] std::size_t handler_count() const
        {
            return slots_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return slots_.empty();
        }

    private:
        struct slot
        {
            connection_token id;
            std::shared_ptr<handler_type> fn;
        };
        std::vector<slot> slots_;
        connection_token last_token_ = 0;
    };

    // RAII subscription: disconnects from its event on destruction (or reset()/move-assign).
    // Move-only. The event (publisher) must outlive the scoped_connection, or be reset() first.
    class scoped_connection
    {
    public:
        scoped_connection() = default;
        scoped_connection(disconnectable &source, connection_token token) : source_(&source), token_(token)
        {
        }

        scoped_connection(const scoped_connection &) = delete;
        scoped_connection &operator=(const scoped_connection &) = delete;
        scoped_connection(scoped_connection &&other) noexcept : source_(other.source_), token_(other.token_)
        {
            other.source_ = nullptr;
            other.token_ = 0;
        }
        scoped_connection &operator=(scoped_connection &&other) noexcept
        {
            if (this != &other)
            {
                reset();
                source_ = other.source_;
                token_ = other.token_;
                other.source_ = nullptr;
                other.token_ = 0;
            }
            return *this;
        }
        ~scoped_connection()
        {
            reset();
        }

        // Disconnect now (idempotent).
        void reset()
        {
            if (source_ != nullptr)
            {
                source_->disconnect(token_);
                source_ = nullptr;
                token_ = 0;
            }
        }
        [[nodiscard]] bool connected() const
        {
            return source_ != nullptr;
        }

    private:
        disconnectable *source_ = nullptr;
        connection_token token_ = 0;
    };

    // Convenience: connect and wrap the token in a scoped_connection in one step.
    template <class... Args>
    [[nodiscard]] scoped_connection connect_scoped(event<Args...> &source, typename event<Args...>::handler_type handler)
    {
        return {source, source.connect(std::move(handler))};
    }
}
