param(
    [string]$BuildRoot = "build/cmake/Debug/i86lePEvs2017-MICROSAR4",
    [string]$Config = "Debug",
    [string]$LibraryName = "librti_me_netiopslzd.a"
)

$ErrorActionPreference = "Stop"

function Fail($msg) {
    Write-Error $msg
    exit 1
}

$libPath = Join-Path $BuildRoot (Join-Path $Config $LibraryName)
if (-not (Test-Path $libPath)) {
    Fail "Missing PSL archive: $libPath"
}

Write-Host "[check] archive exists: $libPath"

$libList = & lib.exe /LIST $libPath 2>&1
if ($LASTEXITCODE -ne 0) {
    Fail "lib.exe /LIST failed for $libPath`n$libList"
}

if (-not ($libList -match "autosarSocket\.obj")) {
    Fail "autosarSocket.obj not found in archive $libPath"
}

Write-Host "[check] archive contains autosarSocket.obj"

$objPath = Get-ChildItem -Path $BuildRoot -Recurse -Filter autosarSocket.obj -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
if (-not $objPath) {
    Fail "autosarSocket.obj not found under build root: $BuildRoot"
}

Write-Host "[check] object located: $objPath"

$symbols = & dumpbin.exe /symbols $objPath 2>&1
if ($LASTEXITCODE -ne 0) {
    Fail "dumpbin /symbols failed for $objPath`n$symbols"
}

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
