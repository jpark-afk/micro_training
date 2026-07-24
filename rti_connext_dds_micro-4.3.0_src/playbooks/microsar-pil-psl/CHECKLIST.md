# Execution Checklist

## Explore
- [ ] Read [build_pil.md](build_pil.md)
- [ ] Read [build_psl.md](build_psl.md)
- [ ] Read [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc)
- [ ] Read [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt)
- [ ] Read baseline source-of-truth file: C:\Users\jpark\Documents\rti_workspace\CMakeLists.txt
- [ ] Confirm output artifact changes are derived from baseline logic

## Output Artifacts
- [ ] Create or update [resource/cmake/architectures/i86lePEvs2017.tc](resource/cmake/architectures/i86lePEvs2017.tc)
- [ ] Create or update [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc)
- [ ] Create or update [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt)
- [ ] Create backup [src/rti_me_psl/CMakeLists.txt.bak](src/rti_me_psl/CMakeLists.txt.bak) before editing CMakeLists
- [ ] Mark added/changed CMakeLists blocks with AI-MOD-BEGIN / AI-MOD-END comments
- [ ] PSL-only run note: treat [resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc](resource/cmake/architectures/i86lePEvs2017-MICROSAR4.tc) and [src/rti_me_psl/CMakeLists.txt](src/rti_me_psl/CMakeLists.txt) as required outputs

## Preconditions
- [ ] OSEK_PATH is set
- [ ] VS2017 x86 build tools available
- [ ] RTI source tree is writable

## Build
- [ ] Build PIL target i86lePEvs2017
- [ ] Build PSL target i86lePEvs2017-MICROSAR4
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
