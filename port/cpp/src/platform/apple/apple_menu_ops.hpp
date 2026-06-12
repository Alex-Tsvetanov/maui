#pragma once
// Shared AppKit menu construction — the platform side of the W1-11 menu chrome: one NSMenu builder the
// window's main menu (window_handler.mm), the context flyout (view_chrome_ops.mm), and the toolbar's
// secondary/overflow menu all reuse. Objective-C++ only — include exclusively from .mm files.
//
// Ported from the Mac Catalyst menu construction (src/Core/src/Platform/iOS/MenuExtensions.cs +
// MenuFlyoutItemHandler/MenuFlyoutSubItemHandler.iOS — UIMenu/UIAction trees built from the IMenuElement
// tree), translated to AppKit's NSMenu/NSMenuItem:
//   - a leaf i_menu_element → an NSMenuItem (title = text(), enabled = is_enabled(), the first
//     keyboard accelerator → keyEquivalent + modifier mask) whose action routes BACK to
//     i_menu_element::send_clicked() through a retained target trampoline (the UIAction handler role);
//   - an i_menu_flyout_separator → NSMenuItem.separatorItem;
//   - an i_menu_flyout_sub_item → an NSMenuItem carrying a recursive NSMenu submenu.
// The single click target is associated to the returned NSMenu (objc_setAssociatedObject, retained), so
// the menu owns its trampoline; the menu items only borrow the i_menu_element pointers — the controls
// own the menu tree (PROFILE §8), and the chrome rebuilds whole menus on change (no per-item handlers).
// autoenablesItems is disabled so is_enabled() drives the items (NSMenu would otherwise grey out
// targetless validation).

#import <AppKit/AppKit.h>

#include <cstddef>
#include <vector>

#include "maui/core/i_flyout.hpp"
#include "maui/core/i_menu_element.hpp"

namespace maui::platform::apple
{
    // Build an NSMenu (title `title`) whose items materialize `elements` (recursively). The menu OWNS a
    // retained click trampoline; the element pointers are borrowed (the controls own them).
    NSMenu* build_menu(NSString* title, const std::vector<maui::core::i_menu_element*>& elements);

    // Build the context menu for a flyout (null or non-i_menu_flyout → nil). The i_menu_flyout's
    // count/at walk feeds build_menu.
    NSMenu* build_menu_from_flyout(const maui::core::i_flyout* flyout);
} // namespace maui::platform::apple
