## Prompt

## Reusable Workflow Entry

This file is the unified entrypoint for Visual Studio 2017 x86 MICROSAR VTT builds.
For repeatable execution and verification, use:

- [build_pil.md](build_pil.md)
- [build_psl.md](build_psl.md)
- [playbooks/microsar-pil-psl/README.md](playbooks/microsar-pil-psl/README.md)
- [playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md](playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md)
- [playbooks/microsar-pil-psl/CHECKLIST.md](playbooks/microsar-pil-psl/CHECKLIST.md)
- [playbooks/microsar-pil-psl/COMMANDS.md](playbooks/microsar-pil-psl/COMMANDS.md)
- [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

Claude-style automation assets (optional):

- [/.claude/skills/microsar-pil-psl-build/SKILL.md](.claude/skills/microsar-pil-psl-build/SKILL.md)
- [/.claude/agents/microsar-psl-verifier.md](.claude/agents/microsar-psl-verifier.md)

### Goal
Create or update a unified batch entrypoint `build_micro4_vtt.bat` that can execute PIL-only, PSL-only, or combined PIL+PSL flows using the same verified logic captured by build_pil.md and build_psl.md.

Additionally, this prompt activity must:

- Generate `i86lePEvs2017.tc` and `i86lePEvs2017-MICROSAR4.tc` as new output files (existing files are not assumed).
- Update an existing root `CMakeLists.txt` as part of the prompt-driven changes.

### Baseline Source of Truth
The authoritative baseline input is:

- `C:\Users\jpark\Documents\rti_workspace\CMakeLists.txt`

Prompt execution must start from this baseline and derive final content changes for all output artifacts listed below.

### Unified Mode Matrix
The unified batch must support:

- `MODE=all`: build PIL then PSL
- `MODE=pil`: build PIL only
- `MODE=psl`: build PSL only

### Expected AI Output Artifacts
The following files must be produced as prompt-execution outputs, not treated as fixed inputs:

- `build_micro4_vtt.bat` (create or update)
- `i86lePEvs2017.tc` (generate as a new file from prompt/reference input)
- `i86lePEvs2017-MICROSAR4.tc` (generate as a new file from prompt/reference input)
- `CMakeLists.txt` (update existing file)

For `CMakeLists.txt` specifically:

- Keep existing content as baseline and apply minimal required changes.
- Create a backup file before modification: `CMakeLists.txt.bak`
- Mark newly added or changed blocks with explicit comments (for example, `AI-MOD-BEGIN` / `AI-MOD-END`)

### Expected Library Output Location
- PIL artifacts: `lib\i86lePEvs2017`
- PSL artifacts: `lib\i86lePEvs2017-MICROSAR4`

### Unified build_micro4_vtt.bat Contract
When generating `build_micro4_vtt.bat`, the script must implement:

- Accepted arguments:
  - `MODE`: `all | pil | psl` (default `all`)
  - `CONFIG`: `Debug | Release` (default `Debug`)
  - `VERIFY`: `verify | noverify` (default `verify`)
- Validate environment:
  - `OSEK_PATH` must be defined for MICROSAR include discovery
  - Visual Studio 2017 x86 toolchain must be available
- Build commands:
  - PIL target: `i86lePEvs2017`
  - PSL target: `i86lePEvs2017-MICROSAR4`
  - Generator: `Visual Studio 15 2017`
  - C-only flags:
    - `-DRTIME_EXCLUDE_CPP_eq_TRUE`
    - `-DRTI_BUILD_UNITTESTS_eq_FALSE`
- Build sequencing:
  - `MODE=all`: PIL then PSL
  - `MODE=pil`: PIL only
  - `MODE=psl`: PSL only
- Verification behavior:
  - `MODE=pil`: confirm PIL archives exist in `lib\i86lePEvs2017`
  - `MODE=psl`: run PSL verification and confirm PSL archives in `lib\i86lePEvs2017-MICROSAR4`
  - `MODE=all`: run both checks
  - if `VERIFY=noverify`, skip verification stage
- Exit code rules:
  - return non-zero on argument errors, environment errors, build failures, or verification failures

### Required Changes From build_pil.md
- Preserve MICROSAR-compatible PIL behavior for VS2017 x86.
- Keep PIL scope focused on PIL target/toolchain artifact generation.
- Route unresolved AUTOSAR callback symbol issues to PSL workflow.

### Required Changes From build_psl.md
- Ensure PSL routing does not fall back to stub path when target PSL is required.
- Ensure `RTIME_PIL_USE_TARGET_PSL` and `RTIME_TARGET_PSL` pathing are applied where needed.
- Ensure AUTOSAR transitive include discovery from `OSEK_PATH` is present for PSL compile chain.

### Recommended Build Commands (Reference)
```bat
rtime-make.bat --config Debug --build --target i86lePEvs2017 --name i86lePEvs2017 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

rtime-make.bat --config Debug --build --target i86lePEvs2017-MICROSAR4 --name i86lePEvs2017-MICROSAR4 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE
```

### Verification
- PIL archive directory check:
  - `Get-ChildItem .\lib\i86lePEvs2017`
- PSL archive directory check:
  - `Get-ChildItem .\lib\i86lePEvs2017-MICROSAR4`
- PSL symbol verification:
  - run [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

### Success Criteria
- Unified batch `build_micro4_vtt.bat` can run all/pil/psl modes with stable exit codes.
- PIL mode produces archives under `lib\i86lePEvs2017`.
- PSL mode produces archives under `lib\i86lePEvs2017-MICROSAR4`.
- PSL verification confirms callback symbol provider path.
- Output artifacts and modification evidence are reported.
