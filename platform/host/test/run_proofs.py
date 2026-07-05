#!/usr/bin/env python3
"""
run_proofs -- devarm の host proof 群を 1 コマンドでビルド・実行・集約する回帰ランナー。

cpp_rewriter/tool/tests/regression.py の自己完結スタイルを踏襲する。ただし devarm の
proof は各自が ground-truth を assert 済み ([OK]/[FAIL] と ALL PASS/SOME FAILED + 終了
コードを自前で出す) ため、golden スナップショットは不要。ランナーは各 proof を

  (1) clang++ -std=c++17 -Wall -Wextra でビルド (警告が出たら FAIL: devarm の基準は警告ゼロ)
  (2) 実行 (stdin が要るものは供給) し、終了コード 0 かつ [FAIL]/SOME FAILED マーカ無しを要求
  (3) manifest 網羅: platform/host/test/*.cpp のうち登録漏れの *_proof/*_diag を検出

の 3 点で検査する。ALL PASS を出さない smoke/診断 (io_proof, heap_diag) は (2) の
「exit 0 かつ FAIL マーカ無し」で自動的に「ビルド＆実行が通る」ことだけを保証する。

依存なし・stdlib のみ (venv 不要)。pytest があれば test_* も discover される。

  python3 run_proofs.py            # 全 proof を検査。失敗で exit 1
  python3 run_proofs.py -k malloc  # 名前に 'malloc' を含む proof のみ
  python3 run_proofs.py -v         # ビルドコマンド・実行出力も表示
  python3 run_proofs.py --list     # 登録済み proof 一覧
  python3 run_proofs.py --keep     # ビルドした実行ファイルを消さずに残す
"""
from __future__ import annotations
import os, sys, glob, argparse, subprocess, tempfile, shutil

HERE = os.path.dirname(os.path.abspath(__file__))          # platform/host/test
DEVARM = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))  # devarm ルート
TESTDIR = "platform/host/test"

CXX = os.environ.get("CXX", "clang++")
CXXFLAGS = ["-std=c++17", "-Wall", "-Wextra"]

# 共通ソース群 (ビルドレシピはこれらの組合せ)
CONVERT = sorted(glob.glob(os.path.join(DEVARM, "core/convert/*.cpp")))
CONVERT = [os.path.relpath(p, DEVARM) for p in CONVERT]     # devarm 相対に正規化
ELF = sorted(glob.glob(os.path.join(DEVARM, "core/elf/*.cpp")))
ELF = [os.path.relpath(p, DEVARM) for p in ELF]
HAL_IO = ["hal/io.cpp", "platform/host/io_host.cpp"]
HEAP_BACK = ["core/mem/mallocator.cpp", "platform/host/heap_host.cpp"]

# ------------------------------------------------------------- manifest
# 各 proof: name -> {srcs, inc, stdin}
#   srcs : proof 本体に加えてリンクする .cpp (devarm 相対)
#   inc  : -I に渡すディレクトリ (devarm 相対)
#   stdin: 実行時に食わせる bytes (無ければ None)
def _p(name, srcs, inc, stdin=None):
    return {"name": name, "srcs": srcs, "inc": inc, "stdin": stdin}

PROOFS = [
    _p("block_proof",  [], ["core/block"]),
    _p("cond_proof",   [], ["core/include", "core/cond"]),
    _p("mem_proof",    [], ["core/include", "core/mem"]),
    _p("io_proof",
       CONVERT + ["core/io/printf.cpp", "core/io/sprintf.cpp", "core/io/snprintf.cpp"] + HAL_IO,
       ["core/include", "core/convert", "core/io", "."]),
    _p("mcheck_proof",
       CONVERT + ["core/io/printf.cpp"] + HAL_IO + ["platform/host/mcheck_host.cpp"],
       ["core/include", "core/convert", "core/mem", "."]),
    _p("heap_proof",
       CONVERT + ["core/io/printf.cpp"] + HEAP_BACK + ["hal/io.cpp", "platform/host/io_host.cpp"],
       ["core/include", "core/convert", "core/mem", "."]),
    _p("heap_diag",
       CONVERT + ["core/io/printf.cpp"] + HEAP_BACK + ["hal/io.cpp", "platform/host/io_host.cpp"],
       ["core/include", "core/convert", "core/mem", "."]),
    _p("malloc_proof",
       ["core/mem/malloc.cpp", "core/mem/mallocator.cpp"] + CONVERT + ["core/io/printf.cpp"]
       + HAL_IO + ["platform/host/heap_host.cpp"],
       ["core/include", "core/convert", "core/mem", "core/block", "."]),
    _p("opnew_proof",
       ["core/mem/operator_new.cpp", "core/mem/mallocator.cpp"] + CONVERT + ["core/io/printf.cpp"]
       + HAL_IO + ["platform/host/heap_host.cpp"],
       ["core/include", "core/convert", "core/mem", "."]),
    _p("gettext_proof",
       ["core/io/gettext.cpp"] + HAL_IO,
       ["core/include", "core/convert", "core/io", "."],
       stdin=b"abc def\r"),
    _p("stimer_proof",
       ["core/util/stimer.cpp", "hal/timer.cpp"] + HAL_IO + ["platform/host/timer_host.cpp"],
       ["core/include", "core/convert", "core/util", "core/io", "."]),
    _p("elf_proof",
       ELF + ["core/util/misc_token.cpp"] + CONVERT + ["core/io/printf.cpp"] + HAL_IO,
       ["core/include", "core/convert", "core/io", "core/util", "core/elf", "."]),
]

# ------------------------------------------------------------- build & run

