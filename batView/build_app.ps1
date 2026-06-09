$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

$pythonProbe = @'
import pathlib
import sys
import sysconfig

version = f"{sys.version_info.major}{sys.version_info.minor}"
include_dir = pathlib.Path(sysconfig.get_path("include") or "")
stdlib_dir = pathlib.Path(sysconfig.get_path("stdlib") or "")
base_prefix = pathlib.Path(sys.base_prefix)
exe_dir = pathlib.Path(sys.executable).parent
library_name = f"python{version}.lib"
library_candidates = [
    base_prefix / "libs" / library_name,
    exe_dir / "libs" / library_name,
    base_prefix / library_name,
    exe_dir / library_name,
]

usable = (
    (include_dir / "Python.h").exists()
    and stdlib_dir.exists()
    and any(path.exists() for path in library_candidates)
)

print(sys.executable if usable else "")
'@

function Test-PythonForEmbedding {
    param([string[]]$Command)

    $program = $Command[0]
    $programArgs = @()
    if ($Command.Length -gt 1) {
        $programArgs = $Command[1..($Command.Length - 1)]
    }

    try {
        $result = & $program @programArgs -c $pythonProbe 2>$null
        if ($LASTEXITCODE -eq 0 -and -not [string]::IsNullOrWhiteSpace($result)) {
            return $true
        }
    } catch {
        return $false
    }

    return $false
}

$candidateCommands = @(
    @("py", "-3.13"),
    @("py", "-3.12"),
    @("py"),
    @("python"),
    @("python3"),
    @((Join-Path $env:LOCALAPPDATA "Programs\Python\Python313\python.exe")),
    @((Join-Path $env:LOCALAPPDATA "Programs\Python\Python312\python.exe")),
    @((Join-Path $env:ProgramFiles "Python313\python.exe")),
    @((Join-Path $env:ProgramFiles "Python312\python.exe")),
    @((Join-Path ${env:ProgramFiles(x86)} "Python313\python.exe")),
    @((Join-Path ${env:ProgramFiles(x86)} "Python312\python.exe"))
)

$selectedCommand = $null
foreach ($candidate in $candidateCommands) {
    if (Test-PythonForEmbedding -Command $candidate) {
        $selectedCommand = $candidate
        break
    }
}

if ($null -eq $selectedCommand) {
    throw "No se encontro un Python valido para compilar Python embebido. Instala Python desde python.org o con: winget install --id Python.Python.3.13 --exact --accept-package-agreements --accept-source-agreements"
}

$pythonCmd = $selectedCommand[0]
$pythonArgs = @()
if ($selectedCommand.Length -gt 1) {
    $pythonArgs = $selectedCommand[1..($selectedCommand.Length - 1)]
}

& $pythonCmd @pythonArgs "$scriptDir\scripts\build_app.py" @Args
