param(
    [switch]$NoRun,
    [switch]$SkipDeps,
    [switch]$NoVcpkg,
    [switch]$Uninstall,
    [ValidateSet("x64-windows", "arm64-windows")]
    [string]$Triplet
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$vcpkgRoot = Join-Path $scriptDir "tools\vcpkg"

function Test-Command {
    param([string]$Name)
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

function Get-DefaultVcpkgTriplet {
    if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") {
        return "x64-windows"
    }

    return "x64-windows"
}

function Update-ProcessPath {
    $pathParts = @(
        [Environment]::GetEnvironmentVariable("Path", "Machine"),
        [Environment]::GetEnvironmentVariable("Path", "User"),
        [Environment]::GetEnvironmentVariable("Path", "Process")
    ) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }

    $env:Path = ($pathParts -join ";")
}

function Get-VisualStudioInstallerPath {
    $installerPath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vs_installer.exe"
    if (Test-Path $installerPath) {
        return $installerPath
    }

    throw "Visual Studio Installer was not found. Reinstall Visual Studio 2022 Build Tools."
}

function Get-VisualStudioBuildToolsPath {
    $vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswherePath) {
        $installPath = & $vswherePath `
            -products Microsoft.VisualStudio.Product.BuildTools `
            -latest `
            -property installationPath
        if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($installPath)) {
            return $installPath.Trim()
        }
    }

    $defaultPath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\2022\BuildTools"
    if (Test-Path $defaultPath) {
        return $defaultPath
    }

    throw "Visual Studio 2022 Build Tools was not found."
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
    Install-PythonDependency
    Install-WingetPackage `
        -Id "Microsoft.VisualStudio.2022.BuildTools" `
        -Name "Visual Studio Build Tools" `
        -ExtraArgs @("--override", "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended")

    Ensure-VisualStudioBuildTools
    Update-ProcessPath
}

function Ensure-VisualStudioBuildTools {
    $components = @("Microsoft.VisualStudio.Workload.VCTools")
    if ($script:BatViewVcpkgTriplet -eq "arm64-windows") {
        $components += "Microsoft.VisualStudio.Component.VC.Tools.ARM64"
    }

    $installerPath = Get-VisualStudioInstallerPath
    $installPath = Get-VisualStudioBuildToolsPath
    $modifyArgs = @(
        "modify",
        "--installPath", $installPath,
        "--quiet",
        "--norestart",
        "--includeRecommended"
    )

    foreach ($component in $components) {
        $modifyArgs += "--add"
        $modifyArgs += $component
    }

    Write-Host "Ensuring Visual Studio Build Tools components for $script:BatViewVcpkgTriplet..."
    & $installerPath @modifyArgs
    if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne 3010) {
        throw "Could not update Visual Studio Build Tools components. Open Visual Studio Installer and add Desktop development with C++ plus ARM64 build tools if needed."
    }

    if ($LASTEXITCODE -eq 3010) {
        Write-Host "Visual Studio Build Tools requested a restart. Restart Windows if the next build step still cannot find the toolchain."
    }
}

