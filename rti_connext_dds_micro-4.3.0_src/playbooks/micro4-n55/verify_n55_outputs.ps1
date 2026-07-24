param(
    [ValidateSet('all', 'pil', 'psl')]
    [string]$Mode = 'all'
)

$ErrorActionPreference = 'Stop'

function Fail($msg) {
    Write-Error $msg
    exit 1
}

$requiredEnv = @(
    'FREERTOS_PATH',
    'LWIP_PATH',
    'LWIP_PORTS_PATH',
    'CONFIG_PATH',
    'RTD_PATH'
)

foreach ($name in $requiredEnv) {
    $value = [Environment]::GetEnvironmentVariable($name)
    if ([string]::IsNullOrWhiteSpace($value)) {
        Fail "Missing environment variable: $name"
    }
    if (-not (Test-Path $value)) {
        Fail "Invalid path for ${name}: $value"
    }
    Write-Host "[check] env ok: $name=$value"
}

$configPath = [Environment]::GetEnvironmentVariable('CONFIG_PATH')
if (-not (Test-Path (Join-Path $configPath 'FreeRTOSConfig.h'))) {
    Fail "Missing FreeRTOSConfig.h in CONFIG_PATH: $configPath"
}
if (-not (Test-Path (Join-Path $configPath 'lwipopts.h'))) {
    Fail "Missing lwipopts.h in CONFIG_PATH: $configPath"
}
Write-Host "[check] config headers present"

switch ($Mode) {
    'pil' {
        $targets = @('s32n55r52leElfgcc10.2')
    }
    'psl' {
        $targets = @('s32n55r52leElfgcc10.2-FreeRTOS10.0')
    }
    default {
        $targets = @(
            's32n55r52leElfgcc10.2',
            's32n55r52leElfgcc10.2-FreeRTOS10.0'
        )
    }
}

foreach ($target in $targets) {
    $libDir = Join-Path 'lib' $target
    if (-not (Test-Path $libDir)) {
        Fail "Missing library directory: $libDir"
    }

    $archives = Get-ChildItem -Path $libDir -Filter 'librti_me*.a' -File -ErrorAction SilentlyContinue
    if (-not $archives -or $archives.Count -eq 0) {
        Fail "No librti_me*.a found in $libDir"
    }

    foreach ($archive in $archives) {
        $list = & arm-none-eabi-ar -t $archive.FullName 2>&1
        if ($LASTEXITCODE -ne 0) {
            Fail "arm-none-eabi-ar -t failed: $($archive.FullName)`n$list"
        }
    }

    Write-Host "[check] archive listing ok: $libDir"
}

Write-Host "[ok] N55 output verification passed (mode=$Mode)"
exit 0
