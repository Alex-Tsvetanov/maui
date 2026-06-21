#pragma once
// maui::samples::header_footer_template_page — ports HeaderFooterTemplate.xaml (+ .xaml.cs) of the C#
// CollectionView gallery (CollectionViewGalleries/HeaderFooterGalleries).
//
// The original page (HeaderFooterTemplate): a CollectionView whose Header and Footer are both
// `{Binding .}` (the BindingContext object itself — a HeaderFooterDemoModel) and whose HeaderTemplate /
// FooterTemplate are DataTemplates. Each template is a two-row Grid carrying an Image (with a
// TapGestureRecognizer whose Command="{Binding TapCommand}"), a centered bold AntiqueWhite Label bound
// to {Binding CurrentTime}, and a static "This Is A Header" / "This Is A Footer" Label. The xaml.cs sets
// ItemTemplate = ExampleTemplates.PhotoTemplate() and BindingContext = a HeaderFooterDemoModel, whose
// Items is DemoFilteredItemSource(3).Items, whose CurrentTime starts at DateTime.Now, and whose
// TapCommand resets CurrentTime = DateTime.Now (so tapping the header/footer Image re-stamps the time
// the Label shows, via INotifyPropertyChanged).
//
// This is the TEMPLATE arm of the Header/Footer trio (HeaderFooterString boxes strings, this boxes
// DataTemplates, HeaderFooterView boxes a live View): the headless collection_view handler's
// realize_supplemental takes the `content_template` branch — it creates the template's content and sets
// its BindingContext to the boxed Header/Footer value — exactly the C# HeaderTemplate path.
//
// The port mirrors the shape code-first:
//   - the Header / Footer VALUE is the model itself (`{Binding .}`): a `header_model` struct carrying
//     the formatted CurrentTime string + the TapCommand, boxed as the Header/Footer payload. Because the
//     port's templated cells render a single root control (data_template::of<TControl>) the template is
//     a Label whose Text binds to the model's `current_time` (the bold AntiqueWhite {Binding CurrentTime}
//     Label — the demonstrated bound text). The static "This Is A Header/Footer" Label and the Image are
//     documented below (see note);
//   - the item template is the PhotoTemplate caption Label (Text bound to each row's caption);
//   - the TapCommand is a live maui::controls::command that re-stamps the time and re-realizes the
//     Header/Footer (tap_header()/tap_footer() are the page-level synthetic-dispatch stand-in for the C#
//     Image's TapGestureRecognizer firing the bound command — the gesture itself is wired on the model's
//     command, see note).
//
// The page OWNS its whole element tree; attach_handlers wires every owned view bottom-up and re-hosts
// the tree (gallery_attach.hpp).
//
// note: the C# templates root a Grid holding three things the port reduces:
//       (1) the {Binding CurrentTime} Label — PORTED as the template's bound Label (the live text);
//       (2) a static "This Is A Header/Footer" Label — the port folds its intent into the title/time
//           text since a single-root templated cell renders one control;
//       (3) an Image with a TapGestureRecognizer Command="{Binding TapCommand}" — the headless backend
//           has no asset pipeline for the image and no per-instance gesture-realization seam inside a
//           templated cell, so the TapCommand is exposed + exercised at the page level (tap_header /
//           tap_footer run the SAME bound command, re-stamping CurrentTime and re-realizing the chrome).
//       The item PhotoTemplate is the caption Label only (no per-row Image), as in the sibling demos.

