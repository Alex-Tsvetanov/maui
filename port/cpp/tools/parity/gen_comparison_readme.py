#!/usr/bin/env python3
"""Generate docs/comparison/README.md — the single self-contained iOS parity comparison.

Inputs (all under docs/comparison/): montages/<key>.png, captures/{cpp,maui}_{light,dark}/<key>.gif (for
animated keys), diff_results.json, analysis_gemini.json, analysis_sonnet/<key>.json. Per-example
descriptions are extracted from each src/samples/pages/<key>_page.hpp header comment.

Layout: Setup section -> classification Summary -> table
  | # | Example (name + what it demonstrates) | Demo (2x2 native montage; GIFs for animated) |
    Sonnet (claude-sonnet-4-6) category + notes | Gemini (<model>) category + notes |
ordered # = simplest->complex (FIX_ORDER).
"""
import datetime
import html
import json
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))            # port/cpp
CMP = os.path.join(ROOT, "docs", "comparison")
PAGES = os.path.join(ROOT, "src", "samples", "pages")

PP_SSIM = 0.99   # "pixel-perfect-grade": cross-engine AA prevents byte-exact 0; SSIM>=this (both themes).
MODEL_SONNET = "claude-sonnet-4-6"
SIM = "iPhone 17 (1206x2622 native, no Display Zoom)"
UDID = "C4926671-2FA7-428E-B4A4-480692EE742B"

ANIMATED = {"activity_indicator", "animation", "carousel_page", "swipe_refresh", "empty_view_load_simulate",
            "swipe_gesture", "swipe_item_position", "gestures", "pan_gesture_events", "pointer_gesture",
            "ios_pan_gesture", "ios_swipe_transition", "ios_blur_effect", "chrome"}

# Simplest -> complex (copied from the legacy gen_parity_readme.FIX_ORDER).
FIX_ORDER = [
    "label", "button", "entry", "editor", "search_bar", "picker", "date_picker", "time_picker", "pickers",
    "slider", "stepper", "switch", "check_box", "progress_bar", "activity_indicator", "indicator",
    "image", "image_button", "box_view", "content_view", "containers", "controls_stack", "input_controls",
    "fonts", "formatted_text", "styles", "triggers", "behaviors", "semantics", "app_theme_binding",
    "stack_layout", "vertical_stack", "horizontal_stack", "grid", "absolute_layout", "flex_layout",
    "relative_layout", "alignment", "z_index", "layout_is_enabled",
    "shapes", "ellipse_gallery", "rectangle_gallery", "line_gallery", "line_join_gallery", "polygon_gallery",
    "polyline_gallery", "path_gallery", "path_aspect_gallery", "path_transform_string", "composition_gallery",
    "transform_playground", "transformations", "update_path_data", "auto_size_shapes", "shape_app_theme",
    "invalidate_brush", "gradient",
    "border", "border_stroke", "border_layout", "border_playground", "border_clip_playground",
    "border_resize_content", "borderless", "clip", "clip_views", "clip_corner_radius", "clip_gallery",
    "clipping", "shadow_playground", "invalidate_shadow_host",
    "collectionview", "items", "single_bound_selection", "multiple_bound_selection", "preselected_item",
    "preselected_items", "selection_command_param", "selection_synchronization", "filter_collection",
    "filter_selection", "header_footer", "header_footer_grid", "header_footer_grid_horizontal",
    "header_footer_template", "header_footer_view", "footer_only_string", "basic_grouping", "grid_grouping",
    "grouping_no_templates", "grouping_plus_selection", "switch_grouping", "some_empty_groups",
    "scroll_to_group", "scroll_mode_test", "adaptive_collection", "staggered_layout", "varied_size_selector",
    "nested_collection",
    "data_template_selector", "cv_visual_states", "empty_view", "empty_view_null", "empty_view_rtl",
    "empty_view_selector", "empty_view_swap", "empty_view_template", "empty_view_view",
    "empty_view_load_simulate", "carousel_page", "chat_example", "items_updating_scroll_mode",
    "radio_button_group", "radio_button_group_binding", "radio_button_group_gallery", "radio_button_border",
    "radio_button_content", "radio_content_properties", "radio_template_from_style", "scattered_radio_button",
    "swipe_gesture", "swipe_item_position", "swipe_item_size", "swipe_threshold", "swipe_view_margin",
    "swipe_view_shadow", "swipe_refresh", "refresh_view", "custom_size_swipe", "custom_swipe_item_view",
    "basic_swipe",
    "gestures", "pan_gesture_events", "pointer_gesture", "drag_drop", "hit_testing",
    "input_transparent", "focus", "dispatcher", "device", "effects", "measure_first_strategy",
    "scroll_view", "web_view", "hybrid_web_view",
    "alerts", "animation", "application_control",
    "ios_entry", "ios_date_picker", "ios_time_picker", "ios_picker", "ios_search_bar", "ios_scroll_view",
    "ios_slider_update_on_tap", "ios_first_responder", "ios_pan_gesture", "ios_safe_area",
    "ios_swipe_transition", "ios_blur_effect",
    "navigation_gallery", "modal", "tabbed_flyout", "toolbar", "menu_bar", "title_bar", "chrome",
    "context_flyout", "templated_view", "custom_layout", "visual_states",
]

