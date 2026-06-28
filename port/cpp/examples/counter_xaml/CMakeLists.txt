# counter_xaml — the with-XAML twin of counter, via COMPILE-TIME XAML (#embed + maui::build_page), structured
# like a MAUI .xaml/.xaml.cs app: main.cpp (app shell) + counter.xaml.cpp (the page factory + code-behind, a
# separate TU owning the #embed). No codegen tool, so it builds on every backend including the iOS bundle.
maui_add_app(counter_xaml SOURCES main.cpp Views/counter.xaml.cpp)
# The example root is on the include path so the Views/... and ViewModels/... includes resolve.
target_include_directories(counter_xaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# build_page hydrates the embedded markup through the runtime loader -> link the XAML layer.
target_link_libraries(counter_xaml PRIVATE maui::xaml)
# counter.xaml.cpp uses #embed (a C23/C++26 feature clang accepts under -std=c++23 but warns on).
target_compile_options(counter_xaml PRIVATE -Wno-c23-extensions)
