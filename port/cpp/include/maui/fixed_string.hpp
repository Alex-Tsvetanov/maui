#pragma once
// maui::fixed_string<N> — a structural, NTTP-usable fixed-capacity string (PUBLIC_API_DESIGN.md §6).
//
// This is the vehicle that lets `build_page<VM, login_xaml>()` receive *raw XAML text* as a non-type
// template parameter: a plain `const char[]` is not a structural type and cannot be an NTTP, but a
// struct whose only members are a char array and a length IS (class-type NTTP, P0732/P1907). So the
// developer embeds the markup once —
//
//     MAUI_EMBED_XAML(login_xaml, "LoginPage.xaml");          // -> constexpr maui::fixed_string login_xaml
//     auto page = maui::build_page<LoginViewModel, login_xaml>();
//
// and the bytes travel into build_page as a template argument, parsed by the compiler's constant
// evaluator with no external codegen step.
//
// #embed note: clang's #embed collapses to a SINGLE initializer in a CTAD/parameter-pack context (so
// `fixed_string{ #embed "f" }` deduces N==1), but expands fully when initializing a char array. The
// MAUI_EMBED_XAML macro therefore routes through a `constexpr unsigned char[] = { #embed ... }` and then
// constructs the fixed_string from that array — the reliable form. (See PUBLIC_API_DESIGN.md §6.)

#include <cstddef>
#include <string_view>

#include "maui/xaml/feature.hpp"

namespace maui
{
    // N is the number of CONTENT bytes (no trailing NUL). One extra slot always holds a NUL so view()
    // and c_str() are safe even for #embed payloads, which carry no terminator.
    template <std::size_t N> struct fixed_string
    {
        char data[N + 1]{};
        static constexpr std::size_t size = N;

        consteval fixed_string() = default;

        // From a string literal of M chars (M counts the literal's own trailing NUL) => content N == M-1.
        template <std::size_t M>
            requires(M == N + 1)
        consteval fixed_string(const char (&s)[M]) // NOLINT(google-explicit-constructor) — NTTP ergonomics
        {
            for (std::size_t i = 0; i < N; ++i)
            {
                data[i] = s[i];
            }
        }

        // From a raw byte array of exactly N bytes (the #embed-into-array form; no trailing NUL assumed).
        // Constrained to a 1-byte non-char element so it never competes with the string-literal ctor.
        template <class Byte, std::size_t M>
            requires(M == N) && (sizeof(Byte) == 1) && (!__is_same(Byte, char))
        consteval fixed_string(const Byte (&s)[M]) // NOLINT(google-explicit-constructor) — NTTP ergonomics
        {
            for (std::size_t i = 0; i < N; ++i)
            {
                data[i] = static_cast<char>(s[i]);
            }
        }

        [[nodiscard]] constexpr std::string_view view() const
        {
            return {data, N};
        }
        [[nodiscard]] constexpr const char* c_str() const
        {
            return data;
        }
    };

    // Deduction: a literal "abc" => fixed_string<3> (drop NUL); a byte array of K bytes => fixed_string<K>.
    template <std::size_t M> fixed_string(const char (&)[M]) -> fixed_string<M - 1>;
    template <class Byte, std::size_t M> fixed_string(const Byte (&)[M]) -> fixed_string<M>;
} // namespace maui

// Canonical compile-time-XAML embed (a #embed directive cannot live inside a macro body, so this is a
// hand-written two-liner the *_xaml examples follow verbatim):
//
//     namespace {
//         constexpr unsigned char counter_xaml_bytes[] = {
//             #embed "counter.xaml"
//         };
//         constexpr maui::fixed_string counter_xaml{counter_xaml_bytes};
//     }
//     auto page = maui::build_page<counter_view_model, counter_xaml>();
