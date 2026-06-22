#!/usr/bin/env python3
# Regenerate README.md — the theme-matched iOS pixel-parity tracker.
#
# Compares the C++ port vs real .NET MAUI on iOS, theme-for-theme (light-vs-light, dark-vs-dark), from the
# 4-way capture dirs (csharp_ios_{light,dark} / cpp_ios_{light,dark}). Per-page parity status is tracked in
# parity_status.json ({key: {"light": <s>, "dark": <s>}}, s in match|minor|diff|blank|pending) and rendered
# as a glanceable grid + progress counter. Re-run after editing parity_status.json or re-capturing:
#   python3 docs/comparison/gen_parity_readme.py
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
STATUS_PATH = os.path.join(HERE, "parity_status.json")
# The Gemini/Claude visual-review verdicts (produced by tools/parity/run_parity.py, review-only). Rendered
# as an extra column alongside the human-tracked board (parity_status.json) — they are independent: the board
# is the curated source of truth; the review column is the latest automated second opinion.
REVIEW_PATH = os.path.join(HERE, "parity_review.json")
# A row height (px) forced on every screenshot so .NET MAUI and C++ render at the SAME height in the GitHub
# preview and can be compared directly (the raw captures differ in pixel height / aspect).
IMG_HEIGHT = 360
IMG_DIRS = ("csharp_ios_light", "cpp_ios_light", "csharp_ios_dark", "cpp_ios_dark")

# The order pages are tackled (top→bottom). Strategy: foundational single controls first — their handler
# fixes CASCADE to every page that uses them — then layouts, shapes, borders/clip, collection-views, radio,
# swipe, gestures, scroll/web, combos, iOS platform-specifics, and chrome/host pages last (hardest, lowest
# value). label/button lead (already in progress). Unlisted keys are appended alphabetically + warned.
FIX_ORDER = [
    # foundational single controls (cascade)
    "label", "button", "entry", "editor", "search_bar", "picker", "date_picker", "time_picker", "pickers",
    "slider", "stepper", "switch", "check_box", "progress_bar", "activity_indicator", "indicator",
    "image", "image_button", "box_view", "content_view", "containers", "controls_stack", "input_controls",
    # text / typography / styling
    "fonts", "formatted_text", "styles", "triggers", "behaviors", "semantics", "app_theme_binding",
    # layouts
    "stack_layout", "vertical_stack", "horizontal_stack", "grid", "absolute_layout", "flex_layout",
    "relative_layout", "alignment", "z_index", "layout_is_enabled",
    # shapes / graphics
    "shapes", "ellipse_gallery", "rectangle_gallery", "line_gallery", "line_join_gallery", "polygon_gallery",
    "polyline_gallery", "path_gallery", "path_aspect_gallery", "path_transform_string", "composition_gallery",
    "transform_playground", "transformations", "update_path_data", "auto_size_shapes", "shape_app_theme",
    "invalidate_brush", "gradient",
    # border / clip / shadow
    "border", "border_stroke", "border_layout", "border_playground", "border_clip_playground",
    "border_resize_content", "borderless", "clip", "clip_views", "clip_corner_radius", "clip_gallery",
    "clipping", "shadow_playground", "invalidate_shadow_host",
    # collection-view family
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
    # radio buttons
    "radio_button_group", "radio_button_group_binding", "radio_button_group_gallery", "radio_button_border",
    "radio_button_content", "radio_content_properties", "radio_template_from_style", "scattered_radio_button",
    # swipe / refresh
    "swipe_gesture", "swipe_item_position", "swipe_item_size", "swipe_threshold", "swipe_view_margin",
    "swipe_view_shadow", "swipe_refresh", "refresh_view", "custom_size_swipe", "custom_swipe_item_view",
    "basic_swipe",
    # gestures / interaction / misc
    "gestures", "pan_gesture_events", "pointer_gesture", "drag_drop", "hit_testing",
    "input_transparent", "focus", "dispatcher", "device", "effects", "measure_first_strategy",
    # scroll / web
    "scroll_view", "web_view", "hybrid_web_view",
    # feature combos
    "alerts", "animation", "application_control",
    # iOS platform-specifics
    "ios_entry", "ios_date_picker", "ios_time_picker", "ios_picker", "ios_search_bar", "ios_scroll_view",
    "ios_slider_update_on_tap", "ios_first_responder", "ios_pan_gesture", "ios_safe_area",
    "ios_swipe_transition", "ios_blur_effect",
    # chrome / host pages (hardest, lowest value) — last
    "navigation_gallery", "modal", "tabbed_flyout", "toolbar", "menu_bar", "title_bar", "chrome",
    "context_flyout", "templated_view", "custom_layout", "visual_states",
]
_present = {f[:-4] for f in os.listdir(os.path.join(HERE, "csharp_ios_light")) if f.endswith(".png")}
_rank = {k: i for i, k in enumerate(FIX_ORDER)}
_unlisted = sorted(_present - set(FIX_ORDER))
if _unlisted:
    print(f"WARNING: {len(_unlisted)} keys not in FIX_ORDER (appended): {_unlisted}")
