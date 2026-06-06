# Windows Installation

Use this guide only if your computer runs Windows.

## If Nothing Is Installed

Install these first:

1. Visual Studio 2022 with the `Desktop development with C++` workload.
2. CMake.
3. Python 3.
4. wxWidgets compatible with your compiler.
5. Git.
6. NSIS, if you want an installer package instead of only a portable build.

Then run the installer script from the `batView` folder in PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

If you do not want the app to open at the end, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -NoRun
```

## If Everything Is Installed

If Visual Studio Build Tools, CMake, Python, Git, and wxWidgets are already installed, you can skip the dependency step.

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps
```

If you also do not want the app to open, use:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps -NoRun
```

## What You Should See

When it works, the script ends with `batView full listo.` and creates a Desktop shortcut named `batView.lnk`.
