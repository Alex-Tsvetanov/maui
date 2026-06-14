#pragma once
// maui::controls::shell_navigation_request  <=  Microsoft.Maui.Controls.ShellNavigationRequest
// (internal)
//
// A resolved navigation: the request_definition plus what to do with the section's stack
// (replace vs push), and the query/fragment of the original URI. Ported from
// ShellNavigationRequest.cs (WhatToDoWithTheStack lives on shell_uri_handler as
// stack_request_kind).

#include <string>
#include <utility>

#include "maui/controls/shell/request_definition.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"

namespace maui::controls
{
    class shell_navigation_request
    {
    public:
        shell_navigation_request(request_definition definition, shell_uri_handler::stack_request_kind stack_request,
                                 std::string query, std::string fragment)
            : definition_(std::move(definition)), stack_request_(stack_request), query_(std::move(query)),
              fragment_(std::move(fragment))
        {
        }

        [[nodiscard]] const request_definition& definition() const
        {
            return definition_;
        }
        [[nodiscard]] shell_uri_handler::stack_request_kind stack_request() const
        {
            return stack_request_;
        }
        [[nodiscard]] const std::string& query() const
        {
            return query_;
        }
        [[nodiscard]] const std::string& fragment() const
        {
            return fragment_;
        }

    private:
        request_definition definition_;
        shell_uri_handler::stack_request_kind stack_request_;
        std::string query_;
        std::string fragment_;
    };
} // namespace maui::controls
