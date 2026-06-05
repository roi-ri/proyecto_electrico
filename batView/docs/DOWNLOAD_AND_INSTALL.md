# Download batView

Use the guide for your operating system:

- [Windows download guide](./DOWNLOAD_WINDOWS.md)
- [macOS download guide](./DOWNLOAD_MACOS.md)
- [Linux download guide](./DOWNLOAD_LINUX.md)

## Before you start

- Download the package made for your own operating system.
- When releases are published, the ready-to-download `.zip`, `.tar.gz`, or `.deb` files will be attached to the GitHub Release page for the matching version.
- Do not reuse `build-release/` from another computer or another OS.
- If someone shares the app with you, ask them for the packaged files from `dist/`.
- On Windows and Linux, keep the executable and the `assets/` folder together.

## If you need to build the app yourself

This download center is for ready-to-use app packages.

If you want to prepare the computer, build batView, and generate an installer-like package from source, use:

- [BUILD_INSTALLER.md](./BUILD_INSTALLER.md)

Quick commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

```bash
./install_macos.sh
./install_linux.sh
```

For architecture and code details, see:

- [PROJECT_DOCUMENTATION.md](./PROJECT_DOCUMENTATION.md)
- [ARCHITECTURE.md](./ARCHITECTURE.md)