def build(proof: dict, outdir: str) -> tuple[bool, str, list[str]]:
    """proof をビルド。(ok, log, cmd) を返す。ok は returncode==0 かつ 警告ゼロ。"""
    binpath = os.path.join(outdir, proof["name"])
    src = os.path.join(TESTDIR, proof["name"] + ".cpp")
    inc = [f"-I{d}" for d in proof["inc"]]
    cmd = [CXX] + CXXFLAGS + inc + [src] + proof["srcs"] + ["-o", binpath]
    r = subprocess.run(cmd, cwd=DEVARM, capture_output=True, text=True)
    log = (r.stdout + r.stderr).strip()
    ok = (r.returncode == 0) and (log == "")   # 警告 (stderr 非空) も FAIL
    return ok, log, cmd


def run(proof: dict, outdir: str) -> tuple[bool, str]:
    """proof を実行。(ok, output) を返す。exit 0 かつ [FAIL]/SOME FAILED マーカ無しで ok。"""
    binpath = os.path.join(outdir, proof["name"])
    r = subprocess.run([binpath], cwd=DEVARM, input=proof["stdin"],
                       capture_output=True)
    out = r.stdout.decode("utf-8", "replace") + r.stderr.decode("utf-8", "replace")
    failed = ("[FAIL]" in out) or ("SOME FAILED" in out)
    ok = (r.returncode == 0) and not failed
    return ok, out.strip()


def evaluate(proof: dict, outdir: str, verbose: bool) -> dict:
    b_ok, b_log, cmd = build(proof, outdir)
    rec = {"name": proof["name"], "build": b_ok, "run": None, "log": b_log, "out": ""}
    if verbose:
        rec["cmd"] = " ".join(cmd)
    if not b_ok:
        rec["verdict"] = "build_fail" if "error:" in b_log else "warnings"
        return rec
    r_ok, out = run(proof, outdir)
    rec["run"], rec["out"] = r_ok, out
    rec["verdict"] = "ok" if r_ok else "run_fail"
    return rec

# ------------------------------------------------------------- checks

def check_manifest_coverage() -> list[str]:
    """platform/host/test/*.cpp のうち manifest 未登録の proof/diag を検出。"""
    registered = {p["name"] for p in PROOFS}
    fails = []
    for p in sorted(glob.glob(os.path.join(DEVARM, TESTDIR, "*.cpp"))):
        name = os.path.splitext(os.path.basename(p))[0]
        if (name.endswith("_proof") or name.endswith("_diag")) and name not in registered:
            fails.append(f"[{name}] manifest 未登録 (PROOFS に追加せよ)")
    return fails

# ------------------------------------------------------------- cli

def selected(pattern: str | None) -> list[dict]:
    if not pattern:
        return PROOFS
    return [p for p in PROOFS if pattern in p["name"]]


def cmd_run(pattern: str | None, verbose: bool, keep: bool) -> int:
    cov = check_manifest_coverage()
    proofs = selected(pattern)
    outdir = tempfile.mkdtemp(prefix="devarm_proofs_")
    total_fail = len(cov)
    try:
        print(f"=== run_proofs: {len(proofs)} proof (CXX={CXX}, {' '.join(CXXFLAGS)}) ===")
        for label, fails in [("manifest 網羅", cov)]:
            mark = "OK  " if not fails else f"FAIL({len(fails)})"
            print(f"  [{mark}] {label}")
            for f in fails:
                print("       - " + f)
        for p in proofs:
            rec = evaluate(p, outdir, verbose)
            mark = {"ok": "OK  ", "warnings": "WARN", "build_fail": "BUILD",
                    "run_fail": "FAIL"}[rec["verdict"]]
            print(f"  [{mark}] {rec['name']}")
            if rec["verdict"] != "ok":
                total_fail += 1
                if verbose:
                    print("       $ " + rec.get("cmd", ""))
                if rec["log"]:
                    print("       " + rec["log"].replace("\n", "\n       "))
                if rec["out"]:
                    print("       " + rec["out"].replace("\n", "\n       "))
            elif verbose and rec["out"]:
                print("       " + rec["out"].replace("\n", "\n       "))
        print("PASS" if total_fail == 0 else f"FAILED: {total_fail} 件")
    finally:
        if keep:
            print(f"(実行ファイルを保持: {outdir})")
        else:
            shutil.rmtree(outdir, ignore_errors=True)
    return 0 if total_fail == 0 else 1


def cmd_list() -> int:
    print(f"登録済み proof: {len(PROOFS)}")
    for p in PROOFS:
        stdin = " (stdin)" if p["stdin"] else ""
        print(f"  {p['name']:<16} srcs={len(p['srcs'])} inc={p['inc']}{stdin}")
    cov = check_manifest_coverage()
    for f in cov:
        print("  ! " + f)
    return 0

# ------------------------------------------------------------- pytest hook

def test_all_proofs():
    assert cmd_run(None, False, False) == 0

# ------------------------------------------------------------- main

def main(argv=None) -> int:
    ap = argparse.ArgumentParser(prog="run_proofs", description=__doc__.splitlines()[1])
    ap.add_argument("-k", dest="pattern", metavar="SUBSTR", help="名前部分一致で絞り込む")
    ap.add_argument("-v", "--verbose", action="store_true", help="ビルドコマンド・実行出力も表示")
    ap.add_argument("--list", action="store_true", help="登録済み proof 一覧")
    ap.add_argument("--keep", action="store_true", help="ビルドした実行ファイルを残す")
    a = ap.parse_args(argv)
    if a.list:
        return cmd_list()
    return cmd_run(a.pattern, a.verbose, a.keep)


if __name__ == "__main__":
    raise SystemExit(main())
