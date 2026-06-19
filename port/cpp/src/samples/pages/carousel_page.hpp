#pragma once
// maui::samples::carousel_page — ports CarouselViewGalleries/CarouselViewPage.xaml (+ .xaml.cs).
//
// The original page (CarouselViewPage): a BasePage titled "CarouselView" with a StackLayout (Margin 12)
// holding a Headline Label "Basic Horizontal Carousel" over a CarouselView whose ItemsSource is the
// string array {"Item 1","Item 2","Item 3"} and whose ItemTemplate is a Grid with a centered Large Label
// bound to {Binding} (the item string), plus a TapGestureRecognizer that, on tap, shows a DisplayAlert
// ("Item", "Tapped", "Successfully").
//
// This port mirrors that shape code-first:
//   - the carousel_view is fed the three string items via set_items_source(observable_collection<string>)
//     (items_view::set_items_source — the typed live source over the erased seam);
//   - the ItemTemplate is a centered Label bound to the item string itself ({Binding} → identity), the
//     items_page identity-binding precedent (set_binding<std::string,std::string> returning the value);
//   - Next / Prev buttons mutate carousel.Position (carousel_view::set_position) — the demo affordance the
//     task calls for (the oracle has no buttons; on a device the user swipes). Next/Prev clamp to the item
//     range; a Position change drives the carousel's command → event → hook choreography;
//   - a CurrentItem readout label reports the settled item: subscribe to `current_item_changed` and render
//     the new boxed item's text. set_position writes Position; the headless carousel keeps CurrentItem in
//     step with Position through the same items virtualization the collection_view uses, so the readout
//     tracks the visible item. The readout also seeds from position 0 at build time.
//
// The page OWNS its whole element tree (the items_page pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless
// carousel reuses the collection_view virtualization simulator, so a static capture realizes the item
// cells and the position/current-item state the readout reflects.
//
// note: the C# cell's TapGestureRecognizer → DisplayAlert("Item","Tapped","Successfully") is a per-cell
//       gesture firing a modal alert. A templated cell's gesture has no headless realization (the
//       simulator does not synthesize taps on virtualized cells) and DisplayAlert is a page-modal service;
//       both are OMITTED here (best-effort: the Next/Prev + readout demonstrate the carousel's core
//       Position/CurrentItem behavior the page is built around). Not invented — left as a documented gap.
// note: the C# Headline style + Margin="12" are cosmetic; the headline text is preserved as a plain Label,
//       the margin left default (no headless effect).

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class carousel_page
    {
    public:
        carousel_page()
            : items_(std::make_shared<maui::core::observable_collection<std::string>>(
                  std::vector<std::string>{"Item 1", "Item 2", "Item 3"})) // C# x:Array of x:String
        {
            page_.set_title("CarouselView");
            root_.set_spacing(12); // stands in for the StackLayout Margin="12" gutter

            headline_.set_text("Basic Horizontal Carousel"); // the Headline Label

            // ItemTemplate: a centered Label bound to {Binding} — the item string itself (identity bind).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, std::string>(maui::controls::label::text_property(),
                                                        [](const std::string& value) { return value; });
            carousel_.set_item_template(cell);
            carousel_.set_items_source(items_);

            // CurrentItem readout: track the settled item as Position changes.
            carousel_.current_item_changed.connect(
                [this](const maui::controls::current_item_changed_event_args& args) { update_readout(args); });
            seed_readout(); // position 0 at build time

            // Next / Prev buttons mutate Position (the demo affordance — see header note).
            prev_button_.set_text("Prev");
            prev_button_.command = [this] { go_prev(); };
            next_button_.set_text("Next");
            next_button_.command = [this] { go_next(); };

            buttons_.set_spacing(12);
            buttons_.add(prev_button_);
            buttons_.add(next_button_);

            root_.add(headline_);
            root_.add(carousel_);
            root_.add(buttons_);
            root_.add(readout_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, headline_, "headline_");
            gallery_attach_one(app, carousel_, "carousel_");
            gallery_attach_one(app, prev_button_, "prev_button_");
            gallery_attach_one(app, next_button_, "next_button_");
            gallery_attach_one(app, buttons_, "buttons_");
            gallery_attach_one(app, readout_, "readout_");
            gallery_attach_one(app, root_, "root_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_layout(buttons_); // the Prev/Next row
            gallery_rehost_layout(root_);    // headline + carousel + buttons + readout
            gallery_rehost_content(page_);
        }

        // ---- owned controls exposed for the hosting main / tests ----
        [[nodiscard]] maui::controls::carousel_view& carousel()
        {
            return carousel_;
        }
        [[nodiscard]] maui::controls::button& prev_button()
        {
            return prev_button_;
        }
        [[nodiscard]] maui::controls::button& next_button()
        {
            return next_button_;
        }
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<std::string>>& items() const
        {
            return items_;
        }

        // Advance one item, clamped to the last index (the carousel snaps to a valid position).
        void go_next()
        {
            const int last = item_count() - 1;
            if (last < 0)
            {
                return;
            }
            const int target = carousel_.position() + 1;
            carousel_.set_position(target > last ? last : target);
        }

        // Step back one item, clamped to the first index.
        void go_prev()
        {
            const int target = carousel_.position() - 1;
            carousel_.set_position(target < 0 ? 0 : target);
        }

    private:
        [[nodiscard]] int item_count() const
        {
            return static_cast<int>(items_->items().size());
        }

        void update_readout(const maui::controls::current_item_changed_event_args& args)
        {
            const std::string item = args.current_item.has_value() ? args.current_item.text() : std::string{"(none)"};
            readout_.set_text("Position " + std::to_string(carousel_.position()) + " — current: " + item);
        }

        // Seed the readout from position 0 (the carousel's initial item) at build time.
        void seed_readout()
        {
            const std::string first = item_count() > 0 ? items_->items().front() : std::string{"(none)"};
            readout_.set_text("Position 0 — current: " + first);
        }

        std::shared_ptr<maui::core::observable_collection<std::string>> items_; // publisher before the view (§8)
        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label headline_;
        maui::controls::carousel_view carousel_;
        maui::controls::horizontal_stack_layout buttons_; // a simple Prev/Next row
        maui::controls::button prev_button_;
        maui::controls::button next_button_;
        maui::controls::label readout_;
    };
} // namespace maui::samples
