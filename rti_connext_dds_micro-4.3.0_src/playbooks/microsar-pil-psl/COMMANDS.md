# Command Pack

## Environment
- set OSEK_PATH=<MICROSAR_SIP_ROOT>

## PIL build (C-only)
- .\\resource\\scripts\\rtime-make.bat --config Debug --build --target i86lePEvs2017 --name i86lePEvs2017 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

## PSL build (C-only)
- .\\resource\\scripts\\rtime-make.bat --config Debug --build --target i86lePEvs2017-MICROSAR4 --name i86lePEvs2017-MICROSAR4 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

## PSL clean rebuild
- .\\resource\\scripts\\rtime-make.bat --config Debug --build --delete --target i86lePEvs2017-MICROSAR4 --name i86lePEvs2017-MICROSAR4 -G "Visual Studio 15 2017" -DRTIME_EXCLUDE_CPP_eq_TRUE -DRTI_BUILD_UNITTESTS_eq_FALSE

## Verification
- powershell -ExecutionPolicy Bypass -File .\\playbooks\\microsar-pil-psl\\verify_psl_symbols.ps1

## Output folder checks
- Get-ChildItem .\\lib\\i86lePEvs2017
- Get-ChildItem .\\lib\\i86lePEvs2017-MICROSAR4

## Optional manual checks
- lib.exe /LIST build\\cmake\\Debug\\i86lePEvs2017-MICROSAR4\\Debug\\librti_me_netiopslzd.a
- dumpbin.exe /symbols build\\cmake\\Debug\\i86lePEvs2017-MICROSAR4\\rti_me_netiopslzd.dir\\Debug\\autosarSocket.obj | Select-String "NETIO_Autosar_TcpIp_udp_rx_indication|NETIO_Autosar_on_ip_assigned|NETIO_Autosar_on_socket_event"
