#pragma once
// maui::samples::templated_view_page — ports TemplatedViewPage.xaml
//
// The C# page contrasts a standard CardView control with a compact one driven by a ControlTemplate
// ("CardViewCompressed") and a custom Rate control built entirely from a ControlTemplate + a heart
// PathGeometry. CardView and Rate are *sample-app* controls (Maui.Controls.Sample.Controls.*), not
// framework types, so the faithful framework-only port demonstrates the underlying machinery the page
// is really showing off: TemplatedView / ControlTemplate via content_view (controls/content_view.hpp +
// controls/templates/control_template.hpp), the exact seam content_view sits on.
//
// PORT MAPPING:
//   - the un-templated "standard CardView"  -> a content_view with NO ControlTemplate whose Content is
//     a small card stack (title + description). content_view presents its developer Content directly
//     when untemplated (ContentView.PresentedContent = TemplateRoot ?? Content).
//   - the "CardViewCompressed" ControlTemplate  -> control_template::of<compact_card_template>(), a
//     loader-minted view subtree (a horizontal stack: an icon block + a title + a content_presenter).
//     Applying it via content_view::set_control_template runs the same
//     template_utilities::on_control_template_changed the C# ControlTemplateProperty change callback
//     runs, parenting the template root as the content_view's one internal logical child, and the
//     presenter "packs" the content_view's developer Content (the content_view_tests "packs the
//     content" behaviour).
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree and attaches every owned view
// bottom-up (the value_controls_page / shapes_page convention).
//
// note: {TemplateBinding CardTitle/CardDescription} (per-property template bindings) and the custom
//       Rate value control + its heart PathGeometry are sample-app-specific; they are not reproduced.
//       What this page demonstrates is the ControlTemplate application + content packing they all rely
//       on. Each card's body is co-owned via shared_ptr (the templated currency) and so is reached
//       through *() in attach_handlers.

