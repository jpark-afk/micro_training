# MICROSAR PIL/PSL Reusable Workflow

This package turns one-off success into repeatable execution with verification.

Distribution intent:

- `build_micro_vtt.md`, `playbooks`, and `.claude` are the portable delivery set.
- `build_micro4_vtt.bat` and the MICROSAR-specific target/CMake files are generated or updated outputs that the delivery set must be able to recreate in another repo copy.

## Goals

- Build PIL and PSL for VS2017 x86 + MICROSAR4
- Keep C-only build path
- Validate symbol-level readiness for AUTOSAR callbacks

## Output locations

- PIL archives: `lib/i86lePEvs2017`
- PSL archives: `lib/i86lePEvs2017-MICROSAR4`

## Files

- Workflow entry: [build_micro_vtt.md](build_micro_vtt.md)
- Prompt template: [playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md](playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md)
- Execution checklist: [playbooks/microsar-pil-psl/CHECKLIST.md](playbooks/microsar-pil-psl/CHECKLIST.md)
- Command pack: [playbooks/microsar-pil-psl/COMMANDS.md](playbooks/microsar-pil-psl/COMMANDS.md)
- Symbol verifier: [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

Generated or updated target-repo artifacts:

- `build_micro4_vtt.bat`
- `resource/cmake/architectures/i86lePEvs2017.tc`
- `resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc`
- `src/rti_me_psl/CMakeLists.txt`
- `CMakeLists.txt`

## Required environment

- OSEK_PATH must point to MICROSAR SIP root
- The delivery set must be able to recreate [build_micro4_vtt.bat](build_micro4_vtt.bat) or update it in place before build execution

## Suggested execution pattern

1. Explore with a subagent for context isolation.
2. Plan exact file and flag changes in the checked-in workflow files.
3. Create or update [build_micro4_vtt.bat](build_micro4_vtt.bat) and the required target files.
4. Run [build_micro4_vtt.bat](build_micro4_vtt.bat) for PIL, PSL, or both.
5. Run deterministic verifier script.
6. Stop only when checks pass.

## Quick start

1. Open [build_micro_vtt.md](build_micro_vtt.md)
2. Recreate or update [build_micro4_vtt.bat](build_micro4_vtt.bat) and required target files from the workflow instructions if they are missing or outdated
3. Run [build_micro4_vtt.bat](build_micro4_vtt.bat)
4. If needed, rerun [build_micro4_vtt.bat](build_micro4_vtt.bat) with `MODE=pil` or `MODE=psl`
5. Verify with:
   - powershell -ExecutionPolicy Bypass -File .\\playbooks\\microsar-pil-psl\\verify_psl_symbols.ps1
6. Spot-check archive folders:
   - Get-ChildItem .\\lib\\i86lePEvs2017
   - Get-ChildItem .\\lib\\i86lePEvs2017-MICROSAR4
