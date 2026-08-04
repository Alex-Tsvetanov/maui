#!/usr/bin/env python3
"""Generate the thesis's data-driven LaTeX from the parity board.

The thesis must never carry hand-copied board numbers: they drift the moment a defect is fixed,
and a transcription error in a results chapter is indistinguishable from a fabricated result.
Everything numeric in Раздел VI is emitted here from `comparison.json` and `\\input{}`-ed.

Outputs (all under `generated/`, regenerable, git-ignorable):

    generated/board_tables.tex     the summary + per-lane detail tables
    generated/board_macros.tex     \\boardGreen{lane} etc. — single numbers for inline prose
    evidence/<platform>/<fw>/…     images the chapters \\includegraphics, see "GIFs" below

**GIFs.** 78 of the 4816 captures are animated `.gif` (gesture/animation pages). pdfLaTeX cannot
include GIF at all, so those pages would silently vanish from the evidence set. Each is converted
to a PNG of its FIRST frame instead, and the conversion is recorded in `generated/gif_frames.tex`
so the thesis can state plainly which figures are a single frame of an animation rather than a
still — an animated page shown as a still without saying so is a misrepresentation.

Everything else is symlinked, not copied: the board is ~2 GB and the thesis should not fork it.

Usage:
    python3 tools/gen_tex.py            # regenerate everything
    python3 tools/gen_tex.py --check    # verify outputs are current (CI / pre-submission)
"""
from __future__ import annotations

import argparse
import json
import re
import os
import shutil
import subprocess
import sys
from pathlib import Path

THESIS = Path(__file__).resolve().parent.parent
COMPARISON = THESIS.parent / "comparison"
JSON = COMPARISON / "comparison.json"
GEN = THESIS / "generated"
EVIDENCE = THESIS / "evidence"

# (json key, Bulgarian display name, whether it is pixel-scored)
LANES = [
    ("ios", "iOS", True),
    ("maccatalyst", "Mac Catalyst", True),
    ("android", "Android", True),
    ("windows", "Windows", True),
]
# (comparison.json verdict key, Bulgarian column head)
COLUMNS = [("pixel", "по код"), ("pixel_xaml", "от описанието")]
STATUS = [("green", "съвпада"), ("yellow", "незначителна"), ("red", "съществена"),
          ("blank", "липсващ кадър"), (None, "неоценен")]


def tex_escape(s: str) -> str:
    for a, b in (("\\", r"\textbackslash{}"), ("&", r"\&"), ("%", r"\%"), ("$", r"\$"),
                 ("#", r"\#"), ("_", r"\_"), ("{", r"\{"), ("}", r"\}"), ("~", r"\textasciitilde{}"),
                 ("^", r"\textasciicircum{}")):
        s = s.replace(a, b)
    return s



def breakable(name: str) -> str:
    """Long fully-qualified type names have no break points in \\texttt, so a p{} column cannot
    wrap them and they run past the margin — 1227 overfull boxes came from this one cause.
    Insert an explicit break opportunity after each separator (no hyphen is added, unlike \\-)."""
    out = tex_escape(name)
    # Separators after which a break is safe. `<` and `,` matter for generic type names such as
    # ObservableCollection<IGestureRecognizer>, which has no dot to break at and overflows alone.
    for sep in (r"\_", ".", "::", "<", ",", "$>$"):
        out = out.replace(sep, sep + r"\allowbreak{}")
    # Last resort for a single token with no separator at all
    # (HybridWebViewInvokeJavaScriptRequest): allow a break at each internal capital, which is
    # where a reader's eye segments the name anyway.
    out = re.sub(r"(?<=[a-z])(?=[A-Z])", r"\\allowbreak{}", out)
    return out


def load() -> list[dict]:
    return json.loads(JSON.read_text(encoding="utf-8"))


