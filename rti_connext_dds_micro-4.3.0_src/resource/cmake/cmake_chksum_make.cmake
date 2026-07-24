###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Create an MD5s for an object-file and sort the resulting file
################################################################################

IF (RTIME_SORT_RESULT)
    IF (NOT EXISTS ${RTIME_OUTPUT_FILE})
        # This is expected since static and shared libraries are built
        # separately, thus only one may exist and it is not an error.
        # MESSAGE(STATUS "${RTIME_OUTPUT_FILE} does not exist, skipping")
        RETURN()
    ENDIF()
    # MESSAGE(STATUS "sort ${RTIME_OUTPUT_FILE}")
    FILE(STRINGS ${RTIME_OUTPUT_FILE} checksum_entries)
    FOREACH(f ${checksum_entries})
        STRING(REGEX MATCH "([0-9a-zA-Z]+)[ ]+(.*)" match ${f})
        IF (match)
            # MESSAGE(STATUS "entry: [${CMAKE_MATCH_1}] [${CMAKE_MATCH_2}]\n")
            LIST(APPEND sorted_chksum_entry ${CMAKE_MATCH_2})
            SET(${CMAKE_MATCH_2}_chksum ${CMAKE_MATCH_1})
        ELSE()
            MESSAGE(FATAL_ERROR "Failed to split [MD5SUM and filename]\n")
        ENDIF()
    ENDFOREACH()
    LIST(SORT sorted_chksum_entry)
    FILE(RENAME ${RTIME_OUTPUT_FILE} ${RTIME_OUTPUT_FILE}-backup)
    FILE(WRITE ${RTIME_OUTPUT_FILE})
    FOREACH(f ${sorted_chksum_entry})
        # MESSAGE(STATUS "sorted entry: [${${f}_chksum}] [${f}]\n")
        FILE(APPEND ${RTIME_OUTPUT_FILE} "${${f}_chksum}  ${f}\n")
    ENDFOREACH()
    FILE(REMOVE ${RTIME_OUTPUT_FILE}-backup)
    RETURN()
ENDIF()

# dirname - directory part of the RTIME_OBJFILE
# objfile - filename part of RTIME_OBJFILE
# objchksumfile = filename part of RTIME_OBJCHKSUMFILE 
# working_dir - Absolute path to objfile and objchksumfile

string(REGEX MATCH "(.*)/(.*)$" filename ${RTIME_OBJFILE})
SET(dirname ${CMAKE_MATCH_1})
SET(objfile ${CMAKE_MATCH_2})

string(REGEX MATCH "(.*)/(.*)$" filename ${RTIME_OBJCHKSUMFILE})
SET(objchksumfile ${CMAKE_MATCH_2})

SET(working_dir ${RTIME_WORKING_DIR}/${dirname})

if(WIN32)
    separate_arguments(opts WINDOWS_COMMAND ${RTIME_OBJDUMPCHKSUM_OPT} )
else()
    separate_arguments(opts UNIX_COMMAND ${RTIME_OBJDUMPCHKSUM_OPT} )
endif()

# Because the path to the objectfile may be embedded in the output it is
# necessary to set WORKING_DIRECTORY to the absolute path of the file to 
# process. After the object file has been created create an MD5 and append
# it to the output file.
#
# NOTE: It would have been easier if these steps could be done as part of the
#       custom command. However, the filename and directory generator 
#       expressions does not work, hence a separate file.

execute_process(
    COMMAND ${RTIME_OBJDUMPCHKSUM} ${opts} ${objfile} 
    OUTPUT_FILE ${objchksumfile}
    WORKING_DIRECTORY ${working_dir}
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E  md5sum  ${objchksumfile} 
    OUTPUT_VARIABLE md5sum
    WORKING_DIRECTORY ${working_dir}
)

file(APPEND ${RTIME_OUTPUT_FILE} ${md5sum})
