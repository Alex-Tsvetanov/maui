# layouts_xaml — the with-XAML twin of layouts, via COMPILE-TIME XAML (#embed + maui::build_page), structured
# like a MAUI .xaml/.xaml.cs app: main.cpp (app shell) + layouts.xaml.cpp (the page factory, a separate TU
# owning the #embed). The runtime loader handles the grid definitions + Grid.Row/Column attached placement,
# so no codegen tool is needed and it builds on every backend including the iOS bundle.
maui_add_app(layouts_xaml SOURCES main.cpp Views/layouts.xaml.cpp)
target_include_directories(layouts_xaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# build_page hydrates the embedded markup through the runtime loader -> link the XAML layer.
target_link_libraries(layouts_xaml PRIVATE maui::xaml)
# layouts.xaml.cpp uses #embed (a C23/C++26 feature clang accepts under -std=c++23 but warns on).
target_compile_options(layouts_xaml PRIVATE -Wno-c23-extensions)
