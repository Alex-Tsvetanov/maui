#pragma once
// counter.xaml.hpp — the page-construction declaration for the counter page (the C++ analog of a .xaml.cs
// partial's generated factory). It takes ownership of the view-model, exactly like the spec's
// `LoginPage(std::unique_ptr<LoginViewModel>)`. Bodyless: the implementation — #embed of counter.xaml,
// build_page, bind_to, and the x:Name code-behind wiring — lives in the separate TU counter.xaml.cpp.

#include <memory>

#include "maui/controls/content_page.hpp"

#include "ViewModels/counter_view_model.hpp"

namespace examples::Views
{
    [[nodiscard]] std::unique_ptr<maui::controls::content_page> counter_page(
        std::unique_ptr<ViewModels::counter_view_model> vm);
} // namespace examples::Views