# --------------------------------------------------------------------------- evidence
def build_evidence(pages: list[dict]) -> list[tuple[str, str]]:
    """Symlink PNGs, convert GIF→first-frame PNG. Returns the list of converted (key, platform)."""
    converted: list[tuple[str, str]] = []
    magick = shutil.which("magick") or shutil.which("convert")
    for page in pages:
        for plat, pl in page["platforms"].items():
            for fw, themes in (pl.get("screenshots") or {}).items():
                for theme, rel in (themes or {}).items():
                    if not rel:
                        continue
                    src = (COMPARISON / rel).resolve()
                    if not src.exists():
                        continue
                    dst = EVIDENCE / plat / fw / f"{page['name']}_{theme}.png"
                    dst.parent.mkdir(parents=True, exist_ok=True)
                    if dst.is_symlink() or dst.exists():
                        dst.unlink()
                    if src.suffix.lower() == ".gif":
                        if not magick:
                            print(f"  ! no imagemagick; skipping {rel}", file=sys.stderr)
                            continue
                        # `[0]` = first frame. A still of an animation is honest ONLY if labelled,
                        # which is what gif_frames.tex is for.
                        subprocess.run([magick, f"{src}[0]", str(dst)], capture_output=True)
                        converted.append((page["name"], plat))
                    else:
                        os.symlink(src, dst)
    return converted


# --------------------------------------------------------------------------- tables
def counts(pages, lane, key) -> dict:
    c = {s: 0 for s, _ in STATUS}
    for p in pages:
        st = ((p["platforms"].get(lane) or {}).get(key) or {}).get("status")
        c[st if st in c else None] += 1
    return c


def summary_table(pages) -> str:
    """Lane x input-path as ROWS, verdicts as columns. The transposed shape (verdicts nested under
    each input path) overflowed \\textwidth at 10 numeric columns; this fits at full size, which
    matters because А.8 forbids shrinking a table below 10 pt."""
    out = [r"% ГЕНЕРИРАН ФАЙЛ — не редактирай. Източник: comparison.json, tools/gen_tex.py",
           r"\begin{table}[H]",
           r"\caption{Степен на съвпадение по платформена лента и входен път}",
           r"\label{tab:parity}", r"\centering",
           r"\begin{tabular}{@{}llrrr@{}}", r"\hline",
           r"\textbf{Лента} & \textbf{Входен път} & \textbf{съвпада} & "
           r"\textbf{незначителна} & \textbf{съществена} \\ \hline"]
    leftover = 0
    for lane, disp, scored in LANES:
        if not scored:
            continue
        for i, (key, cname) in enumerate(COLUMNS):
            c = counts(pages, lane, key)
            leftover += c["blank"] + c[None]
            out.append(f"{disp if i == 0 else ''} & {cname} & {c['green']} & "
                       f"{c['yellow']} & {c['red']} \\\\")
        out.append(r"\addlinespace[2pt]")
    out += [r"\hline", r"\end{tabular}", r"\end{table}", ""]
    if leftover:
        out.insert(-1, r"\noindent\small Незаснети или неоценени слотове: %d.\normalsize" % leftover)
    return "\n".join(out)


def detail_longtable(pages, lane, disp) -> str:
    """Приложение В: every interface on one lane, with both verdicts. `longtable` carries
    „продължение“ in the top-right on carry-over, per the faculty rules (А.8)."""
    out = [r"\begin{longtable}{@{}p{5.4cm}p{4.4cm}p{4.4cm}@{}}",
           r"\caption{Съвпадение по интерфейси --- %s}\label{tab:detail_%s}\\" % (disp, lane),
           r"\hline \textbf{Интерфейс} & \textbf{по код} & \textbf{от описанието} \\ \hline",
           r"\endfirsthead",
           r"\multicolumn{3}{r}{\small\emph{продължение}}\\",
           r"\hline \textbf{Интерфейс} & \textbf{по код} & \textbf{от описанието} \\ \hline",
           r"\endhead", r"\hline", r"\endfoot"]
    glyph = {"green": r"\cmark", "yellow": r"$\circ$", "red": r"\xmark", "blank": "---", None: "?"}
    for p in pages:
        pl = p["platforms"].get(lane) or {}
        a = (pl.get("pixel") or {}).get("status")
        b = (pl.get("pixel_xaml") or {}).get("status")
        if a is None and b is None:
            continue
        out.append(f"{tex_escape(p['title'])} & {glyph.get(a,'?')} & {glyph.get(b,'?')} \\\\")
    out += [r"\end{longtable}", ""]
    return "\n".join(out)



