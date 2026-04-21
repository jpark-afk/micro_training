/*
 * FILE: rtps_checksum.h
 *
 * Copyright 2020-2021 Real-Time Innovations, Inc.
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
 * 15apr2021,tk MICRO-2979/PR.27549
 * - Removed unused functions from CERT:
 *   RTPS_ChecksumClass_is_valid
 * 20feb2021,tk MICRO-2833/PR#28651
 *   - Removed the unused attribute builtin_crc32_class in RTPS_ChecksumProperty
 * 19oct2020,tk MICRO-2575/PR#28172 Remove support for custom checksums.
 */
/*ci \file
 */
#ifndef rtps_checksum_h
#define rtps_checksum_h

/*ci \brief The checksum id, 0-3
 */
typedef RTI_INT16 RTPS_ChecksumId_T;

/* \dref_RTPS_ChecksumClassId_T
 */
typedef RTI_INT16 RTPS_ChecksumClassId_T;

/*ci \brief  Maximum number of built-in checksum functions
 */
#define RTPS_CHECKSUM_CLASS_MAX (3)

/*ci \brief 32 bit checksum wire representation as defined by OMG
 */
typedef RTI_UINT8 RTPS_Checksum32_t[4];

/*ci \brief 64 bit checksum wire representation as defined by OMG
 */
typedef RTI_UINT8 RTPS_Checksum64_t[8];

/*ci \brief 128 bit checksum wire representation as defined by OMG
 */
typedef RTI_UINT8 RTPS_Checksum128_t[16];

/*e \dref_RTPS_Checksum
 */
typedef union RTPS_Checksum
{
    /*e \dref_RTPS_Checksum_checksum32
     */
    RTI_UINT32 checksum32;

    /*e \dref_RTPS_Checksum_checksum64
     */
    RTI_UINT64 checksum64;

    /*e \dref_RTPS_Checksum_checksum128
     */
    RTI_UINT8 checksum128[16];

} RTPS_Checksum_T;

/*ci \brief Default checksum Class ID
 */
#define RTPS_CHECKSUM_CLASSID_NONE (0)

/*ci \brief Builtin 32 checksum Class ID
 */
#define RTPS_CHECKSUM_CLASSID_BUILTIN32  (1)

/*ci \brief Builtin 64 checksum Class ID
 */
#define RTPS_CHECKSUM_CLASSID_BUILTIN64  (2)

/*ci \brief Builtin 128 checksum Class ID
 */
#define RTPS_CHECKSUM_CLASSID_BUILTIN128 (3)

/*ci \brief Builtin 32 checksum Class ID for Pro
 */
#define RTPS_CHECKSUM_CLASSID_BUILTIN32PRO (4)

/*ci \brief The checksum index for participant announcements
 */
#define RTPS_CHECKSUM_PARTICIPANT_DISCOVERY RTPS_CHECKSUM_CLASSID_BUILTIN32

/*e \dref_RTPS_ChecksumCalculate_T
 */
typedef RTI_BOOL
(*RTPS_ChecksumCalculate_T)(void *context,
                            const struct REDA_Buffer *buf,
                            RTI_UINT32 buf_length,
                            RTPS_Checksum_T *checksum);

/*e \dref_RTPS_ChecksumClass
 */
typedef struct RTPS_ChecksumClass
{
    /*e \dref_RTPS_ChecksumClass_class_id
     */
    RTPS_ChecksumClassId_T class_id;

    /*e \dref_RTPS_ChecksumClass_context
     */
    void *context;

    /*e \dref_RTPS_ChecksumClass_checksum_calculate
     */
    RTPS_ChecksumCalculate_T checksum_calculate;
} RTPS_ChecksumClass_T;

/*ci \brief Initializer for a checksum function
 */
#define RTPS_ChecksumClass_INITIALIZER \
{ \
    RTPS_CHECKSUM_CLASSID_NONE,NULL,NULL\
}

/*e \dref_RTPS_ChecksumTxMode_T
 */
typedef enum
{
    /*e \dref_RTPS_ChecksumTxMode_T_RTPS_CHECKSUM_TXMODE_RTICRC32
     */
    RTPS_CHECKSUM_TXMODE_RTICRC32,

    /*e \dref_RTPS_ChecksumTxMode_T_RTPS_CHECKSUM_TXMODE_OMG
     */
    RTPS_CHECKSUM_TXMODE_OMG
} RTPS_ChecksumTxMode_T;

/*e \dref_RTPS_ChecksumProperty
 */
struct RTPS_ChecksumProperty
{
    /*e \dref_RTPS_ChecksumProperty_builtin_checksum32_class
     */
    RTPS_ChecksumClass_T builtin_checksum32_class;

    /*e \dref_RTPS_ChecksumProperty_builtin_checksum64_class
     */
    RTPS_ChecksumClass_T builtin_checksum64_class;

    /*e \dref_RTPS_ChecksumProperty_builtin_checksum128_class
     */
    RTPS_ChecksumClass_T builtin_checksum128_class;

    /*e \dref_RTPS_ChecksumProperty_checksum_tx_mode
     */
    RTPS_ChecksumTxMode_T checksum_tx_mode;

    /*e \dref_RTPS_ChecksumProperty_allow_builtin_override
     */
    RTI_BOOL allow_builtin_override;
};

/*ci brief Initializer for RTPS_ChecksumProperty
 */
#define RTPS_ChecksumProperty_INITIALIZER \
{ \
    RTPS_ChecksumClass_INITIALIZER, \
    RTPS_ChecksumClass_INITIALIZER, \
    RTPS_ChecksumClass_INITIALIZER, \
    RTPS_CHECKSUM_TXMODE_OMG, \
    RTI_FALSE \
}

#ifndef RTI_CERT
RTPSDllExport RTI_BOOL
RTPS_ChecksumClass_is_valid(const struct RTPS_ChecksumClass *function);
#endif

RTPSDllExport RTI_BOOL
RTPS_ChecksumClass_is_supported(const struct RTPS_ChecksumClass *function);

#endif /* rtps_crc_h */
