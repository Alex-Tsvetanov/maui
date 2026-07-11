#!/usr/bin/env python3
"""E2E visual-comparison test runner — host orchestrator.

Deploys the three framework columns (maui_xaml / cpp / cpp_xaml) to a test environment (a macOS UTM
VM over SSH, to start), sets a fixed display resolution, cleans remote staging, then for each example
tag runs a declarative scenario (clicks/scroll/typing at absolute coordinates), captures the app
window after every step, lays the images out under

    port/cpp/docs/comparison/<YYYY-MM-DD-HH_MM_SS>/<tag>/<platform>/<column>/NNNN.png

(+ an NNNN.json sidecar per shot) and pixel-scores maui_xaml vs cpp / cpp_xaml (reusing
port/cpp/tools/parity/pixel_score.py).

The macOS OS primitives live in the guest agent (vm_agent_macos.py), driven over SSH. Interaction is
behind a small driver interface: CoordinateDriver (cliclick, the default) and DevFlowDriver (MAUI
DevFlow CLI for tap-by-automationId, falling back to coordinates). See tools/README_e2e.md.

Usage:
    python3 run_comparison.py --config config/local.example.toml            # all tags, all envs
    python3 run_comparison.py --config <cfg> --only counter,entry           # a few tags
    python3 run_comparison.py --config <cfg> --plan                         # validate + print, no SSH
"""
from __future__ import annotations

import argparse
import json
import os
import posixpath
import shlex
import subprocess
import sys
import time
import tomllib
from datetime import datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent          # …/docs/comparison/tools
COMP = HERE.parent                              # …/docs/comparison  (== pixel_score.COMP)
REPO = COMP.parents[3]                          # repo root (…/maui)
AGENT_SRC = HERE / "vm_agent_macos.py"
MANIFEST = REPO / "port/maui-reference/pages/manifest.json"

sys.path.insert(0, str(REPO / "port/cpp/tools/parity"))
import pixel_score  # noqa: E402  reuse score_theme()/classify()

# Status thresholds (from pixel_score.classify): SSIM>=0.98 & diff<=1% green; >=0.90 & <=8% yellow.


def git_commit() -> str:
    r = subprocess.run(["git", "-C", str(REPO), "rev-parse", "--short", "HEAD"],
                       capture_output=True, text=True)
    return r.stdout.strip() or "unknown"


def load_manifest() -> list[dict]:
    if not MANIFEST.is_file():
        return []
    return json.loads(MANIFEST.read_text())


