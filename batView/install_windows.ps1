param(
    [switch]$NoRun,
    [switch]$SkipDeps,
    [switch]$NoVcpkg,
    [switch]$Uninstall
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$vcpkgRoot = Join-Path $scriptDir "tools\vcpkg"

function Test-Command {
    param([string]$Name)
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

function Install-WingetPackage {
    param(
        [string]$Id,
        [string]$Name,
        [string[]]$ExtraArgs = @()
    )

    if (-not (Test-Command winget)) {
        throw "winget is required for automatic dependency installation. Install App Installer from Microsoft Store or install dependencies manually."
    }

    winget list --id $Id --exact --source winget | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "$Name already installed."
        return
    }

    Write-Host "Installing $Name..."
    $args = @(
        "install",
        "--id", $Id,
        "--exact",
        "--source", "winget",
        "--silent",
        "--accept-package-agreements",
        "--accept-source-agreements"
    ) + $ExtraArgs

    & winget @args
    if ($LASTEXITCODE -ne 0) {
        throw "Could not install $Name with winget. Try running this command manually: winget install --id $Id --exact --source winget"
    }
}

function Install-Dependencies {
    Install-WingetPackage -Id "Git.Git" -Name "Git"
    Install-WingetPackage -Id "Kitware.CMake" -Name "CMake"
    Install-WingetPackage -Id "Python.Python.3.13" -Name "Python 3"
    Install-WingetPackage `
        -Id "Microsoft.VisualStudio.2022.BuildTools" `
        -Name "Visual Studio Build Tools" `
        -ExtraArgs @("--override", "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended")
}

function Install-VcpkgWxWidgets {
    if ($NoVcpkg) {
        Write-Host "Skipping vcpkg setup."
        return
    }

    if (-not (Test-Command git)) {
        throw "Git is required to bootstrap vcpkg. Close and reopen PowerShell if Git was just installed."
    }

    if (-not (Test-Path $vcpkgRoot)) {
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $vcpkgRoot) | Out-Null
        git clone https://github.com/microsoft/vcpkg.git $vcpkgRoot
        if ($LASTEXITCODE -ne 0) {
            throw "Could not clone vcpkg. Check your internet connection and Git installation."
        }
    }

    & (Join-Path $vcpkgRoot "bootstrap-vcpkg.bat")
    if ($LASTEXITCODE -ne 0) {
        throw "Could not bootstrap vcpkg."
    }

    & (Join-Path $vcpkgRoot "vcpkg.exe") install wxwidgets:x64-windows
    if ($LASTEXITCODE -ne 0) {
        throw "Could not install wxWidgets with vcpkg. Make sure Visual Studio 2022 Build Tools with the C++ workload is installed."
    }

    $env:VCPKG_ROOT = $vcpkgRoot
    $env:BATVIEW_CMAKE_TOOLCHAIN_FILE = Join-Path $vcpkgRoot "scripts\buildsystems\vcpkg.cmake"
}

function Get-PythonExecutable {
    param(
        [string]$Program,
        [string[]]$Arguments = @()
    )

    try {
        $pythonPath = & $Program @Arguments -c "import sys; print(sys.executable)" 2>$null
        if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($pythonPath)) {
            return $pythonPath.Trim()
        }
    } catch {
        return $null
    }

    return $null
}

function Get-EmbeddingPython {
    $pythonPath = Get-PythonExecutable -Program "py" -Arguments @("-3.13")
    if (-not [string]::IsNullOrWhiteSpace($pythonPath)) {
        return $pythonPath
    }

    $pythonPath = Get-PythonExecutable -Program "python"
    if (-not [string]::IsNullOrWhiteSpace($pythonPath)) {
        return $pythonPath
    }

    $pythonPath = Get-PythonExecutable -Program "py" -Arguments @("-3.12")
    if (-not [string]::IsNullOrWhiteSpace($pythonPath)) {
        return $pythonPath
    }

    $localPython313 = Join-Path $env:LOCALAPPDATA "Programs\Python\Python313\python.exe"
    if (Test-Path $localPython313) {
        return $localPython313
    }

    throw "No se encontro Python para compilar. Verifica que `py -3.13` o `python` funcionen."
}

function Remove-BatViewPath {
    param([string]$Path)

    if (Test-Path $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
        Write-Host "Removed $Path"
    }
}

function Uninstall-BatView {
    Remove-BatViewPath -Path (Join-Path $env:LOCALAPPDATA "batView")
    Remove-BatViewPath -Path (Join-Path ([Environment]::GetFolderPath("Desktop")) "batView.lnk")
    Remove-BatViewPath -Path (Join-Path $scriptDir "build-release")
    Remove-BatViewPath -Path (Join-Path $scriptDir "python\runtime")
    Remove-BatViewPath -Path (Join-Path $scriptDir "tools")
    Write-Host "batView uninstall complete."
}

Write-Host "batView Windows setup"

if ($Uninstall) {
    Uninstall-BatView
    return
}

if (-not $SkipDeps) {
    Install-Dependencies
    Install-VcpkgWxWidgets
} else {
    Write-Host "Skipping dependency installation."
}

$buildArgs = @()
$embeddingPython = Get-EmbeddingPython
Write-Host "Python embebido: $embeddingPython"
$buildArgs += "--python"
$buildArgs += $embeddingPython

if ($NoRun) {
    $buildArgs += "--no-run"
}

& powershell -ExecutionPolicy Bypass -File (Join-Path $scriptDir "build_app.ps1") @buildArgs
if ($LASTEXITCODE -ne 0) {
    throw "batView build failed."
}
