#!/usr/bin/env python3
"""Generate docs/examples/README.md — previews of the non-gallery example apps (from
port/cpp/examples/) rendered as C++ and C++&XAML on each platform.

For each standalone example (hello_world, counter, layouts, data_binding, collection_view,
custom_drawing) this emits a `##` header and, following the fixed template, an **iOS + Android**
table and a **macOS** table (macOS adds the AppKit/C++ and AppKit/C++&XAML columns). Each row
stacks Light over Dark. Screenshots live at docs/examples/<name>/<framework>/<file>; a missing
shot renders the shared `../comparison/_placeholder.png`. The `*_xaml` twins supply the C++&XAML
column; examples without a twin show a placeholder there.

Running this also scaffolds the per-example maui/cpp/xaml folders (with .gitkeep) so screenshots
can be dropped in later and picked up on the next run.

Screenshot filename convention (under docs/examples/<name>/<framework>/):
    ios_<theme>.png · android_<theme>.png · maccatalyst_<theme>.png · maccatalyst_appkit_<theme>.png
where <framework> in {maui,cpp,xaml} and <theme> in {light,dark}.

Usage: python3 tools/gen_examples_readme.py
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
COMP = os.path.normpath(os.path.join(HERE, ".."))
EXROOT = os.path.normpath(os.path.join(COMP, "..", "examples"))
README = os.path.join(EXROOT, "README.md")
PLACEHOLDER = "../comparison/_placeholder.png"  # relative to docs/examples/README.md
IMG_W = 400
THEMES = ("light", "dark")
FRAMEWORK_DIRS = ("maui", "cpp", "xaml")

# (name, title, description, has_xaml_twin) — the non-gallery examples, in tutorial order.
EXAMPLES = [
    ("hello_world", "Hello World", "The minimal app: a window hosting a page hosting a single label.", True),
    ("counter", "Counter", "The classic interactive example: a button's `clicked` event increments a label.", True),
    ("layouts", "Layouts", "Composing UI with a vertical_stack_layout and a 2x2 grid (rows/columns, spacing, padding).", True),
    ("data_binding", "Data Binding", "A bindable_object view-model bound to a label by name, driven live by an entry.", True),
    ("collection_view", "Collection View", "A list built from an observable_collection and a data_template cell recipe.", False),
    ("custom_drawing", "Custom Drawing", "A graphics_view rendering an i_drawable that paints shapes and text.", False),
]

# (platform-key, display label, framework columns) — iOS/Android share one template; macOS adds AppKit.
IOS_ANDROID = [("ios", "iOS"), ("android", "Android")]
IOS_ANDROID_FW = ["maui", "cpp", "xaml"]
MACOS_FW = ["maui", "cpp", "xaml", "appkit_cpp", "appkit_xaml"]
FW_HEAD = {
    "maui": "MAUI", "cpp": "C++", "xaml": "C++ &amp; XAML",
    "appkit_cpp": "AppKit / C++", "appkit_xaml": "AppKit / C++ &amp; XAML",
}


def esc(s):
    return (s or "").replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def rel(name, framework, fname):
    """Path (relative to docs/examples/README.md) or placeholder if the file is absent."""
    disk = os.path.join(EXROOT, name, framework, fname)
    return f"{name}/{framework}/{fname}" if os.path.isfile(disk) else PLACEHOLDER


def shot(name, fw_col, platform, theme):
    """Resolve one cell's image. fw_col is the display framework column; map it to a
    (folder, filename) under the example dir."""
    if fw_col == "appkit_cpp":
        return rel(name, "cpp", f"{platform}_appkit_{theme}.png")
    if fw_col == "appkit_xaml":
        return rel(name, "xaml", f"{platform}_appkit_{theme}.png")
    return rel(name, fw_col, f"{platform}_{theme}.png")


def img_td(src):
    return f'<td><img width="{IMG_W}" src="{src}" /></td>'


def block(name, platform, label, fws, desc):
    """One platform's Light-over-Dark rows (the platform name sits on the Light row)."""
    light = [f'<th colspan="2">{label}</th>', "<th>Light</th>"]
    light += [img_td(shot(name, fw, platform, "light")) for fw in fws]
    light += [f'<td colspan="2">{esc(desc)}</td>',
              '<td colspan="2">—</td>', '<td colspan="2">—</td>']
    dark = ["<th>Dark</th>"] + [img_td(shot(name, fw, platform, "dark")) for fw in fws]
    return f"<tr>{''.join(light)}</tr><tr>{''.join(dark)}</tr>"


def header_row(fws):
    ths = ["<th>Platform</th>", "<th></th>"] + [f"<th>{FW_HEAD[fw]}</th>" for fw in fws]
    ths += ["<th>Description</th>", "<th>Sonnet 5 parity review</th>", "<th>Gemini parity review</th>"]
    return "<tr>" + "".join(ths) + "</tr>"


def scaffold(name):
    for fw in FRAMEWORK_DIRS:
        d = os.path.join(EXROOT, name, fw)
        os.makedirs(d, exist_ok=True)
        gk = os.path.join(d, ".gitkeep")
        if not os.path.exists(gk):
            open(gk, "w").close()


def main():
    out = [
        "# MAUI C++ examples — previews",
        "",
        "Per-platform **C++** and **C++&amp;XAML** previews of the standalone example apps in "
        "`port/cpp/examples/` (the non-gallery ones). Each example shows an **iOS + Android** table and a "
        "**macOS** table (macOS adds the AppKit backend columns); every row stacks the Light render over the "
        "Dark one. Screenshots live under `docs/examples/<name>/{maui,cpp,xaml}/`; cells with no capture yet "
        "show a placeholder. Generated from the example folders by `../comparison/tools/gen_examples_readme.py`.",
        "",
    ]
    for name, title, desc, _has_xaml in EXAMPLES:
        scaffold(name)
        out.append(f"## {title}")
        out.append("")
        out.append(f"<sub>`port/cpp/examples/{name}`</sub>")
        out.append("")
        # iOS + Android table
        rows = [header_row(IOS_ANDROID_FW)]
        for plat, label in IOS_ANDROID:
            rows.append(block(name, plat, label, IOS_ANDROID_FW, desc))
        out.append("<table>" + "".join(rows) + "</table>")
        out.append("")
        # macOS table
        out.append("<table>" + header_row(MACOS_FW)
                   + block(name, "maccatalyst", "macOS", MACOS_FW, desc) + "</table>")
        out.append("")
    text = "\n".join(out).rstrip("\n") + "\n"
    open(README, "w", encoding="utf-8").write(text)
    print(f"wrote {README} ({len(text)} bytes, {len(EXAMPLES)} examples)")


if __name__ == "__main__":
    main()
