#pragma once
// maui::xaml — DataTemplate body inflation (W4)  <=  ApplyPropertiesVisitor.SetTemplate +
// the XamlLoader.Visit pipeline replayed on a cloned template body.
//
// A `<DataTemplate>` body is captured at LOAD time (a deep clone of the `_CreateContent` subtree) and
// re-inflated per ITEM (per data_template::loader call). The factory that does the per-stamp
// inflation is exactly `data_template::loader` (move_only_function<shared_ptr<bindable_object>()>),
// installed by apply_properties_visitor::set_template; this header exposes the two free functions that
// factory needs.
//
//   - run_hydration_pipeline: the fixed XamlLoader.Visit sequence (parent-set -> expand -> prune ->
//     namescope -> create-values -> register-x-names -> fill-RD -> apply -> drain deferred-attached),
//     extracted from xaml_loader.cpp so the top-level load AND each template stamp replay the SAME
//     sequence. (Declared here, defined in xaml_loader.cpp where the visitor pipeline already lives.)
//
//   - inflate_template_body: clone the captured body AGAIN (so the captured master stays pristine for
//     the next item and each stamp is independent), build a CHILD hydration_context, run the pipeline,
//     and return the inflated root — with the per-stamp OWNERS (the object graph, keep-alive list,
//     subscriptions, root scope) transferred onto the returned shared_ptr via an aliasing control
//     block, so the realized item tree outlives the inflate call. This is the port's no-GC stand-in
//     for C# returning ctx.Values[cnode] and letting the GC keep the inflated tree alive.
//
// OWNERSHIP / LIFETIME (the W4 top risk — PROFILE §8):
//   - Per-stamp graph: C#'s child HydrationContext.Values keeps the inflated objects alive until the
//     parent->child references take over; here the tree-wiring APIs are non-owning, so the child
//     context owns the per-stamp graph/keep-alive/subscriptions and WOULD destroy the freshly inflated
//     tree when it returns. inflate_template_body MOVES those owners into a heap `stamped_tree` bundle
//     and returns an ALIASING shared_ptr<bindable_object> (pointer = the root, owner = the bundle), so
//     the consumer (a collection cell / a test) that holds the returned root co-owns the entire
//     subtree; dropping the last reference tears the stamp down (subscriptions first).
//   - Parent context at stamp time: the loader closure must NOT hold the originating hydration_context
//     by reference — for a CollectionView the items (hence the stamps) arrive AFTER the load returns,
//     so the parent context is already gone. set_template therefore captures only `template_inflater`,
//     a value-type snapshot of what a stamp needs (the four registries + application + exception
//     policy), never a hydration_context*. {StaticResource} lookups against resources declared OUTSIDE
//     the template are a documented W4 follow-up (the child context has no parent to walk); they fail
//     loudly via the registries' normal error channel rather than silently.

#include <any>
#include <functional>
#include <memory>

#include "maui/core/bindable_object.hpp"
#include "maui/xaml/hydration_context.hpp"
#include "maui/xaml/xaml_node.hpp"

namespace maui::controls
{
    class application;
} // namespace maui::controls

namespace maui::xaml
{
    class xaml_type_registry;
    class xaml_property_registry;
    class xaml_converter_registry;
    class markup_extension_registry;

    // The XamlLoader.Visit fixed visitor sequence, shared by the top-level load and every template
    // stamp. Runs over the (already parsed/cloned) `node` tree against `context` — `node` is the parsed
    // root_node for a top-level load, or a cloned `_CreateContent` element node for a template stamp
    // (each visitor dispatches on the dynamic node type, exactly like C#'s cnode.Accept). Defined in
    // xaml_loader.cpp (next to the visitor pipeline it factors out).
    void run_hydration_pipeline(i_xaml_node& node, hydration_context& context);

    // A value-type snapshot of the load environment a DataTemplate stamp needs, captured at LOAD time
    // (when set_template builds the loader closure) so the closure does NOT depend on a live parent
    // hydration_context at STAMP time (CollectionView items arrive later — risk #2). The registries +
    // application MUST outlive the loaded tree (the loader's documented contract); they are the
    // process-wide defaults in the common case, or the caller's registries otherwise.
    struct template_inflater
    {
        const xaml_type_registry* types = nullptr;
        const xaml_property_registry* properties = nullptr;
        const xaml_converter_registry* converters = nullptr;
        const markup_extension_registry* extensions = nullptr;
        maui::controls::application* application = nullptr;
        hydration_context::exception_handler exception_handler;

        // Snapshot the environment from the load's context (called by set_template at load time).
        [[nodiscard]] static template_inflater from(const hydration_context& load_context);
    };

    // Inflate ONE fresh subtree from the captured DataTemplate body. `captured_body` is the cloned
    // `_CreateContent` element node retained by the loader closure (kept pristine — this clones it
    // AGAIN per call). Returns the inflated root as an aliasing shared_ptr that OWNS the per-stamp
    // graph/keep-alive/subscriptions/scope (see the ownership note above); returns nullptr when the
    // body inflated to no control (creation failed under an exception handler, or an empty body).
    [[nodiscard]] std::shared_ptr<maui::core::bindable_object> inflate_template_body(
        const std::shared_ptr<i_xaml_node>& captured_body, const template_inflater& env);
} // namespace maui::xaml
