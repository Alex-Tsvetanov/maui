// maui::controls::picker — out-of-line definitions: the shared bindable-property descriptors (one
// instance per type, like Picker.*Property), the Items/ItemsSource tracking machinery (Picker.cs's
// CollectionChanged / ResetItems / AddItems / RemoveItems / GetSelectedIndex / ClampSelectedIndex /
// UpdateSelectedItem / UpdateSelectedIndex), and the default-handler self-registration.

#include "maui/controls/picker.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "maui/controls/observable_collection.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/binding_mode.hpp"
#include "maui/core/font.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/picker_handler.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/text_alignment.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    namespace
    {
        // NumericExtensions.Clamp(int, min, max): Math.Min(Math.Max(self, min), max) — with an empty
        // list max is -1, so everything lands on -1.
        int clamp_index(int value, int min, int max)
        {
            return value < min ? min : (value > max ? max : value);
        }
    } // namespace

    // Grants the descriptor callbacks access to the picker's selection machinery (the C# analog is
    // the static propertyChanged/coerceValue delegates living inside the Picker class itself).
    struct picker_descriptor_access
    {
        // Picker.CoerceSelectedIndex: clamp into [-1, Items.Count - 1].
        static int coerce_selected_index(const picker& self, int value)
        {
            return clamp_index(value, -1, static_cast<int>(self.items_.count()) - 1);
        }

        // Picker.OnSelectedIndexChanged: refresh the selected item from the CURRENT index (a
        // reentrant clamp may have moved it past the stored new-value), then raise the event.
        static void on_selected_index_changed(picker& self)
        {
            self.update_selected_item(self.selected_index());
            self.selected_index_changed.raise();
        }

        // Picker.OnSelectedItemChanged → UpdateSelectedIndex(newValue).
        static void on_selected_item_changed(picker& self, const std::optional<std::string>& new_value)
        {
            self.update_selected_index(new_value);
        }

        // Picker.OnIsOpenPropertyChanged → HandleIsOpenChanged (raise Opened/Closed by transition).
        static void on_is_open_changed(picker& self, bool new_value)
        {
            self.on_is_open_changed(new_value);
        }

        static void on_items_source_changed(picker& self, const std::shared_ptr<picker::items_source_type>& old_value,
                                            const std::shared_ptr<picker::items_source_type>& new_value)
        {
            self.on_items_source_changed(old_value, new_value);
        }
    };

    const maui::core::bindable_property<std::string>& picker::title_property()
    {
        static const maui::core::bindable_property<std::string> descriptor{"title", std::string{}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& picker::title_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"title_color"};
        return descriptor;
    }

    // Picker.SelectedIndexProperty: default -1, TwoWay, coerced into the Items range; a change updates
    // the selected item and raises SelectedIndexChanged.
    const maui::core::bindable_property<int>& picker::selected_index_property()
    {
        static const maui::core::bindable_property<int> descriptor{
            "selected_index",
            -1,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const int&, const int&) {
                     picker_descriptor_access::on_selected_index_changed(dynamic_cast<picker&>(bindable));
                 },
             .coerce_value =
                 [](maui::core::bindable_object& bindable, const int& value) {
                     return picker_descriptor_access::coerce_selected_index(dynamic_cast<picker&>(bindable), value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // Picker.IsOpenProperty: default false, TwoWay; a change raises Opened/Closed by transition
    // (Picker.OnIsOpenPropertyChanged → HandleIsOpenChanged).
    const maui::core::bindable_property<bool>& picker::is_open_property()
    {
        static const maui::core::bindable_property<bool> descriptor{
            "is_open",
            false,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const bool&, const bool& new_value) {
                     picker_descriptor_access::on_is_open_changed(dynamic_cast<picker&>(bindable), new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    // Picker.ItemsSourceProperty: a change re-seats the collection subscription and resets Items.
    const maui::core::bindable_property<std::shared_ptr<picker::items_source_type>>& picker::items_source_property()
    {
        static const maui::core::bindable_property<std::shared_ptr<items_source_type>> descriptor{
            "items_source",
            nullptr,
            {.property_changed = [](maui::core::bindable_object& bindable,
                                    const std::shared_ptr<items_source_type>& old_value,
                                    const std::shared_ptr<items_source_type>& new_value) {
                picker_descriptor_access::on_items_source_changed(dynamic_cast<picker&>(bindable), old_value,
                                                                  new_value);
            }}};
        return descriptor;
    }

    // Picker.SelectedItemProperty: default null, TwoWay; a change re-derives the index.
    const maui::core::bindable_property<std::optional<std::string>>& picker::selected_item_property()
    {
        static const maui::core::bindable_property<std::optional<std::string>> descriptor{
            "selected_item",
            std::nullopt,
            {.property_changed =
                 [](maui::core::bindable_object& bindable, const std::optional<std::string>&,
                    const std::optional<std::string>& new_value) {
                     picker_descriptor_access::on_selected_item_changed(dynamic_cast<picker&>(bindable), new_value);
                 },
             .default_binding_mode = maui::core::binding_mode::two_way}};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& picker::text_color_property()
    {
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"text_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::font>& picker::font_property()
    {
        static const maui::core::bindable_property<maui::core::font> descriptor{"font"};
        return descriptor;
    }

    const maui::core::bindable_property<double>& picker::character_spacing_property()
    {
        static const maui::core::bindable_property<double> descriptor{"character_spacing", 0.0};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& picker::horizontal_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "horizontal_text_alignment", maui::core::text_alignment::start};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::text_alignment>& picker::vertical_text_alignment_property()
    {
        static const maui::core::bindable_property<maui::core::text_alignment> descriptor{
            "vertical_text_alignment", maui::core::text_alignment::center};
        return descriptor;
    }

    picker::picker()
    {
        this->set_style_target_type<picker>();
        // Picker(): ((INotifyCollectionChanged)Items).CollectionChanged += OnItemsCollectionChanged.
        items_connection_ = maui::core::scoped_connection{
            items_.collection_changed,
            items_.collection_changed.connect(
                [this](const collection_changed_args<std::string>&) { on_items_collection_changed(); })};
    }

    picker::~picker()
    {
        // The tracked ItemsSource may outlive the picker — detach before `this` goes away.
        if (tracked_items_source_)
        {
            tracked_items_source_->collection_changed.disconnect(items_source_token_);
        }
    }

    int picker::get_count() const
    {
        // IItemDelegate<string>.GetCount: Items?.Count ?? ItemsSource?.Count ?? 0 — Items always
        // exists in the port (and mirrors the source after a reset).
        return static_cast<int>(items_.count());
    }

    std::string picker::get_item(int index) const
    {
        // IItemDelegate<string>.GetItem: negative/out-of-range answers empty, never throws.
        if (index < 0)
        {
            return {};
        }
        if (std::cmp_less(index, items_.count()))
        {
            return items_.at(static_cast<std::size_t>(index));
        }
        const auto& source = items_source_.get();
        if (source && std::cmp_less(index, source->count()))
        {
            return source->at(static_cast<std::size_t>(index));
        }
        return {};
    }

    int picker::get_selected_index() const
    {
        // Picker.GetSelectedIndex: with no selected item the stored index stands; otherwise re-derive
        // the index from the item's position (falling back to the stored index on a miss).
        const auto& item = selected_item_.get();
        if (!item.has_value())
        {
            return selected_index_.get();
        }
        const auto& source = items_source_.get();
        const std::ptrdiff_t new_index = source ? source->index_of(*item) : items_.index_of(*item);
        return new_index >= 0 ? static_cast<int>(new_index) : selected_index_.get();
    }

    void picker::clamp_selected_index(int index)
    {
        // Picker.ClampSelectedIndex: clamp + store at FromHandler; an unchanged index still refreshes
        // the selected item (the coerced store is silent then).
        const int old_index = index;
        const int new_index = clamp_index(index, -1, static_cast<int>(items_.count()) - 1);
        selected_index_.set(new_index, maui::core::setter_specificity::from_handler);
        if (new_index == old_index)
        {
            update_selected_item(new_index);
        }
    }

    void picker::update_selected_item(int index)
    {
        // Picker.UpdateSelectedItem (FromHandler specificity, like C#).
        if (index == -1)
        {
            selected_item_.set(std::nullopt, maui::core::setter_specificity::from_handler);
            return;
        }
        const auto& source = items_source_.get();
        if (source)
        {
            std::optional<std::string> item;
            if (std::cmp_less(index, source->count()))
            {
                item = source->at(static_cast<std::size_t>(index));
            }
            selected_item_.set(std::move(item), maui::core::setter_specificity::from_handler);
            return;
        }
        std::optional<std::string> item;
        if (std::cmp_less(index, items_.count()))
        {
            item = items_.at(static_cast<std::size_t>(index));
        }
        selected_item_.set(std::move(item), maui::core::setter_specificity::from_handler);
    }

    void picker::update_selected_index(const std::optional<std::string>& item)
    {
        // Picker.UpdateSelectedIndex: (ItemsSource ?? Items).IndexOf(item), stored at FromHandler.
        std::ptrdiff_t index = -1;
        if (item.has_value())
        {
            const auto& source = items_source_.get();
            index = source ? source->index_of(*item) : items_.index_of(*item);
        }
        selected_index_.set(static_cast<int>(index), maui::core::setter_specificity::from_handler);
    }

    void picker::reset_items()
    {
        // Picker.ResetItems: rebuild the display list from the source (internal mutations — the lock
        // makes on_items_collection_changed skip them), notify the handler, then re-clamp.
        const auto& source = items_source_.get();
        if (!source)
        {
            return;
        }
        items_.clear();
        for (const auto& item : source->items())
        {
            items_.add(item);
        }
        notify_handler_items_changed();
        clamp_selected_index(selected_index_.get());
    }

    void picker::add_items(const collection_changed_args<std::string>& args)
    {
        // Picker.AddItems: insert the new display items, then re-clamp when the insertion shifted the
        // selection (GetSelectedIndex follows the selected ITEM — dotnet/maui#29235).
        const std::size_t insert_index =
            args.new_starting_index < 0 ? items_.count() : static_cast<std::size_t>(args.new_starting_index);
        std::size_t index = insert_index;
        for (const auto& item : args.new_items)
        {
            items_.insert(index++, item);
        }
        const int selected = get_selected_index();
        if (std::cmp_less_equal(insert_index, selected))
        {
            clamp_selected_index(selected);
        }
    }

    void picker::remove_items(const collection_changed_args<std::string>& args)
    {
        // Picker.RemoveItems: items are removed in reverse order; a start index past the end removes
        // from the tail (the C# branch).
        std::size_t remove_start = 0;
        std::size_t index = 0;
        if (args.old_starting_index >= 0 && static_cast<std::size_t>(args.old_starting_index) < items_.count())
        {
            remove_start = static_cast<std::size_t>(args.old_starting_index);
            index = remove_start + args.old_items.size() - 1;
        }
        else
        {
            remove_start = items_.count() - args.old_items.size();
            index = items_.count() - 1;
        }
        for (std::size_t removed = 0; removed < args.old_items.size(); ++removed)
        {
            items_.remove_at(index--);
        }
        const int selected = get_selected_index();
        if (std::cmp_less_equal(remove_start, selected))
        {
            clamp_selected_index(selected);
        }
    }

    void picker::on_items_source_collection_changed(const collection_changed_args<std::string>& args)
    {
        // Picker.CollectionChanged: Add/Remove flow incrementally, everything else resets.
        switch (args.action)
        {
            case collection_changed_action::add:
                add_items(args);
                break;
            case collection_changed_action::remove:
                remove_items(args);
                break;
            default:
                reset_items();
                break;
        }
        notify_handler_items_changed();
    }

    void picker::on_items_collection_changed()
    {
        // Picker.OnItemsCollectionChanged: skipped while locked (ItemsSource updates handle it).
        if (items_locked_)
        {
            return;
        }
        clamp_selected_index(get_selected_index());
        notify_handler_items_changed();
    }

    void picker::on_items_source_changed(const std::shared_ptr<items_source_type>& /*old_value*/,
                                         const std::shared_ptr<items_source_type>& new_value)
    {
        // Picker.OnItemsSourceChanged: re-seat the subscription (the picker's own shared_ptr keeps
        // the OLD collection's event alive until the disconnect lands — §8), then either lock + reset
        // or unlock + clear (the clear runs the unlocked Items handler on purpose, like C#'s comment).
        if (tracked_items_source_)
        {
            tracked_items_source_->collection_changed.disconnect(items_source_token_);
            tracked_items_source_.reset();
            items_source_token_ = 0;
        }
        if (new_value)
        {
            tracked_items_source_ = new_value;
            items_source_token_ = new_value->collection_changed.connect(
                [this](const collection_changed_args<std::string>& args) { on_items_source_collection_changed(args); });
            items_locked_ = true;
            reset_items();
        }
        else
        {
            items_locked_ = false;
            items_.clear();
        }
    }

    void picker::notify_handler_items_changed()
    {
        // Handler?.UpdateValue(nameof(IPicker.Items)) — re-runs the "items" mapper (Reload).
        if (const auto& element_handler = handler())
        {
            element_handler->update_value("items");
        }
    }

    void picker::on_is_open_changed(bool new_value)
    {
        // Picker.HandleIsOpenChanged: the value is already stored, so raise Opened when it turned true
        // and Closed when it turned false (a handler reading is_open() observes the transition result).
        if (new_value)
        {
            opened.raise();
        }
        else
        {
            closed.raise();
        }
    }
} // namespace maui::controls

// Self-register the default handler for picker (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::picker, maui::core::picker_handler)
