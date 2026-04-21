/*
 * FILE: netio_sample_access.h - NETIO Sample Accesor Interface
 *
 * Copyright 2023-2024 Real-Time Innovations, Inc.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef netio_sample_access_h
#define netio_sample_access_h
#include "osapi/osapi_config.h"
#include "osapi/osapi_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef RTI_BOOL
(*NETIO_Sample_retrieve_data_func)(
        const void *const history_user_data,
        void **retrieved_data);

typedef RTI_BOOL
(*NETIO_Sample_return_data_func)(
        const void *const user_data,
        const void *retrieved_data);

typedef RTI_BOOL
(*NETIO_Sample_on_add_data_func)(
        const void *user_data);


typedef RTI_BOOL
(*NETIO_Sample_on_remove_data_func)(
        const void *user_data);


typedef struct NETIO_SampleI
{
    NETIO_Sample_retrieve_data_func retrieve_sample;
    NETIO_Sample_return_data_func return_data;
    NETIO_Sample_on_add_data_func on_remove;
    NETIO_Sample_on_remove_data_func on_add;
}NETIO_SampleI;

/*ci
 *\brief NETIO_IntraInfo initializer constant
 */
#define NETIO_SampleI_INITIALIZER \
{ \
    NULL, /* retrieve_sample */ \
    NULL, /* return_data */ \
    NULL, /* on_remove */ \
    NULL /* on_add */ \
}

/*ci
 * \brief Wrapper to call \ref NETIO_InterfaceI::request
 *
 * \details
 * Request a sample from the sample accessor interface. If this function return false
 * it is considred a catastrophic failure. The caller should check for out_data to be NULL in case
 * the function return true. 
 *
 * \param[in]  self_         The sample accessor interface. 
 * \param[in]  config_data_  Data saved in the history calling this function. 
 * \param[out] out_data_      Data returned 
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_SampleI_retrieve(self_,config_data_,out_data_) \
    ((NETIO_SampleI*)\
            (self_))->retrieve_sample(config_data_,out_data_)


/*ci
 * \brief Wrapper to call \ref NETIO_InterfaceI::return
 *
 * \details
 * Returns a sample to the sample accessor interface. 
 *
 * \param[in]  self_         The sample accessor interface. 
 * \param[in]  config_data_  Data saved in the history calling this function. 
 * \param[out] in_data_      Data returned 
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_SampleI_return(self_,config_data_, in_data_) \
    ((NETIO_SampleI*)(self_))->return_data(config_data_,in_data_)

/*ci
 * \brief Wrapper to call \ref NETIO_InterfaceI::on_remove
 *
 * \details
 * called when a sample is removed
 *
 * \param[in]  self_  The sample accessor interface.
 * \param[in]  config_data_  The sample data when a sample is removed
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_SampleI_on_remove(self_,config_data_) \
    ((NETIO_SampleI*)(self_))->on_remove(config_data_)

/*ci
 * \brief Wrapper to call \ref NETIO_InterfaceI::on_add
 *
 * \details
 * called when a sample is added
 *
 * \param[in]  self_  The sample accessor interface.
 * \param[in]  config_data_  The sample data when a sample is added
 *
 * \return RTI_TRUE on success, RTI_FALSE on failure
 */
#define NETIO_SampleI_on_add(self_,config_data_) \
    ((NETIO_SampleI*)(self_))->on_add(config_data_)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
