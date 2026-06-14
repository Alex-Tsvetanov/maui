// maui::controls::shell_navigating_event_args — the deferral state machine. See the header.

#include "maui/controls/shell/shell_navigating_event_args.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "maui/controls/shell/shell_navigating_deferral.hpp"

namespace maui::controls
{
    std::shared_ptr<shell_navigating_deferral> shell_navigating_event_args::get_deferral()
    {
        if (deferral_completed_)
        {
            throw std::runtime_error{"Deferral has already been completed"};
        }
        if (!can_cancel_)
        {
            return nullptr;
        }
        deferred_event_args_ = true;
        deferral_taken_ = true;
        ++deferral_count_;
        // The token pins these args (self) so completing long after the event finds live state.
        auto self = shared_from_this();
        return std::shared_ptr<shell_navigating_deferral>(
            new shell_navigating_deferral([self] { self->decrement_deferral(); }));
    }

    void shell_navigating_event_args::decrement_deferral()
    {
        if (--deferral_count_ == 0)
        {
            deferral_completed_ = true;
            if (deferral_finished_)
            {
                const std::function<void()> continuation = std::exchange(deferral_finished_, nullptr);
                continuation();
            }
        }
    }
} // namespace maui::controls
