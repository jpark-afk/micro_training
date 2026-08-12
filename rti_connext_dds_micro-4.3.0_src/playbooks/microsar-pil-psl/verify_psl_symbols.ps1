param(
    [string]$ArchiveRoot = "lib/i86lePEvs2017-MICROSAR4",
    [string]$ObjectRoot = "build/cmake/Debug/i86lePEvs2017-MICROSAR4",
    [string]$LibraryName = "librti_me_netiopslzd.a"
)

$ErrorActionPreference = "Stop"

function Fail($msg) {
    Write-Error $msg
    exit 1
}

$libPath = Join-Path $ArchiveRoot $LibraryName
if (-not (Test-Path $libPath)) {
    Fail "Missing PSL archive: $libPath"
}

Write-Host "[check] archive exists: $libPath"

$archiveText = [System.Text.Encoding]::ASCII.GetString([System.IO.File]::ReadAllBytes($libPath))

if (-not ($archiveText -match "autosarSocket\.obj")) {
    Fail "autosarSocket.obj not found in archive $libPath"
}

Write-Host "[check] archive contains autosarSocket.obj"

$objPath = Get-ChildItem -Path $ObjectRoot -Recurse -Filter autosarSocket.obj -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
if (-not $objPath) {
    Fail "autosarSocket.obj not found under object root: $ObjectRoot"
}

Write-Host "[check] object located: $objPath"

$symbols = [System.Text.Encoding]::ASCII.GetString([System.IO.File]::ReadAllBytes($objPath))

$required = @(
    "_NETIO_Autosar_TcpIp_udp_rx_indication",
    "_NETIO_Autosar_on_ip_assigned",
    "_NETIO_Autosar_on_socket_event"
)

foreach ($name in $required) {
    if (-not ($symbols -match [regex]::Escape($name))) {
        Fail "Missing required symbol: $name"
    }
    Write-Host "[check] symbol present: $name"
}

Write-Host "[ok] PSL symbol verification passed"
exit 0
