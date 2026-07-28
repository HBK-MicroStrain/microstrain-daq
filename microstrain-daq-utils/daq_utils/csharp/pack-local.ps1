<#
.SYNOPSIS
  Vendors openDAQ.Net (Linux + Windows natives) and packs MicroStrain.DaqUtils locally.

.DESCRIPTION
  Mirrors the build-nuget job in .github/workflows/cd.yml so you can test local changes to
  DaqUtils.cs / Wireless.cs against a real .nupkg without waiting on a release. Requires
  7-Zip (7z on PATH, or installed at the default location) and the .NET 8 SDK.

.PARAMETER Force
  Re-download and re-vendor openDAQ.Net even if it's already present locally. Use this if
  the vendored folder looks incomplete or you want to pick up a newer openDAQ.Net release.

.EXAMPLE
  ./pack-local.ps1
  Produces microstrain-daq-utils/nuget-output/MicroStrain.DaqUtils.<version>.nupkg

.EXAMPLE
  dotnet add package MicroStrain.DaqUtils --version <version> --source <path from output>
  Install the result into a separate throwaway test project (see README for the full flow).
#>
param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"

# openDAQ release these are vendored from - bump both together when updating.
$openDaqTag            = "v3.40.0"
$openDaqNetPackageName = "opendaq.net.3.40.0.23.nupkg"
$openDaqWinInstaller   = "opendaq-3.40.0-x86_64-windows-msvc-143-release.exe"

$csharpDir   = $PSScriptRoot
$csproj      = Join-Path $csharpDir "DaqUtils.csproj"
$opendaqNet  = Join-Path $csharpDir "opendaq-net"
$repoRoot    = Resolve-Path (Join-Path $csharpDir "..\..\..")
$outputDir   = Join-Path $repoRoot "nuget-output"
$workDir     = Join-Path $csharpDir ".pack-local-work"

function Find-7z {
    $cmd = Get-Command 7z -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $default = "C:\Program Files\7-Zip\7z.exe"
    if (Test-Path $default) { return $default }
    throw "7-Zip not found. Install it first, e.g.:`n  winget install 7zip.7zip`nor download from https://www.7-zip.org/"
}

$hasLinux = Test-Path (Join-Path $opendaqNet "runtimes\linux-x64\native")
$hasWin   = Test-Path (Join-Path $opendaqNet "runtimes\win-x64\native")

if ($Force -or -not ($hasLinux -and $hasWin)) {
    Write-Host "Vendoring openDAQ.Net ($openDaqTag) - this only needs to happen once per machine..." -ForegroundColor Cyan

    $sevenZip = Find-7z
    New-Item -ItemType Directory -Force -Path $workDir | Out-Null
    Remove-Item -Recurse -Force $opendaqNet -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $opendaqNet | Out-Null

    # --- Linux natives: openDAQ.Net's own nupkg ships these ---
    $netNupkg = Join-Path $workDir $openDaqNetPackageName
    Write-Host "  Downloading $openDaqNetPackageName ..."
    Invoke-WebRequest -Uri "https://github.com/openDAQ/openDAQ/releases/download/$openDaqTag/$openDaqNetPackageName" -OutFile $netNupkg
    & $sevenZip x -y $netNupkg "-o$opendaqNet" lib runtimes LICENSE | Out-Null

    # --- Windows natives: backfilled from the Windows SDK installer ---
    # openDAQ.Net's own nupkg has been missing runtimes/win-x64 since 3.40.0.23 (their pack
    # step silently drops a RID when that artifact wasn't available at pack time). Pull the
    # same binaries from the Windows installer released alongside it instead. Keep this in
    # sync with the equivalent step in .github/workflows/cd.yml.
    $winExe = Join-Path $workDir $openDaqWinInstaller
    Write-Host "  Downloading $openDaqWinInstaller (Windows natives) ..."
    Invoke-WebRequest -Uri "https://github.com/openDAQ/openDAQ/releases/download/$openDaqTag/$openDaqWinInstaller" -OutFile $winExe
    $winExtract = Join-Path $workDir "win-installer"
    & $sevenZip x -y $winExe "-o$winExtract" bin | Out-Null

    $winNative = Join-Path $opendaqNet "runtimes\win-x64\native"
    New-Item -ItemType Directory -Force -Path (Join-Path $winNative "modules") | Out-Null
    Copy-Item (Join-Path $winExtract "bin\copendaq.dll") $winNative
    Copy-Item (Join-Path $winExtract "bin\daqcoreobjects-64-3.dll") $winNative
    Copy-Item (Join-Path $winExtract "bin\daqcoretypes-64-3.dll") $winNative
    Copy-Item (Join-Path $winExtract "bin\opendaq-64-3.dll") $winNative
    Copy-Item (Join-Path $winExtract "bin\modules\*.dll") (Join-Path $winNative "modules")

    Remove-Item -Recurse -Force $workDir
    Write-Host "  Vendored openDAQ.Net for linux-x64 and win-x64." -ForegroundColor Green
}
else {
    Write-Host "openDAQ.Net already vendored (use -Force to re-fetch)." -ForegroundColor DarkGray
}

Write-Host "Packing DaqUtils.csproj ..." -ForegroundColor Cyan
dotnet pack $csproj --configuration Release --output $outputDir
if ($LASTEXITCODE -ne 0) { throw "dotnet pack failed." }

$nupkg = Get-ChildItem $outputDir -Filter "MicroStrain.DaqUtils.*.nupkg" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $nupkg) { throw "Pack succeeded but no MicroStrain.DaqUtils .nupkg was found in $outputDir" }

$version = $nupkg.BaseName -replace '^MicroStrain\.DaqUtils\.', ''

Write-Host ""
Write-Host "Packed: $($nupkg.FullName)" -ForegroundColor Green
Write-Host ""
Write-Host "To test it in a separate project:" -ForegroundColor Yellow
Write-Host "  dotnet add package MicroStrain.DaqUtils --version $version --source `"$outputDir`""
