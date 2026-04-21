/* 

 (c) Copyright, Real-Time Innovations, 2013-2016.
 All rights reserved.

 No duplications, whole or partial, manual or electronic, may be made
 without express written permission.  Any such copies, or
 revisions thereof, must display this notice unaltered.
 This code contains trade secrets of Real-Time Innovations, Inc.

modification history
-------------------- 
10feb2016,tk  MICRO-1525 Added assignment operator to RT_ComponentFactoryId
22dec2015,as  Created
===================================================================== */


#ifndef dds_cpp_rt_hxx
#include "dds_cpp/dds_cpp_rt.hxx"
#endif


#ifndef dds_c_subscription_hxx
#include "dds_c/dds_c_subscription.h"
#endif

#ifndef dds_c_domain_hxx
#include "dds_c/dds_c_domain.h"
#endif
/*** SOURCE_BEGIN ***/

RTRegistry *RTRegistry::registry = NULL;

RTRegistry*
RTRegistry::get_instance()
{
    if (registry == NULL)
    {
        registry = new RTRegistry();
    }

    return registry;
}


void
RTRegistry::delete_instance()
{
#ifndef RTI_CERT
    if (registry != NULL)
    {
        delete registry;
        registry = NULL;
    }
#endif
}



RTRegistry::RTRegistry()
{
    DDS_DomainParticipantFactory* factory =
            DDS_DomainParticipantFactory_get_instance();
    this->c_registry = DDS_DomainParticipantFactory_get_registry(factory);
}

RTRegistry::~RTRegistry()
{

}

bool
RTRegistry::register_component(const char *name,
                         RT_ComponentFactoryI *intf,
                         RT_ComponentFactoryProperty *property,
                         RT_ComponentFactoryListener *listener)
{
    return (RT_Registry_register(
                this->c_registry, name, intf, property, listener) == RTI_TRUE);
}

bool
RTRegistry::unregister(const char *name,
                         RT_ComponentFactoryProperty **property,
                         RT_ComponentFactoryListener **listener)
{
    return (RT_Registry_unregister(
                this->c_registry, name, property, listener) == RTI_TRUE);
}

bool
RT_ComponentFactoryId::set_name(const char *const name)
{
    return (RT_ComponentFactoryId_set_name(this, name) == RTI_TRUE);
}

RT_ComponentFactoryId::RT_ComponentFactoryId()
{
    RT_ComponentFactoryId_clear(this);
}

RT_ComponentFactoryId&
RT_ComponentFactoryId::operator=(const char *const name)
{
    set_name(name);
    return *this;
}

RT_ComponentFactoryId&
RT_ComponentFactoryId::operator= (const RT_ComponentFactoryId&  from)
{
    set_name(RT_ComponentFactoryId_get_name(&from));
    return *this;
}

const RT_ComponentFactoryId&
RT_ComponentFactoryId::operator= (const RT_ComponentFactoryId&  /* from */) const
{
    return *this;
}

bool
RT_ComponentFactoryId::operator== (const RT_ComponentFactoryId& from) const
{
    return RT_ComponentFactoryId_equals(this,RT_ComponentFactoryId_get_name(&from)) == RTI_TRUE;
}

bool
RT_ComponentFactoryId::operator== (const char *const name) const
{
    return RT_ComponentFactoryId_equals(this,name) == RTI_TRUE;
}

bool
RT_ComponentFactoryId::operator!= (const RT_ComponentFactoryId& from ) const
{
    return !(*this == from);
}

bool
RT_ComponentFactoryId::operator!= (const char *const name) const
{
    return !(*this == name);
}


