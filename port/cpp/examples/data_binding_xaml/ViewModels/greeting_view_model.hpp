#pragma once
// greeting_view_model.hpp — the view-model for the data_binding page, in its own header like the spec's
// LoginViewModel.hpp. A bindable_object with one observable property whose NAME matches the markup {Binding}
// path ("Message"); a path with no matching registered property simply does not resolve (the runtime
// loader's no-reflection analog of MAUI's binding-path miss).

#include <string>

#include "maui/core/bindable_object.hpp"
#include "maui/core/observable.hpp"

namespace examples::ViewModels
{
    class greeting_view_model : public maui::core::bindable_object
    {
    public:
        maui::core::observable<std::string> Message{*this, "Message", "World"};
    };
} // namespace examples::ViewModels
