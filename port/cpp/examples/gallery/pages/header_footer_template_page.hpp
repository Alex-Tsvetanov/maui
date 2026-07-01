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
// the Label shows, via INotifyPropertyChanged). ExampleTemplates.PhotoTemplate() is a two-row Grid: an
// Image (WidthRequest=100, centered) bound to {Binding Image} above a blue-background caption Label
// bound to {Binding Caption}.
//
// This is the TEMPLATE arm of the Header/Footer trio (HeaderFooterString boxes strings, this boxes
// DataTemplates, HeaderFooterView boxes a live View): the collection_view handler's realize path takes
// the `content_template` branch — it creates the template's content, sets its BindingContext to the
// boxed Header/Footer value (or, per row, the item), and hosts it — exactly the C# HeaderTemplate /
// ItemTemplate path.
//
// The port mirrors the shape code-first. The KEY fidelity point (this file's reason to exist): MAUI's
// header/footer/item templates each render an IMAGE (the header/footer cover photo, the per-row photo
// thumbnail). A `data_template::of<TLeaf>()` cell renders ONE leaf control, so the earlier port reduced
// every template to a single text Label and the images were MISSING (the header/footer cover photo was
// text-only; the item's photo thumbnail was a bare blue caption box — the Android parity red). The port's
// templated cells CAN root a container that OWNS its children: the sibling nested_collection roots its
// cell at a collection_view, and custom_layout_page registers a user layout subclass via register_handlers.
// This page follows both: the three templates root purpose-built COMPOSITE view subclasses that own their
// Image + Label children as members (so the children outlive the realized cell — layout::add is
// non-owning) and push the bound values on BindingContext change. Each composite's handler is the shared
// layout_handler, registered pre-mount via register_handlers (the custom_layout_page dock_layout pattern),
// so the collection_view realize path resolves it for of<TComposite>() and the images render like MAUI.
//
//   - the header / footer VALUE is the model itself (`{Binding .}`): a `header_model` struct carrying the
//     formatted CurrentTime string, boxed as the Header/Footer payload. The header/footer TEMPLATE is a
//     `chrome_cell` (a vertical_stack_layout subclass) that owns the cover-photo Image (oasis.jpg for the
//     header, cover1.jpg for the footer — the XAML Image.Source), the bold AntiqueWhite {Binding
//     CurrentTime} Label, and the static "This Is A Header/Footer" Label; on BindingContext change it pushes
//     the model's current_time into the bound Label;
//   - the item TEMPLATE is a `photo_cell` (a vertical_stack_layout subclass) that owns the per-row Image
//     (source = the row's Image name, bound {Binding Image}) above the blue-background caption Label (bound
//     {Binding Caption}) — ExampleTemplates.PhotoTemplate() faithfully;
//   - the TapCommand is a live maui::controls::command that re-stamps the time and re-realizes the
//     Header/Footer (tap_header()/tap_footer() are the page-level synthetic-dispatch stand-in for the C#
//     Image's TapGestureRecognizer firing the bound command — the gesture itself is wired on the model's
//     command, see note).
//
// The page OWNS its whole element tree; the generic mount (app_host.hpp) attaches every owned view's
// handler and hosts the tree. The composite cells are realized on demand by the collection_view handler.
//
// note: the C# Image carries a TapGestureRecognizer Command="{Binding TapCommand}". The per-instance
//       gesture-realization seam inside a templated cell is not modeled, so the TapCommand is exposed +
//       exercised at the page level (tap_header / tap_footer run the SAME bound command, re-stamping
//       CurrentTime and re-realizing the chrome). The C# header Grid OVERLAYS the CurrentTime Label on the
//       Image (both in Grid row 0); the port's single-axis stack renders the Image above the labels, which
//       matches MAUI's visible frame (the cover-photo band with the caption below it).

#include <chrono>
#include <cstdio>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/command.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/file_image_source.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/collection_view.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/font.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/layout_alignment.hpp"
#include "maui/core/layout_handler.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

namespace maui::samples
{
    // One row of the demo source. CollectionViewGalleryTestItem reduced to what the PhotoTemplate binds:
    // Caption (the blue Label — `"{image}, {n}"`) and Image (the per-row photo file name, {Binding Image}).
    struct header_footer_demo_item
    {
        std::string caption;
        std::string image_name;
        friend bool operator==(const header_footer_demo_item&, const header_footer_demo_item&) = default;
    };

    // HeaderFooterDemoModel as the port's reflection-free Header/Footer BindingContext (`{Binding .}`): the
    // formatted CurrentTime the chrome template's Label binds (`{Binding CurrentTime}`).
    struct header_footer_model
    {
        std::string current_time;
        friend bool operator==(const header_footer_model&, const header_footer_model&) = default;
    };

