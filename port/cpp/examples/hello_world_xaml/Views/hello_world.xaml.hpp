#pragma once
// hello_world.xaml.hpp — the page-construction declaration (the C++ analog of a .xaml.cs partial's
// generated factory). Bodyless on purpose: the implementation — the #embed of hello_world.xaml and the
// build_page call — lives in the separate translation unit hello_world.xaml.cpp, so main.cpp depends only
// on this declaration and never sees #embed/build_page. This page is purely structural (no view-model).

#include <memory>

#include "maui/controls/content_page.hpp"

namespace examples::Views
{
    [[nodiscard]] std::unique_ptr<maui::controls::content_page> hello_world_page();
} // namespace examples::Views
