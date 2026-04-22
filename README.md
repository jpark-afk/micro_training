# Windows

## set env values
```cmd
cd rti_connext_dds_micro-4.3.0_ER1

setenv_micro.bat

cd ..
```

## Run MAG
```cmd
cd xml

%RTIMEHOME%\bin\rtiddsmag.bat -language C -referencedFile HelloWorldQos.xml HelloWorld.xml
```

## Generate Example codes and replace HelloWorld.xml
Please backup the original file!
```cmd
%RTIMEHOME%\bin\rtiddsgen.bat -example -exampleTemplate mag/dpde -language C HelloWorld.xml -replace
```

### Example code update
```c
//HelloWorld_publisher.c:98
/* TODO set sample attributes here */
snprintf(sample->message,64,"Hello World(%d)!\0",i);
sample->count = i;


//HelloWorld_subscriber.c:240
/* TODO read and process sample attributes here */
printf("%s\n", sample->message);
```

## Build (last option is very important!)
```cmd
%RTIMEHOME%\resource\scripts\rtime-make.bat --config Debug -A x64 --target self --name x86_64lePEvs2017-Win10 --build --source-dir . -DRTIME_MAG_FILES_eq_HelloWorld.xml
```
## Run
```cmd
.\objs\x86_64lePEvs2017-Win10\Debug\HelloWorld_publisher.exe    

.\objs\x86_64lePEvs2017-Win10\Debug\HelloWorld_subscriber.exe
```

# Linux

## installation
```bash
sudo apt update && sudo apt install build-essential
sudo apt install cmake
sudo apt install default-jre
```
## java setting
download the deb file from here: https://www.oracle.com/java/technologies/javase/jdk17-archive-downloads.html
```bash
sudo apt install ./jdk-17.0.6_linux-x64_bin.deb
sudo update-alternatives --install /usr/bin/java java /usr/lib/jvm/jdk-17/bin/java 100
sudo update-alternatives --install /usr/bin/javac javac /usr/lib/jvm/jdk-17/bin/javac 100
sudo update-alternatives --config java
## select the number of java-17
```

## Set Envs
```bash
cd rti_connext_dds_micro-4.2.0
source set_micro_env.sh
cd ..
```
### error handling (script from windows)
```bash
sed -i 's/\r$//' set_micro_env.sh
```
## Run MAG
```bash
cd xml
$RTIMEHOME/bin/rtiddsmag -language C -referencedFile HelloWorldQos.xml HelloWorld.xml
```

## Generate Example codes and replace HelloWorld.xml
Please backup the original file!
```bash
$RTIMEHOME/bin/rtiddsgen -example -exampleTemplate mag/dpde -language C HelloWorld.xml -replace
```
### Example code update
```c
//HelloWorld_publisher.c:98
/* TODO set sample attributes here */
snprintf(sample->message,64,"Hello World(%d)!\0",i);
sample->count = i;


//HelloWorld_subscriber.c:240
/* TODO read and process sample attributes here */
printf("%s\n", sample->message);
```

## Build (last option is very important!)
```bash
$RTIMEHOME/resource/scripts/rtime-make --config Debug --build --target x86_64leElfgcc12.3.0-Linux5 --source-dir . -G "Unix Makefiles" --delete -DRTIME_MAG_FILES=HelloWorld.xml
```
## Run
```bash
./objs/x86_64leElfgcc12.3.0-Linux5/HelloWorld_publisher    

./objs/x86_64leElfgcc12.3.0-Linux5/HelloWorld_subscriber
```

## Cross test
No need to set enabled_transport and initial_peers
```bash
## Windows
.\objs\x86_64lePEvs2017-Win10\Debug\HelloWorld_publisher.exe 

## Linux
./objs/x86_64leElfgcc12.3.0-Linux5/HelloWorld_subscriber 
```
