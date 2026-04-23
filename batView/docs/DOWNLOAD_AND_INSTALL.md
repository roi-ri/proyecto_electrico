# Download and Install batView

This guide explains how to build, package, and share batView on macOS, Windows, and Linux.

## Where the release app is generated

batView now creates distributable packages in:

```text
dist/
```

That folder is outside the `build-*` directories, so the app you share or document is not buried inside the build output.

## Prerequisites

### All platforms

- CMake 3.16 or newer
- A C++17 compiler
- wxWidgets development libraries

### macOS

- Xcode Command Line Tools
- wxWidgets installed with Homebrew or another package manager

### Windows

- Visual Studio 2022 with Desktop development for C++
- wxWidgets built or installed for the same compiler toolchain

### Linux

- GCC or Clang
- wxWidgets GTK development package

## Build the desktop app

### macOS or Linux

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
```

### Windows

```powershell
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
```

## Create a release package in `dist/`

### macOS

```bash
cmake --build build-release --config Release --target package
```

Expected result:

- `dist/batView-1.0.0-Darwin.zip`

### Windows

```powershell
cmake --build build-release --config Release --target package
```

Expected result:

- `dist/batView-1.0.0-Windows.zip`

### Linux

```bash
cmake --build build-release --config Release --target package
```

Expected result:

- `dist/batView-1.0.0-Linux.tar.gz`

## How to run the packaged app

### macOS

1. Extract the generated `.zip`.
2. Move `batView.app` to `Applications` if desired.
3. Launch `batView.app`.

### Windows

1. Extract the generated `.zip`.
2. Open the extracted folder.
3. Run `batView.exe`.

### Linux

1. Extract the generated `.tar.gz`.
2. Open the extracted folder.
3. Run `./batView`.

## Notes for sharing with other people

- Share the file inside `dist/`, not the full `build-release/` directory.
- On Windows and Linux, keep the `assets/` folder next to the executable after extracting the package.
- On macOS, the assets are bundled inside `batView.app`.