# --------------------------------------------------------------------------- public API
def api_longtable() -> str:
    """Приложение Б: the public type list, read from the `// maui::x  <=  Microsoft.Maui.Y`
    first-line comment every public header carries. Generated for the same reason the board
    tables are: a hand-kept correspondence list is stale the day after it is written."""
    inc = THESIS.parents[1] / "include" / "maui"
    rows = []
    for h in sorted(inc.rglob("*.hpp")):
        for line in h.read_text(errors="replace").splitlines()[:3]:
            if line.startswith("// maui::") and "<=" in line:
                ours, theirs = (x.strip() for x in line[3:].split("<=", 1))
                rows.append((h.parent.name, ours, theirs))
                break
    out = [r"\begin{longtable}{@{}L{1.7cm}L{5.9cm}L{6.5cm}@{}}",
           r"\caption{Публични типове и съответствието им с типовете на еталона}"
           r"\label{tab:api}\\",
           r"\hline \textbf{Слой} & \textbf{Система} & \textbf{Еталон} \\ \hline",
           r"\endfirsthead",
           r"\multicolumn{3}{r}{\small\emph{продължение}}\\",
           r"\hline \textbf{Слой} & \textbf{Система} & \textbf{Еталон} \\ \hline",
           r"\endhead", r"\hline", r"\endfoot"]
    for layer, ours, theirs in rows:
        # ~150 of the 604 comments describe the correspondence in prose rather than naming a type
        # ("the ObservableCollection<IGestureRecognizer>", "the C# `object` item of…"). Setting
        # those in \\ttfamily is both wrong and the source of the last unbreakable overflows —
        # they are prose and are set as prose.
        # Many comments append a parenthetical gloss ("… .Clipboard (static facade)"): split it off
        # so the TYPE is set in \\ttfamily and the gloss stays prose. Without this the whole cell
        # falls to the prose branch and the type loses its break opportunities.
        head, _, gloss = theirs.partition(" (")
        is_type = head[:1].isupper() and " " not in head.strip()
        if is_type:
            rhs = f"{{\\ttfamily\\small {breakable(head.strip())}}}"
            if gloss:
                rhs += r" \small (" + tex_escape(gloss.rstrip(")")) + ")"
        else:
            # Prose glosses still contain long dotted type names; they need the same break
            # opportunities, only the font differs.
            rhs = f"\\small {breakable(theirs)}"
        out.append(f"{{\\small {breakable(layer)}}} & {{\\ttfamily\\small {breakable(ours)}}} & {rhs} \\\\")
    out += [r"\end{longtable}", ""]
    return "\n".join(out) + f"% {len(rows)} типа\n"



# --------------------------------------------------------------------------- byte identity
def byte_identical(pages):
    """Frames where the reference and the system are byte-identical, i.e. pixel-perfect.

    Hand-written into the chapter once and it went stale within days (a lane was recaptured and
    the count changed from 69 to 0). Generated here so a recapture updates the thesis."""
    import hashlib
    H = lambda q: hashlib.sha256(open(q, "rb").read()).hexdigest()
    rows, macros = [], []
    for lane, disp, scored in LANES:
        if not scored:
            continue
        per = {}
        for theme in ("light", "dark"):
            n = 0
            for pg in pages:
                sc = (pg["platforms"].get(lane) or {}).get("screenshots") or {}
                a = (sc.get("maui") or {}).get(theme)
                b = (sc.get("cpp") or {}).get(theme)
                pa, pb = COMPARISON / (a or ""), COMPARISON / (b or "")
                if a and b and pa.exists() and pb.exists() and H(pa) == H(pb):
                    n += 1
            per[theme] = n
        rows.append((disp, per["light"], per["dark"]))
        macros.append(r"\expandafter\newcommand\csname byteId%s\endcsname{%d}"
                      % (lane.replace("_", ""), per["light"] + per["dark"]))
    tbl = [r"\begin{table}[H]",
           r"\caption{Кадри с побайтово съвпадение между еталона и системата}",
           r"\label{tab:byteid}", r"\centering",
           r"\begin{tabular}{@{}lrr@{}}", r"\hline",
           r"\textbf{Платформа} & \textbf{светла тема} & \textbf{тъмна тема} \\ \hline"]
    for disp, l, dk in rows:
        tbl.append(f"{disp} & {l} & {dk} \\\\")
    tbl += [r"\hline", r"\end{tabular}", r"\end{table}", ""]
    return "\n".join(tbl), "\n".join(macros) + "\n"



# --------------------------------------------------------------------------- heat map