    // ---- the item template root: ExampleTemplates.PhotoTemplate() ----
    // A grid subclass that reproduces PhotoTemplate() EXACTLY: a two-star-row Grid sized WidthRequest=200,
    // HeightRequest=100 (so each item cell is a fixed 100pt tall regardless of content — the compact MAUI
    // row height), with the Image (WidthRequest=100, centered, Margin(2,5,2,2)) in ROW 0 and the blue-
    // background caption Label (centered, Margin(2,0,2,2)) in ROW 1 (Grid.SetRow(caption, 1)). It OWNS its
    // Image + Label children as members (layout::add is non-owning, so a template cell must own the children
    // it hosts). The Image's Source is the row's {Binding Image} and the Label's Text is {Binding Caption},
    // both pushed on BindingContext change (the realize path sets the cell's context to the
    // header_footer_demo_item). Default-constructible so a data_template::of<photo_cell>() activates it; its
    // handler is the shared layout_handler (grid uses it; registered in register_handlers).
    class photo_cell : public maui::controls::grid
    {
    public:
        photo_cell()
        {
            // Grid { RowDefinitions = { new RowDefinition(), new RowDefinition() }, WidthRequest=200,
            // HeightRequest=100 } — two star rows, fixed 100pt cell.
            add_row_definition(maui::core::grid_length::star());
            add_row_definition(maui::core::grid_length::star());
            set_width_request(200);
            set_height_request(100);
            // Image { WidthRequest=100, HorizontalOptions=Center, VerticalOptions=Center, Margin(2,5,2,2) }.
            image_.set_width_request(100);
            image_.set_aspect(maui::core::aspect::aspect_fit);
            image_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            image_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            image_.set_margin(maui::core::thickness(2, 5, 2, 2));
            // Label { FontSize=12, HorizontalOptions=Fill, HorizontalTextAlignment=Center,
            // Margin(2,0,2,2), BackgroundColor=Blue } — the blue caption, bound to {Binding Caption}.
            caption_.set_font(maui::core::font::system_font_of_size(12));
            caption_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            caption_.set_margin(maui::core::thickness(2, 0, 2, 2));
            caption_.set_background(std::static_pointer_cast<maui::graphics::paint>(
                std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::blue)));
            add(image_);   // row 0 (default)
            add(caption_); // row 1
            set_row(caption_, 1);
        }

    protected:
        // Push {Binding Image} → Image.Source and {Binding Caption} → Label.Text when the cell's
        // BindingContext (the header_footer_demo_item) is set by the realize path.
        void on_binding_context_changed() override
        {
            maui::controls::grid::on_binding_context_changed(); // propagate to children first
            if (const auto item = binding_context<header_footer_demo_item>())
            {
                caption_.set_text(item->caption);
                image_.set_source(maui::controls::image_source::from_file(item->image_name));
            }
        }

