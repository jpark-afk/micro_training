## Reusable Prompt Template: Micro4 N55 Build

### Objective
Build and validate RTI Micro4 PIL/PSL for NXP S32N55(R52) + FreeRTOS with GCC.

### Inputs
- RTI source root: <RTI_SOURCE_ROOT>
- NXP SDK/toolchain roots: <S32N5_* variables>
- Config: Debug or Release

### Constraints
- C only
- ABI must match R52 build flags
- Use Unix Makefiles for cross-compile

### Workflow Contract
1. Explore
- Read [build_micro4_n55.md](build_micro4_n55.md)
- Read toolchain files:
  - [resource/cmake/architectures/s32n55r52leElfgcc10.2.tc](resource/cmake/architectures/s32n55r52leElfgcc10.2.tc)
  - [resource/cmake/architectures/s32n55r52leElfgcc10.2-FreeRTOS10.0.tc](resource/cmake/architectures/s32n55r52leElfgcc10.2-FreeRTOS10.0.tc)

2. Build
- PIL target then PSL target

3. Verify
- Library directories exist
- librti_me*.a exists in both targets
- archive listing works via arm-none-eabi-ar -t

4. Evidence
- Commands executed
- Artifact paths
- Verification script output
