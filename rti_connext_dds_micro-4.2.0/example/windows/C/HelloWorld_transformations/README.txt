
=======================================
C HelloWorld w/ Transformations Example
=======================================

An example publication and subscription pair to send and receive simple strings.

Discovery of endpoints is done with the dynamic-endpoint discovery.

It is possible to register two different UDP transports or only one. In case
only one transport is registered both discovery and user-data will be sent/received
using this transport. In case 2 transports are registered, the first one will be
used for discovery and the second one for user-data. The first transport always
uses transformed/encrypted data and the second transport uses plain data.

The UDP transformation used in this example uses OpenSSL to encrypt and decrypt
data. For this reason it is needed to link this example with library OpenSSL.
This reference can be easily removed by changing the transformation implementation.

Purpose
=======

This example shows how to
1) perform basic communication using transformed discovery and transformed
user-data.
2) perform basic communication using transformed discovery and plain user-data.

For the examples below it is assume that A PC with two network interfaces
named "Ethernet" and "Wif-Fi" is used. IP address of network interface "Ethernet"
is 10.70.1.174. Of course it would be possible to use two PC's with one network
interface each.

Example 1: Transformed discovery and transformed user-data using the same
transformation. Run example with commands:

HelloSubscriber.exe -domain 58 -udp_intf1 Ethernet -peer 10.70.1.174
HelloPublisher.exe -domain 58 -udp_intf1 Ethernet -peer 10.70.1.174

"Ethernet" network interface is used for both discovery and user-data, and a 
transformation is registered for discovery.

Example 2: Transformed discovery and plain user-data. Run example with 
commands:

HelloSubscriber.exe -domain 58 -udp_intf1 Ethernet -udp_intf2 Wi-Fi -peer 10.70.1.174
HelloPublisher.exe -domain 58 -udp_intf1 Ethernet -udp_intf2 Wi-Fi -peer 10.70.1.174

"Ethernet" network interface is used for discovery and a transformation is done.
"Wi-Fi" network interface is used for user-data and no transformation is done.

Source Overview
===============

Before compiling, set environment variable RTIMEHOME to the Connext DDS Micro
installation directory.

A simple "HelloWorld" type, containing a message string, is defined in
HelloWorld.idl.

For the type to be useable by Connext Micro, type-support files must be
generated that implement a type-plugin interface.  The example Makefile
will generate these support files, by invoking rtiddsgen.  Note that rtiddsgen
can be invoked manually, with an example command like this:

    ${RTIMEHOME}/rtiddsgen/scripts/rtiddsgen -micro -language C HelloWorld.idl

The generated source files are HelloWorld.c, HelloWorldSupport.c, and
HelloWorldPlugin.c.  Associated header files are also generated.

The DataWriter and DataReader of the type are managed in HelloWorld_publisher.c
and HelloWorld_subscriber.c, respectively. The DomainParticipant of each is
managed in HelloWorldApplication.c.


Compiling
=========
Before compiling, set environment variable RTIMEHOME to the Connext DDS Micro
installation directory. The OPENSSLHOME environment variable can be set
or passed in as an argument. Otherwise the build will attempt to find
an installed version of OpenSSL.
This example was tested with openssl-1.1.1w.

The RTI Connext Micro source bundle includes a bash (Unix) and BAT (Windows)
script to simplify the invocation of CMake. These scripts are a convenient way
to invoke CMake with the correct options. E.g:

Linux
-----
cd "<HelloWorldApplication directory>"
rtime-make  --config <Debug|Release> --build --name armv8leElfgcc7.3.0-Linux4 --target Linux --source-dir . -G "Unix Makefiles" -DOPENSSLHOME=<openssl/path>

Windows
-------
cd "<HelloWorldApplication directory>"
rtime-make.bat --config <Debug|Release> --build --name i86Win32VS2010 --target Windows --source-dir . -G "Visual Studio 10 2010" -DOPENSSLHOME=<openssl/path>

Darwin
------
cd "<HelloWorldApplication directory>"
rtime-make --config <Debug|Release> --build --name x86_64leMachOclang15.0-Darwin23 --target Darwin --source-dir . -G "Unix Makefiles" -DOPENSSLHOME=<openssl/path>



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
This file creates the plugin for the HelloWorld data type. This file contains
the code for serializing and deserializing the HelloWorld type, creating,
copying, printing and deleting the HelloWorld type, determining the size of the
serialized type, and handling hashing a key, and creating the plug-in.

HelloWorldSupport.c
This file defines the HelloWorld type and its typed DataWriter, DataReader, and
Sequence.

HelloWorld.c
This file contains the APIs for managing the HelloWorld type.

HelloWorldUdpTransform.c
This file contains the UDP transformation implementation. A different
transformation can be implemented just by changing the implementation of functions
HelloWorldUdpTransform_transform_source() and
HelloWorldUdpTransform_transform_destination().

HelloWorldEncryption.c
This files contains the basic functions to perform encryption using OpenSSL.

Running HelloWorld_publisher and HelloWorld_subscriber
======================================================

E.g. in case the HelloWorld has been compiled for Windows VS2017 run the subscriber by typing:

.\objs\x86_64lePEvs2017-Win10\HelloSubscriber.exe -domain 58 -udp_intf1 <interface 1> -udp_intf2 <interface 2> -peer <peer 2>

and run the publisher by typing:

.\objs\x86_64lePEvs2017-Win10\HelloPublisher.exe -domain 58 -udp_intf1 <interface 1> -udp_intf2 <interface 2> -peer <peer 1>
