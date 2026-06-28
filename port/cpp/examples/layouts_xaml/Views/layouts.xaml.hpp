#pragma once
// layouts.xaml.hpp — the page-construction declaration for the layouts page. Bodyless: the implementation —
// #embed of layouts.xaml and build_page — lives in the separate TU layouts.xaml.cpp. Purely structural page
// (a vertical stack above a 2x2 Grid), so no view-model.

#include <memory>

#include "maui/controls/content_page.hpp"

namespace examples::Views
{
    [[nodiscard]] std::unique_ptr<maui::controls::content_page> layouts_page();
} // namespace examples::Views
