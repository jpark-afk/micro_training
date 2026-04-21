
=====================
C HelloWorld w/ Transformations Example
=====================

An example publication and subscription pair to send and receive simple strings.

Discovery of endpoints is done with the dynamic-endpoint discovery.

It is possible to register two different UDP transports or only one. In case
only one transport is registered both discovey and user-data will be sent/received
using this transport. In case 2 transports are registered, the first one will be 
used for discovery and the second one for user-data. The first transport always
uses tranformed/encrypted data and the second transport uses plain data.

The UDP transformation used in this example uses OpenSSL to encryp and decrypt
data. For this reason it is needed to link this example with library OpenSSL. 
This reference can be esily removed by changing the tranformation implementation.

NOTE: Please note that Connext Micro needs to be compiled explicitly to
      support UDP transformations. This is done by adding the
      option -DRTIME_UDP_ENABLE_TRANSFORMS=1 to rtime-make:

      rtime-make -DRTIME_UDP_ENABLE_TRANSFORMS=1 <other options>

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
"Wi-Fi" network interface is used for user-data and no tranformation is done.

Source Overview
===============

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
managed in HelloWorldApplication.c  


Example Files Overview
======================

HelloWorldApplication.c:
This file contains the logic for creating an application.  This includes steps 
for configuring discovery and creating a DomainParticipant.  This file also 
includes code for registering a type with the DomainParticipant.

HelloWorld_publisher.c:
This file contains the logic for creating a Publisher and a DataWriter, and 
sending data.  

HelloWorld_subscriber.c:
This file contains the logic for creating a Subscriber and a DataReader, a 
DataReaderListener, and listening for data.

HelloWorldPlugin.c:
This file creates the plugin for the HelloWorld data type.  This file contains 
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

E.g. in case the HelloWorld has been compiled for Windows VS2010 i86 run the subscriber by typing:

.\objs\i86Win32VS2010\HelloSubscriber.exe -domain 58 -udp_intf1 <interface 1> -udp_intf2 <interface 2> -peer <peer 2>

and run the publisher by typing:

.\objs\i86Win32VS2010\HelloPublisher.exe -domain 58 -udp_intf1 <interface 1> -udp_intf2 <interface 2> -peer <peer 1>
