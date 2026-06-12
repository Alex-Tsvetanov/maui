// items_source_factory — the concrete IItemsViewSource family (internal, like the C# classes):
//   empty_source              <= EmptySource
//   list_source               <= ListSource (snapshot; no change feed)
//   observable_items_source   <= ObservableItemsSource (flat live source; also the per-group child)
//   observable_grouped_source <= ObservableGroupedSource (sections over a grouped collection)
// Each translates one collection change into ONE source_update op (the UICollectionView drive the C#
// sources perform directly), raised after its own counts are adjusted — see items_view_source.hpp.
//
// Documented deviations from the C# bodies:
//   - ShouldReload's "Remove with OldStartingIndex < 0" reload fallback is unreachable: the port's
//     observable_collection always reports indices.
//   - Replace with unequal old/new counts falls back to reload_data like C#; the port's collection
//     only emits equal-size replaces, so the branch is defensive.
//   - The multi-item Move reloads the literal C# range (start = min(old,new), COUNT = max(old,new) +
//     moved-count — C# passes that sum as CreateIndexesFrom's count argument), over-covering exactly
//     as the original does.
//   - The grouped source's "is this element a group" test is structural (i_item_collection::
//     group_items non-null) instead of `item is IEnumerable and not string`.

#include "maui/controls/items/items_source_factory.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "maui/controls/items/boxed_item.hpp"
#include "maui/controls/items/item_collection.hpp"
#include "maui/controls/items/items_view_source.hpp"
#include "maui/core/event.hpp"
#include "maui/core/observable_collection.hpp"

namespace maui::controls
{
    namespace
    {
        [[noreturn]] void throw_empty()
        {
            throw std::out_of_range("i_items_view_source is empty");
        }

        // <= EmptySource (the loop surface is out of scope with the carousel).
        class empty_source final : public i_items_view_source
        {
        public:
            [[nodiscard]] int item_count() const override
            {
                return 0;
            }
            [[nodiscard]] int group_count() const override
            {
                return 0;
            }
            [[nodiscard]] int item_count_in_group(int /*group*/) const override
            {
                return 0;
            }
            [[nodiscard]] boxed_item item(const index_path& /*path*/) const override
            {
                throw_empty();
            }
            [[nodiscard]] boxed_item group(const index_path& /*path*/) const override
            {
                throw_empty();
            }
            [[nodiscard]] std::shared_ptr<i_items_view_source> group_items_view_source(
                const index_path& /*path*/) const override
            {
                throw_empty();
            }
            [[nodiscard]] index_path get_index_for_item(const boxed_item& /*item*/) const override
            {
                throw_empty();
            }
        };

        // <= ListSource: a snapshot over the erased collection (one section, no change feed).
        class list_source final : public i_items_view_source
        {
        public:
            explicit list_source(std::shared_ptr<i_item_collection> source) : source_(std::move(source))
            {
            }

            [[nodiscard]] int item_count() const override
            {
                return static_cast<int>(source_->count());
            }
            [[nodiscard]] int group_count() const override
            {
                return 1;
            }
            [[nodiscard]] int item_count_in_group(int group) const override
            {
                if (group > 0)
                {
                    throw std::out_of_range("list_source: group out of range");
                }
                return item_count();
            }
            [[nodiscard]] boxed_item item(const index_path& path) const override
            {
                if (path.section > 0)
                {
                    throw std::out_of_range("list_source: section out of range");
                }
                return source_->at(static_cast<std::size_t>(path.item));
            }
            [[nodiscard]] boxed_item group(const index_path& /*path*/) const override
            {
                return {};
            }
            [[nodiscard]] std::shared_ptr<i_items_view_source> group_items_view_source(
                const index_path& /*path*/) const override
            {
                return nullptr;
            }
            [[nodiscard]] index_path get_index_for_item(const boxed_item& item) const override
            {
                const int found = source_->index_of(item);
                return found >= 0 ? index_path{0, found} : index_path{-1, -1};
            }

