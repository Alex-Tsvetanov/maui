#pragma once
// maui::controls::shell_uri  <=  the System.Uri subset Shell routing relies on
//
// C#'s Shell URI machinery (ShellUriHandler / ShellNavigationState) leans on System.Uri for a handful
// of accessors: IsAbsoluteUri, OriginalString, Host, PathAndQuery, LocalPath, Query, Fragment. The
// port has no System.Uri, so this small value type reproduces exactly that subset with the same
// observable behavior for the URI shapes Shell produces ("app://shell/IMPL_shell/path?query#frag",
// "app:/shell/path", "//path", "path", "../path"):
//   - an ABSOLUTE uri has a scheme ("app:"), an optional authority ("//host"), and a path;
//     "app:/path" (single slash) parses absolute with an EMPTY host, like System.Uri;
//   - a RELATIVE uri only carries its original string (the absolute accessors return empty, where
//     System.Uri would throw — Shell never calls them on a relative uri).
// Equality compares the canonical string (scheme://host/path?query#fragment for absolute; the
// original string for relative), matching how the C# code compares Uris it has normalized itself.

#include <string>
#include <string_view>

namespace maui::controls
{
    class shell_uri
    {
    public:
        shell_uri() = default;

        // Parse `value` as absolute when it carries a scheme, else as relative (Uri.TryCreate(...,
        // UriKind.Absolute) + the relative fallback). NOTE: callers wanting C#'s CreateUri semantics
        // (a leading "/" forces relative) should use shell_uri_handler::create_uri instead.
        [[nodiscard]] static shell_uri parse(std::string value);
        // Force-relative (new Uri(value, UriKind.Relative)).
        [[nodiscard]] static shell_uri relative(std::string value);

        [[nodiscard]] bool is_absolute() const
        {
            return is_absolute_;
        }
        [[nodiscard]] const std::string& original_string() const
        {
            return original_;
        }

        // ---- absolute-only accessors (empty on a relative uri) ----
        [[nodiscard]] const std::string& scheme() const
        {
            return scheme_;
        }
        [[nodiscard]] const std::string& host() const
        {
            return host_;
        }
        // Uri.PathAndQuery: the path (always with a leading '/') plus the "?query" (no fragment).
        [[nodiscard]] std::string path_and_query() const;
        // Uri.LocalPath: the path only (leading '/', no query, no fragment).
        [[nodiscard]] const std::string& local_path() const
        {
            return path_;
        }
        // Uri.Query: "?query" or "".
        [[nodiscard]] std::string query() const;
        // Uri.Fragment: "#fragment" or "".
        [[nodiscard]] std::string fragment() const;

        // The canonical string: scheme://host/path?query#fragment when absolute, else the original.
        [[nodiscard]] std::string to_string() const;

        friend bool operator==(const shell_uri& lhs, const shell_uri& rhs)
        {
            return lhs.to_string() == rhs.to_string();
        }

    private:
        std::string original_;
        std::string scheme_;
        std::string host_;
        std::string path_;     // with the leading '/'
        std::string query_;    // without the '?'
        std::string fragment_; // without the '#'
        bool is_absolute_ = false;
        bool has_query_ = false;
        bool has_fragment_ = false;
    };
} // namespace maui::controls
