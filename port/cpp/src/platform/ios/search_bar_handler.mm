// search_bar_handler — iOS (UIKit) platform recipe. The managed platform view is a real UISearchBar,
// the value properties map to it (most through its inner UISearchTextField — C#'s QueryEditor), and
// native events flow back through a UISearchBarDelegate proxy to i_search_bar::send_text_changed(old,
// new) / send_search_button_pressed(). Compiled as Objective-C++ with ARC only for the `ios` backend.
//
// Ported DIRECTLY from SearchBarHandler.iOS.cs + MauiSearchBar.cs + Platform/iOS/SearchBarExtensions.cs:
//   CreatePlatformView = new MauiSearchBar { BarStyle = Default }; QueryEditor = GetSearchTextField()
//   (the public searchTextField on this SDK). MauiSearchBarProxy: TextSetOrChanged/EditingChanged →
//   UpdateText (send_text_changed), SearchButtonClicked → SearchButtonPressed
//   (send_search_button_pressed), CancelButtonClicked → virtualView.Text = string.Empty,
//   ShouldChangeTextInRange → the MaxLength gate, plus UpdateCancelButtonVisibility on text changes.
//   Map bodies below = SearchBarExtensions.UpdateText/UpdatePlaceholder/UpdateIsReadOnly/
//   UpdateMaxLength/UpdateCancelButton/UpdateSearchIcon(leftView tint)/UpdateReturnType +
//   TextFieldish font/color/alignment/prediction/spellcheck pushes onto the QueryEditor.
// Keyboard subsystem (W8-53): MapKeyboard pushes UIKeyboardType + the autocapitalization/spellcheck/
// autocorrection traits onto the search field (ios_keyboard_ops.hpp). The Done input accessory is NOT
// added (C# adds it only on Entry/Editor, not SearchBar). Focus (W8-53): the begin/end editing delegate
// callbacks reflect IsFocused; the shared view_command_mapper drives becomeFirstResponder on the bar.
// Not ported here (deferred): OnMovedToWindow's cancel-color re-fire (the cancel button is tinted
// directly when visible), and the QueryEditor UITextPosition cursor arithmetic beyond the clamped-range
// write (the entry carries the full port).

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "ios_conversions.hpp"
#include "ios_keyboard_ops.hpp"
#include "ios_text_ops.hpp"
#include "maui/core/i_search_bar.hpp"
#include "maui/core/return_type.hpp"
#include "maui/core/search_bar_handler.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

// Obj-C trampoline: forwards the UISearchBar's delegate callbacks to the C++ handler's virtual view.
// Ports SearchBarHandler.MauiSearchBarProxy — tracking the previous string for the (old, new) pair.
@interface MauiIosSearchBarProxy : NSObject <UISearchBarDelegate>
@property(nonatomic) maui::core::search_bar_handler* handler;
@property(nonatomic, copy) NSString* previousText;
@end

namespace
{
    // Key for the associated MauiIosSearchBarProxy kept alive by the UISearchBar (`delegate` is weak).
    const char k_proxy_key = 0;

    UISearchBar* as_search_bar(void* native)
    {
        return (__bridge UISearchBar*)native;
    }

    UISearchTextField* query_editor(void* native)
    {
        return as_search_bar(native).searchTextField; // C# GetSearchTextField / QueryEditor
    }

    using maui::platform::ios::to_ns_text_alignment;
    using maui::platform::ios::to_ui_color;
    using maui::platform::ios::to_ui_control_content_vertical_alignment;
    using maui::platform::ios::to_ui_font;
    using maui::platform::ios::to_ui_return_key_type;
    using maui::platform::ios::with_character_spacing;

    // SearchBarExtensions.UpdatePlaceholder: plain placeholder unless an explicit color is set (the
    // default-constructed opaque-black color counts as "unset" — the entry collapse).
    void refresh_search_placeholder(UISearchBar* bar, const maui::core::i_search_bar& view)
    {
        const std::string placeholder(view.placeholder());
        NSString* const text = [NSString stringWithUTF8String:placeholder.c_str()];
        const maui::graphics::color color = view.placeholder_color();
        const bool explicit_color = color != maui::graphics::color{};
        if (!explicit_color)
        {
            bar.placeholder = text;
            return;
        }
        if (text == nil || text.length == 0)
        {
            bar.placeholder = text;
            return;
        }
        bar.searchTextField.attributedPlaceholder =
            [[NSAttributedString alloc] initWithString:text
                                            attributes:@{NSForegroundColorAttributeName : to_ui_color(color)}];
    }

