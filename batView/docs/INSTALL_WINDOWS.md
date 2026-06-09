# Windows Installation

Use this guide only if your computer runs Windows.

## If Nothing Is Installed

Follow these steps in order.

### Step 1: Open PowerShell

Open `PowerShell` as a normal user and move into the project folder:

```powershell
cd C:\path\to\proyecto_electrico\batView
```

The script is inside the `batView` folder. If you run it from the parent
`proyecto_electrico` folder, PowerShell will say that `.\install_windows.ps1`
does not exist.

### Step 2: Check if `winget` is available

The Windows installer script uses `winget` to install missing tools automatically.

Check it with:

```powershell
winget --version
```

If that command fails, install `App Installer` from the Microsoft Store first, then open PowerShell again.

### Step 3: Install the required build tools

The project needs:

1. Visual Studio 2022 Build Tools with the C++ workload.
2. CMake.
3. Python 3 from python.org or `winget` package `Python.Python.3.13`.
4. Git.
5. NSIS.
6. `wxWidgets`, which the script installs through `vcpkg`.

You can install the main tools one by one with these commands:

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --silent --accept-package-agreements --accept-source-agreements --override "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
winget install --id Kitware.CMake --exact --silent --accept-package-agreements --accept-source-agreements
winget install --id Python.Python.3.13 --exact --silent --accept-package-agreements --accept-source-agreements
winget install --id Git.Git --exact --silent --accept-package-agreements --accept-source-agreements
winget install --id NSIS.NSIS --exact --silent --accept-package-agreements --accept-source-agreements
```

After those finish, close PowerShell and open it again so commands like `git` and `cmake` are available in the terminal.
If you forget this step, the build script also checks the usual CMake install folders on Windows.

Avoid using only the Microsoft Store Python app alias. It can run Python scripts, but it may not include
`libs\pythonXY.lib`, which is required to link batView with embedded Python.

### Step 4: Run the project installer

From the `batView` folder, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

This script will:

1. Verify or install the required Windows tools.
2. Clone `vcpkg` into `tools\vcpkg` if it is missing.
3. Install `wxwidgets:x64-windows` through `vcpkg`.
4. Build batView.

### Step 5: Optional no-run mode

If you do not want the app to open at the end, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -NoRun
```

## If Everything Is Installed

If Visual Studio Build Tools, CMake, Python, Git, NSIS, and wxWidgets are already installed, you can skip the dependency step.

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
