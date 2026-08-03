#!/usr/bin/env python3
"""Google Gemini vision client for the parity review (extracted from the retired gemini_*.py sweeps).

No third-party deps (urllib + base64) so it runs anywhere python3 does.
Key resolution: $GEMINI_API_KEY -> ~/.config/maui-parity/gemini_api_key (mode 600, outside the repo).

QUOTA is the interesting part. A full board sweep is thousands of image calls and the free tiers are
per-model daily caps, so a 429 is a NORMAL outcome, not a crash: `Quota` is raised, the caller rotates
to the next model in MODELS, and when they are all exhausted the run stops cleanly with whatever it
scored. A 404 (model not available to this key) rotates the same way — one bad model id must not kill
a sweep.
"""
from __future__ import annotations

import base64
import json
import os
import time
import urllib.error
import urllib.request

KEY_FILE = os.path.expanduser("~/.config/maui-parity/gemini_api_key")
# Cascade: a full-flash model first (reliable bucketing), then the higher-RPD lite model. NOT
# gemini-flash-latest — that resolves to a model capped at ~20 requests/day.
MODELS = ("gemini-2.5-flash", "gemini-2.0-flash", "gemini-2.0-flash-lite")


class Quota(Exception):
    """Rate-limited or model unavailable — rotate to the next model."""


class Failed(Exception):
    """Hard failure (bad response, repeated 5xx, unparseable JSON)."""


def read_key() -> str:
    key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not key:
        try:
            with open(KEY_FILE, encoding="utf-8") as fh:
                key = fh.read().strip()
        except OSError:
            key = ""
    if not key:
        raise Failed(f"no API key: set $GEMINI_API_KEY or write it to {KEY_FILE}")
    return key


def image_part(path: str, label: str) -> list[dict]:
    """A labelled image, as the two parts Gemini wants (the label matters — the prompts refer to
    the panes by name, and an unlabelled batch gets judged in the wrong order)."""
    mime = "image/gif" if path.endswith(".gif") else "image/png"
    with open(path, "rb") as fh:
        data = base64.b64encode(fh.read()).decode("ascii")
    return [{"text": f"Image: {label}"}, {"inline_data": {"mime_type": mime, "data": data}}]


def generate(model: str, api_key: str, parts: list, prompt: str, schema: dict,
             timeout: int = 120, retries: int = 2) -> dict:
    """One controlled-generation call. Returns the parsed JSON object the schema describes."""
    url = f"https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
    body = json.dumps({
        "contents": [{"parts": parts + [{"text": prompt}]}],
        "generationConfig": {"responseMimeType": "application/json",
                             "responseSchema": schema, "temperature": 0},
    }).encode("utf-8")

    last = ""
    for attempt in range(retries + 1):
        req = urllib.request.Request(url, data=body, method="POST",
                                     headers={"Content-Type": "application/json",
                                              "X-goog-api-key": api_key})
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                payload = json.loads(resp.read().decode("utf-8"))
                break
        except urllib.error.HTTPError as e:
            detail = e.read().decode("utf-8", "replace")
            if e.code == 429 or "RESOURCE_EXHAUSTED" in detail:
                raise Quota(f"{model}: rate limited ({e.code})")
            if e.code == 404:
                raise Quota(f"{model}: not available to this key (404)")
            last = f"HTTP {e.code}: {detail[:300]}"
            if 500 <= e.code < 600 and attempt < retries:
                time.sleep(2 ** attempt)
                continue
            raise Failed(last)
        except (urllib.error.URLError, TimeoutError) as e:
            last = f"network error: {e}"
            if attempt < retries:
                time.sleep(2 ** attempt)
                continue
            raise Failed(last)
    else:
        raise Failed(last or "no response")

    try:
        text = payload["candidates"][0]["content"]["parts"][0]["text"]
    except (KeyError, IndexError, TypeError):
        fb = payload.get("promptFeedback") or payload.get("candidates", [{}])[0].get("finishReason")
        raise Failed(f"no text in response (finish/feedback: {fb})")
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        raise Failed(f"response was not JSON: {text[:200]}")