function Install-PythonDependency {
    if ($script:BatViewVcpkgTriplet -eq "x64-windows" -and $env:PROCESSOR_ARCHITECTURE -eq "ARM64") {
        Write-Host "Ensuring x64 Python 3.13 for x64 build..."
        $args = @(
            "install",
            "--id", "Python.Python.3.13",
            "--exact",
            "--source", "winget",
            "--architecture", "x64",
            "--force",
            "--silent",
            "--accept-package-agreements",
            "--accept-source-agreements"
        )
        & winget @args
        if ($LASTEXITCODE -ne 0) {
            throw "Could not install x64 Python 3.13 with winget."
        }
        return
    }

    Install-WingetPackage -Id "Python.Python.3.13" -Name "Python 3"
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

    $env:VCPKG_DEFAULT_TRIPLET = $script:BatViewVcpkgTriplet
    $env:VCPKG_TARGET_TRIPLET = $script:BatViewVcpkgTriplet
    $env:VCPKG_DEFAULT_HOST_TRIPLET = $script:BatViewVcpkgHostTriplet
    $env:VCPKG_HOST_TRIPLET = $script:BatViewVcpkgHostTriplet

    & (Join-Path $vcpkgRoot "vcpkg.exe") install "wxwidgets:$script:BatViewVcpkgTriplet" "--host-triplet=$script:BatViewVcpkgHostTriplet"
    if ($LASTEXITCODE -ne 0) {
        throw "Could not install wxWidgets with vcpkg for target $script:BatViewVcpkgTriplet and host $script:BatViewVcpkgHostTriplet. Make sure Visual Studio 2022 Build Tools has the C++ workload and the matching architecture tools installed."
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

function Get-RequiredPythonMachine {
    if ($script:BatViewVcpkgTriplet -eq "arm64-windows") {
        return "arm64"
    }

    return "amd64"
}

function Test-PythonArchitecture {
    param([string]$PythonPath)

    try {
        $machine = & $PythonPath -c "import platform; print(platform.machine().lower())" 2>$null
        if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($machine)) {
            return $false
        }

        return $machine.Trim().ToLowerInvariant() -eq (Get-RequiredPythonMachine)
    } catch {
        return $false
    }
}

function Use-PythonIfMatching {
    param([string]$PythonPath)

    if ([string]::IsNullOrWhiteSpace($PythonPath)) {
        return $null
    }

    if (Test-PythonArchitecture -PythonPath $PythonPath) {
        return $PythonPath
    }

    Write-Host "Skipping Python with wrong architecture: $PythonPath"
    return $null
}

function Find-InstalledPython {
    $candidatePaths = @(
        (Join-Path $env:LOCALAPPDATA "Programs\Python\Python313\python.exe"),
        (Join-Path $env:LOCALAPPDATA "Programs\Python\Python313-arm64\python.exe"),
        (Join-Path $env:LOCALAPPDATA "Programs\Python\Python312\python.exe"),
        (Join-Path $env:LOCALAPPDATA "Programs\Python\Python312-arm64\python.exe"),
        (Join-Path $env:ProgramFiles "Python313\python.exe"),
        (Join-Path $env:ProgramFiles "Python312\python.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Python313\python.exe"),
        (Join-Path ${env:ProgramFiles(x86)} "Python312\python.exe")
    )

    foreach ($candidate in $candidatePaths) {
        if (Test-Path $candidate) {
            $pythonPath = Get-PythonExecutable -Program $candidate
            $matchingPython = Use-PythonIfMatching -PythonPath $pythonPath
            if (-not [string]::IsNullOrWhiteSpace($matchingPython)) {
                return $matchingPython
            }
        }
    }

    return $null
}

function Get-EmbeddingPython {
    Update-ProcessPath

    $pythonPath = Get-PythonExecutable -Program "py" -Arguments @("-3.13")
    $matchingPython = Use-PythonIfMatching -PythonPath $pythonPath
    if (-not [string]::IsNullOrWhiteSpace($matchingPython)) {
        return $matchingPython
    }

    $pythonPath = Get-PythonExecutable -Program "python"
    $matchingPython = Use-PythonIfMatching -PythonPath $pythonPath
    if (-not [string]::IsNullOrWhiteSpace($matchingPython)) {
        return $matchingPython
    }

    $pythonPath = Get-PythonExecutable -Program "py" -Arguments @("-3.12")
    $matchingPython = Use-PythonIfMatching -PythonPath $pythonPath
    if (-not [string]::IsNullOrWhiteSpace($matchingPython)) {
        return $matchingPython
    }

    $pythonPath = Find-InstalledPython
    if (-not [string]::IsNullOrWhiteSpace($pythonPath)) {
        return $pythonPath
    }

    throw "No se encontro Python para compilar con arquitectura $(Get-RequiredPythonMachine). Cierra y abre PowerShell, o instala Python 3.13 x64 desde python.org o winget."
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

$script:BatViewVcpkgTriplet = $Triplet
if ([string]::IsNullOrWhiteSpace($script:BatViewVcpkgTriplet)) {
    $script:BatViewVcpkgTriplet = Get-DefaultVcpkgTriplet
}
Write-Host "vcpkg triplet: $script:BatViewVcpkgTriplet"
$script:BatViewVcpkgHostTriplet = "x64-windows"
Write-Host "vcpkg host triplet: $script:BatViewVcpkgHostTriplet"
if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64" -and $script:BatViewVcpkgTriplet -eq "x64-windows") {
    Write-Host "ARM64 Windows detected. Building x64 by default because it runs under Windows emulation and is supported by standard Build Tools."
}

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
