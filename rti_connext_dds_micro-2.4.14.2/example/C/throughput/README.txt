=====================
C Throughput Example
=====================

An example throughput test that also measures CPU and memory usage

Purpose
=======

This example measures throughput by sending data to one or more subscriber applications. The test goes through the following phases:

1. The publisher signals the subscriber applications that it will commence, and then
starts its own clock. You may specify the duration of the test.

2. The subscriber starts measuring the number of samples received.

3. After the desired duration is over, the publisher signals the subscribers that one
experiment is over. The subscriber will then divide the number of samples
received by the elapsed time to report the throughput observed at the receiver.
When there are multiple subscribers, the throughput reported may be different.
The publisher also reports the throughput on its end.
Maximum throughput is achieved when the publisher sends as fast as the subscribers
can handle messages without dropping a packet. That is, the maximum throughput is
obtained somewhere between the publisher sending too slowly (not maximizing the
available pipe) and the publisher swamping the subscriber (overflowing the pipe).
For this reason, the test makes the publisher try a range of sending rates; the range is
provided as an input to the test. For the absolute maximum throughput to be observed,
the optimal sending rate must be in the range. An ever increasing reported throughput
is indicative that the tested range is too small. To observe the
maximum throughput, the range must be extended to include faster sending rates.
By default, the message size ranges from 16 bytes to 8K where the maximum size is indicated
in the IDL file.

For convenience, complete source code and a sample makefile for Linux, Solaris, and VxWorks has been provided.

This example uses static discovery.

How to Compile and Run
======================

Make sure the environment variable RTIMEHOME is defined and it points to
Connext Micro installation directory before compiling the example.
You can compile then compile the example:
  
    gmake RTIMEARCH=i86Linux2.6gcc3.4.3

If compiling for VxWorks, make sure the environment variable WIND_BASE is defined and that
RTIMEARCH is specified in the command line:
    
    gmake RTIMEHOME=ppc604Vx5.5gcc

You may have to change the following line in Throughput_publisher.c and Throughput_subscriber.c to use your network interface instead of "eth0":

discovery_plugin_properties.allowed_interface = DDS_String_dup("eth0");

A description of the command line arguments is printed when you run with -help.

Solaris Systems
---------------

Before compiling, make sure that the desired version of compiler and linker
is in your PATH environment variable.

Then compile by typing:

gmake RTIMEARCH=sparcSol2.8gcc3.2

Run the publisher by typing:

objs/sparcSol2.8gcc3.2/Throughput_publisher -domainId <Domain_ID> -subscribers 1 -duration 1 -size <Message_Size> -demand 1000:1000:9000 -reliable -peer <address_of_subscriber>

and run the subscriber by typing:

objs/sparcSol2.8gcc3.2/Throughput_subscriber -domainId <Domain_ID> -participantId <Participant_ID> -reliable -peer <address_of_publisher>

Linux Systems
-------------

Before compiling, make sure that the desired version of compiler and linker
is in your PATH environment variable.

Then compile by typing:

gmake RTIMEARCH=i86Linux2.6gcc3.4.3

Run the publisher by typing:

objs/i86Linux2.6gcc3.4.3/Throughput_publisher -domainId <Domain_ID> -subscribers 1 -duration 1 -size <Message_Size> -demand 1000:1000:9000 -reliable -peer <address_of_subscriber>

and run the subscriber by typing:

objs/i86Linux2.6gcc3.4.3/Throughput_subscriber -domainId <Domain_ID> -participantId <Participant_ID> -reliable -peer <address_of_publisher>

Replace i86Linux2.6gcc3.4.3 with the specific architecture you want to
compile.

VxWorks Systems
---------------

Before compiling, make sure that the desired version of compiler and linker
is in your PATH environment variable.

Change the arguments in publisher_main in Throughput_publisher.c and subscriber_main in Throughput_subscriber.c to your liking.

Then compile by typing:

gmake RTIMEARCH=ppc604Vx5.5gcc

On the VxWorks target, run the publisher by typing:

ld < <working directory>/objs/ppc604Vx5.5gcc/Throughput_publisher.so
taskSpawn "pub", 255, 0x8, 300000, publisher_main

and run the subscriber by typing:

ld < <working directory>/objs/ppc604Vx5.5gcc/Throughput_subscriber.so
taskSpawn "sub", 255, 0x8, 300000, subscriber_main

How to run a test with several subscribers
------------------------------------------

When running the publisher indicate the number of subscribers in the test. Publisher will wait until the specified number
of subscribers are found, e.g. use following command to run a test with 3 subscribers:

Throughput_publisher.exe -participantId 0 -subscribers 3

When running the subscribers use a different subscriber ID for each instance, starting with ID 0 until the number of
subscribers - 1, e.g. use the following commands to run a test with 3 subscribers

Throughput_subscriber.exe -subscriberId 0
Throughput_subscriber.exe -subscriberId 1
Throughput_subscriber.exe -subscriberId 2

Example Files
-------------

ThroughputArgs.h, ThroughputArgs.c:
Command-line argument parsing

TimeManager.h, TimeManager.c:
Test duration control

PerfMon.h, PerfMon.c:
On supported platforms (Linux and VxWorks), CPU and memory usage measurement

ThroughputQos.h, ThroughputQos.c:
QoS settings for entities that are used in the test

ThroughputCommon.h, ThroughputCommon.c:
Common utilities for both the publisher and subscriber

Throughput_publisher.c:
Publisher application

Throughput_subscriber.c:
Subscriber application

Throughput.h:
Defines the Throughput data type

ThroughputPlugin.c:
This file creates the plugin for the Throughput data type.  This file contains the code for serializing and deserializing the Throughput type, creating, copying, printing and deleting the Throughput type, determining the size of the serialized type, and handling hashing a key, and creating the plug-in.
