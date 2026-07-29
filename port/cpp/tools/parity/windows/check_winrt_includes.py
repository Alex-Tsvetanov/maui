#!/usr/bin/env python3
"""Fail locally on the one C++/WinRT mistake this backend keeps repeating.

THREE consecutive handler batches shipped with the same defect and each cost a full round trip to the
Windows guest to discover: a file calls a member inherited from a base class in another namespace
without including that namespace's FULL header. C++/WinRT's impl/*.0.h headers only FORWARD-DECLARE
those members, so the call fails with

    error C3779: 'X': a function that returns 'auto' cannot be used before it is defined

which names neither the missing header nor the concept. Warning agents about it in prose demonstrably
does not work -- the rule is mechanical and checkable, so it belongs here, where it fails in a second
on the dev machine instead of ten minutes later on the guest.

Deliberately a TEXTUAL check, not a compiler: it cannot see through typedefs or `auto`, so it is a
lint (catch the common shape cheaply) rather than a gate (prove correctness). It reports what it is
sure about and stays quiet otherwise; a false negative costs what we already pay today, while a false
positive would train people to ignore it.

Usage:  python3 tools/parity/windows/check_winrt_includes.py [files...]
        (no arguments = every .cpp under src/platform/windows/)
Exit 1 if anything looks wrong.
"""
from __future__ import annotations

import pathlib
import re
import sys

HERE = pathlib.Path(__file__).resolve()
WINDOWS_SRC = HERE.parents[3] / "src" / "platform" / "windows"

# member call -> the header that must be included for it to be more than a forward declaration.
# Keep each entry justified by a real failure or a real base-class relationship; speculative entries
# make the lint noisy and noisy lints get ignored.
RULES: list[tuple[str, re.Pattern[str], str]] = [
    (
        "winrt/Windows.Foundation.Collections.h",
        # IVector / IMap members. Children()/Items()/Inlines()/Resources() all hand back one of these.
        re.compile(r"\.(?:Append|InsertAt|RemoveAt|RemoveAtEnd|ReplaceAll|GetAt|IndexOf|SetAt|Clear|"
                   r"Insert|Lookup|HasKey|Remove|Size)\s*\("),
        "IVector/IMap members (Children(), Items(), Inlines(), Resources() ...)",
    ),
    (
        "winrt/Microsoft.UI.Xaml.Controls.Primitives.h",
        # Selector is where ComboBox/ListView get SelectedIndex + SelectionChanged; RangeBase is where
        # Slider gets Value/Minimum/Maximum.
        # NOT SelectionChanged: TextBox declares its own, in Microsoft.UI.Xaml.Controls, so matching it
        # false-positived on entry_handler.cpp the first time this lint ran. Only members that exist
        # NOWHERE but Selector belong here -- a lint that cries wolf is a lint people learn to ignore,
        # which is the exact failure this file exists to prevent.
        re.compile(r"\.(?:SelectedIndex|IsSelectionActive)\s*\("),
        "Selector members inherited by ComboBox/ListViewBase",
    ),
    (
        "winrt/Microsoft.UI.Xaml.Documents.h",
        re.compile(r"\.(?:Inlines|TextHighlighters)\s*\(|winui::Documents::|Documents::Run\b"),
        "Inlines/Run/TextHighlighter (Microsoft.UI.Xaml.Documents)",
    ),
]


def check(path: pathlib.Path) -> list[str]:
    text = path.read_text(encoding="utf-8", errors="replace")
    # Strip line comments so a rule name mentioned in prose does not trip its own check.
    body = re.sub(r"//[^\n]*", "", text)
    problems = []
    for header, pattern, why in RULES:
        if pattern.search(body) and f"#include <{header}>" not in text:
            hit = pattern.search(body)
            line = body[: hit.start()].count("\n") + 1
            problems.append(f"{path.name}:{line}: calls {why} but does not "
                            f"#include <{header}>  (this is C3779 on the guest)")
    return problems


def main(argv: list[str]) -> int:
    files = [pathlib.Path(a) for a in argv] or sorted(WINDOWS_SRC.glob("*.cpp"))
    if not files:
        print(f"no sources found under {WINDOWS_SRC}", file=sys.stderr)
        return 2
    problems = [p for f in files for p in check(f)]
    for p in problems:
        print(p)
    print(f"checked {len(files)} file(s): {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