class Env:
    """One test environment (a VM/emulator) + its SSH connection and column/tool config."""

    def __init__(self, name: str, cfg: dict):
        self.name = name
        self.cfg = cfg
        self.platform = cfg.get("platform", name)
        c = cfg["connection"]
        self.hostspec = f"{c['user']}@{c['host']}"
        self.connect_timeout = c.get("connect_timeout_seconds", 10)
        self.staging = cfg["staging"]["root"].rstrip("/")
        self.tools = cfg.get("tools", {})
        self.python3 = self.tools.get("python3", "/usr/bin/python3")
        self.display = cfg.get("display", {})
        self.columns = cfg["columns"]
        self.agent_remote = posixpath.join(self.staging, "vm_agent_macos.py")
        self.apps_remote = posixpath.join(self.staging, "apps")
        self.scratch = posixpath.join(self.staging, "scratch")

    # -- ssh plumbing -------------------------------------------------------
    def _ssh(self) -> list[str]:
        return ["ssh", "-o", "BatchMode=yes", "-o", f"ConnectTimeout={self.connect_timeout}", self.hostspec]

    def ssh_run(self, tokens: list[str], timeout: int = 120) -> subprocess.CompletedProcess:
        """Run a command on the VM (tokens are shell-quoted into one remote string)."""
        return subprocess.run(self._ssh() + [shlex.join(tokens)],
                              capture_output=True, text=True, timeout=timeout)

    def reachable(self) -> bool:
        return subprocess.run(self._ssh() + ["true"], capture_output=True).returncode == 0

    def mkdirs(self, remote: str) -> None:
        self.ssh_run(["/bin/mkdir", "-p", remote])

    def deploy(self, local: Path, remote: str) -> None:
        self.mkdirs(posixpath.dirname(remote))
        src = str(local) + ("/" if local.is_dir() else "")
        dst = f"{self.hostspec}:{remote}" + ("/" if local.is_dir() else "")
        subprocess.run(["rsync", "-a", "--delete", "-e", "ssh -o BatchMode=yes", src, dst], check=True)

    def pull(self, remote: str, local: Path) -> bool:
        local.parent.mkdir(parents=True, exist_ok=True)
        rc = subprocess.run(["scp", "-o", "BatchMode=yes", f"{self.hostspec}:{remote}", str(local)]).returncode
        return rc == 0 and local.is_file()

    # -- guest agent --------------------------------------------------------
    def agent(self, subcmd: str, *args, timeout: int = 120) -> dict:
        env_prefix = [f"{k}={v}" for k, v in {
            "MAUI_E2E_CLICLICK": self.tools.get("cliclick"),
            "MAUI_E2E_DISPLAYPLACER": self.tools.get("displayplacer"),
            "MAUI_E2E_SCREENCAPTURE": self.tools.get("screencapture"),
            "MAUI_E2E_OPEN": self.tools.get("open"),
        }.items() if v]
        tokens = ["/usr/bin/env", *env_prefix, self.python3, self.agent_remote, subcmd, *map(str, args)]
        r = self.ssh_run(tokens, timeout=timeout)
        return self._parse(r.stdout, r.stderr, r.returncode)

    @staticmethod
    def _parse(stdout: str, stderr: str, rc: int) -> dict:
        for line in reversed(stdout.strip().splitlines()):
            line = line.strip()
            if line.startswith("{"):
                try:
                    return json.loads(line)
                except json.JSONDecodeError:
                    break
        return {"ok": False, "error": f"no JSON from agent (rc={rc})", "stderr": stderr.strip()[:400]}


# --------------------------------------------------------------------------- drivers
class CoordinateDriver:
    """Absolute-coordinate interaction via the guest agent (cliclick). The default."""

    def __init__(self, env: Env):
        self.env = env

    def run_action(self, step: dict) -> None:
        action = step.get("action")
        if not action:
            return
        if action == "click":
            self.env.agent("click", *step["at"])
        elif action == "type":
            self.env.agent("type", step["text"])
        elif action == "scroll":
            x, y = step["at"]
            self.env.agent("scroll", x, y, step["dy"])
        else:
            raise ValueError(f"unknown scenario action: {action!r}")


class MauiDevFlowDriver(CoordinateDriver):
    """maui_xaml column: tap-by-automationId via Microsoft's MAUI DevFlow CLI; coordinates otherwise.

    EXPERIMENTAL: DevFlow's CLI surface changes between releases. Any DevFlow call that fails falls
    back to the coordinate path, so scenarios that use `at = [x, y]` (no automation_id) work today
    regardless of whether DevFlow is set up on the VM.
    """

    def __init__(self, env: Env, column_cfg: dict):
        super().__init__(env)
        self.cli = column_cfg.get("devflow_cli", "maui")

    def run_action(self, step: dict) -> None:
        if step.get("action") == "click" and step.get("automation_id"):
            try:
                r = self.env.ssh_run([self.cli, "devflow", "agent", "interact", "tap",
                                      "--automationid", step["automation_id"]], timeout=60)
                if r.returncode == 0:
                    return
                print(f"      maui-devflow tap failed (rc={r.returncode}); using coordinates")
            except Exception as e:
                print(f"      maui-devflow tap error ({e}); using coordinates")
        super().run_action(step)


