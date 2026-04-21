
C HelloWorld Dynamic Discovery Example
=====================

An example publication and subscription pair to send and receive the type
provide by user.

Discovery of endpoints is done with the dynamic-endpoint discovery.

Purpose
=======

This example shows how to perform basic publish-subscribe communication.

This example performs endpoint discovery dynamically: state of remote endpoints
are propagated automatically by built-in discovery endpoints, and the user does
not need to manually configure remote endpoint state.

Subscriber application creates a DataReader which uses a listener to receive
notifications about new samples and matched publishers. These notifications are
received in the middleware thread (instead of the application thread).

How to Compile and Run
======================

--------------------
Compiling with CMake
--------------------
Before compiling, set environment variable RTIMEHOME to the Connext DDS Micro
installation directory.

The RTI Connext Micro source bundle includes a bash (Unix) and BAT (Windows)
script to simplify the invocation of CMake. These scripts are a convenient way
to invoke CMake with the correct options. E.g:

Linux
-----
cd "<HelloWorldApplication directory>"
rtime-make --config <Debug|Release> --build --name x64Linux3gcc4.8.2 --target Linux --source-dir . -G "Unix Makefiles" --delete  [-DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE=true]

Windows
-------
cd "<HelloWorldApplication directory>"
rtime-make.bat --config <Debug|Release> --build --name i86Win32VS2010 --target Windows --source-dir . -G "Visual Studio 10 2010" --delete  [-DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE_eq_true]

Note: When building for Windows on any architecture other than x86, the user
must add the -A option for their architecture. For example, "-A x64".

Darwin
------
cd "<HelloWorldApplication directory>"
rtime-make --config <Debug|Release> --build --name x64Darwin17.3.0Clang9.0.0 --target Darwin --source-dir . -G "Unix Makefiles" --delete  [-DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE=true]

The executable can be found on directory "objs"

It is also possible to compile using CMake, e.g. in case the RTI Connext DDS
Micro source bundle is not installed.

Linux
-----
cmake [-DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE=true] [-DCMAKE_BUILD_TYPE=<Debug|Release>]  -G "Unix Makefiles" -B./<your build directory> -H. -DRTIME_TARGET_NAME=x64Linux3gcc4.8.2
cmake --build ./<your build directory> [--config <Debug|Release>]

Windows
-------
cmake [-DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE=true] [-DCMAKE_BUILD_TYPE=<Debug|Release>]  -G "Visual Studio 10 2010" -B./<your build directory> -H. -DRTIME_TARGET_NAME=i86Win32VS2010
cmake --build ./<your build directory> [--config <Debug|Release>]

Note: When building for Windows on any architecture other than x86, the user
must add the -A option for their architecture. For example, "-A x64".

Darwin
------
cmake [-DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE=true] [-DCMAKE_BUILD_TYPE=<Debug|Release>]  -G "Unix Makefiles" -B./<your build directory> -H. -DRTIME_TARGET_NAME=x64Darwin17.3.0Clang9.0.0
cmake --build ./<your build directory> [--config <Debug|Release>]

The executable can be found on ./objs

Option -DRTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE=true adds a rule to regenerate
type support plugin source files if the input IDL/XML file changes.
Default value is 'false'.

Option -DEXCLUDE_SHMEM=true excludes the Shared memory libraries from being
included in builds.

Option -DEXCLUDE_ZCV2=true excludes the ZCv2 libraries from being
included in builds.

------------------------------------------------------
Running HelloWorld_publisher and HelloWorld_subscriber
------------------------------------------------------

By default the example uses two interfaces to receive samples. The names of these
interfaces are configured in file HelloWorldApplication.c.
Some OS installations might use different names which would prevent communication.
For this reason it is recommended to use option -udp_intf <interface name>. Normally
you can use command 'ifconfig' (Linux) and 'ipconfig' (Windows) to know the name
of all available interfaces.

E.g. in case the HelloWorld has been compiled for Linux i86Linux3gcc4.8.2 run the subscriber by typing:

objs/x64Linux3gcc4.8.2/HelloWorld_subscriber [-domain <Domain_ID>] [-udp_intf <interface name>] [-peer <address>] [-sleep <sleep_time>] [-count <seconds_to_run>]

and run the publisher by typing:

objs/x64Linux3gcc4.8.2/HelloWorld_publisher [-domain <Domain_ID>] [-udp_intf <interface name>] [-peer <address>] [-sleep <sleep_time>] [-count <seconds_to_run>]

Source Overview
===============

Type HelloWorld is provided by user and defined in file
"HelloWorld.idl".

For convenience a CMakeList.txt is also generated so the example can be easily
compiled with CMake. A dependency can be added so the type-plugin interface
is regenerated if file HelloWorld.idl changes.

For the type to be usable by Connext DDS Micro, type-support files must be
generated that implement a type-plugin interface. The CMakeList.txt file
will generate these support files, by invoking rtiddsgen. Note that rtiddsgen
can be invoked manually, with an example command like this:

"$(RTIMEHOME)/rtiddsmag/scripts/rtiddsgen" -micro -language C HelloWorld.idl

The generated source files are HelloWorld.c,
HelloWorldSupport.c, and
HelloWorldPlugin.c. Associated header files are also
generated.

The DataWriter and DataReader of the type are managed in
HelloWorld_publisher.c and
HelloWorld_subscriber.c, respectively. The
DomainParticipant of each is managed in
HelloWorldApplication.c.

Example Files Overview
======================

HelloWorldApplication.c:
This file contains the logic for creating an application. This includes steps
for configuring discovery and creating a DomainParticipant. This file also
includes code for registering a type with the DomainParticipant.

HelloWorld_publisher.c:
This file contains the logic for creating a Publisher and a DataWriter, and
sending data.

HelloWorld_subscriber.c:
This file contains the logic for creating a Subscriber and a DataReader, a
DataReaderListener, and listening for data.

HelloWorldPlugin.c:
This file creates the plugin for the HelloWorld data type.  This
file contains the code for serializing and deserializing the HelloWorld
type, creating, copying, printing and deleting the HelloWorld type,
determining the size of the serialized type, and handling hashing a key, and
creating the plug-in.

HelloWorldSupport.c:
This file defines the HelloWorld type and its typed DataWriter,
DataReader, and Sequence.

HelloWorld.c:
This file contains the APIs for managing the HelloWorld type.

