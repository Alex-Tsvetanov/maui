# hello_world_xaml — the with-XAML twin of hello_world, via COMPILE-TIME XAML (#embed + maui::build_page),
# structured like a MAUI .xaml/.xaml.cs app: main.cpp (app shell) + hello_world.xaml.cpp (the page factory,
# a separate TU owning the #embed). No codegen tool, so it builds on every backend including the iOS bundle.
maui_add_app(hello_world_xaml SOURCES main.cpp Views/hello_world.xaml.cpp)
# Resolve the Views/... (and ViewModels/...) includes from the example root.
target_include_directories(hello_world_xaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# build_page hydrates the embedded markup through the runtime loader -> link the XAML layer.
target_link_libraries(hello_world_xaml PRIVATE maui::xaml)
# hello_world.xaml.cpp uses #embed (a C23/C++26 feature clang accepts under -std=c++23 but warns on).
target_compile_options(hello_world_xaml PRIVATE -Wno-c23-extensions)
