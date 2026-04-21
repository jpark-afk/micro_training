/*
 * FILE: REDAIndexer.h - Indexer implementation
 *
 * Copyright 2012-2021 Real-Time Innovations, Inc.
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
 * 31jul2014,tk MICRO-172/PR#1064 - Removed superfluous REDA_Indexer fields
 * 25oct2012,tk Written
 *
 */
/*ce
 * \file
 */
/*ci \addtogroup REDAIndexerClass
 * @{
 */
#ifndef REDAIndexer_h
#define REDAIndexer_h

#ifndef reda_index_h
#include "reda/reda_indexer.h"
#endif

struct REDA_Indexer;

/*ci
 * \brief Concrete implementation of the index iterator
 */
struct REDA_IndexIterator
{
    /*ci
     * \brief The current index for the iterator
     */
    RTI_INT32 current_index;

    /*ci
     * \brief The indexer the iterator belongs to
     */
    struct REDA_Indexer *indexer;
};

/*ci
 * \brief Concrete implementation of the indexer
 */
struct REDA_Indexer
{
    /*ci
     * \brief Properties the indexer was created with
     */
    struct REDA_IndexerProperty property;

    /*ci
     * \brief Array for pointer to indexed elements
     */
    void **elements;

    /*ci
     * \brief Function to compare indexed elements to maintain order
     */
    REDA_Indexer_compare_T compare;

    /*ci
     * \brief The currently highest index in use
     */
    RTI_INT32 high_index;

    /*ci
     * \brief Each indexer contains one iterator
     */
    REDA_IndexIterator_T iterator;
};

#endif /* REDAIndexer_h */

/*ci
 * @}
 */
