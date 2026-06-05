param(
    [switch]$NoRun,
    [switch]$SkipDeps,
    [switch]$NoVcpkg
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

    winget list --id $Id --exact | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "$Name already installed."
        return
    }

    Write-Host "Installing $Name..."
    $args = @(
        "install",
        "--id", $Id,
        "--exact",
        "--silent",
        "--accept-package-agreements",
        "--accept-source-agreements"
    ) + $ExtraArgs

    & winget @args
}

function Install-Dependencies {
    Install-WingetPackage -Id "Git.Git" -Name "Git"
    Install-WingetPackage -Id "Kitware.CMake" -Name "CMake"
    Install-WingetPackage -Id "Python.Python.3.13" -Name "Python 3"
    Install-WingetPackage -Id "NSIS.NSIS" -Name "NSIS"
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
    }

    & (Join-Path $vcpkgRoot "bootstrap-vcpkg.bat")
    & (Join-Path $vcpkgRoot "vcpkg.exe") install wxwidgets:x64-windows

    $env:VCPKG_ROOT = $vcpkgRoot
    $env:BATVIEW_CMAKE_TOOLCHAIN_FILE = Join-Path $vcpkgRoot "scripts\buildsystems\vcpkg.cmake"
}

Write-Host "batView Windows setup"

if (-not $SkipDeps) {
    Install-Dependencies
    Install-VcpkgWxWidgets
} else {
    Write-Host "Skipping dependency installation."
}

$buildArgs = @()
if ($NoRun) {
    $buildArgs += "--no-run"
}

& powershell -ExecutionPolicy Bypass -File (Join-Path $scriptDir "build_app.ps1") @buildArgs
