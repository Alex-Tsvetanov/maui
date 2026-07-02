// picker_handler â€” Windows (WinUI 3) platform partial: a REAL Microsoft.UI.Xaml.Controls.ComboBox.
// The windows twin of src/platform/apple/picker_handler.mm (NSPopUpButton) / the android JNI EditText
// partial, and the real-native sibling of the headless mirror partial
// (src/platform/headless/picker_handler.cpp). The items/selection maps keep the headless UpdatePicker
// mirror byte-for-byte AND drive the real control; a native row pick flows back through
// i_picker::set_selected_index from the ComboBox's SelectionChanged, and the drop-down open/close
// events write is_open back (the C# OnMauiComboBoxDropDownOpened/Closed pair).
//
// Ported DIRECTLY from PickerHandler.Windows.cs + Platform/Windows/{PickerExtensions.cs (UpdateTitle /
// UpdateBackground / UpdateTextColor / UpdateSelectedIndex / UpdateCharacterSpacing / UpdateFont /
// UpdateHorizontalTextAlignment / UpdateVerticalTextAlignment / UpdateIsOpen), ControlExtensions.cs
// (UpdateFont / UpdateIsEnabled), AlignmentExtensions.cs (ToPlatformHorizontalAlignment /
// ToPlatformVerticalAlignment), ViewExtensions.cs, FontExtensions.cs, CharacterSpacingExtensions.cs}
// + Fonts/FontManager.Windows.cs.
//
// DOCUMENTED DEVIATIONS (each an infrastructure gap of this first cut, not a behavior guess):
//   - Items: C#'s Reload assigns ItemsSource = new ItemDelegateList<string>(VirtualView) (a live
//     virtualizing wrapper over the IItemDelegate face). The port has no INotifyCollectionChanged /
//     ItemsSource seam, so map_items REBUILDS ComboBox.Items() from the same i_item_delegate face as
//     boxed hstrings â€” the same reset-selection side effect C# guards with UpdatingItemSource is
//     guarded with the platform struct's updating_item_source flag, and the SetUpdatingItemSource(false)
//     tail (re-push the in-range SelectedIndex) is replayed verbatim.
//   - Title: C#'s PickerExtensions.UpdateTitle binds Header/HeaderTemplate to the app-resource
//     "ComboBoxHeader" DataTemplate (Application.Current.Resources). No Application resources exist on
//     this backend, so the title rides ComboBox.PlaceholderText (the visible-when-unselected slot) and
//     the title COLOR rides PlaceholderForeground â€” deferred: the Header/HeaderTemplate composition.
//     MapTitle's Semantics re-map (handler.UpdateValue(nameof(IView.Semantics))) is deferred with the
//     update_value seam.
//   - Text color / background land on the DIRECT dependency properties (Foreground / Background). C#
//     writes the themed resource keys (ComboBoxForeground* / ComboBoxBackground* +
//     RefreshThemeResources) so hover/pressed/disabled states track the value â€” deferred: the
//     resource-dictionary seam (the button partial's identical cut). The C# null branches map to
//     ClearValue, discriminated through BindableObject.IsSet where the port's color value type has no
//     null.
//   - FontManager's registrar lookups are skipped (no font registrar in the port yet): a named family
//     goes straight to FontFamily(name); the default size is the constant 14
//     (ControlContentThemeFontSize). Byte-for-byte the button/label partials' map_font.
//   - UpdateIsOpen's IsLoaded â†’ Loaded-event retry gate is deferred (no Loaded trampoline yet); the
//     IsDropDownOpen push itself is verbatim.
//
// XAML-less degradation (the windows twin of the android VM-less fallback): the windows preset also
// runs the cross-platform suite on the host where NO XAML runtime exists; create_platform_view catches
// the construction failure and keeps native null, while the headless mirrors are ALWAYS maintained and
// on_done stays an invokable C++ callback (the cross-platform suite drives it) â€” so that suite
// observes exactly the headless partial's behavior.

#include "maui/core/picker_handler.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h> // Selector: ComboBox's SelectedIndex/SelectionChanged live on the projected base
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include "maui/core/bindable_object.hpp"
#include "maui/core/dimension.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/core/view_platform_base.hpp"
#include "maui/core/visibility.hpp"
#include "maui/graphics/paint.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"
#include "windows_native.hpp"

