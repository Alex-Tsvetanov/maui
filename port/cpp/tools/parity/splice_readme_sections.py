#!/usr/bin/env python3
"""Splice the generated macOS / Android sections into docs/comparison/README.md and
make all three platform tables collapsible — idempotently.

Steps (all idempotent):
  1. Wrap the existing iOS Examples table in a <details> block.
     The iOS rows are hand/historically maintained, so we DON'T regenerate them —
     we only surround the "## Examples …" header + its 172 rows with
     <!-- IOS:BEGIN --> <details>…<summary><h2>iOS (172 examples) — click to expand</h2></summary>
     … the unchanged table …
     </details> <!-- IOS:END -->.
     If the IOS markers already exist, the block is left as-is.
  2. Replace everything between <!-- MACOS:BEGIN --> / <!-- MACOS:END --> with the
     output of gen_macos_readme_section.py.
  3. Replace everything between <!-- ANDROID:BEGIN --> / <!-- ANDROID:END --> with the
     output of gen_android_readme_section.py.

Usage: python3 tools/parity/splice_readme_sections.py
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
README = os.path.join(ROOT, "docs", "comparison", "README.md")

IOS_BEGIN = "<!-- IOS:BEGIN -->"
IOS_END = "<!-- IOS:END -->"
IOS_SUMMARY = "<summary><h2>iOS (172 examples) — click to expand</h2></summary>"


def gen(script):
    out = subprocess.run([sys.executable, os.path.join(HERE, script)],
                         capture_output=True, text=True, check=True)
    return out.stdout.rstrip("\n")


def replace_between(text, begin, end, body):
    bi = text.index(begin)
    ei = text.index(end)
    return text[:bi + len(begin)] + "\n\n" + body + "\n\n" + text[ei:]


def wrap_ios(text):
    """Wrap the iOS Examples table in a collapsible <details>, idempotently."""
    if IOS_BEGIN in text:
        return text  # already wrapped
    header = "## Examples (simplest → most complex)"
    hi = text.index(header)
    # The iOS table runs from `header` up to the blank line before <!-- MACOS:BEGIN -->.
    macos_i = text.index("<!-- MACOS:BEGIN -->")
    before = text[:hi]
    block = text[hi:macos_i].rstrip("\n")
    after = text[macos_i:]
    wrapped = (
        f"{IOS_BEGIN}\n<details>\n{IOS_SUMMARY}\n\n"
        f"{block}\n\n"
        f"</details>\n{IOS_END}\n\n"
    )
    return before + wrapped + after


def main():
    with open(README, encoding="utf-8") as f:
        text = f.read()

    text = wrap_ios(text)
    text = replace_between(text, "<!-- MACOS:BEGIN -->", "<!-- MACOS:END -->",
                           gen("gen_macos_readme_section.py"))
    text = replace_between(text, "<!-- ANDROID:BEGIN -->", "<!-- ANDROID:END -->",
                           gen("gen_android_readme_section.py"))

    if not text.endswith("\n"):
        text += "\n"
    with open(README, "w", encoding="utf-8") as f:
        f.write(text)
    print("Spliced macOS + Android sections and wrapped the iOS table.")


if __name__ == "__main__":
    main()