    private:
        maui::controls::image image_;
        maui::controls::label caption_;
    };

    // ---- the header/footer template root: the two-row Grid (HeaderTemplate/FooterTemplate) ----
    // A grid subclass reproducing the XAML Grid EXACTLY: two rows RowDefinition Height=50 + Height=20. The
    // cover-photo Image (Aspect=AspectFill, HeightRequest=80 header / 50 footer) and the bold AntiqueWhite
    // {Binding CurrentTime} Label (InputTransparent) are BOTH in row 0 (the Image with no Grid.Row, the time
    // Label with no Grid.Row — they overlap: the time reads over the cover photo). The static "This Is A
    // Header/Footer" Label sits in row 1 (Grid.Row=1). It OWNS its Image + two Label children. The image
    // file, font size, and static caption differ between header and footer, but a data_template::of<T>()
    // default-constructs the root — so those differences are baked into two concrete subclasses
    // (header_chrome_cell / footer_chrome_cell) via a shared base whose ctor takes them.
    class chrome_cell : public maui::controls::grid
    {
    public:
        chrome_cell(std::string image_file, double time_font_size, double image_height, std::string static_caption)
        {
            // RowDefinitions: Height=50 (the cover-photo + overlaid time), Height=20 (the static caption).
            add_row_definition(maui::core::grid_length{50});
            add_row_definition(maui::core::grid_length{20});
            // Image { Source, Aspect=AspectFill, HeightRequest=80|50 } in row 0.
            image_.set_source(maui::controls::image_source::from_file(std::move(image_file)));
            image_.set_aspect(maui::core::aspect::aspect_fill);
            image_.set_height_request(image_height);
            // Label { Text={Binding CurrentTime}, TextColor=AntiqueWhite, HorizontalTextAlignment=Center,
            // FontAttributes=Bold, FontSize=36|20 } in row 0 — overlaps the Image (the time over the photo).
            time_.set_text_color(maui::graphics::colors::antique_white);
            time_.set_horizontal_text_alignment(maui::core::text_alignment::center);
            time_.set_font(maui::core::font::system_font_of_size(time_font_size, maui::core::font_weight::bold));
            // Label { Grid.Row=1, Text="This Is A Header/Footer" } — the static caption below.
            static_.set_text(std::move(static_caption));
            add(image_); // row 0 (default)
            add(time_);  // row 0 (default) — overlaps the image
            add(static_);
            set_row(static_, 1); // Grid.Row="1"
        }

    protected:
        // Push {Binding CurrentTime} → the bound time Label when the cell's BindingContext (the
        // header_footer_model) is set by the realize path.
        void on_binding_context_changed() override
        {
            maui::controls::grid::on_binding_context_changed();
            if (const auto model = binding_context<header_footer_model>())
            {
                time_.set_text(model->current_time);
            }
        }

    private:
        maui::controls::image image_;
        maui::controls::label time_;
        maui::controls::label static_;
    };

    // Header cover photo = oasis.jpg, HeightRequest=80, CurrentTime FontSize=36, "This Is A Header".
    class header_chrome_cell : public chrome_cell
    {
    public:
        header_chrome_cell() : chrome_cell("oasis.jpg", 36, 80, "This Is A Header")
        {
        }
    };

    // Footer cover photo = cover1.jpg, HeightRequest=50, CurrentTime FontSize=20, "This Is A Footer".
    class footer_chrome_cell : public chrome_cell
    {
    public:
        footer_chrome_cell() : chrome_cell("cover1.jpg", 20, 50, "This Is A Footer")
        {
        }
    };

    class header_footer_template_page
    {
    public:
        // Kept as public aliases so any host / test referencing the old names still resolves.
        using demo_item = header_footer_demo_item;
        using header_model = header_footer_model;

        header_footer_template_page()
            : items_(std::make_shared<maui::core::observable_collection<demo_item>>(seed_items())),
              // HeaderFooterDemoModel(): CurrentTime = DateTime.Now.
              model_{now_string()},
              // TapCommand => CurrentTime = DateTime.Now (re-stamp + re-realize the chrome).
              tap_command_(std::make_shared<maui::controls::command>([this] { restamp_time(); }))
        {
            page_.set_title("Header/Footer (template)");

            // The item template: ExampleTemplates.PhotoTemplate() — a photo_cell (Image over blue caption).
            list_.set_item_template(maui::controls::data_template::of<photo_cell>());
            list_.set_items_source(items_);

            // The HeaderTemplate / FooterTemplate: the cover-photo chrome cells (Image + bound time + static).
            list_.set_header_template(maui::controls::data_template::of<header_chrome_cell>());
            list_.set_footer_template(maui::controls::data_template::of<footer_chrome_cell>());

            // The Header / Footer VALUE = the model itself (`{Binding .}`).
            list_.set_header(maui::controls::boxed_item::of(model_));
            list_.set_footer(maui::controls::boxed_item::of(model_));

            page_.set_content(list_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // PRE-MOUNT hook (gallery_host.hpp gallery_pre_mount): register the composite cell subclasses'
        // handlers BEFORE the generic mount / collection_view realize walks the tree. photo_cell /
        // header_chrome_cell / footer_chrome_cell are brand-new user types (like custom_layout_page's
        // dock_layout), so their handler isn't self-registered; the collection_view realize path resolves a
        // template's handler via THIS app's per-app handler_registry (of<TCell>() → create_handler by the
        // cell's type_tag), so without this the cells would fall back to the text mirror and the images would
        // be missing again. All three share the layout_handler (they are vertical_stack_layout subclasses).
        void register_handlers(maui::hosting::maui_app& app)
        {
            maui::core::register_handler<photo_cell, maui::core::layout_handler>(app.handlers());
            maui::core::register_handler<header_chrome_cell, maui::core::layout_handler>(app.handlers());
            maui::core::register_handler<footer_chrome_cell, maui::core::layout_handler>(app.handlers());
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
        // DemoFilteredItemSource(3).AddItems: three rows, captioned "<image>, <n>" off the image ring, each
        // carrying its Image file name (the {Binding Image} the PhotoTemplate's Image resolves).
        [[nodiscard]] static std::vector<demo_item> seed_items()
        {
            static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg", "photo.jpg"};
            std::vector<demo_item> rows;
            for (int n = 0; n < 3; ++n)
            {
                const std::string& image = images[static_cast<std::size_t>(n) % images.size()];
                rows.push_back(demo_item{.caption = image + ", " + std::to_string(n), .image_name = image});
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
        // (the analog of INotifyPropertyChanged re-pushing {Binding CurrentTime} — re-setting the boxed
        // Header/Footer value re-runs the realize path against the fresh model).
        void restamp_time()
        {
            model_.current_time = now_string();
            list_.set_header(maui::controls::boxed_item::of(model_));
            list_.set_footer(maui::controls::boxed_item::of(model_));
        }

        std::shared_ptr<maui::core::observable_collection<demo_item>> items_; // publisher before the list (§8)
        header_model model_;                                                  // the `{Binding .}` Header/Footer context
        std::shared_ptr<maui::controls::command> tap_command_;                // TapGestureRecognizer.Command
        maui::controls::content_page page_;
        maui::controls::collection_view list_;
    };
} // namespace maui::samples
