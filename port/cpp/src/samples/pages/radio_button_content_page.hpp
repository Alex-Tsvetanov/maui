#pragma once
// maui::samples::radio_button_content_page — ports RadioButtonContentGallery.xaml
//
// A self-contained, code-first demo of the RadioButton.Content surface. It mirrors the C# controls
// gallery page (Pages/Controls/RadioButtonGalleries/RadioButtonContentGallery.xaml): a ScrollView over
// a StackLayout of explanatory labels interleaved with RadioButtons that exercise Content set to a
// string vs. a View, plus the ControlTemplate path that lets a View Content render on any platform.
//
// WHAT MAUI'S PAGE SHOWS (and how this port maps it):
//   1. Content set to a STRING ("Option A", "Option C") — always supported. RadioButton renders the
//      string natively as its title. → radio_button::set_content(std::string), the port's native
//      string-content path (radio_button.hpp). Ported DIRECTLY.
//   2. Content set to a VIEW — displayed only "where supported (e.g. when using ControlTemplates)".
//      The XAML wraps this in OnPlatform: iOS/UWP get a horizontal stack (coffee image + label) as
//      Content; Android/Tizen/WPF fall back to plain text. → The port's radio_button cuts View-Content
//      to the native STRING path only (the View-Content + DefaultTemplate machinery is documented-
//      deferred at the radio_button level, radio_button.hpp). So this port reproduces the *string*
//      fallback the unsupported platforms use, with an honest note about the deferred View path.
//   3. Content as a View under a ControlTemplate — "we can use a View as Content on any platform". This
//      is the real machinery the page is showing off: a ControlTemplate hosts a content_presenter that
//      packs the developer Content (the coffee image), so the View renders regardless of platform. The
//      radio_button itself can't yet take a control_template (deferred), but content_view — the sibling
//      templated_view over the SAME content_presenter seam — can, so the port demonstrates the
//      View-as-Content-through-a-template concept with a content_view carrying a custom ControlTemplate
//      (a horizontal stack: two cross "Line" stand-ins + a content_presenter packing the coffee image),
//      faithfully reproducing the XAML's "weird-looking custom template" group. The Checked/Unchecked
//      VisualState that toggles the CheckedIndicator opacity is part of the radio_button's own visual
//      state machine; here the template is hosted on a content_view (not a radio), so the live
//      checked-state opacity flip is noted as deferred rather than invented.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// attaches handlers bottom-up via the hosting layer and hosts page() in a window; the headless/apple/
// ios test trees exercise the same wiring directly.
//
// note: radio_button View-Content + RadioButton.DefaultTemplate are documented-deferred at the
//       radio_button level (radio_button.hpp). The XAML's <RadioButton ControlTemplate="{x:Static
//       RadioButton.DefaultTemplate}"> and the per-platform <On Platform="iOS,UWP"> View Content are
//       therefore represented via the content_view template seam (the same content_presenter the C#
//       RadioButton template machinery uses), not by mutating radio_button. The cross "Line" shapes are
//       stand-ins (box_view bars) since the page's point is the template+presenter hosting, not the
//       exact glyph. Nothing here is invented beyond those clearly-noted stand-ins.

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/content_view.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/frame.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/stack_layout.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/core/line_break_mode.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    // The XAML's "weird-looking custom template" root: <ControlTemplate><StackLayout
    // Orientation="Horizontal"> two cross Lines + a <ContentPresenter></ControlTemplate>. The two Lines
    // (Button / CheckedIndicator) are stand-in box_view bars; the content_presenter packs the templated
    // parent's developer Content (the coffee image). control_template::of<weird_radio_template>() mints
    // one of these as the content_view's single internal logical child when applied.
    class weird_radio_template : public maui::controls::horizontal_stack_layout
    {
    public:
        weird_radio_template()
        {
            set_spacing(4);

            // <Line x:Name="Button" Stroke="Black" StrokeThickness="2" .../> — the always-visible stroke.
            button_bar_.set_color(maui::graphics::colors::black);
            button_bar_.set_width_request(50);
            button_bar_.set_height_request(2);

            // <Line x:Name="CheckedIndicator" Stroke="Red" Opacity="0" .../> — the checked indicator. In
            // the XAML the Checked VisualState flips its Opacity to 1; that state flip belongs to the
            // radio_button's visual-state machine and is noted as deferred here (this template is hosted
            // on a content_view stand-in, not on a radio_button).
            indicator_bar_.set_color(maui::graphics::colors::red);
            indicator_bar_.set_width_request(50);
            indicator_bar_.set_height_request(2);

            // <ContentPresenter/> — packs the content_view's developer Content (the coffee image).
            presenter_ = std::make_shared<maui::controls::content_presenter>();

            add(button_bar_);
            add(indicator_bar_);
            add(*presenter_);
        }

        [[nodiscard]] const std::shared_ptr<maui::controls::content_presenter>& presenter() const
        {
            return presenter_;
        }

        // Attach a handler to every view in this MINTED template subtree (the page holds no member for
        // these — it only has the content_view card, reaching the template root through it), then re-host
        // the layout whose "add" commands fired in this ctor before any handler existed. The presenter's
        // PACKED content (the coffee image) is owned + attached + re-hosted by the page (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, button_bar_, "weird_button_bar_");
            gallery_attach_one(app, indicator_bar_, "weird_indicator_bar_");
            gallery_attach_one(app, *presenter_, "weird_presenter_");
            // The template root IS a horizontal_stack_layout subclass; attach it via that REGISTERED base
            // type (the handler registry keys on the exact static type — there is no handler for the
            // subclass; layout_handler drives it through the i_layout interface regardless).
            gallery_attach_one(app, static_cast<maui::controls::horizontal_stack_layout&>(*this),
                               "weird_template_root_");

            gallery_rehost_layout(*this); // the hstack hosts the two bars + the presenter
        }

    private:
        maui::controls::box_view button_bar_;
        maui::controls::box_view indicator_bar_;
        std::shared_ptr<maui::controls::content_presenter> presenter_;
    };

    class radio_button_content_page
    {
    public:
        radio_button_content_page()
        {
            page_.set_title("RadioButton Content");
            stack_.set_spacing(8);

            // 1) Content as a string (always supported).
            caption_string_.set_text("We can set 'Content' on a RadioButton to a string:");
            option_a_.set_content("Option A");

            caption_string2_.set_text(
                "If 'Content' is just a String, it will be translated to Text (always supported):");
            option_c_.set_content("Option C");

            // 2) Content as a View, "displayed where supported (e.g., when using ControlTemplates)".
            caption_view_.set_text(
                "If 'Content' is a View, it will be displayed where supported (e.g., when using ControlTemplates):");

            // The XAML's OnPlatform: the View-Content branch (iOS/UWP) is deferred at the radio_button
            // level, so the port reproduces the Android/Tizen/WPF string fallback ("Can't use View for
            // Content on this platform, so just plain old text") inside the Frame, with a note.
            framed_radio_.set_content("Can't use View for Content on this platform, so just plain old text");
            frame_.set_content(framed_radio_);

            // The "display the Content where it can, else the string representation" radio: the View
            // Content (coffee image) is deferred, so this shows the string fallback the radio renders.
            caption_fallback_.set_text("This will display the Content (coffee cup) where it can, and fall back to the "
                                       "string representation where it cannot:");
            fallback_radio_.set_content("coffee.png"); // note: View Content (Image) deferred → string repr.

            // 3) Content as a View under a ControlTemplate → renders on ANY platform. The radio_button's
            // ControlTemplate path is deferred, so the content_view template seam stands in (see header).
            caption_template_.set_text("If we select a ControlTemplate, we can use a View as Content on any platform:");

            // The "weird-looking custom template" group (two content_views carrying the custom template,
            // each packing a coffee image as Content). In the XAML these are two GroupName="weird"
            // RadioButtons; the grouping/mutual-exclusion lives on radio_button, while the template
            // hosting lives on the content_view seam — so the port shows the template+presenter packing
            // (the page's actual subject) and notes the radio grouping is not driven through these views.
            caption_weird_.set_text("Also, here's a group with a weird-looking custom template:");
            add_weird_card();
            add_weird_card();

            // Assemble the stack in XAML order.
            stack_.add(caption_string_);
            stack_.add(option_a_);
            stack_.add(caption_string2_);
            stack_.add(option_c_);
            stack_.add(caption_view_);
            stack_.add(frame_);
            stack_.add(caption_fallback_);
            stack_.add(fallback_radio_);
            stack_.add(caption_template_);
            stack_.add(caption_weird_);
            for (const auto& card : weird_cards_)
            {
                stack_.add(*card);
            }

            scroll_.set_content(stack_);
            page_.set_content(scroll_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (leaves first, the page last), then re-host the
        // tree built in the ctor (gallery_attach.hpp).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& view, const char* name) { gallery_attach_one(app, view, name); };

            // Leaves / captions first.
            one(caption_string_, "caption_string_");
            one(option_a_, "option_a_");
            one(caption_string2_, "caption_string2_");
            one(option_c_, "option_c_");
            one(caption_view_, "caption_view_");
            one(framed_radio_, "framed_radio_");
            one(frame_, "frame_");
            one(caption_fallback_, "caption_fallback_");
            one(fallback_radio_, "fallback_radio_");
            one(caption_template_, "caption_template_");
            one(caption_weird_, "caption_weird_");

            // The coffee images each weird template's content_presenter packs (attach BEFORE the cards so
            // the presenter has a hostable child when it re-hosts below).
            for (const auto& coffee : coffee_images_)
            {
                one(*coffee, "coffee_image");
            }
            // Each weird card, then its MINTED custom template subtree (the bars + presenter), reached
            // through the card's template_root() — the page holds no direct member for it.
            for (const auto& card : weird_cards_)
            {
                one(*card, "weird_card");
                if (auto* tmpl = dynamic_cast<weird_radio_template*>(card->template_root()))
                {
                    tmpl->attach_handlers(app);
                }
            }

            // Containers, then the page.
            one(stack_, "stack_");
            one(scroll_, "scroll_");
            one(page_, "page_");

            // Replay the host commands recorded during construction, bottom-up: the presenters pack the
            // coffee images, then the cards host their presented content (the template root).
            gallery_rehost_content(frame_);
            for (const auto& card : weird_cards_)
            {
                if (auto* tmpl = dynamic_cast<weird_radio_template*>(card->template_root()))
                {
                    gallery_rehost_content(*tmpl->presenter()); // the presenter packs the coffee image
                }
            }
            for (const auto& card : weird_cards_)
            {
                gallery_rehost_content(*card);
            }
            gallery_rehost_layout(stack_);
            gallery_rehost_content(scroll_);
            gallery_rehost_content(page_);
        }

        // The owned controls, exposed for the hosting main's inspection.
        [[nodiscard]] maui::controls::stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::radio_button& option_a()
        {
            return option_a_;
        }
        [[nodiscard]] maui::controls::radio_button& option_c()
        {
            return option_c_;
        }
        [[nodiscard]] const std::vector<std::shared_ptr<maui::controls::content_view>>& weird_cards() const
        {
            return weird_cards_;
        }

    private:
        // A "weird custom template" card: a content_view whose ControlTemplate is weird_radio_template and
        // whose Content is a coffee image (the presenter packs it). Applying set_control_template runs the
        // same template_utilities::on_control_template_changed the C# ControlTemplate change callback runs.
        void add_weird_card()
        {
            auto card = std::make_shared<maui::controls::content_view>();
            card->set_control_template(maui::controls::control_template::of<weird_radio_template>());

            auto coffee = std::make_shared<maui::controls::image>();
            coffee->set_source(maui::controls::image_source::from_file("coffee.png"));
            card->set_content(coffee);

            coffee_images_.push_back(std::move(coffee));
            weird_cards_.push_back(std::move(card));
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroll_;
        maui::controls::stack_layout stack_;

        maui::controls::label caption_string_;
        maui::controls::radio_button option_a_;
        maui::controls::label caption_string2_;
        maui::controls::radio_button option_c_;
        maui::controls::label caption_view_;
        maui::controls::frame frame_;
        maui::controls::radio_button framed_radio_;
        maui::controls::label caption_fallback_;
        maui::controls::radio_button fallback_radio_;
        maui::controls::label caption_template_;
        maui::controls::label caption_weird_;

        // The two custom-template cards + the coffee images they co-own (kept alive + reachable).
        std::vector<std::shared_ptr<maui::controls::content_view>> weird_cards_;
        std::vector<std::shared_ptr<maui::controls::image>> coffee_images_;
    };
} // namespace maui::samples
