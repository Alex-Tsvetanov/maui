#pragma once
// data_binding.xaml.hpp — the page-construction declaration for the data_binding page. Takes ownership of
// the view-model like the spec's `LoginPage(std::unique_ptr<LoginViewModel>)`. Bodyless: the implementation
// — #embed of data_binding.xaml, build_page, and bind_to (which makes the markup {Binding}s go live) — lives
// in the separate TU data_binding.xaml.cpp.

#include <memory>

#include "maui/controls/content_page.hpp"

#include "ViewModels/greeting_view_model.hpp"

namespace examples::Views
{
    [[nodiscard]] std::unique_ptr<maui::controls::content_page> data_binding_page(
        std::unique_ptr<ViewModels::greeting_view_model> vm);
} // namespace examples::Views
