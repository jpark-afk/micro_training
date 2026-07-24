---
name: microsar-pil-psl-build
description: Build and verify RTI Micro PIL/PSL for VS2017 32-bit MICROSAR4 with symbol-level validation.
disable-model-invocation: true
---

# microsar-pil-psl-build

Use this workflow when building PIL/PSL for MICROSAR AUTOSAR on Windows with VS2017 x86.

Portable delivery rule

- The distributable package is `build_micro_vtt.md`, `.claude`, and `playbooks`.
- `build_micro4_vtt.bat`, the MICROSAR `.tc` files, and required CMake edits must be reproducible from that package even if they are absent in the target repo copy.

Inputs

- OSEK_PATH points to MICROSAR SIP root
- build_micro4_vtt.bat prepares the remaining repo-local build environment internally
- if build_micro4_vtt.bat is missing, recreate it from build_micro_vtt.md before running builds
- Target names:
  - PIL: i86lePEvs2017
  - PSL: i86lePEvs2017-MICROSAR4

Execution Steps

1. Explore

- Read [build_micro_vtt.md](build_micro_vtt.md)
- Determine whether [build_micro4_vtt.bat](build_micro4_vtt.bat), [resource/cmake/architectures/i86lePEvs2017.tc](resource/cmake/architectures/i86lePEvs2017.tc), [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc), and [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt) exist or must be recreated.
- If any of them are absent or inconsistent, rebuild them to match the regeneration rules in build_micro_vtt.md before continuing.

1. Build PIL (C-only)

- Command:
  - .\\build_micro4_vtt.bat MODE=pil CONFIG=Debug VERIFY=verify

1. Build PSL (C-only)

- Command:
  - .\\build_micro4_vtt.bat MODE=psl CONFIG=Debug VERIFY=verify

1. Verify output artifacts

- Check for:
  - lib/i86lePEvs2017
  - lib/i86lePEvs2017-MICROSAR4

1. Verify symbol providers

- Run:
  - powershell -ExecutionPolicy Bypass -File .\\playbooks\\microsar-pil-psl\\verify_psl_symbols.ps1
- Must include in autosarSocket.obj:
  - _NETIO_Autosar_TcpIp_udp_rx_indication
  - _NETIO_Autosar_on_ip_assigned
  - _NETIO_Autosar_on_socket_event

Stop Conditions

- Build command returns non-zero
- Required archive missing
- Required symbol missing

If Failed

- Treat unresolved AUTOSAR symbols as configuration/routing issue first
- Re-check RTIME_PIL_USE_TARGET_PSL and RTIME_TARGET_PSL routing
- Re-check OSEK_PATH and BSW include availability (Det, Dem, NvM)
- Confirm `resource/cmake/architectures/i86lePEvs2017.tc` and `resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc` still match the checked-in workflow
