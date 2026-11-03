param([string]$Number = '*', [switch]$RunChecks)
$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$installation = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (-not $installation) { throw 'Visual Studio MSBuild를 찾을 수 없습니다.' }
$msbuild = Join-Path $installation 'MSBuild/Current/Bin/MSBuild.exe'
$files = @(Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'examples') -Filter '*.cpp' | Where-Object { $_.BaseName -like "$Number`_*" } | Sort-Object Name)
if ($files.Count -eq 0) { throw '해당 번호의 .cpp가 없습니다. 전체는 *를 사용하세요.' }
$summary = @()
foreach ($file in $files) {
    $folder = Join-Path $PSScriptRoot ".validation/$($file.BaseName)"
    New-Item -ItemType Directory -Force -Path $folder | Out-Null
    $source = [System.Security.SecurityElement]::Escape($file.FullName)
    $project = @"
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
 <ItemGroup Label="ProjectConfigurations"><ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration></ItemGroup>
 <PropertyGroup Label="Globals"><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
 <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
 <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v145</PlatformToolset><UseDebugLibraries>false</UseDebugLibraries><LinkIncremental>false</LinkIncremental></PropertyGroup>
 <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.props" />
 <ImportGroup Label="PropertySheets"><Import Project="`$(UserRootDir)\Microsoft.Cpp.x64.user.props" Condition="exists('`$(UserRootDir)\Microsoft.Cpp.x64.user.props')" /></ImportGroup>
 <ItemDefinitionGroup><ClCompile><LanguageStandard>stdcpp17</LanguageStandard><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><ExceptionHandling>Sync</ExceptionHandling><DebugInformationFormat>None</DebugInformationFormat></ClCompile><Link><AdditionalDependencies>opengl32.lib;glew32.lib;glfw3.lib;%(AdditionalDependencies)</AdditionalDependencies><SubSystem>Console</SubSystem><GenerateDebugInformation>false</GenerateDebugInformation></Link></ItemDefinitionGroup>
 <ItemGroup><ClCompile Include="$source" /></ItemGroup>
 <Import Project="`$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
"@
    $projectFile = Join-Path $folder 'check.vcxproj'
    Set-Content -LiteralPath $projectFile -Value $project -Encoding utf8
    $log = Join-Path $folder 'build.log'
    & $msbuild $projectFile /t:Build /p:Configuration=Release /p:Platform=x64 "/p:OutDir=$folder/" "/p:IntDir=$folder/obj/" /p:TargetName=check /v:minimal /nologo *> $log
    if ($LASTEXITCODE -ne 0) { Get-Content -LiteralPath $log -Tail 20; throw "$($file.Name) 빌드 실패" }
    if ($RunChecks) {
        & (Join-Path $folder 'check.exe') --check *> (Join-Path $folder 'check.log')
        if ($LASTEXITCODE -ne 0) { Get-Content (Join-Path $folder 'check.log') -Tail 10; throw "$($file.Name) 검사 실패" }
    }
    $summary += "$($file.Name): PASS (build=$true, runtime=$RunChecks)"
    Write-Output $summary[-1]
}
$summary | Set-Content -LiteralPath (Join-Path $PSScriptRoot '.validation/results.txt') -Encoding utf8
