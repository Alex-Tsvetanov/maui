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


def title(k):
    return TITLE.get(k, k.replace("_", " ").title())


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
    counts = {s: 0 for s in EMOJI}
    for k in KEYS:
        counts[combined(status[k])] += 1
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
    o.append("Status legend: 🟢 pixel-match (both themes) · 🟡 minor diff · 🔴 notable diff to fix · "
             "⬜ not yet reviewed. Per-theme verdicts in `parity_status.json`.")
    o.append("")
    o.append("> macOS / Mac Catalyst 4-way comparison is **Phase 2** (pending: aligning the gallery window size "
             "to the C# window). The earlier 2-way macOS grid + notes live in [PARITY_FINDINGS.md](PARITY_FINDINGS.md).")
    o.append("")
    o.append("Rows are in **fix order** (top → bottom): foundational single controls first (their fixes "
             "cascade), then layouts, shapes, borders/clip, collection-views, radio, swipe, gestures, "
             "scroll/web, combos, iOS-specifics, and chrome/host pages last.")
    o.append("")
    o.append("| # | Page | Status | .NET MAUI (light) | C++ (light) | .NET MAUI (dark) | C++ (dark) |")
    o.append("| --: | --- | :---: | --- | --- | --- | --- |")
    for i, k in enumerate(KEYS, start=1):
        st = status[k]
        c = combined(st)
        badge = f"{EMOJI[c]}<br>L:{LABEL[st.get('light','pending')]}<br>D:{LABEL[st.get('dark','pending')]}"
        o.append(
            f"| {i} | {title(k)} | {badge} "
            f"| ![](csharp_ios_light/{k}.png) | ![](cpp_ios_light/{k}.png) "
            f"| ![](csharp_ios_dark/{k}.png) | ![](cpp_ios_dark/{k}.png) |"
        )
    o.append("")
    open(os.path.join(HERE, "README.md"), "w").write("\n".join(o))
    print(f"README: {total} pages — {counts['match']} matched, {counts['minor']} minor, "
          f"{counts['diff'] + counts['blank']} diff, {counts['pending']} pending")


if __name__ == "__main__":
    main()
