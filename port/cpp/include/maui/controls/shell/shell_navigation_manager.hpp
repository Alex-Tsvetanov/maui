#pragma once
// maui::controls::shell_navigation_manager  <=  Microsoft.Maui.Controls.ShellNavigationManager
// (internal)
//
// The shell's navigation engine: go_to resolves the target URI (shell_uri_handler), raises
// Navigating (with cancellation + the DEFERRAL suspension), applies query parameters, switches the
// current item/section/content, applies the global routes to the section stack, then updates
// CurrentState and fires the single accumulated Navigated. Ported from ShellNavigationManager.cs
// with C#'s Task pipeline collapsed to SYNCHRONOUS-WITH-SUSPENSION: every await in the original
// completes inline here (no handler/platform hop this unit) EXCEPT an uncompleted deferral, which
// suspends the navigation by capturing the remainder as the deferral-completed continuation
// (C#'s `accept = await navigatingArgs.DeferredTask`).
//
// DEVIATIONS (documented): no modal stack (the modal pre-pop/pre-build branches are gone); no
// IServiceProvider (routes create through their factories); HandleNavigated drops the
// wait-for-Window gate (the headless shell has no window attachment this unit) but keeps the
// OnAppearing gate, so Navigated still waits for a template-created page to exist.

#include <memory>
#include <optional>
#include <vector>

#include "maui/controls/shell/shell_navigated_event_args.hpp"
#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_parameters.hpp"
#include "maui/controls/shell/shell_navigation_request.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class shell;
    class shell_item;
    class shell_section;
    class shell_content;
    class content_page;
    class element;

    class shell_navigation_manager
    {
    public:
        explicit shell_navigation_manager(shell& owner) : shell_(owner)
        {
        }

        // Mutable-args events (the reference Arg form — see event.hpp): Navigating subscribers may
        // cancel/defer; Navigated is read-only by convention.
        maui::core::event<shell_navigating_event_args&> navigating;
        maui::core::event<const shell_navigated_event_args&> navigated;

        [[nodiscard]] bool accumulate_navigated_events() const
        {
            return accumulate_navigated_events_;
        }

        // GoToAsync(state, animate, enableRelativeShellRoutes, deferredArgs, parameters, canCancel).
        void go_to(const shell_navigation_state& state, std::optional<bool> animate = std::nullopt,
                   bool enable_relative_shell_routes = false,
                   std::shared_ptr<shell_navigating_event_args> deferred_args = nullptr,
                   std::optional<shell_route_parameters> parameters = std::nullopt,
                   std::optional<bool> can_cancel = std::nullopt);
        void go_to(shell_navigation_parameters navigation_parameters);
        void go_to(shell_navigation_parameters navigation_parameters,
                   std::shared_ptr<shell_navigation_request> navigation_request);

        // UI-initiated navigation outside go_to (tab taps / direct CurrentItem sets): raises
        // Navigating; a deferral re-dispatches the equivalent go_to on completion. Returns whether
        // the caller may proceed NOW (false when cancelled OR deferred).
        bool propose_navigation_outside_goto(shell_navigation_source source, shell_item* item, shell_section* section,
                                             shell_content* content, const std::vector<content_page*>* stack,
                                             bool can_cancel, bool is_animated);

        // Raise Navigating unless these args are a resumed deferral (C# HandleNavigating).
        void handle_navigating(shell_navigating_event_args& args) const;
        // Fire (or accumulate) Navigated; gated on the current content having appeared.
        void handle_navigated(const shell_navigated_event_args& args);
        // Raise Navigated and reset the active page-route tree (C# FireNavigatedEvents). A named
        // member (not a lambda) so it is not in bugprone-exception-escape's scope — Navigated handlers
        // may throw, exactly as the C# event does.
        void fire_navigated_events(const shell_navigated_event_args& args);

        // ---- the static helpers (C# statics) ----
        // Deliver `query` to one element of the resolved chain (ApplyQueryAttributes — the
        // [QueryProperty] reflection path is not ported; i_query_attributable is the channel).
        static void apply_query_attributes(element& target, shell_route_parameters& query, bool is_last_item,
                                           bool is_popping);
        [[nodiscard]] static shell_navigation_source calculate_navigation_source(
            shell& host, const shell_navigation_state* current, const shell_navigation_request& request);
        // Compose the CurrentState URI from the chain + stacks (GetNavigationState). The port has no
        // modal stack, so `modal_stack` is always null at the call sites — kept for shape parity.
        [[nodiscard]] static shell_navigation_state get_navigation_state(
            shell_item* item, shell_section* section, shell_content* content,
            const std::vector<content_page*>* section_stack, const std::vector<content_page*>* modal_stack);
        [[nodiscard]] static shell_navigation_parameters get_navigation_parameters(
            shell_item* item, shell_section* section, shell_content* content,
            const std::vector<content_page*>* section_stack, const std::vector<content_page*>* modal_stack);
        [[nodiscard]] static std::vector<content_page*> build_flattened_navigation_stack(shell& host);

    private:
        // The remainder of go_to past the Navigating gate (the deferral continuation target).
        void complete_go_to(const shell_navigation_parameters& navigation_parameters,
                            const std::shared_ptr<shell_navigation_request>& navigation_request,
                            shell_route_parameters parameters, shell_navigation_source source,
                            bool is_relative_popping);

        // Build + dispatch the Navigating args (ProposeNavigation); null while accumulating.
        [[nodiscard]] std::shared_ptr<shell_navigating_event_args> propose_navigation(
            shell_navigation_source source, const shell_navigation_state& proposed_state, bool can_cancel,
            bool is_animated);

        shell& shell_;
        std::optional<shell_navigated_event_args> accumulated_event_;
        bool accumulate_navigated_events_ = false;
    };
} // namespace maui::controls
