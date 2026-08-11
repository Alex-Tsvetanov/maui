#pragma once
// maui::xaml compile-time XAML validation  <=  the XamlC half of Microsoft.Maui.Controls.Xaml.
//
// MAUI validates XAML TWICE. The runtime loader (ApplyPropertiesVisitor) throws when it cannot assign a
// member, and XamlC (src/Controls/src/Build.Tasks/SetPropertiesVisitor.cs) resolves the same markup at
// BUILD time and FAILS THE BUILD — ConnectEvent throws BuildException(MissingEventHandler) at :1319, an
// unresolvable member throws MemberResolution at :1222. A compile-time rejection is therefore the
// FAITHFUL behavior for markup the loader cannot hydrate, not a nicety bolted on top of it.
//
// The port has no XamlC, but it does not need one for this class of error: build_page<VM, Xaml> receives
// the markup as a class-type NTTP (maui::fixed_string), so the bytes are ALREADY a constant expression
// and the compiler's own constant evaluator can walk them. This header is that walk.
//
// SCOPE — deliberately narrow, and it is a BLOCKLIST, not an allowlist. It rejects exactly one class:
// the INLINE EVENT ATTRIBUTE (Clicked="OnClicked"). That class is decidable from the markup text alone,
// which is why it can be checked here. The general question "can the loader assign this property?" is
// NOT decidable at compile time in this port: the answer lives in xaml_property_registry, a runtime
// std::unordered_map of std::function seeded by dynamic initialization across the register_xaml_*.cpp
// files, and type_tag::of<T>() returns the address of a function-local static — a link-time address is
// never a constant expression. Validating the full surface needs the registry as CONSTANT DATA, i.e. a
// generated table, i.e. XamlC. Everything else keeps surfacing at load time through the loader's
// exception_handler (hydration_context::handle).
//
// WHY EVENT ATTRIBUTES CANNOT WORK AT RUNTIME (the thing this check makes visible early): C#'s
// TryConnectEvent (ApplyPropertiesVisitor.cs:473-537) needs three reflection operations —
// GetRuntimeEvent(localName), rootElementType.GetMethod((string)value, ...) walking BaseType, and
// mi.CreateDelegate(...). PROFILE.md section 6 removes all three: this port resolves types, properties and
// converters through EXPLICIT registration and never discovers members from a string at runtime. So a
// string can never name a member function here. The supported spelling is code-behind wiring through
// x:Name, which is typed and compile-checked already — see examples/gallery_xaml/Views/button.xaml.cpp:
//     page->find<controls::button>("ClickedButton")->clicked.connect(...); page->retain(...);
//
// POSITION CONVENTION: the reported column is the ATTRIBUTE NAME's, matching XmlReader (and therefore
// C#: XamlParser.cs:361-362 builds an attribute's ValueNode from the reader's own IXmlLineInfo, which is
// positioned on the attribute). This is deliberately NOT the port's runtime column: xaml_parser.hpp:71-74
// documents that port nodes built from attribute values carry the OWNING ELEMENT's position instead,
// because pugixml exposes no per-attribute offset. The runtime says 16:10 (the <Button element) where
// this check says 16:33 (the Clicked attribute) — the check is the more faithful of the two.

#include <cstddef>
#include <string_view>

