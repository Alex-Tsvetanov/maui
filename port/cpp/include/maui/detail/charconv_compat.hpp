#pragma once
// maui::detail::from_chars_general — internal toolchain-compatibility shim, NOT a ported MAUI type
// (no originating C# FQN). The port's numeric-token rule (PROFILE: invariant-culture parses go
// through std::from_chars) uses the floating-point overload with chars_format::general, which libc++
// only implements from version 20 (LLVM landed FP std::from_chars in libc++ 20; integral charconv
// shipped much earlier). The Android NDK r27 LTS — the `android` preset's pinned toolchain — ships
// libc++ 18, so on that backend the call below falls back to a strtod/strtof wrapper that reproduces
// from_chars' general-format semantics exactly:
//   - no leading whitespace and no leading '+' (strtod accepts both; from_chars rejects at `first`),
//   - no hex floats: a "0x…"/"-0X…" token parses as the plain zero and stops at the 'x',
//   - the longest valid general-format prefix is consumed (inf/infinity/nan(seq) included),
//   - out-of-range input reports errc::result_out_of_range with `value` left untouched and the
//     returned ptr past the consumed token, mirroring std::from_chars.
// Locale note: strtod reads the decimal point from the current C locale. The port never calls
// setlocale, and bionic (the only consumer of this fallback today) only ever provides the C/POSIX
// locale, so the parse stays invariant — the on-device parse tests pin this down.
// Every other backend (AppleClang libc++ >= 20, libstdc++, MSVC) compiles the one-line forward.

#include <charconv>
#include <concepts>

#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 200000

    #include <cerrno>
    #include <cstdlib>
    #include <iterator>
    #include <string>
    #include <string_view>
    #include <system_error>

namespace maui::detail
{
    template <std::floating_point T>
    std::from_chars_result from_chars_general(const char* first, const char* last, T& value)
    {
        const auto length = static_cast<std::size_t>(std::distance(first, last));
        const std::string_view text(first, length);
        if (text.empty())
        {
            return {first, std::errc::invalid_argument};
        }
        const char head = text.front();
        // strtod skips leading whitespace (the " \t\n\v\f\r" set) and accepts a leading '+';
        // std::from_chars rejects both without consuming anything.
        if (head == '+' || head == ' ' || head == '\t' || head == '\n' || head == '\v' || head == '\f' || head == '\r')
        {
            return {first, std::errc::invalid_argument};
        }
        // strtod also accepts hex floats ("0x1p3"); chars_format::general does not — from_chars parses
        // the plain zero in front of the 'x' and stops there.
        const std::size_t zero_pos = head == '-' ? 1 : 0;
        if (text.size() >= zero_pos + 2 && text[zero_pos] == '0' &&
            (text[zero_pos + 1] == 'x' || text[zero_pos + 1] == 'X'))
        {
            value = head == '-' ? -T{0} : T{0};
            return {std::next(first, static_cast<std::ptrdiff_t>(zero_pos) + 1), std::errc{}};
        }
        const std::string token(text); // NUL-terminated copy: [first,last) need not be terminated
        char* token_end = nullptr;
        errno = 0;
        T parsed{};
        if constexpr (std::same_as<T, float>)
        {
            parsed = std::strtof(token.c_str(), &token_end);
        }
        else
        {
            parsed = static_cast<T>(std::strtod(token.c_str(), &token_end));
        }
        const char* token_stop = token_end; // const view of strtod's end pointer for std::distance
        const std::ptrdiff_t consumed = std::distance(token.c_str(), token_stop);
        if (consumed == 0)
        {
            return {first, std::errc::invalid_argument};
        }
        const char* stop = std::next(first, consumed);
        if (errno == ERANGE)
        {
            return {stop, std::errc::result_out_of_range}; // value stays untouched, like std::from_chars
        }
        value = parsed;
        return {stop, std::errc{}};
    }
} // namespace maui::detail

#else // full <charconv>: forward straight to the standard overload

namespace maui::detail
{
    template <std::floating_point T>
    std::from_chars_result from_chars_general(const char* first, const char* last, T& value)
    {
        return std::from_chars(first, last, value, std::chars_format::general);
    }
} // namespace maui::detail

#endif
