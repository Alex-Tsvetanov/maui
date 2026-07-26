#!/usr/bin/env bash
# Cross-compile the Windows pipeline smoke app (smoke_window.cpp) from the macOS/Linux dev machine
# with mingw-w64 — no MSVC, no Windows SDK, no guest build. Produces a real PE32+ GUI executable that
# vm_agent_windows.py can launch/present/capture, so the whole VM pipeline is testable before the
# WinUI 3 backend exists (see docs/WINDOWS_TOOLCHAIN.md for why the smoke app is NOT a parity backend).
#
# Usage:
#   tools/parity/windows/build_smoke.sh                  # -> build/windows-smoke/maui_smoke.exe
#   tools/parity/windows/build_smoke.sh --out <dir>
#
# Requires: brew install mingw-w64   (provides x86_64-w64-mingw32-g++)
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cpp_root="$(cd "${script_dir}/../../.." && pwd)"   # port/cpp
out_dir="${cpp_root}/build/windows-smoke"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --out) out_dir="$2"; shift 2 ;;
    -h|--help) sed -n '1,14p' "${BASH_SOURCE[0]}"; exit 0 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

CXX="${MAUI_MINGW_CXX:-x86_64-w64-mingw32-g++}"
if ! command -v "${CXX}" >/dev/null 2>&1; then
  echo "error: ${CXX} not found. Install it with:  brew install mingw-w64" >&2
  exit 1
fi

mkdir -p "${out_dir}"
exe="${out_dir}/maui_smoke.exe"

# -municode  : wWinMain (wide entry) is the entry point
# -mwindows  : GUI subsystem, so launching does not open a console window that would appear in shots
# -static    : FULLY static — the .exe must be SELF-CONTAINED because a stock Windows guest has no mingw
#              runtime DLLs, and a missing one fails at launch with a modal dialog that the E2E runner
#              can report no more precisely than "process exited early".
#              `-static-libstdc++ -static-libgcc` alone is NOT enough: it leaves libwinpthread-1.dll as a
#              dynamic import (GCC's threading model), which was verified by objdump on this exact binary.
#              The import check below is what caught it, so it stays as a build-time gate.
"${CXX}" -std=c++23 -O2 -municode -mwindows \
  -static \
  -Wall -Wextra \
  "${script_dir}/smoke_window.cpp" \
  -lgdi32 -luser32 \
  -o "${exe}"

echo "[smoke] built ${exe}"
file "${exe}" || true

# Fail loudly if it is somehow not a Windows GUI binary — a silently-wrong artifact would be deployed
# to the guest and fail there, much further from the cause.
if ! file "${exe}" | grep -q 'PE32+ executable (GUI)'; then
  echo "error: ${exe} is not a PE32+ GUI executable" >&2
  exit 1
fi

# GATE: refuse to ship an .exe that imports a mingw runtime DLL. A stock Windows guest has none of them,
# so the app dies at launch behind a modal dialog and the runner reports only "process exited early" —
# a failure that costs a VM session to diagnose. api-ms-win-crt-* (the UCRT) is fine: it ships with
# Windows 10+. Anything named lib*.dll is a mingw runtime that must have been linked statically.
if objdump="$(command -v "${CXX%-g++}-objdump" || true)"; [[ -n "${objdump}" ]]; then
  bad="$("${objdump}" -p "${exe}" | awk '/DLL Name:/ {print $3}' | grep -i '^lib' || true)"
  if [[ -n "${bad}" ]]; then
    echo "error: ${exe} dynamically imports mingw runtime DLL(s):" >&2
    echo "${bad}" | sed 's/^/  /' >&2
    echo "  -> add -static (not just -static-libstdc++ -static-libgcc)" >&2
    exit 1
  fi
  echo "[smoke] import check: no mingw runtime DLLs (self-contained)"
else
  echo "[smoke] warning: objdump not found; skipped the self-contained import check" >&2
fi
echo "[smoke] ok — deploy + drive it on the VM with:"
echo "         tools/parity/windows/vm_smoke.py --host <vm> --user <user>"
