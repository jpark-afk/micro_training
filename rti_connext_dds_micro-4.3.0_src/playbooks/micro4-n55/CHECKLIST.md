# Execution Checklist

## Explore
- [ ] Read [build_micro4_n55.md](build_micro4_n55.md)
- [ ] Confirm target tc files exist

## Preconditions
- [ ] arm-none-eabi-gcc available
- [ ] CONFIG_PATH has FreeRTOSConfig.h and lwipopts.h
- [ ] FREERTOS_PATH, LWIP_PATH, LWIP_PORTS_PATH are valid

## Build
- [ ] Build PIL: s32n55r52leElfgcc10.2
- [ ] Build PSL: s32n55r52leElfgcc10.2-FreeRTOS10.0

## Verify
- [ ] lib/s32n55r52leElfgcc10.2 exists
- [ ] lib/s32n55r52leElfgcc10.2-FreeRTOS10.0 exists
- [ ] librti_me*.a exists in each
- [ ] arm-none-eabi-ar -t succeeds for both