namespace
{
    namespace mux = winrt::Microsoft::UI::Xaml;
    namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
    namespace muxm = winrt::Microsoft::UI::Xaml::Media;
    namespace wut = winrt::Windows::UI::Text;
    namespace wnative = maui::platform::win;

    // FontManager.Windows.DefaultFontSize â€” the ControlContentThemeFontSize theme resource (14.0);
    // read as a constant here (no Application.Current on the XAML-less test host).
    constexpr double k_default_font_size = 14.0;

    [[nodiscard]] muxc::ComboBox combo_of(const maui::core::picker_platform& platform)
    {
        return wnative::borrow<muxc::ComboBox>(platform.native);
    }

    // FontExtensions.ToFontStyle: Slant â†’ FontStyle (Italic / Oblique / Normal).
    [[nodiscard]] wut::FontStyle to_font_style(maui::core::font_slant slant)
    {
        switch (slant)
        {
            case maui::core::font_slant::italic:
                return wut::FontStyle::Italic;
            case maui::core::font_slant::oblique:
                return wut::FontStyle::Oblique;
            case maui::core::font_slant::normal:
            default:
                return wut::FontStyle::Normal;
        }
    }

    // FontExtensions.ToFontWeight â€” the port's font_weight enum values ARE the numeric OpenType
    // weights C#'s switch resolves to, so the numeric FontWeight constructor is the whole mapping.
    [[nodiscard]] wut::FontWeight to_font_weight(maui::core::font_weight weight)
    {
        return wut::FontWeight{static_cast<std::uint16_t>(weight)};
    }

    // AlignmentExtensions.ToPlatformHorizontalAlignment: Center â†’ Center, End â†’ Right,
    // Justify â†’ Stretch, else â†’ Left.
    [[nodiscard]] mux::HorizontalAlignment to_horizontal_alignment(maui::core::text_alignment alignment)
    {
        switch (alignment)
        {
            case maui::core::text_alignment::center:
                return mux::HorizontalAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::HorizontalAlignment::Right;
            case maui::core::text_alignment::justify:
                return mux::HorizontalAlignment::Stretch;
            default:
                return mux::HorizontalAlignment::Left;
        }
    }

    // AlignmentExtensions.ToPlatformVerticalAlignment: Center â†’ Center, End â†’ Bottom, else â†’ Top.
    [[nodiscard]] mux::VerticalAlignment to_vertical_alignment(maui::core::text_alignment alignment)
    {
        switch (alignment)
        {
            case maui::core::text_alignment::center:
                return mux::VerticalAlignment::Center;
            case maui::core::text_alignment::end:
                return mux::VerticalAlignment::Bottom;
            default:
                return mux::VerticalAlignment::Top;
        }
    }
} // namespace

namespace maui::core
{
    namespace
    {
        // The headless UpdatePicker mirror (byte-for-byte src/platform/headless/picker_handler.cpp's
        // update_picker â€” PickerExtensions.UpdatePicker's display-text + item-list + virtual write-back
        // algorithm): the XAML-less cross-platform suite observes exactly these mirrors. The NATIVE
        // half of each map (the ComboBox pushes per PickerHandler.Windows.cs) lives in the map bodies â€”
        // the Windows recipe splits items (Reload â†’ ItemsSource) from selection (UpdateSelectedIndex),
        // unlike the iOS field recipe this mirror descends from.
        void update_picker_mirror(picker_handler& handler, i_picker& view, int selected_index)
        {
            auto* platform = handler.typed_platform_view();
            if (platform == nullptr)
            {
                return;
            }
            platform->text = selected_index != -1 ? view.get_item(selected_index) : std::string{};

            const int count = view.get_count();
            platform->items.clear();
            platform->items.reserve(static_cast<std::size_t>(count));
            for (int at = 0; at < count; ++at)
            {
                platform->items.push_back(view.get_item(at));
            }

            if (count == 0)
            {
                return;
            }
            platform->selected_index = selected_index;
            view.set_selected_index(selected_index); // picker.SelectedIndex = selectedIndex (FromHandler)
        }

