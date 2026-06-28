#pragma once
// maui::ui::view_ref<T> + maui::ui::weak_ref<T> — the consumer handle pair (PUBLIC_API_DESIGN.md §2.5).
//
// view_ref<T>: the MOVE-ONLY owning handle a consumer holds for a control. Copy is deleted, so capturing
// an owning handle inside the control's own event handler ([h]{...}) is a COMPILE ERROR — §8's cycle
// footgun is enforced by the type system, not a lint. It wraps the §8-mandated shared_ptr<control_impl>
// (a stable heap address) plus, for a builder node, the shared_ptr to each child it owns (retained_) and
// the RAII tokens of any parked event subscriptions (tokens_).
//
// weak_ref<T>: the COPYABLE non-owning observer. .lock() yields a transient owning view_ref (empty if the
// control has died) to dot into — the dangling-safe way to keep and later mutate a held control, and the
// blessed form for self-capture in a handler ([w = h.weak()]{ if (auto l = w.lock()) ... }).
//
// .share() mints an explicit, visible SECOND owner for the rare genuinely-co-owned control. Refcount is
// currently std::shared_ptr (atomic); // TODO(perf): swap for a non-atomic intrusive refcount (§8 UI-thread).

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/core/event.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/thickness.hpp"

namespace maui::ui
{
    template <class Impl> class weak_ref;

    template <class Impl> class view_ref
    {
    public:
        view_ref() = default;
        explicit view_ref(std::shared_ptr<Impl> impl) noexcept : impl_(std::move(impl))
        {
        }

        view_ref(view_ref&&) noexcept = default;
        view_ref& operator=(view_ref&&) noexcept = default;
        view_ref(const view_ref&) = delete; // move-only: capturing an owner into its own handler won't compile
        view_ref& operator=(const view_ref&) = delete;
        ~view_ref() = default;

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return impl_ != nullptr;
        }
        [[nodiscard]] Impl* operator->() const noexcept
        {
            return impl_.get();
        }
        [[nodiscard]] Impl& impl() const noexcept
        {
            return *impl_;
        }

        // A non-owning observer over the same control (copyable; survives this handle being moved/dropped).
        [[nodiscard]] weak_ref<Impl> weak() const noexcept
        {
            return weak_ref<Impl>(impl_);
        }

        // Mint an explicit, visible second OWNER (co-ownership across independent lifetimes). Use weak() for
        // stored/self-capture references; reserve share() for an audited cluster.
        [[nodiscard]] view_ref share() const
        {
            view_ref copy;
            copy.impl_ = impl_;
            copy.retained_ = retained_;
            return copy;
        }

        // ---- curated fluent chainers (rvalue-qualified, thread through the builder move; forward to the
        // REAL set_* so side effects run). The long tail uses operator-> / impl(). ----
        view_ref&& text(std::string value) &&
            requires requires(Impl& v, std::string s) { v.set_text(std::move(s)); }
        {
            impl_->set_text(std::move(value));
            return std::move(*this);
        }
        view_ref&& placeholder(std::string value) &&
            requires requires(Impl& v, std::string s) { v.set_placeholder(std::move(s)); }
        {
            impl_->set_placeholder(std::move(value));
            return std::move(*this);
        }
        view_ref&& spacing(double value) &&
            requires requires(Impl& v) { v.set_spacing(value); }
        {
            impl_->set_spacing(value);
            return std::move(*this);
        }
        view_ref&& padding(maui::core::thickness value) &&
            requires requires(Impl& v, maui::core::thickness t) { v.set_padding(t); }
        {
            impl_->set_padding(value);
            return std::move(*this);
        }
        // Subscribe to the control's `clicked` event; the RAII token is parked IN this handle (tokens_) and
        // is absorbed by the parent on adoption, so it tears down before the control it subscribes to.
        template <class F>
            requires std::is_invocable_r_v<void, F&> && requires(Impl& v) { v.clicked; }
        view_ref&& on_click(F handler) &&
        {
            tokens_.push_back(maui::core::connect_scoped(impl_->clicked, std::move(handler)));
            return std::move(*this);
        }

