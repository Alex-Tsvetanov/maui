#pragma once
// maui::samples::swipe_view_shadow_page — ports SwipeViewShadowGallery.xaml
//
// A code-first port of the MAUI SwipeView sub-gallery
// Pages/Controls/SwipeViewGalleries/SwipeViewShadowGallery.xaml: a padded vertical StackLayout proving a
// drop Shadow renders correctly on SwipeView content. It hosts a headline, then two labelled SwipeViews:
//   - "SwipeItems":     a SwipeView with Left {Execute "Delete" / Tomato} + Right {Execute "Add" /
//                       LimeGreen} SwipeItems, over a black-stroked (thickness 3) RoundRectangle Border
//                       (corner radius 10) that itself casts a black Shadow (offset 20,20, radius 40,
//                       opacity 0.8) around a "Content" label;
//   - "SwipeItemViews": the same shadowed Border content, but the Left + Right items are CUSTOM
//                       SwipeItemViews — each a rounded (RoundRectangle 12) coloured Border (Tomato
//                       "Delete" / LimeGreen "Add", width 80) instead of a text/icon SwipeItem.
//
// The page OWNS its whole element tree (the swipe_refresh_page / value_controls_page pattern). It is
// backend-agnostic — a sample main attaches handlers bottom-up via the hosting layer and hosts page() in a
// window; the headless/apple/ios test trees exercise the same control wiring directly.
//
// Interactions demonstrated:
//   - the page is a purely visual shadow demo (the C# code-behind is just InitializeComponent() — there
//     is no logic to port);
//   - in attach_handlers, the first SwipeView is synthetically opened toward its RightItems side so a
//     static capture shows the revealed "Add" item alongside the shadowed content (the swipe-to-reveal
//     gesture has no headless analogue; open() routes through the now-attached handler — see the
//     swipe_view_seam tests).
//
// PORT NOTES (faithful best-effort, never invented):
//   note: the headline Label's Style="{StaticResource Headline}" is a resource-dictionary style; the
//         resource/style-application layer is deferred, so only the headline TEXT is ported (the styled
//         font is not applied) — the label still reads "Shadow in SwipeView Content".
//   note: the Border.Shadow (Brush=Black, Offset 20,20, Radius 40, Opacity 0.8) is reconstructed as a
//         maui::core::shadow with set_color(black) + set_offset(point(20,20)) + set_radius(40) +
//         set_opacity(0.8); the Border.StrokeShape RoundRectangle CornerRadius=10 → round_rectangle(10).
//   note: the SwipeItemView content Borders use the XAML "StrokeShape=RoundRectangle 12" shorthand →
//         round_rectangle(12); Background="Tomato"/"LimeGreen" is reconstructed via set_background over a
//         solid_paint (the documented Brush→Paint bridge — there is no set_background_color on a view).
//   note: VisualElement Padding="12" on the StackLayout and HeightRequest=60 / WidthRequest=80 are set
//         where the XAML names them; the SwipeItems Mode="Execute" maps to set_mode(swipe_mode::execute).

#include <memory>