#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/font.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/corner_radius.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    // The "CardViewCompressed" ControlTemplate root (TemplatedViewPage.xaml's <ControlTemplate
    // x:Key="CardViewCompressed">): a horizontal stack — a fixed icon block beside a heading + a
    // content_presenter. control_template::of<compact_card_template>() mints one of these as the
    // content_view's single internal logical child when the template is applied, and the presenter
    // packs the content_view's developer Content (mirroring the compressed card hosting its body). The
    // template-content double pattern is content_view_tests' simple_template.
    class compact_card_template : public maui::controls::horizontal_stack_layout
    {
    public:
        compact_card_template()
        {
            set_spacing(8);

            // The square icon block (Image WidthRequest/HeightRequest=100 BackgroundColor=SlateGray in
            // the XAML) — a SlateGray box_view stands in for the icon image.
            icon_.set_color(maui::graphics::color::from_rgb(112, 128, 144)); // SlateGray #708090
            icon_.set_corner_radius(maui::graphics::corner_radius(6));
            icon_.set_width_request(100);
            icon_.set_height_request(100);

            heading_.set_text("Compact card");
            heading_.set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));

            // The presenter into which the content_view packs its developer Content (the card body).
            presenter_ = std::make_shared<maui::controls::content_presenter>();

            column_.set_spacing(4);
            column_.add(heading_);
            column_.add(*presenter_);

            add(icon_);
            add(column_);
        }

        [[nodiscard]] const std::shared_ptr<maui::controls::content_presenter>& presenter() const
        {
            return presenter_;
        }

        // Attach a handler to every view in this MINTED template subtree (the page can't reach these
        // by member — it only holds the content_view card, then the template root through it), then
        // re-host the layouts whose "add" commands fired in this ctor before any handler existed. The
        // presenter's PACKED content (the card body) is owned + attached + re-hosted by the page, which
        // re-fires this presenter's "set_content" afterward (gallery_attach.hpp convention).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, icon_, "compact_icon_");
            gallery_attach_one(app, heading_, "compact_heading_");
            gallery_attach_one(app, *presenter_, "compact_presenter_");
            gallery_attach_one(app, column_, "compact_column_");
            // The template root IS a horizontal_stack_layout subclass; attach it via that REGISTERED
            // base type (the handler registry keys on the exact static type, and there is no handler for
            // the subclass — layout_handler drives it through the i_layout interface regardless).
            gallery_attach_one(app, static_cast<maui::controls::horizontal_stack_layout&>(*this),
                               "compact_template_root_");

            gallery_rehost_layout(column_); // the column hosts the heading + the presenter
            gallery_rehost_layout(*this);   // the hstack hosts the icon block + the column
        }

    private:
        maui::controls::box_view icon_;
        maui::controls::vertical_stack_layout column_;
        maui::controls::label heading_;
        std::shared_ptr<maui::controls::content_presenter> presenter_;
    };

    class templated_view_page
    {
    public:
        templated_view_page()
        {
            page_.set_title("TemplatedView");
            root_.set_padding(maui::core::thickness(12)); // StackLayout Padding="12".
            root_.set_spacing(10);

            // The four explanatory red-italic labels (the XAML's <Label FontAttributes="Italic"
            // TextColor="Red"> captions). Built up-front so each precedes its card.
            caption_standard_.set_text("A standard CardView control is suitable for grid layouts:");
            caption_compact_.set_text("A ControlTemplate overrides the standard view, creating a more compact view:");
            for (maui::controls::label* caption : {&caption_standard_, &caption_compact_})
            {
                caption->set_text_color(maui::graphics::color::from_rgb(255, 0, 0)); // Red.
                caption->set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::regular,
                                                                          maui::core::font_slant::italic));
            }

            // ---- The standard (un-templated) card: content_view shows its Content directly ----
            standard_card_.set_content(make_card_body("Slavko Vlasic", "Lorem ipsum dolor sit amet, consectetur "
                                                                       "adipiscing elit. Nulla elit dolor."));

            // ---- The three compact cards: each content_view applies the CardViewCompressed template ----
            add_compact_card("Carolina Pena", "Phasellus eu convallis mi. In tempus augue eu dignissim fermentum.");
            add_compact_card("Wade Blanks",
                             "Aliquam sagittis, odio lacinia fermentum dictum, mi erat scelerisque erat.");
            add_compact_card("Colette Quint", "In pellentesque odio eget augue elementum lobortis. Sed augue massa.");

            root_.add(caption_standard_);
            root_.add(standard_card_);
            root_.add(caption_compact_);
            for (const auto& card : compact_cards_)
            {
                root_.add(*card);
            }
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP, then re-host the tree built in the ctor.
        // Unlike the leaf-content pages, each card body is a NESTED layout (a stack of two labels) and
        // each compact card additionally hosts a MINTED ControlTemplate subtree, so attachment must
        // recurse into BOTH (the label leaves + the template's icon/heading/column/presenter) and
        // re-host every intermediate layout — not just the cards. Otherwise the bodies render as empty
        // panels (their leaves never hosted) and the template subtrees never mount (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& view, const char* name) { gallery_attach_one(app, view, name); };

            one(caption_standard_, "caption_standard_");
            one(caption_compact_, "caption_compact_");

            // Each card body's leaf labels first, then the body stacks themselves (so the body panels
            // have hostable children once re-hosted below).
            for (const auto& leaf : card_leaves_)
            {
                one(*leaf, "card_leaf");
            }
            for (const auto& body : card_bodies_)
            {
                one(*body, "card_body");
            }

            // The standard (un-templated) card hosts its body directly.
            one(standard_card_, "standard_card_");

            // The compact cards: attach the card, then its minted CardViewCompressed template subtree
            // (reached through the card's template_root() — the page holds no direct member for it).
            for (const auto& card : compact_cards_)
            {
                one(*card, "compact_card");
                if (auto* tmpl = dynamic_cast<compact_card_template*>(card->template_root()))
                {
                    tmpl->attach_handlers(app);
                }
            }

            one(root_, "root_");
            one(page_, "page_");

            // Re-host bottom-up: each body's children first, then each compact presenter packs its body,
            // then the cards host their presented content (the body, or the template root), then the
            // root and the page (each via its sibling host command).
            for (const auto& body : card_bodies_)
            {
                gallery_rehost_layout(*body);
            }
            for (const auto& card : compact_cards_)
            {
                if (auto* tmpl = dynamic_cast<compact_card_template*>(card->template_root()))
                {
                    gallery_rehost_content(*tmpl->presenter()); // the presenter packs the card body
                }
            }
            gallery_rehost_content(standard_card_);
            for (const auto& card : compact_cards_)
            {
                gallery_rehost_content(*card);
            }
            gallery_rehost_layout(root_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's inspection.
        [[nodiscard]] maui::controls::vertical_stack_layout& root()
        {
            return root_;
        }
        [[nodiscard]] maui::controls::content_view& standard_card()
        {
            return standard_card_;
        }
        [[nodiscard]] const std::vector<std::shared_ptr<maui::controls::content_view>>& compact_cards() const
        {
            return compact_cards_;
        }

    private:
        // Build a card body (a title + a description label in a vertical stack) and co-own it (the
        // content_view co-owns its Content; we also keep a copy so attach_handlers can reach the labels).
        std::shared_ptr<maui::controls::vertical_stack_layout> make_card_body(const std::string& title,
                                                                              const std::string& description)
        {
            auto body = std::make_shared<maui::controls::vertical_stack_layout>();
            body->set_spacing(2);

            auto title_label = std::make_shared<maui::controls::label>();
            title_label->set_text(title);
            title_label->set_font(maui::core::font::system_font_of_weight(maui::core::font_weight::bold));

            auto description_label = std::make_shared<maui::controls::label>();
            description_label->set_text(description);

            body->add(*title_label);
            body->add(*description_label);

            // Keep the leaves alive + reachable for handler attachment.
            card_leaves_.push_back(std::move(title_label));
            card_leaves_.push_back(std::move(description_label));
            card_bodies_.push_back(body);
            return body;
        }

        // A compact card: a content_view whose Content is a card body and whose ControlTemplate is the
        // CardViewCompressed template (control_template::of mints the template root; the presenter packs
        // the body). Applying set_control_template runs template_utilities::on_control_template_changed.
        void add_compact_card(const std::string& title, const std::string& description)
        {
            auto card = std::make_shared<maui::controls::content_view>();
            card->set_control_template(maui::controls::control_template::of<compact_card_template>());
            card->set_content(make_card_body(title, description));
            compact_cards_.push_back(std::move(card));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;
        maui::controls::label caption_standard_;
        maui::controls::label caption_compact_;
        maui::controls::content_view standard_card_; // the un-templated standard card

        // The compact cards + every co-owned body / leaf so the whole tree stays alive and attachable.
        std::vector<std::shared_ptr<maui::controls::content_view>> compact_cards_;
        std::vector<std::shared_ptr<maui::controls::vertical_stack_layout>> card_bodies_;
        std::vector<std::shared_ptr<maui::controls::label>> card_leaves_;
    };
} // namespace maui::samples
