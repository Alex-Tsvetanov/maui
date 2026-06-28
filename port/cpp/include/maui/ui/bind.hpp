#pragma once
// maui::ui::bind — typed, compile-checked binding from a control property to a view-model observable<T>
// (PUBLIC_API_DESIGN.md §3-E). All writes route through the control's real set_* (so side effects — e.g.
// entry truncation — run) and the control's own change event; the source is a typed observable<T>, so a
// wrong name is a compile error. It never touches the control's private property<T>.
//
//   one-way (source -> control):   ui::bind(ctrl, &Ctrl::set_x).to(vm.obs);
//   two-way (source <-> control):  ui::bind(ctrl, &Ctrl::set_x, &Ctrl::x, &ctrl.changed_evt).to_two_way(vm.obs);
//
// Returns a maui::core::binding_handle (RAII; reset()s the subscriptions). The handle must not outlive the
// control or the source — store it where both outlive it (§8 token-as-member discipline).

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/core/binding.hpp" // binding_handle
#include "maui/core/event.hpp"
#include "maui/core/observable.hpp"

namespace maui::ui
{
    namespace detail
    {
        // SetBase is the class that declares the setter — equal to Ctrl for a leaf-declared property (e.g.
        // label::set_text), or a base of Ctrl for an INHERITED property (e.g. button's set_is_enabled, which is
        // declared on the view<> base). `ctrl_->*setter_` is well-formed either way since Ctrl derives SetBase.
        template <class Ctrl, class SetBase, class SetArg> class one_way_binder
        {
        public:
            one_way_binder(Ctrl& ctrl, void (SetBase::*setter)(SetArg)) : ctrl_(&ctrl), setter_(setter)
            {
            }

            template <class T> [[nodiscard]] maui::core::binding_handle to(maui::core::observable<T>& source) &&
            {
                std::vector<maui::core::scoped_connection> conns;
                (ctrl_->*setter_)(static_cast<SetArg>(source.get())); // initial push at bind time
                conns.push_back(maui::core::connect_scoped(
                    source.changed(), [ctrl = ctrl_, setter = setter_](const T& /*old*/, const T& nv) {
                        (ctrl->*setter)(static_cast<SetArg>(nv));
                    }));
                return maui::core::binding_handle(std::move(conns));
            }

        private:
            Ctrl* ctrl_;
            void (SetBase::*setter_)(SetArg);
        };

        template <class Ctrl, class SetBase, class SetArg, class GetBase, class GetRet, class... EvtArgs>
        class two_way_binder
        {
        public:
            two_way_binder(Ctrl& ctrl, void (SetBase::*setter)(SetArg), GetRet (GetBase::*getter)() const,
                           maui::core::event<EvtArgs...>* evt)
                : ctrl_(&ctrl), setter_(setter), getter_(getter), evt_(evt)
            {
            }

            // One-way (source -> control) only: ignore the getter/event.
            template <class T> [[nodiscard]] maui::core::binding_handle to(maui::core::observable<T>& source) &&
            {
                return one_way_binder<Ctrl, SetBase, SetArg>(*ctrl_, setter_).to(source);
            }

            // Two-way (source <-> control), reusing the core ladder's shape: initial push, both
            // subscriptions, a shared re-entrancy guard, RAII teardown.
            template <class T> [[nodiscard]] maui::core::binding_handle to_two_way(maui::core::observable<T>& source) &&
            {
                auto guard = std::make_shared<bool>(false);
                std::vector<maui::core::scoped_connection> conns;

                (ctrl_->*setter_)(static_cast<SetArg>(source.get())); // initial push source -> control

                conns.push_back(maui::core::connect_scoped(
                    source.changed(), [ctrl = ctrl_, setter = setter_, guard](const T& /*old*/, const T& nv) {
                        if (*guard)
                        {
                            return;
                        }
                        *guard = true;
                        (ctrl->*setter)(static_cast<SetArg>(nv));
                        *guard = false;
                    }));

                conns.push_back(maui::core::connect_scoped(
                    *evt_, [ctrl = ctrl_, getter = getter_, src = &source, guard](const EvtArgs&... /*args*/) {
                        if (*guard)
                        {
                            return;
                        }
                        *guard = true;
                        src->set(static_cast<T>((ctrl->*getter)()));
                        *guard = false;
                    }));

                return maui::core::binding_handle(std::move(conns));
            }

        private:
            Ctrl* ctrl_;
            void (SetBase::*setter_)(SetArg);
            GetRet (GetBase::*getter_)() const;
            maui::core::event<EvtArgs...>* evt_;
        };
    } // namespace detail

    // The setter (and getter) may be declared on Ctrl OR on any base of Ctrl, so a binding can target an
    // inherited control property (e.g. set_is_enabled on the view<> base) — `&button::set_is_enabled` is a
    // base-class member pointer. is_base_of<X, Ctrl> is true when X == Ctrl, so the common leaf case still binds.
    template <class Ctrl, class SetBase, class SetArg>
        requires std::is_base_of_v<SetBase, Ctrl>
    [[nodiscard]] detail::one_way_binder<Ctrl, SetBase, SetArg> bind(Ctrl& ctrl, void (SetBase::*setter)(SetArg))
    {
        return detail::one_way_binder<Ctrl, SetBase, SetArg>(ctrl, setter);
    }

    template <class Ctrl, class SetBase, class SetArg, class GetBase, class GetRet, class... EvtArgs>
        requires std::is_base_of_v<SetBase, Ctrl> && std::is_base_of_v<GetBase, Ctrl>
    [[nodiscard]] detail::two_way_binder<Ctrl, SetBase, SetArg, GetBase, GetRet, EvtArgs...> bind(
        Ctrl& ctrl, void (SetBase::*setter)(SetArg), GetRet (GetBase::*getter)() const,
        maui::core::event<EvtArgs...>* evt)
    {
        return detail::two_way_binder<Ctrl, SetBase, SetArg, GetBase, GetRet, EvtArgs...>(ctrl, setter, getter, evt);
    }
} // namespace maui::ui
