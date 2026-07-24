# Command Pack

## One-shot (env check + build + verify)
build_micro4_n55.bat all Debug

## One-shot with verification disabled
build_micro4_n55.bat all Debug noverify

## PIL
resource\scripts\rtime-make.bat --config Debug --delete --build ^
  --target s32n55r52leElfgcc10.2 ^
  --name s32n55r52leElfgcc10.2 ^
  -G "Unix Makefiles" ^
  -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

## PSL
resource\scripts\rtime-make.bat --config Debug --delete --build ^
  --target s32n55r52leElfgcc10.2-FreeRTOS10.0 ^
  --name s32n55r52leElfgcc10.2-FreeRTOS10.0 ^
  -G "Unix Makefiles" ^
  -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

## Verify
powershell -ExecutionPolicy Bypass -File .\playbooks\micro4-n55\verify_n55_outputs.ps1
