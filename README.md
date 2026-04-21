# Windows

## set env values
```cmd
cd rti_connext_dds_micro-2.4.14.2

setenv_micro.bat

cd ..
```

## Build libraries
```cmd
%RTIMEHOME%\resource\scripts\rtime-make --target self --name x64Win64VS2017 -G "Visual Studio 15 2017" --source-dir %RTIMEHOME% --build
```
## Generate examples codes and build
```cmd
%RTIMEHOME%\rtiddsgen\scripts\rtiddsgen -micro -ppDisable -language C -example .\HelloWorld.idl

%RTIMEHOME%\resource\scripts\rtime-make --target self --name x64Win64VS2017 -G "Visual Studio 15 2017" --source-dir . --build
```
## Execution
```cmd
.\objs\x64Win64VS2017\Debug\HelloWorld_publisher.exe

.\objs\x64Win64VS2017\Debug\HelloWorld_subscriber.exe
```

## Cross test
```cmd
.\objs\x64Win64VS2017\Debug\HelloWorld_publisher.exe -udp_intf "Ethernet 2" -peer 192.168.56.10
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
## Build libraries
```bash
$RTIMEHOME/resource/scripts/rtime-make --target self --name $RTIMEARCH -G "Unix Makefiles" --source-dir "$RTIMEHOME" --build
```
## Generate examples codes and build
```bash
$RTIMEHOME/rtiddsgen/scripts/rtiddsgen -micro -ppDisable -language C -example ./HelloWorld.idl

$RTIMEHOME/resource/scripts/rtime-make --target Linux --name $RTIMEARCH -G "Unix Makefiles" --source-dir . --build
```
## Cross test
```bash
./objs/x64Linux4gcc7.3.0/HelloWorld_subscriber -udp_intf enp0s8 -peer 192.168.56.1
```