KEYS = sorted(_present, key=lambda k: (_rank.get(k, len(FIX_ORDER)), k))

# Curated display titles (the 6 keys whose C# page name differs from the C++ gallery page); others title-cased.
TITLE = {
    "controls_stack": "Control stack",
    "alignment": "Layout alignment (Start/Center/End/Fill)",
    "shapes": "Shapes",
    "border": "Border",
    "collectionview": "CollectionView",
    "gradient": "Gradient brushes",
}
EMOJI = {"match": "🟢", "minor": "🟡", "diff": "🔴", "blank": "⬛", "pending": "⬜"}
LABEL = {"match": "match", "minor": "minor", "diff": "diff", "blank": "blank", "pending": "pending"}
# Per-page flags (orthogonal to the light/dark status): the .NET MAUI REFERENCE capture is itself broken
# (black/home-screen/garbled — needs a re-shoot before a fair comparison), and the page demonstrates MOTION
# or a transient EFFECT a single still frame can't fairly capture (needs an animated GIF to judge parity).
FLAG_EMOJI = {"maui_broken": "⚠️", "needs_gif": "🎬"}
FLAG_DESC = {"maui_broken": "MAUI reference capture broken — re-shoot needed",
             "needs_gif": "motion/effect — needs an animated GIF to judge"}


# The review column uses the same status vocabulary plus the tool's per-side blank verdicts.
REVIEW_EMOJI = {"match": "🟢", "minor": "🟡", "diff": "🔴", "cpp_blank": "⬛", "cs_blank": "◻️", "pending": "⬜"}


def load_review():
    """Map page-key -> review verdict from parity_review.json (empty if the sweep hasn't run)."""
    if not os.path.exists(REVIEW_PATH):
        return {}
    data = json.load(open(REVIEW_PATH))
    return {v["key"]: v for v in data.get("verdicts", []) if isinstance(v, dict) and v.get("key")}


def review_combined(v):
    pair = (v.get("light", "pending"), v.get("dark", "pending"))
    for worst in ("diff", "cpp_blank", "cs_blank", "minor"):
        if worst in pair:
            return worst
    return "match" if pair == ("match", "match") else "pending"


def review_cell(v):
    """A compact badge for the Gemini/Claude review column ('—' when a page hasn't been judged yet)."""
    if not v:
        return "—"
    rc = review_combined(v)
    sev = v.get("severity")
    src = v.get("model") or v.get("source")  # the judging model id (gemini-*/claude), recorded per page
    out = f"{REVIEW_EMOJI.get(rc, '⬜')}<br>L:{v.get('light', 'pending')}<br>D:{v.get('dark', 'pending')}"
    if sev:
        out += f"<br>_{sev}_"
    if src:
        out += f"<br><sub>{src}</sub>"
    return out


def _esc_cell(s):
    """Make note text safe inside a markdown table cell: no raw pipes / newlines."""
    return (s or "").replace("|", "\\|").replace("\r", " ").replace("\n", " ").strip()


def fmt_notes(ln, dn):
    """Render a reviewer's per-theme notes as a compact table-cell fragment (prefixed with <br>)."""
    ln, dn = _esc_cell(ln), _esc_cell(dn)
    if not ln and not dn:
        return ""
    if ln and dn and ln == dn:
        return f"<br>{ln}"
    parts = []
    if ln:
        parts.append(f"**L:** {ln}")
    if dn:
        parts.append(f"**D:** {dn}")
    return "<br>" + "<br>".join(parts)


