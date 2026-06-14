// maui::controls::route_request_builder — ported 1:1 from RouteRequestBuilder.cs. See the header.
//
// C#'s null string sentinels (NextSegment / GetNextSegment returning null) become the EMPTY string —
// a real segment can never be empty (retrieve_paths drops empty entries), so "" is unambiguous.

#include "maui/controls/shell/route_request_builder.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "maui/controls/shell/base_shell_item.hpp"
#include "maui/controls/shell/routing.hpp"
#include "maui/controls/shell/shell.hpp"
#include "maui/controls/shell/shell_content.hpp"
#include "maui/controls/shell/shell_item.hpp"
#include "maui/controls/shell/shell_section.hpp"
#include "maui/controls/shell/shell_uri_handler.hpp"

namespace maui::controls
{
    route_request_builder::route_request_builder(std::vector<std::string> all_segments)
        : all_segments_(std::move(all_segments))
    {
    }

    route_request_builder::route_request_builder(const std::string& shell_segment, const std::string& user_segment,
                                                 const search_node* node, std::vector<std::string> all_segments)
        : route_request_builder(std::move(all_segments))
    {
        if (node != nullptr)
        {
            add_match(shell_segment, user_segment, *node);
        }
        else
        {
            add_global_route(user_segment, shell_segment); // the C# argument swap is deliberate
        }
    }

    search_node route_request_builder::lowest_child() const
    {
        if (content_ != nullptr)
        {
            return content_;
        }
        if (section_ != nullptr)
        {
            return section_;
        }
        if (item_ != nullptr)
        {
            return item_;
        }
        return shell_;
    }

    void route_request_builder::add_global_route(const std::string& route_name, const std::string& segment)
    {
        global_route_matches_.push_back(route_name);
        for (const std::string& path : shell_uri_handler::retrieve_paths(segment))
        {
            full_segments_.push_back(path);
            matched_segments_.push_back(path);
        }
    }

    bool route_request_builder::add_match(const shell_uri_handler::node_location& node_location)
    {
        const auto add_node = [this](base_shell_item* node, const search_node& as_node) -> bool {
            if (node == nullptr)
            {
                throw std::invalid_argument{"baseShellItem"}; // C# ArgumentNullException
            }
            const std::string node_route = node->route();
            if (routing::is_user_defined(node_route) && node_route != next_segment())
            {
                return false;
            }
            const std::string user_segment = routing::is_user_defined(node_route) ? node_route : std::string{};
            add_match(node_route, user_segment, as_node);
            return true;
        };

        if (item_ == nullptr && !add_node(node_location.item(), search_node{node_location.item()}))
        {
            return false;
        }
        if (section_ == nullptr && !add_node(node_location.section(), search_node{node_location.section()}))
        {
            return false;
        }
        if (content_ == nullptr && !add_node(node_location.content(), search_node{node_location.content()}))
        {
            return false;
        }
        return true;
    }

    void route_request_builder::add_match(const std::string& shell_segment, const std::string& user_segment,
                                          const search_node& node)
    {
        if (const auto* global = std::get_if<global_route_item>(&node))
        {
            if (global->is_finished())
            {
                global_route_matches_.push_back(global->source_route());
            }
        }
        else if (auto* const* host = std::get_if<shell*>(&node))
        {
            if (*host == shell_)
            {
                return;
            }
            shell_ = *host;
        }
        else if (auto* const* item = std::get_if<shell_item*>(&node))
        {
            if (*item == item_)
            {
                return;
            }
            item_ = *item;
        }
        else if (auto* const* section = std::get_if<shell_section*>(&node))
        {
            if (*section == section_)
            {
                return;
            }
            section_ = *section;
            if (item_ == nullptr)
            {
                item_ = section_->parent_item();
                full_segments_.push_back(item_ != nullptr ? item_->route() : std::string{});
            }
        }
        else if (auto* const* content = std::get_if<shell_content*>(&node))
        {
            if (*content == content_)
            {
                return;
            }
            content_ = *content;
            if (section_ == nullptr)
            {
                section_ = dynamic_cast<shell_section*>(content_->logical_parent());
                full_segments_.push_back(section_ != nullptr ? section_->route() : std::string{});
            }
            if (item_ == nullptr && section_ != nullptr)
            {
                item_ = section_->parent_item();
                full_segments_.insert(full_segments_.begin(), item_ != nullptr ? item_->route() : std::string{});
            }
        }

        if (item_ != nullptr)
        {
            if (auto* host = dynamic_cast<shell*>(item_->logical_parent()))
            {
                shell_ = host;
            }
        }

        // shellSegment == userSegment means the implicit route is explicitly part of the request.
        if (routing::is_user_defined(shell_segment) || shell_segment == user_segment || shell_segment == next_segment())
        {
            matched_segments_.push_back(shell_segment);
        }
        full_segments_.push_back(shell_segment);
    }

