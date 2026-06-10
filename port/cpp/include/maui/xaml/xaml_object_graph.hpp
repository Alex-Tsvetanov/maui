#pragma once
// maui::xaml::xaml_object_graph — the owner of a XAML-loaded element tree (M7 wave 1).
//
// C# counterpart: the HydrationContext.Values dictionary (src/Controls/src/Xaml/XamlLoader.cs +
// CreateValuesVisitor.cs) holds every object a XAML pass instantiates; the GC then keeps the tree
// alive through the parent→child references. The C++ port has no GC and its tree-wiring APIs are
// NON-owning by doctrine (PROFILE §8: content_page::set_content / layout<>::add /
// window::set_content / navigation_page::push all borrow — "the caller owns the child's lifetime"),
// so the loader needs an explicit owner: this graph. The xaml_type_registry's factories return
// shared_ptr<bindable_object>; the loader add()s each created object here and wires the borrowed
// references through the registry's add_child metadata. The graph is thus the SINGLE owner of the
// loaded tree — destroy the graph, destroy the tree.
//
// Teardown is deterministic: objects are released in REVERSE insertion order (and the root handle
// first, since it aliases one of them). Order is a determinism nicety, not a correctness need — every
// parent/child link between controls is a non-owning raw pointer that no control's destructor
// dereferences, so the nodes may die in any order so long as nothing uses the tree afterwards.
//
// Movable (a loader returns the graph by value), non-copyable (one owner per loaded tree).

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "maui/core/bindable_object.hpp"

namespace maui::xaml
{
    class xaml_object_graph
    {
    public:
        xaml_object_graph() = default;
        xaml_object_graph(const xaml_object_graph&) = delete;
        xaml_object_graph& operator=(const xaml_object_graph&) = delete;
        xaml_object_graph(xaml_object_graph&&) = default;
        xaml_object_graph& operator=(xaml_object_graph&&) = default;
        ~xaml_object_graph()
        {
            root_.reset(); // the root aliases an owned object; drop the extra handle first
            while (!objects_.empty())
            {
                objects_.pop_back(); // reverse insertion order — deterministic teardown
            }
        }

        // Adopt one loader-created object (insertion order is preserved for teardown).
        void add(std::shared_ptr<maui::core::bindable_object> object)
        {
            objects_.push_back(std::move(object));
        }

        [[nodiscard]] std::size_t size() const
        {
            return objects_.size();
        }
        [[nodiscard]] bool empty() const
        {
            return objects_.empty();
        }

        // The XAML document's root element (XamlLoader's `view`); typically also held in objects_.
        void set_root(std::shared_ptr<maui::core::bindable_object> root)
        {
            root_ = std::move(root);
        }
        [[nodiscard]] const std::shared_ptr<maui::core::bindable_object>& root() const
        {
            return root_;
        }
        // The root as the concrete control type, or nullptr when unset / a different type.
        template <class TControl> [[nodiscard]] std::shared_ptr<TControl> root_as() const
        {
            return std::dynamic_pointer_cast<TControl>(root_);
        }

    private:
        std::vector<std::shared_ptr<maui::core::bindable_object>> objects_;
        std::shared_ptr<maui::core::bindable_object> root_;
    };
} // namespace maui::xaml
