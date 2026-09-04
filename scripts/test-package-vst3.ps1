$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$packageScript = Join-Path $PSScriptRoot "package-vst3.ps1"
$temp = Join-Path ([System.IO.Path]::GetTempPath()) ([System.IO.Path]::GetRandomFileName())
New-Item -ItemType Directory -Path $temp | Out-Null
function Assert-Fails([scriptblock]$Action, [string]$Message) {
    $failed = $false
    try { & $Action } catch { $failed = $true }
    if (-not $failed) { throw "Expected failure: $Message" }
}
try {
    $bundle = Join-Path $temp "G3X Test Plugin.vst3"
    $binary = Join-Path $bundle "Contents/x86_64-win/G3X Test Plugin.vst3"
    $resource = Join-Path $bundle "Contents/Resources/moduleinfo.json"
    $destination = Join-Path $temp "dist"
    Assert-Fails { & $packageScript -BundlePath $bundle -Destination $destination } "missing bundle"
    New-Item -ItemType Directory -Path (Split-Path $resource) -Force | Out-Null
    '{}' | Set-Content -LiteralPath $resource
    Assert-Fails { & $packageScript -BundlePath $bundle -Destination $destination } "metadata is not the binary"
    New-Item -ItemType Directory -Path (Split-Path $binary) -Force | Out-Null
    [System.IO.File]::WriteAllBytes($binary, [byte[]]@())
    Assert-Fails { & $packageScript -BundlePath $bundle -Destination $destination } "empty binary"
    [System.IO.File]::WriteAllBytes($binary, [byte[]]@(77, 90, 1, 2, 3, 255))
    & $packageScript -BundlePath $bundle -Destination $destination
    $entries = @(Get-ChildItem -LiteralPath $destination -Force)
    if ($entries.Count -ne 2) { throw "Unexpected package root" }
    $line = (Get-Content -LiteralPath (Join-Path $destination "SHA256SUMS.txt") -Raw).Trim()
    $hash, $relativePath = $line -split '  ', 2
    if ($relativePath -cne "G3X Test Plugin.vst3/Contents/x86_64-win/G3X Test Plugin.vst3") {
        throw "Checksum must reference the binary relative to the ZIP root"
    }
    $actual = (Get-FileHash -LiteralPath (Join-Path $destination $relativePath) -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -cne $actual) { throw "Checksum mismatch" }
    if (-not (Test-Path -LiteralPath (Join-Path $destination "G3X Test Plugin.vst3/Contents/Resources/moduleinfo.json"))) {
        throw "Bundle resources were lost"
    }
    'unexpected' | Set-Content -LiteralPath (Join-Path $destination "extra.txt")
    Assert-Fails { & $packageScript -BundlePath $bundle -Destination $destination } "stale or extra output"
    Write-Host "Packaging tests passed (spaces, resources, checksum, missing bundle/binary, empty binary, extra output)."
} finally {
    Remove-Item -LiteralPath $temp -Recurse -Force
}
