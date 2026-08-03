#!/usr/bin/env bash
# android-aar-lib.sh — shared AndroidX/Material linking for the two Android app-host APK builds
# (build_android_apphost.sh and build_android_apphost_xaml.sh). Sourced, not executed.
#
# Both hosts are built by a bare javac -> d8 -> aapt2 link pipeline (no gradle, no AGP). This file
# supplies the four things AGP would otherwise do for an AAR dependency:
#   1. unpack each AAR's classes.jar onto the javac classpath and into the d8 input set,
#   2. aapt2-compile each AAR's res/ tree and merge them all at link time,
#   3. emit an R.java per library package (--extra-packages) — an AAR carries no R class,
#   4. keep the whole lot cached so routine per-page capture iteration does not re-pay for it.
#
# After `maui_android_aar_prepare "<build_dir>/aardeps"`:
#   maui_aar_jars   array of jar paths — join with ':' for javac -classpath, pass directly to d8
#   maui_aar_link   array of aapt2-link args — the merged library resource units (first positional,
#                   the rest as -R overlays) plus one --extra-packages per resource-owning library.
#                   The caller appends `-R <its own compiled app res>` LAST so app resources win.
# The caller must also pass --auto-add-overlay and --java <gendir> to aapt2 link, and add every
# <gendir>/**/R.java to its javac source list.

maui_android_aar_prepare() {
  local cache="$1"
  local deps="${cpp_root}/tools/parity/android_aar_deps.txt"
  local stage="${cpp_root}/tools/parity/android_aar_stage.py"
  [[ -f "${deps}" ]] || maui_die "missing ${deps}"

  # Cache stamp: the dep list + the stager + the aapt2 binary. Any change re-stages from scratch.
  local stamp_want stamp_file="${cache}/.stamp"
  stamp_want="$(cat "${deps}" "${stage}" | shasum -a 256 | cut -d' ' -f1)-$(basename "$(dirname "${aapt2}")")"
  if [[ ! -f "${stamp_file}" || "$(cat "${stamp_file}")" != "${stamp_want}" ]]; then
    echo "[aar] staging AndroidX + Material from ~/.nuget/packages (first run / deps changed)..." >&2
    rm -rf "${cache}"
    mkdir -p "${cache}/jars" "${cache}/res" "${cache}/work" "${cache}/flat"
    python3 "${stage}" "${deps}" "${cache}" >&2 || maui_die "AAR staging failed"
    echo "[aar] aapt2 compile $(ls "${cache}/res" | wc -l | tr -d ' ') library resource set(s)..." >&2
    local d
    for d in "${cache}"/res/*; do
      "${aapt2}" compile --dir "${d}" -o "${cache}/flat/$(basename "${d}").zip" >&2 \
        || maui_die "aapt2 compile failed for $(basename "${d}")"
    done
    echo "${stamp_want}" > "${stamp_file}"
  fi

  maui_aar_jars=()
  local j
  for j in "${cache}"/jars/*.jar; do maui_aar_jars+=("${j}"); done
  [[ "${#maui_aar_jars[@]}" -gt 0 ]] || maui_die "no staged AAR jars under ${cache}/jars"

  # Only the FIRST resource unit is positional; the rest are -R overlays. Co-equal positional units
  # are a hard link error wherever two libraries define the same name (material + constraintlayout both
  # define styleable/Carousel; material + appcompat both define styleable/SearchView) — as overlays,
  # aapt2 merges them, which is the precedence model AGP's resource merger applies.
  maui_aar_link=()
  local f first=1
  for f in "${cache}"/flat/*.zip; do
    if [[ "${first}" -eq 1 ]]; then maui_aar_link+=("${f}"); first=0; else maui_aar_link+=(-R "${f}"); fi
  done
  local p
  while IFS= read -r p; do
    [[ -n "${p}" ]] && maui_aar_link+=(--extra-packages "${p}")
  done < "${cache}/packages.txt"
}
