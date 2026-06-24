# Windows Installation From Zero

Use this guide if your computer runs Windows and you do not have Git,
CMake, Python, Visual Studio Build Tools, or batView installed yet.

The expected result is:

- batView installed at `%LOCALAPPDATA%\batView\batView.exe`.
- A Desktop shortcut named `batView.lnk`.
- Local build files in the project folder under `build-release\`.

## Step 1: Open PowerShell

Open `PowerShell` as a normal user.

You can find it from the Start menu by searching for `PowerShell`.

## Step 2: Check That `winget` Works

batView uses `winget` to install Windows tools automatically.

Run:

```powershell
winget --version
```

If you see a version number, continue to the next step.

If Windows says that `winget` is not recognized:

1. Open the Microsoft Store.
2. Search for `App Installer`.
3. Install or update `App Installer`.
4. Close PowerShell.
5. Open PowerShell again and run `winget --version` one more time.

## Step 3: Install Git

Git is needed first because it downloads the batView source code.

Run:

```powershell
winget install --id Git.Git --exact --silent --accept-package-agreements --accept-source-agreements
```

If `winget` complains about `msstore` but says Git was found in the `winget`
source, run the same command with `--source winget`:

```powershell
winget install --id Git.Git --exact --source winget --silent --accept-package-agreements --accept-source-agreements
```

When the command finishes, close PowerShell and open it again so Windows
refreshes the terminal PATH.

Confirm Git is available:

```powershell
git --version
```

If `git` is still not recognized, add Git to the Windows PATH manually.

### Add Git to PATH manually

1. Open the Start menu.
2. Search for `Environment Variables`.
3. Click `Edit the system environment variables`.
4. In the `System Properties` window, click `Environment Variables...`.
5. Under `User variables for rodrigo`, select `Path`.
6. Click `Edit`.
7. Click `New`.
8. Add this path:

   ```text
   C:\Program Files\Git\cmd
   ```

9. Click `OK` on each window to save the changes.
10. Close PowerShell.
11. Open PowerShell again.
12. Run:

```powershell
git --version
```

If Git was installed somewhere else, first find `git.exe` and add the folder
that contains it. The usual Git path is `C:\Program Files\Git\cmd\git.exe`.

## Step 4: Download batView

Choose where you want to keep the project. This example uses your Documents
folder.

Run:

```powershell
cd $env:USERPROFILE\Documents
git clone https://github.com/roi-ri/proyecto_electrico.git
cd .\proyecto_electrico\batView
```

You must be inside the `batView` folder before running the install script.

To confirm you are in the correct folder, run:

```powershell
dir
```

You should see files such as:

- `install_windows.ps1`
- `build_app.ps1`
- `CMakeLists.txt`
- `src`
- `docs`

## Step 5: Run the batView Installer

From the `batView` folder, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

The script installs or prepares:

1. Git, if needed.
2. CMake.
3. Python 3.13.
4. Visual Studio 2022 Build Tools with the C++ workload.
5. `vcpkg` inside `tools\vcpkg`.
6. `wxWidgets` for the detected Windows architecture through `vcpkg`.
7. batView itself.

On ARM64 Windows, the installer uses `arm64-windows` instead of `x64-windows`
and asks Visual Studio Build Tools for the ARM64 C++ tools.

This step can take a while the first time because Visual Studio Build Tools,
wxWidgets, and the C++ build are large.

If the install stops after a failed first attempt, update the project and run it
again:

```powershell
git pull
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

If the failure happened while installing `vcpkg` or `wxWidgets`, you can remove
the partial `tools` folder before trying again:

```powershell
Remove-Item -Recurse -Force .\tools
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

## Step 6: Wait for the Success Message

When everything works, the terminal ends with:

```text
batView full listo.
```

The installer also creates:

- `%LOCALAPPDATA%\batView\batView.exe`
- A Desktop shortcut named `batView.lnk`
- `build-release\` inside the `batView` project folder

If you did not use `-NoRun`, batView opens automatically at the end.

## Optional: Install Without Opening batView

If you want to install batView but not open it automatically, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -NoRun
```

## Optional: Run Again After Everything Is Installed

If Visual Studio Build Tools, CMake, Python, Git, `vcpkg`, and wxWidgets are
already installed or prepared, you can skip dependency installation:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps
```

To skip dependencies and also avoid opening the app:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps -NoRun
```

To force a specific vcpkg target architecture, use `-Triplet`:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -Triplet arm64-windows
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -Triplet x64-windows
```

## Optional: Update batView Later

Open PowerShell and go back to the project:

```powershell
cd $env:USERPROFILE\Documents\proyecto_electrico\batView
git pull
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

## Uninstall

To remove the local batView install, Desktop shortcut, generated build files,
Python runtime cache, and the project-local `tools` folder, run this from the
`batView` folder:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -Uninstall
```

This does not remove Visual Studio Build Tools, CMake, Python, Git, or
`winget`.

## Common Problems

- `winget` is not recognized: install or update `App Installer` from the
  Microsoft Store, close PowerShell, and open it again.
- `winget` fails with `Failed when searching source: msstore`: rerun the
  command with `--source winget`, for example
  `winget install --id Git.Git --exact --source winget --silent --accept-package-agreements --accept-source-agreements`.
- `Unable to find a valid Visual Studio instance`: Visual Studio Build Tools
  did not install correctly. Run `git pull`, remove the partial `tools` folder
  with `Remove-Item -Recurse -Force .\tools`, and run
  `powershell -ExecutionPolicy Bypass -File .\install_windows.ps1` again.
- `Unable to find a valid toolchain for requested target architecture arm64`:
  update the repo with `git pull`, remove the partial `tools` folder with
  `Remove-Item -Recurse -Force .\tools`, and run the installer again. The
  current installer adds the Visual Studio ARM64 C++ tools and uses the
  `arm64-windows` triplet automatically on ARM64 Windows.
- `git` is not recognized after installing it: close PowerShell and open a new
  PowerShell window. If it still fails, add `C:\Program Files\Git\cmd` to your
  user `Path` environment variable, then reopen PowerShell.
- `.\install_windows.ps1 does not exist`: you are not inside the `batView`
  folder. Run `cd $env:USERPROFILE\Documents\proyecto_electrico\batView`.
- Python link errors mention `pythonXY.lib`: install Python from python.org or
  with `winget install --id Python.Python.3.13 --exact`; do not rely only on
  the Microsoft Store Python app alias.
- `Could NOT find wxWidgets`: run the installer without `-SkipDeps` so it can
  install `wxwidgets:x64-windows` through `vcpkg`.
- Visual Studio or C++ compiler errors: rerun the installer without
  `-SkipDeps` so it can install Visual Studio 2022 Build Tools with the C++
  workload.