        private:
            std::shared_ptr<i_item_collection> source_;
        };

        // <= ObservableItemsSource: the flat live source. `section` > -1 makes it a grouped child
        // (its ops carry that section). Holds the collection alive (the closures' publisher) and
        // disconnects via the scoped_connection member declared AFTER it (§8: reverse destruction
        // order tears the subscription down before the pin drops).
        class observable_items_source final : public i_observable_items_view_source
        {
        public:
            explicit observable_items_source(std::shared_ptr<i_item_collection> source, int group = -1)
                : source_(std::move(source)), section_(group < 0 ? 0 : group),
                  count_(static_cast<int>(source_->count()))
            {
                if (auto* feed = source_->changed())
                {
                    subscription_ =
                        maui::core::connect_scoped(*feed, [this](const maui::core::collection_changed_args& args) {
                            on_collection_changed(args);
                        });
                }
            }

            [[nodiscard]] int item_count() const override
            {
                return count_;
            }
            [[nodiscard]] int group_count() const override
            {
                return 1;
            }
            [[nodiscard]] int item_count_in_group(int /*group*/) const override
            {
                return count_; // C# returns Count regardless of the group argument
            }
            [[nodiscard]] boxed_item item(const index_path& path) const override
            {
                if (path.section != section_)
                {
                    throw std::out_of_range("observable_items_source: section out of range");
                }
                return source_->at(static_cast<std::size_t>(path.item));
            }
            [[nodiscard]] boxed_item group(const index_path& /*path*/) const override
            {
                return {};
            }
            [[nodiscard]] std::shared_ptr<i_items_view_source> group_items_view_source(
                const index_path& /*path*/) const override
            {
                return nullptr;
            }
            [[nodiscard]] index_path get_index_for_item(const boxed_item& item) const override
            {
                for (int index = 0; index < count_; ++index)
                {
                    if (source_->at(static_cast<std::size_t>(index)) == item)
                    {
                        return {section_, index};
                    }
                }
                return {-1, -1};
            }

            [[nodiscard]] bool observe_changes() const override
            {
                return observe_changes_;
            }
            void set_observe_changes(bool value) override
            {
                observe_changes_ = value;
            }

        private:
            void on_collection_changed(const maui::core::collection_changed_args& args)
            {
                if (!observe_changes_)
                {
                    return;
                }
                switch (args.action)
                {
                    case maui::core::collection_changed_action::add:
                        count_ += static_cast<int>(args.new_count);
                        updated.raise({.kind = source_update_kind::insert_items,
                                       .section = section_,
                                       .index = args.new_starting_index,
                                       .count = args.new_count});
                        break;

                    case maui::core::collection_changed_action::remove:
                        count_ -= static_cast<int>(args.old_count);
                        updated.raise({.kind = source_update_kind::delete_items,
                                       .section = section_,
                                       .index = args.old_starting_index,
                                       .count = args.old_count});
                        break;

                    case maui::core::collection_changed_action::replace:
                        if (args.new_count != args.old_count)
                        {
                            reload(); // defensive — see the header deviation note
                            break;
                        }
                        updated.raise({.kind = source_update_kind::reload_items,
                                       .section = section_,
                                       .index = args.new_starting_index,
                                       .count = args.new_count});
                        break;

                    case maui::core::collection_changed_action::move:
                        if (args.new_count == 1)
                        {
                            updated.raise({.kind = source_update_kind::move_item,
                                           .section = section_,
                                           .index = args.old_starting_index,
                                           .count = 1,
                                           .move_to = {section_, args.new_starting_index}});
                            break;
                        }
                        // The literal C# multi-move reload window (header deviation note).
                        updated.raise({.kind = source_update_kind::reload_items,
                                       .section = section_,
                                       .index = std::min(args.old_starting_index, args.new_starting_index),
                                       .count = static_cast<std::size_t>(
                                                    std::max(args.old_starting_index, args.new_starting_index)) +
                                                args.new_count});
                        break;

                    case maui::core::collection_changed_action::reset:
                        reload();
                        break;
                }
            }

