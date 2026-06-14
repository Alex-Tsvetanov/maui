#pragma once
// maui::controls::route_request_builder  <=  Microsoft.Maui.Controls.RouteRequestBuilder (internal)
//
// One candidate interpretation of a navigation URI while the matcher walks the tree: the shell
// nodes matched so far, the matched/full segment lists, and the global-route matches appended at
// the tail. A COPYABLE value (the C# copy-constructor clones candidates at every branch). Ported
// 1:1 from RouteRequestBuilder.cs.

#include <string>
#include <vector>

#include "maui/controls/shell/shell_uri_handler.hpp"

namespace maui::controls
{
    class route_request_builder
    {
    public:
        explicit route_request_builder(std::vector<std::string> all_segments);
        route_request_builder(const std::string& shell_segment, const std::string& user_segment,
                              const search_node* node, std::vector<std::string> all_segments);

        // ---- the matched shell nodes (borrowed — alive for the duration of one resolution) ----
        [[nodiscard]] shell* get_shell() const
        {
            return shell_;
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
        [[nodiscard]] search_node lowest_child() const;

        void add_global_route(const std::string& route_name, const std::string& segment);
        // Complete this candidate from a node location (AddMatch(NodeLocation)); false when a
        // user-defined route on the location refuses the next segment.
        bool add_match(const shell_uri_handler::node_location& node_location);
        void add_match(const std::string& shell_segment, const std::string& user_segment, const search_node& node);

        // The longest prefix of `match_me` that continues this candidate's segments ("" when none).
        [[nodiscard]] std::string get_next_segment_match(const std::string& match_me) const;

        [[nodiscard]] std::string next_segment() const;
        [[nodiscard]] std::string remaining_path() const;
        [[nodiscard]] std::vector<std::string> remaining_segments() const;
        [[nodiscard]] int matched_parts() const;
        [[nodiscard]] std::string path_no_implicit() const;
        [[nodiscard]] std::string path_full() const;
        [[nodiscard]] bool is_full_match() const;
        [[nodiscard]] const std::vector<std::string>& global_route_matches() const
        {
            return global_route_matches_;
        }
        [[nodiscard]] const std::vector<std::string>& segments_matched() const
        {
            return matched_segments_;
        }
        [[nodiscard]] const std::vector<std::string>& full_segments() const
        {
            return full_segments_;
        }
        [[nodiscard]] shell_uri_handler::node_location get_node_location() const;

    private:
        [[nodiscard]] std::string get_next_segment(const std::vector<std::string>& matched_segments) const;
        [[nodiscard]] static std::string make_uri_string(const std::vector<std::string>& segments);

        std::vector<std::string> global_route_matches_;
        std::vector<std::string> matched_segments_;
        std::vector<std::string> full_segments_;
        std::vector<std::string> all_segments_;
        shell* shell_ = nullptr;
        shell_item* item_ = nullptr;
        shell_section* section_ = nullptr;
        shell_content* content_ = nullptr;
    };
} // namespace maui::controls
