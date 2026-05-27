Import("env")

import filecmp
import os
import shutil
import subprocess
from pathlib import Path


def _log(msg):
    print("[pio_debug_fix] {0}".format(msg))


def _kill_openocd_windows():
    if os.name != "nt":
        return
    subprocess.run(
        ["taskkill", "/F", "/IM", "openocd.exe"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    _log("OpenOCD residual process cleanup requested")


def _force_utf8_locale():
    run_env = env.setdefault("ENV", {})
    run_env["LANG"] = "C.UTF-8"
    run_env["LC_ALL"] = "C.UTF-8"
    run_env["PYTHONUTF8"] = "1"
    _log("Forced UTF-8 locale for debug subprocesses")


def _patch_gdb_binary():
    gdb_pkg_dir = env.PioPlatform().get_package_dir("tool-xtensa-esp-elf-gdb")
    if not gdb_pkg_dir:
        _log("GDB package directory not found")
        return

    bin_dir = Path(gdb_pkg_dir) / "bin"
    src = bin_dir / "xtensa-esp-elf-gdb-no-python.exe"
    dst = bin_dir / "xtensa-esp-elf-gdb-3.12.exe"
    bak = bin_dir / "xtensa-esp-elf-gdb-3.12.exe.bak"

    if not src.exists() or not dst.exists():
        _log("Expected GDB binaries were not found; skipping patch")
        return

    if not bak.exists():
        shutil.copy2(dst, bak)
        _log("Backup created: {0}".format(bak))

    if filecmp.cmp(src, dst, shallow=False):
        _log("GDB 3.12 already forced to no-python mode")
        return

    shutil.copy2(src, dst)
    _log("Patched {0} to run in no-python mode".format(dst.name))


_kill_openocd_windows()
_force_utf8_locale()
_patch_gdb_binary()
