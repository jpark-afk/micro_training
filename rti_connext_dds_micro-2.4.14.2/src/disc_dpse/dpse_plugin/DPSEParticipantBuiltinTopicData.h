/*
 * FILE: DPSEParticipantBuiltinTopicData.h - Exported topic data functions
 *
 * (c) Copyright, Real-Time Innovations, 2010-2015
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
 * 20feb2015,eh MICRO-813/PR#9172 Fix Lint warnings
 */
/*ci \addtogroup DPSEModule
 * @{
 */
#ifndef DPSEParticipantBuiltinTopicData_h
#define DPSEParticipantBuiltinTopicData_h

typedef RTI_BOOL
(*DDS_ParticipantBuiltinTopicData_TypePluginDeserializeParameterValueFunction)(
        void *parameters, struct CDR_Stream_t * stream,
        DDS_UnsignedShort parameterId, DDS_UnsignedShort parameterLength,
        void *param);

typedef void
(*DDS_ParticipantBuiltinTopicData_TypePluginSetDefaultParameterValuesFunction)(void *parameter);

MUST_CHECK_RETURN extern struct NDDS_Type_Plugin*
DPSE_ParticipantBuiltinTopicDataTypePlugin_get(void);

MUST_CHECK_RETURN extern RTI_BOOL
DPSE_ParticipantBuiltinTopicData_serialize_locator(
                                               struct CDR_Stream_t *stream,
                                               struct DDS_LocatorSeq *loc_seq,
                                               DDS_UnsignedShort param_id);
MUST_CHECK_RETURN extern RTI_BOOL
DPSE_ParticipantBuiltinTopicData_deserialize_locator(
                                            struct DDS_LocatorSeq *locator_seq,
                                            struct CDR_Stream_t *stream,
                                            DDS_UnsignedShort pid,
                                            void *param);

#endif

/*ci @} */