class HttpDevFlowDriver(CoordinateDriver):
    """cpp / cpp_xaml columns: tap-by-automationId via the C++ port's in-app DevFlow agent.

    The agent (port/cpp/src/devflow) exposes JSON-over-HTTP on 127.0.0.1:<MAUI_DEVFLOW_PORT> — see
    port/cpp/docs/DEVFLOW_PROTOCOL.md. It only implements tap (v1), so type/scroll and coordinate taps
    fall back to cliclick. The app must be built with -DMAUI_DEVFLOW=ON and launched with
    MAUI_DEVFLOW_PORT set (the runner does this when a column's driver is "cpp_devflow"). Requests are
    issued with the VM's curl over SSH, so no persistent port-forward is needed.
    """

    def __init__(self, env: Env, column_cfg: dict):
        super().__init__(env)
        self.port = column_cfg.get("devflow_port")

    def run_action(self, step: dict) -> None:
        aid = step.get("automation_id")
        if step.get("action") == "click" and aid and self.port:
            body = json.dumps({"automation_id": aid})
            try:
                r = self.env.ssh_run(["/usr/bin/curl", "-s", "-X", "POST",
                                      f"http://127.0.0.1:{self.port}/tap", "-d", body], timeout=60)
                resp = json.loads(r.stdout.strip() or "{}")
                if r.returncode == 0 and resp.get("found"):
                    return
                print(f"      cpp-devflow tap miss ({r.stdout.strip()[:80]}); using coordinates")
            except Exception as e:
                print(f"      cpp-devflow tap error ({e}); using coordinates")
        super().run_action(step)


def make_driver(env: Env, column: str, column_cfg: dict):
    driver = column_cfg.get("driver", "coordinate")
    if driver == "maui_devflow":
        return MauiDevFlowDriver(env, column_cfg)
    if driver == "cpp_devflow":
        return HttpDevFlowDriver(env, column_cfg)
    return CoordinateDriver(env)


# --------------------------------------------------------------------------- scenarios
def load_scenario(scenarios_dir: Path, tag: str) -> dict:
    """The tag's scenario, or a one-step default (single idle screenshot) when absent."""
    f = scenarios_dir / f"{tag}.toml"
    if f.is_file():
        s = tomllib.loads(f.read_text())
        s.setdefault("steps", [{"name": "initial"}])
        s.setdefault("themes", ["light"])
        return s
    return {"tag": tag, "themes": ["light"], "steps": [{"name": "initial"}]}


# --------------------------------------------------------------------------- run
def columns_for(env: Env, tag: str, twin_keys: set[str] | None) -> list[str]:
    cols = []
    for col in env.columns:
        if col == "cpp" and twin_keys is not None and tag not in twin_keys:
            continue  # builder_twin:false — no code-first page (would capture a wrong-page fallback)
        cols.append(col)
    return cols


