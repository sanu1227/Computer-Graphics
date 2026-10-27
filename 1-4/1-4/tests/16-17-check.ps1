param([ValidateSet(16, 17)][int]$Number = 16)

$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (-not $installation) { throw 'Visual Studio MSBuild를 찾을 수 없습니다.' }
$msbuild = Join-Path $installation 'MSBuild/Current/Bin/MSBuild.exe'
$projectPath = Join-Path $PSScriptRoot '../1-4.vcxproj'
[xml]$project = Get-Content -LiteralPath $projectPath -Raw
$namespace = [System.Xml.XmlNamespaceManager]::new($project.NameTable)
$namespace.AddNamespace('p', $project.DocumentElement.NamespaceURI)
# 기존 프로젝트의 컴파일 설정을 사용하되, 임시 프로젝트에서 검사 파일만 빌드한다.
foreach ($item in @($project.SelectNodes('//p:ClCompile[@Include]', $namespace))) {
    $item.ParentNode.RemoveChild($item) | Out-Null
}
foreach ($target in @($project.SelectNodes('//p:Target', $namespace))) {
    $target.ParentNode.RemoveChild($target) | Out-Null
}
$group = $project.CreateElement('ItemGroup', $project.DocumentElement.NamespaceURI)
$compile = $project.CreateElement('ClCompile', $project.DocumentElement.NamespaceURI)
$compile.SetAttribute('Include', (Join-Path $PSScriptRoot '16-17-check.cpp'))
$options = $project.CreateElement('AdditionalOptions', $project.DocumentElement.NamespaceURI)
$options.InnerText = "/FC /utf-8 /DASSIGNMENT=$Number %(AdditionalOptions)"
$compile.AppendChild($options) | Out-Null
$group.AppendChild($compile) | Out-Null
$project.DocumentElement.AppendChild($group) | Out-Null
$testFolder = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "../x64/tests/1-$Number"))
New-Item -ItemType Directory -Force -Path $testFolder | Out-Null
$testProject = Join-Path $testFolder 'check.vcxproj'
$project.Save($testProject)
& $msbuild $testProject /t:Build /p:Configuration=Release /p:Platform=x64 "/p:OutDir=$testFolder/" "/p:IntDir=$testFolder/obj/" /p:TargetName=check /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw '검사 프로그램 빌드 실패' }
Push-Location $testFolder
try {
    & (Join-Path $testFolder 'check.exe')
    if ($LASTEXITCODE -ne 0) { throw "1-$Number 검사 실패" }
    Write-Output "검사 이미지: $testFolder"
} finally {
    Pop-Location
}
