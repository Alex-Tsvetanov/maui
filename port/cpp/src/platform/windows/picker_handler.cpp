// picker_handler — WinUI 3 platform recipe: a real Microsoft.UI.Xaml.Controls.ComboBox, the same native
// type PickerHandler.Windows.cs creates. Ported from PickerHandler.Windows.cs + PickerExtensions.cs.
//
// DOCUMENTED SIMPLIFICATIONS against C#, each narrowed on purpose rather than left vague:
//
//  1. Title/TitleColor (UpdateTitle). C# sets `ComboBox.Header = picker` (the IPicker itself) plus a
//     `HeaderTemplate` resolved from `Application.Current.Resources["ComboBoxHeader"]` — a DataTemplate
//     (MauiComboBoxStyle.xaml) that data-binds a TextBlock's Text/Foreground to Title/TitleColor. This
//     backend has no Application-resource-dictionary / DataTemplate-binding infrastructure (the same gap
//     button_handler.cpp's DefaultMauiButtonContent note and label_handler.cpp's NeedsContainer note
//     document for their own controls). This slice reproduces the OBSERVABLE result instead: build a
//     plain TextBlock with that Text/Foreground already resolved and set it directly as Header (Header's
//     type is `object`, and a UIElement assigned there is rendered as-is with no template needed) —
//     identical at rest, no live re-binding if Title/TitleColor changed through some OTHER path than
//     these two mappers (there is none in this contract).
//  2. IsTextScaleFactorEnabled. ControlExtensions.UpdateFont assigns this on every `Control`, but per
//     button_handler.cpp's map_font note, this WinRT projection only exposes the property on
//     `TextBlock` — a ComboBox has no single content TextBlock this handler owns (unlike Button's
//     DefaultMauiButtonContent slot), so this push is skipped, matching entry_handler.cpp's identical
//     skip for the same reason.
//  3. Background (MapBackground → PickerExtensions.UpdateBackground) is a Windows-only theme-resource
//     remap in C#, but picker_handler.cpp's cross-platform mapper() ALREADY documents "Android/Windows-
//     only Background... are not replicated" — background is not in this handler's mapper table at all
//     (confirmed against the headless twin, which also has no map_background). The generic IView
//     Background still reaches the ComboBox through the five-override block below (a direct brush push,
//     not the per-visual-state resource keys) — see button_handler.cpp's map_stroke_color for why THAT
//     distinction matters for state-restyled brushes; Background here is unstated in the C# picker
//     mapper, so the plain generic push is not a narrowing of anything this handler is asked to do.

#include "maui/core/picker_handler.hpp"

// ComboBox inherits SelectedIndex/SelectionChanged from Selector, which lives in
// Controls.Primitives - the C++/WinRT include rule again: without the FULL header those
// members are only forward-declared and every use is C3779, an error naming neither the
// header nor the concept. Same class of miss as the Collections header last batch.
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "maui/core/bindable_object.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "winui_interop.hpp"
#include "winui_visual_ops.hpp"

namespace
{
    // Named `winui`, NOT `xaml`: the port already has a maui::xaml namespace (the XAML loader), and
    // inside namespace maui::* that name WINS over a file-scope alias - an `xaml::Application` here
    // would resolve to maui::xaml and fail with "'Start': is not a member of 'maui::xaml'".
    namespace winui = winrt::Microsoft::UI::Xaml;
    using combo_box = winui::Controls::ComboBox;
    using text_block = winui::Controls::TextBlock;

    combo_box as_combo(void* native)
    {
        return maui::platform::windows::ref<winui::UIElement>(native).as<combo_box>();
    }

    // MAUI's Windows FontManager defaults — see label_handler.cpp's identical constants; the port has no
    // i_font_manager for this backend yet, so these stand in for fontManager.GetFontSize/GetFontFamily's
    // resolved defaults (ControlContentThemeFontSize / the Fluent theme's default control font).
    constexpr double k_default_font_size = 14.0;
    constexpr std::wstring_view k_default_font_family = L"Segoe UI Variable Text";