    std::string route_request_builder::get_next_segment_match(const std::string& match_me) const
    {
        std::vector<std::string> segments_to_match = shell_uri_handler::retrieve_paths(match_me);
        // An absolute route only matches when the already-matched segments are its prefix.
        if (match_me.starts_with('/') || match_me.starts_with('\\'))
        {
            for (const std::string& seg : matched_segments_)
            {
                if (segments_to_match.empty() || segments_to_match.front() != seg)
                {
                    return {};
                }
                segments_to_match.erase(segments_to_match.begin());
            }
        }

        std::vector<std::string> matches;
        std::vector<std::string> current_set = matched_segments_;
        for (const std::string& split : segments_to_match)
        {
            const std::string next = get_next_segment(current_set);
            if (next == split && !next.empty())
            {
                current_set.push_back(split);
                matches.push_back(split);
            }
            else
            {
                return {};
            }
        }

        std::string joined;
        for (const std::string& match : matches)
        {
            if (!joined.empty())
            {
                joined += "/";
            }
            joined += match;
        }
        return joined;
    }

    std::string route_request_builder::get_next_segment(const std::vector<std::string>& matched_segments) const
    {
        const std::size_t next_match = matched_segments.size();
        if (next_match >= all_segments_.size())
        {
            return {};
        }
        return all_segments_[next_match];
    }

    std::string route_request_builder::next_segment() const
    {
        return get_next_segment(matched_segments_);
    }

    std::string route_request_builder::remaining_path() const
    {
        std::string joined;
        for (std::size_t i = matched_segments_.size(); i < all_segments_.size(); ++i)
        {
            if (!joined.empty())
            {
                joined += "/";
            }
            joined += all_segments_[i];
        }
        return routing::format_route(std::move(joined));
    }

    std::vector<std::string> route_request_builder::remaining_segments() const
    {
        std::vector<std::string> remaining;
        for (std::size_t i = matched_segments_.size(); i < all_segments_.size(); ++i)
        {
            remaining.push_back(all_segments_[i]);
        }
        return remaining;
    }

    int route_request_builder::matched_parts() const
    {
        int count = static_cast<int>(global_route_matches_.size());
        if (item_ != nullptr)
        {
            ++count;
        }
        if (content_ != nullptr)
        {
            ++count;
        }
        if (section_ != nullptr)
        {
            ++count;
        }
        return count;
    }

    std::string route_request_builder::make_uri_string(const std::vector<std::string>& segments)
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

    std::string route_request_builder::path_no_implicit() const
    {
        return make_uri_string(matched_segments_);
    }

    std::string route_request_builder::path_full() const
    {
        return make_uri_string(full_segments_);
    }

    bool route_request_builder::is_full_match() const
    {
        return matched_segments_.size() == all_segments_.size();
    }

    shell_uri_handler::node_location route_request_builder::get_node_location() const
    {
        shell_uri_handler::node_location location;
        if (item_ != nullptr)
        {
            location.set_node(search_node{item_});
        }
        if (section_ != nullptr)
        {
            location.set_node(search_node{section_});
        }
        if (content_ != nullptr)
        {
            location.set_node(search_node{content_});
        }
        return location;
    }
} // namespace maui::controls
