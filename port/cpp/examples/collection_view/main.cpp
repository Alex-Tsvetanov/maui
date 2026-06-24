// collection_view — a scrollable list built from a data source and a cell template.
//
// ONE concept: data-driven lists. Three pieces, exactly as in MAUI:
//   - an observable_collection<T> is the ItemsSource: a change-notifying list. add/remove/clear raise
//     events the collection_view listens to, so the list re-renders as the data changes.
//   - a data_template is the cell recipe. `data_template::of<label>()` mints a label per item; the cell's
//     BindingContext is set to the item, so a binding on the cell resolves against that item.
//   - set_binding(label::text_property()) with NO path is the self-binding (`{Binding .}`): the item
//     itself is the value. Here items are std::string, so each row's label shows its string directly.
//
// For struct items you would instead use the typed selector overload, e.g.
//   tmpl->set_binding<std::string, person>(label::text_property(), [](const person& p){ return p.name; });
//
// 100% PORTABLE C++: no platform headers. Same source builds + runs on headless, macOS, and iOS.

#include "maui/maui_main.hpp"

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/observable_collection.hpp"

#include <memory>
#include <string>

class collection_view_app : public maui::controls::application
{
public:
    collection_view_app()
    {
        // ---- The data source: a change-notifying collection of strings ----
        items_ = std::make_shared<maui::core::observable_collection<std::string>>();
        items_->add("Mercury");
        items_->add("Venus");
        items_->add("Earth");
        items_->add("Mars");
        // A later items_->add(...) / remove_at(...) / clear() would re-render the list automatically.

        // ---- The cell template: one label per item, its text self-bound to the item string ----
        auto cell_template = maui::controls::data_template::of<maui::controls::label>();
        cell_template->set_binding<std::string>(maui::controls::label::text_property());

        // ---- Wire the list: template first, then the source ----
        list_.set_item_template(cell_template);
        list_.set_items_source(items_);

        page_.set_content(list_);
        window_.set_content(page_);
        window_.set_title("Collection View");
    }

    maui::core::i_window* create_window() override
    {
        return &window_;
    }

private:
    std::shared_ptr<maui::core::observable_collection<std::string>> items_;

    maui::controls::window window_;
    maui::controls::content_page page_;
    maui::controls::collection_view list_;
};

maui::hosting::maui_app_builder use_shared_maui_app(maui::hosting::maui_app_builder builder)
{
    builder.use_maui_app<collection_view_app>();
    return builder;
}