    // PickerExtensions' ComboBoxForeground resource-key set (see button_handler.cpp's identical
    // k_text_color_keys pattern for WHY a local Foreground alone is insufficient: the control template
    // binds per-visual-state brushes to these theme resources, so an unset override is dropped again the
    // moment the pointer enters/leaves/focuses the combo).
    constexpr std::array<std::wstring_view, 5> k_text_color_keys{
        L"ComboBoxForeground", L"ComboBoxForegroundPointerOver", L"ComboBoxForegroundDisabled",
        L"ComboBoxForegroundFocused", L"ComboBoxForegroundFocusedPressed"};

    void set_resources(const combo_box& combo, std::span<const std::wstring_view> keys,
                       const winui::Media::Brush& brush)
    {
        for (const auto& key : keys)
        {
            combo.Resources().Insert(winrt::box_value(winrt::hstring{key}), brush);
        }
    }

    void remove_resources(const combo_box& combo, std::span<const std::wstring_view> keys)
    {
        for (const auto& key : keys)
        {
            combo.Resources().Remove(winrt::box_value(winrt::hstring{key}));
        }
    }

    // FrameworkElementExtensions.RefreshThemeResources: flip RequestedTheme away and back so the control
    // template re-resolves the resources just overridden.
    void refresh_theme_resources(const winui::FrameworkElement& element)
    {
        const auto previous = element.RequestedTheme();
        element.RequestedTheme(element.ActualTheme() == winui::ElementTheme::Dark ? winui::ElementTheme::Light
                                                                                  : winui::ElementTheme::Dark);
        element.RequestedTheme(previous);
    }

    // "Was this property explicitly set?" — see the twin in button_handler.cpp/label_handler.cpp for why
    // this must not be a value comparison ([[cpp-unset-color-sentinel-collision]]).
    bool is_set(const maui::core::i_view& view, std::string_view property)
    {
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        return bindable != nullptr && bindable->is_property_set(property);
    }