            void reload()
            {
                count_ = static_cast<int>(source_->count());
                updated.raise({.kind = source_update_kind::reload_data});
            }

            std::shared_ptr<i_item_collection> source_;
            int section_;
            int count_;
            bool observe_changes_ = true;
            maui::core::scoped_connection subscription_; // after source_ (§8 — see class comment)
        };

        // <= ObservableGroupedSource: sections over a grouped collection. Tracks one child
        // observable_items_source per LIVE group (nested changed() non-null), forwarding the child's
        // already-sectioned item ops through this source's `updated`; group-level changes become
        // section ops and rebuild the tracking (the C# ResetGroupTracking choreography).
        class observable_grouped_source final : public i_observable_items_view_source
        {
        public:
            explicit observable_grouped_source(std::shared_ptr<i_item_collection> source) : source_(std::move(source))
            {
                group_count_ = static_cast<int>(source_->count());
                reset_group_tracking();
                if (auto* feed = source_->changed())
                {
                    subscription_ =
                        maui::core::connect_scoped(*feed, [this](const maui::core::collection_changed_args& args) {
                            on_collection_changed(args);
                        });
                }
            }

            [[nodiscard]] int item_count() const override
            {
                int total = 0;
                const std::size_t groups = source_->count();
                for (std::size_t group = 0; group < groups; ++group)
                {
                    total += group_size(group);
                }
                return total;
            }
            [[nodiscard]] int group_count() const override
            {
                return group_count_;
            }
            [[nodiscard]] int item_count_in_group(int group) const override
            {
                return group_size(static_cast<std::size_t>(group));
            }
            [[nodiscard]] boxed_item item(const index_path& path) const override
            {
                const auto nested = source_->group_items(static_cast<std::size_t>(path.section));
                if (!nested)
                {
                    throw std::out_of_range("observable_grouped_source: section is not a group");
                }
                return nested->at(static_cast<std::size_t>(path.item));
            }
            [[nodiscard]] boxed_item group(const index_path& path) const override
            {
                return source_->at(static_cast<std::size_t>(path.section));
            }
            [[nodiscard]] std::shared_ptr<i_items_view_source> group_items_view_source(
                const index_path& path) const override
            {
                const auto section = static_cast<std::size_t>(path.section);
                if (section < children_.size() && children_[section].source)
                {
                    return children_[section].source;
                }
                // A snapshot group still answers as a source (C# only tracks the INCC groups; the
                // port mints the snapshot view on demand).
                const auto nested = source_->group_items(section);
                return nested ? std::make_shared<list_source>(nested) : nullptr;
            }
            [[nodiscard]] index_path get_index_for_item(const boxed_item& item) const override
            {
                const std::size_t groups = source_->count();
                for (std::size_t group = 0; group < groups; ++group)
                {
                    const auto nested = source_->group_items(group);
                    if (!nested)
                    {
                        continue;
                    }
                    const int index = nested->index_of(item);
                    if (index >= 0)
                    {
                        return {static_cast<int>(group), index};
                    }
                }
                return {-1, -1};
            }

            [[nodiscard]] bool observe_changes() const override
            {
                return observe_changes_;
            }
            void set_observe_changes(bool value) override
            {
                observe_changes_ = value;
                for (auto& child : children_)
                {
                    if (child.source)
                    {
                        child.source->set_observe_changes(value); // propagated (port collapse; header)
                    }
                }
            }

        private:
            // One tracked section: the live child source (null for a snapshot group) + the forward.
            // Member order matters (§8): `forward` (the subscription to source->updated) is declared
            // after `source` so it disconnects first when the entry dies.
            struct child_entry
            {
                std::shared_ptr<observable_items_source> source;
                maui::core::scoped_connection forward;
            };

