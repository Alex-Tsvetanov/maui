#!/usr/bin/env bash
# Claude Code PostToolUse hook (Write|Edit|MultiEdit): auto-run clang-format -i on C/C++ files
# under port/cpp/ after each write. Wired from .claude/settings.json. No-op for anything else.
#
# The hook receives the tool-call JSON on stdin; we pull tool_input.file_path from it.

file="$(jq -r '.tool_input.file_path // empty' 2>/dev/null)"
[ -n "$file" ] || exit 0
[ -f "$file" ] || exit 0

case "$file" in
*/port/cpp/*.hpp | */port/cpp/*.cpp | */port/cpp/*.h | */port/cpp/*.hh | */port/cpp/*.cc | \
    */port/cpp/*.cxx | */port/cpp/*.hxx | */port/cpp/*.mm | */port/cpp/*.ipp)
    cf="$(command -v clang-format || echo /opt/homebrew/bin/clang-format)"
    "$cf" -i "$file" >/dev/null 2>&1 || true
    ;;
esac
exit 0
