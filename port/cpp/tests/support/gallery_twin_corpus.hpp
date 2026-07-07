#pragma once
// maui::test — gallery-twin corpus helpers shared by the two twin test binaries: the load-gate
// (tests/xaml/gallery_twin_tests.cpp) and the render-gate (tests/hosting/gallery_twin_render_tests.cpp).
// These were hand-synced verbatim in both TUs; one copy now. Both binaries define the compile-time
// paths SHARED_PAGES_DIR + GALLERY_TWINS_DIR, which these helpers use.

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace maui::test
{
    // The page files: the shared-pages dir (port/maui-reference/pages, SHARED_PAGES_DIR) UNION the
    // legacy twins dir, a shared page superseding a same-stem legacy twin (e2e.py gen's precedence).
    [[nodiscard]] inline std::vector<std::filesystem::path> twin_files()
    {
        std::vector<std::filesystem::path> files;
        std::vector<std::string> seen;
        for (const char* dir_name : {SHARED_PAGES_DIR, GALLERY_TWINS_DIR})
        {
            const std::filesystem::path dir{dir_name};
            if (!std::filesystem::is_directory(dir))
            {
                continue;
            }
            for (const auto& entry : std::filesystem::directory_iterator(dir))
            {
                if (!entry.is_regular_file() || entry.path().extension() != ".xaml")
                {
                    continue;
                }
                const std::string stem = entry.path().stem().string();
                if (std::find(seen.begin(), seen.end(), stem) != seen.end())
                {
                    continue; // shared page already claimed this key
                }
                seen.push_back(stem);
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    [[nodiscard]] inline std::string read_file(const std::filesystem::path& path)
    {
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        return buffer.str();
    }

    // Minimal scanner for pages/manifest.json's flat "key"/"expected_port_status" string pairs (NOT a
    // general JSON parser — the manifest is a flat array of objects with no nesting in these two fields).
    // Returns key -> expected_port_status.
    [[nodiscard]] inline std::map<std::string, std::string> load_expected_statuses()
    {
        std::map<std::string, std::string> statuses;
        const std::filesystem::path manifest_path = std::filesystem::path(SHARED_PAGES_DIR) / "manifest.json";
        const std::string text = read_file(manifest_path);

        auto next_string_field = [&text](std::size_t& pos, std::string_view field_name) -> std::string {
            const std::string needle = "\"" + std::string(field_name) + "\"";
            const std::size_t field_pos = text.find(needle, pos);
            if (field_pos == std::string::npos)
            {
                return {};
            }
            const std::size_t colon = text.find(':', field_pos);
            const std::size_t value_start = text.find('"', colon + 1);
            const std::size_t value_end = text.find('"', value_start + 1);
            if (colon == std::string::npos || value_start == std::string::npos || value_end == std::string::npos)
            {
                return {};
            }
            pos = value_end + 1;
            return text.substr(value_start + 1, value_end - value_start - 1);
        };

        std::size_t pos = 0;
        while (true)
        {
            const std::size_t key_field = text.find("\"key\"", pos);
            if (key_field == std::string::npos)
            {
                break;
            }
            std::size_t scan_pos = key_field;
            const std::string key = next_string_field(scan_pos, "key");
            const std::string status = next_string_field(scan_pos, "expected_port_status");
            if (!key.empty() && !status.empty())
            {
                statuses[key] = status;
            }
            pos = scan_pos;
        }
        return statuses;
    }
} // namespace maui::test
