// maui::controls::shell_navigation_manager — ported from ShellNavigationManager.cs. See the header.
//
// Suspension lifetime note: a deferral's continuation captures `this` (the manager lives inside the
// shell). Completing a deferral after the shell was destroyed is undefined — complete (or abandon)
// deferrals while the shell is alive, exactly as every test does. (C# leans on the GC here.)

#include "maui/controls/shell/shell_navigation_manager.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/i_query_attributable.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_navigated_event_args.hpp"
#include "maui/controls/shell/shell_navigating_event_args.hpp"
#include "maui/controls/shell/shell_navigation_source.hpp"
#include "maui/controls/shell/shell_navigation_state.hpp"
#include "maui/controls/shell/shell_route_parameters.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"

namespace maui::controls
{
    namespace
    {
        std::string join(const std::vector<std::string>& segments)
        {
            std::string joined;
            bool first = true;
            for (const std::string& segment : segments)
            {
                if (!first)
                {
                    joined += "/";
                }
                first = false;
                joined += segment;
            }
            return joined;
        }

        // The reflection-free ApplyQueryAttributes delivery (see i_query_attributable.hpp).
        void deliver_query_attributes(element& target, const shell_route_parameters& query)
        {
            if (auto* attributable = dynamic_cast<i_query_attributable*>(&target))
            {
                attributable->apply_query_attributes(query);
            }
        }
    } // namespace

    void shell_navigation_manager::go_to(const shell_navigation_state& state, std::optional<bool> animate,
                                         bool enable_relative_shell_routes,
                                         std::shared_ptr<shell_navigating_event_args> deferred_args,
                                         std::optional<shell_route_parameters> parameters,
                                         std::optional<bool> can_cancel)
    {
        shell_navigation_parameters navigation_parameters;
        navigation_parameters.target_state = state;
        navigation_parameters.animated = animate;
        navigation_parameters.enable_relative_shell_routes = enable_relative_shell_routes;
        navigation_parameters.deferred_args = std::move(deferred_args);
        navigation_parameters.parameters = std::move(parameters);
        navigation_parameters.can_cancel = can_cancel;
        go_to(std::move(navigation_parameters));
    }

    void shell_navigation_manager::go_to(shell_navigation_parameters navigation_parameters)
    {
        go_to(std::move(navigation_parameters), nullptr);
    }

    void shell_navigation_manager::go_to(shell_navigation_parameters navigation_parameters,
                                         std::shared_ptr<shell_navigation_request> navigation_request)
    {
        if (navigation_parameters.page_pushing != nullptr && navigation_request == nullptr)
        {
            routing::register_implicit_page_route(*navigation_parameters.page_pushing);
        }

        const shell_navigation_state state =
            navigation_parameters.target_state
                ? *navigation_parameters.target_state
                : shell_navigation_state{routing::get_route(*navigation_parameters.page_pushing), false};

        if (navigation_request == nullptr)
        {
            navigation_request = shell_uri_handler::get_navigation_request(
                shell_, state.full_location_uri(), navigation_parameters.enable_relative_shell_routes, true,
                &navigation_parameters);
        }

        const bool is_relative_popping = shell_uri_handler::is_target_relative_pop(navigation_parameters);
        shell_route_parameters parameters =
            navigation_parameters.parameters ? *navigation_parameters.parameters : shell_route_parameters{};

        const shell_navigation_source source =
            calculate_navigation_source(shell_, shell_.current_state(), *navigation_request);

        // Deferred args mean the user already chose to proceed with a delayed navigation.
        if (navigation_parameters.deferred_args == nullptr)
        {
            const bool can_cancel = navigation_parameters.can_cancel ? *navigation_parameters.can_cancel
                                                                     : shell_.current_state() != nullptr;
            const std::shared_ptr<shell_navigating_event_args> navigating_args =
                propose_navigation(source, state, can_cancel, navigation_parameters.animated.value_or(true));

            if (navigating_args != nullptr)
            {
                if (navigating_args->deferral_taken() && !navigating_args->deferral_completed())
                {
                    // SUSPEND: the rest of this navigation runs when the last deferral completes
                    // (C#'s `accept = await navigatingArgs.DeferredTask`).
                    navigating_args->register_deferral_completed_callback(
                        [this, navigation_parameters = std::move(navigation_parameters),
                         navigation_request = std::move(navigation_request), parameters = std::move(parameters), source,
                         is_relative_popping, navigating_args]() mutable {
                            if (navigating_args->cancelled())
                            {
                                return;
                            }
                            complete_go_to(navigation_parameters, navigation_request, std::move(parameters), source,
                                           is_relative_popping);
                        });
                    return;
                }
                if (navigating_args->cancelled())
                {
                    return;
                }
            }
        }

        complete_go_to(navigation_parameters, navigation_request, std::move(parameters), source, is_relative_popping);
    }

