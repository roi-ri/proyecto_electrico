# Build and Installer Scripts

These scripts prepare the computer, build batView, package it, and optionally open it.

This guide assumes no previous build experience. Follow it in order.

## Start Here

Use only the script for your own computer. Do not mix the scripts.

| Computer | Script |
|---|---|
| Mac | `install_macos.sh` |
| Linux | `install_linux.sh` |
| Windows | `install_windows.ps1` |

If you are unsure, stop here and check the label on your computer first.

## What These Scripts Do

Each install script does the same basic job:

1. It installs the tools needed to build batView.
2. It builds the app.
3. It creates the package or desktop launcher for your system.

The results are written to:

```text
dist/
```

The build files are written to:

```text
build-release/
```

## Before You Begin

Make sure you are inside the batView folder before running anything.

If you are one folder above it, go into the folder first:

```bash
cd batView
```

On Windows, the full path may look like this:

```powershell
cd C:\Users\YourName\Downloads\proyecto_electrico\batView
```

If you cloned the repository with Git, you can confirm the branch with:

```bash
git branch --show-current
```

It should say `batView`.

## Quick Start

### Windows

1. Open PowerShell in the batView folder.
2. Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

If you do not want the app to open at the end, use:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -NoRun
```

What this installs:

- Git
- CMake
- Python 3
- Visual Studio Build Tools with C++ support
- NSIS when available, for installer packaging
- wxWidgets through local `tools/vcpkg`

If Windows just installed or updated a tool, close PowerShell, open it again, and run the same command again.

### macOS

1. Open Terminal in the batView folder.
2. Run:

```bash
./install_macos.sh
```

If you do not want the app to open at the end, use:

```bash
./install_macos.sh --no-run
```

What this installs:

- Apple command line tools
- Homebrew
- CMake
- wxWidgets
- Python
- pkg-config

If the Apple command line tools installer opens, finish it first, then run the script again.

### Linux

1. Open Terminal in the batView folder.
2. Run:

```bash
./install_linux.sh
```

If you do not want the app to open at the end, use:

```bash
./install_linux.sh --no-run
```

What this installs:

- CMake
- A C++ compiler and build tools
- wxWidgets development files
- GTK development files
- Python 3, Python headers, and pip
- Packaging helpers such as `dpkg-dev`, `fakeroot`, or `zip` when available

The Linux script works with `apt`, `dnf`, and `pacman`.

## How to Know It Worked

The install worked if:

1. The terminal ends with `batView full listo.`
2. The app opens automatically, unless you used `--no-run` or `-NoRun`.
3. A package appears in `dist/`.
4. The compiled app appears in `build-release/`.

Desktop results by system:

- Mac: `~/Desktop/batView.app`
- Linux: `~/Desktop/batView.desktop`
- Windows: `~/Desktop/batView.lnk`

## If Something Fails

Check these things first:

1. You are on the `batView` branch.
2. You are inside the `batView` folder.
3. On Windows, PowerShell was closed and opened again after tool installation.
4. On macOS, the Apple command line tools installer finished.
5. On Linux, you entered your password if `sudo` asked for it.

Then run the same install command again.

## If the Tools Are Already Installed

Use these commands when the computer already has the required build tools:

```bash
./install_macos.sh --skip-deps --no-run
./install_linux.sh --skip-deps --no-run
```

On Windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps -NoRun
```

## Clean Generated Artifacts

If you want to remove what the build scripts generated and start over, run:

```bash
./build_app.sh --clean
```

On Windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1 --clean
```

This removes `build-release/`, `dist/`, `python/runtime/`, and the Desktop launcher or bundle created for the current OS.

## Existing Build Scripts

The install scripts call the build scripts after preparing dependencies:

```bash
./build_app.sh
```

```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1
```

Use the build scripts directly only when the tools are already installed.

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
