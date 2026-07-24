/*
 * FILE: RemoteEntity.h - RemoteEntity implementation
 *
 * (c) Copyright, Real-Time Innovations, 2008-2015.
 *
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 * Modification History
 * --------------------
 * 21jan2015,tk MICRO-999/PR#13241 Removed unused plugin field
 * 17may2012,tk Updated
 * 30apr2008,tk Created
 */
/*ce
 * \file
 * \brief RemoteEntity implementation
 */
/*ci \addtogroup DDSDomainModule
 * @{
 */
#ifndef RemoteEntity_h
#define RemoteEntity_h

/*ci
 * \brief Enable function for remote entities.
 *
 * \param[in] participant The participant the entity was discovered in
 * \param[in] self        The remote endpoint that is enabled
 * \param[in] new_key     The key for the remote endpoint, if applicable
 *
 * \return DDS_RETCODE_OK on success, one of the standard return codes on error
 */
FUNCTION_MUST_TYPEDEF(
DDS_ReturnCode_t
(*NDDS_RemoteEntityEnableFunction)(DDS_DomainParticipant *const participant,
                                   NDDS_RemoteEntity *self,
                                   const DDS_BuiltinTopicKey_t *new_key)
)

/*ci
 * \brief Implementation of a remote entity base-class
 */
struct NDDS_RemoteEntityImpl
{
    /*ci
     * \brief The enable function for this remote endpoint
     */
    NDDS_RemoteEntityEnableFunction enable_func;

    /*ci
     * \brief The state of the remote endpoint
     */
    RTIDDS_EntityState state;

    /*ci
     * \brief The kind of remote endpoint
     */
    DDS_EntityKind_t kind;
};

MUST_CHECK_RETURN extern DDS_Boolean
NDDS_RemoteEntity_is_enabled(struct NDDS_RemoteEntityImpl *entity);

#endif

/*ci @} */