#include <chrono>
#include <cstdio>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/command.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/font.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class header_footer_template_page
    {
    public:
        // One row of the demo source — the caption the PhotoTemplate binds.
        struct demo_item
        {
            std::string caption;
        };

        // HeaderFooterDemoModel as the port's reflection-free Header/Footer BindingContext (`{Binding .}`):
        // the formatted CurrentTime the templates' Label binds (`{Binding CurrentTime}`). The TapCommand
        // lives on the page (it must mutate this value + re-realize), so the struct itself carries only the
        // bound display string — copied into the boxed_item, value-typed (the data_template binds
        // <std::string, header_model>).
        struct header_model
        {
            std::string current_time;
            friend bool operator==(const header_model&, const header_model&) = default;
        };

        header_footer_template_page()
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(seed_items())),
              // HeaderFooterDemoModel(): CurrentTime = DateTime.Now.
              model_{now_string()},
              // TapCommand => CurrentTime = DateTime.Now (re-stamp + re-realize the chrome).
              tap_command_(std::make_shared<maui::controls::command>([this] { restamp_time(); }))
        {
            page_.set_title("Header/Footer (template)");

            // The PhotoTemplate caption Label: Text binds to the item's caption (C# Binding("Caption")),
            // styled as the C# PhotoTemplate's blue caption (a blue background paint, centered text).
            auto cell = maui::controls::data_template::of<maui::controls::label>();
            cell->set_binding<std::string, demo_item>(maui::controls::label::text_property(),
                                                      [](const demo_item& item) { return item.caption; });
            cell->set_value(maui::controls::background_property(),
                            std::static_pointer_cast<maui::graphics::paint>(
                                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue)));
            cell->set_value(maui::controls::label::horizontal_text_alignment_property(),
                            maui::core::text_alignment::center);
            list_.set_item_template(cell);
            list_.set_items_source(items_);

            // ---- the HeaderTemplate / FooterTemplate: a bold AntiqueWhite centered Label bound to the
            // model's CurrentTime (the {Binding CurrentTime} Label, the live bound text), with the static
            // "This Is A Header/Footer" Label folded onto the bound time (a single-root cell renders one
            // control, so the static caption is appended on a second line of the same Label) ----
            list_.set_header_template(make_time_template(36, "\nThis Is A Header")); // header Label FontSize=36
            list_.set_footer_template(make_time_template(20, "\nThis Is A Footer")); // footer Label FontSize=20

            // ---- the Header / Footer VALUE = the model itself (`{Binding .}`) ----
            list_.set_header(maui::controls::boxed_item::of(model_));
            list_.set_footer(maui::controls::boxed_item::of(model_));

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the list first, the page last), then re-host
        // the tree built in the ctor (gallery_attach.hpp). The Header/Footer templated content is created
        // and hosted by the collection_view handler itself (no separately-owned chrome views to attach).
        void attach_handlers(maui::hosting::maui_app& app)
        {
            gallery_attach_one(app, list_, "list_");
            gallery_attach_one(app, page_, "page_");

            gallery_rehost_content(page_); // the page hosts the collection_view
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment / tests.
        [[nodiscard]] maui::controls::collection_view& list()
        {
            return list_;
        }
        [[nodiscard]] const std::shared_ptr<maui::core::observable_collection<demo_item>>& items() const
        {
            return items_;
        }
        // The bound TapCommand (TapGestureRecognizer.Command="{Binding TapCommand}") — exposed so a host /
        // test can fire it as the header/footer Image's tap would.
        [[nodiscard]] const std::shared_ptr<maui::controls::command>& tap_command() const
        {
            return tap_command_;
        }
        [[nodiscard]] const std::string& current_time() const
        {
            return model_.current_time;
        }

        // The page-level synthetic-dispatch stand-in for the header/footer Image's TapGestureRecognizer
        // firing its bound Command (see header note): both run the SAME tap_command_.
        void tap_header()
        {
            tap_command_->execute({});
        }
        void tap_footer()
        {
            tap_command_->execute({});
        }

    private:
        // DemoFilteredItemSource(3).AddItems: three rows, captioned "<image>, <n>" off the image ring.
        [[nodiscard]] static std::vector<demo_item> seed_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg", "photo.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 3; ++n)
            {
                rows.push_back(
                    demo_item{images[static_cast<std::size_t>(n) % images.size()] + ", " + std::to_string(n)});
            }
            return rows;
        }

        // DateTime.Now, formatted as C#'s DateTime.ToString() does for en-US: "M/d/yyyy h:mm:ss tt"
        // (no leading zeros on month/day/hour, 12-hour clock with an AM/PM designator). Built by hand so the
        // stamp is locale-independent (strftime's %p/%I are locale-dependent and zero-pad the hour).
        [[nodiscard]] static std::string now_string()
        {
            const auto now = std::chrono::system_clock::now();
            const std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm_buf{};
#if defined(_WIN32)
            ::localtime_s(&tm_buf, &t);
#else
            ::localtime_r(&t, &tm_buf);
#endif
            const int hour24 = tm_buf.tm_hour;
            const char* const meridiem = hour24 < 12 ? "AM" : "PM";
            int hour12 = hour24 % 12;
            if (hour12 == 0)
            {
                hour12 = 12; // midnight / noon read as 12, not 0 (en-US 12-hour clock)
            }
            char buf[40] = {};
            std::snprintf(buf, sizeof(buf), "%d/%d/%d %d:%02d:%02d %s", tm_buf.tm_mon + 1, tm_buf.tm_mday,
                          tm_buf.tm_year + 1900, hour12, tm_buf.tm_min, tm_buf.tm_sec, meridiem);
            return std::string{buf};
        }

        // TapCommand => CurrentTime = DateTime.Now: re-stamp the model's time and re-realize the chrome
        // (the headless analog of INotifyPropertyChanged re-pushing {Binding CurrentTime} — re-setting the
        // boxed Header/Footer value re-runs realize_supplemental against the fresh model).
        void restamp_time()
        {
            model_.current_time = now_string();
            list_.set_header(maui::controls::boxed_item::of(model_));
            list_.set_footer(maui::controls::boxed_item::of(model_));
        }

        // A HeaderTemplate/FooterTemplate: a bold AntiqueWhite centered Label whose Text binds to the
        // model's CurrentTime ({Binding CurrentTime}), at the given font size (header 36 / footer 20), with
        // the static "This Is A Header/Footer" caption appended (`static_suffix`) so the C# template's static
        // Label content surfaces in the single-root cell.
        [[nodiscard]] static std::shared_ptr<maui::controls::data_template> make_time_template(
            double font_size, std::string static_suffix)
        {
            auto tmpl = maui::controls::data_template::of<maui::controls::label>();
            tmpl->set_binding<std::string, header_model>(
                maui::controls::label::text_property(),
                [suffix = std::move(static_suffix)](const header_model& m) { return m.current_time + suffix; });
            tmpl->set_value(maui::controls::label::text_color_property(), maui::graphics::colors::antique_white);
            tmpl->set_value(maui::controls::label::horizontal_text_alignment_property(),
                            maui::core::text_alignment::center);
            tmpl->set_value(maui::controls::label::font_property(),
                            maui::core::font::system_font_of_size(font_size, maui::core::font_weight::bold));
            return tmpl;
        }

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        header_model model_;                                                  // the `{Binding .}` Header/Footer context
        std::shared_ptr<maui::controls::command> tap_command_;                // TapGestureRecognizer.Command
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