    // SearchBarHandler.UpdateCancelButtonVisibility + SearchBarExtensions.UpdateCancelButton: the
    // cancel button shows while the bar has text; when visible and an explicit color is set, tint the
    // cancel UIButton (C# finds the descendant UIButton the same way).
    UIButton* find_descendant_button(UIView* root)
    {
        for (UIView* subview in root.subviews)
        {
            if ([subview isKindOfClass:[UIButton class]])
            {
                return (UIButton*)subview;
            }
            if (UIButton* const nested = find_descendant_button(subview))
            {
                return nested;
            }
        }
        return nil;
    }

    void refresh_cancel_button(UISearchBar* bar, const maui::core::i_search_bar& view)
    {
        const bool should_show = bar.text != nil && bar.text.length > 0; // ShouldShowCancelButton()
        [bar setShowsCancelButton:should_show ? YES : NO animated:NO];
        const maui::graphics::color color = view.cancel_button_color();
        const bool explicit_color = color != maui::graphics::color{};
        if (!should_show || !explicit_color)
        {
            return;
        }
        if (UIButton* const cancel = find_descendant_button(bar))
        {
            UIColor* const tint = to_ui_color(color);
            cancel.tintColor = tint;
            [cancel setTitleColor:tint forState:UIControlStateNormal];
        }
    }

    // SearchBarExtensions.UpdateMaxLength's trim arm.
    void apply_max_length(UISearchBar* bar, const maui::core::i_search_bar& view)
    {
        const int max_length = view.max_length();
        if (max_length < 0)
        {
            return;
        }
        NSString* const current = bar.text != nil ? bar.text : @"";
        if (current.length > static_cast<NSUInteger>(max_length))
        {
            bar.text = [current substringToIndex:static_cast<NSUInteger>(max_length)];
        }
    }
} // namespace

@implementation MauiIosSearchBarProxy
- (void)mauiSyncTextFrom:(UISearchBar*)bar
{
    NSString* const previous = self.previousText;
    NSString* const current = bar.text;
    NSString* const old_value = previous != nil ? previous : @"";
    NSString* const new_value = current != nil ? current : @"";
    if ([old_value isEqualToString:new_value])
    {
        return;
    }
    self.previousText = new_value;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            const char* const old_utf8 = old_value.UTF8String;
            const char* const new_utf8 = new_value.UTF8String;
            view->send_text_changed(old_utf8 != nullptr ? old_utf8 : "", new_utf8 != nullptr ? new_utf8 : "");
            // OnTextPropertySet → UpdateCancelButtonVisibility.
            refresh_cancel_button(bar, *view);
        }
    }
}

- (void)searchBar:(UISearchBar*)searchBar textDidChange:(NSString*)searchText
{
    (void)searchText;
    [self mauiSyncTextFrom:searchBar];
}

- (void)searchBarSearchButtonClicked:(UISearchBar*)searchBar
{
    (void)searchBar;
    // OnSearchButtonClicked → VirtualView.SearchButtonPressed().
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->send_search_button_pressed();
        }
    }
}

- (void)searchBarCancelButtonClicked:(UISearchBar*)searchBar
{
    // OnCancelClicked → virtualView.Text = string.Empty: clear the native bar (its source of truth) and
    // report the change through the same diff channel.
    searchBar.text = @"";
    [self mauiSyncTextFrom:searchBar];
}

- (void)searchBarTextDidBeginEditing:(UISearchBar*)searchBar
{
    // The search field took first responder: reflect IsFocused = true onto the virtual view (fires
    // Focused + ChangeVisualState through set_is_focused) — the native focus callback's analog.
    (void)searchBar;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->set_is_focused(true);
        }
    }
}

- (void)searchBarTextDidEndEditing:(UISearchBar*)searchBar
{
    // The search field resigned first responder: reflect IsFocused = false (fires Unfocused).
    (void)searchBar;
    if (self.handler != nullptr)
    {
        if (auto* view = self.handler->virtual_view())
        {
            view->set_is_focused(false);
        }
    }
}

- (BOOL)searchBar:(UISearchBar*)searchBar shouldChangeTextInRange:(NSRange)range replacementText:(NSString*)text
{
    // ShouldChangeText: newLength <= MaxLength.
    auto* const view = self.handler != nullptr ? self.handler->virtual_view() : nullptr;
    if (view == nullptr)
    {
        return YES;
    }
    const int max_length = view->max_length();
    if (max_length < 0)
    {
        return YES;
    }
    NSString* const current = searchBar.text != nil ? searchBar.text : @"";
    if (range.location + range.length > current.length)
    {
        return NO;
    }
    const NSUInteger add_length = text != nil ? text.length : 0;
    const NSUInteger new_length = current.length + add_length - range.length;
    return new_length <= static_cast<NSUInteger>(max_length) ? YES : NO;
}
@end