    void shell_navigation_manager::complete_go_to(const shell_navigation_parameters& navigation_parameters,
                                                  const std::shared_ptr<shell_navigation_request>& navigation_request,
                                                  shell_route_parameters parameters, shell_navigation_source source,
                                                  bool is_relative_popping)
    {
        routing::register_implicit_page_routes(shell_);

        accumulate_navigated_events_ = true;

        parameters.set_query_string_parameters(navigation_request->query());
        apply_query_attributes(shell_, parameters, false, false);

        shell_item* item = navigation_request->definition().item();
        shell_section* section = navigation_request->definition().section();
        shell_content* content = navigation_request->definition().content();
        const shell_section* current_shell_section = shell_.current_section();
        shell_section* next_active_section =
            section != nullptr ? section : (item != nullptr ? item->current_item() : nullptr);
        const std::vector<std::string>& global_routes = navigation_request->definition().global_routes();

        // (C#'s modalStackPreBuilt flag fed a modal-pop branch the port doesn't model — no modal stack.)

        // Replacing the whole stack with global routes: build the stack BEFORE making the section
        // visible (a different target section transitions un-animated, like C#).
        if (!global_routes.empty() && next_active_section != nullptr &&
            navigation_request->stack_request() == shell_uri_handler::stack_request_kind::replace_it)
        {
            const std::optional<bool> is_animated = next_active_section != current_shell_section
                                                        ? std::optional<bool>{false}
                                                        : navigation_parameters.animated;
            next_active_section->go_to(*navigation_request, parameters, is_animated, is_relative_popping);
        }

        if (item != nullptr)
        {
            apply_query_attributes(*item, parameters, section == nullptr, false);

            if (section != nullptr && content != nullptr)
            {
                apply_query_attributes(*content, parameters, global_routes.empty(), is_relative_popping);
                if (section->current_item() != content)
                {
                    section->set_current_item(content);
                }
            }

            if (section != nullptr)
            {
                apply_query_attributes(*section, parameters, content == nullptr, false);
                if (item->current_item() != section)
                {
                    item->set_current_item(section);
                }
            }

            if (shell_.current_item() != item)
            {
                shell_.set_current_item_core(shell_.owner_of(item));
            }

            // (the "pop the modal stack when navigating to a new shell element" branch is gone —
            // the port's shell has no modal stack)

            if (!global_routes.empty() &&
                navigation_request->stack_request() != shell_uri_handler::stack_request_kind::replace_it)
            {
                if (shell_section* target = shell_.current_section())
                {
                    target->go_to(*navigation_request, parameters, navigation_parameters.animated, is_relative_popping);
                }
            }
            else if (global_routes.empty() &&
                     navigation_request->stack_request() == shell_uri_handler::stack_request_kind::replace_it &&
                     next_active_section != nullptr && next_active_section->stack().size() > 1)
            {
                if (shell_section* target = shell_.current_section())
                {
                    target->go_to(*navigation_request, parameters, navigation_parameters.animated, is_relative_popping);
                }
            }
        }
        else if (shell_section* target = shell_.current_section())
        {
            target->go_to(*navigation_request, parameters, navigation_parameters.animated, is_relative_popping);
        }

        shell_.update_current_state(source);
        accumulate_navigated_events_ = false;

        // null when no navigation actually took place
        if (accumulated_event_)
        {
            const shell_navigated_event_args accumulated = *accumulated_event_;
            accumulated_event_.reset();
            handle_navigated(accumulated);
        }
    }

    bool shell_navigation_manager::propose_navigation_outside_goto(shell_navigation_source source, shell_item* item,
                                                                   shell_section* section, shell_content* content,
                                                                   const std::vector<content_page*>* stack,
                                                                   bool can_cancel, bool is_animated)
    {
        if (accumulate_navigated_events_)
        {
            return true;
        }

        const shell_navigation_state proposed_state =
            get_navigation_state(item, section, content, stack, /*modal_stack=*/nullptr);
        const std::shared_ptr<shell_navigating_event_args> nav_args =
            propose_navigation(source, proposed_state, can_cancel, is_animated);

        if (nav_args->deferral_count() > 0)
        {
            nav_args->register_deferral_completed_callback([this, nav_args] {
                if (nav_args->cancelled())
                {
                    return;
                }
                go_to(nav_args->target(), nav_args->animate(), false, nav_args);
            });
        }

        return !nav_args->navigation_delayed_or_cancelled();
    }

