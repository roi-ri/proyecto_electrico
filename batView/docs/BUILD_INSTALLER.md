# Build and Installer Scripts

This page is now an index. Use the guide for your operating system:

- [macOS installation](./INSTALL_MACOS.md)
- [Linux installation](./INSTALL_LINUX.md)
- [Windows installation](./INSTALL_WINDOWS.md)

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

## Build Scripts

The install scripts call the build scripts after preparing dependencies:

```bash
./build_app.sh
```

```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1
```

Use the build scripts directly only when the tools are already installed.