namespace maui::core
{
    search_bar_platform::~search_bar_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    // The generic-IView property pushes (the shared view_mapper calls these via view_platform_base).
    void search_bar_platform::update_visibility(maui::core::visibility value)
    {
        as_search_bar(native).hidden = value != maui::core::visibility::visible;
    }

    void search_bar_platform::update_opacity(double value)
    {
        as_search_bar(native).alpha = value;
    }

    void search_bar_platform::update_is_enabled(bool value)
    {
        // SearchBarExtensions.UpdateIsEnabled → UserInteractionEnabled (a UISearchBar is not a UIControl).
        as_search_bar(native).userInteractionEnabled = static_cast<BOOL>(value);
    }

    void search_bar_platform::update_automation_id(std::string_view value)
    {
        const std::string id(value);
        NSString* const raw = [NSString stringWithUTF8String:id.c_str()];
        as_search_bar(native).accessibilityIdentifier = raw != nil ? raw : @"";
    }

    std::unique_ptr<search_bar_platform> search_bar_handler::create_platform_view()
    {
        auto platform = std::make_unique<search_bar_platform>();
        // CreatePlatformView: new MauiSearchBar() { BarStyle = UIBarStyle.Default }.
        UISearchBar* const bar = [[UISearchBar alloc] initWithFrame:CGRectZero];
        bar.barStyle = UIBarStyleDefault;
        platform->native = (__bridge_retained void*)bar; // the void* slot owns one reference
        return platform;
    }

    void search_bar_handler::on_connect_handler(search_bar_platform& platform)
    {
        UISearchBar* const bar = as_search_bar(platform.native);
        MauiIosSearchBarProxy* const proxy = [[MauiIosSearchBarProxy alloc] init];
        proxy.handler = this;
        proxy.previousText = bar.text != nil ? bar.text : @"";
        bar.delegate = proxy; // weak, so the proxy is retained via an associated object
        objc_setAssociatedObject(bar, &k_proxy_key, proxy, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void search_bar_handler::on_disconnect_handler(search_bar_platform& platform)
    {
        UISearchBar* const bar = as_search_bar(platform.native);
        bar.delegate = nil;
        objc_setAssociatedObject(bar, &k_proxy_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    void search_bar_handler::map_text(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        const std::string text(view.text());
        NSString* const value = [NSString stringWithUTF8String:text.c_str()];
        UISearchBar* const bar = as_search_bar(platform->native);
        bar.text = value; // SearchBarExtensions.UpdateText
        // MapText → MapFormatting: character spacing + alignment + max length.
        map_character_spacing(handler, view);
        query_editor(platform->native).textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        apply_max_length(bar, view);
        refresh_cancel_button(bar, view);
        if (auto* const proxy = (MauiIosSearchBarProxy*)objc_getAssociatedObject(bar, &k_proxy_key))
        {
            proxy.previousText = bar.text != nil ? bar.text : @"";
        }
    }

    void search_bar_handler::map_placeholder(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_search_placeholder(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_placeholder_color(search_bar_handler& handler, i_search_bar& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            refresh_search_placeholder(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_is_read_only(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // SearchBarExtensions.UpdateIsReadOnly: read-only (or input-transparent) disables interaction.
            platform->is_read_only = view.is_read_only();
            as_search_bar(platform->native).userInteractionEnabled =
                (view.is_read_only() || view.input_transparent()) ? NO : YES;
        }
    }

    void search_bar_handler::map_max_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            apply_max_length(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_text_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            query_editor(platform->native).textColor = to_ui_color(view.text_color());
        }
    }

    void search_bar_handler::map_font(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            query_editor(platform->native).font = to_ui_font(view.font(), static_cast<double>(UIFont.systemFontSize));
        }
    }

    void search_bar_handler::map_character_spacing(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // QueryEditor.UpdateCharacterSpacing: kern the text and the placeholder when present.
        UISearchTextField* const editor = query_editor(platform->native);
        const double spacing = view.character_spacing();
        NSAttributedString* const text_attr = with_character_spacing(editor.attributedText, spacing);
        if (text_attr != nil)
        {
            editor.attributedText = text_attr;
        }
        NSAttributedString* const placeholder_attr = with_character_spacing(editor.attributedPlaceholder, spacing);
        if (placeholder_attr != nil)
        {
            editor.attributedPlaceholder = placeholder_attr;
        }
    }

    void search_bar_handler::map_horizontal_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            query_editor(platform->native).textAlignment = to_ns_text_alignment(view.horizontal_text_alignment());
        }
    }

    void search_bar_handler::map_vertical_text_alignment(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // UpdateVerticalTextAlignment → the QueryEditor's contentVerticalAlignment (a UIControl).
            platform->vertical_alignment = view.vertical_text_alignment();
            query_editor(platform->native).contentVerticalAlignment =
                to_ui_control_content_vertical_alignment(view.vertical_text_alignment());
        }
    }

    void search_bar_handler::map_is_text_prediction_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_text_prediction_enabled = view.is_text_prediction_enabled();
            query_editor(platform->native).autocorrectionType =
                view.is_text_prediction_enabled() ? UITextAutocorrectionTypeYes : UITextAutocorrectionTypeNo;
        }
    }

