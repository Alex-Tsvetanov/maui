#pragma once
// maui::samples::chat_example_page — ports ChatExample.xaml (+ .xaml.cs)
//   (Maui.Controls.Sample.Pages.CollectionViewGalleries.ItemSizeGalleries.ChatExample).
//
// The C# page is the variable-height-items demo: a ChatExampleViewModel holds an
// ObservableCollection<ChatMessage>, each message a { Text, IsLocal }. A ChatTemplateSelector picks one of
// two DataTemplates per message — Local (a green, right-aligned bubble in the right 2/3 of a 1*/2* Grid)
// or Remote (a blue, left-aligned bubble in the left 2/3 of a 2*/1* Grid). The CollectionView uses
// ItemSizingStrategy="MeasureAllItems" and a LinearItemsLayout(Vertical, ItemSpacing=5) so every bubble is
// measured at its own height — the messages are random-length Lorem substrings, so cells differ in height.
// Three buttons drive it: Append Random Message, Clear, and "Add 1000 Messages" (which swaps in a fresh VM
// of 1000 messages).
//
// This headless port owns its whole tree and reproduces all of that code-first:
//   - a chat_message { text, is_local } model and an observable_collection<chat_message> source;
//   - a chat_template_selector (a data_template_selector subclass) returning local_template_ vs
//     remote_template_ off message.is_local — the variable-template-per-item core;
//   - the cell label binds Text = message.text, so different-length messages realize different-size cells
//     under the virtualization sim (the "variable item size" the page exists to exercise);
//   - item_sizing_strategy::measure_all_items + a vertical linear_items_layout(item_spacing 5), verbatim;
//   - append_random_message() / clear_messages() / add_lots(n) mirror the three C# buttons (the deterministic
//     pseudo-random length generator stands in for the C# Random so the demo is reproducible headless).
//
// note: the C# Local/Remote DataTemplates wrap the Label in a colored, corner-radiused Frame inside a 1*/2*
//   (or 2*/1*) Grid for the left/right bubble alignment; that geometry + color is cosmetic chrome the
//   headless sim does not render, so each template here is the bound Label (the displayable signal the sim
//   measures). The TWO distinct templates are preserved (a selector really chooses between them per item),
//   which is the behavior this gallery page demonstrates. The C# "Add 1000 Messages" rebuilds the VM and
//   re-points BindingContext; the port mutates the live observable source instead (add_range), the
//   equivalent reload under the items source seam — same 1000 variable-height cells.

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/items/item_sizing_strategy.hpp"
#include "maui/controls/items/items_layout_orientation.hpp"
#include "maui/controls/items/linear_items_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/type_tag.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class chat_example_page
    {
    public:
        // ChatMessage { Text, IsLocal }.
        struct chat_message
        {
            std::string text;
            bool is_local = true;
        };

        // ChatTemplateSelector: IsLocal -> LocalTemplate, else RemoteTemplate (C# OnSelectTemplate, which
        // throws ArgumentOutOfRangeException for a non-ChatMessage item — here a non-chat_message box falls
        // back to the remote template, since the headless source only ever boxes chat_message).
        class chat_template_selector : public maui::controls::data_template_selector
        {
        public:
            std::shared_ptr<maui::controls::data_template> local_template;
            std::shared_ptr<maui::controls::data_template> remote_template;

        protected:
            std::shared_ptr<maui::controls::data_template> on_select_template(
                const item_box& item, maui::core::bindable_object* /*container*/) override
            {
                if (item.value && item.type == maui::core::type_tag::of<chat_message>())
                {
                    const auto* message = static_cast<const chat_message*>(item.value.get());
                    return message->is_local ? local_template : remote_template;
                }
                return remote_template;
            }
        };

        chat_example_page() : messages_(std::make_shared<maui::core::observable_collection<chat_message>>())
        {
            page_.set_title("Chat Example");

            // The Local + Remote DataTemplates: each a Label bound to message.Text (the bubble's text).
            selector_.local_template = make_bubble_template();
            selector_.remote_template = make_bubble_template();

            // ItemTemplate = the ChatTemplateSelector (a data_template_selector IS a data_template, so it
            // slots straight into set_item_template; the handler resolves it per item).
            list_.set_item_template(std::shared_ptr<maui::controls::data_template>(
                std::shared_ptr<void>{}, &selector_)); // non-owning alias: selector_ outlives the list
            list_.set_items_source(messages_);
            list_.set_item_sizing_strategy(maui::controls::item_sizing_strategy::measure_all_items);
            list_.set_items_layout(make_chat_layout());

            // Seed a couple of messages so the page is not empty on first appear (the C# page starts empty
            // and is filled via the buttons; one local + one remote shows both templates immediately).
            messages_->add({"Hi there!", true});
            messages_->add({"Hello — how can I help you today?", false});

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // AppendRandomChatMessage: add one random-length, random-side message (deterministic generator).
        void append_random_message()
        {
            messages_->add(generate_random_message());
        }

        // ClearMessages: ChatMessages.Clear().
        void clear_messages()
        {
            messages_->clear();
        }

        // LotsOfMessages: the C# swaps in a fresh VM of `count` messages; the port appends `count` to the
        // live source (the equivalent reload — same variable-height cells under the sim). Default 1000.
        void add_lots(std::size_t count = 1000)
        {
            std::vector<chat_message> batch;
            batch.reserve(count);
            for (std::size_t n = 0; n < count; ++n)
            {
                batch.push_back(generate_random_message());
            }
            messages_->add_range(std::move(batch));
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp). The list is the page's only content child.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment.
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<chat_message>>& messages() const
        {
            return messages_;
        }

    private:
        // One bubble template: a Label whose Text = message.text (the C# Frame/Grid bubble chrome is
        // cosmetic — see the header note).
        [[nodiscard]] static std::shared_ptr<maui::controls::data_template> make_bubble_template()
        {
            auto bubble = maui::controls::data_template::of<maui::controls::label>();
            bubble->set_binding<std::string, chat_message>(maui::controls::label::text_property(),
                                                           [](const chat_message& value) { return value.text; });
            return bubble;
        }

        // LinearItemsLayout(Orientation=Vertical, ItemSpacing=5).
        [[nodiscard]] static std::shared_ptr<maui::controls::linear_items_layout> make_chat_layout()
        {
            auto layout = std::make_shared<maui::controls::linear_items_layout>(
                maui::controls::items_layout_orientation::vertical);
            layout->set_item_spacing(5.0);
            return layout;
        }

        // The C# GenerateRandomMessage takes a random-length prefix of a Lorem string and a random side.
        // The port uses a deterministic linear-congruential step so the gallery is reproducible headless
        // (no Random/clock dependency) while still producing varied lengths + both sides.
        [[nodiscard]] chat_message generate_random_message()
        {
            static const std::string lorem =
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
                "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
                "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit "
                "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
                "in culpa qui officia deserunt mollit anim id est laborum.";

            rng_ = rng_ * 1103515245U + 12345U; // glibc LCG step (deterministic)
            const bool is_local = ((rng_ >> 16) & 1U) == 1U;
            const std::size_t length = static_cast<std::size_t>((rng_ >> 8) % (lorem.size()));
            return {lorem.substr(0, length), is_local};
        }

        std::shared_ptr<maui::core::observable_collection<chat_message>> messages_; // publisher first (§8)
        chat_template_selector selector_;                                           // outlives the list (aliased in)
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
        unsigned int rng_ = 2463534242U; // deterministic seed
    };
} // namespace maui::samples