            [[nodiscard]] int group_size(std::size_t group) const
            {
                const auto nested = source_->group_items(group);
                return nested ? static_cast<int>(nested->count()) : 0;
            }

            void reset_group_tracking()
            {
                children_.clear();
                const std::size_t groups = source_->count();
                children_.reserve(groups);
                for (std::size_t group = 0; group < groups; ++group)
                {
                    child_entry entry;
                    const auto nested = source_->group_items(group);
                    if (nested && nested->changed() != nullptr)
                    {
                        entry.source = std::make_shared<observable_items_source>(nested, static_cast<int>(group));
                        entry.source->set_observe_changes(observe_changes_);
                        entry.forward = maui::core::connect_scoped(
                            entry.source->updated, [this](const source_update& update) { updated.raise(update); });
                    }
                    children_.push_back(std::move(entry));
                }
            }

            void on_collection_changed(const maui::core::collection_changed_args& args)
            {
                if (!observe_changes_)
                {
                    return;
                }
                switch (args.action)
                {
                    case maui::core::collection_changed_action::add:
                        group_count_ += static_cast<int>(args.new_count);
                        reset_group_tracking();
                        updated.raise({.kind = source_update_kind::insert_sections,
                                       .section = args.new_starting_index,
                                       .count = args.new_count});
                        break;

                    case maui::core::collection_changed_action::remove:
                        group_count_ -= static_cast<int>(args.old_count);
                        reset_group_tracking();
                        updated.raise({.kind = source_update_kind::delete_sections,
                                       .section = args.old_starting_index,
                                       .count = args.old_count});
                        break;

                    case maui::core::collection_changed_action::replace:
                        if (args.new_count != args.old_count)
                        {
                            reload(); // defensive — see the header deviation note
                            break;
                        }
                        reset_group_tracking();
                        updated.raise({.kind = source_update_kind::reload_sections,
                                       .section = args.new_starting_index,
                                       .count = args.new_count});
                        break;

                    case maui::core::collection_changed_action::move:
                        reset_group_tracking();
                        if (args.new_count == 1)
                        {
                            updated.raise({.kind = source_update_kind::move_section,
                                           .section = args.old_starting_index,
                                           .count = 1,
                                           .move_to = {args.new_starting_index, -1}});
                            break;
                        }
                        // The literal C# multi-move reload window (header deviation note).
                        updated.raise({.kind = source_update_kind::reload_sections,
                                       .section = std::min(args.old_starting_index, args.new_starting_index),
                                       .count = static_cast<std::size_t>(
                                                    std::max(args.old_starting_index, args.new_starting_index)) +
                                                args.new_count});
                        break;

                    case maui::core::collection_changed_action::reset:
                        reload();
                        break;
                }
            }

            void reload()
            {
                group_count_ = static_cast<int>(source_->count());
                reset_group_tracking();
                updated.raise({.kind = source_update_kind::reload_data});
            }

            std::shared_ptr<i_item_collection> source_;
            int group_count_ = 0;
            bool observe_changes_ = true;
            std::vector<child_entry> children_;
            maui::core::scoped_connection subscription_; // after source_ (§8)
        };
    } // namespace

    std::shared_ptr<i_items_view_source> items_source_factory::create(std::shared_ptr<i_item_collection> source)
    {
        if (!source)
        {
            return std::make_shared<empty_source>();
        }
        if (source->changed() != nullptr)
        {
            return std::make_shared<observable_items_source>(std::move(source));
        }
        return std::make_shared<list_source>(std::move(source));
    }

    std::shared_ptr<i_items_view_source> items_source_factory::create_grouped(std::shared_ptr<i_item_collection> source)
    {
        if (!source)
        {
            return std::make_shared<empty_source>();
        }
        return std::make_shared<observable_grouped_source>(std::move(source));
    }
} // namespace maui::controls
