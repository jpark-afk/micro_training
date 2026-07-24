---
name: micro4-n55-build
description: Build and verify RTI Micro4 PIL/PSL for NXP S32N55(R52) + FreeRTOS using GCC.
disable-model-invocation: true
---

Inputs
- OSEK-style env not required; NXP env vars required
- Targets:
  - PIL: s32n55r52leElfgcc10.2
  - PSL: s32n55r52leElfgcc10.2-FreeRTOS10.0

Steps
1. Read [build_micro4_n55.md](build_micro4_n55.md)
2. Build PIL target (C-only)
3. Build PSL target (C-only)
4. Run verifier:
   - powershell -ExecutionPolicy Bypass -File .\\playbooks\\micro4-n55\\verify_n55_outputs.ps1
5. Return evidence:
   - Commands run
   - Generated library paths
   - Verifier result
