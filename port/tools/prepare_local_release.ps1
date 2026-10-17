param([Parameter(Mandatory=$true)][ValidatePattern('^v\d+\.\d+(\.\d+)?$')][string]$Version)
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$projectRoot = Split-Path -Parent $repoRoot
$releaseZip = Join-Path $repoRoot "build/releases/ReKHCoM-$Version-windows-x86.zip"
$destination = [IO.Path]::GetFullPath((Join-Path $projectRoot "ReKHCoM-$Version"))
$application = Join-Path $destination "ReKHCoM-$Version-windows-x86"
if (-not $destination.StartsWith($projectRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid release destination' }
if (-not (Test-Path -LiteralPath (Join-Path $application 'ReKHCoM.exe'))) {
    if (-not (Test-Path -LiteralPath $releaseZip)) { throw "Missing package: $releaseZip" }
    $checksumPath = $releaseZip + '.sha256'
    $expected = ((Get-Content -LiteralPath $checksumPath -Raw).Trim() -split '\s+')[0]
    if ((Get-FileHash -LiteralPath $releaseZip -Algorithm SHA256).Hash -ne $expected) { throw 'Package checksum mismatch' }
    Expand-Archive -LiteralPath $releaseZip -DestinationPath $destination
}
$romDirectory = Join-Path $application 'roms'
[IO.Directory]::CreateDirectory($romDirectory) | Out-Null
$romHashes = @{
    B8CP = '8db73586cdb11b3795907edebf43228dbcd3e6b2'
    B8CE = '10729bd884f8fdca7a310b6d606c52e46657aa48'
    B8CJ = '59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa'
}
foreach ($code in $romHashes.Keys) {
    $source = Join-Path $repoRoot "roms/$code.gba"
    $target = Join-Path $romDirectory "$code.gba"
    if ((Test-Path -LiteralPath $source) -and -not (Test-Path -LiteralPath $target)) {
        if ((Get-FileHash -LiteralPath $source -Algorithm SHA1).Hash -eq $romHashes[$code]) {
            Copy-Item -LiteralPath $source -Destination $target
        }
    }
}
$previous = Get-ChildItem -LiteralPath $projectRoot -Directory | ForEach-Object {
    if ($_.Name -match '^ReKHCoM-(v\d+\.\d+(?:\.\d+)?)$') {
        $olderVersion = [version]$Matches[1].Substring(1)
        if ($olderVersion -lt [version]$Version.Substring(1)) {
            $path = Join-Path $_.FullName ($_.Name + '-windows-x86')
            if (Test-Path -LiteralPath $path) { [pscustomobject]@{ Version=$olderVersion; Path=$path } }
        }
    }
} | Sort-Object -Property Version -Descending
foreach ($previousRelease in $previous) {
    foreach ($file in Get-ChildItem -LiteralPath $previousRelease.Path -File) {
        if ($file.Name -notlike 'kh_com*.sav' -and $file.Name -ne 'kh_com.ini' -and $file.Name -ne 'kh_com_recom.ini') { continue }
        $target = Join-Path $application $file.Name
        if (-not (Test-Path -LiteralPath $target)) {
            Copy-Item -LiteralPath $file.FullName -Destination $target
        }
    }
}
Write-Output "Ready: $application"
