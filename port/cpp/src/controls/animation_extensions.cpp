// maui::controls animation extensions — see include/maui/controls/animation_extensions.hpp. Ported
// from src/Controls/src/Core/AnimationExtensions.cs (the HandleTweenerUpdated/Finished static
// handlers become per-run closures that re-resolve the info by (element, handle) and verify the
// tweener's identity — the exact effect of C#'s key lookup + handler detach choreography).
#include "maui/controls/animation_extensions.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "detail/element_animations.hpp"
#include "detail/tweener.hpp"
#include "maui/animations/easing.hpp"
#include "maui/animations/i_animation_manager.hpp"
#include "maui/animations/i_ticker.hpp"
#include "maui/controls/animation.hpp"
#include "maui/controls/element.hpp"
#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/core/service_registry.hpp"

namespace maui::controls
{
    namespace
    {
        // C# AbortAnimation(key)'s while loop: a finished/abort callback may have re-registered an
        // animation under the same key; keep cancelling until the key is gone. Stopping the tweener
        // re-raises finished, but the info is already out of the map so the handler no-ops.
        void abort_named(element& self, const std::string& handle)
        {
            if (!self.has_animation_state())
            {
                return;
            }
            auto& state = self.animation_state();
            while (true)
            {
                const auto found = state.animations.find(handle);
                if (found == state.animations.end())
                {
                    break;
                }
                const detail::animation_info info = std::move(found->second);
                state.animations.erase(found);
                info.tween->stop();
                if (info.finished)
                {
                    info.finished(1.0, true);
                }
            }
        }

        // C# AbortKinetic(key): remove the kinetic's raw-step animation from its manager.
        void abort_kinetic(element& self, const std::string& handle)
        {
            if (!self.has_animation_state())
            {
                return;
            }
            auto& state = self.animation_state();
            const auto found = state.kinetics.find(handle);
            if (found == state.kinetics.end())
            {
                return;
            }
            if (found->second)
            {
                if (const auto manager = found->second->animation_manager())
                {
                    manager->remove(*found->second);
                }
            }
            state.kinetics.erase(found);
        }

        // C# HandleTweenerUpdated: batch the eased callback. The identity check against the raising
        // tweener stands in for C#'s event-handler detach (a stale tweener can never touch a newer
        // run under the same name).
        void handle_tweener_updated(element& owner, const std::string& name, const detail::tweener* source)
        {
            if (!owner.has_animation_state())
            {
                return;
            }
            auto& state = owner.animation_state();
            const auto found = state.animations.find(name);
            if (found == state.animations.end() || found->second.tween.get() != source)
            {
                return;
            }
            owner.batch_begin();
            found->second.callback(found->second.easing_function.ease(found->second.tween->value()));
            owner.batch_commit();
        }

        // C# HandleTweenerFinished: final callback, the repeat decision (suppressed when the system
        // disabled animations), cleanup or restart, then the finished(value, !enabled) completion.
        void handle_tweener_finished(element& owner, const std::string& name, const detail::tweener* source)
        {
            if (!owner.has_animation_state())
            {
                return;
            }
            auto& state = owner.animation_state();
            const auto found = state.animations.find(name);
            if (found == state.animations.end() || found->second.tween.get() != source)
            {
                return;
            }

            const double tweener_value = found->second.tween->value();
            owner.batch_begin();
            found->second.callback(tweener_value);

            bool repeat = false;
            // If the ticker has been disabled (e.g. by power save mode), don't repeat the animation.
            const bool animations_enabled = found->second.manager->ticker().system_enabled();
            if (found->second.repeat && animations_enabled)
            {
                repeat = found->second.repeat();
            }

            if (!repeat)
            {
                // Move the info out before erasing: the local keeps the tweener alive while its
                // stop() re-raises finished (which now finds nothing under the key).
                const detail::animation_info info = std::move(found->second);
                state.animations.erase(found);
                info.tween->stop();
                if (info.finished)
                {
                    info.finished(tweener_value, !animations_enabled);
                }
                owner.batch_commit();
                return;
            }

            if (found->second.finished)
            {
                found->second.finished(tweener_value, !animations_enabled);
            }
            owner.batch_commit();
            found->second.tween->start();
        }

