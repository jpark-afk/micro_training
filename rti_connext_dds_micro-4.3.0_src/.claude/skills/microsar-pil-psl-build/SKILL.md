---
name: microsar-pil-psl-build
description: Build and verify RTI Micro PIL/PSL for VS2017 32-bit MICROSAR4 with symbol-level validation.
disable-model-invocation: true
---

Use this workflow when building PIL/PSL for MICROSAR AUTOSAR on Windows with VS2017 x86.

Inputs
- OSEK_PATH points to MICROSAR SIP root
- Target names:
  - PIL: i86lePEvs2017
  - PSL: i86lePEvs2017-MICROSAR4

Execution Steps
1. Explore
- Read [build_pil.md](build_pil.md) and [build_psl.md](build_psl.md)
- Read [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc)
- Read [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt)

2. Build PIL (C-only)
- Command:
  - .\\resource\\scripts\\rtime-make.bat --config Debug --build --target i86lePEvs2017 --name i86lePEvs2017 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

3. Build PSL (C-only)
- Command:
  - .\\resource\\scripts\\rtime-make.bat --config Debug --build --delete --target i86lePEvs2017-MICROSAR4 --name i86lePEvs2017-MICROSAR4 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

4. Verify output artifacts
- Check for:
  - build/cmake/Debug/i86lePEvs2017/Debug/librti_mezd.a
  - build/cmake/Debug/i86lePEvs2017-MICROSAR4/Debug/librti_me_netiopslzd.a

5. Verify symbol providers
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
