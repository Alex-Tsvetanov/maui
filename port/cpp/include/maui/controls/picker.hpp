#pragma once
// maui::controls::picker  <=  Microsoft.Maui.Controls.Picker
//
// A single-selection item picker. Ported from src/Controls/src/Core/Picker/Picker.cs — the
// string-ItemsSource subset of the W1-06 unit: `items()` is the developer-facing display list (C#'s
// LockableObservableListWrapper face: direct mutation throws while an ItemsSource drives it), and
// `items_source()` is an observable_collection<std::string> the picker tracks (Add/Remove flow into
// Items incrementally, everything else resets — Picker.CollectionChanged). C#'s object-items +
// ItemDisplayBinding (GetDisplayMember) are OUT OF SCOPE here (markup-era; the display value IS the
// string item), so SelectedItem collapses from `object` to std::optional<std::string> (nullopt = C#'s
// null "no selection").
//
// Selection semantics (the PickerTests.cs oracle):
//   - selected_index coerces through Clamp(-1, Items.Count - 1) (CoerceSelectedIndex) on every set;
//   - a collection change re-derives the index from the selected ITEM when one is set
//     (GetSelectedIndex), so inserting/removing before the selection moves the index with the item
//     (dotnet/maui#29235), then clamps (ClampSelectedIndex, stored at from_handler specificity);
//   - an index change updates selected_item (UpdateSelectedItem) and raises selected_index_changed;
//     a selected_item change re-derives the index (UpdateSelectedIndex, from_handler).
//
// Specificity note (documented deviation, the entry/slider precedent): C#'s explicit
// IPicker.SelectedIndex setter writes at FromHandler; the port's set_selected_index doubles as the
// developer setter, so it stores at manual_value_setter — the INTERNAL clamp/update writes keep
// C#'s from_handler specificity.
//
// IsOpen (Picker.IsOpenProperty, default false, TwoWay) tracks whether the native dialog/wheel is
// visible; a transition raises Opened/Closed (Picker.OnIsOpenPropertyChanged → HandleIsOpenChanged).
// The events fire AFTER the value is stored (the property_changed callback runs post-store), so a
// handler reading is_open() sees the new value. The C# pending-action queue (deferring the raise until
// a handler attaches) is NOT replicated — the port raises eagerly on every transition (the
// PickerTests.cs oracle only asserts the transition→raise).
//
// Deferred (documented, not stubbed): ItemDisplayBinding, TextTransform (a Picker no-op in C# anyway).

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "maui/controls/observable_collection.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/font.hpp"
#include "maui/core/i_picker.hpp"
#include "maui/core/property.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class picker : public view<maui::core::i_picker>
    {
    public:
        using items_source_type = observable_collection<std::string>;

        picker();
        picker(const picker&) = delete;
        picker(picker&&) = delete;
        picker& operator=(const picker&) = delete;
        picker& operator=(picker&&) = delete;
        // Detaches from the tracked ItemsSource (the collection may outlive the picker).
        ~picker() override;

        // Shared bindable-property descriptors (one instance per type, like Picker.*Property).
        static const maui::core::bindable_property<std::string>& title_property();
        static const maui::core::bindable_property<maui::graphics::color>& title_color_property();
        static const maui::core::bindable_property<int>& selected_index_property();
        static const maui::core::bindable_property<bool>& is_open_property();
        static const maui::core::bindable_property<std::shared_ptr<items_source_type>>& items_source_property();
        static const maui::core::bindable_property<std::optional<std::string>>& selected_item_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::core::font>& font_property();
        static const maui::core::bindable_property<double>& character_spacing_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& horizontal_text_alignment_property();
        static const maui::core::bindable_property<maui::core::text_alignment>& vertical_text_alignment_property();

        // The Items face (C#'s LockableObservableListWrapper): public mutation throws std::logic_error
        // (the InvalidOperationException analog) while an ItemsSource holds the lock; the picker's own
        // internal_* mutations bypass the lock but still notify (so the handler reloads).
        class item_list
        {
        public:
            void add(std::string item)
            {
                insert(owner_->items_.count(), std::move(item));
            }
            void insert(std::size_t index, std::string item)
            {
                throw_if_locked();
                owner_->items_.insert(index, std::move(item));
            }
            void remove_at(std::size_t index)
            {
                throw_if_locked();
                owner_->items_.remove_at(index);
            }
            void clear()
            {
                throw_if_locked();
                owner_->items_.clear();
            }
            [[nodiscard]] std::size_t count() const
            {
                return owner_->items_.count();
            }
            [[nodiscard]] const std::string& at(std::size_t index) const
            {
                return owner_->items_.at(index);
            }
            [[nodiscard]] std::ptrdiff_t index_of(const std::string& item) const
            {
                return owner_->items_.index_of(item);
            }

        private:
            friend class picker;
            explicit item_list(picker& owner) : owner_(&owner)
            {
            }
            void throw_if_locked() const
            {
                if (owner_->items_locked_)
                {
                    throw std::logic_error("picker: Items cannot be modified while ItemsSource is set");
                }
            }
            picker* owner_;
        };

        [[nodiscard]] item_list& items()
        {
            return item_list_;
        }
        [[nodiscard]] const item_list& items() const
        {
            return item_list_;
        }

        // ---- i_picker ----
        [[nodiscard]] std::string_view title() const override
        {
            return title_.get();
        }
        [[nodiscard]] maui::graphics::color title_color() const override
        {
            return title_color_.get();
        }
        [[nodiscard]] int selected_index() const override
        {
            return selected_index_.get();
        }
        // The developer setter AND the handler write-back channel (see the specificity note above).
        void set_selected_index(int value) override
        {
            selected_index_.set(value);
        }
        [[nodiscard]] bool is_open() const override
        {
            return is_open_.get();
        }
        // The developer setter AND the handler write-back channel (editing-begin/end on iOS).
        void set_is_open(bool value) override
        {
            is_open_.set(value);
        }

        // ---- i_item_delegate<std::string> (the face the platform recipes read items through) ----
        [[nodiscard]] int get_count() const override;
        [[nodiscard]] std::string get_item(int index) const override;

        // ---- i_text_style / i_text_alignment ----
        [[nodiscard]] maui::graphics::color text_color() const override
        {
            return text_color_.get();
        }
        [[nodiscard]] maui::core::font font() const override
        {
            return font_.get();
        }
        [[nodiscard]] double character_spacing() const override
        {
            return character_spacing_.get();
        }
        [[nodiscard]] maui::core::text_alignment horizontal_text_alignment() const override
        {
            return horizontal_text_alignment_.get();
        }
        [[nodiscard]] maui::core::text_alignment vertical_text_alignment() const override
        {
            return vertical_text_alignment_.get();
        }

        // ---- ItemsSource / SelectedItem ----
        [[nodiscard]] const std::shared_ptr<items_source_type>& items_source() const
        {
            return items_source_.get();
        }
        void set_items_source(std::shared_ptr<items_source_type> value)
        {
            items_source_.set(std::move(value));
        }
        [[nodiscard]] const std::optional<std::string>& selected_item() const
        {
            return selected_item_.get();
        }
        void set_selected_item(std::optional<std::string> value)
        {
            selected_item_.set(std::move(value));
        }

        // ---- public setters (drive the handler via on_property_changed → update_value) ----
        void set_title(std::string value)
        {
            title_.set(std::move(value));
        }
        void set_title_color(maui::graphics::color value)
        {
            title_color_.set(value);
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        void set_font(maui::core::font value)
        {
            font_.set(std::move(value));
        }
        void set_character_spacing(double value)
        {
            character_spacing_.set(value);
        }
        void set_horizontal_text_alignment(maui::core::text_alignment value)
        {
            horizontal_text_alignment_.set(value);
        }
        void set_vertical_text_alignment(maui::core::text_alignment value)
        {
            vertical_text_alignment_.set(value);
        }

        // ---- developer-facing events ----
        maui::core::event<> selected_index_changed; // Picker.SelectedIndexChanged
        maui::core::event<> opened;                 // Picker.Opened (PickerOpenedEventArgs.Empty)
        maui::core::event<> closed;                 // Picker.Closed (PickerClosedEventArgs.Empty)

    private:
        // The descriptor callbacks (picker.cpp) reach the private machinery below.
        friend struct picker_descriptor_access;

        // Picker.HandleIsOpenChanged: raise Opened when the new value is true, else Closed (the value
        // is already stored, so a handler reading is_open() sees the transition's result).
        void on_is_open_changed(bool new_value) const;

        // Picker.GetSelectedIndex: re-derive the index from the selected item when one is set.
        [[nodiscard]] int get_selected_index() const;
        // Picker.ClampSelectedIndex: clamp + store at from_handler; an unchanged index still refreshes
        // the selected item.
        void clamp_selected_index(int index);
        // Picker.UpdateSelectedItem / UpdateSelectedIndex (both store at from_handler, like C#).
        void update_selected_item(int index);
        void update_selected_index(const std::optional<std::string>& item);
        // Picker.ResetItems / AddItems / RemoveItems / CollectionChanged / OnItemsCollectionChanged.
        void reset_items();
        void add_items(const collection_changed_args<std::string>& args);
        void remove_items(const collection_changed_args<std::string>& args);
        void on_items_source_collection_changed(const collection_changed_args<std::string>& args);
        void on_items_collection_changed();
        void on_items_source_changed(const std::shared_ptr<items_source_type>& old_value,
                                     const std::shared_ptr<items_source_type>& new_value);
        // Handler?.UpdateValue(nameof(IPicker.Items)).
        void notify_handler_items_changed();

        items_source_type items_; // the display list (LockableObservableListWrapper)
        bool items_locked_ = false;
        item_list item_list_{*this};
        maui::core::scoped_connection items_connection_; // the picker's own Items subscription
        // The tracked ItemsSource subscription: the picker keeps its own shared_ptr alive for the
        // connection's lifetime (§8 — a scoped_connection alone could outlive the source event), and
        // disconnects in the destructor (the collection may outlive the picker).
        std::shared_ptr<items_source_type> tracked_items_source_;
        maui::core::connection_token items_source_token_ = 0;

        maui::core::property<std::string> title_{*this, title_property()};
        maui::core::property<maui::graphics::color> title_color_{*this, title_color_property()};
        maui::core::property<int> selected_index_{*this, selected_index_property()};
        maui::core::property<bool> is_open_{*this, is_open_property()};
        maui::core::property<std::shared_ptr<items_source_type>> items_source_{*this, items_source_property()};
        maui::core::property<std::optional<std::string>> selected_item_{*this, selected_item_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::core::font> font_{*this, font_property()};
        maui::core::property<double> character_spacing_{*this, character_spacing_property()};
        maui::core::property<maui::core::text_alignment> horizontal_text_alignment_{
            *this, horizontal_text_alignment_property()};
        maui::core::property<maui::core::text_alignment> vertical_text_alignment_{*this,
                                                                                  vertical_text_alignment_property()};
    };
} // namespace maui::controls
