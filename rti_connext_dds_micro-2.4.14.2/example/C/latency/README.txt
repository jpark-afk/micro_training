=====================
C Latency Example
=====================

An example round-trip latency test

Purpose
=======

This example measures round-trip latency by sending a message and waiting for an echo message of the same size.

For convenience, complete source code and a sample makefile for Linux, Darwin and VxWorks has been provided.

This example uses static discovery.

How to Compile and Run
======================

Make sure the environment variable RTIMEHOME is defined before compiling the example. 
If not set, RTIMEHOME will be set to the default value "../../../..", assuming the example to be located within
an installation of RTI Connext Micro.

In order to compile the example, invoke gmake specifying which architecture should be built:
  
gmake RTIMEARCH=i86Linux2.6gcc4.4.5

The value specified for RTIMEARCH must match one of the architectures for which RTI Connext Micro libraries have 
been installed in the configured RTIMEHOME.

For ease of use, it is also possible to set RTIMEARCH in the shell's environment. E.g. on a bash terminal:

export RTIMEARCH=i86Linux2.6gcc4.4.5
gmake
 
If compiling for VxWorks, make sure the environment variable WIND_BASE is set before invoking gmake, and that it
points to a valid installation of the required VxWorks build environment.

Configuration of the example
============================

Configuration parameters used by this example may be changed by modifying 
the header file LatencyGlobalSettings.h
  
Configuration parameters are specified as macro definition, for example:

#define DEFAULT_UDP_ALLOWED_INTERFACE_EXT 	"en1"

The complete list of available parameters may be found in LatencyGlobalSettings.h.

Running the example
===================

Linux and Darwin Systems
-------------

Run the publisher by typing:

objs/$RTIMEARCH/LatencyPublisher -domainId <Domain_ID> -subscribers 1 -numIter 100000 -peer <address_of_subscriber>

and run the subscriber by typing:

objs/$RTIMEARCH/LatencySubscriber -domainId <Domain_ID> -cookie 1 -peer <address_of_publisher>

$RTIMEARCH refers to the value of an environment variable specifying the architecture used to build the example
(e.g. i86Linux2.6gcc4.4.5, i86Darwin10gcc4.2.1).

VxWorks Systems
---------------

Kernel Mode
-----------

If you want to provide custom invocation arguments you can configure the default main arguments to 
publisher_main_no_args (from LatencyPublisher.c) and subscriber_main_no_args 
(from LatencySubscriber.c) by setting them in the appropriate configuration header files, and then recompile
the example.

On the VxWorks target, run the publisher by typing:

ld < <working directory>/objs/$RTIMEARCH/LatencyPublisher.so
taskSpawn "pub", 255, 0x8, 300000, publisher_main_no_args

and run the subscriber by typing:

ld < <working directory>/objs/$RTIMEARCH/LatencySubscriber.so
taskSpawn "sub", 255, 0x8, 300000, subscriber_main_no_args

$RTIMEARCH refers to the value of an environment variable specifying the architecture used to build the example
(e.g. ppc604Vx6.9gcc4.3.3).

RTP Mode
--------

On the VxWorks target, run the publisher by typing:

cmd rtp exec -p 100 -u 150000 objs <working directory>/objs/$RTIMEARCH/LatencyPublisher -- -domainId <Domain_ID> -subscribers 1 -numIter 100000 -peer <address_of_subscriber>

and run the subscriber by typing:

cmd rtp exec -p 100 -u 150000 objs <working directory>/objs/$RTIMEARCH/LatencySubscriber -- -domainId <Domain_ID> -cookie 1 -peer <address_of_publisher>


Example Files
-------------
Communicator.h, Communicator.c:
Abstract RTI Connext Micro communication endpoints

DataProcessor.h, DataProcessor.c:
Measured data processing

LatencyPublisher.c:
Publisher application

LatencySubscriber.c:
Subscriber application

LatencyExample.c:
Support logic for measuring latency

Latency.h:
Defines the Latency data type

LatencyPlugin.c:
This file creates the plugin for the Latency data type.  
It contains the code for serializing and deserializing the Latency type, 
creating, copying, printing and deleting the Latency type, determining the size
 of the serialized type, and handling hashing a key, and creating the plug-in.

LatencySupport.c:
Defines a type-support class for the Latency data type which is
used to register the data type with a DomainParticipant.

LatencyGlobalSettings.h:
Test parameters