        // C# AnimateInternal<T> at T = double.
        void animate_internal(element& self, std::shared_ptr<maui::animations::i_animation_manager> manager,
                              std::string name, const std::function<double(double)>& transform,
                              animation::step_fn callback,
                              std::uint32_t rate, std::uint32_t length,
                              std::optional<maui::animations::easing> easing_function,
                              animation::finished_with_result_fn finished, animation::repeat_fn repeat)
        {
            abort_named(self, name);

            detail::animation_info info;
            info.rate = rate;
            info.length = length;
            if (easing_function) // animation_info default-initializes the easing to Linear otherwise
            {
                info.easing_function = std::move(*easing_function);
            }
            info.manager = std::move(manager);
            info.callback = [callback = std::move(callback), transform](double f) { callback(transform(f)); };
            if (finished)
            {
                info.finished = [finished = std::move(finished), transform](double f, bool canceled) {
                    finished(transform(f), canceled);
                };
            }
            info.repeat = std::move(repeat);

            auto tween = std::make_shared<detail::tweener>(info.length, info.rate, info.manager);
            element* owner = &self;
            const detail::tweener* tween_identity = tween.get();
            tween->value_updated.connect(
                [owner, name, tween_identity] { handle_tweener_updated(*owner, name, tween_identity); });
            tween->finished.connect(
                [owner, name, tween_identity] { handle_tweener_finished(*owner, name, tween_identity); });
            info.tween = std::move(tween);

            auto& state = self.animation_state();
            const auto [stored, inserted] = state.animations.insert_or_assign(std::move(name), std::move(info));
            stored->second.callback(0.0);
            stored->second.tween->start();
        }
    } // namespace

    std::function<double(double)> interpolate(double start, double end, double reverse_val, bool reverse)
    {
        const double target = reverse ? reverse_val : end;
        return [start, target](double x) { return start + ((target - start) * x); };
    }

    std::shared_ptr<maui::animations::i_animation_manager> get_animation_manager(element& animatable)
    {
        // C# FindMauiContext: this element's handler context, else the first ancestor's.
        for (element* current = &animatable; current != nullptr; current = current->logical_parent())
        {
            const auto* as_element = dynamic_cast<maui::core::i_element*>(current);
            if (as_element == nullptr || !as_element->handler())
            {
                continue;
            }
            auto* context = as_element->handler()->maui_context();
            if (context == nullptr)
            {
                continue;
            }
            // C# GetRequiredService<IAnimationManager> on the first context found.
            return context->services().get_required_service<maui::animations::i_animation_manager>();
        }
        throw std::invalid_argument("get_animation_manager: unable to find an i_animation_manager");
    }

    void animate(element& self, std::string name, const std::shared_ptr<animation>& animation_to_run,
                 std::uint32_t rate, std::uint32_t length, std::optional<maui::animations::easing> easing_function,
                 animation::finished_with_result_fn finished, animation::repeat_fn repeat)
    {
        if (!repeat)
        {
            animate(self, std::move(name), animation_to_run->get_callback(), rate, length, std::move(easing_function),
                    std::move(finished), {});
            return;
        }
        // C#: wrap the repeat decision so a rerun rewinds the composite (ResetChildren).
        auto rerun = [repeat = std::move(repeat), animation_to_run]() {
            const bool val = repeat();
            if (val)
            {
                (*animation_to_run).reset(); // the pointee's Reset (explicit form; not shared_ptr::reset)
            }
            return val;
        };
        animate(self, std::move(name), animation_to_run->get_callback(), rate, length, std::move(easing_function),
                std::move(finished), std::move(rerun));
    }

