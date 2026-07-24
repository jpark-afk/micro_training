# Reusable Workflow Entry

This file is the self-contained entrypoint for Visual Studio 2017 x86 MICROSAR VTT builds.
The required generated artifacts are now part of the repo and can be maintained directly without relying on `.cheat`.

## New Context Quick Start

1. Confirm `OSEK_PATH` is already defined in the user environment.
2. From the repo root, run `build_micro4_vtt.bat all Debug verify`.
3. If the build needs investigation, read this file first, then inspect `build_micro4_vtt.bat`, `resource/cmake/architectures/i86lePEvs2017*.tc`, and `playbooks/microsar-pil-psl/verify_psl_symbols.ps1`.

Environment assumption for this workflow:

- `OSEK_PATH` is already defined as a user environment variable.
- `build_micro4_vtt.bat` prepares the repo-local build environment from the current directory before invoking the build.
- No separate `setenv_micro_32bit.bat` step is required for the supported workflow.

Primary workflow assets:

- [build_micro4_vtt.bat](build_micro4_vtt.bat)
- [rtimemake.bat](rtimemake.bat)
- [CMakeLists.txt](CMakeLists.txt)
- [resource/cmake/architectures/i86lePEvs2017.tc](resource/cmake/architectures/i86lePEvs2017.tc)
- [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc)
- [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt)
- [playbooks/microsar-pil-psl/README.md](playbooks/microsar-pil-psl/README.md)
- [playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md](playbooks/microsar-pil-psl/PROMPT_TEMPLATE.md)
- [playbooks/microsar-pil-psl/CHECKLIST.md](playbooks/microsar-pil-psl/CHECKLIST.md)
- [playbooks/microsar-pil-psl/COMMANDS.md](playbooks/microsar-pil-psl/COMMANDS.md)
- [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

Optional Claude automation assets:

- [/.claude/skills/microsar-pil-psl-build/SKILL.md](.claude/skills/microsar-pil-psl-build/SKILL.md)
- [/.claude/agents/microsar-psl-verifier.md](.claude/agents/microsar-psl-verifier.md)

## Goal

Use [build_micro4_vtt.bat](build_micro4_vtt.bat) to run PIL-only, PSL-only, or combined PIL+PSL flows with the checked-in CMake and architecture changes required for VS2017 x86 MICROSAR4 builds.

The workflow is considered reproducible only when a fresh shell can execute the wrapper directly from the repo root and complete build plus verification without any extra manual environment setup.

## Managed Artifacts

These files define the workflow and should be edited directly when the build logic changes:

- `build_micro4_vtt.bat`
- `rtimemake.bat`
- `resource/cmake/architectures/i86lePEvs2017.tc`
- `resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc`
- `src/rti_me_psl/CMakeLists.txt`
- `CMakeLists.txt`
- `playbooks/microsar-pil-psl/verify_psl_symbols.ps1`

Back up an existing file to a sibling `.bak` file before changing it.
Changed CMake blocks should keep explicit `AI-MOD-BEGIN` and `AI-MOD-END` comments.

## Mode Matrix

- `MODE=all`: build PIL then PSL
- `MODE=pil`: build PIL only
- `MODE=psl`: build PSL only

## Script Contract

[build_micro4_vtt.bat](build_micro4_vtt.bat) supports:

- `MODE=all|pil|psl` default `all`
- `CONFIG=Debug|Release` default `Debug`
- `VERIFY=verify|noverify` default `verify`

Validation requirements:

- `OSEK_PATH` must be defined
- `build_micro4_vtt.bat` must populate the repo-local build environment
- `resource\scripts\rtime-make.bat` must exist
- `rtimemake.bat` must resolve from the repo root after wrapper setup

Wrapper responsibilities:

- prepend repo root, `resource\scripts`, and `bin` to `PATH`
- set `RTIMEHOME`, `NDDSHOME`, `RTIME_DIST`, and target-specific `RTIMEARCH`
- call `rtimemake` with the correct target and C-only flags
- synchronize built archives from `build\cmake\<Config>\<Target>\<Config>` into `lib\<Target>`
- run PSL symbol verification without requiring `lib.exe` or `dumpbin.exe`

## Wrapper Regeneration Checklist

If `build_micro4_vtt.bat` is missing or must be rewritten in a new context, the regenerated script must preserve all of the following behavior:

- support `all`, `pil`, and `psl` modes
- support `Debug` and `Release` config selection
- support `verify` and `noverify` verification modes
- accept positional arguments and named forms split by `cmd`, including:
  - `build_micro4_vtt.bat pil Debug verify`
  - `build_micro4_vtt.bat MODE pil CONFIG Debug VERIFY verify`
  - `build_micro4_vtt.bat MODE=pil CONFIG=Debug VERIFY=verify`
- validate that `OSEK_PATH` is defined and exists
- set repo-local environment directly in the wrapper instead of requiring `setenv_micro_32bit.bat`
- ensure `rtimemake` resolves from the repo root by preparing `PATH`
- call `rtimemake` with:
  - generator `Visual Studio 15 2017`
  - `-DRTIME_EXCLUDE_CPP_eq_TRUE`
  - `-DRTI_BUILD_UNITTESTS_eq_FALSE`
- update `RTIMEARCH` per target, especially during `MODE=all`
- copy resulting `.a` or `.lib` files from `build\cmake\<Config>\<Target>\<Config>` to `lib\<Target>` after each target build
- fail with non-zero exit code on invalid args, environment errors, build failures, sync failures, or verification failures
- invoke [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1) for PSL verification

If a regenerated wrapper does not satisfy every item above, it is not equivalent to the validated workflow.

## Build Commands

```bat
build_micro4_vtt.bat
build_micro4_vtt.bat pil Debug verify
build_micro4_vtt.bat psl Debug verify
build_micro4_vtt.bat psl Release noverify
build_micro4_vtt.bat all Debug noverify
```

Recommended reproduction order:

```bat
build_micro4_vtt.bat pil Debug verify
build_micro4_vtt.bat psl Debug verify
build_micro4_vtt.bat all Debug verify
```

Internal low-level commands used by the wrapper:

```bat
rtimemake --config Debug --build --target i86lePEvs2017 --name i86lePEvs2017 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

rtimemake --config Debug --build --target i86lePEvs2017-MICROSAR4 --name i86lePEvs2017-MICROSAR4 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE
```

Accepted invocation styles:

```bat
build_micro4_vtt.bat pil Debug verify
build_micro4_vtt.bat MODE pil CONFIG Debug VERIFY verify
build_micro4_vtt.bat MODE=pil CONFIG=Debug VERIFY=verify
```

The positional form is the simplest and is the recommended form for repeatable use.

## Verification

- PIL archives: `lib\i86lePEvs2017`
- PSL archives: `lib\i86lePEvs2017-MICROSAR4`
- PSL symbol verification: [playbooks/microsar-pil-psl/verify_psl_symbols.ps1](playbooks/microsar-pil-psl/verify_psl_symbols.ps1)

Verification details:

- PIL verification counts synchronized `.a` or `.lib` archives in `lib\i86lePEvs2017`
- PSL verification counts synchronized `.a` or `.lib` archives in `lib\i86lePEvs2017-MICROSAR4`
- PSL symbol verification reads the built archive and `autosarSocket.obj` directly in PowerShell and checks these symbols:
  - `_NETIO_Autosar_TcpIp_udp_rx_indication`
  - `_NETIO_Autosar_on_ip_assigned`
  - `_NETIO_Autosar_on_socket_event`

Known failure modes and fixes captured during implementation:

- Do not rely on `setenv_micro_32bit.bat` for the primary workflow. The wrapper must be sufficient on its own.
- Do not rely on `lib.exe` or `dumpbin.exe` being present in `PATH`. The checked-in verifier avoids those tools.
- Do not assume PSL archives are copied to `lib\i86lePEvs2017-MICROSAR4` automatically by the build system. The wrapper performs explicit synchronization after each target build.
- Batch parsing must tolerate positional arguments and named tokens split by `cmd` parsing behavior.
- For `MODE=all`, `RTIMEARCH` must be updated per target rather than fixed once at startup.

Evidence from the validated workflow in this repo:

- `build_micro4_vtt.bat pil Debug noverify` completed successfully
- `build_micro4_vtt.bat MODE pil CONFIG Debug VERIFY noverify` completed successfully
- `build_micro4_vtt.bat psl Debug verify` completed successfully
- `build_micro4_vtt.bat all Debug verify` completed successfully
- PIL archives were synchronized into `lib\i86lePEvs2017`
- PSL archives were synchronized into `lib\i86lePEvs2017-MICROSAR4`

## Success Criteria

- The batch exits non-zero on invalid args, missing environment, build failure, or verification failure.
- PIL mode produces archives under `lib\i86lePEvs2017`.
- PSL mode produces archives under `lib\i86lePEvs2017-MICROSAR4`.
- PSL verification confirms the AUTOSAR callback symbol provider path.
