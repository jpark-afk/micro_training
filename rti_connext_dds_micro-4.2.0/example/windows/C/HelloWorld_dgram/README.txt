
C HelloWorld Datagram Interface Example
=====================

A publication and subscription pair that sends and receives using
an example UDPv4 Datagram interface

Discovery of endpoints is done with the static-endpoint discovery.

Purpose
=======

This example shows how to perform basic publish-subscribe communication.

This example performs endpoint discovery statically; the state of remote remote
endpoints are manually configured by the user, and only the configured endpoints
can be discovered.

Subscriber application creates a DataReader which uses a listener to receive
notifications about new samples and matched publishers. These notifications are
received in the middleware thread (instead of the application thread).

The Datagram Transport is a transport mechanism used for sending
and receiving data packets over a network using a user supplied interface.
This allows a simplified interface which can integrate with a variety of
different networking technologies.

How to Compile and Run
======================

--------------------
Compiling with CMake
--------------------
Before compiling, set environment variable RTIMEHOME to the Connext DDS Micro
installation directory.

The RTI Connext Micro source bundle includes a bash (Unix) and BAT (Windows)
script to simplify the invocation of CMake. Do not run these examples with the
--delete option. These scripts are a convenient way to invoke CMake with
the correct options. E.g:

Linux
-----
cd "<HelloWorldApplication directory>"
rtime-make --config <Debug|Release>  --build --name x64Linux4gcc7.3.0 --target Linux --source-dir . -G "Unix Makefiles"

Windows
-------
cd "<HelloWorldApplication directory>"
rtime-make.bat --config <Debug|Release> --build --name i86Win32VS2010 --target Windows --source-dir . -G "Visual Studio 10 2010"

Darwin
------
cd "<HelloWorldApplication directory>"
rtime-make --config <Debug|Release> --build --name x64Darwin17.3.0Clang9.0.0 --target Darwin --source-dir . -G "Unix Makefiles"

The executable can be found on directory "objs"

------------------------------------------------------
Running HelloWorld_publisher and HelloWorld_subscriber
------------------------------------------------------

By default the example uses the loopback interface.

E.g. in case the HelloWorld has been compiled for Linux i86Linux3gcc4.8.2 run the subscriber by typing:

objs/x64Linux3gcc4.8.2/HelloWorld_subscriber [-domain <Domain_ID>] [-sleep <sleep_time>] [-count <seconds_to_run>]

and run the publisher by typing:

objs/x64Linux3gcc4.8.2/HelloWorld_publisher [-domain <Domain_ID>] [-sleep <sleep_time>] [-count <seconds_to_run>]

Source Overview
===============

Type HelloWorld is provided and defined in file "HelloWorld.idl". In this
example, the type file must not be changed.

The type-support files are automatically by the supplied CMakeList.txt file.
rtime-make will generate these support files by invoking rtiddsgen.

The generated source files are:
HelloWorld.c
HelloWorldSupport.c
HelloWorldPlugin.c
HelloWorldAppgen.c

The associated header files are also generated. The DataWriter and DataReader
of the type are managed in HelloWorld_publisher.c and HelloWorld_subscriber.c,
respectively. The DomainParticipant of each is managed in
HelloWorldApplication.c.

Example Files Overview
======================

HelloWorld_dgram_udpv4.c:
This file contains this example Datagram Interface implementation for
unicast UDPv4.
The Datagram Interface is a component that is registered using
NETIO_DGRAM_InterfaceFactory_register().This interface must be compliant with
the NETIO_DGRAM_InterfaceI structure.

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

