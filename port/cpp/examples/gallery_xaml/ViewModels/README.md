# ViewModels/

Kept to mirror the MAUI project layout (`Views/` + `ViewModels/` + `Models/`), but intentionally empty: the
gallery twins are **purely structural** demo pages. Any data they show is inline in the markup (`x:Array`),
so none binds to a view-model — each page builds with `maui::no_view_model`.

A twin that needed real data/commands would add its view-model here and switch its `Views/<name>.xaml.cpp`
to `build_page<MyViewModel, …>()` + `page->bind_to(...)`, exactly as [`counter_xaml`](../../counter_xaml)
does (see its `ViewModels/counter_view_model.hpp` and the click-wiring in `counter.xaml.cpp`).
