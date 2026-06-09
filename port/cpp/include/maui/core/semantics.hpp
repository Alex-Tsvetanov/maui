#pragma once
// maui::core::semantics  <=  Microsoft.Maui.Semantics (+ Microsoft.Maui.SemanticHeadingLevel)
//
// Accessibility metadata for a view (IView.Semantics): a Description (the accessible name), a Hint (extra
// usage detail), and a HeadingLevel (so assistive tech can navigate by heading). Ported from
// src/Core/src/Primitives/Semantics.cs + SemanticHeadingLevel.cs. A control owns its semantics object via
// a property<shared_ptr<semantics>>; i_view::semantics() hands back the raw borrow, which the shared
// view_mapper pushes to the platform view (headless mirrors it; the native accessibility push —
// NSView.accessibilityLabel / accessibilityHelp — is deferred, see STATUS.md).

#include <cstdint>
#include <string>
#include <utility>

namespace maui::core
{
    // SemanticHeadingLevel: None plus HTML-style heading levels 1–9 (h1…h9).
    enum class semantic_heading_level : std::uint8_t
    {
        none = 0,
        level1 = 1,
        level2 = 2,
        level3 = 3,
        level4 = 4,
        level5 = 5,
        level6 = 6,
        level7 = 7,
        level8 = 8,
        level9 = 9,
    };

    class semantics
    {
    public:
        [[nodiscard]] const std::string& description() const
        {
            return description_;
        }
        void set_description(std::string value)
        {
            description_ = std::move(value);
        }
        [[nodiscard]] const std::string& hint() const
        {
            return hint_;
        }
        void set_hint(std::string value)
        {
            hint_ = std::move(value);
        }
        [[nodiscard]] semantic_heading_level heading_level() const
        {
            return heading_level_;
        }
        void set_heading_level(semantic_heading_level value)
        {
            heading_level_ = value;
        }
        // Semantics.IsHeading: true when a heading level is assigned (i.e. not None).
        [[nodiscard]] bool is_heading() const
        {
            return heading_level_ != semantic_heading_level::none;
        }

    private:
        std::string description_;
        std::string hint_;
        semantic_heading_level heading_level_ = semantic_heading_level::none;
    };
} // namespace maui::core
