// picker_handler — Apple (AppKit / macOS) platform recipe. The managed platform view is an
// NSPopUpButton (held, retained, in picker_platform::native): the items rebuild the popup menu (the
// PickerExtensions.UpdatePicker reload, reading through the i_item_delegate face), the selection
// drives selectItemAtIndex, and a native pick flows back through a target-action trampoline into
// i_picker::set_selected_index. Compiled as Objective-C++ with ARC only for the `apple` backend.
//
// Idiomatic translation of the C# recipes (MAUI's macOS is Mac Catalyst, which presents a
// UITextField + UIAlertController wheel — see PickerHandler.iOS.cs; AppKit's native idiom is the
// popup button). Documented deviations:
//   - an NSPopUpButton commits a pick on menu selection, so the write-back is immediate (the
//     UpdateMode.Immediately platform-specific is the only behavior; there is no Done accessory).
//     `on_done` is still wired to the same commit (FinishSelectItem semantics) for portable drives.
//   - the Title plays its placeholder role only while nothing is selected: a DETACHED NSMenuItem on
//     the popup cell displays it (titled in title_color) without disturbing the menu/indices.
//   - menu items are appended via NSMenuItem (addItemWithTitle: would de-duplicate equal titles).
//   - vertical_text_alignment has no AppKit popup analog (the bezel centers its single line) and is
//     intentionally not pushed; horizontal alignment lands on NSControl.alignment.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <string_view>

#include "apple_conversions.hpp"
#include "apple_semantics_ops.hpp"
#include "apple_text_ops.hpp"
#include "apple_view_ops.hpp"
#include "apple_visual_ops.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the NSPopUpButton's target-action (fired on a menu pick) to the C++
// handler — the PickerSource.Selected + UpdatePickerFromPickerSource port (immediate commit).
@interface MauiPopUpTarget : NSObject
@property(nonatomic) maui::core::picker_handler* handler;
- (void)onSelectionChanged:(id)sender;
@end

namespace
{
    // Key for the associated MauiPopUpTarget kept alive by the NSPopUpButton (its `target` is weak).
    const char k_target_key = 0;

    NSPopUpButton* as_popup(void* native)
    {
        return (__bridge NSPopUpButton*)native;
    }

    NSTextAlignment to_ns_text_alignment(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return NSTextAlignmentCenter;
            case maui::core::text_alignment::end:
                return NSTextAlignmentRight;
            case maui::core::text_alignment::justify:
                return NSTextAlignmentJustified;
            case maui::core::text_alignment::start:
                break;
        }
        return NSTextAlignmentLeft;
    }

    NSString* to_ns_string(std::string_view value)
    {
        const std::string utf8(value);
        NSString* const result = [NSString stringWithUTF8String:utf8.c_str()];
        return result != nil ? result : @"";
    }

    // The display refresh shared by the title/color/spacing maps: a selected row shows its menu
    // item (kerned/colored via the item's attributedTitle); no selection shows the Title placeholder
    // on a DETACHED cell item (UpdatePickerTitle — the attributed-placeholder analog).
    void refresh_display(maui::core::picker_handler& handler, const maui::core::i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSPopUpButton* const popup = as_popup(platform->native);
        const double spacing = view.character_spacing();
        const maui::graphics::color text_color = view.text_color();
        NSColor* const text_foreground =
            text_color != maui::graphics::color{} ? maui::platform::apple::to_ns_color(text_color) : nil;
        const NSInteger selected = popup.indexOfSelectedItem;
        if (selected >= 0)
        {
            NSMenuItem* const item = [popup itemAtIndex:selected];
            NSAttributedString* const attributed =
                maui::platform::apple::kern_attributed(item.title, spacing, text_foreground);
            if (attributed != nil)
            {
                item.attributedTitle = attributed;
            }
            return;
        }
        // No selection: display the Title as a placeholder via a detached menu item on the cell.
        const maui::graphics::color title_color = view.title_color();
        NSColor* const title_foreground =
            title_color != maui::graphics::color{} ? maui::platform::apple::to_ns_color(title_color) : nil;
        NSString* const title = to_ns_string(view.title());
        NSMenuItem* const placeholder = [[NSMenuItem alloc] initWithTitle:title action:nil keyEquivalent:@""];
        NSAttributedString* const attributed = maui::platform::apple::kern_attributed(title, spacing, title_foreground);
        if (attributed != nil)
        {
            placeholder.attributedTitle = attributed;
        }
        [(NSPopUpButtonCell*)popup.cell setMenuItem:placeholder];
        [popup setNeedsDisplay:YES];
    }

    // PickerExtensions.UpdatePicker(platformPicker, picker, newSelectedIndex): reload the menu from
    // the item delegate, select the row, then write the selection back to the virtual view (skipped
    // while empty), and refresh the displayed title.
    void update_picker(maui::core::picker_handler& handler, maui::core::i_picker& view, int selected_index)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        NSPopUpButton* const popup = as_popup(platform->native);
        [popup removeAllItems]; // Reload (ReloadAllComponents)
        const int count = view.get_count();
        for (int at = 0; at < count; ++at)
        {
            NSMenuItem* const item = [[NSMenuItem alloc] initWithTitle:to_ns_string(view.get_item(at))
                                                                action:nil
                                                         keyEquivalent:@""];
            [popup.menu addItem:item];
        }
        if (selected_index >= 0 && selected_index < count)
        {
            [popup selectItemAtIndex:selected_index];
        }
        else
        {
            [popup selectItem:nil];
        }
        if (count != 0)
        {
            view.set_selected_index(selected_index); // picker.SelectedIndex = selectedIndex
        }
        refresh_display(handler, view);
    }

    // FinishSelectItem: an unset (-1) row with items present commits row 0, then the pick lands on
    // the virtual view through UpdatePicker.
    void commit_row(maui::core::picker_handler& handler, int row)
    {
        auto* view = handler.virtual_view();
        if (view == nullptr)
        {
            return;
        }
        if (row == -1 && view->get_count() > 0)
        {
            row = 0;
        }
        update_picker(handler, *view, row);
    }
} // namespace

