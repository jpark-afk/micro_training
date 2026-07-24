## Reusable Prompt Template: MICROSAR PIL/PSL Build

### Objective
Build PIL and PSL for MICROSAR4 using VS2017 x86 and validate AUTOSAR callback symbols.

### Inputs
- RTI source root: <RTI_SOURCE_ROOT>
- OSEK_PATH: <OSEK_PATH>
- PIL target: i86lePEvs2017
- PSL target: i86lePEvs2017-MICROSAR4
- Config: Debug or Release

### Baseline Source of Truth
- C:\Users\jpark\Documents\rti_workspace\CMakeLists.txt

All required output artifacts must be derived from this baseline first, then adapted for MICROSAR4 PIL/PSL requirements.

### Constraints
- C only
- 32-bit
- No stub PSL fallback for MICROSAR callback path

### Required Output Artifacts
Prompt execution must create or update these files as final deliverables:
- resource/cmake/architectures/i86lePEvs2017.tc
- resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc
- src/rti_me_psl/CMakeLists.txt

For PSL-only runs, required output artifacts are:
- resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc
- src/rti_me_psl/CMakeLists.txt

For src/rti_me_psl/CMakeLists.txt changes:
- Create backup file first: src/rti_me_psl/CMakeLists.txt.bak
- Add explicit comments around new/changed blocks (AI-MOD-BEGIN / AI-MOD-END)

### Workflow Contract
1. Explore first:
   - Use subagent Explore to map PSL routing and toolchain flags.
   - Read and summarize the baseline source-of-truth file before planning edits.
2. Plan next:
   - List exact file changes and build commands.
3. Implement:
   - Apply required config changes.
   - Write final content for required output artifacts.
   - Build PIL then PSL.
4. Verify:
   - Run symbol verification script.
   - Confirm archives under lib/i86lePEvs2017 and lib/i86lePEvs2017-MICROSAR4.
   - Report evidence: build output path + symbol matches.
5. Finish condition:
   - autosarSocket object in netiopsl archive
   - 3 required symbols present

### Required Symbols
- _NETIO_Autosar_TcpIp_udp_rx_indication
- _NETIO_Autosar_on_ip_assigned
- _NETIO_Autosar_on_socket_event

### Evidence Format
- Modified files
- Commands executed
- Artifacts generated
- Verification result (pass/fail)
