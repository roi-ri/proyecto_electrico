# Build and Installer Scripts

These scripts prepare the computer, build batView, package it, and optionally open it.

The result is written to:

```text
dist/
```

The local build files are written to:

```text
build-release/
```

## Quick Start

### Windows

Open PowerShell as Administrator from the project folder:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

Build without opening the app at the end:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -NoRun
```

What the script prepares:

- Git
- CMake
- Python 3
- Visual Studio Build Tools with C++ tools
- NSIS for `.exe` installer packaging when available
- wxWidgets through local `tools/vcpkg`

If Windows was just updated by the script, close PowerShell, open it again, and rerun the same command.

### macOS

Run from the project folder:

```bash
./install_macos.sh
```

Build without opening the app at the end:

```bash
./install_macos.sh --no-run
```

What the script prepares:

- Apple command line tools
- Homebrew packages: `cmake`, `wxwidgets`, `python`, `pkg-config`

If the Apple command line tools installer opens, finish that installer and rerun the script.

### Linux

Run from the project folder:

```bash
./install_linux.sh
```

Build without opening the app at the end:

```bash
./install_linux.sh --no-run
```

What the script prepares:

- CMake
- C++ compiler and build tools
- wxWidgets development package
- GTK development package
- Python 3, Python headers, and pip
- packaging helpers such as `dpkg-dev`, `fakeroot`, or `zip` when available

The Linux script supports `apt`, `dnf`, and `pacman`.

## Skip Dependency Installation

If the computer already has the required tools installed:

```bash
./install_macos.sh --skip-deps --no-run
./install_linux.sh --skip-deps --no-run
```

On Windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps -NoRun
```

## Existing Build Scripts

The install scripts call the existing build scripts after preparing dependencies:

```bash
./build_app.sh
```

```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1
```

Use the build scripts directly only when dependencies are already installed.

## Package Types

batView chooses the best package format for the current OS:

- Windows: `.exe` installer with NSIS when available, otherwise portable package.
- macOS: portable `.zip` containing `batView.app`.
- Linux: native `.deb` when Debian tools are available, otherwise portable archive.

The app also embeds a local Python runtime for export/render features, including `Pillow` and `openpyxl`.

## Clean Rebuild

If a build fails after changing dependencies or switching computers:

```bash
rm -rf build-release python/runtime dist
./install_macos.sh --no-run
```

Linux:

```bash
rm -rf build-release python/runtime dist
./install_linux.sh --no-run
```

Windows PowerShell:

```powershell
Remove-Item -Recurse -Force build-release, python\runtime, dist -ErrorAction SilentlyContinue
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -NoRun
```