def run_env(env: Env, tags: list[str], scenarios_dir: Path, run_root: Path,
            settle: float, twin_keys: set[str] | None, commit: str) -> dict:
    print(f"[{env.name}] reachability check …")
    if not env.reachable():
        raise SystemExit(f"[{env.name}] SSH not reachable: ssh -o BatchMode=yes {env.hostspec} true failed")

    print(f"[{env.name}] deploy agent + clean scratch + set resolution")
    env.deploy(AGENT_SRC, env.agent_remote)
    env.agent("clean", env.scratch)
    if env.display:
        r = env.agent("set-resolution", env.display["width"], env.display["height"])
        if not r.get("ok"):
            print(f"  ! set-resolution: {r.get('error') or r.get('stderr')}")

    # Deploy each column's artifact once.
    for col, ccfg in env.columns.items():
        local = REPO / ccfg["artifact"]
        if not local.exists():
            print(f"  ! {col}: artifact missing on host: {local} (skipping this column)")
            ccfg["_missing"] = True
            continue
        remote = posixpath.join(env.apps_remote, col, local.name)
        print(f"  deploy {col}: {local.name}")
        env.deploy(local, remote)
        ccfg["_remote"] = remote

    remote_shot = posixpath.join(env.scratch, "shot.png")
    frames: dict[tuple, dict] = {}  # (tag, column, n) -> {theme, step, local}
    for tag in tags:
        scenario = load_scenario(scenarios_dir, tag)
        themes = scenario["themes"]
        for col in columns_for(env, tag, twin_keys):
            ccfg = env.columns[col]
            if ccfg.get("_missing"):
                continue
            driver = make_driver(env, col, ccfg)
            n = 0
            for theme in themes:
                theme_val = ccfg[f"theme_{theme}"]
                launch_env = [f"{ccfg['page_env']}={tag}", f"{ccfg['theme_env']}={theme_val}",
                              "MAUI_CAPTURE_TINT_NORMAL=1"]
                if ccfg.get("driver") == "cpp_devflow" and ccfg.get("devflow_port"):
                    launch_env.append(f"MAUI_DEVFLOW_PORT={ccfg['devflow_port']}")  # starts the in-app agent
                launch_args = ["launch", "--bundle", ccfg["_remote"], "--proc", ccfg["process"]]
                for kv in launch_env:
                    launch_args += ["--env", kv]
                res = env.agent(*launch_args)
                pid = res.get("pid")
                if pid is None:
                    print(f"  ! {tag}/{col}/{theme}: launch failed: {res.get('error')}")
                    continue
                time.sleep(settle)
                win = env.agent("window-id", pid)
                win_id, bounds = win.get("id", 0), win.get("bounds")
                try:
                    for step in scenario["steps"]:
                        driver.run_action(step)
                        time.sleep(settle)
                        n += 1
                        env.agent("shot", remote_shot, "--window", win_id or 0)
                        local = run_root / tag / env.platform / col / f"{n:04d}.png"
                        if not env.pull(remote_shot, local):
                            print(f"  ! {tag}/{col}/{theme}#{n}: capture pull failed")
                            continue
                        sidecar = {
                            "tag": tag, "platform": env.platform, "column": col, "theme": theme,
                            "step": step.get("name", f"step{n}"), "frame": n,
                            "window_bounds": bounds, "commit": commit,
                            "captured_at": datetime.now().astimezone().isoformat(),
                        }
                        local.with_suffix(".json").write_text(json.dumps(sidecar, indent=2))
                        frames[(tag, col, n)] = {"theme": theme, "step": sidecar["step"], "local": local}
                        print(f"  {tag}/{col}/{theme} {sidecar['step']:16} -> {local.relative_to(run_root)}")
                finally:
                    env.agent("stop", pid)
                    time.sleep(0.3)

    return score(env, tags, run_root, frames)