    std::shared_ptr<shell_navigating_event_args> shell_navigation_manager::propose_navigation(
        shell_navigation_source source, const shell_navigation_state& proposed_state, bool can_cancel, bool is_animated)
    {
        if (accumulate_navigated_events_)
        {
            return nullptr;
        }

        std::optional<shell_navigation_state> current;
        if (const shell_navigation_state* current_state = shell_.current_state())
        {
            current = *current_state;
        }
        auto nav_args =
            std::make_shared<shell_navigating_event_args>(std::move(current), proposed_state, source, can_cancel);
        nav_args->set_animate(is_animated);
        handle_navigating(*nav_args);
        return nav_args;
    }

    void shell_navigation_manager::handle_navigating(shell_navigating_event_args& args) const
    {
        if (!args.deferred_event_args())
        {
            navigating.raise(args);
        }
    }

    void shell_navigation_manager::handle_navigated(const shell_navigated_event_args& args)
    {
        // DEVIATION (documented): C# first waits for the shell to be attached to a Window with a live
        // CurrentPage; the headless shell has no window attachment this unit, so only the
        // OnAppearing gate below remains (it still defers Navigated until a template-created page
        // exists, because shell_content::send_appearing won't fire without a page).
        if (accumulate_navigated_events_)
        {
            if (!accumulated_event_)
            {
                accumulated_event_ = args;
            }
            return;
        }

        accumulated_event_.reset();
        base_shell_item* base_item = shell_.current_content();

        if (base_item != nullptr)
        {
            // Capture the args behind a shared_ptr so the queued closure is nothrow-copyable (a deferred
            // OnAppearing copies it into std::function); the named member below carries the throwing work.
            auto pinned = std::make_shared<shell_navigated_event_args>(args);
            base_item->on_appearing([this, pinned] { fire_navigated_events(*pinned); });
        }
        else
        {
            fire_navigated_events(args);
        }
    }

    void shell_navigation_manager::fire_navigated_events(const shell_navigated_event_args& args)
    {
        navigated.raise(args);
        // reset the active page route tree
        routing::clear_implicit_page_routes();
        routing::register_implicit_page_routes(shell_);
    }

    void shell_navigation_manager::apply_query_attributes(element& target, shell_route_parameters& query,
                                                          bool is_last_item, bool is_popping)
    {
        std::string prefix;
        if (!is_last_item)
        {
            const std::string route = routing::get_route(target);
            if (route.empty() || routing::is_implicit(route))
            {
                return;
            }
            prefix = route + ".";
        }

        // If the last item is implicitly wrapped, unwrap down to the actual content / page.
        element* unwrapped = &target;
        if (is_last_item)
        {
            if (auto* item = dynamic_cast<shell_item*>(unwrapped))
            {
                const std::vector<shell_section*> sections = item->visible_items();
                if (!sections.empty())
                {
                    unwrapped = sections.front();
                }
            }
            if (auto* section = dynamic_cast<shell_section*>(unwrapped))
            {
                const std::vector<shell_content*> contents = section->visible_items();
                if (!contents.empty())
                {
                    unwrapped = contents.front();
                }
            }
            if (auto* content = dynamic_cast<shell_content*>(unwrapped))
            {
                if (content->content() != nullptr)
                {
                    unwrapped = content->content();
                }
            }
        }

        auto* base_item = dynamic_cast<base_shell_item*>(unwrapped);
        if (base_item == nullptr)
        {
            base_item = dynamic_cast<base_shell_item*>(unwrapped->logical_parent());
        }

        // filter the query down to the keys with a matching prefix
        const shell_route_parameters filtered_query{query, prefix};

        // DEVIATION (documented): C#'s MergeData re-applies the attributes STORED on the element when
        // popping back; the port keeps the stored set only on shell_content (its query_attributes()),
        // so a popped-to plain page re-receives the incoming dictionary without the historical merge.
        if (auto* shell_content_item = dynamic_cast<shell_content*>(base_item))
        {
            if (!filtered_query.empty() || !is_popping)
            {
                shell_content_item->apply_query_attributes(filtered_query);
            }
        }
        else if (is_last_item)
        {
            if (!query.empty() || !is_popping)
            {
                deliver_query_attributes(*unwrapped, query);
            }
        }
    }

