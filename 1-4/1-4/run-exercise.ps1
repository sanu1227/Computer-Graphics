param(
    [ValidateSet(16, 17, 18, 19)][int]$Number = 18,
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [switch]$BuildOnly
)

$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (-not $installation) { throw 'Visual Studio MSBuild를 찾을 수 없습니다.' }
$msbuild = Join-Path $installation 'MSBuild/Current/Bin/MSBuild.exe'
& $msbuild (Join-Path $PSScriptRoot '1-4.vcxproj') /t:Build "/p:Configuration=$Configuration" /p:Platform=x64 "/p:Exercise=$Number" /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw "1-$Number 빌드 실패" }

$executable = Join-Path $PSScriptRoot "x64/$Configuration/1-$Number/1-$Number.exe"
if ($BuildOnly) {
    Write-Output $executable
} else {
    & $executable
    if ($LASTEXITCODE -ne 0) { throw "1-$Number 실행 실패" }
}
