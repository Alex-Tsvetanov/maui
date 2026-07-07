#pragma once
// maui::samples::carousel_page — the code-first twin of the shared port/maui-reference/pages/carousel_page.xaml.
//
// The shared XAML is a bare CarouselView (the ContentPage's only content) whose ItemsSource is an inline
// x:Array of three strings and whose ItemTemplate is a purple-stroked Border (StrokeThickness 2, Padding 16)
// wrapping a centered Label with the LITERAL text "Card":
//
//   <CarouselView>
//     <CarouselView.ItemsSource><x:Array Type="{x:Type x:String}">Card 1/2/3</x:Array></CarouselView.ItemsSource>
//     <CarouselView.ItemTemplate><DataTemplate>
//       <Border Stroke="Purple" StrokeThickness="2" Padding="16">
//         <Label Text="Card" HorizontalTextAlignment="Center" VerticalTextAlignment="Center" />
//       </Border>
//     </DataTemplate></CarouselView.ItemTemplate>
//   </CarouselView>
//
// A CarouselView shows ONE item at a time, so the resting page is a single full-viewport purple-bordered card
// reading "Card". This twin mirrors that EXACTLY (page root = the CarouselView; no headline / Prev-Next /
// readout chrome — those were an earlier richer-demo divergence that made the builder column diverge from
// MAUI on the board; carousel_page is now structurally strict, removed from the equivalence gate's
// known_diverging list).
//
// The item cell is a data_template::of<carousel_card>() — carousel_card is a border subclass that OWNS its
// Label child as a member (layout/border content is a NON-OWNING reference — PROFILE §8 — so a template cell
// that adds a freshly-created child must own it), the same composite-cell pattern as cv_visual_states_page's
// line_item_cell and header_footer_template_page's photo_cell. The Label text is the literal "Card" (the XAML
// binds no item value into it), so the cell needs no binding. carousel_card is a brand-new user type whose
// handler isn't self-registered; register_handlers() maps it to border_handler in the app's per-app registry
// BEFORE the collection_view realize walk (gallery_pre_mount), exactly like line_item_cell -> layout_handler.

#include <memory>
#include <string>
#include <vector>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/items/carousel_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/border_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    class carousel_page
    {
    public:
        // The ItemTemplate cell root: a Border (Stroke=Purple, StrokeThickness=2, Padding=16) owning a
        // centered "Card" Label. Default-constructible so data_template::of<carousel_card>() activates it.
        // The Label fills the Border's content rect, so its H/V text alignment centers "Card" in the card.
        class carousel_card final : public maui::controls::border
        {
        public:
            carousel_card()
            {
                set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::purple));
                set_stroke_thickness(2);
                set_padding(maui::core::thickness(16));
                label_.set_text("Card");
                label_.set_horizontal_text_alignment(maui::core::text_alignment::center);
                label_.set_vertical_text_alignment(maui::core::text_alignment::center);
                set_content(label_);
            }

        private:
            maui::controls::label label_;
        };

        carousel_page()
            : items_(std::make_shared<maui::core::observable_collection<std::string>>(
                  std::vector<std::string>{"Card 1", "Card 2", "Card 3"})) // the x:Array of x:String
        {
            page_.set_title("CarouselView");

            carousel_.set_item_template(maui::controls::data_template::of<carousel_card>());
            carousel_.set_items_source(items_);

            // The ContentPage's only content IS the CarouselView (the shared XAML root).
            page_.set_content(carousel_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        [[nodiscard]] maui::controls::carousel_view& carousel()
        {
            return carousel_;
        }

        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<std::string>>& items() const
        {
            return items_;
        }

        // PRE-MOUNT hook (gallery_host.hpp gallery_pre_mount): register carousel_card's handler BEFORE the
        // collection_view realize walk. carousel_card is a border subclass, so it shares border's handler;
        // without this the native cell realize silently no-ops (no registered handler for its type_tag) and
        // the carousel renders blank. Mirrors cv_visual_states_page::register_handlers.
        void register_handlers(maui::hosting::maui_app& app)
        {
            maui::core::register_handler<carousel_card, maui::core::border_handler>(app.handlers());
        }

    private:
        std::shared_ptr<maui::core::observable_collection<std::string>> items_; // publisher before the view (§8)
        maui::controls::content_page page_;
        maui::controls::carousel_view carousel_;
    };
} // namespace maui::samples
