// read_uri_bytes — cross-platform local-file byte fetch for the uri image pipeline. See uri_bytes.hpp.
// Supports `file://` URIs + bare paths; http(s) and other schemes return empty (production HTTP deferred).

#include "maui/core/uri_bytes.hpp"

#include <cstddef>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <string_view>

#include "maui/core/i_stream_image_source.hpp"

namespace maui::core
{
    namespace
    {
        // Map a URI to a local filesystem path, or empty for an unsupported scheme. Handles
        // `file:///abs/path`, `file://localhost/abs/path`, and a bare `/abs/path` (no scheme).
        std::string to_local_path(std::string_view uri)
        {
            constexpr std::string_view file_scheme = "file://";
            if (uri.starts_with(file_scheme))
            {
                std::string_view rest = uri.substr(file_scheme.size());
                constexpr std::string_view localhost = "localhost";
                if (rest.starts_with(localhost))
                {
                    rest.remove_prefix(localhost.size());
                }
                // After `file://`, a leading '/' begins the absolute path; keep it.
                return std::string(rest);
            }
            // A bare absolute/relative path (no scheme) is read directly; an explicit non-file scheme
            // (http:, https:, …) is unsupported this cut.
            if (uri.contains("://"))
            {
                return {}; // some other scheme (e.g. http) — deferred
            }
            return std::string(uri);
        }
    } // namespace

    image_bytes read_uri_bytes(std::string_view uri)
    {
        const std::string path = to_local_path(uri);
        if (path.empty())
        {
            return {};
        }

        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            return {};
        }

        // Stream the whole file as chars, converting each to std::byte (no reinterpret_cast — std::byte*
        // is not pointer-compatible with char* for file.read, and a NOLINT would be a forbidden
        // suppression). istreambuf_iterator reads unformatted bytes.
        image_bytes bytes;
        for (std::istreambuf_iterator<char> it(file), end; it != end; ++it)
        {
            bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(*it)));
        }
        return bytes;
    }
} // namespace maui::core