        // PickerExtensions.UpdateTextColor's brush push (also re-run by nothing here â€” kept as a helper
        // for symmetry with the date/time partials). null â†’ RemoveKeys + ClearValue(Foreground);
        // value â†’ SetValueForAllKey + Foreground = brush. The port pushes the direct Foreground
        // (deferred: the ComboBoxForeground* resource keys â€” header), with the null branch
        // discriminated through BindableObject.IsSet (the port's color has no null).
        void push_text_color(const picker_platform& platform, i_picker& view)
        {
            auto combo = combo_of(platform);
            if (combo == nullptr)
            {
                return;
            }
            const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
            const bool color_is_set = bindable != nullptr && bindable->is_property_set("text_color");
            if (color_is_set)
            {
                combo.Foreground(wnative::to_brush(view.text_color()));
            }
            else
            {
                combo.ClearValue(muxc::Control::ForegroundProperty());
            }
        }
    } // namespace

    // Releases the one strong ref pinning the ComboBox (the wnative shape of the pimpl-owned-native
    // doctrine; the apple twin CFReleases its NSPopUpButton here). Event tokens are revoked in
    // on_disconnect_handler; the dtor only drops the ref.
    picker_platform::~picker_platform()
    {
        wnative::release(native);
    }

    // The generic-IView pushes (the shared view_mapper calls these through view_platform_base). Each
    // calls the base body FIRST â€” the headless mirrors must stay live for the XAML-less cross-platform
    // suite (header note) â€” then pushes to the real ComboBox when one exists.

    void picker_platform::update_visibility(maui::core::visibility value)
    {
        view_platform_base::update_visibility(value);
        // ViewExtensions.UpdateVisibility (Windows): Hidden rides Opacity 0, Collapsed collapses.
        wnative::apply_visibility(native, value, alpha);
    }

    void picker_platform::update_opacity(double value)
    {
        view_platform_base::update_opacity(value);
        // ViewExtensions.UpdateOpacity: the Hidden state pins opacity 0 (apply_visibility restores it).
        if (!hidden)
        {
            wnative::apply_opacity(native, value);
        }
    }

    void picker_platform::update_is_enabled(bool value)
    {
        view_platform_base::update_is_enabled(value);
        // ViewExtensions.UpdateIsEnabled â†’ ControlExtensions.UpdateIsEnabled: Control.IsEnabled.
        if (auto combo = combo_of(*this))
        {
            combo.IsEnabled(value);
        }
    }

    void picker_platform::update_automation_id(std::string_view value)
    {
        view_platform_base::update_automation_id(value);
        // ViewExtensions.UpdateAutomationId: AutomationProperties.SetAutomationId.
        wnative::apply_automation_id(native, value);
    }

    void picker_platform::update_background(const maui::graphics::paint* value)
    {
        view_platform_base::update_background(value);
        auto combo = combo_of(*this);
        if (combo == nullptr)
        {
            return;
        }
        // PickerHandler.Windows.MapBackground â†’ PickerExtensions.UpdateBackground: a null brush removes
        // the ComboBoxBackground* resource keys (â†’ theme default), a value sets them all (+
        // RefreshThemeResources). The port pushes the DIRECT Background property (deferred: the
        // per-state resource keys â€” header) and ClearValue for the null branch.
        if (value == nullptr)
        {
            combo.ClearValue(muxc::Control::BackgroundProperty());
            return;
        }
        if (const auto* solid = dynamic_cast<const maui::graphics::solid_paint*>(value))
        {
            combo.Background(wnative::to_brush(solid->color()));
            return;
        }
        // deferred: gradient / image-source paints (Paint.ToPlatform) â€” the base mirror above keeps the
        // borrow observable.
    }

    std::unique_ptr<picker_platform> picker_handler::create_platform_view()
    {
        auto platform = std::make_unique<picker_platform>();
        try
        {
            // PickerHandler.Windows.CreatePlatformView: new ComboBox().
            const muxc::ComboBox combo;
            platform->native = wnative::store(combo); // released in ~picker_platform
        }
        catch (const winrt::hresult_error&)
        {
            platform->native = nullptr; // XAML-less degradation (header note)
        }
        return platform;
    }

