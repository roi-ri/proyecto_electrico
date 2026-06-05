#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
BUILD_DIR = PROJECT_ROOT / "build-release"
DIST_DIR = PROJECT_ROOT / "dist"
PYTHON_CACHE_ROOT = PROJECT_ROOT / "python" / "runtime"


def run(cmd: list[str], cwd: Path | None = None) -> None:
    print("+", " ".join(str(part) for part in cmd))
    subprocess.run(cmd, cwd=cwd, check=True)


def capture(cmd: list[str]) -> str:
    return subprocess.check_output(cmd, text=True).strip()


def find_python_executable(explicit: str | None) -> str:
    if explicit:
        return explicit

    candidates = []
    if sys.executable:
        candidates.append(sys.executable)
    if os.name == "nt":
        candidates.extend(["py", "python", "python3"])
    else:
        candidates.extend(["python3", "python"])

    for candidate in candidates:
        try:
            subprocess.run([candidate, "--version"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return candidate
        except Exception:
            continue

    raise SystemExit("No se encontro un ejecutable de Python para preparar la version full.")


def python_info(python_exe: str) -> dict[str, str]:
    code = r"""
import json
import os
import sys
import sysconfig

version = f"{sys.version_info.major}.{sys.version_info.minor}"
data = {
    "executable": sys.executable,
    "base_prefix": sys.base_prefix,
    "include_dir": sysconfig.get_path("include"),
    "stdlib_dir": sysconfig.get_path("stdlib"),
    "platstdlib_dir": sysconfig.get_path("platstdlib"),
    "site_packages_dir": sysconfig.get_path("purelib"),
    "libdir": sysconfig.get_config_var("LIBDIR") or "",
    "ldlibrary": sysconfig.get_config_var("LDLIBRARY") or "",
    "version": version,
}

framework_python = os.path.join(sys.base_prefix, "Python")
if os.path.exists(framework_python):
    data["framework_python"] = framework_python
else:
    data["framework_python"] = ""

print(json.dumps(data))
"""
    return json.loads(capture([python_exe, "-c", code]))


def copy_tree(src: Path, dst: Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst, dirs_exist_ok=True)


def copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def prepare_runtime(python_exe: str) -> tuple[Path, str]:
    info = python_info(python_exe)
    system = platform.system()
    version = info["version"]
    runtime_root = PYTHON_CACHE_ROOT / f"{system.lower()}-python{version}"

    include_src = Path(info["include_dir"])
    stdlib_src = Path(info["stdlib_dir"])
    if not include_src.exists() or not stdlib_src.exists():
        raise SystemExit("No se pudieron localizar los headers o la libreria estandar de Python.")

    if runtime_root.exists():
        shutil.rmtree(runtime_root)
    runtime_root.mkdir(parents=True, exist_ok=True)

    copy_tree(include_src, runtime_root / "include" / include_src.name)
    copy_tree(stdlib_src, runtime_root / "lib" / f"python{version}")

    libdir = Path(info["libdir"]) if info["libdir"] else None
    ldlibrary = info["ldlibrary"]
    framework_python = Path(info["framework_python"]) if info["framework_python"] else None

    if system == "Darwin" and framework_python and framework_python.exists():
        copy_file(framework_python, runtime_root / "Python")
    elif libdir and ldlibrary:
        lib_src = libdir / ldlibrary
        if lib_src.exists():
            copy_file(lib_src, runtime_root / "lib" / ldlibrary)

    site_packages = runtime_root / "lib" / f"python{version}" / "site-packages"
    site_packages.mkdir(parents=True, exist_ok=True)

    run(
        [
            python_exe,
            "-m",
            "pip",
            "install",
            "--upgrade",
            "--target",
            str(site_packages),
            "pillow",
            "openpyxl",
        ]
    )

    return runtime_root, version


def package_format_for_host() -> str:
    system = platform.system()
    if system == "Windows":
        return "NATIVE" if shutil.which("makensis") else "PORTABLE"
    if system == "Linux":
        return "NATIVE" if shutil.which("dpkg-deb") else "PORTABLE"
    return "PORTABLE"


def launch_app() -> None:
    system = platform.system()
    if system == "Darwin":
        run(["open", str(BUILD_DIR / "batView.app")])
        return
    if system == "Windows":
        candidates = [BUILD_DIR / "Release" / "batView.exe", BUILD_DIR / "batView.exe"]
        for candidate in candidates:
            if candidate.exists():
                os.startfile(candidate)  # type: ignore[attr-defined]
                return
        raise SystemExit("No se encontro el ejecutable de Windows para abrir la app.")

    candidate = BUILD_DIR / "batView"
    if candidate.exists():
        subprocess.Popen([str(candidate)], cwd=PROJECT_ROOT)
        return
    raise SystemExit("No se encontro el ejecutable de Linux para abrir la app.")


def main() -> None:
    parser = argparse.ArgumentParser(description="Construye y abre batView como app full en un solo comando.")
    parser.add_argument("--python", dest="python_exe", help="Ejecutable de Python a usar para preparar el runtime full.")
    parser.add_argument("--no-run", action="store_true", help="Construye y empaqueta, pero no abre la app al final.")
    args = parser.parse_args()

    python_exe = find_python_executable(args.python_exe)
    runtime_root, version = prepare_runtime(python_exe)
    package_format = package_format_for_host()

    print(f"Runtime Python preparado en: {runtime_root}")
    print(f"Python embebido detectado: {version}")
    print(f"Formato de paquete seleccionado: {package_format}")

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    DIST_DIR.mkdir(parents=True, exist_ok=True)

    configure_cmd = [
        "cmake",
        "-S",
        ".",
        "-B",
        str(BUILD_DIR),
        "-DBATVIEW_ENABLE_WX=ON",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DBATVIEW_PACKAGE_FORMAT={package_format}",
        "-DBATVIEW_ENABLE_EMBEDDED_PYTHON=ON",
        f"-DBATVIEW_EMBEDDED_PYTHON_ROOT={runtime_root}",
    ]

    run(configure_cmd, cwd=PROJECT_ROOT)
    run(["cmake", "--build", str(BUILD_DIR), "--config", "Release"], cwd=PROJECT_ROOT)
    run(["cmake", "--build", str(BUILD_DIR), "--config", "Release", "--target", "package"], cwd=PROJECT_ROOT)

    if not args.no_run:
        launch_app()

    print("batView full listo.")
    print(f"Build: {BUILD_DIR}")
    print(f"Artefactos: {DIST_DIR}")


if __name__ == "__main__":
    main()
