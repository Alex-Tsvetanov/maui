#pragma once
// maui/maui_main.hpp — the ONE convenience header a framework user includes to get a working main().
//
// Usage (PURE C++ — no Objective-C, no .mm, no platform headers anywhere in the user's code):
//
//     #include "maui/maui_main.hpp"
//     #include "maui/controls/application.hpp"
//     // ... your controls ...
//
//     class my_app : public maui::controls::application {
//     public:
//         my_app() { /* build window -> page -> content tree */ }
//         maui::core::i_window* create_window() override { return &window_; }
//     private:
//         maui::controls::window window_;
//         // ... owned page + controls ...
//     };
//
//     // The ONE function the USER defines (the MauiProgram.CreateMauiApp shape): receive a fresh builder,
//     // register your application, return it. Called by run_app via the main() below.
//     maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder b) {
//         b.use_maui_app<my_app>();
//         return b;
//     }
//
// Including this header DEFINES main() for you (it forwards to run_app on whichever backend is linked).
//
// INCLUDE IN EXACTLY ONE TRANSLATION UNIT (like Catch2's CATCH_CONFIG_MAIN / GoogleTest's gtest_main):
// this header defines main() and DECLARES use_shared_maui_app, so including it in two TUs is a duplicate-
// symbol link error on main(). Every OTHER TU includes only the maui/hosting + control headers it needs.
//
// This header is PURE C++: it pulls in only host_run.hpp + maui_app_builder.hpp (both platform-free); the
// backend-specific run loop lives behind run_app (host_run.hpp), selected at link time. Nothing here names
// a platform type.

#include "maui/hosting/host_run.hpp"
#include "maui/hosting/maui_app_builder.hpp"

// The user-supplied configurator (defined in the user's single main TU — see the usage note above). Declared
// here so the main() below can take its address; the user provides the body.
maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder);

// The portable entry point. Forwards to the backend's run_app with the user's configurator. The user never
// writes main() themselves — including this header is the opt-in (one TU only).
int main(int argc, char** argv)
{
    return ::maui::hosting::run_app(argc, argv, &use_shared_maui_app);
}