namespace maui::xaml
{
    // The event-attribute names rejected in markup. SINGLE SOURCE OF TRUTH: port/tools/e2e/e2e.py parses
    // this array out of this header to build its own lint regex (EVENT_ATTRS), so the two can never drift.
    // Keep one name per line between the BEGIN/END markers — the parser there is deliberately dumb.
    // MAUI_EVENT_ATTRIBUTE_NAMES BEGIN
    inline constexpr std::string_view event_attribute_names[] = {
        "Clicked",
        "Pressed",
        "Released",
        "Tapped",
        "TextChanged",
        "ValueChanged",
        "Toggled",
        "CheckedChanged",
        "SelectedIndexChanged",
        "SelectionChanged",
        "ItemSelected",
        "ItemTapped",
        "Scrolled",
        "ScrollToRequested",
        "Refreshing",
        "SearchButtonPressed",
        "DateSelected",
        "TimeSelected",
        "Completed",
        "Unfocused",
        "Focused",
        "Loaded",
        "Unloaded",
        "Appearing",
        "Disappearing",
        "NavigatedTo",
        "NavigatingFrom",
        "NavigatedFrom",
        "PositionChanged",
        "CurrentItemChanged",
        "RemainingItemsThresholdReached",
        "SwipeStarted",
        "SwipeChanging",
        "SwipeEnded",
        "DragStarting",
        "DropCompleted",
        "Navigating",
        "Navigated",
    };
    // MAUI_EVENT_ATTRIBUTE_NAMES END

    // Where an offending attribute sits, 1-based. line == 0 means "none found" — the success value.
    // A structural type (public scalar members only), so it can travel as a non-type template argument
    // and show up in the compiler's instantiation trail. std::string_view could NOT: its private data
    // pointer makes it non-structural, which is why the name itself is not carried here.
    struct event_attribute_hit
    {
        int line = 0;
        int column = 0;
    };

    namespace detail
    {
        [[nodiscard]] constexpr bool is_xaml_space(char c)
        {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }

        // XML name characters, restricted to what an event attribute can actually contain. Used only to
        // reject a PREFIX match (so "ClickedTwice" never matches "Clicked").
        [[nodiscard]] constexpr bool is_xaml_name_char(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' ||
                   c == '.' || c == ':';
        }

        [[nodiscard]] constexpr bool starts_with_at(std::string_view text, std::size_t at, std::string_view needle)
        {
            if (needle.size() > text.size() - at)
            {
                return false;
            }
            for (std::size_t i = 0; i < needle.size(); ++i)
            {
                if (text[at + i] != needle[i])
                {
                    return false;
                }
            }
            return true;
        }
    } // namespace detail