def review_notes(v):
    """Gemini's per-theme notes from a review verdict (run_parity stores light_note/dark_note)."""
    if not v:
        return ""
    return fmt_notes(v.get("light_note", ""), v.get("dark_note", ""))


def img(rel):
    """A fixed-height <img> so MAUI and C++ captures line up at the same height in the GitHub preview."""
    return f'<img src="{rel}" height="{IMG_HEIGHT}">'


def title(k):
    return TITLE.get(k, k.replace("_", " ").title())


def flags_of(st):
    """Normalize the optional flag carriers in a status entry into a flag-key list."""
    out = []
    raw = st.get("flags") or []
    if st.get("maui_capture_suspect") or "maui_broken" in raw:
        out.append("maui_broken")
    if st.get("needs_motion_gif") or "needs_gif" in raw:
        out.append("needs_gif")
    return out


def load_status():
    data = json.load(open(STATUS_PATH)) if os.path.exists(STATUS_PATH) else {}
    changed = False
    for k in KEYS:
        if k not in data:
            data[k] = {"light": "pending", "dark": "pending"}
            changed = True
    if changed:
        json.dump(data, open(STATUS_PATH, "w"), indent=2, sort_keys=True)
    return data


def combined(st):
    pair = (st.get("light", "pending"), st.get("dark", "pending"))
    if "diff" in pair or "blank" in pair:
        return "diff" if "diff" in pair else "blank"
    if "minor" in pair:
        return "minor"
    if pair == ("match", "match"):
        return "match"
    return "pending"