# Цветова скала за топлинната карта: 1.00 зелено, 0.75 жълто, 0.50 (и по-ниско) червено,
# с линейна интерполация между тях. Смесва се към бяло, за да остане текстът четим върху фона.
GREEN, YELLOW, RED = (0.20, 0.65, 0.25), (0.98, 0.82, 0.10), (0.85, 0.20, 0.15)


def shade(v: float, wash: float = 0.45) -> tuple[float, float, float]:
    if v >= 0.75:
        t = min(1.0, (v - 0.75) / 0.25)
        base = tuple(y + (g - y) * t for g, y in zip(GREEN, YELLOW))
    else:
        t = max(0.0, (v - 0.50) / 0.25)
        base = tuple(r + (y - r) * t for y, r in zip(YELLOW, RED))
    return tuple(1 - (1 - c) * wash for c in base)


def score_of(pg, lane, key="pixel"):
    """Worst SSIM across the themes for one interface, one platform, one input path."""
    pl = (pg["platforms"].get(lane) or {}).get(key) or {}
    m = re.findall(r"SSIM ([0-9.]+)", pl.get("review") or "")
    return min(float(x) for x in m) if m else None


def heatmap(pages):
    """ONE table, an interface per row and a platform per column.

    Each cell carries BOTH input paths — code-first / from-markup — because showing only one of
    two available measurements (or the better of the two) is exactly the selection §5.3.4 argues
    against for the four comparisons. Shaded by the WORSE of the two, mirroring the rule that a
    verdict follows the worse theme."""
    lanes = [(l, d) for l, d, sc in LANES if sc]
    head = (r"\hline \textbf{Интерфейс}" + "".join(f" & \\textbf{{{d}}}" for _, d in lanes)
            + r" \\ \hline")
    out = [r"{\small",
           r"\begin{longtable}{@{}L{3.4cm}" + "c" * len(lanes) + r"@{}}",
           r"\caption{Съвпадение по интерфейси и платформи. Клетката носи структурното сходство "
           r"за ДВАТА входни пътя --- \emph{по код\,/\,от описанието}}\label{tab:heat}\\",
           head, r"\endfirsthead",
           r"\multicolumn{%d}{r}{\small\emph{продължение}}\\" % (len(lanes) + 1),
           head, r"\endhead", r"\hline", r"\endfoot"]
    for pg in pages:
        cells = []
        for lane, _ in lanes:
            a = score_of(pg, lane, "pixel")
            b = score_of(pg, lane, "pixel_xaml")
            vals = [x for x in (a, b) if x is not None]
            if not vals:
                cells.append(r"\cellcolor{gray!12}---")
                continue
            rgb = ",".join(f"{x:.3f}" for x in shade(min(vals)))
            f = lambda x: "---" if x is None else f"{x:.3f}"
            cells.append(f"\\cellcolor[rgb]{{{rgb}}}{f(a)}\\,/\\,{f(b)}")
        out.append(tex_escape(pg["title"]) + " & " + " & ".join(cells) + r" \\")
    out += [r"\end{longtable}", r"}", ""]
    return "\n".join(out)



# --------------------------------------------------------------------------- TTFF
def ttff_table():
    """Time-to-first-frame, from measurements.json (written by the recapture runner).

    Lanes the instrument cannot measure are printed as unmeasured WITH the reason — a lane that
    silently vanishes from a table reads as a lane that had nothing to report."""
    m = json.loads((COMPARISON / "measurements.json").read_text(encoding="utf-8"))
    t = m.get("ttff") or {}
    if not t:
        return ""
    disp = {l: d for l, d, _ in LANES}
    disp.update({"appkit": "AppKit"})
    cols = [("maui_xaml", "еталон"), ("cpp", "по код"), ("cpp_xaml", "от описанието")]
    out = [r"\begin{table}[H]",
           r"\caption{Време до първи кадър при студено стартиране (медиана; в скоби 95-и процентил)}",
           r"\label{tab:ttff}", r"\centering\small",
           r"\begin{tabular}{@{}lccc@{}}", r"\hline",
           r"\textbf{Платформа}" + "".join(f" & \\textbf{{{d}}}" for _, d in cols) + r" \\ \hline"]
    unmeasured = []
    for lane, rec in t.items():
        cells = []
        any_measured = False
        for key, _ in cols:
            r = rec.get(key) or {}
            if r.get("measured"):
                any_measured = True
                cells.append(f"{r['median_s']:.3f}\\,s ({r['p95_s']:.3f})".replace(".", "{,}"))
            else:
                cells.append("---")
        if any_measured:
            res = max((rec[k].get("poll_resolution_s", 0) for k, _ in cols
                       if (rec.get(k) or {}).get("measured")), default=0)
            out.append(f"{disp.get(lane, lane)}" + "".join(f" & {c}" for c in cells)
                       + f" \\\\ % res {res}")
        else:
            note = next((rec[k].get("reason") or rec[k].get("note") or rec[k].get("why") or ""
                         for k, _ in cols if rec.get(k)), "")
            unmeasured.append((disp.get(lane, lane), note))
    out += [r"\hline", r"\end{tabular}", r"\end{table}", ""]
    if unmeasured:
        out.append(r"\noindent\small Неизмерени платформи: "
                   + "; ".join(f"\\textbf{{{n}}} --- {tex_escape(w.split('.')[0].strip())}"
                               for n, w in unmeasured) + r".\normalsize" + "\n")
    return "\n".join(out)



