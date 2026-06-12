// view_chrome_ops — iOS (UIKit) platform recipe behind the shared view_mapper's "tool_tip" /
// "context_flyout" maps (view_chrome_ops.hpp).
//   - apply_native_tool_tip: documented NO-OP — C# materializes tooltips on desktop/Catalyst only
//     (ToolTipExtensions has Windows + a UIToolTipInteraction guarded to Mac Catalyst; a plain-iOS
//     touch device shows none). The view_platform_base mirror stays the observable state.
//   - apply_native_context_flyout: a REAL UIContextMenuInteraction attached to the view (the C#
//     MauiUIContextMenuInteraction from FlyoutExtensions/iOS). The interaction's delegate builds the
//     UIMenu ON DEMAND from the flyout's i_menu_element tree when the user long-presses — menu
//     materialization needs that interaction, so tests assert the ATTACH (view.interactions) only.
//     The delegate trampoline is retained via an associated object on the view (the interaction holds
//     it weakly); re-attaching replaces the previous interaction, null removes it.
// Compiled as Objective-C++ with ARC for the ios backend.

#include "maui/core/view_chrome_ops.hpp"

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/core/i_flyout.hpp"
#include "maui/core/i_menu_element.hpp"
#include "maui/core/i_menu_flyout.hpp"
#include "maui/core/i_menu_flyout_separator.hpp"
#include "maui/core/i_menu_flyout_sub_item.hpp"

// The context-menu delegate trampoline: borrows the attached i_flyout (the control owns it) and
// builds the UIMenu when UIKit asks (the user's long-press). Each UIAction routes back through
// i_menu_element::send_clicked.
@interface MauiContextFlyoutInteraction : NSObject <UIContextMenuInteractionDelegate>
@property(nonatomic) const maui::core::i_flyout* flyout;
@end

namespace
{
    // The associated-object keys anchoring the retained delegate + its interaction to the UIView.
    const void* context_delegate_key()
    {
        static const char key = 0;
        return &key;
    }
    const void* context_interaction_key()
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

    // One element → a UIMenuElement: a leaf becomes a UIAction (send_clicked routing + enabled state);
    // a sub item becomes a nested UIMenu; a separator becomes an inline UIMenu boundary (UIKit has no
    // literal separator element — the inline-section break is the UIMenu idiom for it).
    UIMenuElement* build_element(maui::core::i_menu_element* element);

    NSArray<UIMenuElement*>* build_children(const std::vector<maui::core::i_menu_element*>& elements)
    {
        NSMutableArray<UIMenuElement*>* const children = [NSMutableArray array];
        for (maui::core::i_menu_element* const element : elements)
        {
            if (element == nullptr)
            {
                continue;
            }
            if (UIMenuElement* const built = build_element(element))
            {
                [children addObject:built];
            }
        }
        return children;
    }

    UIMenuElement* build_element(maui::core::i_menu_element* element)
    {
        if (dynamic_cast<maui::core::i_menu_flyout_separator*>(element) != nullptr)
        {
            // An empty inline section reads as a separator line between the neighbouring items.
            return [UIMenu menuWithTitle:@"" image:nil identifier:nil options:UIMenuOptionsDisplayInline children:@[]];
        }
        if (auto* sub = dynamic_cast<maui::core::i_menu_flyout_sub_item*>(element))
        {
            std::vector<maui::core::i_menu_element*> nested;
            nested.reserve(sub->item_count());
            for (std::size_t i = 0; i < sub->item_count(); ++i)
            {
                nested.push_back(sub->item_at(i));
            }
            return [UIMenu menuWithTitle:to_ns_string(element->text()) children:build_children(nested)];
        }
        maui::core::i_menu_element* const borrowed = element;
        UIAction* const action = [UIAction actionWithTitle:to_ns_string(element->text())
                                                     image:nil
                                                identifier:nil
                                                   handler:^(UIAction* act) {
                                                     (void)act;
                                                     borrowed->send_clicked();
                                                   }];
        if (!element->is_enabled())
        {
            action.attributes = UIMenuElementAttributesDisabled;
        }
        return action;
    }
} // namespace

@implementation MauiContextFlyoutInteraction
- (UIContextMenuConfiguration*)contextMenuInteraction:(UIContextMenuInteraction*)interaction
                       configurationForMenuAtLocation:(CGPoint)location
{
    (void)interaction;
    (void)location;
    const auto* menu_flyout = dynamic_cast<const maui::core::i_menu_flyout*>(self.flyout);
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
    NSArray<UIMenuElement*>* const children = build_children(elements);
    return [UIContextMenuConfiguration configurationWithIdentifier:nil
                                                   previewProvider:nil
                                                    actionProvider:^UIMenu*(NSArray<UIMenuElement*>* suggested) {
                                                      (void)suggested;
                                                      return [UIMenu menuWithTitle:@"" children:children];
                                                    }];
}
@end

namespace maui::core
{
    void apply_native_tool_tip(void* /*native_view*/, const std::optional<std::string>& /*text*/)
    {
        // Documented no-op: C# shows tooltips on desktop/Catalyst only — a plain-iOS touch device has
        // no hover tooltip surface (the view_platform_base mirror remains the observable state).
    }

    void apply_native_context_flyout(void* native_view, const i_flyout* flyout)
    {
        if (native_view == nullptr)
        {
            return;
        }
        UIView* const view = (__bridge UIView*)native_view;
        // Detach any previous interaction (a replace or a clear).
        if (UIContextMenuInteraction* const previous = objc_getAssociatedObject(view, context_interaction_key()))
        {
            [view removeInteraction:previous];
            objc_setAssociatedObject(view, context_interaction_key(), nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            objc_setAssociatedObject(view, context_delegate_key(), nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
        if (flyout == nullptr)
        {
            return;
        }
        MauiContextFlyoutInteraction* const delegate = [[MauiContextFlyoutInteraction alloc] init];
        delegate.flyout = flyout;
        UIContextMenuInteraction* const interaction = [[UIContextMenuInteraction alloc] initWithDelegate:delegate];
        [view addInteraction:interaction];
        // The interaction holds its delegate weakly — anchor both to the view.
        objc_setAssociatedObject(view, context_delegate_key(), delegate, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        objc_setAssociatedObject(view, context_interaction_key(), interaction, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
} // namespace maui::core
