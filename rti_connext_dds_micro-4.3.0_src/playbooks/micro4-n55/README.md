# S32N55(R52) FreeRTOS Micro4 Reusable Workflow

Reusable workflow for GCC cross-build of RTI Micro4 PIL/PSL on NXP S32N55(R52).

## Scope
- PIL target: s32n55r52leElfgcc10.2
- PSL target: s32n55r52leElfgcc10.2-FreeRTOS10.0
- C-only builds
- Verify library outputs and archive members

## Files
- Prompt template: [playbooks/micro4-n55/PROMPT_TEMPLATE.md](playbooks/micro4-n55/PROMPT_TEMPLATE.md)
- Checklist: [playbooks/micro4-n55/CHECKLIST.md](playbooks/micro4-n55/CHECKLIST.md)
- Commands: [playbooks/micro4-n55/COMMANDS.md](playbooks/micro4-n55/COMMANDS.md)
- Verifier: [playbooks/micro4-n55/verify_n55_outputs.ps1](playbooks/micro4-n55/verify_n55_outputs.ps1)

## Preconditions
- arm-none-eabi-gcc toolchain in PATH
- Environment variables are configured:
  - S32N5_GCC_DIR, S32N5_FREE_RTOS_DIR, S32N5_TCPIP_DIR, S32N5_RTD_DIR
  - FREERTOS_PATH, LWIP_PATH, LWIP_PORTS_PATH, CONFIG_PATH, RTD_PATH

## One-pass flow
1. Set env variables
2. Build PIL
3. Build PSL
4. Run verifier script
5. Stop only when verifier passes

## Batch quick start
- Full flow: `build_micro4_n55.bat all Debug`
- PIL only: `build_micro4_n55.bat pil Debug`
- PSL only: `build_micro4_n55.bat psl Debug`
- Skip verify (if needed): `build_micro4_n55.bat all Debug noverify`
