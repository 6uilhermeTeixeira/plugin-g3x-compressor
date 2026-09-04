[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$BundlePath,
    [string]$Destination = "dist"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if (-not (Test-Path -LiteralPath $BundlePath -PathType Container)) {
    throw "VST3 bundle not found: $BundlePath"
}
$bundle = Get-Item -LiteralPath $BundlePath
if ($bundle.Extension -ne ".vst3") { throw "Expected a .vst3 bundle directory" }
$relativeBinary = "$($bundle.Name)/Contents/x86_64-win/$($bundle.Name)"
$sourceBinary = Join-Path $bundle.FullName "Contents/x86_64-win/$($bundle.Name)"
if (-not (Test-Path -LiteralPath $sourceBinary -PathType Leaf)) {
    throw "Windows x64 VST3 binary not found: $sourceBinary"
}
if ((Get-Item -LiteralPath $sourceBinary).Length -eq 0) { throw "VST3 binary is empty" }

# Refuse stale output instead of publishing files left by an earlier build.
if (Test-Path -LiteralPath $Destination) {
    if (-not (Test-Path -LiteralPath $Destination -PathType Container)) {
        throw "Destination must be a directory"
    }
    if (@(Get-ChildItem -LiteralPath $Destination -Force).Count -ne 0) {
        throw "Destination must be empty: $Destination"
    }
} else {
    New-Item -ItemType Directory -Path $Destination | Out-Null
}
Copy-Item -LiteralPath $bundle.FullName -Destination $Destination -Recurse
$binary = Join-Path $Destination $relativeBinary
$hash = (Get-FileHash -LiteralPath $binary -Algorithm SHA256).Hash.ToLowerInvariant()
"$hash  $relativeBinary" | Set-Content -LiteralPath (Join-Path $Destination "SHA256SUMS.txt") -Encoding utf8NoBOM

$entries = @(Get-ChildItem -LiteralPath $Destination -Force)
if ($entries.Count -ne 2 -or
    -not (Test-Path -LiteralPath (Join-Path $Destination $bundle.Name) -PathType Container) -or
    -not (Test-Path -LiteralPath (Join-Path $Destination "SHA256SUMS.txt") -PathType Leaf)) {
    throw "Package must contain only the VST3 bundle and SHA256SUMS.txt"
}
Write-Host "Packaged $($bundle.Name): SHA256 $hash"
