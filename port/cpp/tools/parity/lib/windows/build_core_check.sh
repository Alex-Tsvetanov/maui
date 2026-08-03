#!/usr/bin/env bash
# Cross-compile the port's CROSS-PLATFORM CORE for Windows and link a runnable probe.
#
# Why this exists: before any WinUI 3 backend work, we want to know that the port's platform-independent
# half — graphics, core, controls, layouts, hosting, plus the headless handler mirrors — actually
# compiles AND LINKS for Windows. A syntax check alone would miss missing symbols; a link proves the
# whole core resolves. The resulting probe binary also RUNS on the guest, so it is evidence the core
# behaves on Windows (it builds an app, mounts a window, drives a layout pass and prints measured
# geometry) rather than merely evidence that it compiled.
#
# This is Lane 2 (see docs/WINDOWS_TOOLCHAIN.md): mingw-w64, NOT a parity backend. The headless mirrors
# render nothing; they are here because MAUI_BACKEND=windows has no platform-source branch yet, so the
# headless handlers are what a Windows build currently links.
#
# Usage:
#   tools/parity/windows/build_core_check.sh              # build lib + probe
#   tools/parity/windows/build_core_check.sh --jobs 4
#   tools/parity/windows/build_core_check.sh --syntax-only # fast portability check, no objects
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cpp_root="$(cd "${script_dir}/../../../.." && pwd)"   # port/cpp
out_dir="${cpp_root}/build/windows-core"
jobs="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
syntax_only=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --jobs) jobs="$2"; shift 2 ;;
    --out) out_dir="$2"; shift 2 ;;
    --syntax-only) syntax_only=1; shift ;;
    -h|--help) sed -n '1,20p' "${BASH_SOURCE[0]}"; exit 0 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

CXX="${MAUI_MINGW_CXX:-x86_64-w64-mingw32-g++}"
AR="${MAUI_MINGW_AR:-x86_64-w64-mingw32-ar}"
command -v "${CXX}" >/dev/null || { echo "error: ${CXX} not found (brew install mingw-w64)" >&2; exit 1; }

cd "${cpp_root}"
obj_dir="${out_dir}/obj"
mkdir -p "${obj_dir}"

# The platform-independent core + the headless handler mirrors. Deliberately EXCLUDES:
#   src/platform/{apple,ios,apple_shared,android}  — other backends
#   src/platform/headless/host_run.cpp             — the probe supplies its own main/mount instead
#   src/devflow, src/xaml                          — optional subsystems with extra deps (pugixml)
#
# src/essentials and src/animations are REQUIRED, not optional: src/controls/state_trigger.cpp calls
# device_display::current() / device_info::current() / device_platform::create(), which are defined in
# src/essentials. Omitting them compiled fine and only failed at LINK time with undefined references —
# which is precisely why this script links a probe instead of stopping at a syntax check.
src_list="${out_dir}/sources.txt"
# `mapfile` is bash 4+ and macOS ships bash 3.2 — keep the list in a file rather than an array so this
# script runs on the stock dev-machine shell.
find src/graphics src/core src/controls src/layouts src/hosting src/animations src/essentials \
     src/platform/headless \
     -name '*.cpp' ! -name 'host_run.cpp' | sort > "${src_list}"
src_count="$(wc -l < "${src_list}" | tr -d ' ')"
echo "[core] ${src_count} sources, ${jobs} jobs, target x86_64-w64-mingw32"

fail_log="${out_dir}/failures.txt"
: > "${fail_log}"

# One compile. Everything it needs arrives through the environment (arrays do not survive `export`), and
# the flag string is re-split with `set --` so it works in any POSIX-ish shell.
compile_one() {
  src="$1"
  # shellcheck disable=SC2086  # deliberate word-splitting of the flag string
  set -f; IFS=' '; set -- ${MAUI_CXXFLAGS_STR}; unset IFS; set +f
  if [ "${MAUI_SYNTAX_ONLY:-0}" = "1" ]; then
    "${MAUI_CXX}" "$@" -fsyntax-only "${src}" || echo "FAIL ${src}" >> "${MAUI_FAIL_LOG}"
    return 0
  fi
  # Flatten the path into the object name so two same-named sources in different directories cannot
  # collide — src/core/*_handler.cpp and src/platform/headless/*_handler.cpp share MANY basenames, so a
  # basename-keyed object dir would silently overwrite half the objects and yield a short archive.
  obj="${MAUI_OBJ_DIR}/$(echo "${src}" | tr '/' '_' | sed 's/\.cpp$/.o/')"
  "${MAUI_CXX}" "$@" -c "${src}" -o "${obj}" || echo "FAIL ${src}" >> "${MAUI_FAIL_LOG}"
  return 0
}
export -f compile_one
export MAUI_CXX="${CXX}" MAUI_OBJ_DIR="${obj_dir}" MAUI_FAIL_LOG="${fail_log}"
export MAUI_SYNTAX_ONLY="${syntax_only}"
export MAUI_CXXFLAGS_STR="-std=c++23 -O1 -Iinclude -I. -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE -Wall -Wextra"

# `xargs -n1 bash -c '… "$0"'` puts the argument in $0 — no -I placeholder, so nothing inside the script
# string needs quoting or substitution.
# shellcheck disable=SC2016
xargs -P "${jobs}" -n1 bash -c 'compile_one "$0"' < "${src_list}"

fails=$(grep -c '^FAIL' "${fail_log}" 2>/dev/null || true)
fails=${fails:-0}
if [[ "${fails}" -gt 0 ]]; then
  echo "[core] ${fails} source(s) FAILED to compile for Windows:" >&2
  head -20 "${fail_log}" >&2
  exit 1
fi

if [[ "${syntax_only}" == "1" ]]; then
  echo "[core] ok — all ${src_count} sources are Windows-clean (syntax only)"
  exit 0
fi

objs=$(find "${obj_dir}" -name '*.o' | wc -l | tr -d ' ')
echo "[core] compiled ${objs} objects"
if [[ "${objs}" -ne "${src_count}" ]]; then
  echo "error: ${objs} objects for ${src_count} sources — object-name collision?" >&2
  exit 1
fi

lib="${out_dir}/libmaui_core_win.a"
rm -f "${lib}"
find "${obj_dir}" -name '*.o' -print0 | xargs -0 "${AR}" qcs "${lib}"
echo "[core] archived ${lib} ($(du -h "${lib}" | cut -f1))"

# ---- link the probe -------------------------------------------------------------------------------
probe_src="${script_dir}/core_probe.cpp"
probe="${out_dir}/maui_core_probe.exe"
# shellcheck disable=SC2086  # deliberate word-splitting of the shared flag string
"${CXX}" ${MAUI_CXXFLAGS_STR} "${probe_src}" "${lib}" -static -o "${probe}"
echo "[core] linked ${probe}"
file "${probe}"

# Same self-contained gate as build_smoke.sh: a mingw runtime DLL import would make this fail at launch
# on the guest with nothing but a modal dialog.
objdump_bin="$(command -v "${CXX%-g++}-objdump" || true)"
if [[ -n "${objdump_bin}" ]]; then
  bad="$("${objdump_bin}" -p "${probe}" | awk '/DLL Name:/ {print $3}' | grep -i '^lib' || true)"
  if [[ -n "${bad}" ]]; then
    echo "error: probe imports mingw runtime DLL(s):" >&2; echo "${bad}" | sed 's/^/  /' >&2
    exit 1
  fi
  echo "[core] import check: no mingw runtime DLLs (self-contained)"
fi
echo "[core] ok — run it on the guest:  ${probe##*/}"
