// maui::xaml — DataTemplate body inflation (W4). See xaml_template_inflater.hpp for the design, the
// C# mapping (ApplyPropertiesVisitor.SetTemplate), and the ownership / lifetime contract.
#include "maui/xaml/xaml_template_inflater.hpp"

#include <any>
#include <memory>
#include <utility>
#include <vector>

#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp" // scoped_connection
#include "maui/xaml/markup_extensions.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_object_graph.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    template_inflater template_inflater::from(const hydration_context& load_context)
    {
        return template_inflater{.types = &load_context.type_registry(),
                                 .properties = &load_context.property_registry(),
                                 .converters = &load_context.converter_registry(),
                                 .extensions = &load_context.extension_registry(),
                                 .application = load_context.application,
                                 .exception_handler = load_context.handler()};
    }

    namespace
    {
        // One realized stamp's owners (the per-stamp analog of xaml_load_result): the inflated control
        // graph plus the non-control keep-alive list, the load's event subscriptions, and the stamp's
        // root name scope. The consumer co-owns this bundle through the aliasing shared_ptr returned by
        // inflate_template_body; dropping the last reference tears the stamp down. Member ORDER matters
        // for teardown: subscriptions are declared LAST so they disconnect FIRST, before the graph tears
        // the targets down (mirrors xaml_load_result + hydration_context's own ordering note).
        struct stamped_tree
        {
            xaml_object_graph graph;
            std::vector<std::shared_ptr<void>> keep_alive;
            std::shared_ptr<void> root_scope; // the stamp's name scope (kept alive for x:Reference)
            std::vector<maui::core::scoped_connection> subscriptions;
        };
    } // namespace

    std::shared_ptr<maui::core::bindable_object> inflate_template_body(
        const std::shared_ptr<i_xaml_node>& captured_body, const template_inflater& env)
    {
        if (captured_body == nullptr || env.types == nullptr || env.properties == nullptr ||
            env.converters == nullptr || env.extensions == nullptr)
        {
            return nullptr;
        }

        // (a) Clone the captured body AGAIN per call: each stamp is an INDEPENDENT subtree and the
        //     captured master stays pristine for the next item (per-stamp independence — risk #4: a
        //     fresh clone + a fresh context gives every stamp its own name scope, so x:Name uniqueness
        //     holds across many stamps; the master is never mutated by a stamp's visitors).
        const std::shared_ptr<i_xaml_node> clone = captured_body->clone();

        // (b) A CHILD hydration_context for this stamp: same registries + same doNotThrow policy +
        //     same application as the originating load (snapshotted by value into `env`, so the stamp
        //     does NOT depend on a live parent context — CollectionView items, hence stamps, arrive
        //     after the load returns; risk #2). parent_context stays null: {StaticResource} lookups for
        //     resources declared OUTSIDE the template are a documented W4 follow-up (no live parent to
        //     walk); they fail loudly through the registries' normal error channel.
        hydration_context child{*env.types, *env.properties, *env.converters, *env.extensions, env.exception_handler};
        child.application = env.application;

        // (c) Run the shared visitor pipeline on the clone (create-values mints the body root + its
        //     subtree into child.graph(); apply wires properties, {Binding}s, attached props). The body
        //     is an element node, not a root_node, so the create pass mints it directly (no RootNode
        //     special-case) — the inflated root is the clone's created value.
        run_hydration_pipeline(*clone, child);

        // (d) The inflated root = the clone node's created value (C# ctx.Values[cnode]).
        const std::any* created = child.try_get_value(*clone);
        if (created == nullptr)
        {
            return nullptr; // creation failed (under an exception handler) / empty body
        }
        const auto* root_ptr = std::any_cast<std::shared_ptr<maui::core::bindable_object>>(created);
        if (root_ptr == nullptr || *root_ptr == nullptr)
        {
            return nullptr; // the body inflated to a non-control payload — no stamp root
        }
        maui::core::bindable_object* const root_raw = root_ptr->get();

        // (e) OWNERSHIP TRANSFER (the W4 top risk): move the per-stamp owners off the child context and
        //     onto a heap bundle, then return an ALIASING shared_ptr (pointer = the root control, owner
        //     = the bundle). The child context is about to be destroyed; without this, its graph would
        //     tear the freshly inflated tree down on return (use-after-free at the consumer). With it,
        //     the consumer's handle co-owns the whole subtree; the root pointer is one of the graph's
        //     objects (the graph holds its own strong ref), so it stays valid exactly as long as any
        //     handle to the stamp lives.
        auto bundle = std::make_shared<stamped_tree>();
        bundle->graph = std::move(child.graph());
        bundle->keep_alive = std::move(child.kept_alive());
        bundle->subscriptions = std::move(child.subscriptions());
        // The root scope lives on the clone node (element_node::scope_ref); keep the clone tree alive
        // for the stamp's lifetime so x:Reference / find-by-name against the stamp keep resolving (the
        // scope's entries are co-owned by the graph already, but the node tree owns the scope handle).
        bundle->root_scope = clone;

        return {std::move(bundle), root_raw};
    }
} // namespace maui::xaml