# --------------------------------------------------------------------------- worst performers
# Reasons are the ONLY hand-maintained part: a score is data, a root cause is a finding. A page
# absent from this map prints "открит" — an honest default that cannot become a stale claim.
WORST_REASON = {
    "context_flyout":  "живо външно уеб съдържание",
    "hybrid_web_view": "страница за грешка на браузъра",
    "image":           "незареден отдалечен образ",
    "drag_drop":       "непокрита област: черна в еталона, боядисана в системата",
    "swipe_item_size": "равномерно отместване 32\\,px (изрязване от рамката)",
    "clip":            "заглавието на прозореца, не съдържанието",
}
WORST_CUTOFF = 0.90


def worst_table(pages):
    rows = []
    for pg in pages:
        for lane, disp, sc in LANES:
            if not sc:
                continue
            a, b = score_of(pg, lane, "pixel"), score_of(pg, lane, "pixel_xaml")
            vals = [x for x in (a, b) if x is not None]
            if vals and min(vals) < WORST_CUTOFF:
                rows.append((min(vals), a, b, pg["name"], disp))
    rows.sort()
    out = [r"\begin{table}[H]",
           r"\caption{Интерфейси със структурно сходство под %s}" % f"0{{,}}{int(WORST_CUTOFF*100)}",
           r"\label{tab:worst}", r"\centering\small",
           r"\begin{tabular}{@{}L{3.6cm}L{2.3cm}ccL{4.6cm}@{}}", r"\hline",
           r"\textbf{Интерфейс} & \textbf{Платформа} & \textbf{по код} & "
           r"\textbf{от опис.} & \textbf{Причина} \\ \hline"]
    f = lambda x: "---" if x is None else f"{x:.3f}".replace(".", "{,}")
    for _, a, b, name, disp in rows:
        out.append(f"{{\\ttfamily\\small {breakable(name)}}} & {disp} & {f(a)} & {f(b)} & "
                   f"{WORST_REASON.get(name, 'открит')} \\\\")
    out += [r"\hline", r"\end{tabular}", r"\end{table}", ""]
    out.append(r"\noindent\small Общо %d двойки под %s, от които %d под 0{,}75.\normalsize"
               % (len(rows), f"0{{,}}{int(WORST_CUTOFF*100)}",
                  sum(1 for r in rows if r[0] < 0.75)) + "\n")
    return "\n".join(out)



def worse_theme(pg, lane):
    """Which theme drags the verdict down — the figure must show THAT one. Showing the light
    frame for a page whose defect is dark-only illustrates nothing."""
    rv = ((pg["platforms"].get(lane) or {}).get("pixel") or {}).get("review") or ""
    got = {}
    for th, pat in (("light", r"Light: SSIM ([0-9.]+)"), ("dark", r"Dark: SSIM ([0-9.]+)")):
        m = re.search(pat, rv)
        if m:
            got[th] = float(m.group(1))
    return min(got, key=got.get) if got else "light"


