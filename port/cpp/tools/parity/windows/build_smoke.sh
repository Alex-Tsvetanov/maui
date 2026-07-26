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
# -static... : link libstdc++/libgcc statically so the .exe is SELF-CONTAINED — the guest has no mingw
#              runtime DLLs, and a missing libstdc++-6.dll fails at launch with a dialog that the
#              runner would report only as "process exited early".
"${CXX}" -std=c++23 -O2 -municode -mwindows \
  -static-libstdc++ -static-libgcc \
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
echo "[smoke] ok — deploy + drive it on the VM with:"
echo "         tools/parity/windows/vm_smoke.py --host <vm> --user <user>"