def score(env: Env, tags: list[str], run_root: Path, frames: dict) -> dict:
    """Pixel-score maui_xaml vs cpp / cpp_xaml per frame; return the per-column summary."""
    summary = {"platform": env.platform, "pages": {}}
    cpp_cols = [c for c in env.columns if c != "maui_xaml"]
    for tag in tags:
        maxframe = max((n for (t, c, n) in frames if t == tag), default=0)
        page = {}
        # per-column theme-grouped scores for a final classify()
        grouped: dict[str, dict] = {c: {} for c in cpp_cols}
        compare_dir = run_root / tag / env.platform / "compare"
        for n in range(1, maxframe + 1):
            maui = frames.get((tag, "maui_xaml", n))
            if not maui:
                continue
            report = {"frame": n, "step": maui["step"], "theme": maui["theme"], "scores": {}}
            for col in cpp_cols:
                other = frames.get((tag, col, n))
                if not other:
                    continue
                s = pixel_score.score_theme(str(maui["local"].relative_to(COMP)),
                                            str(other["local"].relative_to(COMP)))
                report["scores"][col] = s
                if s is not None:
                    grouped[col].setdefault(maui["theme"], []).append(s)
            compare_dir.mkdir(parents=True, exist_ok=True)
            (compare_dir / f"{n:04d}-report.json").write_text(json.dumps(report, indent=2))
        # worst-case classify per column across all its frames
        for col in cpp_cols:
            # reduce each theme's frames to its worst (min ssim / max diff), then classify the themes.
            theme_scores = {}
            for theme, lst in grouped[col].items():
                if lst:
                    theme_scores[theme] = {"ssim": min(x["ssim"] for x in lst),
                                           "diff_pct": max(x["diff_pct"] for x in lst)}
            if theme_scores:
                status, review = pixel_score.classify(theme_scores)
                page[col] = {"status": status, "review": review}
        if page:
            summary["pages"][tag] = page
    return summary


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="E2E visual-comparison runner")
    ap.add_argument("--config", required=True)
    ap.add_argument("--env", help="comma-separated environment names (default: all in config)")
    ap.add_argument("--only", help="comma-separated tags (default: all manifest pages minus gap_*)")
    ap.add_argument("--scenarios", default=str(COMP / "scenarios"), help="scenarios dir")
    ap.add_argument("--settle", type=float, default=1.0, help="seconds to settle after each action")
    ap.add_argument("--plan", action="store_true", help="validate config/scenarios and print the plan; no SSH")
    args = ap.parse_args(argv)

    cfg = tomllib.loads(Path(args.config).read_text())
    out_root = REPO / cfg.get("output", {}).get("root", "port/cpp/docs/comparison")
    ts_fmt = cfg.get("output", {}).get("timestamp_format", "%Y-%m-%d-%H_%M_%S")

    manifest = load_manifest()
    all_tags = [r["key"] for r in manifest if not r["key"].startswith("gap_")]
    twin_keys = {r["key"] for r in manifest if r.get("builder_twin", True)} or None
    tags = [t.strip() for t in args.only.split(",")] if args.only else all_tags
    if not tags:
        print("no tags to run (empty manifest and no --only)")
        return 2

    env_names = args.env.split(",") if args.env else list(cfg["environments"])
    scenarios_dir = Path(args.scenarios)

    if args.plan:
        print(f"plan: {len(tags)} tag(s) x envs {env_names}")
        for name in env_names:
            env = Env(name, cfg["environments"][name])
            print(f"  env {name}: platform={env.platform} columns={list(env.columns)} host={env.hostspec}")
        custom = [t for t in tags if (scenarios_dir / f"{t}.toml").is_file()]
        print(f"  {len(custom)} tag(s) have a scenario file; the rest get one idle screenshot: {custom[:20]}")
        return 0

    stamp = datetime.now().strftime(ts_fmt)
    run_root = out_root / stamp
    commit = git_commit()
    print(f"run {stamp}  ->  {run_root.relative_to(REPO)}  ({len(tags)} tag(s), commit {commit})")

    all_summaries = {}
    for name in env_names:
        summary = run_env(Env(name, cfg["environments"][name]), tags, scenarios_dir, run_root,
                          args.settle, twin_keys, commit)
        all_summaries[name] = summary

    (run_root / "summary.json").write_text(json.dumps(all_summaries, indent=2))
    (run_root / "run-manifest.json").write_text(json.dumps({
        "timestamp": stamp, "commit": commit, "tags": tags, "environments": env_names,
    }, indent=2))
    _write_report_md(run_root, all_summaries)
    print(f"\ndone -> {run_root.relative_to(REPO)} (summary.json, report.md)")
    return 0


def _write_report_md(run_root: Path, summaries: dict) -> None:
    lines = [f"# Comparison run {run_root.name}\n"]
    for env_name, summary in summaries.items():
        lines.append(f"## {env_name} ({summary['platform']})\n")
        lines.append("| Tag | Column | Status | Review |")
        lines.append("|---|---|---|---|")
        for tag, cols in sorted(summary["pages"].items()):
            for col, v in cols.items():
                lines.append(f"| {tag} | {col} | {v['status']} | {v['review']} |")
        lines.append("")
    (run_root / "report.md").write_text("\n".join(lines))


if __name__ == "__main__":
    sys.exit(main())
