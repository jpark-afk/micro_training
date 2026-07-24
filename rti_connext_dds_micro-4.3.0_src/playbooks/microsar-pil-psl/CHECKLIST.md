# Execution Checklist

## Explore

- [ ] Read [build_micro_vtt.md](build_micro_vtt.md)
- [ ] Read [build_micro4_vtt.bat](build_micro4_vtt.bat)
- [ ] Read [resource/cmake/architectures/i86lePEvs2017.tc](resource/cmake/architectures/i86lePEvs2017.tc)
- [ ] Read [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc)
- [ ] Read [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt)
- [ ] Read [CMakeLists.txt](CMakeLists.txt)
- [ ] Confirm output artifact changes are derived from checked-in workflow logic

## Output Artifacts

- [ ] Create or update [build_micro4_vtt.bat](build_micro4_vtt.bat)
- [ ] Create or update [resource/cmake/architectures/i86lePEvs2017.tc](resource/cmake/architectures/i86lePEvs2017.tc)
- [ ] Create or update [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc)
- [ ] Create or update [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt)
- [ ] Create backup [CMakeLists.txt.bak](CMakeLists.txt.bak) before editing [CMakeLists.txt](CMakeLists.txt)
- [ ] Create backup [src/rti_me_psl/CMakeLists.txt.bak](src/rti_me_psl/CMakeLists.txt.bak) before editing CMakeLists
- [ ] Mark added/changed CMakeLists blocks with AI-MOD-BEGIN / AI-MOD-END comments
- [ ] PSL-only run note: treat [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc) and [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt) as required outputs

## Preconditions

- [ ] OSEK_PATH is set
- [ ] [build_micro4_vtt.bat](build_micro4_vtt.bat) can initialize the repo-local build environment
- [ ] `rtimemake` is available inside the wrapper flow
- [ ] RTI source tree is writable

## Build

- [ ] Build PIL target i86lePEvs2017
- [ ] Build PSL target i86lePEvs2017-MICROSAR4
- [ ] Prefer [build_micro4_vtt.bat](build_micro4_vtt.bat) for repeatable execution
- [ ] Confirm archives exist in [lib/i86lePEvs2017](lib/i86lePEvs2017)
- [ ] Confirm archives exist in [lib/i86lePEvs2017-MICROSAR4](lib/i86lePEvs2017-MICROSAR4)

## Validate

- [ ] netiopsl archive exists
- [ ] autosarSocket.obj is inside archive
- [ ] Symbol _NETIO_Autosar_TcpIp_udp_rx_indication exists
- [ ] Symbol _NETIO_Autosar_on_ip_assigned exists
- [ ] Symbol _NETIO_Autosar_on_socket_event exists

## Closeout

- [ ] Save final command transcript
- [ ] Save modified file list
- [ ] Save artifact paths