def worst_figures(pages):
    """Side-by-side captures for every interface below the cutoff, WITH its scores in the column
    headings. A figure whose caption merely says "worst case" makes the reader trust the claim;
    a figure carrying the two measured numbers lets them check it against the table."""
    rows = []
    for pg in pages:
        for lane, disp, sc in LANES:
            if not sc:
                continue
            a, b = score_of(pg, lane, "pixel"), score_of(pg, lane, "pixel_xaml")
            vals = [x for x in (a, b) if x is not None]
            if vals and min(vals) < WORST_CUTOFF:
                rows.append((min(vals), a, b, pg, lane, disp))
    rows.sort(key=lambda r: r[0])
    out = []
    f = lambda x: "---" if x is None else f"{x:.3f}".replace(".", "{,}")
    for _, a, b, pg, lane, disp in rows:
        th = worse_theme(pg, lane)
        thb = "светла" if th == "light" else "тъмна"
        key = pg["name"]
        reason = WORST_REASON.get(key, "открит")
        out.append(
            r"\begin{figure}[H]\centering" "\n"
            r"\begin{tabular}{@{}cc@{}}" "\n"
            r"\small\textbf{еталон} & \small\textbf{система} "
            + f"({f(a)}\\,/\\,{f(b)})" + r" \\" "\n"
            + f"\\capture{{evidence/{lane}/maui/{key}_{th}.png}} &\n"
            + f"\\capture{{evidence/{lane}/cpp/{key}_{th}.png}} \\\\\n"
            + r"\end{tabular}" "\n"
            + f"\\caption{{\\code{{{breakable(key)}}} --- {disp}, {thb} тема. "
              f"Структурно сходство {f(a)} по код и {f(b)} от описанието. {reason.capitalize()}.}}\n"
            + f"\\label{{fig:worst-{lane}-{key}}}\n"
            + r"\end{figure}" "\n")
    return "\n".join(out)


def macros(pages) -> str:
    """Single numbers for inline prose, so a sentence cannot disagree with a table."""
    out = [r"% ГЕНЕРИРАН ФАЙЛ — не редактирай.",
           r"\newcommand{\boardPages}{%d}" % len(pages)]
    for lane, disp, scored in LANES:
        if not scored:
            continue
        c = counts(pages, lane, "pixel")
        name = lane.replace("_", "")
        out.append(r"\expandafter\newcommand\csname boardGreen%s\endcsname{%d}" % (name, c["green"]))
        out.append(r"\expandafter\newcommand\csname boardYellow%s\endcsname{%d}" % (name, c["yellow"]))
        out.append(r"\expandafter\newcommand\csname boardRed%s\endcsname{%d}" % (name, c["red"]))
    return "\n".join(out) + "\n"


def gif_note(converted) -> str:
    if not converted:
        return "% няма преобразувани анимации\n"
    keys = sorted({k for k, _ in converted})
    head = "% ГЕНЕРИРАН ФАЙЛ — не редактирай.\n"
    return (head
            + "\\newcommand{\\gifPageCount}{" + str(len(keys)) + "}\n"
            + "\\newcommand{\\gifPageList}{" + ", ".join(tex_escape(k) for k in keys) + "}\n")


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true", help="fail if the outputs are stale")
    a = ap.parse_args(argv)

    pages = load()
    GEN.mkdir(exist_ok=True)
    outputs = {
        GEN / "board_tables.tex": summary_table(pages),
        GEN / "heatmap.tex": heatmap(pages),
        GEN / "board_macros.tex": macros(pages),
        GEN / "api_table.tex": api_longtable(),
        GEN / "ttff_table.tex": ttff_table(),
        GEN / "worst_table.tex": worst_table(pages),
        GEN / "worst_figures.tex": worst_figures(pages),
    }
    if a.check:
        stale = [p.name for p, t in outputs.items() if not p.exists() or p.read_text() != t]
        print("СТАРИ:" if stale else "актуални", *stale or ["(всички)"])
        return 1 if stale else 0

    bt, bm = byte_identical(pages)
    outputs[GEN / "byteid_table.tex"] = bt
    outputs[GEN / "board_macros.tex"] += bm
    converted = build_evidence(pages)
    outputs[GEN / "gif_frames.tex"] = gif_note(converted)
    for path, text in outputs.items():
        path.write_text(text, encoding="utf-8")
        print(f"  wrote {path.relative_to(THESIS)}")
    print(f"  evidence: {sum(1 for _ in EVIDENCE.rglob('*.png'))} images "
          f"({len(converted)} GIF перви кадри)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