def main():
    status = load_status()
    review = load_review()
    counts = {s: 0 for s in EMOJI}
    flag_counts = {f: 0 for f in FLAG_EMOJI}
    for k in KEYS:
        counts[combined(status[k])] += 1
        for f in flags_of(status[k]):
            flag_counts[f] += 1
    total = len(KEYS)
    o = []
    o.append("# C++ port vs .NET MAUI — iOS pixel-parity tracker")
    o.append("")
    o.append("Theme-matched iOS comparison: each page rendered by **real .NET MAUI** vs the **C++ port**, on the "
             "same iPhone 17 simulator, compared **light-vs-light** and **dark-vs-dark**. Both stacks render "
             "native-default controls + the system font (the C# app's `dotnet new maui` default `Styles.xaml` + "
             "OpenSans are stripped; appearance forced via `MAUI_THEME` / `MAUI_APPEARANCE`). Goal: pixel-perfect "
             "parity, fixed example-by-example.")
    o.append("")
    o.append(f"**Progress: {counts['match']} / {total} 🟢 matched** "
             f"· {counts['minor']} 🟡 minor · {counts['diff'] + counts['blank']} 🔴 diff · {counts['pending']} ⬜ pending")
    o.append("")
    o.append(f"**Flags: {flag_counts['maui_broken']} ⚠️ broken MAUI reference captures (re-shoot needed) "
             f"· {flag_counts['needs_gif']} 🎬 motion/effect pages needing an animated GIF to judge.**")
    o.append("")
    o.append("Status legend: 🟢 pixel-match (both themes) · 🟡 minor diff · 🔴 notable diff to fix · "
             "⬛ C++ renders blank · ⬜ not yet reviewed · ⚠️ MAUI reference capture itself is broken "
             "(re-shoot needed) · 🎬 motion/effect page — a still frame can't judge it; needs a GIF. Per-theme "
             "verdicts + per-page notes in `parity_status.json`; the **Per-page findings** section below lists "
             "every non-matching page's concrete diffs.")
    o.append("")
    o.append("> macOS / Mac Catalyst 4-way comparison is **Phase 2** (pending: aligning the gallery window size "
             "to the C# window). The earlier 2-way macOS grid + notes live in [PARITY_FINDINGS.md](PARITY_FINDINGS.md).")
    o.append("")
    o.append("Rows are in **fix order** (top → bottom): foundational single controls first (their fixes "
             "cascade), then layouts, shapes, borders/clip, collection-views, radio, swipe, gestures, "
             "scroll/web, combos, iOS-specifics, and chrome/host pages last.")
    o.append("")
    reviewed = sum(1 for k in KEYS if k in review)
    o.append(f"Each page carries **two independent visual reviews**, both from a fresh capture of the same images: "
             f"the **Claude review** column (Opus 4.x vision, the curated board in `parity_status.json` — drives the "
             f"progress counter + fixes) and the **Gemini review** column (Gemini's best-available model per "
             f"`tools/parity/run_parity.py`, quota-aware cascade, recorded in `parity_review.json`). Each cell shows "
             f"that reviewer's per-theme verdict (L/D) + its note. {reviewed}/{total} pages Gemini-reviewed; "
             f"**—** = not yet judged by Gemini.")
    if reviewed < total:
        o.append("")
        o.append(f"> ⏳ **Gemini sweep pending for {total - reviewed} page(s).** The free-tier daily quota was "
                 f"exhausted on the last run; the **—** pages are queued for a later Gemini pass when the quota "
                 f"resets. Meanwhile the **Status** column (the human/Claude-reviewed board) is authoritative and "
                 f"drives fixes.")
    o.append("")
    o.append("| # | Page | Claude review | Gemini review | .NET MAUI (light) | C++ (light) | .NET MAUI (dark) | C++ (dark) |")
    o.append("| --: | --- | :--- | :--- | :---: | :---: | :---: | :---: |")
    for i, k in enumerate(KEYS, start=1):
        st = status[k]
        c = combined(st)
        fl = "".join(FLAG_EMOJI[f] for f in flags_of(st))
        rv = review.get(k)
        claude_cell = (f"{EMOJI[c]}{fl}<br>L:{LABEL[st.get('light','pending')]}<br>D:{LABEL[st.get('dark','pending')]}"
                       + fmt_notes(st.get("light_note"), st.get("dark_note")))
        gemini_cell = review_cell(rv) + review_notes(rv)
        o.append(
            f"| {i} | {title(k)} | {claude_cell} | {gemini_cell} "
            f"| {img(f'csharp_ios_light/{k}.png')} | {img(f'cpp_ios_light/{k}.png')} "
            f"| {img(f'csharp_ios_dark/{k}.png')} | {img(f'cpp_ios_dark/{k}.png')} |"
        )
    o.append("")

    # ---- Per-page findings: concrete notes for every page that is not a clean both-themes match. ----
    o.append("## Per-page findings")
    o.append("")
    o.append("Concrete, per-theme notes for every page with a diff, a broken reference, or a motion/effect "
             "caveat. Clean both-theme matches with no note are omitted. Numbers match the grid above.")
    o.append("")
    any_finding = False
    for i, k in enumerate(KEYS, start=1):
        st = status[k]
        c = combined(st)
        fl = flags_of(st)
        ln = (st.get("light_note") or "").strip()
        dn = (st.get("dark_note") or "").strip()
        # Skip clean matches that carry no note and no flag.
        if c == "match" and not ln and not dn and not fl:
            continue
        any_finding = True
        flag_md = ("  " + " ".join(f"{FLAG_EMOJI[f]} _{FLAG_DESC[f]}_" for f in fl)) if fl else ""
        o.append(f"### {i}. {title(k)} — {EMOJI[c]} (L:{LABEL[st.get('light','pending')]} / "
                 f"D:{LABEL[st.get('dark','pending')]}){flag_md}")
        if ln and dn and ln == dn:
            o.append(f"- **Both themes:** {ln}")
        else:
            if ln:
                o.append(f"- **Light:** {ln}")
            if dn:
                o.append(f"- **Dark:** {dn}")
        if not ln and not dn:
            o.append("- _(no note recorded)_")
        o.append("")
    if not any_finding:
        o.append("_All pages are clean matches with no recorded notes._")
        o.append("")
    open(os.path.join(HERE, "README.md"), "w").write("\n".join(o))
    print(f"README: {total} pages — {counts['match']} matched, {counts['minor']} minor, "
          f"{counts['diff'] + counts['blank']} diff, {counts['pending']} pending")


if __name__ == "__main__":
    main()
