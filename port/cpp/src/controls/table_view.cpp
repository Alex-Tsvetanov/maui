// maui::controls::table_view — the Root/Model wiring, RowHeight/HasUnevenRows descriptors, the nested
// table_section_model, and the cell-parenting machinery. See table_view.hpp; ported from
// src/Controls/src/Core/TableView/TableView.cs (+ its nested TableSectionModel). Self-registers the
// table_view_handler at the bottom.

#include "maui/controls/table_view.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/observable_collection.hpp"
#include "maui/controls/table_intent.hpp"
#include "maui/controls/table_model.hpp"
#include "maui/controls/table_root.hpp"
#include "maui/controls/table_section.hpp"
#include "maui/controls/table_view_handler.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    namespace
    {
        // TableView.TableSectionModel: the concrete table_model reading the OWNING table_view's current
        // table_root live (so a Root swap needs no model mutation — no downcast back to this type). GetCell
        // returns the cell at [section, row]; OnRowSelected taps the cell (TableSectionModel.OnRowSelected
        // → cell.OnTapped()).
        class table_section_model : public table_model
        {
        public:
            explicit table_section_model(const table_view& owner) : owner_(&owner)
            {
            }

            [[nodiscard]] std::shared_ptr<cell> get_cell(int section, int row) const override
            {
                const table_section* const section_ref = section_at(section);
                if (section_ref == nullptr || row < 0)
                {
                    return nullptr;
                }
                const auto row_index = static_cast<std::size_t>(row);
                return row_index < section_ref->count() ? section_ref->at(row_index) : nullptr;
            }

            [[nodiscard]] int get_row_count(int section) const override
            {
                const table_section* const section_ref = section_at(section);
                return section_ref == nullptr ? 0 : static_cast<int>(section_ref->count());
            }

            [[nodiscard]] int get_section_count() const override
            {
                const auto& root = owner_->root();
                return root == nullptr ? 0 : static_cast<int>(root->count());
            }

            [[nodiscard]] std::string get_section_title(int section) const override
            {
                const table_section* const section_ref = section_at(section);
                return section_ref == nullptr ? std::string{} : section_ref->title();
            }

            [[nodiscard]] maui::graphics::color get_section_text_color(int section) const override
            {
                const table_section* const section_ref = section_at(section);
                return section_ref == nullptr ? maui::graphics::color{} : section_ref->text_color();
            }

        protected:
            // TableSectionModel.OnRowSelected: tap the selected cell.
            void on_row_selected(const std::shared_ptr<cell>& item) override
            {
                if (item != nullptr)
                {
                    item->on_tapped();
                }
            }

        private:
            // The section at `index` in the owner's current root, or null (bounds-checked).
            [[nodiscard]] const table_section* section_at(int index) const
            {
                const auto& root = owner_->root();
                if (root == nullptr || index < 0)
                {
                    return nullptr;
                }
                const auto section_index = static_cast<std::size_t>(index);
                return section_index < root->count() ? root->at(section_index).get() : nullptr;
            }

            const table_view* owner_; // non-owning back-ref to the table_view that owns this model
        };
    } // namespace

    const maui::core::bindable_property<int>& table_view::row_height_property()
    {
        static const maui::core::bindable_property<int> descriptor{"row_height", -1};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& table_view::has_uneven_rows_property()
    {
        static const maui::core::bindable_property<bool> descriptor{"has_uneven_rows", false};
        return descriptor;
    }

    table_view::table_view() : table_view(nullptr)
    {
    }

    table_view::table_view(std::shared_ptr<table_root> root)
    {
        this->set_style_target_type<table_view>();
        root_ = root != nullptr ? std::move(root) : std::make_shared<table_root>();
        model_ = std::make_unique<table_section_model>(*this); // reads root_ live (no Root-swap mutation)
        hook_root();
        parent_all_cells();
    }

    // §8: the root subscription tokens are declared AFTER root_/model_, so they destruct FIRST — they
    // disconnect from the root's events before the root (publisher) is destroyed. No explicit teardown.
    table_view::~table_view() = default;

    void table_view::set_root(std::shared_ptr<table_root> value)
    {
        // TableView.Root setter: unhook the old root, swap in the new one (or a fresh empty root), inherit
        // the binding context, parent all cells, hook the new root, then OnModelChanged.
        unhook_root();
        root_ = value != nullptr ? std::move(value) : std::make_shared<table_root>();
        root_->set_inherited_binding_context(raw_binding_context());
        // The model reads root_ live (table_section_model holds a back-ref to this table), so a Root swap
        // needs no model mutation.
        hook_root();
        on_model_changed(); // parents all cells + raises ModelChanged
    }

    void table_view::set_intent(table_intent value)
    {
        if (intent_ == value)
        {
            return;
        }
        on_property_changing("intent");
        intent_ = value;
        on_property_changed("intent");
    }

    void table_view::on_binding_context_changed()
    {
        view::on_binding_context_changed();
        if (root_ != nullptr)
        {
            root_->set_inherited_binding_context(raw_binding_context());
        }
    }

    void table_view::visit_cells(const std::function<void(element&)>& visit) const
    {
        // The Root is not an element; the table parents the Root's cells directly, so flatten to them.
        if (root_ == nullptr)
        {
            return;
        }
        for (const auto& section : root_->sections())
        {
            if (section == nullptr)
            {
                continue;
            }
            for (const auto& row : section->cells())
            {
                if (row != nullptr)
                {
                    visit(*row);
                }
            }
        }
    }

    void table_view::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        visit_cells(visit);
    }

    void table_view::on_property_changed(std::string_view name)
    {
        view::on_property_changed(name);
        // Cell.RenderHeight depends on the parent's RowHeight; a change re-notifies each cell's
        // RenderHeight INPC (Cell.OnParentPropertyChanged on "RowHeight").
        if (name == "row_height")
        {
            visit_cells([](element& child) {
                if (auto* row = dynamic_cast<cell*>(&child))
                {
                    row->notify_render_height_changed();
                }
            });
        }
    }

    void table_view::on_model_changed()
    {
        parent_all_cells();
        model_changed.raise();
    }

    void table_view::parent_all_cells()
    {
        // TableView.OnModelChanged / Root setter: cell.Parent = this for every cell in the root. Uses the
        // non-virtual visit_cells (safe from the constructor — no virtual dispatch during construction).
        visit_cells([this](element& child) { this->attach_logical_child(child); });
    }

    void table_view::parent_added_cells(const collection_changed_args<std::shared_ptr<cell>>& args)
    {
        for (const auto& added : args.new_items)
        {
            if (added != nullptr)
            {
                attach_logical_child(*added);
            }
        }
    }

    void table_view::hook_root()
    {
        if (root_ == nullptr)
        {
            return;
        }
        // TableView.OnSectionCollectionChanged: parent the newly-added cells + OnModelChanged.
        section_changed_token_ = maui::core::connect_scoped(root_->section_collection_changed,
                                                            [this](const table_root::section_change& change) {
                                                                parent_added_cells(change.args);
                                                                on_model_changed();
                                                            });
        // TableView.OnTableModelRootPropertyChanged: a section Title change (bubbled by the root) re-runs
        // OnModelChanged.
        root_property_token_ = maui::core::connect_scoped(root_->property_changed, [this](std::string_view name) {
            if (name == "title")
            {
                on_model_changed();
            }
        });
    }

    void table_view::unhook_root()
    {
        section_changed_token_.reset();
        root_property_token_.reset();
    }
} // namespace maui::controls

// Self-register the default handler (opt-in, PROFILE §6). This TU is always linked (the descriptors
// above are referenced by every user of the control), so the registrar runs.
MAUI_REGISTER_HANDLER(maui::controls::table_view, maui::controls::table_view_handler)
