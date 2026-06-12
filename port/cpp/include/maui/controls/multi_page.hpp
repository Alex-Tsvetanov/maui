#pragma once
// maui::controls::multi_page<TPage>  <=  Microsoft.Maui.Controls.MultiPage<T>
//
// The abstract multi-page container (the base of tabbed_page): an ordered Children list of pages with a
// CurrentPage, plus the templated-items path — ItemsSource + ItemTemplate materializing one page per
// item (BindingContext = the item) with SelectedItem coercion. Ported from MultiPage.cs with the
// TemplatedItemsList collapsed INTO this class (the port's items sources are typed
// observable_collection<TItem> / plain vectors, adapted through type-erased closures captured when the
// typed set_items_source<TItem> is called — the reflection-free stand-in for IEnumerable + ListProxy).
//
// Shape notes (all deviations documented):
//   - TPage is the port's page base (content_page or a subtype) — C#'s `where T : Page`; the port's
//     minimal Page IS content_page (Title + the Appearing/Disappearing lifecycle).
//   - Children the CALLER adds are NON-owning (PROFILE §8); pages the TEMPLATE path creates are OWNED
//     by this container (the template mints them — element_template returns an owning shared_ptr).
//   - The internal "Index" attached property (MultiPage.SetIndex/GetIndex) lives in a pointer-keyed map
//     on the container (the grid cell_info precedent) — get_index(page) / get_page_by_index(index).
//     The C# incremental re-index choreography is collapsed to a full re-index after each mutation
//     (identical end state: every child's index == its position).
//   - pages_changed carries maui::core::collection_changed_args (counts + indices; the C# args carry
//     the item lists — see observable_collection.hpp).
//   - With an ItemsSource set, the Children list is READ-ONLY: add/insert/remove no-op (C#'s
//     ElementCollection throws NotSupportedException — the port's no-exception seam convention).
//   - SelectedItem is held as a boxed std::any + an equality closure captured from the typed setter
//     (C#'s object SelectedItem with object equality); it is not a bindable property<T> here (std::any
//     has no operator== — SelectedItem binding is markup-era and lands with XAML).
//   - C#'s `SelectedItem is T` page-typed fallback in UpdateCurrentPage and the SendNavigatingFrom /
//     SendNavigatedFrom/To page navigation events in the CurrentPage setter are DEFERRED (content_page
//     has no per-page navigation plumbing at this layer — the navigation_page precedent).
//   - The handler drive mirrors TabbedPage.Mapper.cs + OnHandlerChangingCore: every pages change
//     refreshes the handler's "items_source" (C# Handler.UpdateValue(ItemsSource)); a child's Title
//     change does too (the OnPagePropertyChanged wiring — wired permanently here, not only while a
//     handler is attached; firing on a null handler is a no-op).
//
// CurrentPage setter ORDER (MultiPage.CurrentPage): previous.SendDisappearing → the SelectedItem sync
// (the OnPropertyChanged("CurrentPage") hook body, which runs BEFORE the event) → the "current_page"
// property-changed event → OnCurrentPageChanged (current_page_changed) → if HasAppeared the new page's
// SendAppearing.

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::controls
{
    template <class TPage> class multi_page : public content_page
    {
        static_assert(std::is_base_of_v<content_page, TPage>,
                      "TPage must derive maui::controls::content_page (the port's Page)");

    public:
        // ---- events ----
        // C# MultiPage<T>.CurrentPageChanged.
        maui::core::event<> current_page_changed;
        // C# MultiPage<T>.PagesChanged (NotifyCollectionChangedEventHandler).
        maui::core::event<const maui::core::collection_changed_args&> pages_changed;

        // ---- Children (IList<T>; the caller owns pages it adds — template-created pages are owned here) ----
        [[nodiscard]] const std::vector<TPage*>& children() const
        {
            return children_;
        }

        void add(TPage& page)
        {
            insert(children_.size(), page);
        }

        void insert(std::size_t index, TPage& page)
        {
            if (items_)
            {
                return; // Children.IsReadOnly while an ItemsSource is set (C# throws; no-op seam)
            }
            if (index > children_.size())
            {
                index = children_.size();
            }
            children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(index), &page);
            on_page_entered(page);
            on_children_changed({.action = maui::core::collection_changed_action::add,
                                 .new_starting_index = static_cast<int>(index),
                                 .new_count = 1});
        }

        void remove(TPage& page)
        {
            if (items_)
            {
                return; // read-only (see add)
            }
            const int index = index_of_child(&page);
            if (index < 0)
            {
                return;
            }
            children_.erase(children_.begin() + index);
            on_page_left(page);
            on_children_changed(
                {.action = maui::core::collection_changed_action::remove, .old_starting_index = index, .old_count = 1});
        }

        // ---- the internal Index attached property (MultiPage.GetIndex / GetPageByIndex) ----
        // -1 when the page was never indexed by this container (C#'s IndexProperty default).
        [[nodiscard]] int get_index(const TPage& page) const
        {
            const auto found = indices_.find(&page);
            return found != indices_.end() ? found->second : -1;
        }

        [[nodiscard]] TPage* get_page_by_index(int index) const
        {
            for (TPage* const page : children_)
            {
                if (get_index(*page) == index)
                {
                    return page;
                }
            }
            return nullptr;
        }

        // ---- CurrentPage ----
        [[nodiscard]] TPage* current_page() const
        {
            return current_;
        }

        void set_current_page(TPage* value)
        {
            if (current_ == value)
            {
                return;
            }
            TPage* const previous = current_;
            // C# SendNavigatingFrom(previous → value) — deferred (no page navigation plumbing here).
            current_ = value;
            if (previous != nullptr)
            {
                previous->send_disappearing();
            }
            // The C# OnPropertyChanged("CurrentPage") hook body runs BEFORE the event is raised: with an
            // ItemsSource, SelectedItem follows the new current page (null page → null selection).
            sync_selected_from_current();
            this->on_property_changed("current_page"); // the event + the handler's MapCurrentPage
            current_page_changed.raise();              // OnCurrentPageChanged
            if (this->has_appeared() && current_ != nullptr)
            {
                current_->send_appearing();
            }
            // C# SendNavigatedFrom/To — deferred (see above).
        }

        // ---- ItemsSource (typed; the IEnumerable + TemplatedItemsList collapse) ----
        // Bind to a live observable collection: every collection change re-materializes pages
        // (add/remove/move/replace translate 1:1; a clear resets).
        template <class TItem> void set_items_source(std::shared_ptr<maui::core::observable_collection<TItem>> items)
        {
            items_adapter adapter = make_adapter<TItem>(items);
            adapter.changed = maui::core::connect_scoped(
                items->collection_changed,
                [this](const maui::core::collection_changed_args& args) { on_items_changed(args); });
            install_adapter(std::move(adapter));
        }

        // Bind to a fixed snapshot (the C# array ItemsSource — no change notifications).
        template <class TItem> void set_items_source(std::vector<TItem> items)
        {
            auto fixed = std::make_shared<maui::core::observable_collection<TItem>>(std::move(items));
            install_adapter(make_adapter<TItem>(std::move(fixed)));
        }

        // C# ItemsSource = null: back to the caller-managed Children list (emptied by the reset).
        void clear_items_source()
        {
            if (!items_)
            {
                return;
            }
            items_.reset();
            this->on_property_changed("items_source"); // C# flips _children.IsReadOnly here
            reset_pages();
        }

        [[nodiscard]] bool has_items_source() const
        {
            return items_.has_value();
        }

        // ---- ItemTemplate ----
        [[nodiscard]] const std::shared_ptr<data_template>& item_template() const
        {
            return item_template_;
        }

        void set_item_template(std::shared_ptr<data_template> value)
        {
            if (item_template_ == value)
            {
                return;
            }
            item_template_ = std::move(value);
            this->on_property_changed("item_template");
            if (items_)
            {
                reset_pages(); // TemplatedItemsList resets on an ItemTemplateProperty change
            }
        }

        // ---- SelectedItem (object, TwoWay in C#; boxed + typed accessors here) ----
        template <class TItem> void set_selected_item(TItem value)
        {
            set_selected_boxed(std::any{std::move(value)}, make_equals<TItem>());
        }

        template <class TItem> [[nodiscard]] std::optional<TItem> selected_item() const
        {
            if (const auto* value = std::any_cast<TItem>(&selected_item_))
            {
                return *value;
            }
            return std::nullopt;
        }

        [[nodiscard]] bool has_selected_item() const
        {
            return selected_item_.has_value();
        }

    protected:
        multi_page() = default;

        // C# MultiPage<T>.CreateDefault(object item): make the default page for an untemplated item. The
        // reflection-free port hands the item's display text (C#'s item.ToString()); the typed item is
        // applied as the created page's BindingContext by the caller.
        [[nodiscard]] virtual std::shared_ptr<TPage> create_default(const std::string& item_text) = 0;

        // Every child page is a logical child (InternalChildren), so BindingContext + Window inherit.
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            for (TPage* const page : children_)
            {
                visit(*page);
            }
        }

    private:
        // The type-erased view of one typed items source (the ListProxy role).
        struct items_adapter
        {
            std::function<std::size_t()> count;
            std::function<std::any(std::size_t)> at;          // boxed copy of the item
            std::function<int(const std::any&)> index_of;     // -1 when absent or a different type
            std::function<std::string(const std::any&)> text; // item.ToString() stand-in
            std::function<void(maui::core::bindable_object&, const std::any&)> apply_context;
            std::function<bool(const std::any&, const std::any&)> equals;
            maui::core::scoped_connection changed; // observable subscription (empty for snapshots)
        };

        template <class TItem> [[nodiscard]] static std::function<bool(const std::any&, const std::any&)> make_equals()
        {
            return [](const std::any& left, const std::any& right) {
                const auto* typed_left = std::any_cast<TItem>(&left);
                const auto* typed_right = std::any_cast<TItem>(&right);
                return typed_left != nullptr && typed_right != nullptr && *typed_left == *typed_right;
            };
        }

        template <class TItem>
        [[nodiscard]] items_adapter make_adapter(std::shared_ptr<maui::core::observable_collection<TItem>> items)
        {
            items_adapter adapter;
            adapter.count = [items] { return items->size(); };
            adapter.at = [items](std::size_t index) { return std::any{items->at(index)}; };
            adapter.index_of = [items](const std::any& value) {
                const auto* typed = std::any_cast<TItem>(&value);
                return typed != nullptr ? items->index_of(*typed) : -1;
            };
            adapter.text = [](const std::any& value) {
                // item.ToString(): string-convertible items render themselves; anything else has no
                // reflection-free display form (documented — the oracle's items are strings).
                if constexpr (std::is_convertible_v<TItem, std::string>)
                {
                    return std::string{std::any_cast<const TItem&>(value)};
                }
                else
                {
                    return std::string{};
                }
            };
            adapter.apply_context = [](maui::core::bindable_object& target, const std::any& value) {
                // TemplatedItemsList: the created content's BindingContext is the item.
                target.set_binding_context(std::make_shared<TItem>(std::any_cast<const TItem&>(value)));
            };
            adapter.equals = make_equals<TItem>();
            return adapter;
        }

        void install_adapter(items_adapter adapter)
        {
            // Disconnect the OLD subscription FIRST, while the old closures still pin the old source:
            // a plain member-wise move-assignment replaces the closures (dropping the shared_ptr that
            // keeps the source alive) BEFORE assigning `changed`, whose disconnect would then touch a
            // freed event (ASan-caught heap-use-after-free on items_source replacement).
            if (items_.has_value())
            {
                items_->changed = {};
            }
            items_ = std::move(adapter);
            this->on_property_changed("items_source"); // C# flips _children.IsReadOnly here
            reset_pages();
        }

        [[nodiscard]] int index_of_child(const TPage* page) const
        {
            for (std::size_t i = 0; i < children_.size(); ++i)
            {
                if (children_[i] == page)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        // A page joined the children list: it becomes a logical child (Element.OnChildAdded) and its
        // Title changes refresh the handler's tab items (the OnPagePropertyChanged wiring).
        void on_page_entered(TPage& page)
        {
            this->attach_logical_child(page);
            title_connections_[&page] =
                maui::core::connect_scoped(page.property_changed, [this](std::string_view name) {
                    if (name == "title")
                    {
                        notify_handler_pages_changed();
                    }
                });
        }

        // A page left the children list (Element.OnChildRemoved). A template-created page's ownership
        // moves to the retirement list instead of dying here: the change handler may still need the
        // page alive (C#'s CurrentPage setter fires SendDisappearing on the page being replaced — in GC
        // land a removed page outlives its removal). release_retired_pages() runs once the whole change
        // (including the CurrentPage re-coercion) has completed.
        void on_page_left(TPage& page)
        {
            title_connections_.erase(&page);
            indices_.erase(&page);
            detach_logical_child(page);
            if (auto owned = owned_pages_.find(&page); owned != owned_pages_.end())
            {
                retired_pages_.push_back(std::move(owned->second));
                owned_pages_.erase(owned);
            }
        }

        void release_retired_pages()
        {
            retired_pages_.clear();
        }

        void reindex()
        {
            indices_.clear();
            for (std::size_t i = 0; i < children_.size(); ++i)
            {
                indices_[children_[i]] = static_cast<int>(i);
            }
        }

        void notify_handler_pages_changed()
        {
            if (const auto& element_handler = this->handler())
            {
                element_handler->update_value("items_source"); // C# Handler.UpdateValue(ItemsSource)
            }
        }

        // MultiPage.OnChildrenChanged (the caller-managed path): re-index, raise PagesChanged, refresh
        // the handler, then coerce CurrentPage onto the list.
        void on_children_changed(const maui::core::collection_changed_args& args)
        {
            reindex();
            pages_changed.raise(args);
            notify_handler_pages_changed();
            if (current_ == nullptr || index_of_child(current_) == -1)
            {
                set_current_page(children_.empty() ? nullptr : children_.front());
            }
        }

        // Create the page for one boxed item: the ItemTemplate's content when set (and page-typed),
        // else CreateDefault — either way the item becomes the page's BindingContext.
        [[nodiscard]] std::shared_ptr<TPage> create_page_for(const std::any& item)
        {
            std::shared_ptr<TPage> page;
            if (item_template_ != nullptr && item_template_->has_load_template())
            {
                page = std::dynamic_pointer_cast<TPage>(item_template_->create_content());
            }
            if (page == nullptr)
            {
                page = create_default(items_->text(item));
            }
            items_->apply_context(*page, item);
            return page;
        }

        void insert_item_page(std::size_t index, const std::any& item)
        {
            std::shared_ptr<TPage> page = create_page_for(item);
            TPage* const raw = page.get();
            owned_pages_[raw] = std::move(page);
            children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(index), raw);
            on_page_entered(*raw);
        }

        void remove_item_page(std::size_t index)
        {
            TPage* const page = children_[index];
            children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(index));
            on_page_left(*page);
        }

        // MultiPage.OnTemplatedItemsChanged: translate an items-source change into page mutations, raise
        // PagesChanged with the same shape, then re-coerce CurrentPage from SelectedItem.
        void on_items_changed(const maui::core::collection_changed_args& args)
        {
            switch (args.action)
            {
                case maui::core::collection_changed_action::add:
                    for (std::size_t i = 0; i < args.new_count; ++i)
                    {
                        const std::size_t index = static_cast<std::size_t>(args.new_starting_index) + i;
                        insert_item_page(index, items_->at(index));
                    }
                    break;

                case maui::core::collection_changed_action::remove:
                    for (std::size_t i = 0; i < args.old_count; ++i)
                    {
                        remove_item_page(static_cast<std::size_t>(args.old_starting_index));
                    }
                    break;

                case maui::core::collection_changed_action::move: {
                    const auto old_index = static_cast<std::size_t>(args.old_starting_index);
                    const auto new_index = static_cast<std::size_t>(args.new_starting_index);
                    if (new_index == old_index)
                    {
                        return;
                    }
                    // The C# Move handler: extract the moved pages, then insert at the index adjusted
                    // when moving forward (insertIndex -= count - 1) — the ObservableList math.
                    std::vector<TPage*> moved(children_.begin() + static_cast<std::ptrdiff_t>(old_index),
                                              children_.begin() +
                                                  static_cast<std::ptrdiff_t>(old_index + args.old_count));
                    children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(old_index),
                                    children_.begin() + static_cast<std::ptrdiff_t>(old_index + args.old_count));
                    std::size_t insert_index = new_index;
                    if (new_index > old_index)
                    {
                        insert_index -= args.old_count - 1;
                    }
                    children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(insert_index), moved.begin(),
                                     moved.end());
                    break;
                }

                case maui::core::collection_changed_action::replace:
                    for (std::size_t i = 0; i < args.new_count; ++i)
                    {
                        const std::size_t index = static_cast<std::size_t>(args.new_starting_index) + i;
                        remove_item_page(index);
                        insert_item_page(index, items_->at(index));
                    }
                    break;

                case maui::core::collection_changed_action::reset:
                    reset_pages();
                    return;
            }

            reindex();
            pages_changed.raise(args);
            notify_handler_pages_changed();
            update_current_page();
            release_retired_pages(); // safe now — current_ no longer points at a removed page
        }

        // MultiPage.Reset: rebuild every page from the items source (or empty the list without one),
        // coerce SelectedItem into the new source, and raise ONE Reset PagesChanged.
        void reset_pages()
        {
            const std::vector<TPage*> snapshot = children_;
            children_.clear();
            for (TPage* const page : snapshot)
            {
                on_page_left(*page);
            }

            if (items_)
            {
                const std::size_t count = items_->count();
                for (std::size_t i = 0; i < count; ++i)
                {
                    insert_item_page(i, items_->at(i));
                }
            }
            reindex();

            bool current_needs_update = true;
            this->batch_begin();
            if (items_)
            {
                // C#: a missing/now-absent SelectedItem snaps to the first item; setting it runs the
                // SelectedItem property hook (which updates CurrentPage), so the explicit update is skipped.
                if (!selected_item_.has_value() || items_->index_of(selected_item_) == -1)
                {
                    if (items_->count() > 0)
                    {
                        set_selected_boxed(items_->at(0), items_->equals);
                    }
                    else
                    {
                        clear_selected();
                    }
                    current_needs_update = false;
                }
            }
            if (current_needs_update)
            {
                update_current_page();
            }
            pages_changed.raise({.action = maui::core::collection_changed_action::reset});
            notify_handler_pages_changed();
            this->batch_commit();
            release_retired_pages(); // safe now — current_ no longer points at a dropped page
        }

        // MultiPage.UpdateCurrentPage: with an ItemsSource, CurrentPage follows SelectedItem's index
        // (absent → the first child). (C#'s untyped `SelectedItem is T` fallback is deferred — header.)
        void update_current_page()
        {
            if (!items_)
            {
                return;
            }
            const int index = selected_item_.has_value() ? items_->index_of(selected_item_) : -1;
            if (index == -1)
            {
                set_current_page(children_.empty() ? nullptr : children_.front());
            }
            else if (static_cast<std::size_t>(index) < children_.size())
            {
                set_current_page(children_[static_cast<std::size_t>(index)]);
            }
        }

        // The SelectedItem property store: mutate, run the C# OnPropertyChanged(SelectedItem) hook
        // (UpdateCurrentPage), THEN raise the property-changed event — the C# member order.
        void set_selected_boxed(std::any value, std::function<bool(const std::any&, const std::any&)> equals)
        {
            if (selected_item_.has_value() && value.has_value() && selected_equals_ &&
                selected_equals_(selected_item_, value))
            {
                return; // SetValue with an equal value is a no-op
            }
            selected_item_ = std::move(value);
            selected_equals_ = std::move(equals);
            update_current_page();
            this->on_property_changed("selected_item");
        }

        void clear_selected()
        {
            if (!selected_item_.has_value())
            {
                return;
            }
            selected_item_.reset();
            selected_equals_ = nullptr;
            update_current_page();
            this->on_property_changed("selected_item");
        }

        // The C# MultiPage.OnPropertyChanged("CurrentPage") hook: with an ItemsSource, SelectedItem
        // follows the current page (a null current page clears the selection).
        void sync_selected_from_current()
        {
            if (!items_)
            {
                return;
            }
            if (current_ == nullptr)
            {
                clear_selected();
                return;
            }
            const int index = index_of_child(current_);
            if (index >= 0 && static_cast<std::size_t>(index) < items_->count())
            {
                set_selected_boxed(items_->at(static_cast<std::size_t>(index)), items_->equals);
            }
            else
            {
                clear_selected();
            }
        }

        std::vector<TPage*> children_; // non-owning list (template-created pages owned below)
        std::unordered_map<const TPage*, std::shared_ptr<TPage>> owned_pages_;
        std::vector<std::shared_ptr<TPage>> retired_pages_; // removed template pages, alive until the
                                                            // change completes (see on_page_left)
        std::unordered_map<const TPage*, int> indices_;     // the internal Index attached property
        std::unordered_map<const TPage*, maui::core::scoped_connection> title_connections_;
        TPage* current_ = nullptr;
        std::shared_ptr<data_template> item_template_;
        std::optional<items_adapter> items_;
        std::any selected_item_; // empty = null (C# object SelectedItem)
        std::function<bool(const std::any&, const std::any&)> selected_equals_;
    };
} // namespace maui::controls