    // AlignmentExtensions.ToPlatformHorizontalAlignment / ToPlatformVerticalAlignment — the Picker-
    // specific alignment mapping (HorizontalContentAlignment/VerticalContentAlignment on the Control
    // itself), distinct from Label's TextBlock.TextAlignment mapping in label_handler.cpp: Justify has no
    // TextAlignment analog there, but here it maps to Stretch.
    winui::HorizontalAlignment to_platform_horizontal(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::HorizontalAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::HorizontalAlignment::Right;
            case maui::core::text_alignment::justify:
                return winui::HorizontalAlignment::Stretch;
            case maui::core::text_alignment::start:
            default:
                return winui::HorizontalAlignment::Left;
        }
    }

    winui::VerticalAlignment to_platform_vertical(maui::core::text_alignment value)
    {
        switch (value)
        {
            case maui::core::text_alignment::center:
                return winui::VerticalAlignment::Center;
            case maui::core::text_alignment::end:
                return winui::VerticalAlignment::Bottom;
            case maui::core::text_alignment::start:
            default:
                return winui::VerticalAlignment::Top;
        }
    }

    winrt::Windows::UI::Text::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return winrt::Windows::UI::Text::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    winrt::Windows::UI::Text::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return winrt::Windows::UI::Text::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return winrt::Windows::UI::Text::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return winrt::Windows::UI::Text::FontStyle::Normal;
        }
    }

    // UpdateTitle, shared by MapTitle and MapTitleColor (C# calls the same UpdateTitle from both). See
    // the file-top deviation note 1: a plain TextBlock built with the resolved Text/Foreground already
    // baked in, set directly as Header, standing in for Header=picker + the ComboBoxHeader DataTemplate.
    void apply_title(const combo_box& combo, const maui::core::i_view& view,
                     const maui::core::picker_platform& platform)
    {
        if (platform.title.empty())
        {
            combo.Header(nullptr);
            return;
        }
        text_block header;
        header.Text(maui::platform::windows::to_hstring(platform.title));
        // Unset TitleColor: the DataTemplate's ColorConverter falls back to DefaultTextForegroundThemeBrush
        // — leaving Foreground untouched on a freshly-built TextBlock resolves the same themed default.
        if (is_set(view, "title_color"))
        {
            header.Foreground(
                winui::Media::SolidColorBrush{maui::platform::windows::to_ui_color(platform.title_color)});
        }
        combo.Header(header);
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // Unhook everything on_connect_handler registered. Called from on_disconnect_handler AND from
        // ~picker_platform, same rationale as button_handler.cpp's detach_native_events: the lambdas
        // capture the handler, so an undisconnected teardown must not leave them subscribed.
        void detach_native_events(picker_platform& platform)
        {
            if (platform.native != nullptr)
            {
                const combo_box combo = as_combo(platform.native);
                combo.SelectionChanged(winrt::event_token{platform.selection_changed_token});
                combo.DropDownOpened(winrt::event_token{platform.drop_down_opened_token});
                combo.DropDownClosed(winrt::event_token{platform.drop_down_closed_token});
            }
            platform.selection_changed_token = 0;
            platform.drop_down_opened_token = 0;
            platform.drop_down_closed_token = 0;
        }

        // Reload(handler): rebuild the native item list, guarded by UpdatingItemSource so the
        // SelectionChanged the rebuild fires is not written back into the virtual view, then — exactly
        // like SetUpdatingItemSource(false) — re-push SelectedIndex via the normal mapper key so it lands
        // AFTER the rebuild (a stale index rejected by the old, shorter list is otherwise silently lost).
        void reload(picker_handler& handler, i_picker& view)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr || platform->native == nullptr)
            {
                return;
            }
            platform->updating_item_source = true;
            const combo_box combo = as_combo(platform->native);
            combo.Items().Clear();
            platform->items.clear();
            const int count = view.get_count();
            platform->items.reserve(static_cast<std::size_t>(count));
            for (int at = 0; at < count; ++at)
            {
                std::string item = view.get_item(at);
                combo.Items().Append(winrt::box_value(maui::platform::windows::to_hstring(item)));
                platform->items.push_back(std::move(item));
            }
            platform->updating_item_source = false;
            // SetUpdatingItemSource(false)'s guard: `VirtualView.SelectedIndex < VirtualView.GetCount()`.
            if (view.selected_index() < view.get_count())
            {
                handler.update_value("selected_index");
            }
        }
    } // namespace

    picker_platform::~picker_platform()
    {
        detach_native_events(*this);
        if (native != nullptr)
        {
            maui::platform::windows::drop<winui::UIElement>(native);
        }
    }

    std::unique_ptr<picker_platform> picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<picker_platform>();
        combo_box combo;
        platform->native = maui::platform::windows::take<winui::UIElement>(combo);
        return platform;
    }

    void picker_handler::on_connect_handler(picker_platform& platform)
    {
        if (platform.native == nullptr)
        {
            return;
        }
        auto* self = this;
        const combo_box combo = as_combo(platform.native);

        // OnControlSelectionChanged: write the native selection back UNLESS a reload is in flight, then
        // unconditionally release the MinWidth pin DropDownOpened below installs.
        platform.selection_changed_token =
            combo
                .SelectionChanged([self](const winrt::Windows::Foundation::IInspectable& sender,
                                         const winui::Controls::SelectionChangedEventArgs&) {
                    const auto native = sender.try_as<combo_box>();
                    if (!native)
                    {
                        return;
                    }
                    auto* view = self->virtual_view();
                    auto* platform_view = self->typed_platform_view();
                    if (view != nullptr && platform_view != nullptr && !platform_view->updating_item_source)
                    {
                        view->set_selected_index(native.SelectedIndex());
                    }
                    native.MinWidth(0);
                })
                .value;

        // OnMauiComboBoxDropDownOpened: pin the width so the dropdown doesn't resize the closed combo,
        // then tell the virtual view it's open (IsOpen flows both ways, like SelectedIndex).
        platform.drop_down_opened_token =
            combo
                .DropDownOpened([self](const winrt::Windows::Foundation::IInspectable& sender,
                                       const winrt::Windows::Foundation::IInspectable&) {
                    const auto native = sender.try_as<combo_box>();
                    if (!native)
                    {
                        return;
                    }
                    native.MinWidth(native.ActualWidth());
                    if (auto* view = self->virtual_view())
                    {
                        view->set_is_open(true);
                    }
                })
                .value;

        // OnMauiComboBoxDropDownClosed: release the width pin, then tell the virtual view it's closed.
        platform.drop_down_closed_token =
            combo
                .DropDownClosed([self](const winrt::Windows::Foundation::IInspectable& sender,
                                       const winrt::Windows::Foundation::IInspectable&) {
                    auto* view = self->virtual_view();
                    if (view == nullptr)
                    {
                        return;
                    }
                    if (const auto native = sender.try_as<combo_box>(); native && native.MinWidth() > 0)
                    {
                        native.MinWidth(0);
                    }
                    view->set_is_open(false);
                })
                .value;
    }

    void picker_handler::on_disconnect_handler(picker_platform& platform)
    {
        detach_native_events(platform);
    }

    void picker_handler::map_items(picker_handler& handler, i_picker& view)
    {
        reload(handler, view);
    }

    void picker_handler::map_selected_index(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->selected_index = view.selected_index();
        platform->text = (platform->selected_index >= 0 && platform->selected_index < view.get_count())
                             ? view.get_item(platform->selected_index)
                             : std::string{};
        as_combo(platform->native).SelectedIndex(platform->selected_index);
    }

    void picker_handler::map_title(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->title = std::string(view.title());
        apply_title(as_combo(platform->native), view, *platform);
        // MapTitle -> handler.UpdateValue(nameof(IView.Semantics)): the shared view_mapper's "semantics"
        // key re-runs update_semantics on the base mirror (this backend has no windows-specific a11y push
        // yet), matching the oracle's call even though there is nothing further to observe here.
        handler.update_value("semantics");
    }

    void picker_handler::map_title_color(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->title_color = view.title_color();
        apply_title(as_combo(platform->native), view, *platform);
    }

    void picker_handler::map_text_color(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color();
        const combo_box combo = as_combo(platform->native);
        // An UNSET TextColor must leave the theme brush alone rather than paint transparent black — see
        // [[cpp-unset-color-sentinel-collision]] and button_handler.cpp's identical map_text_color.
        if (!is_set(view, "text_color"))
        {
            remove_resources(combo, k_text_color_keys);
            combo.ClearValue(winui::Controls::Control::ForegroundProperty());
            refresh_theme_resources(combo);
            return;
        }
        const winui::Media::SolidColorBrush brush{maui::platform::windows::to_ui_color(platform->text_color)};
        set_resources(combo, k_text_color_keys, brush);
        combo.Foreground(brush);
        refresh_theme_resources(combo);
    }

    void picker_handler::map_font(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->text_font = view.font();
        const combo_box combo = as_combo(platform->native);
        const font& f = platform->text_font;
        // ALWAYS assign, never skip — see label_handler.cpp's identical map_font note: C#'s UpdateFont
        // resolves fontManager.GetFontSize/GetFontFamily unconditionally, which fall back to the
        // framework defaults (k_default_font_size/k_default_font_family) when the Font is unset.
        combo.FontSize(f.size() > 0 ? f.size() : k_default_font_size);
        combo.FontFamily(f.family().empty()
                             ? winui::Media::FontFamily{k_default_font_family}
                             : winui::Media::FontFamily{maui::platform::windows::to_hstring(f.family())});
        combo.FontStyle(to_font_style(f.slant()));
        combo.FontWeight(to_font_weight(f.weight()));
        // No IsTextScaleFactorEnabled push — see the file-top deviation note 2.
    }

    void picker_handler::map_character_spacing(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing();
        // CharacterSpacingExtensions.ToEm: pt * 0.0624 * 1000, in 1/1000 em units.
        const auto em = static_cast<std::int32_t>(std::lround(platform->character_spacing * 0.0624 * 1000.0));
        as_combo(platform->native).CharacterSpacing(em);
    }

    void picker_handler::map_horizontal_text_alignment(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment();
        as_combo(platform->native).HorizontalContentAlignment(to_platform_horizontal(platform->horizontal_alignment));
    }

    void picker_handler::map_vertical_text_alignment(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment();
        as_combo(platform->native).VerticalContentAlignment(to_platform_vertical(platform->vertical_alignment));
    }

    void picker_handler::map_is_open(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        const combo_box combo = as_combo(platform->native);
        if (!combo.IsLoaded())
        {
            // UpdateIsOpen's self-unsubscribing Loaded handler: re-run map_is_open once loaded (re-reading
            // the virtual view's CURRENT IsOpen at that time, not a value captured now — the view's IsOpen
            // may flip again before Loaded fires).
            auto token = std::make_shared<winrt::event_token>();
            auto* self = &handler;
            *token = combo.Loaded(
                [combo, token, self](const winrt::Windows::Foundation::IInspectable&, const winui::RoutedEventArgs&) {
                    combo.Loaded(*token);
                    if (auto* live_view = self->virtual_view())
                    {
                        map_is_open(*self, *live_view);
                    }
                });
            return;
        }
        combo.IsDropDownOpen(view.is_open());
    }

    maui::graphics::size picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return {0, 0};
        }
        // GetDesiredSizeFromHandler's first guard: a negative constraint measures to nothing. XAML's
        // Measure THROWS on a negative Size, so this is a crash guard, not a formality.
        if (width_constraint < 0 || height_constraint < 0)
        {
            return {0, 0};
        }
        const combo_box combo = as_combo(platform->native);
        // Clear the pinned size FIRST — see button_handler.cpp's get_desired_size for why (a Canvas child
        // with an explicit Width/Height re-measures to that stale frame instead of the live content).
        const auto auto_size = std::numeric_limits<double>::quiet_NaN();
        combo.Width(auto_size);
        combo.Height(auto_size);
        combo.Measure(winrt::Windows::Foundation::Size{maui::platform::windows::measure_constraint(width_constraint),
                                                       maui::platform::windows::measure_constraint(height_constraint)});
        const auto desired = combo.DesiredSize();
        return {desired.Width, desired.Height};
    }

    void picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        // PlatformArrangeHandler's guard, WIDENED to non-finite — see button_handler.cpp's
        // platform_arrange for why (a NaN reaching XAML's arrange is an unrecoverable stowed exception).
        if (!std::isfinite(frame.x) || !std::isfinite(frame.y) || !std::isfinite(frame.width) ||
            !std::isfinite(frame.height) || frame.width < 0 || frame.height < 0)
        {
            return;
        }
        const combo_box combo = as_combo(platform->native);
        winui::Controls::Canvas::SetLeft(combo, frame.x);
        winui::Controls::Canvas::SetTop(combo, frame.y);
        combo.Width(frame.width);
        combo.Height(frame.height);
    }

    // ---- generic-IView property pushes (view_platform_base overrides) ---------------------------
    // Delegated to the shared winui_visual_ops free functions so all five controls behave identically;
    // see that header for why they are free functions taking the void* slot.
    void picker_platform::update_visibility(maui::core::visibility value)
    {
        maui::platform::windows::apply_visibility(native, value);
    }

    void picker_platform::update_opacity(double value)
    {
        maui::platform::windows::apply_opacity(native, value);
    }

    void picker_platform::update_is_enabled(bool value)
    {
        maui::platform::windows::apply_is_enabled(native, value);
    }

    void picker_platform::update_automation_id(std::string_view value)
    {
        maui::platform::windows::apply_automation_id(native, value);
    }

    void picker_platform::update_background(const maui::graphics::paint* value)
    {
        maui::platform::windows::apply_background(native, value);
    }
} // namespace maui::core
