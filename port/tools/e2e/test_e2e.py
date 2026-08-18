#!/usr/bin/env python3
"""Focused regressions for the dependency-free E2E generator."""

from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import tempfile
import unittest


SPEC = importlib.util.spec_from_file_location("maui_e2e", Path(__file__).with_name("e2e.py"))
assert SPEC and SPEC.loader
E2E = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(E2E)


class BytesModeTest(unittest.TestCase):
    def test_preserves_hand_written_codebehind(self) -> None:
        with tempfile.TemporaryDirectory() as root:
            root = Path(root)
            pages, views, out = root / "pages", root / "Views", root / "out"
            pages.mkdir()
            views.mkdir()
            markup = '<ContentPage x:Class="MauiReference.Pages.MotionPage" />\n'
            (pages / "motion.xaml").write_text(markup, encoding="utf-8")
            (views / "motion.xaml.cpp").write_text(
                'constexpr unsigned char bytes[] = {\n#embed "motion.xaml"\n};\n'
                "void wire_codebehind() {}\n",
                encoding="utf-8",
            )

            old_pages, old_views = E2E.PAGES, E2E.VIEWS
            try:
                E2E.PAGES, E2E.VIEWS = str(pages), str(views)
                rc = E2E.cmd_gen(argparse.Namespace(embed_mode="bytes", out_dir=str(out)))
            finally:
                E2E.PAGES, E2E.VIEWS = old_pages, old_views

            generated = (out / "motion.xaml.cpp").read_text(encoding="utf-8")
            self.assertEqual(0, rc)
            self.assertIn("void wire_codebehind() {}", generated)
            self.assertNotIn("#embed", generated)
            self.assertIn("0x3c, 0x43, 0x6f, 0x6e", generated)


if __name__ == "__main__":
    unittest.main()