    void picker_handler::on_connect_handler(picker_platform& platform)
    {
        // on_done stays wired even XAML-less so the cross-platform suite can drive a row commit
        // (FinishSelectItem: an unset (-1) row with items present commits row 0) â€” the headless twin's
        // body, plus the native SelectedIndex push when a control exists.
        platform.on_done = [this](int row) {
            auto* view = virtual_view();
            if (view == nullptr)
            {
                return;
            }
            if (row == -1 && view->get_count() > 0)
            {
                row = 0;
            }
            update_picker_mirror(*this, *view, row);
            auto* typed = typed_platform_view();
            if (typed != nullptr)
            {
                if (auto combo = combo_of(*typed))
                {
                    combo.SelectedIndex(row); // PickerExtensions.UpdateSelectedIndex
                }
            }
        };
        auto combo = combo_of(platform);
        if (combo == nullptr)
        {
            return;
        }
        // ConnectHandler: SelectionChanged += OnControlSelectionChanged; DropDownOpened/Closed +=
        // OnMauiComboBoxDropDownOpened/Closed. The C# handlers are handler INSTANCE methods, so the
        // lambdas capture `this` (+ the platform peer for the guard flag); on_disconnect_handler
        // revokes the tokens before the handler/platform die, so the captures never dangle.
        auto* peer = &platform;
        platform.selection_changed_token =
            combo
                .SelectionChanged([this, peer](const winrt::Windows::Foundation::IInspectable&,
                                               const muxc::SelectionChangedEventArgs&) {
                    // OnControlSelectionChanged: VirtualView.SelectedIndex = PlatformView.SelectedIndex
                    // (suppressed while UpdatingItemSource â€” the ItemsSource rebuild resets it to -1);
                    // then PlatformView.MinWidth = 0 (release the drop-down-opened width pin).
                    auto sender = combo_of(*peer);
                    if (sender == nullptr)
                    {
                        return;
                    }
                    auto* view = virtual_view();
                    if (view != nullptr && !peer->updating_item_source)
                    {
                        view->set_selected_index(sender.SelectedIndex());
                    }
                    sender.MinWidth(0.0);
                })
                .value;
        platform.drop_down_opened_token =
            combo
                .DropDownOpened([this, peer](const winrt::Windows::Foundation::IInspectable&,
                                             const winrt::Windows::Foundation::IInspectable&) {
                    // OnMauiComboBoxDropDownOpened: pin MinWidth to the current width (so the open
                    // drop-down does not shrink the box), then VirtualView.IsOpen = true.
                    auto sender = combo_of(*peer);
                    if (sender == nullptr)
                    {
                        return;
                    }
                    sender.MinWidth(sender.ActualWidth());
                    if (auto* view = virtual_view())
                    {
                        view->set_is_open(true);
                    }
                })
                .value;
        platform.drop_down_closed_token =
            combo
                .DropDownClosed([this, peer](const winrt::Windows::Foundation::IInspectable&,
                                             const winrt::Windows::Foundation::IInspectable&) {
                    // OnMauiComboBoxDropDownClosed: reset the MinWidth pin so the ComboBox can resize
                    // with its parent again, then VirtualView.IsOpen = false.
                    auto sender = combo_of(*peer);
                    if (sender != nullptr && sender.MinWidth() > 0)
                    {
                        sender.MinWidth(0.0);
                    }
                    if (auto* view = virtual_view())
                    {
                        view->set_is_open(false);
                    }
                })
                .value;
    }

    void picker_handler::on_disconnect_handler(picker_platform& platform)
    {
        // DisconnectHandler: SelectionChanged/DropDownOpened/DropDownClosed -= â€¦. The C++ callback is
        // cleared like the headless twin.
        platform.on_done = nullptr;
        if (auto combo = combo_of(platform))
        {
            if (platform.selection_changed_token != 0)
            {
                combo.SelectionChanged(winrt::event_token{platform.selection_changed_token});
            }
            if (platform.drop_down_opened_token != 0)
            {
                combo.DropDownOpened(winrt::event_token{platform.drop_down_opened_token});
            }
            if (platform.drop_down_closed_token != 0)
            {
                combo.DropDownClosed(winrt::event_token{platform.drop_down_closed_token});
            }
        }
        platform.selection_changed_token = 0;
        platform.drop_down_opened_token = 0;
        platform.drop_down_closed_token = 0;
    }

