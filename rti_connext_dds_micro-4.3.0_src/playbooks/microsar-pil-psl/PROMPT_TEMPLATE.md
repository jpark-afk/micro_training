# Reusable Prompt Template: MICROSAR PIL/PSL Build

## Objective

Build PIL and PSL for MICROSAR4 using VS2017 x86 and validate AUTOSAR callback symbols without relying on `.cheat` reference files.

## Inputs

- RTI source root: <RTI_SOURCE_ROOT>
- OSEK_PATH: <OSEK_PATH>
- PIL target: i86lePEvs2017
- PSL target: i86lePEvs2017-MICROSAR4
- Config: Debug or Release

## Baseline Source of Truth

- build_micro_vtt.md
- build_micro4_vtt.bat
- CMakeLists.txt
- resource/cmake/architectures/i86lePEvs2017.tc
- resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc
- src/rti_me_psl/CMakeLists.txt

All required output artifacts must be maintained directly from these checked-in files.

## Constraints

- C only
- 32-bit
- No stub PSL fallback for MICROSAR callback path

## Required Output Artifacts

Prompt execution must create or update these files as final deliverables:

- build_micro4_vtt.bat
- resource/cmake/architectures/i86lePEvs2017.tc
- resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc
- src/rti_me_psl/CMakeLists.txt

For PSL-only runs, required output artifacts are:

- resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc
- src/rti_me_psl/CMakeLists.txt

For src/rti_me_psl/CMakeLists.txt changes:

- Create backup file first: src/rti_me_psl/CMakeLists.txt.bak
- Add explicit comments around new/changed blocks (AI-MOD-BEGIN / AI-MOD-END)

For build_micro4_vtt.bat changes or regeneration:

- Treat the checked-in wrapper behavior in build_micro_vtt.md as authoritative.
- Preserve internal environment setup instead of relying on setenv_micro_32bit.bat.
- Preserve argument parsing for positional, `NAME value`, and `NAME=value` forms.
- Preserve archive synchronization from build/cmake output into `lib/<target>`.
- Preserve verify integration with playbooks/microsar-pil-psl/verify_psl_symbols.ps1.

## Workflow Contract

1. Explore first:
   - Use subagent Explore to map PSL routing and toolchain flags.
   - Read the checked-in workflow files before planning edits.
2. Plan next:
   - List exact file changes and build commands.
3. Implement:
   - Apply required config changes.
   - Back up each edited file to a sibling `.bak` file before modifying it.
   - Write final content for required output artifacts.
   - If build_micro4_vtt.bat is missing, recreate it from the regeneration checklist in build_micro_vtt.md before attempting builds.
   - Build PIL then PSL, or use the requested mode through `build_micro4_vtt.bat`.
4. Verify:
   - Run symbol verification script.
   - Confirm archives under lib/i86lePEvs2017 and lib/i86lePEvs2017-MICROSAR4.
   - Report evidence: build output path + symbol matches.
5. Finish condition:
   - autosarSocket object in netiopsl archive
   - 3 required symbols present

## Required Symbols

- _NETIO_Autosar_TcpIp_udp_rx_indication
- _NETIO_Autosar_on_ip_assigned
- _NETIO_Autosar_on_socket_event

## Evidence Format

- Modified files
- Commands executed
- Artifacts generated
- Verification result (pass/fail)