        // ---- grid chainers (ui::grid()): column/row definitions, gutters, and cell placement ----
        template <class... Lengths>
        view_ref&& columns(Lengths... lengths) &&
            requires requires(Impl& v, maui::core::grid_length g) { v.add_column_definition(g); }
        {
            (impl_->add_column_definition(lengths), ...);
            return std::move(*this);
        }
        template <class... Lengths>
        view_ref&& rows(Lengths... lengths) &&
            requires requires(Impl& v, maui::core::grid_length g) { v.add_row_definition(g); }
        {
            (impl_->add_row_definition(lengths), ...);
            return std::move(*this);
        }
        view_ref&& row_spacing(double value) &&
            requires requires(Impl& v) { v.set_row_spacing(value); }
        {
            impl_->set_row_spacing(value);
            return std::move(*this);
        }
        view_ref&& column_spacing(double value) &&
            requires requires(Impl& v) { v.set_column_spacing(value); }
        {
            impl_->set_column_spacing(value);
            return std::move(*this);
        }
        // Place a child at (row, column): the framework add() + Grid.Row/Grid.Column, and absorb its ownership.
        template <class Child>
        view_ref&& cell(int row, int column, view_ref<Child>&& child) &&
            requires requires(Impl& v, Child& c) {
                v.add(c);
                v.set_row(c, row);
                v.set_column(c, column);
            }
        {
            if (child.impl_)
            {
                impl_->add(*child.impl_);
                impl_->set_row(*child.impl_, row);
                impl_->set_column(*child.impl_, column);
                absorb_ownership(std::move(child));
            }
            return std::move(*this);
        }

        // ---- builder adoption (used by ui::vstack / ui::page / ...): take over a child's ownership ----
        // Container (i_layout) parent: wire the framework's non-owning add() AND absorb the child's owner.
        template <class Child>
            requires requires(Impl& parent, Child& child) { parent.add(child); }
        void adopt_child(view_ref<Child>&& child)
        {
            if (!child.impl_)
            {
                return;
            }
            impl_->add(*child.impl_);
            absorb_ownership(std::move(child));
        }
        // Single-content host (content_page / border / ...) parent: wire set_content() AND absorb the owner.
        template <class Child>
            requires requires(Impl& parent, Child& child) { parent.set_content(child); }
        void adopt_content(view_ref<Child>&& child)
        {
            if (!child.impl_)
            {
                return;
            }
            impl_->set_content(*child.impl_);
            absorb_ownership(std::move(child));
        }

        // --- detail (builder / test access) ---
        [[nodiscard]] const std::shared_ptr<Impl>& shared() const noexcept
        {
            return impl_;
        }

    private:
        template <class U> friend class view_ref;
        template <class U> friend class weak_ref;

        // Move a child's whole ownership (its control, its descendants, and its parked tokens) into this
        // node, so the root view_ref owns the entire subtree and tears it down in the correct order
        // (tokens_ first — they disconnect from still-alive events — then retained_, then impl_).
        template <class Child> void absorb_ownership(view_ref<Child>&& child)
        {
            retained_.push_back(std::static_pointer_cast<maui::controls::element>(child.impl_));
            for (auto& descendant : child.retained_)
            {
                retained_.push_back(std::move(descendant));
            }
            for (auto& token : child.tokens_)
            {
                tokens_.push_back(std::move(token));
            }
            child.impl_.reset();
            child.retained_.clear();
            child.tokens_.clear();
        }

        // Declaration order is teardown order (reverse): tokens_ destruct FIRST (disconnect from events on
        // still-alive controls), then retained_ (children), then impl_ (this control). Do not reorder.
        std::shared_ptr<Impl> impl_;
        std::vector<std::shared_ptr<maui::controls::element>> retained_;
        std::vector<maui::core::scoped_connection> tokens_;
    };

    template <class Impl> class weak_ref
    {
    public:
        weak_ref() = default;
        explicit weak_ref(const std::shared_ptr<Impl>& source) noexcept : w_(source)
        {
        }

        // A transient owning handle to dot into (empty if the control has been destroyed). The returned
        // owner is local-only, so it cannot form a cycle.
        [[nodiscard]] view_ref<Impl> lock() const noexcept
        {
            return view_ref<Impl>(w_.lock());
        }
        [[nodiscard]] bool alive() const noexcept
        {
            return !w_.expired();
        }

    private:
        std::weak_ptr<Impl> w_;
    };
} // namespace maui::ui
