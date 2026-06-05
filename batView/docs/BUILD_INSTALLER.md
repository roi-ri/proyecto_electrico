# Build and Installer Scripts

These scripts prepare the computer, build batView, package it, and optionally open it.

This guide assumes no previous build experience. Follow it in order.

## Before Running Anything

### 1. Use the Correct Branch

For this version of batView, use the `batView` branch.

If you already have the project folder, open a terminal inside the repository and run:

```bash
git checkout batView
```

On Windows PowerShell:

```powershell
git checkout batView
```

If Git says the branch does not exist, fetch the branches and try again:

```bash
git fetch --all
git checkout batView
```

### 2. Go to the Project Folder

All commands must be run from the folder that contains these files:

```text
README.md
CMakeLists.txt
install_windows.ps1
install_macos.sh
install_linux.sh
```

If your terminal is one folder above the project, enter the folder first:

```bash
cd batView
```

On Windows, a common location looks like:

```powershell
cd C:\Users\YourName\Downloads\proyecto_electrico\batView
```

### 3. Pick the Script for Your OS

Run only the script for the computer you are using:

- Windows: `install_windows.ps1`
- macOS: `install_macos.sh`
- Linux: `install_linux.sh`

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

Open PowerShell as Administrator from the `batView` project folder.

If you are not sure you are in the correct folder, run:

```powershell
dir
```

You should see `README.md`, `CMakeLists.txt`, and `install_windows.ps1`.

Then run:

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

Open Terminal from the `batView` project folder.

If you are not sure you are in the correct folder, run:

```bash
ls
```

You should see `README.md`, `CMakeLists.txt`, and `install_macos.sh`.

Then run:

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

Open Terminal from the `batView` project folder.

If you are not sure you are in the correct folder, run:

```bash
ls
```

You should see `README.md`, `CMakeLists.txt`, and `install_linux.sh`.

Then run:

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

## How to Know It Worked

The script worked if:

- the terminal prints `batView full listo.`;
- the app opens automatically, unless you used `--no-run` or `-NoRun`;
- the `dist/` folder contains a generated package;
- the `build-release/` folder contains the compiled app.

On Windows, look in:

```text
dist/
build-release/
```

On macOS, the app bundle is usually:

```text
build-release/batView.app
```

On Linux, the executable is usually:

```text
build-release/batView
```

## If Something Fails

Do these checks first:

- Make sure you are on branch `batView`.
- Make sure you are inside the `batView` project folder.
- On Windows, close and reopen PowerShell after installing tools.
- On macOS, finish the Apple command line tools installer if it appears.
- On Linux, enter your password if `sudo` asks for it.

Then rerun the same install command.

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
