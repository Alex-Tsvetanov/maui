#pragma once
// maui::controls::request_definition  <=  Microsoft.Maui.Controls.RequestDefinition (internal)
//
// The settled outcome of a route resolution: the winning shell_item / shell_section / shell_content
// (the section/content falling back to the item's current chain), the global route segments to
// push, and the full standard-format URI. Ported from RequestDefinition.cs.

#include <string>
#include <vector>

#include "maui/controls/shell/shell_uri.hpp"

namespace maui::controls
{
    class shell;
    class shell_item;
    class shell_section;
    class shell_content;
    class route_request_builder;

    class request_definition
    {
    public:
        request_definition(const route_request_builder& winning_route, shell& host);

        [[nodiscard]] const shell_uri& full_uri() const
        {
            return full_uri_;
        }
        [[nodiscard]] shell_item* item() const
        {
            return item_;
        }
        [[nodiscard]] shell_section* section() const
        {
            return section_;
        }
        [[nodiscard]] shell_content* content() const
        {
            return content_;
        }
        [[nodiscard]] const std::vector<std::string>& global_routes() const
        {
            return global_routes_;
        }

    private:
        // The standard-format URI assembled from the resolved item/section/content + global routes.
        [[nodiscard]] static shell_uri build_full_uri(shell_item* item, shell_section* section, shell_content* content,
                                                      const std::vector<std::string>& global_routes, shell& host);

        // Declaration order matters: section_/content_ initializers read item_/section_, and full_uri_
        // reads them all, so the borrowed nodes + routes are declared (and thus initialized) first.
        shell_item* item_ = nullptr;       // borrowed — the live tree node the request targets
        shell_section* section_ = nullptr; // borrowed
        shell_content* content_ = nullptr; // borrowed
        std::vector<std::string> global_routes_;
        shell_uri full_uri_;
    };
} // namespace maui::controls