    void search_bar_handler::map_is_spell_check_enabled(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->is_spell_check_enabled = view.is_spell_check_enabled();
            query_editor(platform->native).spellCheckingType =
                view.is_spell_check_enabled() ? UITextSpellCheckingTypeYes : UITextSpellCheckingTypeNo;
        }
    }

    void search_bar_handler::map_keyboard(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->keyboard = view.keyboard();
        // SearchBarExtensions.UpdateKeyboard: ApplyKeyboard onto the search field (UISearchTextField
        // conforms to UITextInputTraits), then (for non-custom keyboards) re-apply prediction/spellcheck,
        // then ReloadInputViews so a live keyboard re-styles.
        UISearchBar* const bar = as_search_bar(platform->native);
        maui::platform::ios::apply_keyboard(query_editor(platform->native), view.keyboard());
        if (!maui::platform::ios::is_custom_keyboard(view.keyboard()))
        {
            map_is_text_prediction_enabled(handler, view);
            map_is_spell_check_enabled(handler, view);
        }
        [bar reloadInputViews];
    }

    void search_bar_handler::map_cursor_position(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->cursor_position = view.cursor_position();
        // Clamped-range write onto the QueryEditor (the entry recipe's UpdateCursorSelection, collapsed:
        // without an editing session there is no selectedTextRange and the mirror records the intent).
        UISearchTextField* const editor = query_editor(platform->native);
        if (editor.selectedTextRange == nil)
        {
            return;
        }
        const int text_length = static_cast<int>(editor.text != nil ? editor.text.length : 0);
        const int start = view.cursor_position() < text_length ? view.cursor_position() : text_length;
        const int span_max = text_length - start;
        const int span = view.selection_length() < span_max ? view.selection_length() : span_max;
        UITextPosition* const from = [editor positionFromPosition:editor.beginningOfDocument offset:start];
        UITextPosition* const to = [editor positionFromPosition:editor.beginningOfDocument offset:start + span];
        if (from != nil && to != nil)
        {
            editor.selectedTextRange = [editor textRangeFromPosition:from toPosition:to];
        }
    }

    void search_bar_handler::map_selection_length(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->selection_length = view.selection_length();
        map_cursor_position(handler, view); // both re-establish the whole range from the pair
    }

    void search_bar_handler::map_cancel_button_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            platform->cancel_button_color = view.cancel_button_color();
            refresh_cancel_button(as_search_bar(platform->native), view);
        }
    }

    void search_bar_handler::map_search_icon_color(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        // SearchBarExtensions.UpdateSearchIcon: tint the loupe (the QueryEditor's leftView image).
        platform->search_icon_color = view.search_icon_color();
        const maui::graphics::color color = view.search_icon_color();
        const bool explicit_color = color != maui::graphics::color{};
        UIView* const left = query_editor(platform->native).leftView;
        if (explicit_color && [left isKindOfClass:[UIImageView class]])
        {
            auto* const icon = (UIImageView*)left;
            icon.image = [icon.image imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
            icon.tintColor = to_ui_color(color);
        }
    }

    void search_bar_handler::map_return_type(search_bar_handler& handler, i_search_bar& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform != nullptr)
        {
            // SearchBarExtensions.UpdateReturnType → the QueryEditor's returnKeyType (REAL on iOS).
            platform->bar_return_type = view.return_type();
            query_editor(platform->native).returnKeyType = to_ui_return_key_type(view.return_type());
        }
    }

    maui::graphics::size search_bar_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // SearchBarHandler.iOS.GetDesiredSize: infinite constraints collapse to the bar's fitted size.
        const CGFloat width = std::isfinite(width_constraint) ? static_cast<CGFloat>(width_constraint) : CGFLOAT_MAX;
        const CGFloat height = std::isfinite(height_constraint) ? static_cast<CGFloat>(height_constraint) : CGFLOAT_MAX;
        const CGSize fitting = [as_search_bar(platform->native) sizeThatFits:CGSizeMake(width, height)];
        return {fitting.width, fitting.height};
    }

    void search_bar_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_search_bar(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::core
