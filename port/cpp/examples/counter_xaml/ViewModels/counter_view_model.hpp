#pragma once
// counter_view_model.hpp — the view-model for the counter page, in its own header like the spec's
// LoginViewModel.hpp. A member-free view-model: only a bindable cell + a command (no widget instances, no
// fixed ordering — the verbose-tree complaint this whole layer answers).

#include "maui/command.hpp"
#include "maui/property.hpp"

namespace examples::ViewModels
{
    struct counter_view_model
    {
        maui::property<int> Count{0};
        maui::command Increment;
        counter_view_model() : Increment{[this] { Count(Count() + 1); }}
        {
        }
    };
} // namespace examples::ViewModels