@implementation MauiPopUpTarget
- (void)onSelectionChanged:(id)sender
{
    if (self.handler == nullptr)
    {
        return;
    }
    auto* const native = (NSPopUpButton*)sender;
    commit_row(*self.handler, static_cast<int>(native.indexOfSelectedItem));
}
@end

namespace maui::core
{
    picker_platform::~picker_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void picker_platform::update_visibility(maui::core::visibility value)
    {
        as_popup(native).hidden = value != maui::core::visibility::visible;
    }

    void picker_platform::update_opacity(double value)
    {
        as_popup(native).alphaValue = value;
    }

    void picker_platform::update_is_enabled(bool value)
    {
        [as_popup(native) setEnabled:static_cast<BOOL>(value)];
    }

    void picker_platform::update_automation_id(std::string_view value)
    {
        as_popup(native).accessibilityIdentifier = to_ns_string(value);
    }

    void picker_platform::update_transform(const maui::core::transform_spec& value)
    {
        maui::platform::apple::apply_transform(native, value);
    }

    void picker_platform::update_flow_direction(maui::core::flow_direction value)
    {
        maui::platform::apple::apply_flow_direction(native, value);
    }

    void picker_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::apple::apply_background(native, value);
    }

    void picker_platform::update_shadow(const maui::core::i_shadow* value)
    {
        maui::platform::apple::apply_shadow(native, value);
    }

    void picker_platform::update_clip(const maui::graphics::i_shape* value)
    {
        const NSRect bounds = ((__bridge NSView*)native).bounds;
        maui::platform::apple::apply_clip(
            native, value,
            maui::graphics::rect{bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height});
    }

    void picker_platform::update_semantics(const maui::core::semantics* value)
    {
        maui::platform::apple::apply_semantics((__bridge NSView*)native, value);
    }

    void picker_platform::update_input_transparent(bool value)
    {
        maui::platform::apple::apply_input_transparent((__bridge NSView*)native, value);
    }

    std::unique_ptr<picker_platform> picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<picker_platform>();
        NSPopUpButton* const native = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 0, 0) pullsDown:NO];
        platform->native = (__bridge_retained void*)native; // the void* slot owns one reference
        return platform;
    }

    void picker_handler::on_connect_handler(picker_platform& platform)
    {
        NSPopUpButton* const native = as_popup(platform.native);
        MauiPopUpTarget* const target = [[MauiPopUpTarget alloc] init];
        target.handler = this;
        native.target = target; // NSControl holds its target weakly (target-action convention)...
        native.action = @selector(onSelectionChanged:);
        // ...so keep it alive for the popup's lifetime via an associated object.
        objc_setAssociatedObject(native, &k_target_key, target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        // FinishSelectItem for portable drives (the popup itself commits per pick — see the header).
        platform.on_done = [this](int row) { commit_row(*this, row); };
    }

    void picker_handler::on_disconnect_handler(picker_platform& platform)
    {
        NSPopUpButton* const native = as_popup(platform.native);
        native.target = nil;
        native.action = nil;
        objc_setAssociatedObject(native, &k_target_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        platform.on_done = nullptr;
    }

    void picker_handler::map_items(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // Reload -> UpdatePicker(picker)
    }

    void picker_handler::map_selected_index(picker_handler& handler, i_picker& view)
    {
        update_picker(handler, view, view.selected_index()); // UpdateSelectedIndex
    }

    void picker_handler::map_title(picker_handler& handler, i_picker& view)
    {
        refresh_display(handler, view); // UpdatePickerTitle (the placeholder)
    }

    void picker_handler::map_title_color(picker_handler& handler, i_picker& view)
    {
        refresh_display(handler, view);
    }

    void picker_handler::map_text_color(picker_handler& handler, i_picker& view)
    {
        refresh_display(handler, view);
    }

    void picker_handler::map_font(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_popup(platform->native).font = maui::platform::apple::to_ns_font(view.font());
        }
    }

    void picker_handler::map_character_spacing(picker_handler& handler, i_picker& view)
    {
        refresh_display(handler, view);
    }

    void picker_handler::map_horizontal_text_alignment(picker_handler& handler, i_picker& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            as_popup(platform->native).alignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void picker_handler::map_vertical_text_alignment(picker_handler& handler, i_picker& view)
    {
        // No AppKit popup analog (the bezel centers its single line) — see the header note. The
        // mirror on picker_platform stays headless-only.
        (void)handler;
        (void)view;
    }

    maui::graphics::size picker_handler::get_desired_size(double /*width_constraint*/,
                                                          double /*height_constraint*/) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        const NSSize fitting = [as_popup(platform->native) fittingSize];
        return {fitting.width, fitting.height};
    }

    void picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_popup(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
