// maui::controls::request_definition — ported from RequestDefinition.cs. See the header.

#include "maui/controls/shell/request_definition.hpp"

#include <string>
#include <vector>

#include "maui/controls/shell/route_request_builder.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"

namespace maui::controls
{
    namespace
    {
        std::string make_uri_string(const std::vector<std::string>& segments)
        {
            std::string joined;
            for (const std::string& segment : segments)
            {
                if (!joined.empty())
                {
                    joined += "/";
                }
                joined += segment;
            }
            if (!segments.empty() && (segments.front().starts_with('/') || segments.front().starts_with('\\')))
            {
                return joined;
            }
            return "//" + joined;
        }
    } // namespace

    request_definition::request_definition(const route_request_builder& winning_route, shell& host)
        : item_(winning_route.item()),
          section_(winning_route.section() != nullptr ? winning_route.section()
                                                      : (item_ != nullptr ? item_->current_item() : nullptr)),
          content_(winning_route.content() != nullptr ? winning_route.content()
                                                      : (section_ != nullptr ? section_->current_item() : nullptr)),
          global_routes_(winning_route.global_route_matches()),
          full_uri_(build_full_uri(item_, section_, content_, global_routes_, host))
    {
    }

    shell_uri request_definition::build_full_uri(shell_item* item, shell_section* section, shell_content* content,
                                                 const std::vector<std::string>& global_routes, shell& host)
    {
        std::vector<std::string> builder;
        if (item != nullptr)
        {
            builder.push_back(item->route());
        }
        if (section != nullptr)
        {
            builder.push_back(section->route());
        }
        if (content != nullptr)
        {
            builder.push_back(content->route());
        }
        builder.insert(builder.end(), global_routes.begin(), global_routes.end());

        const std::string uri_path = make_uri_string(builder);
        const shell_uri uri = shell_uri_handler::create_uri(uri_path);
        return shell_uri_handler::convert_to_standard_format(&host, uri);
    }
} // namespace maui::controls
