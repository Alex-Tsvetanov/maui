#!/usr/bin/env python3
"""Stage the AndroidX/Material AARs named in android_aar_deps.txt out of the local NuGet cache.

The Android app hosts are built by a bare javac -> d8 -> aapt2 link pipeline (no gradle, no AGP), so
the AAR unpacking + resource-merge bookkeeping AGP would do has to happen here. Writes, under <cache>:

  jars/NN-<name>.jar   javac -classpath entries and d8 inputs, in dependency order
  res/NN-<name>/       symlink to the AAR's res/ tree (aapt2 compile --dir input), dependency order
  packages.txt         the library package names, for aapt2 link --extra-packages: an AAR's classes.jar
                       carries NO R class (it is generated per-app against the merged table), so every
                       library that owns resources needs its R.java emitted or its widgets hit
                       NoClassDefFoundError at inflate time.

Usage: android_aar_stage.py <deps.txt> <cache-dir>
"""
import os
import re
import sys
import zipfile

NUGET = os.path.expanduser("~/.nuget/packages")

# The d8/R8 in build-tools 34.0.0 (R8 8.2.2-dev) dies with
#   NullPointerException: Cannot invoke "String.length()" because "<parameter1>" is null
# on any MethodParameters attribute holding an unnamed ("<no name>") parameter — which the Kotlin 2.x
# compiler emits for mandated/synthetic parameters throughout androidx.collection, androidx.core and the
# Kotlin stdlib. 34.0.0 is the only build-tools installed and fetching a newer d8 is out of scope, so
# defuse the attribute instead: rename its constant-pool name to an equal-length unknown name. JVMS 4.7
# requires unrecognised attributes to be ignored, and MethodParameters is reflection-only metadata that
# dex does not encode, so nothing observable changes. The rewrite preserves every byte offset, and only
# fires where the bytes really are a CONSTANT_Utf8 entry of length 16 — a same-named *string* constant
# elsewhere in a class can never be hit. Drop this once a d8 >= 8.3 is available.
ATTR = b"MethodParameters"
ATTR_NEUTERED = b"MethodParameterZ"


def newest_version_dir(pkg):
    root = os.path.join(NUGET, pkg)
    if not os.path.isdir(root):
        raise SystemExit(f"missing NuGet package {pkg} (expected under {NUGET})")
    subs = sorted(d for d in os.listdir(root) if os.path.isdir(os.path.join(root, d)))
    if not subs:
        raise SystemExit(f"no version dir under {root}")
    return os.path.join(root, subs[-1])


def defuse_method_parameters(data):
    out = bytearray(data)
    i = 0
    while True:
        i = out.find(ATTR, i)
        if i < 0:
            return bytes(out)
        if i >= 3 and out[i - 3] == 1 and out[i - 2] * 256 + out[i - 1] == len(ATTR):
            out[i:i + len(ATTR)] = ATTR_NEUTERED
        i += len(ATTR)


def copy_jar(src, dst):
    with zipfile.ZipFile(src) as z:
        entries = [(i.filename, z.read(i.filename)) for i in z.infolist() if not i.is_dir()]
    if not any(ATTR in blob for _, blob in entries):
        with open(src, "rb") as s, open(dst, "wb") as d:
            d.write(s.read())
        return
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as out:
        for name, blob in entries:
            out.writestr(name, defuse_method_parameters(blob) if name.endswith(".class") else blob)


def has_classes(jar):
    with zipfile.ZipFile(jar) as z:
        return any(n.endswith(".class") for n in z.namelist())


def main(deps_file, cache):
    packages = []
    idx = 0
    for line in open(deps_file):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        pkg, rest = line.split("/", 1)
        art = os.path.join(newest_version_dir(pkg), rest)
        if not os.path.isfile(art):
            raise SystemExit(f"missing artifact {art}")
        idx += 1
        tag = "%02d-%s" % (idx, os.path.splitext(os.path.basename(art))[0])

        if art.endswith(".jar"):
            copy_jar(art, f"{cache}/jars/{tag}.jar")
            continue

        # python zipfile, not `unzip`: the .NET-packed AARs (Xamarin.Kotlin.StdLib,
        # Xamarin.Jetbrains.Annotations) have local file headers Info-ZIP rejects outright with
        # "unknown compression method" even though the central directory says plain deflate.
        ex = os.path.join(cache, "work", tag)
        with zipfile.ZipFile(art) as z:
            z.extractall(ex)

        jars = []
        if os.path.isfile(f"{ex}/classes.jar"):
            jars.append(("", f"{ex}/classes.jar"))
        libs = os.path.join(ex, "libs")
        if os.path.isdir(libs):
            jars += [(f"-{n}", os.path.join(libs, n))
                     for n in sorted(os.listdir(libs)) if n.endswith(".jar")]
        for sfx, jar in jars:
            if has_classes(jar):  # multiplatform facade AARs ship an empty classes.jar
                copy_jar(jar, f"{cache}/jars/{tag}{sfx}.jar")

        res = os.path.join(ex, "res")
        if os.path.isdir(res) and os.listdir(res):
            os.symlink(res, f"{cache}/res/{tag}")
            man = os.path.join(ex, "AndroidManifest.xml")
            m = re.search(r'package="([^"]+)"', open(man).read()) if os.path.isfile(man) else None
            if m:
                packages.append(m.group(1))

    with open(f"{cache}/packages.txt", "w") as f:
        f.write("\n".join(dict.fromkeys(packages)) + "\n")
    print(f"[aar] staged {idx} artifact(s): "
          f"{len(os.listdir(cache + '/jars'))} jar(s), {len(os.listdir(cache + '/res'))} res set(s)",
          file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