TITLE = {"controls_stack": "Control stack", "alignment": "Layout alignment (Start/Center/End/Fill)",
         "collectionview": "CollectionView", "gradient": "Gradient brushes", "cv_visual_states": "CV visual states"}

CAT_LABEL = {
    "pixel_perfect": "🟢 Pixel-perfect", "match": "🟢 Match",
    "cpp_minor": "🟡 C++ minor", "cpp_major": "🔴 C++ major", "cpp_blank": "⬛ C++ blank",
    "maui_minor": "🟠 MAUI minor", "maui_major": "🟣 MAUI major", "maui_blank": "◻️ MAUI blank",
    "pending": "⬜ pending",
}
SEV = {"pixel_perfect": 0, "match": 0, "cpp_minor": 1, "maui_minor": 1, "cpp_major": 2, "maui_major": 2,
       "cpp_blank": 3, "maui_blank": 3, "pending": -1}


def title_of(key):
    return TITLE.get(key) or key.replace("_", " ").title()


def description(key):
    """First clause of the page header comment (after the em-dash), else a generic line."""
    path = os.path.join(PAGES, f"{key}_page.hpp")
    if not os.path.exists(path):
        return ""
    txt = open(path, encoding="utf-8", errors="replace").read()
    # join the leading // comment block, find "— <desc>"
    head = []
    for ln in txt.splitlines():
        s = ln.strip()
        if s.startswith("//"):
            head.append(s.lstrip("/").strip())
        elif head:
            break
    blob = " ".join(head)
    m = re.search(r"[—-]{1,2}\s*(.+?)(?:\.\s|\.$|$)", blob)
    desc = (m.group(1) if m else blob).strip()
    desc = re.sub(r"\s+", " ", desc)
    return desc[:240]


def load_sonnet():
    d = {}
    sdir = os.path.join(CMP, "analysis_sonnet")
    if os.path.isdir(sdir):
        for f in os.listdir(sdir):
            if f.endswith(".json"):
                try:
                    d[f[:-5]] = json.load(open(os.path.join(sdir, f)))
                except (OSError, ValueError):
                    pass
    return d


FAM = {"pixel_perfect": "ok", "match": "ok", "cpp_minor": "minor", "maui_minor": "minor",
       "cpp_major": "major", "maui_major": "major", "cpp_blank": "blank", "maui_blank": "blank"}


