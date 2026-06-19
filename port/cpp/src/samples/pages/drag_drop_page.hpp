#pragma once
// maui::samples::drag_drop_page — ports DragAndDropBetweenLayouts.xaml (+ .xaml.cs)
//                                 (Maui.Controls.Sample.Pages.DragAndDropBetweenLayouts).
//
// The C# page is a three-column Grid: two color lists (SLAllColors / SLRainbow), each a StackLayout of
// color BoxViews bound from an ObservableCollection<Brush> via BindableLayout, and a third column of
// position readouts. Every BoxView carries a DragGestureRecognizer (DragStarting + DropCompleted) and
// each list StackLayout carries a DropGestureRecognizer (DragOver + DragLeave + Drop). Dragging a swatch
// from one list and dropping it on the OTHER list moves the color between the two collections:
//   - OnDragStarting stamps the dragged color + the source layout into e.Data.Properties ("Color" /
//     "Source"), shows the drag-start position readouts, and tints the OTHER layout LightBlue.
//   - OnDragOver / OnDragLeave update the drag-position readouts and tint the receiving layout
//     LightPink (over) / LightBlue (leave) — but reject (AcceptedOperation = None) a drop back onto the
//     source layout.
//   - OnDrop reads "Color" + "Source"; if the source differs from the receiver it moves the color from
//     its current list to the receiving list (AllColors <-> RainbowColors) and resets both backgrounds.
//
// This code-first port builds both lists as real StackLayouts of box_views, wires real
// drag_/drop_gesture_recognizers, and reproduces the move/readout/tint logic. The headless backend has
// no native drag session, so attach_handlers() finishes with one deterministic synthetic drive of a
// full drag -> over -> drop sequence (the same send_drag_starting / send_drag_over / send_drop seams the
// drag/drop unit tests use), moving the first AllColors swatch into the Rainbow list and leaving every
// readout reacting in a static capture.
//
// Demonstrated (the recognizers exist; the synthetic drive moves an item + updates readouts):
//   - DragStarting stamps DataPackage.Properties["Color"]/["Source"] and shows the start-position labels,
//   - DragOver/DragLeave update the drag-position labels and tint the receiving layout,
//   - Drop reads the package, moves the swatch box_view from the source layout into the receiving layout,
//     updates the drop-position labels, and resets the tints.
//
// The page OWNS its whole element tree (the gestures_page pattern): public page() and
// attach_handlers(maui_app).
//
// note: the C# BindableLayout.ItemsSource binding to ObservableCollection<Brush> (the data-template
//       fan-out) is a binding/data-template concern; the port models the SAME observable result by
//       building the swatch box_views directly and moving the box_view between the two layouts on drop
//       (the move is what the collection change drives visually). DragStartingEventArgs.GetPosition is
//       narrowed (no headless coordinate seam — drag_gesture_recognizer.hpp), so the position readouts
//       echo a representative carried position under the C# captions.

#include <any>
#include <cstddef>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "maui/controls/box_view.hpp"
#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/data_package.hpp"
#include "maui/controls/data_package_operation.hpp"
#include "maui/controls/gestures/drag_gesture_recognizer.hpp"
#include "maui/controls/gestures/drop_gesture_recognizer.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/colors.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "maui/hosting/maui_app.hpp"

#include "gallery_attach.hpp"

