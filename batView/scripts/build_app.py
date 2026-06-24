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
DESKTOP_DIR = Path.home() / "Desktop"
ICLOUD_DESKTOP_DIR = Path.home() / "Library" / "Mobile Documents" / "com~apple~CloudDocs" / "Desktop"
DESKTOP_APP = DESKTOP_DIR / "batView.app"
DESKTOP_LINUX_LAUNCHER = DESKTOP_DIR / "batView.desktop"
DESKTOP_WINDOWS_SHORTCUT = DESKTOP_DIR / "batView.lnk"


def run(cmd: list[str], cwd: Path | None = None) -> None:
    print("+", " ".join(str(part) for part in cmd))
    try:
        subprocess.run(cmd, cwd=cwd, check=True)
    except FileNotFoundError as exc:
        program = cmd[0]
        raise SystemExit(
            f"No se encontro el comando '{program}'. Instala la herramienta requerida "
            "o cierra y abre PowerShell para actualizar el PATH."
        ) from exc


def capture(cmd: list[str]) -> str:
    return subprocess.check_output(cmd, text=True).strip()


def find_cmake_executable() -> str:
    cmake = shutil.which("cmake")
    if cmake:
        return cmake

    if platform.system() == "Windows":
        candidates = [
            Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "CMake" / "bin" / "cmake.exe",
            Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / "CMake" / "bin" / "cmake.exe",
            Path(os.environ.get("LOCALAPPDATA", "")) / "Programs" / "CMake" / "bin" / "cmake.exe",
        ]

        for candidate in candidates:
            if candidate.exists():
                return str(candidate)

    raise SystemExit(
        "No se encontro CMake. Instala CMake con winget o cierra y abre PowerShell "
        "si CMake se instalo en esta misma sesion."
    )


def python_info_from_command(command: list[str]) -> dict[str, str]:
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
    return json.loads(capture([*command, "-c", code]))


def python_info(python_exe: str) -> dict[str, str]:
    return python_info_from_command([python_exe])


def windows_python_import_library(info: dict[str, str]) -> Path | None:
    version_token = info["version"].replace(".", "")
    library_name = f"python{version_token}.lib"
    base_prefix = Path(info["base_prefix"])
    executable_dir = Path(info["executable"]).parent
    candidates = [
        base_prefix / "libs" / library_name,
        executable_dir / "libs" / library_name,
        base_prefix / library_name,
        executable_dir / library_name,
    ]

    for candidate in candidates:
        if candidate.exists():
            return candidate

    return None


def is_python_usable_for_embedding(info: dict[str, str]) -> bool:
    include_src = Path(info["include_dir"])
    stdlib_src = Path(info["stdlib_dir"])
    if not (include_src / "Python.h").exists() or not stdlib_src.exists():
        return False

    if platform.system() == "Windows":
        return windows_python_import_library(info) is not None

    return True