    void animate(element& self, std::string name, animation::step_fn callback, std::uint32_t rate, std::uint32_t length,
                 std::optional<maui::animations::easing> easing_function, animation::finished_with_result_fn finished,
                 animation::repeat_fn repeat)
    {
        animate(
            self, std::move(name), [](double x) { return x; }, std::move(callback), rate, length,
            std::move(easing_function), std::move(finished), std::move(repeat));
    }

    void animate(element& self, std::string name, const std::function<double(double)>& transform,
                 animation::step_fn callback,
                 std::uint32_t rate, std::uint32_t length, std::optional<maui::animations::easing> easing_function,
                 animation::finished_with_result_fn finished, animation::repeat_fn repeat,
                 std::shared_ptr<maui::animations::i_animation_manager> manager)
    {
        if (!transform)
        {
            throw std::invalid_argument("animate: transform must not be empty");
        }
        if (!callback)
        {
            throw std::invalid_argument("animate: callback must not be empty");
        }
        if (!manager)
        {
            manager = get_animation_manager(self);
        }
        animate_internal(self, std::move(manager), std::move(name), transform, std::move(callback), rate,
                         length, std::move(easing_function), std::move(finished), std::move(repeat));
    }

    void animate_kinetic(element& self, std::string name, maui::core::move_only_function<bool(double, double)> callback,
                         double velocity, double drag, maui::core::move_only_function<void()> finished,
                         std::shared_ptr<maui::animations::i_animation_manager> manager)
    {
        if (!manager)
        {
            manager = get_animation_manager(self);
        }
        abort_kinetic(self, name);

        const double sign = velocity / std::abs(velocity);
        velocity = std::abs(velocity);
        element* owner = &self;

        auto step = [owner, name, callback = std::move(callback), finished = std::move(finished), velocity, drag,
                     sign](std::int64_t step_milliseconds) mutable -> bool {
            const auto ms = static_cast<double>(step_milliseconds);
            velocity -= drag * ms;
            velocity = std::max(0.0, velocity);

            bool result = false;
            if (velocity > 0)
            {
                result = callback(sign * velocity * ms, velocity);
            }
            if (!result)
            {
                if (finished)
                {
                    finished();
                }
                // C#: remove the raw-step animation from the manager + the kinetics table. The erase
                // drops the owning shared_ptr, but the manager's tick snapshot keeps the running
                // animation (and this closure) alive through the call.
                if (owner->has_animation_state())
                {
                    auto& state = owner->animation_state();
                    const auto found = state.kinetics.find(name);
                    if (found != state.kinetics.end())
                    {
                        if (found->second)
                        {
                            if (const auto found_manager = found->second->animation_manager())
                            {
                                found_manager->remove(*found->second);
                            }
                        }
                        state.kinetics.erase(found);
                    }
                }
            }
            return result;
        };

        auto kinetic = std::make_shared<detail::tweener_animation>(std::move(step));
        self.animation_state().kinetics.insert_or_assign(std::move(name), kinetic);
        kinetic->commit(manager);
        if (!manager->ticker().is_running())
        {
            manager->ticker().start();
        }
    }

    bool abort_animation(element& self, std::string_view handle)
    {
        if (!self.has_animation_state())
        {
            return false;
        }
        const std::string key{handle};
        auto& state = self.animation_state();
        if (!state.animations.contains(key) && !state.kinetics.contains(key))
        {
            return false;
        }
        abort_named(self, key);
        abort_kinetic(self, key);
        return true;
    }

    bool animation_is_running(element& self, std::string_view handle)
    {
        return self.has_animation_state() && self.animation_state().animations.contains(std::string{handle});
    }

    batch_scope::batch_scope(element& target) : target_(&target)
    {
        target_->batch_begin();
    }

    batch_scope::~batch_scope()
    {
        target_->batch_commit();
    }
} // namespace maui::controls