def consensus(key, diff, gem, son):
    """Agreement-based + diff-aware consensus (the two models diverge ~58%, so worst-of over-flags):
    - pixel_perfect: deterministic SSIM>=threshold both themes AND neither model sees a real defect.
    - blank: either model reports a blank side.
    - match: both models say ok.
    - major: BOTH models say major (high-confidence problem).
    - minor: everything else (one model major + one not, or both minor) — the uncertain middle.
    Side (C++ vs MAUI) follows the models: maui_* only if every verdict is MAUI-side."""
    cats = [v["category"] for v in (gem, son) if v and v.get("category")]
    if not cats:
        return "pending"
    fams = [FAM.get(c, "minor") for c in cats]
    dd = diff.get(key, {})
    ssims = [dd.get(t, {}).get("ssim") or 0 for t in ("light", "dark")]
    maui_side = all(c.startswith("maui") for c in cats)
    if any(f == "blank" for f in fams):
        return next(c for c in cats if FAM.get(c) == "blank")
    if all(f == "ok" for f in fams) and all(s >= PP_SSIM for s in ssims):
        return "pixel_perfect"
    if all(f == "ok" for f in fams):
        return "match"
    if all(f == "major" for f in fams):
        return "maui_major" if maui_side else "cpp_major"
    return "maui_minor" if maui_side else "cpp_minor"


DEMO_H = 350  # px height per screenshot


def md_cell(s):
    """Make free text safe inside a Markdown table cell: escape HTML + the cell-delimiter pipe."""
    return html.escape(s).replace("|", "&#124;")


def cell(v):
    """Model-verdict cell for a Markdown table: bold category + model + bulleted notes (pipe-safe)."""
    if not v:
        return "⬜ pending"
    notes = "".join(f"<br>• {md_cell(n)}" for n in v.get("notes", [])[:4])
    return f"**{CAT_LABEL.get(v['category'], v['category'])}**<br><sub>{v.get('model', '')}</sub>{notes}"


def _img(d, key):
    """One demo image — the animated GIF if present, else the still PNG, at a uniform height."""
    ext = "gif" if (key in ANIMATED and os.path.exists(os.path.join(CMP, "captures", d, f"{key}.gif"))) else "png"
    return f'<img src="captures/{d}/{key}.{ext}" height="{DEMO_H}">'


def demo(key):
    """2x2 of the NATIVE captures as plain <img> tags (no nested <table> — that breaks GitHub's
    Markdown-cell rendering): top row light (MAUI, C++), bottom row dark (MAUI, C++)."""
    return (f'{_img("maui_light", key)} {_img("cpp_light", key)}<br>'
            f'{_img("maui_dark", key)} {_img("cpp_dark", key)}')