def find_python_executable(explicit: str | None) -> str:
    if explicit:
        try:
            info = python_info(explicit)
        except Exception as exc:
            raise SystemExit(f"No se pudo ejecutar Python en '{explicit}': {exc}") from exc
        if not is_python_usable_for_embedding(info):
            raise SystemExit(
                "El Python indicado no sirve para compilar Python embebido. "
                "En Windows debe incluir include\\Python.h y libs\\pythonXY.lib."
            )
        return info["executable"]

    candidate_commands: list[list[str]] = []
    if os.name == "nt":
        candidate_commands.extend(
            [
                ["py", "-3.13"],
                ["py", "-3.12"],
                ["py"],
                ["python"],
                ["python3"],
            ]
        )
        for root in (
            Path(os.environ.get("LOCALAPPDATA", "")) / "Programs" / "Python",
            Path(os.environ.get("ProgramFiles", r"C:\Program Files")),
            Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")),
        ):
            for version_dir in ("Python313", "Python312"):
                candidate = root / version_dir / "python.exe"
                if candidate.exists():
                    candidate_commands.append([str(candidate)])
    else:
        if sys.executable:
            candidate_commands.append([sys.executable])
        candidate_commands.extend([["python3"], ["python"]])

    rejected: list[str] = []
    for command in candidate_commands:
        try:
            subprocess.run([*command, "--version"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            info = python_info_from_command(command)
        except Exception:
            continue

        if is_python_usable_for_embedding(info):
            return info["executable"]

        rejected.append(info.get("executable", " ".join(command)))

    if rejected and platform.system() == "Windows":
        raise SystemExit(
            "Se encontro Python, pero ninguno incluye la libreria de enlace requerida "
            "libs\\pythonXY.lib. Instala Python desde python.org o con "
            "`winget install --id Python.Python.3.13 --exact`, cierra PowerShell, "
            "abre una nueva ventana y vuelve a ejecutar install_windows.ps1."
        )

    raise SystemExit("No se encontro un ejecutable de Python para preparar la version full.")


def copy_tree(src: Path, dst: Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst, dirs_exist_ok=True)


def copy_file(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def copy_optional_tree(src: Path, dst: Path) -> None:
    if src.exists():
        copy_tree(src, dst)


def desktop_app_path() -> Path:
    return DESKTOP_APP


def desktop_launcher_path() -> Path:
    system = platform.system()
    if system == "Windows":
        return DESKTOP_WINDOWS_SHORTCUT
    if system == "Linux":
        return DESKTOP_LINUX_LAUNCHER
    return DESKTOP_APP


def desktop_executable_path() -> Path:
    system = platform.system()
    if system == "Windows":
        candidates = [BUILD_DIR / "Release" / "batView.exe", BUILD_DIR / "batView.exe"]
        for candidate in candidates:
            if candidate.exists():
                return candidate
        return candidates[0]

    if system == "Linux":
        return BUILD_DIR / "batView"

    return BUILD_DIR / "batView.app"


def copy_app_to_desktop() -> Path:
    source_app = BUILD_DIR / "batView.app"
    target_app = desktop_app_path()

    if not source_app.exists():
        raise SystemExit("No se encontro el bundle de macOS para copiar al Desktop.")

    if target_app.exists():
        shutil.rmtree(target_app)

    target_app.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(source_app, target_app)
    return target_app


def create_linux_desktop_launcher(executable_path: Path) -> Path:
    launcher_path = desktop_launcher_path()
    icon_path = BUILD_DIR / "assets" / "BatView.png"
    launcher_contents = "\n".join(
        [
            "[Desktop Entry]",
            "Type=Application",
            "Name=batView",
            "Comment=Desktop application for ESP32 battery test control",
            f"Exec={executable_path}",
            f"Path={executable_path.parent}",
            f"Icon={icon_path}",
            "Terminal=false",
            "Categories=Utility;",
            "",
        ]
    )

    launcher_path.parent.mkdir(parents=True, exist_ok=True)
    launcher_path.write_text(launcher_contents, encoding="utf-8")
    launcher_path.chmod(0o755)
    return launcher_path


def create_windows_shortcut(executable_path: Path) -> Path:
    shortcut_path = desktop_launcher_path()
    icon_path = BUILD_DIR / "assets" / "BatView.ico"
    shortcut_path.parent.mkdir(parents=True, exist_ok=True)

    script = rf"""
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut('{shortcut_path}')
$shortcut.TargetPath = '{executable_path}'
$shortcut.WorkingDirectory = '{executable_path.parent}'
$shortcut.IconLocation = '{icon_path}'
$shortcut.Save()
""".strip()

    run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", script])
    return shortcut_path


def create_desktop_launcher() -> Path:
    system = platform.system()
    if system == "Darwin":
        return copy_app_to_desktop()

    executable_path = desktop_executable_path()
    if system == "Windows":
        if not executable_path.exists():
            raise SystemExit("No se encontro el ejecutable de Windows para crear el acceso directo.")
        return create_windows_shortcut(executable_path)

    if not executable_path.exists():
        raise SystemExit("No se encontro el ejecutable de Linux para crear el acceso directo.")
    return create_linux_desktop_launcher(executable_path)


def remove_path(path: Path) -> bool:
    if not path.exists():
        return False

    if path.is_dir():
        shutil.rmtree(path)
    else:
        path.unlink()
    return True


def clean_generated_artifacts() -> list[Path]:
    removed_paths = []

    desktop_candidates = [DESKTOP_APP, DESKTOP_LINUX_LAUNCHER, DESKTOP_WINDOWS_SHORTCUT]
    if platform.system() == "Darwin":
        desktop_candidates.append(ICLOUD_DESKTOP_DIR / "batView.app")

    for path in (BUILD_DIR, DIST_DIR, PYTHON_CACHE_ROOT, *desktop_candidates):
        if remove_path(path):
            removed_paths.append(path)

    return removed_paths


def copy_windows_python_runtime(info: dict[str, str], runtime_root: Path) -> Path:
    base_prefix = Path(info["base_prefix"])
    include_src = Path(info["include_dir"])
    stdlib_src = Path(info["stdlib_dir"])
    import_library = windows_python_import_library(info)
    if import_library is None:
        raise SystemExit(
            "No se encontro la libreria de enlace de Python para Windows "
            f"(python{info['version'].replace('.', '')}.lib). "
            "Instala Python desde python.org o con winget, no solo el alias de Microsoft Store."
        )

    copy_tree(include_src, runtime_root / "include")
    copy_tree(stdlib_src, runtime_root / "Lib")
    copy_optional_tree(base_prefix / "libs", runtime_root / "libs")
    copy_optional_tree(base_prefix / "DLLs", runtime_root / "DLLs")
    copy_file(import_library, runtime_root / "libs" / import_library.name)

    python_dlls = list(base_prefix.glob("python*.dll"))
    executable_dir = Path(info["executable"]).parent
    python_dlls.extend(executable_dir.glob("python*.dll"))
    for dll in python_dlls:
        copy_file(dll, runtime_root / dll.name)

    return runtime_root / "Lib" / "site-packages"


def generate_windows_icon(build_dir: Path, site_packages: Path) -> None:
    try:
        sys.path.insert(0, str(site_packages))
        from PIL import Image
    except Exception as exc:  # pragma: no cover - packaging fallback
        raise SystemExit(f"No se pudo cargar Pillow para generar el icono de Windows: {exc}") from exc

    source_icon = PROJECT_ROOT / "assets" / "BatView.png"
    target_icon = build_dir / "assets" / "BatView.ico"
    target_icon.parent.mkdir(parents=True, exist_ok=True)

    with Image.open(source_icon) as image:
        image.save(
            target_icon,
            format="ICO",
            sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)],
        )


def prepare_runtime(python_exe: str) -> tuple[Path, str, Path]:
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

    if system == "Windows":
        site_packages = copy_windows_python_runtime(info, runtime_root)
    else:
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

    return runtime_root, version, site_packages


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
        desktop_app = desktop_app_path()
        if desktop_app.exists():
            run(["open", str(desktop_app)])
        else:
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
    parser = argparse.ArgumentParser(description="Construye, limpia o abre batView como app full en un solo comando.")
    parser.add_argument("--python", dest="python_exe", help="Ejecutable de Python a usar para preparar el runtime full.")
    parser.add_argument("--no-run", action="store_true", help="Construye y empaqueta, pero no abre la app al final.")
    parser.add_argument("--clean", action="store_true", help="Elimina los artefactos generados y sale.")
    parser.add_argument("--cmake-toolchain", help="Archivo CMake toolchain opcional, por ejemplo vcpkg.cmake.")
    args = parser.parse_args()

    if args.clean:
        removed_paths = clean_generated_artifacts()
        if removed_paths:
            print("Artefactos eliminados:")
            for path in removed_paths:
                print(f"- {path}")
        else:
            print("No habia artefactos generados para eliminar.")
        return

    python_exe = find_python_executable(args.python_exe)
    runtime_root, version, site_packages = prepare_runtime(python_exe)
    package_format = package_format_for_host()

    print(f"Runtime Python preparado en: {runtime_root}")
    print(f"Python embebido detectado: {version}")
    print(f"Formato de paquete seleccionado: {package_format}")

    cmake_exe = find_cmake_executable()
    print(f"CMake detectado en: {cmake_exe}")

    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    DIST_DIR.mkdir(parents=True, exist_ok=True)

    if platform.system() == "Windows":
        generate_windows_icon(BUILD_DIR, site_packages)

    configure_cmd = [
        cmake_exe,
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

    if platform.system() == "Darwin":
        configure_cmd.append("-DBATVIEW_BUILD_MACOS_ICON=ON")

    toolchain_file = args.cmake_toolchain or os.environ.get("BATVIEW_CMAKE_TOOLCHAIN_FILE")
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    if not toolchain_file and vcpkg_root:
        candidate = Path(vcpkg_root) / "scripts" / "buildsystems" / "vcpkg.cmake"
        if candidate.exists():
            toolchain_file = str(candidate)

    if toolchain_file:
        configure_cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}")
        print(f"CMake toolchain: {toolchain_file}")

    run(configure_cmd, cwd=PROJECT_ROOT)
    run([cmake_exe, "--build", str(BUILD_DIR), "--config", "Release"], cwd=PROJECT_ROOT)
    run([cmake_exe, "--build", str(BUILD_DIR), "--config", "Release", "--target", "package"], cwd=PROJECT_ROOT)

    desktop_launcher = create_desktop_launcher()
    print(f"Acceso directo creado en el Desktop: {desktop_launcher}")

    if not args.no_run:
        launch_app()

    print("batView full listo.")
    print(f"Build: {BUILD_DIR}")
    print(f"Artefactos: {DIST_DIR}")


if __name__ == "__main__":
    main()
