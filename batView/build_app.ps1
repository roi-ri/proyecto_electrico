$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

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

$pythonExe = Get-PythonExecutable -Program "py" -Arguments @("-3.13")
if ([string]::IsNullOrWhiteSpace($pythonExe)) {
    $pythonExe = Get-PythonExecutable -Program "python"
}
if ([string]::IsNullOrWhiteSpace($pythonExe)) {
    $pythonExe = Get-PythonExecutable -Program "py" -Arguments @("-3.12")
}
if ([string]::IsNullOrWhiteSpace($pythonExe)) {
    $localPython313 = Join-Path $env:LOCALAPPDATA "Programs\Python\Python313\python.exe"
    if (Test-Path $localPython313) {
        $pythonExe = $localPython313
    }
}
if ([string]::IsNullOrWhiteSpace($pythonExe)) {
    throw "No se encontro Python para ejecutar build_app.py"
}

Write-Host "Python build launcher: $pythonExe"
& $pythonExe "$scriptDir\scripts\build_app.py" --python $pythonExe @Args