    // Scan `xaml` for the first inline event attribute, as a constant expression.
    //
    // This is a real (if tiny) tokenizer rather than a substring search, because a substring search is
    // WRONG on the corpus and provably so: port/maui-reference/pages/gap_event_attribute.xaml carries
    // Clicked="..." inside an XML COMMENT on line 4 and Clicked=&quot; inside a Label's Text on line 15,
    // and the real attribute only appears on line 16. A scan that ignores comments and quoted values
    // reports the wrong position on that file — which is the standing bug in e2e.py's EVENT_RE regex.
    //
    // Only three states are needed: inside a comment, inside a quoted attribute value, inside a tag.
    // An attribute name is only considered when it is inside a tag and preceded by whitespace, which
    // also keeps the constant-evaluation step count low (element names are never even attempted).
    [[nodiscard]] constexpr event_attribute_hit find_inline_event_attribute(std::string_view xaml)
    {
        int line = 1;
        int column = 1;
        bool in_tag = false;
        char quote = '\0';

        std::size_t i = 0;
        while (i < xaml.size())
        {
            // Comments first: <!-- ... --> can hold anything, including a literal event attribute, and it
            // must not open a tag either. Only outside a quoted value (a value may contain "<!--").
            //
            // The `xaml[i] == '<'` guard before the call is NOT redundant — it is a constant-evaluation
            // budget guard, and so is the first-char gate on the name table below. Both branches are
            // reached once per character. MEASURED (clang -fsyntax-only on the largest page,
            // selection_synchronization.xaml at 10556 B): with the two guards the scan needs 200000-250000
            // constexpr steps; without them, over 400000. Keep them — dropping either roughly doubles the
            // budget this file costs to compile.
            if (quote == '\0' && xaml[i] == '<' && detail::starts_with_at(xaml, i, "<!--"))
            {
                while (i < xaml.size() && !(xaml[i] == '-' && detail::starts_with_at(xaml, i, "-->")))
                {
                    if (xaml[i] == '\n')
                    {
                        ++line;
                        column = 1;
                    }
                    else
                    {
                        ++column;
                    }
                    ++i;
                }
                // Step over the "-->" itself (or stop, on an unterminated comment).
                for (int skipped = 0; skipped < 3 && i < xaml.size(); ++skipped, ++i, ++column)
                {
                }
                continue;
            }

            const char c = xaml[i];
            if (quote != '\0')
            {
                if (c == quote)
                {
                    quote = '\0';
                }
            }
            else if (c == '<')
            {
                in_tag = true;
            }
            else if (c == '>')
            {
                in_tag = false;
            }
            else if (in_tag && (c == '"' || c == '\''))
            {
                quote = c;
            }
            else if (in_tag && i > 0 && detail::is_xaml_space(xaml[i - 1]))
            {
                // An attribute-name position. Match the table, then require the name to be FOLLOWED by
                // optional whitespace and '=' — a bare word (or an element name) is not an assignment.
                for (const std::string_view name : event_attribute_names)
                {
                    // First-char gate before the full compare — the second half of the measured budget
                    // guard described at the comment branch above; this table is 38 entries and is walked
                    // at every attribute position.
                    if (c != name[0] || !detail::starts_with_at(xaml, i, name))
                    {
                        continue;
                    }
                    std::size_t after = i + name.size();
                    if (after < xaml.size() && detail::is_xaml_name_char(xaml[after]))
                    {
                        continue; // a longer name that merely starts with an event name
                    }
                    while (after < xaml.size() && detail::is_xaml_space(xaml[after]))
                    {
                        ++after;
                    }
                    if (after < xaml.size() && xaml[after] == '=')
                    {
                        return {.line = line, .column = column};
                    }
                }
            }

            if (c == '\n')
            {
                ++line;
                column = 1;
            }
            else
            {
                ++column;
            }
            ++i;
        }
        return {};
    }

    // The failure carrier. On success this is instantiated as <{0, 0}> and `none_found` is true; on
    // failure the compiler prints the hit — line and column — as part of the instantiation trail, which
    // is how the diagnostic carries a position without needing C++26's user-generated static_assert
    // messages (P2741). P2741 is NOT usable here: Apple clang 21 accepts it under -std=c++23 as an
    // extension, but the Android NDK's Clang 18 rejects it outright, and the NDK compiles these same TUs
    // in bytes mode. A string-literal message plus template arguments works on both.
    template <event_attribute_hit Hit> struct inline_event_attributes_are_unsupported
    {
        static constexpr bool none_found = (Hit.line == 0);
    };
} // namespace maui::xaml

// Reject inline event attributes in `xaml_constant` (a maui::fixed_string) at COMPILE TIME. Emitted by
// port/tools/e2e/e2e.py gen into every generated page TU except the deliberate gap probe, so it covers
// both delivery modes: the committed #embed TUs (desktop/iOS/macOS) and the generated bytes-mode TUs
// (Android NDK). The offending line/column appear in the instantiation trail; the FILE comes free, since
// the static_assert sits in <page>.xaml.cpp.
#define MAUI_XAML_REJECT_EVENT_ATTRIBUTES(xaml_constant)                                                               \
    static_assert(::maui::xaml::inline_event_attributes_are_unsupported<::maui::xaml::find_inline_event_attribute(     \
                      (xaml_constant).view())>::none_found,                                                            \
                  "This XAML uses an inline event attribute (e.g. Clicked=\"OnClicked\"). The maui C++ port "          \
                  "resolves members through explicit registration and has no reflection (PROFILE.md section 6), so a " \
                  "string can never name a member function. Wire the handler in code-behind via x:Name instead: "      \
                  "page->find<maui::controls::button>(\"MyButton\")->clicked.connect(...). See the "                   \
                  "inline_event_attributes_are_unsupported template argument for the line and column.")
