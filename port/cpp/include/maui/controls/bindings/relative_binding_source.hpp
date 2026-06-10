#pragma once
// maui::controls::relative_binding_source  <=  Microsoft.Maui.Controls.RelativeBindingSource
//                                              (+ RelativeBindingSourceMode)
//
// A binding source resolved RELATIVE to the target element: the element itself (Self), the element
// that applied the enclosing control template (TemplatedParent), or an ancestor — the element chain
// walked through Element.Parent — matched by type (FindAncestor) or by binding-context type
// (FindAncestorBindingContext), at a 1-based AncestorLevel.
//
// Port notes (no-reflection consequences, PROFILE §6):
//   - C#'s `Type AncestorType` + IsAssignableFrom becomes a typed factory: find_ancestor<TStack>()
//     captures a dynamic_cast probe for elements, find_ancestor_binding_context<TVm>() a probe that
//     dynamic_casts an object context (subclasses match) and falls back to an exact type_tag match
//     for plain contexts. The C# ctor's "FindAncestor requires AncestorType" InvalidOperationException
//     is unrepresentable — the factories enforce it at compile time.
//   - TemplatedParent is a documented STUB until control templates land: it resolves to no source
//     (the binding applies its FallbackValue / target default). See binding.cpp.
//   - The resolution + ancestry re-subscription ORCHESTRATION lives on the owning binding (C# keeps
//     that state on BindingExpression too — this type stays the immutable descriptor; Self and
//     TemplatedParent are shared singletons exactly like the C# static properties).

#include <cstdint>
#include <functional>
#include <memory>
#include <utility>

#include "maui/core/bindable_object.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    class element;

    enum class relative_binding_source_mode : std::uint8_t
    {
        // 0 reserved (C# keeps it for a possible PreviousData)
        templated_parent = 1,
        self = 2,
        find_ancestor = 3,
        find_ancestor_binding_context = 4,
    };

    class relative_binding_source
    {
    public:
        using ancestor_predicate = std::function<bool(element&)>;
        using context_predicate = std::function<bool(const maui::core::bindable_object::binding_context_box&)>;

        // RelativeBindingSource.Self — binds to the element the binding is set on.
        [[nodiscard]] static std::shared_ptr<relative_binding_source> self();
        // RelativeBindingSource.TemplatedParent — STUB until control templates are ported (see above).
        [[nodiscard]] static std::shared_ptr<relative_binding_source> templated_parent();

        // Binds to the nearest (level-th) ancestor element that is a TAncestor.
        template <class TAncestor>
        [[nodiscard]] static std::shared_ptr<relative_binding_source> find_ancestor(int ancestor_level = 1)
        {
            return std::make_shared<relative_binding_source>(
                relative_binding_source_mode::find_ancestor,
                ancestor_predicate{[](element& candidate) { return dynamic_cast<TAncestor*>(&candidate) != nullptr; }},
                context_predicate{}, ancestor_level);
        }

        // Binds to the binding CONTEXT of the nearest (level-th) ancestor whose context is a TContext.
        template <class TContext>
        [[nodiscard]] static std::shared_ptr<relative_binding_source> find_ancestor_binding_context(
            int ancestor_level = 1)
        {
            return std::make_shared<relative_binding_source>(
                relative_binding_source_mode::find_ancestor_binding_context, ancestor_predicate{},
                context_predicate{[](const maui::core::bindable_object::binding_context_box& box) {
                    if (box.object && dynamic_cast<TContext*>(box.object.get()) != nullptr)
                    {
                        return true; // an object context matches with subclassing
                    }
                    return box.type == maui::core::type_tag::of<TContext>(); // plain contexts: exact type
                }},
                ancestor_level);
        }

        // Use the typed factories above; public only for make_shared.
        relative_binding_source(relative_binding_source_mode mode, ancestor_predicate matches_ancestor,
                                context_predicate matches_context, int ancestor_level)
            : mode_(mode), matches_ancestor_(std::move(matches_ancestor)), matches_context_(std::move(matches_context)),
              ancestor_level_(ancestor_level)
        {
        }

        [[nodiscard]] relative_binding_source_mode mode() const
        {
            return mode_;
        }
        [[nodiscard]] int ancestor_level() const
        {
            return ancestor_level_;
        }
        [[nodiscard]] bool matches_ancestor(element& candidate) const
        {
            return matches_ancestor_ && matches_ancestor_(candidate);
        }
        [[nodiscard]] bool matches_context(const maui::core::bindable_object::binding_context_box& box) const
        {
            return matches_context_ && matches_context_(box);
        }

    private:
        relative_binding_source_mode mode_;
        ancestor_predicate matches_ancestor_;
        context_predicate matches_context_;
        int ancestor_level_ = 1;
    };
} // namespace maui::controls