    void picker_handler::map_items(picker_handler& handler, i_picker& view)
    {
        // Reload â†’ the headless UpdatePicker mirror first (the XAML-less suite's observable)â€¦
        update_picker_mirror(handler, view, view.selected_index());
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        auto combo = combo_of(*platform);
        if (combo == nullptr)
        {
            return;
        }
        // â€¦then PickerHandler.Windows.Reload: SetUpdatingItemSource(true); ItemsSource = new
        // ItemDelegateList<string>(VirtualView); SetUpdatingItemSource(false). The port rebuilds
        // Items() from the same i_item_delegate face as boxed hstrings (header deviations); the
        // rebuild resets the native SelectedIndex, which the guard keeps away from the virtual view.
        platform->updating_item_source = true;
        auto items = combo.Items();
        items.Clear();
        const int count = view.get_count();
        for (int at = 0; at < count; ++at)
        {
            items.Append(winrt::box_value(wnative::to_hstring_utf8(view.get_item(at))));
        }
        platform->updating_item_source = false;
        // SetUpdatingItemSource(false)'s tail: an in-range selection is re-pushed
        // (UpdateValue(nameof(IPicker.SelectedIndex)) â†’ MapSelectedIndex).
        if (view.selected_index() < view.get_count())
        {
            map_selected_index(handler, view);
        }
    }

    void picker_handler::map_selected_index(picker_handler& handler, i_picker& view)
    {
        update_picker_mirror(handler, view, view.selected_index()); // headless mirror (UpdateSelectedIndex)
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (auto combo = combo_of(*platform))
        {
            // PickerExtensions.UpdateSelectedIndex: nativeComboBox.SelectedIndex = picker.SelectedIndex.
            combo.SelectedIndex(view.selected_index());
        }
    }

    void picker_handler::map_title(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->title = std::string(view.title()); // headless mirror (UpdatePickerTitle)
        if (auto combo = combo_of(*platform))
        {
            // PickerExtensions.UpdateTitle binds Header/HeaderTemplate to the app-resource
            // "ComboBoxHeader" DataTemplate; the port lands the title on PlaceholderText (header
            // deviations). deferred: the Header/HeaderTemplate composition + MapTitle's Semantics
            // re-map (handler.UpdateValue(nameof(IView.Semantics))).
            combo.PlaceholderText(wnative::to_hstring_utf8(view.title()));
        }
    }

    void picker_handler::map_title_color(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->title_color = view.title_color(); // headless mirror
        auto combo = combo_of(*platform);
        if (combo == nullptr)
        {
            return;
        }
        // C#'s MapTitleColor re-runs UpdateTitle (the title color rides the ComboBoxHeader template's
        // TitleColor binding). With the title on PlaceholderText (header deviations), the color lands
        // on PlaceholderForeground â€” the same visible slot. The unset case restores the theme default
        // via ClearValue, discriminated through BindableObject.IsSet (the port's color has no null).
        const auto* bindable = dynamic_cast<const maui::core::bindable_object*>(&view);
        const bool color_is_set = bindable != nullptr && bindable->is_property_set("title_color");
        if (color_is_set)
        {
            combo.PlaceholderForeground(wnative::to_brush(view.title_color()));
        }
        else
        {
            combo.ClearValue(muxc::ComboBox::PlaceholderForegroundProperty());
        }
    }

    void picker_handler::map_text_color(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_color = view.text_color(); // headless mirror
        push_text_color(*platform, view);         // PickerExtensions.UpdateTextColor
    }

