# Download and Install batView

This guide explains how to install or build batView on Windows, macOS, and Linux.

## Before you start

- Do not reuse `build-release/` between different computers or operating systems.
- If you copy the project from another machine, delete any old build folder before configuring again.
- Share packaged files from `dist/`, not the full build directory.

## Option 1: Install from a packaged build

Use this option if someone already built the app for your operating system and gave you the package.

### Windows

1. Download the Windows package.
2. If you received `batView-windows-portable.zip`, extract it.
3. Open the extracted folder.
4. Run `batView.exe`.

Important:

- Keep `batView.exe`, the DLL files, and the `assets/` folder together.
- If you received an installer `.exe`, run it and follow the installer steps.

### macOS

1. Download the macOS package, usually a `.zip`.
2. Extract the file.
3. Move `batView.app` to `Applications` if you want.
4. Open `batView.app`.

If macOS blocks the app the first time:

1. Open `System Settings` -> `Privacy & Security`.
2. Find the warning for `batView`.
3. Choose `Open Anyway`.

### Linux

1. Download the Linux package.
2. Extract it.
3. Open a terminal inside the extracted folder.
4. Run:

```bash
chmod +x batView
./batView
```

Important:

- Keep the `assets/` folder next to the executable.
- On Debian or Ubuntu, if you received a `.deb`, you can install it with:

```bash
sudo apt install ./batView-*.deb
```

## Option 2: Build from source

Use this option if you want to compile batView yourself.

### Requirements for all platforms

- CMake 3.16 or newer
- A C++17 compiler
- wxWidgets development libraries

### macOS

Install dependencies:

```bash
xcode-select --install
brew install wxwidgets
```

Build:

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release
open build-release/batView.app
```

Optional package:

```bash
cpack --config build-release/CPackConfig.cmake
```

Expected package location:

- `dist/`

### Windows

Install dependencies:

- Visual Studio 2022 with `Desktop development with C++`
- wxWidgets for the same compiler toolchain you plan to use

Build:

```powershell
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release
```

Run:

```powershell
.\build-release\batView.exe
```

Optional package:

```powershell
cpack --config build-release\CPackConfig.cmake
```

### Linux

Install dependencies on Ubuntu 24.04:

```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev
```

Build:

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release
./build-release/batView
```

Optional package:

```bash
cpack --config build-release/CPackConfig.cmake
```

Expected package location:

- `dist/`

## Common problems

### `CMakeCache.txt directory is different`

Delete the old build folder and configure again:

```bash
rm -rf build-release
```

On Windows PowerShell:

```powershell
Remove-Item -Recurse -Force build-release
```

### `Could NOT find wxWidgets`

Make sure the wxWidgets development package is installed for your platform and compiler.

### The app opens but images are missing

On Windows and Linux, confirm the `assets/` folder is next to the executable.
