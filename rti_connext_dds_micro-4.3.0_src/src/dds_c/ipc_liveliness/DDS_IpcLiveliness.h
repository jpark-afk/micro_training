/*
 * FILE: DDS_IpcLiveliness.h - IPC Liveliness channel;
 *
 * Copyright (c) 2017-2025 Real-Time Innovations,Inc. All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 *
 */
/*ce
 */
#ifndef DDS_IpcLiveliness_h
#define DDS_IpcLiveliness_h

#ifndef dds_c_common_impl_h
#include "dds_c/dds_c_common_impl.h"
#endif

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif

#include "Entity.h"
#include "RemoteEntity.h"
#include "RemoteParticipant.h"
#include "DomainParticipant.h"
#include "DDS_ParticipantMessageData.h"
#include "DDS_ParticipantMessageDataSupport.h"
#include "DDS_ParticipantMessageDataPlugin.h"

#define DDS_PARTICIPANT_MESSAGE_DATA_TYPE_NAME "ParticipantMessageDataType"
#define PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE 0x01
#define PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE    0x02
#define NDDS_BUILTIN_IPC_TOPIC_NAME_MESSAGE_DATA  "DCPSParticipantMessage"
#define NDDS_BUILTIN_IPC_CHANNEL_MESSAGE_DATA 0x1

/*i \dref_RemoteWriterMatchEntry_t
 */
typedef struct RemoteWriterMatchEntry_t
{
    DDS_UnsignedLong local_reader_oid;
    DDS_BuiltinTopicKey_t remote_writer_key;
} RemoteWriterMatchEntry_t;

/*ci
 * \brief The DB table used for managing publications
 */
RTI_PRIVATE char *const DDS_REMOTE_MATCH_DW_TABLE_NAME = "dw_match";

struct DDS_IpcLiveliness
{
    DDS_DomainParticipant *participant;

    /*ci
     * \brief The timeout used to send DW liveliness assert
     */
    OSAPI_TimeoutHandle_T liveliness_event;

    /*ci
     * \brief Current duration for the timeout used to send DW liveliness
     *        assert. If the timeout is not create this field has the value
     *        DDS_DURATION_INFINITE.
     */
    struct DDS_Duration_t liveliness_timer_duration;

    /*ci
     * \brief True is any of the DW in the participant has asserted
     *        DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS liveliness
     */
    RTI_BOOL manual_by_participant_asserted;

    /*ci
     * \brief True is there is at least one DW enabled which has finite
     *        lease duration and automatic liveliness kind
     */
    RTI_BOOL automatic_finite_dw;

    /*ci
     * \brief Index with dw sorted by their lease duration
     */
    DB_Index_T min_lease_idx;

    /*ci
     * \brief Index with only automatic liveness datawriters sorted by their
     * lease duration
     */
    DB_Index_T automatic_idx;

    /*ci
     * \brief DDS Topic used by this channel's endpoints.
     */
    DDS_Topic *topic;

    /*ci
     * \brief DataWriter used to send data over this channel.
     */
    DDS_Liveliness_ParticipantMessageDataDataWriter *writer;

    /*ci
     * \brief DataReader used to receive data from this channel.
     */
    DDS_Liveliness_ParticipantMessageDataDataReader *reader;

    /*ci
     * \brief Sequence to read liveliness data
     */
    struct DDS_Liveliness_ParticipantMessageDataSeq data;

    /*ci
     * \brief Sequence to read liveliness info
     */
    struct DDS_SampleInfoSeq info;

    /*ci
     * \brief Sample used to send participant message samples, that is data(m)
     * messages with automatic and manual_by_participant liveliness kind in
     * the dw
     */
    struct DDS_Liveliness_ParticipantMessageData liveliness_sample;

    /*ci
     * \brief Table whith all matched remote dw. Each entry in the table
     * has a local DR which matches a remote DW. Only remote DW with
     * finite liveliness and liveliness kind automatic or manual_by_participant
     * are added in this table.
     */
    DB_Table_T liveliness_match_table;

    /*ci
     * \brief Qos for IPC reader
     */
    struct DDS_DataReaderQos dr_qos;

    /*ci
     * \brief Qos for IPC writer
     */
    struct DDS_DataWriterQos dw_qos;
};

extern DDS_Boolean
DDS_IpcLiveliness_enable(struct DDS_IpcLiveliness *ipc);

extern DDS_Boolean
DDS_IpcLiveliness_delete(struct DDS_IpcLiveliness *ipc);

extern DDS_Boolean
DDS_IpcLiveliness_reserve(DDS_DomainParticipant *participant);

extern void
DDS_IpcLiveliness_unreserve(DDS_DomainParticipant *participant);

extern DDS_Boolean
DDS_IpcLiveliness_assert(DDS_DomainParticipant *participant);

extern DDS_ReturnCode_t
DDS_IpcLiveliness_update_datawriter_liveliness_timeout(
                                    struct DDS_IpcLiveliness *ipc);

extern DDS_Boolean
DDS_IpcLiveliness_delete_liveliness_timeout(struct DDS_IpcLiveliness *ipc);
extern DDS_Boolean
DDS_IpcLiveliness_remove_routes(
        struct DDS_IpcLiveliness *ipc,
        struct DDS_RemoteParticipantImpl *remote_dp);

extern DDS_Boolean
DDS_IpcLiveliness_assert_routes(
    struct DDS_IpcLiveliness *ipc,
    struct DDS_RemoteParticipantImpl *remote_dp);

extern void
DDS_IpcLiveliness_assert_liveliness(struct DDS_IpcLiveliness *ipc);

extern void
DDS_IpcLiveliness_on_after_datawriter_deleted(struct DDS_IpcLiveliness *ipc,
                                              DDS_DataWriter *const writer);

extern DDS_Boolean
DDS_IpcLiveliness_remove_remote_writer(struct DDS_IpcLiveliness *ipc,
                                       DDS_UnsignedLong reader_oid,
                                       const DDS_BuiltinTopicKey_t *key);

extern DDS_Boolean
DDS_IpcLiveliness_add_remote_writer(struct DDS_IpcLiveliness *ipc,
                            DDS_UnsignedLong reader_oid,
                            const DDS_BuiltinTopicKey_t *key,
                            const struct DDS_LivelinessQosPolicy *liveliness);
#endif
