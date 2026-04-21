# Windows

## set env values
```cmd
cd rti_connext_dds_micro-4.2.0

setenv_micro.bat

cd ..
```

## Generate examples codes and build
```cmd
%RTIMEHOME%bin\rtiddsgen -example -language C -ppDisable HelloWorld.idl

%RTIMEHOME%resource\scripts\rtime-make.bat --config Debug -A x64 --target self --name %RTIMEARCH% --build --source-dir 
```
## Execution
```cmd
.\objs\x86_64lePEvs2017-Win10\Debug\HelloWorld_publisher.exe

.\objs\x86_64lePEvs2017-Win10\Debug\HelloWorld_subscriber.exe
```

## Cross test
```cmd
.\objs\x86_64lePEvs2017-Win10\Debug\HelloWorld_publisher.exe -udp_intf "Ethernet 2" -peer 192.168.56.10
```
# Linux

## installation
```bash
sudo apt update && sudo apt install build-essential
sudo apt install cmake
sudo apt install default-jre
```
## Set Envs
```bash
cd rti_connext_dds_micro-2.4.14.2
source set_micro_env.sh
cd ..
```

## Generate examples codes and build
```bash
$RTIMEHOME/bin/rtiddsgen -example -language C HelloWorld.idl

$RTIMEHOME/resource/scripts/rtime-make --config Debug --build --target $RTIMEARCH --source-dir . -G "Unix Makefiles" --delete
```
## Cross test
```bash
./objs/x86_64leElfgcc12.3.0-Linux5/HelloWorld_subscriber -udp_intf enp0s8 -peer 192.168.56.1
```
