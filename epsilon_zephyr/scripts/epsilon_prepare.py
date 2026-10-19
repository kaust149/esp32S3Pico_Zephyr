#!/usr/bin/env python3
"""
Prepare NumWorks Epsilon for the Zephyr module.

Runs Epsilon's own host (linux) build far enough to generate its sources (translations,
fonts, icons, MicroPython tables), then reads the compile commands it would use and writes
epsilon_sources.cmake: the source list, include paths and defines for the Zephyr build.
Simulator and SDL sources are dropped; the Ion backend comes from this module instead.

Usage:  epsilon_prepare.py /path/to/epsilon-repo [-j N]
Needs on the host: gcc/g++, make, python3 with lz4, pypng and stringcase, libfreetype and libpng dev packages.
"""
import argparse, os, re, shlex, subprocess, sys, zlib

def run(cmd, cwd, check=True):
    print("+", " ".join(cmd), flush=True)
    r = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if check and r.returncode != 0:
        print(r.stdout[-4000:])
        sys.exit("command failed: " + " ".join(cmd))
    return r

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("repo")
    ap.add_argument("-j", type=int, default=os.cpu_count() or 2)
    ap.add_argument("--out", default=None, help="where to write epsilon_sources.cmake (default: next to this script's module)")
    a = ap.parse_args()

    eps = os.path.abspath(os.path.join(a.repo, "epsilon"))
    if not os.path.isfile(os.path.join(eps, "Makefile")):
        sys.exit("%s does not look like the Epsilon repository (no epsilon/Makefile)" % a.repo)
    out = a.out or os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "epsilon_sources.cmake")

    # 1. Generate sources. The final link of the desktop simulator is expected to fail
    #    (SDL/JPEG), only the generated files matter. -k keeps going past it, and a second
    #    pass covers files that were requested before their generated headers were ready.
    for _ in range(2):
        run(["make", "-k", "-j%d" % a.j, "PLATFORM=linux", "epsilon.bin"], eps, check=False)
    gen = os.path.join(eps, "output/release/linux/apps/i18n.h")
    if not os.path.isfile(gen):
        sys.exit("Source generation failed: %s is missing (check the python modules lz4, pypng, stringcase)" % gen)

    # 2. Read the compile commands.
    r = run(["make", "-n", "-B", "-j1", "PLATFORM=linux", "epsilon.bin"], eps, check=False)
    entries = []
    for line in r.stdout.splitlines():
        if not (line.startswith("g++ ") or line.startswith("gcc ")) or " -c " not in line:
            continue
        t = shlex.split(line)
        i = t.index("-c")
        flags, k = [], 1
        while k < i:
            if t[k] in ("-MMD", "-MP"):
                k += 1
            elif t[k] == "-include":
                flags.append("-include " + t[k + 1])  # keep the option and its file together
                k += 2
            else:
                flags.append(t[k])
                k += 1
        entries.append((t[0], t[i + 1], flags))
    if not entries:
        sys.exit("no compile commands found")

    def is_foreign(src, flags):
        # Host tools (inliner, rasterizer), SDL, and the desktop simulator's Ion backend.
        return (any(f.startswith("-DSDL_") or f == "-DGENERATE_PNG=1" for f in flags)
                or any(f.startswith("-I/usr/include") for f in flags)
                or "output/tools" in src or "external/sdl" in src or "ion/src/simulator" in src)

    kept = [(cc, s, f) for cc, s, f in entries if not is_foreign(s, f)]

    # Flags shared by every kept C++ file (the first kept C++ file is the reference).
    cxx = [e for e in kept if e[0] == "g++"]
    common = set(cxx[0][2])
    for _, _, f in cxx:
        common &= set(f)
    ref_flags = [x for x in cxx[0][2] if x in common]

    def absolute(path):
        return os.path.normpath(os.path.join(eps, path))

    # Include paths and defines from the reference flags; -I paths made absolute.
    includes, defines, forced, other = [], [], [], []
    k = 0
    while k < len(ref_flags):
        f = ref_flags[k]
        if f.startswith("-I"):
            includes.append(absolute(f[2:]))
        elif f.startswith("-D"):
            defines.append(f[2:])
        elif f.startswith("-include "):
            forced.append(absolute(f[len("-include "):]))
        k += 1
    # The simulator include dirs are dropped, they only exist for the SDL backend.
    includes = [p for p in includes if "ion/src/simulator" not in p]
    # "-I.." style paths in the log are relative to epsilon/; keep the ones inside the repo.
    repo = os.path.dirname(eps)
    includes = [p for p in includes if p == repo or p.startswith(repo + os.sep)]
    includes.append(os.path.join(eps, "apps"))
    defines = [d for d in defines if not d.startswith("ION_SIMULATOR") and d != "ION_EVENTS_JOURNAL"]

    # Per-source extras: flags that are not in the common set (defines, include dirs, warnings).
    def extras(flags):
        ex = []
        k = 0
        while k < len(flags):
            f = flags[k]
            if f.startswith("-include ") and f not in common:
                ex.append("-include" + absolute(f[len("-include "):]))  # one token: no list de-duplication
                k += 1; continue
            if (f not in common and not f.startswith("-std=") and f not in ("-w",)
                    and not (f.startswith("-D") and "ION_SIMULATOR" in f) and f != "-Iapps"):
                if f.startswith("-I"):
                    f = "-I" + absolute(f[2:])
                ex.append(f)
            k += 1
        return ex

    lines = ["# Generated by scripts/epsilon_prepare.py. Do not edit.", ""]
    lines.append("set(EPSILON_ROOT \"%s\")" % os.path.dirname(eps))
    lines.append("")
    lines.append("set(EPSILON_INCLUDES")
    lines += ["  \"%s\"" % p for p in includes]
    lines.append(")\n")
    lines.append("set(EPSILON_DEFINES")
    for d in defines:
        lines.append("  \"%s\"" % d.replace("\\", "\\\\").replace("\"", "\\\""))
    lines.append(")\n")
    lines.append("set(EPSILON_FORCED_INCLUDES")
    lines += ["  \"%s\"" % p for p in forced]
    lines.append(")\n")
    cpp, c = [], []
    per_file = []
    # Defines whose value holds spaces, quotes, ';' or ',' do not survive a CMake option list,
    # so they are written into a small header that the file includes instead.
    outdir = os.path.dirname(os.path.abspath(out))
    gen_dir = os.path.join(outdir, "generated")
    os.makedirs(gen_dir, exist_ok=True)
    headers = {}

    def defs_header(defs):
        body = "// Generated by epsilon_prepare.py\n"
        for d in defs:
            name, _, val = d[2:].partition("=")
            body += "#define %s %s\n" % (name, val if _ else "1")
        key = zlib.crc32(body.encode())
        if body not in headers:
            path = os.path.join(gen_dir, "epsilon_defs_%08x.h" % key)
            with open(path, "w") as hf:
                hf.write(body)
            headers[body] = path
        return headers[body]

    for cc, s, f in kept:
        p = absolute(s)
        (cpp if cc == "g++" else c).append(p)
        ex = extras(f)
        # Anything that is not a plain NAME or NAME=simple-value goes through the header.
        wild = [x for x in ex if x.startswith("-D") and not re.fullmatch(r"-D\w+(=[\w./+-]*)?", x)]
        ex = [x for x in ex if x not in wild]
        if wild:
            ex += ["-include" + defs_header(wild)]
        if ex:
            per_file.append((p, ex))
    lines.append("set(EPSILON_CXX_SOURCES")
    lines += ["  \"%s\"" % p for p in sorted(cpp)]
    lines.append(")\n")
    lines.append("set(EPSILON_C_SOURCES")
    lines += ["  \"%s\"" % p for p in sorted(c)]
    lines.append(")\n")
    lines.append("# Per-file extra compile options")
    for p, ex in sorted(per_file):
        q = ";".join(x.replace("\\", "\\\\").replace("\"", "\\\"") for x in ex)
        lines.append("set_source_files_properties(\"%s\" PROPERTIES COMPILE_OPTIONS \"%s\")" % (p, q))
    with open(out, "w") as fh:
        fh.write("\n".join(lines) + "\n")
    print("wrote %s: %d C++ and %d C sources, %d include dirs, %d per-file overrides"
          % (os.path.normpath(out), len(cpp), len(c), len(includes), len(per_file)))

main()
