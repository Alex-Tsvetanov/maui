# data_binding_xaml — the with-XAML twin of data_binding, via COMPILE-TIME XAML (#embed + maui::build_page),
# structured like a MAUI .xaml/.xaml.cs app: main.cpp (app shell) + data_binding.xaml.cpp (the page factory,
# a separate TU owning the #embed). data_binding.xaml is fully markup-bound; bind_to makes the {Binding}s go
# live. No codegen tool, so it builds on every backend including the iOS bundle.
maui_add_app(data_binding_xaml SOURCES main.cpp Views/data_binding.xaml.cpp)
# Views/... and ViewModels/... includes resolve from the example root.
target_include_directories(data_binding_xaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# build_page hydrates the embedded markup (and registers the runtime {Binding} applier) -> link the XAML layer.
target_link_libraries(data_binding_xaml PRIVATE maui::xaml)
# data_binding.xaml.cpp uses #embed (a C23/C++26 feature clang accepts under -std=c++23 but warns on).
target_compile_options(data_binding_xaml PRIVATE -Wno-c23-extensions)
