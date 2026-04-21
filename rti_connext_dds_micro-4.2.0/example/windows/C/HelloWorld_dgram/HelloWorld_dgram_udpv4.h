/*********************************************************************************************
Copyright (c) 2025-2025 Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.                                                                            
**********************************************************************************************/

#ifndef HelloWorld_dgram_udpv4_h
#define HelloWorld_dgram_udpv4_h

#include "rti_me_c.h"
#include "netio_dgram/netio_dgram.h"

/**
 * API to register the Example UDPv4 transport.
 *
 * registry
 * name

 * Return RTI_TRUE on success, RTI_FALSE on failure.
 */
extern RTI_BOOL
HelloWorld_dgram_udpv4_Interface_register(RT_Registry_T *registry,const char *name);

#endif
