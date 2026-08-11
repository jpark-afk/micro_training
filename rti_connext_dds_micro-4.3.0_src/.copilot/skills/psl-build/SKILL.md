# PSL Build Skill Notes (Mobilgene Target)

## Scope
This document captures the proven build knowledge for PSL after the successful context beginning with:

- Vector SIP reference: `C:\Users\jpark\Documents\rti_workspace\CBD1500710_D12`
- Goal: complete PSL driver build and produce `lib/psl` with 3 static libraries.

## Success Criteria
The work is successful when all of the following are true:

1. PSL build completes for Debug and Release on both targets.
2. Artifacts are packaged under target-scoped lib folders.
3. For each target/config, these 3 C libraries exist:
   - `librti_me_rti_me_psl*.a`
   - `librti_me_ospsl*.a`
   - `librti_me_netiopsl*.a`

## Effective Strategy Used

### 1) Missing headers
When required headers do not exist:

- Create stubs under `${USAR_PATH}/stubs`.
- Consolidate missing compatibility headers in that single stub folder.
- Ensure `MICROSAR.cmake` includes `${USAR_PATH}/stubs` in include paths for Mobilgene builds.

This unblocks compile-time include failures without changing top-level project structure.

### 2) Missing symbols
When symbols are missing:

- Search VECTOR SIP/BSW first.
- If equivalent declarations/definitions exist, port the minimum required definitions into stubs.
- Keep stubs minimal and focused on compatibility for PSL build.

This resolves unresolved symbol/type expectations from AUTOSAR integration boundaries.

## Known-Good Build Context

- Target names:
   - `armv7emleElfghs2019.1.4_PDIO-Mobilgene`
   - `armv7emleElfghs2019.1.4_PDIOCERT-Mobilgene`
- Build wrapper: `build_psl_mobilgene.bat`
- Tooling: GHS compiler + Ninja generator
- Required environment inputs:
  - `RTIMEHOME`
  - `OSEK_PATH`
  - `GHS_COMPILER_PATH`
- Build mode behavior in wrapper:
   - Runs Debug/Release for `PDIO-Mobilgene`
   - Runs Debug/Release for `PDIOCERT-Mobilgene`
   - Packages output to `lib/<target>/`
   - Uses clean flow (`--delete`) to avoid stale config contamination

## Practical Rules That Prevent Regressions

1. Keep slash normalization for CMake-sensitive paths in batch scripts.
2. Use Ninja for this cross-build flow on Windows.
3. Keep stubs centralized under `${USAR_PATH}/stubs`.
4. Prefer importing definitions from VECTOR SIP/BSW over inventing new behavior.
5. Do not modify top-level `CMakeLists.txt` if avoidable; solve through toolchain/platform config and environment.

## Validation Commands Used

### Build
```powershell
.\build_psl_mobilgene.bat
```

### Symbol verification (archive + object)
```powershell
$gnm='C:\ghs\comp_201914\gnm.exe'; $archive='C:\RTI\rti_connext_drive-4.0.0\rti_connext_dds-7.3.1\rti_connext_dds_micro-4.3.0_ER738\build\cmake\Debug\armv7emleElfghs2019.1.4_PDIO-Mobilgene\librti_me_netiopslzd.a'; $obj='C:\RTI\rti_connext_drive-4.0.0\rti_connext_dds-7.3.1\rti_connext_dds_micro-4.3.0_ER738\build\cmake\Debug\armv7emleElfghs2019.1.4_PDIO-Mobilgene\CMakeFiles\rti_me_netiopslzd.dir\src\rti_me_psl\netiopsl\udp\autosar\autosarSocket.c.o'; '--- archive ---'; & $gnm $archive 2>&1 | Select-String -Pattern 'NETIO_Autosar_(TcpIp_udp_rx_indication|on_ip_assigned|on_socket_event)' -AllMatches | ForEach-Object { $_.Line }; '--- object ---'; & $gnm $obj 2>&1 | Select-String -Pattern 'NETIO_Autosar_(TcpIp_udp_rx_indication|on_ip_assigned|on_socket_event)' -AllMatches | ForEach-Object { $_.Line }
```

## Symbol Name Convention Note
Requested names with a leading underscore may not appear as-is in ELF outputs.
In this build, symbols were observed in exported form without the leading underscore:

- `NETIO_Autosar_TcpIp_udp_rx_indication`
- `NETIO_Autosar_on_ip_assigned`
- `NETIO_Autosar_on_socket_event`

## Artifact Location
Expected output folders:

- `lib/armv7emleElfghs2019.1.4_PDIO-Mobilgene`
- `lib/armv7emleElfghs2019.1.4_PDIOCERT-Mobilgene`

Expected naming by configuration:

- Debug: `*zd.a` (for example `librti_me_rti_me_pslzd.a`)
- Release: `*z.a` (for example `librti_me_rti_me_pslz.a`)

Packaged files per target:

- `librti_me_rti_me_psl<suffix>.a`
- `librti_me_ospsl<suffix>.a`
- `librti_me_netiopsl<suffix>.a`

## About `librti_me_netiopsl_cpp*`
`librti_me_netiopsl_cpp*` can appear because the generic build system enables C and CXX in toolchains and may generate optional C++ PSL artifacts.
For C-focused deliverables, this file is not required; packaging filters it out and keeps only the 3 C PSL libraries above.

## Short Troubleshooting Checklist

1. Build fails early on headers:
   - Check `${USAR_PATH}/stubs` exists and contains required stub headers.
   - Confirm stub include path is active in `MICROSAR.cmake`.
2. Build fails on unresolved symbol/type:
   - Search VECTOR SIP/BSW for matching declaration/definition.
   - Port minimal required definition into stub side.
3. Build behaves inconsistently:
   - Re-run with clean config (`--delete`) and verify env variables.
4. Symbol check mismatch with underscore prefix:
   - Verify both underscore and non-underscore forms in `gnm` output.