    shell_navigation_source shell_navigation_manager::calculate_navigation_source(
        shell& host, const shell_navigation_state* current, const shell_navigation_request& request)
    {
        if (request.stack_request() == shell_uri_handler::stack_request_kind::push_to_it)
        {
            return shell_navigation_source::push;
        }
        if (current == nullptr)
        {
            return shell_navigation_source::shell_item_changed;
        }

        const shell_uri target_uri =
            shell_uri_handler::convert_to_standard_format(&host, request.definition().full_uri());
        const shell_uri current_uri =
            shell_uri_handler::convert_to_standard_format(&host, current->full_location_uri());

        const std::vector<std::string> target_paths = shell_uri_handler::retrieve_paths(target_uri.path_and_query());
        const std::vector<std::string> current_paths = shell_uri_handler::retrieve_paths(current_uri.path_and_query());

        const std::size_t target_length = target_paths.size();
        const std::size_t current_length = current_paths.size();

        if (target_length < 4 || current_length < 4)
        {
            return shell_navigation_source::unknown;
        }

        if (target_paths[1] != current_paths[1])
        {
            return shell_navigation_source::shell_item_changed;
        }
        if (target_paths[2] != current_paths[2])
        {
            return shell_navigation_source::shell_section_changed;
        }
        if (target_paths[3] != current_paths[3])
        {
            return shell_navigation_source::shell_content_changed;
        }

        if (target_length == current_length)
        {
            return shell_navigation_source::unknown;
        }

        if (target_length < current_length)
        {
            for (std::size_t i = 0; i < target_length; ++i)
            {
                if (target_paths[i] != current_paths[i])
                {
                    break;
                }
                if (i == target_length - 1)
                {
                    if (target_length == 4)
                    {
                        return shell_navigation_source::pop_to_root;
                    }
                    return shell_navigation_source::pop;
                }
            }

            if (target_paths[target_length - 1] == current_paths[current_length - 1])
            {
                return shell_navigation_source::remove;
            }
            if (target_length == 4)
            {
                return shell_navigation_source::pop_to_root;
            }
            return shell_navigation_source::pop;
        }
        if (target_length > current_length)
        {
            for (std::size_t i = 0; i < current_length; ++i)
            {
                if (target_paths[i] != current_paths[i])
                {
                    break;
                }
                if (i == target_length - 1)
                {
                    return shell_navigation_source::push;
                }
            }
        }

        if (target_paths[target_length - 1] == current_paths[current_length - 1])
        {
            return shell_navigation_source::insert;
        }

        return shell_navigation_source::push;
    }

    shell_navigation_state shell_navigation_manager::get_navigation_state(
        shell_item* item, shell_section* section, shell_content* content,
        const std::vector<content_page*>* section_stack, const std::vector<content_page*>* modal_stack)
    {
        (void)modal_stack; // no modal stack in the port (kept for C# call-shape parity)

        std::vector<std::string> route_stack;

        const bool stack_at_root = section_stack == nullptr || section_stack->size() <= 1;
        const bool has_user_defined_route =
            routing::is_user_defined(item) || routing::is_user_defined(section) || routing::is_user_defined(content);

        if (item != nullptr)
        {
            route_stack.push_back(item->route());

            if (section != nullptr)
            {
                route_stack.push_back(section->route());

                if (content != nullptr)
                {
                    route_stack.push_back(content->route());
                }

                if (!stack_at_root)
                {
                    for (std::size_t i = 1; i < section_stack->size(); ++i)
                    {
                        const content_page* page = (*section_stack)[i];
                        const std::vector<std::string> collapsed = shell_uri_handler::collapse_path(
                            routing::get_route(*page), route_stack, has_user_defined_route);
                        route_stack.insert(route_stack.end(), collapsed.begin(), collapsed.end());
                    }
                }
            }
        }

        if (!route_stack.empty())
        {
            route_stack.insert(route_stack.begin(), "/");
        }

        return shell_navigation_state{join(route_stack), true};
    }

    shell_navigation_parameters shell_navigation_manager::get_navigation_parameters(
        shell_item* item, shell_section* section, shell_content* content,
        const std::vector<content_page*>* section_stack, const std::vector<content_page*>* modal_stack)
    {
        shell_navigation_parameters navigation_parameters;
        navigation_parameters.target_state = get_navigation_state(item, section, content, section_stack, modal_stack);
        navigation_parameters.animated = false;
        if (content != nullptr && content->has_query_attributes())
        {
            navigation_parameters.parameters = content->query_attributes();
        }
        return navigation_parameters;
    }

    std::vector<content_page*> shell_navigation_manager::build_flattened_navigation_stack(shell& host)
    {
        const shell_section* section = host.current_section();
        if (section == nullptr)
        {
            return {};
        }
        return section->stack(); // no modal stack to flatten in
    }
} // namespace maui::controls
