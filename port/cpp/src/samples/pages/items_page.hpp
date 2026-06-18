#pragma once
// items_page — a self-contained demo page for the W2-19 items core: a collection_view over a live
// observable items source with a templated cell, single selection driving a readout label, and an
// EmptyView for the cleared state (the C# CollectionView gallery shape, code-first; the
// pickers_page pattern).
//
// The page OWNS its whole element tree. It is backend-agnostic — a sample main attaches handlers
// bottom-up via the hosting layer and hosts page() in a window; the test trees exercise the same
// controls directly.
//
// Interactions demonstrated:
//   - the collection_view renders the task list through a recyclable label template (Text bound to
//     the string item),
//   - selection_mode::single + selection_changed feed the readout,
//   - add_task()/clear_tasks() mutate the live observable collection (the EmptyView appears when
//     cleared).

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/selection_changed_event_args.hpp"
#include "maui/controls/items/selection_mode.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class items_page
    {
    public:
        items_page()
            : tasks_(std::make_shared<maui::core::observable_collection<std::string>>(
                  std::vector<std::string>{"Water the plants", "Review the port", "Ship wave 2"}))
        {
            page_.set_title("Items");
            stack_.set_spacing(12);

            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, std::string>(maui::controls::label::text_property(),
                                                        [](const std::string& value) { return value; });
            list_.set_item_template(cell);
            list_.set_items_source(tasks_);
            list_.set_empty_view(maui::controls::boxed_item::of(std::string{"All done!"}));
            list_.set_header(maui::controls::boxed_item::of(std::string{"Today"}));
            list_.set_selection_mode(maui::controls::selection_mode::single);
            list_.selection_changed.connect(
                [this](const maui::controls::selection_changed_event_args& args) { update_readout(args); });

            update_readout({});

            stack_.add(list_);
            stack_.add(readout_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<std::string>>& tasks() const
        {
            return tasks_;
        }

        void add_task(std::string task)
        {
            tasks_->add(std::move(task));
        }
        void clear_tasks()
        {
            tasks_->clear();
        }

    private:
        void update_readout(const maui::controls::selection_changed_event_args& args)
        {
            readout_.set_text(args.current_selection.empty() ? std::string{"Pick a task"}
                                                             : "Selected: " + args.current_selection.front().text());
        }

        std::shared_ptr<maui::core::observable_collection<std::string>> tasks_; // publisher before the list (§8)
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::collection_view list_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