def main():
    diff = json.load(open(os.path.join(CMP, "diff_results.json")))
    gem = json.load(open(os.path.join(CMP, "analysis_gemini.json"))) if os.path.exists(
        os.path.join(CMP, "analysis_gemini.json")) else {}
    son = load_sonnet()
    keys = [ln.strip() for ln in open(os.path.join(HERE, "page_keys.txt")) if ln.strip()]
    rank = {k: i for i, k in enumerate(FIX_ORDER)}
    keys.sort(key=lambda k: (rank.get(k, len(FIX_ORDER)), k))

    # classification summary
    summary = {c: [] for c in ("pixel_perfect", "match", "cpp_minor", "cpp_major", "cpp_blank",
                               "maui_minor", "maui_major", "maui_blank", "pending")}
    finals = {}
    gmodel = next((v.get("model") for v in gem.values() if v.get("model")), "gemini-2.5-flash")
    for k in keys:
        c = consensus(k, diff, gem.get(k), son.get(k))
        finals[k] = c
        summary[c].append(k)

    today = datetime.date.today().isoformat()
    L = []
    L.append("# iOS visual parity — C++ MAUI port vs real .NET MAUI\n")
    L.append("Single source of truth for iOS pixel parity. Each example is rendered by BOTH the C++ port "
             "and real .NET MAUI on the **same** simulator, then compared deterministically (pixel diff + "
             "SSIM) and categorized by two independent vision models.\n")
    L.append("## Setup\n")
    L.append(f"- **Simulator:** {SIM} · UDID `{UDID}` · both apps on the same instance, no Display Zoom.")
    L.append("- **C++ port:** `dev.maui-cpp.ios-gallery` (current branch build).")
    L.append("- **.NET MAUI baseline:** `com.companyname.mauicompare` on **MAUI 10.0.71** (stable; matches "
             "git tag `10.0.71` used as the C# oracle). The app declares `UILaunchScreen` so it renders "
             "native (a missing one previously letterboxed it ~1.25×).")
    L.append(f"- **Deterministic diff:** Pillow per-pixel delta + ImageMagick DSSIM→SSIM, status-bar clock "
             f"masked. \"Pixel-perfect\" = SSIM ≥ {PP_SSIM} both themes (byte-exact 0 is unreachable "
             f"cross-engine due to anti-aliasing).")
    L.append(f"- **Vision models:** Claude Sonnet `{MODEL_SONNET}` and Gemini `{gmodel}` (exact id per row).")
    L.append("- **Animated pages** (🎬) carry GIFs (+ mp4 archive in `captures/`); interactive-only pages "
             "show the idle frame.")
    L.append(f"- **Generated:** {today}. Diff data: `diff_results.json`; per-model: `analysis_*`.\n")

    L.append("## Summary\n")
    L.append(f"172 examples · pixel-perfect-grade **{len(summary['pixel_perfect'])}** + match "
             f"**{len(summary['match'])}**. The two vision models judge independently and diverge often "
             "(Gemini runs stricter), so the **consensus** below is conservative: a page is *major* only "
             "when BOTH models flag it, *pixel-perfect/match* only when both agree (+ SSIM), and the "
             "uncertain middle falls to *minor*. The per-page Sonnet & Gemini columns carry the raw, "
             "independent verdicts.\n")
    L.append("**Consensus classification:**\n")
    L.append("| Classification | Count | Examples |")
    L.append("| --- | --- | --- |")
    for c in ("pixel_perfect", "match", "cpp_minor", "cpp_major", "cpp_blank",
              "maui_minor", "maui_major", "maui_blank", "pending"):
        ks = summary[c]
        if ks:
            shown = ", ".join(ks[:24]) + (f" … (+{len(ks)-24})" if len(ks) > 24 else "")
            L.append(f"| {CAT_LABEL[c]} | {len(ks)} | {shown} |")
    L.append("")
    # per-model independent distributions (transparency on divergence)
    from collections import Counter
    sc = Counter(son[k]["category"] for k in keys if k in son)
    gc = Counter(gem[k]["category"] for k in keys if k in gem)
    L.append(f"**Per-model distribution** (independent; family agreement ≈ "
             f"{round(100*sum(1 for k in keys if k in son and k in gem and FAM.get(son[k]['category'])==FAM.get(gem[k]['category']))/len(keys))}%):\n")
    L.append(f"| Category | Sonnet `{MODEL_SONNET}` | Gemini `{gmodel}` |")
    L.append("| --- | --- | --- |")
    for c in ("pixel_perfect", "match", "cpp_minor", "cpp_major", "cpp_blank",
              "maui_minor", "maui_major", "maui_blank"):
        if sc.get(c) or gc.get(c):
            L.append(f"| {CAT_LABEL[c]} | {sc.get(c, 0)} | {gc.get(c, 0)} |")
    L.append("")

    L.append("## Examples (simplest → most complex)\n")
    # Markdown table (renders reliably on GitHub). Example holds only the NAME (so its column stays
    # narrow, content-driven); the long description sits in its own column between Demo and Sonnet.
    L.append(f"| # | Example | Demo — MAUI ┃ C++ · light/dark | Description | Sonnet `{MODEL_SONNET}` | Gemini |")
    L.append("| --- | --- | --- | --- | --- | --- |")
    for i, k in enumerate(keys, 1):
        anim = " 🎬" if k in ANIMATED else ""
        L.append(f"| {i} | **{title_of(k)}**{anim} | {demo(k)} | {md_cell(description(k))} | "
                 f"{cell(son.get(k))} | {cell(gem.get(k))} |")
    L.append("")

    out = os.path.join(CMP, "README.md")
    open(out, "w", encoding="utf-8").write("\n".join(L))
    print(f"README -> {out}  ({len(keys)} rows; pixel-perfect={len(summary['pixel_perfect'])} "
          f"match={len(summary['match'])} sonnet={len(son)} gemini={len(gem)})")


if __name__ == "__main__":
    main()
