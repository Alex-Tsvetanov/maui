#pragma once
// maui::core::bind  <=  Microsoft.Maui.Controls.Binding / BindingExpression (the typed-accessor port)
//
// C# bindings resolve a string Path ("Customer.Name") against a source via REFLECTION and observe
// changes through INotifyPropertyChanged. C++23 has no reflection (PROFILE §6), and the port is
// code-first, so M5 binds the typed way: a binding connects a target `property<T>` to a source
// `property<U>` directly, observing the source through the property's existing typed `.changed` event
// and pushing values at `setter_specificity::from_binding` (so a manual set still wins, and clearing
// the manual value restores the bound value — the exact precedence ladder C# uses for FromBinding).
//
// Nested string paths, the registered-name→accessor table, StringFormat / FallbackValue / TargetNullValue,
// MultiBinding, and compiled-binding source-gen are deferred to M7 (XAML) — see STATUS.md.
//
// Lifetime: a binding holds references to the two properties and RAII subscriptions to their `.changed`
// events. The returned binding_handle owns those subscriptions; it MUST NOT outlive either property (the
// usual case — the target's owning control holds the handle, and the source view-model outlives the view,
// or the handle is reset() first). bindable_object::add_binding/remove_binding manage handle lifetime for
// a target property.

#include <memory>
#include <utility>
#include <vector>

#include "maui/core/binding_mode.hpp"
#include "maui/core/event.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"

namespace maui::core
{
    // RAII teardown for one binding: dropping it disconnects the source/target subscriptions (the role
    // of C#'s BindingExpression.Unapply + the weak-event proxies). Move-only.
    class binding_handle
    {
    public:
        binding_handle() = default;
        explicit binding_handle(std::vector<scoped_connection> connections) : connections_(std::move(connections))
        {
        }
        binding_handle(const binding_handle&) = delete;
        binding_handle& operator=(const binding_handle&) = delete;
        binding_handle(binding_handle&&) noexcept = default;
        binding_handle& operator=(binding_handle&&) noexcept = default;
        ~binding_handle() = default;

        // Tear the binding down now (idempotent).
        void reset()
        {
            connections_.clear();
        }
        [[nodiscard]] bool active() const
        {
            return !connections_.empty();
        }

    private:
        std::vector<scoped_connection> connections_;
    };

    // Resolve binding_mode::default_mode against the target's default_binding_mode, and downgrade two_way
    // to one_way_to_source on a read-only target (mirrors BindingBaseExtensions.GetRealizedMode).
    template <class T> [[nodiscard]] binding_mode resolve_binding_mode(const property<T>& target, binding_mode mode)
    {
        if (mode == binding_mode::default_mode)
        {
            mode = target.default_binding_mode();
        }
        if (mode == binding_mode::default_mode)
        {
            mode = binding_mode::one_way; // a property whose default is itself "default" => one_way
        }
        if (mode == binding_mode::two_way && target.is_read_only())
        {
            mode = binding_mode::one_way_to_source;
        }
        return mode;
    }

    // Bind `target` to `source` with explicit converter functors: `convert` maps source→target (used for
    // one_way/two_way/one_time), `convert_back` maps target→source (used for two_way/one_way_to_source).
    template <class TTarget, class TSource, class Convert, class ConvertBack>
    [[nodiscard]] binding_handle bind(property<TTarget>& target, property<TSource>& source, binding_mode mode,
                                      Convert convert, ConvertBack convert_back)
    {
        mode = resolve_binding_mode(target, mode);
        const bool to_target =
            mode == binding_mode::one_way || mode == binding_mode::two_way || mode == binding_mode::one_time;
        const bool to_source = mode == binding_mode::two_way || mode == binding_mode::one_way_to_source;

        // Shared re-entrancy guard: while applying one direction, suppress the other so a two_way binding
        // settles in a single round-trip instead of ping-ponging (property<T>::set already no-ops on an
        // equal value, but the guard makes it robust to asymmetric converters).
        auto guard = std::make_shared<bool>(false);
        std::vector<scoped_connection> conns;

        // Initial push (C# applies the binding once at bind time).
        if (to_target)
        {
            target.set(convert(source.get()), setter_specificity::from_binding);
        }
        else if (to_source)
        {
            source.set(convert_back(target.get()));
        }

        if (to_target && mode != binding_mode::one_time)
        {
            conns.push_back(connect_scoped(source.changed,
                                           [&target, convert, guard](const TSource& /*old*/, const TSource& new_value) {
                                               if (*guard)
                                               {
                                                   return;
                                               }
                                               *guard = true;
                                               target.set(convert(new_value), setter_specificity::from_binding);
                                               *guard = false;
                                           }));
        }
        if (to_source)
        {
            conns.push_back(connect_scoped(
                target.changed, [&source, convert_back, guard](const TTarget& /*old*/, const TTarget& new_value) {
                    if (*guard)
                    {
                        return;
                    }
                    *guard = true;
                    source.set(convert_back(new_value));
                    *guard = false;
                }));
        }
        return binding_handle(std::move(conns));
    }

    // One-directional converter binding (for one_way / one_time): only `convert` (source→target) is given.
    // NOT for two_way/one_way_to_source — the back-conversion is a never-called stub here.
    template <class TTarget, class TSource, class Convert>
    [[nodiscard]] binding_handle bind(property<TTarget>& target, property<TSource>& source, binding_mode mode,
                                      Convert convert)
    {
        return bind(target, source, mode, std::move(convert),
                    [](const TTarget& /*unused*/) -> TSource { return TSource{}; });
    }

    // No-converter binding (same value type both ends): identity in both directions.
    template <class T>
    [[nodiscard]] binding_handle bind(property<T>& target, property<T>& source,
                                      binding_mode mode = binding_mode::default_mode)
    {
        return bind(target, source, mode, [](const T& value) { return value; }, [](const T& value) { return value; });
    }
} // namespace maui::core