    void picker_handler::map_font(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->text_font = view.font(); // headless mirror
        auto combo = combo_of(*platform);
        if (combo == nullptr)
        {
            return;
        }
        // PickerExtensions.UpdateFont â†’ ControlExtensions.UpdateFont: FontSize + FontFamily +
        // FontStyle + FontWeight + IsTextScaleFactorEnabled (FontManager.GetFontSize/GetFontFamily â€”
        // registrar skipped, header).
        const font value = view.font();
        const double size = value.size();
        combo.FontSize((size > 0 && !std::isnan(size)) ? size : k_default_font_size);
        if (!value.family().empty())
        {
            combo.FontFamily(muxm::FontFamily{wnative::to_hstring_utf8(value.family())});
        }
        else
        {
            combo.ClearValue(muxc::Control::FontFamilyProperty()); // C# null Family â†’ the default family
        }
        combo.FontStyle(to_font_style(value.slant()));
        combo.FontWeight(to_font_weight(value.weight()));
        combo.IsTextScaleFactorEnabled(value.auto_scaling_enabled());
    }

    void picker_handler::map_character_spacing(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->character_spacing = view.character_spacing(); // headless mirror
        if (auto combo = combo_of(*platform))
        {
            // PickerExtensions.UpdateCharacterSpacing: CharacterSpacing = value.ToEm().
            combo.CharacterSpacing(wnative::to_em(view.character_spacing()));
        }
    }

    void picker_handler::map_horizontal_text_alignment(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->horizontal_alignment = view.horizontal_text_alignment(); // headless mirror
        if (auto combo = combo_of(*platform))
        {
            // PickerExtensions.UpdateHorizontalTextAlignment: HorizontalContentAlignment =
            // alignment.ToPlatformHorizontalAlignment().
            combo.HorizontalContentAlignment(to_horizontal_alignment(view.horizontal_text_alignment()));
        }
    }

    void picker_handler::map_vertical_text_alignment(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        platform->vertical_alignment = view.vertical_text_alignment(); // headless mirror
        if (auto combo = combo_of(*platform))
        {
            // PickerExtensions.UpdateVerticalTextAlignment: VerticalContentAlignment =
            // alignment.ToPlatformVerticalAlignment().
            combo.VerticalContentAlignment(to_vertical_alignment(view.vertical_text_alignment()));
        }
    }

    void picker_handler::map_is_open(picker_handler& handler, i_picker& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr)
        {
            return;
        }
        if (auto combo = combo_of(*platform))
        {
            // PickerExtensions.UpdateIsOpen: IsDropDownOpen = picker.IsOpen. deferred: the IsLoaded â†’
            // Loaded-event retry gate (the push on an unloaded ComboBox is queued by XAML itself here).
            combo.IsDropDownOpen(view.is_open());
        }
    }

    maui::graphics::size picker_handler::get_desired_size(double width_constraint, double height_constraint) const
    {
        const auto* platform = typed_platform_view();
        if (platform == nullptr)
        {
            return {0, 0};
        }
        if (platform->native == nullptr)
        {
            // XAML-less degradation: the headless partial's placeholder metric (a single-line field
            // ~150pt wide, clamped to a finite width constraint, one line tall), so the backend-
            // agnostic size-request suites see consistent numbers.
            double width = 150.0;
            if (width_constraint > 0 && width_constraint < width)
            {
                width = width_constraint;
            }
            return {width, 22.0};
        }
        // ViewHandlerExtensions.Windows.cs GetDesiredSizeFromHandler: FrameworkElement.Measure +
        // DesiredSize, with the AdjustForExplicitSize clamp fed from the virtual view's explicit
        // width()/height() (see measure_native).
        const double explicit_width = virtual_view() != nullptr ? virtual_view()->width() : dimension::unset;
        const double explicit_height = virtual_view() != nullptr ? virtual_view()->height() : dimension::unset;
        return wnative::measure_native(platform->native, width_constraint, height_constraint, explicit_width,
                                       explicit_height);
    }

    void picker_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return; // XAML-less: no native layout to apply
        }
        // The shared Canvas recipe: Canvas.SetLeft/SetTop + explicit Width/Height pin the ComboBox to
        // the frame (ViewHandlerExtensions.Windows.cs PlatformArrangeHandler on the Canvas layout model).
        wnative::arrange_native(platform->native, frame);
    }
} // namespace maui::core
