$ErrorActionPreference = 'Stop'

$vswhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
if (-not (Test-Path $vswhere)) {
    $vswhere = "$env:ProgramFiles/Microsoft Visual Studio/Installer/vswhere.exe"
}
if (-not (Test-Path $vswhere)) {
    throw 'Visual Studio Installer vswhere.exe was not found'
}

$installations = & $vswhere -latest -products '*' -version '[17.0,19.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64 -format json
if ($LASTEXITCODE -ne 0) { throw 'Visual Studio discovery failed' }
$installation = $installations | ConvertFrom-Json | Select-Object -First 1
if (-not $installation) {
    throw 'Visual Studio 2022 or 2026 with ARM64 C++ tools is required'
}

$major = ([version]$installation.installationVersion).Major
$generator = switch ($major) {
    17 { 'Visual Studio 17 2022' }
    18 { 'Visual Studio 18 2026' }
    default { throw "Unsupported Visual Studio version: $major" }
}
Write-Host "Using $generator at $($installation.installationPath)"
[PSCustomObject]@{
    Generator = $generator
    InstallationPath = $installation.installationPath
}
