# MICROSAR PIL/PSL Reusable Workflow

This package turns one-off success into repeatable execution with verification.

## Goals

- Build PIL and PSL for VS2017 x86 + MICROSAR4
- Keep C-only build path
- Validate symbol-level readiness for AUTOSAR callbacks

## Output locations

- PIL archives: `lib/i86lePEvs2017`
- PSL archives: `lib/i86lePEvs2017-MICROSAR4`

## Files

- Unified entry script: [build_micro4_vtt.bat](build_micro4_vtt.bat)
- Workflow entry: [build_micro_vtt.md](build_micro_vtt.md)
- Prompt template: [playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md](playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md)
- Execution checklist: [playbooks/microsar-pil-psl/CHECKLIST.md](playbooks/microsar-pil-psl/CHECKLIST.md)
- Command pack: [playbooks/microsar-pil-psl/COMMANDS.md](playbooks/microsar-pil-psl/COMMANDS.md)
- Symbol verifier: [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

## Required environment

- OSEK_PATH must point to MICROSAR SIP root
- Use [build_micro4_vtt.bat](build_micro4_vtt.bat) as the self-contained entrypoint for repo-local environment setup and build execution

## Suggested execution pattern

1. Explore with a subagent for context isolation.
2. Plan exact file and flag changes in the checked-in workflow files.
3. Run [build_micro4_vtt.bat](build_micro4_vtt.bat) for PIL, PSL, or both.
4. Run deterministic verifier script.
5. Stop only when checks pass.

## Quick start

1. Open [build_micro_vtt.md](build_micro_vtt.md)
2. Run [build_micro4_vtt.bat](build_micro4_vtt.bat)
3. If needed, rerun [build_micro4_vtt.bat](build_micro4_vtt.bat) with `MODE=pil` or `MODE=psl`
4. Verify with:
   - powershell -ExecutionPolicy Bypass -File .\\playbooks\\microsar-pil-psl\\verify_psl_symbols.ps1
5. Spot-check archive folders:
   - Get-ChildItem .\\lib\\i86lePEvs2017
   - Get-ChildItem .\\lib\\i86lePEvs2017-MICROSAR4