#include "maui/controls/border.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/swipe_item.hpp"
#include "maui/controls/swipe_item_view.hpp"
#include "maui/controls/swipe_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/open_swipe_item.hpp"
#include "maui/core/shadow.hpp"
#include "maui/core/swipe_mode.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/point.hpp"
#include "maui/graphics/shapes/round_rectangle.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class swipe_view_shadow_page
    {
    public:
        swipe_view_shadow_page()
        {
            page_.set_title("Swipe ViewShadow Gallery");
            stack_.set_padding(maui::core::thickness{12});

            // The headline + the two section captions (the Headline style is deferred — text only).
            headline_.set_text("Shadow in SwipeView Content");
            items_caption_.set_text("SwipeItems");
            item_views_caption_.set_text("SwipeItemViews");

            // ---- SwipeView #1 — text/icon SwipeItems over the shadowed border ----
            items_delete_.set_text("Delete");
            items_delete_.set_background_color(maui::graphics::colors::tomato);
            items_add_.set_text("Add");
            items_add_.set_background_color(maui::graphics::colors::lime_green);
            items_swipe_.left_items_collection().set_mode(maui::core::swipe_mode::execute);
            items_swipe_.left_items_collection().add(items_delete_);
            items_swipe_.right_items_collection().set_mode(maui::core::swipe_mode::execute);
            items_swipe_.right_items_collection().add(items_add_);
            build_shadowed_border(items_border_, items_content_grid_, items_content_label_);
            items_swipe_.set_content(items_border_);

            // ---- SwipeView #2 — custom SwipeItemViews (coloured rounded borders) over the same content ----
            view_delete_border_.set_background(solid(maui::graphics::colors::tomato));
            view_delete_border_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(12.0));
            view_delete_border_.set_width_request(80);
            view_delete_label_.set_text("Delete");
            view_delete_label_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            view_delete_label_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            view_delete_border_.set_content(view_delete_label_);
            view_delete_item_.set_content(view_delete_border_);

            view_add_border_.set_background(solid(maui::graphics::colors::lime_green));
            view_add_border_.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(12.0));
            view_add_border_.set_width_request(80);
            view_add_label_.set_text("Add");
            view_add_label_.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            view_add_label_.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            view_add_border_.set_content(view_add_label_);
            view_add_item_.set_content(view_add_border_);

            view_swipe_.left_items_collection().set_mode(maui::core::swipe_mode::execute);
            view_swipe_.left_items_collection().add(view_delete_item_);
            view_swipe_.right_items_collection().set_mode(maui::core::swipe_mode::execute);
            view_swipe_.right_items_collection().add(view_add_item_);
            build_shadowed_border(view_border_, view_content_grid_, view_content_label_);
            view_swipe_.set_content(view_border_);

            stack_.add(headline_);
            stack_.add(items_caption_);
            stack_.add(items_swipe_);
            stack_.add(item_views_caption_);
            stack_.add(view_swipe_);
            page_.set_content(stack_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the content labels + grids, the shadowed
        // borders, the custom item-view borders + their swipe_item_views, the two swipe_views, the captions
        // + headline, the stack, then the page), then re-host the tree built in the ctor. The text
        // swipe_item members are NON-view items (no standalone handler), deliberately excluded — attaching
        // one would throw. The swipe_item_views ARE views, so they DO attach. After re-hosting,
        // synthetically open the first swipe_view toward its RightItems side. (gallery_attach.hpp)
        void attach_handlers(maui::hosting::maui_app& app)
        {
            // #1 content (bottom-up): label → grid → shadowed border → swipe.
            gallery_attach_one(app, items_content_label_, "items_content_label_");
            gallery_attach_one(app, items_content_grid_, "items_content_grid_");
            gallery_attach_one(app, items_border_, "items_border_");
            gallery_attach_one(app, items_swipe_, "items_swipe_");

            // #2 custom item-view borders (bottom-up): label → border → swipe_item_view.
            gallery_attach_one(app, view_delete_label_, "view_delete_label_");
            gallery_attach_one(app, view_delete_border_, "view_delete_border_");
            gallery_attach_one(app, view_delete_item_, "view_delete_item_");
            gallery_attach_one(app, view_add_label_, "view_add_label_");
            gallery_attach_one(app, view_add_border_, "view_add_border_");
            gallery_attach_one(app, view_add_item_, "view_add_item_");
            // #2 content (bottom-up): label → grid → shadowed border → swipe.
            gallery_attach_one(app, view_content_label_, "view_content_label_");
            gallery_attach_one(app, view_content_grid_, "view_content_grid_");
            gallery_attach_one(app, view_border_, "view_border_");
            gallery_attach_one(app, view_swipe_, "view_swipe_");

            gallery_attach_one(app, headline_, "headline_");
            gallery_attach_one(app, items_caption_, "items_caption_");
            gallery_attach_one(app, item_views_caption_, "item_views_caption_");
            gallery_attach_one(app, stack_, "stack_");
            gallery_attach_one(app, page_, "page_");

            // The tree was built in the ctor before any handler existed, so replay the host commands now.
            gallery_rehost_layout(items_content_grid_);
            gallery_rehost_content(items_border_);
            gallery_rehost_content(items_swipe_);

            gallery_rehost_content(view_delete_border_); // border hosts its label
            gallery_rehost_content(view_delete_item_);   // item-view hosts its border
            gallery_rehost_content(view_add_border_);
            gallery_rehost_content(view_add_item_);
            gallery_rehost_layout(view_content_grid_);
            gallery_rehost_content(view_border_);
            gallery_rehost_content(view_swipe_);

            gallery_rehost_layout(stack_);
            gallery_rehost_content(page_);

            // Static-capture seam: reveal the first swipe_view's RightItems ("Add"). open() routes through
            // the now-attached handler (the swipe gesture has no headless analogue — swipe_view_seam tests).
            items_swipe_.open(maui::core::open_swipe_item::right_items);
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& stack()
        {
            return stack_;
        }
        [[nodiscard]] maui::controls::swipe_view& items_swipe()
        {
            return items_swipe_;
        }
        [[nodiscard]] maui::controls::swipe_view& view_swipe()
        {
            return view_swipe_;
        }
        [[nodiscard]] maui::controls::border& items_border()
        {
            return items_border_;
        }
        [[nodiscard]] maui::controls::swipe_item_view& view_delete_item()
        {
            return view_delete_item_;
        }
        [[nodiscard]] maui::controls::swipe_item_view& view_add_item()
        {
            return view_add_item_;
        }

    private:
        // Build the shadowed content Border the XAML repeats for both SwipeViews: black stroke (thickness
        // 3), RoundRectangle corner radius 10, a black Shadow (offset 20,20, radius 40, opacity 0.8), and a
        // single-cell Grid hosting a centred "Content" label. Out-of-line so both sites stay identical.
        static void build_shadowed_border(maui::controls::border& bordered, maui::controls::grid& content_grid,
                                          maui::controls::label& content_label)
        {
            bordered.set_stroke(std::make_shared<maui::graphics::solid_paint>(maui::graphics::colors::black));
            bordered.set_stroke_thickness(3);
            bordered.set_height_request(60);
            bordered.set_stroke_shape(std::make_shared<maui::graphics::shapes::round_rectangle>(10.0));

            auto drop = std::make_shared<maui::core::shadow>();
            drop->set_color(maui::graphics::colors::black);
            drop->set_offset(maui::graphics::point(20, 20));
            drop->set_radius(40);
            drop->set_opacity(0.8);
            bordered.set_shadow(std::move(drop));

            content_label.set_text("Content");
            // XAML: the Content Label is HorizontalOptions/VerticalOptions=Center within its single-cell Grid.
            content_label.set_horizontal_layout_alignment(maui::core::layout_alignment::center);
            content_label.set_vertical_layout_alignment(maui::core::layout_alignment::center);
            content_grid.add(content_label);
            bordered.set_content(content_grid);
        }

        static std::shared_ptr<maui::graphics::solid_paint> solid(maui::graphics::color value)
        {
            return std::make_shared<maui::graphics::solid_paint>(value);
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label headline_;
        maui::controls::label items_caption_;
        maui::controls::label item_views_caption_;

        // #1 — text SwipeItems
        maui::controls::swipe_view items_swipe_;
        maui::controls::swipe_item items_delete_;
        maui::controls::swipe_item items_add_;
        maui::controls::border items_border_;
        maui::controls::grid items_content_grid_;
        maui::controls::label items_content_label_;

        // #2 — custom SwipeItemViews
        maui::controls::swipe_view view_swipe_;
        maui::controls::swipe_item_view view_delete_item_;
        maui::controls::border view_delete_border_;
        maui::controls::label view_delete_label_;
        maui::controls::swipe_item_view view_add_item_;
        maui::controls::border view_add_border_;
        maui::controls::label view_add_label_;
        maui::controls::border view_border_;
        maui::controls::grid view_content_grid_;
        maui::controls::label view_content_label_;
    };
} // namespace maui::samples