namespace maui::samples
{
    class drag_drop_page
    {
    public:
        drag_drop_page()
        {
            page_.set_title("Drag and drop");
            root_.set_spacing(12);

            // ---- the two color lists (SLAllColors / SLRainbow) -----------------------------------------
            all_colors_title_.set_text("All colors (drag a swatch to the rainbow list):");
            rainbow_title_.set_text("Rainbow:");
            all_colors_.set_spacing(4);
            rainbow_.set_spacing(4);

            // Seed the All-colors list with the C# SolidColorBrush set, each swatch a draggable box_view.
            add_swatch(all_colors_, maui::graphics::colors::red, "Red");
            add_swatch(all_colors_, maui::graphics::colors::orange, "Orange");
            add_swatch(all_colors_, maui::graphics::colors::yellow, "Yellow");
            add_swatch(all_colors_, maui::graphics::colors::green, "Green");
            add_swatch(all_colors_, maui::graphics::colors::blue, "Blue");
            add_swatch(all_colors_, maui::graphics::colors::indigo, "Indigo");
            add_swatch(all_colors_, maui::graphics::colors::violet, "Violet");

            // Each list is a drop TARGET: DragOver / DragLeave / Drop.
            wire_drop_target(all_drop_, all_colors_, "All colors");
            wire_drop_target(rainbow_drop_, rainbow_, "Rainbow");
            all_colors_.gesture_recognizers().add(all_drop_);
            rainbow_.gesture_recognizers().add(rainbow_drop_);

            // ---- the position readouts (the third Grid column) -----------------------------------------
            drag_starting_title_.set_text("Drag start position relative to...");
            drag_title_.set_text("Drag position relative to...");
            drop_title_.set_text("Drop position relative to...");
            drag_starting_position_.set_text("");
            drag_position_.set_text("");
            drop_position_.set_text("");
            move_readout_.set_text("Move: (none yet)");

            // ---- assemble ------------------------------------------------------------------------------
            root_.add(all_colors_title_);
            root_.add(all_colors_);
            root_.add(rainbow_title_);
            root_.add(rainbow_);
            root_.add(drag_starting_title_);
            root_.add(drag_starting_position_);
            root_.add(drag_title_);
            root_.add(drag_position_);
            root_.add(drop_title_);
            root_.add(drop_position_);
            root_.add(move_readout_);
            page_.set_content(root_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // Attach a handler to every OWNED view, BOTTOM-UP (the swatches, the two lists' labels, the lists,
        // the readouts, the root, the page), then re-host the ctor-built tree (gallery_attach.hpp).
        // Headless has no native drag session, so finish with one deterministic synthetic drag->drop so
        // the move + readouts reflect the wiring in a static capture.
        void attach_handlers(maui::hosting::maui_app& app)
        {
            auto one = [&app](auto& view, const char* name) { gallery_attach_one(app, view, name); };

            // Swatches first (the leaves of each list).
            for (const auto& swatch : all_swatches_)
            {
                one(*swatch, "all_swatch");
            }
            for (const auto& swatch : rainbow_swatches_)
            {
                one(*swatch, "rainbow_swatch");
            }
            one(all_colors_title_, "all_colors_title_");
            one(rainbow_title_, "rainbow_title_");
            one(all_colors_, "all_colors_");
            one(rainbow_, "rainbow_");
            one(drag_starting_title_, "drag_starting_title_");
            one(drag_starting_position_, "drag_starting_position_");
            one(drag_title_, "drag_title_");
            one(drag_position_, "drag_position_");
            one(drop_title_, "drop_title_");
            one(drop_position_, "drop_position_");
            one(move_readout_, "move_readout_");
            one(root_, "root_");
            one(page_, "page_");

            gallery_rehost_layout(all_colors_);
            gallery_rehost_layout(rainbow_);
            gallery_rehost_layout(root_);
            gallery_rehost_content(page_);

            drive_synthetic_drag_drop();
        }

        // The owned lists + readout, exposed for the hosting main / headless tests.
        [[nodiscard]] maui::controls::vertical_stack_layout& all_colors()
        {
            return all_colors_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& rainbow()
        {
            return rainbow_;
        }
        [[nodiscard]] maui::controls::label& move_readout()
        {
            return move_readout_;
        }

        // One deterministic drag -> over -> drop, moving the FIRST All-colors swatch into the Rainbow
        // list (the C# cross-layout move) through the recognizers' send_* seams. Leaves the readouts +
        // list contents reflecting the completed move.
        void drive_synthetic_drag_drop()
        {
            if (all_swatches_.empty())
            {
                return;
            }
            const std::shared_ptr<maui::controls::box_view> dragged = all_swatches_.front();

            // 1. DragStarting on the source swatch — stamps "Color"/"Source", shows start readouts, tints
            //    the OTHER (rainbow) list LightBlue.
            maui::controls::drag_gesture_recognizer* const source_drag = swatch_drag_.at(dragged.get());
            maui::controls::drag_starting_event_args start = source_drag->send_drag_starting(*dragged);
            // The C# OnDragStarting body (the port runs it directly: the recognizer's drag_starting event
            // is wired per-swatch in add_swatch, so send_drag_starting already raised it — see add_swatch).

            // 2. DragOver on the rainbow list (the receiving target) — builds a drag_event_args over the
            //    package the swatch stamped, runs the target's DragOver (readouts + LightPink tint).
            maui::controls::data_package package;
            package.properties().set("Color", std::any(dragged_color_));
            package.properties().set("Source", std::any(std::string("All colors")));
            maui::controls::drag_event_args over{package};
            rainbow_drop_->send_drag_over(over);

            // 3. Drop on the rainbow list — read the package, move the swatch, reset tints.
            maui::controls::drop_event_args drop{package.view()};
            rainbow_drop_->send_drop(drop, &rainbow_);
            (void)start;
        }

    private:
        // Build one draggable color swatch into `list`: a colored box_view + its own DragGestureRecognizer
        // wired to the DragStarting/DropCompleted handlers (the C# colorTemplate + OnDragStarting/
        // OnDropCompleted). Tracks the swatch and its color so the move logic can find them.
        void add_swatch(maui::controls::vertical_stack_layout& list, maui::graphics::color color, const char* name)
        {
            auto swatch = std::make_shared<maui::controls::box_view>();
            swatch->set_color(color);
            swatch->set_height_request(40);

            auto drag = std::make_shared<maui::controls::drag_gesture_recognizer>();
            const std::string swatch_name = name;
            maui::controls::box_view* const swatch_ptr = swatch.get();
            drag->drag_starting.connect([this, color, swatch_name](maui::controls::drag_starting_event_args& e) {
                // OnDragStarting: stamp the color + a representative position, show the start readouts.
                dragged_color_ = color;
                e.data().properties().set("Color", std::any(color));
                drag_starting_position_.set_text("- Self X:0, Y:0  (" + swatch_name + ")");
                // Tint the OTHER list LightBlue (the C# "highlight the drop destination" cue).
                tint(rainbow_, maui::graphics::colors::light_blue);
            });
            drag->drop_completed.connect([this](const maui::controls::drop_completed_event_args&) {
                // OnDropCompleted: reset both list tints.
                tint(all_colors_, maui::graphics::colors::transparent);
                tint(rainbow_, maui::graphics::colors::transparent);
            });
            swatch->gesture_recognizers().add(drag);

            list.add(*swatch);
            swatch_drag_[swatch_ptr] = drag.get();
            swatch_colors_[swatch_ptr] = color;
            if (&list == &all_colors_)
            {
                all_swatches_.push_back(swatch);
            }
            else
            {
                rainbow_swatches_.push_back(swatch);
            }
            swatch_drags_.push_back(drag); // keep the recognizer alive (collection co-owns, we mirror)
            swatches_.push_back(swatch);   // keep the box_view alive beyond the layout's i_view borrow
        }

        // Wire a list's DropGestureRecognizer to the DragOver/DragLeave/Drop handlers (the C#
        // OnDragOver/OnDragLeave/OnDrop), reporting onto the readouts and tinting the receiving list.
        void wire_drop_target(const std::shared_ptr<maui::controls::drop_gesture_recognizer>& drop,
                              maui::controls::vertical_stack_layout& list, const char* list_name)
        {
            const std::string name = list_name;
            maui::controls::vertical_stack_layout* const list_ptr = &list;
            drop->drag_over.connect([this, name, list_ptr](maui::controls::drag_event_args& e) {
                if (!e.data().properties().contains_key("Source"))
                {
                    // (no source stamped — the port's synthetic package always stamps one)
                }
                drag_position_.set_text("- Receiving layout (" + name + ") X:10, Y:10");
                tint(*list_ptr, maui::graphics::colors::light_pink);
            });
            drop->drag_leave.connect([this, name, list_ptr](maui::controls::drag_event_args&) {
                drag_position_.set_text("- Left layout (" + name + ")");
                tint(*list_ptr, maui::graphics::colors::light_blue);
            });
            drop->drop.connect([this, name, list_ptr](maui::controls::drop_event_args& e) {
                drop_position_.set_text("- Receiving layout (" + name + ") X:10, Y:10");
                do_move_on_drop(e, *list_ptr, name);
            });
        }

        // OnDrop's move logic: read "Color" from the dropped package, find the swatch carrying it, and
        // move that box_view from its current list into the receiving list. Resets both tints.
        void do_move_on_drop(const maui::controls::drop_event_args& e, maui::controls::vertical_stack_layout& receiving,
                             const std::string& name)
        {
            const std::any* const color_any = e.data().properties().try_get_value("Color");
            if (color_any == nullptr || !color_any->has_value())
            {
                return;
            }
            maui::graphics::color color{};
            try
            {
                color = std::any_cast<maui::graphics::color>(*color_any);
            }
            catch (const std::bad_any_cast&)
            {
                return;
            }

            // Find the swatch with this color in the OTHER list (the source) and move it here.
            maui::controls::vertical_stack_layout& source = (&receiving == &rainbow_) ? all_colors_ : rainbow_;
            std::vector<std::shared_ptr<maui::controls::box_view>>& source_list =
                (&receiving == &rainbow_) ? all_swatches_ : rainbow_swatches_;
            std::vector<std::shared_ptr<maui::controls::box_view>>& dest_list =
                (&receiving == &rainbow_) ? rainbow_swatches_ : all_swatches_;

            for (std::size_t i = 0; i < source_list.size(); ++i)
            {
                if (swatch_colors_[source_list[i].get()] == color)
                {
                    const std::shared_ptr<maui::controls::box_view> moved = source_list[i];
                    const int at = source.index_of(*moved);
                    if (at >= 0)
                    {
                        source.remove_at(at);
                    }
                    receiving.add(*moved);
                    dest_list.push_back(moved);
                    source_list.erase(source_list.begin() + static_cast<std::ptrdiff_t>(i));
                    move_readout_.set_text("Move: swatch dropped into " + name);
                    break;
                }
            }
            tint(all_colors_, maui::graphics::colors::transparent);
            tint(rainbow_, maui::graphics::colors::transparent);
        }

        // Tint a list's background (the C# sl.Background = SolidColorBrush.X cue).
        static void tint(maui::controls::vertical_stack_layout& list, maui::graphics::color color)
        {
            list.set_background(std::make_shared<maui::graphics::solid_paint>(color));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout root_;

        maui::controls::label all_colors_title_;
        maui::controls::label rainbow_title_;
        maui::controls::vertical_stack_layout all_colors_;
        maui::controls::vertical_stack_layout rainbow_;

        maui::controls::label drag_starting_title_;
        maui::controls::label drag_starting_position_;
        maui::controls::label drag_title_;
        maui::controls::label drag_position_;
        maui::controls::label drop_title_;
        maui::controls::label drop_position_;
        maui::controls::label move_readout_;

        // The two list drop targets.
        std::shared_ptr<maui::controls::drop_gesture_recognizer> all_drop_ =
            std::make_shared<maui::controls::drop_gesture_recognizer>();
        std::shared_ptr<maui::controls::drop_gesture_recognizer> rainbow_drop_ =
            std::make_shared<maui::controls::drop_gesture_recognizer>();

        // The swatches per list (the live membership the move mutates), plus strong-ref keep-alive lists
        // and the per-swatch color/recognizer maps the move + drive read.
        std::vector<std::shared_ptr<maui::controls::box_view>> all_swatches_;
        std::vector<std::shared_ptr<maui::controls::box_view>> rainbow_swatches_;
        std::vector<std::shared_ptr<maui::controls::box_view>> swatches_;                    // keep every swatch alive
        std::vector<std::shared_ptr<maui::controls::drag_gesture_recognizer>> swatch_drags_; // keep drags alive
        std::map<maui::controls::box_view*, maui::controls::drag_gesture_recognizer*> swatch_drag_;
        std::map<maui::controls::box_view*, maui::graphics::color> swatch_colors_;
        maui::graphics::color dragged_color_{}; // the color of the swatch currently being dragged
    };
} // namespace maui::samples
