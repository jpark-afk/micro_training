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
- Prompt template: [playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md](playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md)
- Execution checklist: [playbooks/microsar-pil-psl/CHECKLIST.md](playbooks/microsar-pil-psl/CHECKLIST.md)
- Command pack: [playbooks/microsar-pil-psl/COMMANDS.md](playbooks/microsar-pil-psl/COMMANDS.md)
- Symbol verifier: [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

## Required environment
- Windows + Visual Studio 2017 toolchain available in PATH
- OSEK_PATH must point to MICROSAR SIP root

## Suggested execution pattern
1. Explore with a subagent for context isolation.
2. Plan exact file/flag changes.
3. Build PIL, then PSL.
4. Run deterministic verifier script.
5. Stop only when checks pass.

## Quick start
1. Open [playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md](playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md)
2. Fill placeholders
3. Execute commands from [playbooks/microsar-pil-psl/COMMANDS.md](playbooks/microsar-pil-psl/COMMANDS.md)
4. Verify with:
   - powershell -ExecutionPolicy Bypass -File .\\playbooks\\microsar-pil-psl\\verify_psl_symbols.ps1
5. Spot-check archive folders:
   - Get-ChildItem .\\lib\\i86lePEvs2017
   - Get-ChildItem .\\lib\\i86lePEvs2017-MICROSAR4
