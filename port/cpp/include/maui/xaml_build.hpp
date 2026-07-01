#pragma once
// maui::build_page<VM, Xaml>() — COMPILE-TIME XAML hosting (PUBLIC_API_DESIGN.md §6).
//
// The keystone of the compiler-managed (not build-system-managed) XAML path. The developer embeds raw
// markup with #embed and hands it to build_page as a fixed_string non-type template argument:
//
//     namespace {
//         constexpr unsigned char login_bytes[] = {
//             #embed "LoginPage.xaml"
//         };
//         constexpr maui::fixed_string login_xaml{login_bytes};
//     }
//     std::unique_ptr<maui::controls::content_page> login_page(std::unique_ptr<login_view_model> vm) {
//         auto page = maui::build_page<login_view_model, login_xaml>();   // unique_ptr<page_impl<VM>>
//         page->bind_to(std::move(vm));
//         return page;                                                    // upcast to content_page*
//     }
//
// Why this exists alongside the runtime loader and the build-time codegen: the markup travels into the
// translation unit as bytes the COMPILER embedded — there is no external codegen step, so this is the one
// XAML path that cross-compiles to the iOS app bundle (the codegen tool can't run during an iOS build).
//
// HOW IT BUILDS THE TREE: build_page mints a page_impl<VM> (an empty content_page subclass that also owns
// the view-model + the load's keep-alive owners) and hydrates the embedded markup INTO it via the runtime
// xaml_loader (load_into). So the structural surface the runtime loader supports — layouts, controls,
// literal properties — works today on every backend. Two runtime-loader deferrals carry over (both are
// pre-existing M7 gaps, not new): attached properties (Grid.Row/Column) and VM-resolving {Binding}. Until
// those close, a page with a Grid or {Binding} wires that part in code-behind (find<T>(name) + ui::bind).
//
// REFLECTION GATE: with static reflection (maui/xaml/feature.hpp, MAUI_HAS_XAML_REFLECTION) build_page can
// validate every markup binding PATH against VM at compile time and auto-wire it — the "missing member is
// a compile error" guarantee. That toolchain is not yet here (Apple clang 21 has no std::meta), so the
// auto-binder is compiled out and bind_to just adopts the VM; bindings are wired explicitly (still typed,
// still compile-checked, just not name-driven). The seam is in place so enabling it is additive.

#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/event.hpp"
#include "maui/fixed_string.hpp"
#include "maui/xaml/feature.hpp"
#include "maui/xaml/xaml_loader.hpp"
#include "maui/xaml/xaml_runtime_bindings.hpp"

namespace maui
{
    // A view-model placeholder for purely structural pages (no bindings, no commands) — e.g. a static
    // layout: `build_page<no_view_model, xaml>()`.
    struct no_view_model
    {
    };

    // page_impl<VM> — a content_page that additionally owns the bound view-model and the load's keep-alive
    // owners (the hydrated subtree, x:Name scope, {AppThemeBinding} subscriptions). The whole page — view
    // tree, view-model, and subscriptions — has one lifetime: destroying the page_impl tears it all down
    // in declaration order (subscriptions first, then the view-model, then the page's own controls).
    template <class VM> class page_impl : public maui::controls::content_page
    {
    public:
        page_impl() = default;

        // Adopt the view-model as XAML's BindingContext. Returns *this so a caller can chain.
        //
        // If VM is a bindable_object, this sets the page's BindingContext to it — which inherits down the
        // tree and RE-EVALUATES the {Binding}s the loader attached during hydration, so a page whose markup
        // is "{Binding Foo}" becomes live with NO code-behind and NO reflection (the path resolves against
        // the registered property "Foo" on the VM). The owning shared_ptr is held by the page's own
        // BindingContext (part of the content_page base, destroyed LAST), so the VM outlives the tree that
        // references it — teardown-safe. A non-bindable_object VM is simply stored for code-behind to use.
        page_impl& bind_to(std::unique_ptr<VM> view_model)
        {
            auto shared = std::shared_ptr<VM>(std::move(view_model));
            if constexpr (std::is_base_of_v<maui::core::bindable_object, VM>)
            {
                this->set_binding_context(shared);
            }
            view_model_ = std::move(shared);
#if MAUI_HAS_XAML_REFLECTION
            auto_wire_bindings(); // reflection path: resolve markup {Binding Path} against VM members by name
#endif
            return *this;
        }

        [[nodiscard]] VM* view_model() const noexcept
        {
            return view_model_.get();
        }

        // x:Name lookup for code-behind (FindByName over the load's root scope).
        template <class TControl> [[nodiscard]] std::shared_ptr<TControl> find(std::string_view name) const
        {
            return load_result_.template find_by_name<TControl>(name);
        }

        // Park a code-behind event subscription on the page so it lives exactly as long as the page does.
        // This is what lets the page-construction function (the .xaml.cpp factory) wire button.clicked /
        // property.changed and then RETURN the page by value: the tokens travel with it instead of needing a
        // member on the caller. retained_tokens_ is the LAST member, so it is destroyed FIRST — every
        // subscription disconnects while its control and the view-model are still alive.
        void retain(maui::core::scoped_connection token)
        {
            retained_tokens_.push_back(std::move(token));
        }

        // The owners the load produced (keep-alive + subscriptions). Public so build_page can populate it;
        // declared LAST so it (and the subscriptions it carries) is destroyed FIRST.
        maui::xaml::xaml_load_result load_result_;

    private:
#if MAUI_HAS_XAML_REFLECTION
        void auto_wire_bindings(); // defined only where reflection exists (provided by the reflection unit)
#endif
        std::shared_ptr<VM> view_model_;
        // Declared LAST → destroyed FIRST: code-behind subscriptions disconnect before the controls (in
        // load_result_) and the view-model they reference are torn down.
        std::vector<maui::core::scoped_connection> retained_tokens_;
    };

    // build_page<VM, Xaml>(): hydrate the embedded markup into a fresh page_impl<VM>. `Xaml` is the markup
    // bytes carried as a class-type NTTP. options forwards the loader knobs (application pointer, registries).
    template <class VM, fixed_string Xaml>
    [[nodiscard]] std::unique_ptr<page_impl<VM>> build_page(const maui::xaml::xaml_load_options& options = {})
    {
        static_assert(MAUI_HAS_BUILD_PAGE,
                      "build_page<VM, Xaml> needs class-type NTTPs to carry the markup bytes; this toolchain "
                      "lacks them. Use the runtime xaml_loader or the build-time codegen path instead. (The "
                      "bytes may come from #embed OR a byte-array literal — build_page needs neither #embed "
                      "nor reflection, only class-type NTTP + the runtime loader.)");
        // Ensure markup {Binding}s have a real applier (idempotent, process-wide) so a fully markup-bound
        // page resolves once bind_to supplies the BindingContext. No-op for binding-free markup.
        maui::xaml::register_runtime_bindings();
        auto page = std::make_unique<page_impl<VM>>();
        page->load_result_ = maui::xaml::xaml_loader::load_into(*page, Xaml.view(), options);
        return page;
    }
} // namespace maui
