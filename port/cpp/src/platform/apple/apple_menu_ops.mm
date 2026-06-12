// apple_menu_ops — the AppKit NSMenu builder behind the W1-11 menu chrome (see apple_menu_ops.hpp).
// Compiled once into the apple backend; the trampoline class lives here so every includer shares one
// Obj-C class definition.

#include "apple_menu_ops.hpp"

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/i_menu_flyout.hpp"
#include "maui/core/i_menu_flyout_item.hpp"
#include "maui/core/i_menu_flyout_separator.hpp"
#include "maui/core/i_menu_flyout_sub_item.hpp"
#include "maui/core/keyboard_accelerator.hpp"

// The shared click trampoline: every built NSMenuItem targets this object; the acted-on item's
// representedObject carries the borrowed i_menu_element*, which routes the click back to the
// cross-platform control (send_clicked → the clicked event — the UIAction handler role).
@interface MauiMenuItemTarget : NSObject
- (void)itemClicked:(NSMenuItem*)sender;
@end

@implementation MauiMenuItemTarget
- (void)itemClicked:(NSMenuItem*)sender
{
    NSValue* const value = sender.representedObject;
    if (value == nil)
    {
        return;
    }
    auto* const element = static_cast<maui::core::i_menu_element*>(value.pointerValue);
    if (element != nullptr)
    {
        element->send_clicked(); // IMenuElement.Clicked() → MenuItem.Activate → clicked
    }
}
@end

namespace
{
    // The associated-object key anchoring the retained trampoline to its NSMenu.
    const void* menu_target_key()
    {
        static const char key = 0;
        return &key;
    }

    NSString* to_ns_string(std::string_view text)
    {
        const std::string owned(text);
        NSString* const value = [NSString stringWithUTF8String:owned.c_str()];
        return value != nil ? value : @"";
    }

    // The first accelerator's key/modifiers → keyEquivalent + mask (KeyboardAcceleratorExtensions.iOS's
    // UIKeyCommand mapping, translated to AppKit; Windows-only modifiers have no AppKit analog).
    void apply_accelerator(NSMenuItem* item, const maui::core::i_menu_element& element)
    {
        const auto* flyout_item = dynamic_cast<const maui::core::i_menu_flyout_item*>(&element);
        if (flyout_item == nullptr)
        {
            return;
        }
        const std::vector<maui::core::keyboard_accelerator> accelerators = flyout_item->keyboard_accelerators();
        if (accelerators.empty() || accelerators.front().key.empty())
        {
            return;
        }
        const maui::core::keyboard_accelerator& accelerator = accelerators.front();
        item.keyEquivalent = [to_ns_string(accelerator.key) lowercaseString];
        NSEventModifierFlags mask = 0;
        if (has_modifier(accelerator.modifiers, maui::core::keyboard_accelerator_modifiers::shift))
        {
            mask |= NSEventModifierFlagShift;
        }
        if (has_modifier(accelerator.modifiers, maui::core::keyboard_accelerator_modifiers::ctrl))
        {
            mask |= NSEventModifierFlagControl;
        }
        if (has_modifier(accelerator.modifiers, maui::core::keyboard_accelerator_modifiers::alt))
        {
            mask |= NSEventModifierFlagOption;
        }
        if (has_modifier(accelerator.modifiers, maui::core::keyboard_accelerator_modifiers::cmd))
        {
            mask |= NSEventModifierFlagCommand;
        }
        item.keyEquivalentModifierMask = mask;
    }

    // One element → one NSMenuItem (or the separator singleton). `target` is the menu's trampoline.
    NSMenuItem* build_item(maui::core::i_menu_element* element, MauiMenuItemTarget* target)
    {
        if (dynamic_cast<maui::core::i_menu_flyout_separator*>(element) != nullptr)
        {
            return [NSMenuItem separatorItem];
        }
        NSMenuItem* const item = [[NSMenuItem alloc] initWithTitle:to_ns_string(element->text())
                                                            action:@selector(itemClicked:)
                                                     keyEquivalent:@""];
        item.representedObject = [NSValue valueWithPointer:element];
        item.enabled = static_cast<BOOL>(element->is_enabled());
        apply_accelerator(item, *element);

        if (auto* sub = dynamic_cast<maui::core::i_menu_flyout_sub_item*>(element))
        {
            // A sub-menu: the item opens a recursive NSMenu instead of firing an action.
            item.action = nil;
            std::vector<maui::core::i_menu_element*> children;
            children.reserve(sub->item_count());
            for (std::size_t i = 0; i < sub->item_count(); ++i)
            {
                children.push_back(sub->item_at(i));
            }
            item.submenu = maui::platform::apple::build_menu(to_ns_string(element->text()), children);
        }
        else
        {
            item.target = target;
        }
        return item;
    }
} // namespace

namespace maui::platform::apple
{
    NSMenu* build_menu(NSString* title, const std::vector<maui::core::i_menu_element*>& elements)
    {
        NSMenu* const menu = [[NSMenu alloc] initWithTitle:title != nil ? title : @""];
        menu.autoenablesItems = NO; // is_enabled() drives the items, not NSMenu validation
        MauiMenuItemTarget* const target = [[MauiMenuItemTarget alloc] init];
        // The menu owns its trampoline (NSMenuItem.target is weak) — associate it retained.
        objc_setAssociatedObject(menu, menu_target_key(), target, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        for (maui::core::i_menu_element* const element : elements)
        {
            if (element != nullptr)
            {
                [menu addItem:build_item(element, target)];
            }
        }
        return menu;
    }

    NSMenu* build_menu_from_flyout(const maui::core::i_flyout* flyout)
    {
        const auto* menu_flyout = dynamic_cast<const maui::core::i_menu_flyout*>(flyout);
        if (menu_flyout == nullptr)
        {
            return nil;
        }
        std::vector<maui::core::i_menu_element*> elements;
        elements.reserve(menu_flyout->item_count());
        for (std::size_t i = 0; i < menu_flyout->item_count(); ++i)
        {
            elements.push_back(menu_flyout->item_at(i));
        }
        return build_menu(@"", elements);
    }
} // namespace maui::platform::apple
