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

function Get-EmbeddingPython {
    $probe = @'
import pathlib
import sys
import sysconfig

version = f"{sys.version_info.major}{sys.version_info.minor}"
library = pathlib.Path(sys.base_prefix) / "libs" / f"python{version}.lib"
include = pathlib.Path(sysconfig.get_path("include") or "") / "Python.h"
stdlib = pathlib.Path(sysconfig.get_path("stdlib") or "")

print(sys.executable if library.exists() and include.exists() and stdlib.exists() else "")
'@

    $commands = @(
        ,@("py", "-3.13"),
        ,@("py", "-3.12"),
        ,@("py"),
        ,@("python"),
        ,@("python3"),
        ,@((Join-Path $env:LOCALAPPDATA "Programs\Python\Python313\python.exe")),
        ,@((Join-Path $env:LOCALAPPDATA "Programs\Python\Python312\python.exe")),
        ,@((Join-Path $env:ProgramFiles "Python313\python.exe")),
        ,@((Join-Path $env:ProgramFiles "Python312\python.exe"))
    )

    foreach ($command in $commands) {
        $program = $command[0]
        $programArgs = @()
        if ($command.Length -gt 1) {
            $programArgs = $command[1..($command.Length - 1)]
        }

        try {
            $pythonPath = & $program @programArgs -c $probe 2>$null
            if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($pythonPath)) {
                return $pythonPath.Trim()
            }
        } catch {
            continue
        }
    }

    throw "No se encontro un Python valido para compilar Python embebido. Verifica que Python 3.13 tenga libs\python313.lib."
}

Write-Host "batView Windows setup"

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
