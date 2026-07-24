###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Create an MD5 for an object-file. 
################################################################################

# This rule requires a directory to compare against, otherwise the build 
# against self.

if ((NOT RTIME_COMPARE_WITH) OR (${RTIME_COMPARE_WITH} STREQUAL "self"))
    if (RTIME_DEBUG)
        message("selftest")
    endif()
    file(STRINGS ${RTIME_COMPARE}/checksum${RTIME_LIB_SUFFIX_STATIC} checksum_left)
else()
    file(STRINGS ${RTIME_COMPARE_WITH}/checksum${RTIME_LIB_SUFFIX_STATIC} checksum_left)
endif()

# The file with checksum are read and split into to lists, one with hashes
# and one with filename. List index N in the hash_value list will 
# correspond to the Nth name_value. Only the basename is needed as Micro uses
# unique filenames across all modules.

foreach(l ${checksum_left})
    string(REGEX MATCH "([0-9a-f]+)[ ]+(.*)" match_value ${l})
    if (match_value)
        list(APPEND hash_value ${CMAKE_MATCH_1})
        string(REGEX REPLACE "\\.(c|cxx|o|obj)\\.objchksum$" "" basename ${CMAKE_MATCH_2})
        list(APPEND name_value ${basename})
    endif()    
endforeach()

# The list of libraries to compare. This list is hard-coded since this file
# does not have access to all the build information as it is a standalone
# script. 
# 
# NOTE: The variables match the ones used as part of the build. However, these
#       are passed to this script from the main build-file.

if (RTIME_BUILD_SINGLE_LIB)
    set(library_list rti_me) 
else()
    set(library_list
        rti_me
        rti_me_rhsm 
        rti_me_whsm 
        rti_me_discdpde 
        rti_me_discdpse 
    )
endif()

if(WIN32) 
    separate_arguments(chksum_opt WINDOWS_COMMAND ${RTIME_OBJDUMPCHKSUM_OPT})
    separate_arguments(ar_opt WINDOWS_COMMAND ${RTIME_AR_EXRACT})
else()
    separate_arguments(chksum_opt UNIX_COMMAND ${RTIME_OBJDUMPCHKSUM_OPT})
    separate_arguments(ar_opt UNIX_COMMAND ${RTIME_AR_EXRACT})
endif()

# The process is as follows:
# - unarchive the library
# - For each file, do an object-dump and create an MD5
#   - Lookup the MD5 in hash_value
#     - if the hash_value does not exist, it is an error
#     - if the hash_value exists, make sure the filenames are the same
#
# NOTE: Later versions of cmake added continue(), but it is not used here since
#       as far as we know, the oldest version we have to support is 2.8.4.11

foreach(lib ${library_list})
    set(libname ${CMAKE_STATIC_LIBRARY_PREFIX}${lib}${RTIME_LIB_SUFFIX_STATIC}${CMAKE_STATIC_LIBRARY_SUFFIX})

    set(failed_count 0)
    set(file_count 0)
    set(passed_count 0)

    if (EXISTS ${RTIME_COMPARE}/${libname})
        if (EXISTS ${RTIME_COMPARE}/tmp)
            file(REMOVE_RECURSE ${RTIME_COMPARE}/tmp)
        endif()               
        file(MAKE_DIRECTORY ${RTIME_COMPARE}/tmp)
        execute_process(COMMAND ${RTIME_AR} ${ar_opt} ../${libname}
                        WORKING_DIRECTORY  ${RTIME_COMPARE}/tmp)
        file(GLOB filelist RELATIVE ${RTIME_COMPARE}/tmp ${RTIME_COMPARE}/tmp/*.${RTIME_OBJ_SUFFIX})
        foreach(f ${filelist})
            math(EXPR file_count "${file_count} + 1")
            string(REGEX REPLACE "\\.(c|cxx)\\.(o|obj)$" "" basename ${f})
            execute_process(
                COMMAND ${RTIME_OBJDUMPCHKSUM} ${chksum_opt} ${f}
                OUTPUT_FILE objfile
                WORKING_DIRECTORY  ${RTIME_COMPARE}/tmp
            )
            file(MD5  ${RTIME_COMPARE}/tmp/objfile md5sum)
            list(FIND hash_value ${md5sum} index)
            if (${index} GREATER -1)
                # Found the MD5, check if the filename is the same
                list(GET name_value ${index} md5_filename)
                if (NOT ${md5_filename} STREQUAL ${basename})
                    message("Checksum=${md5sum} FAILED: ${md5_filename} != ${basename}")
                    math(EXPR failed_count "${failed_count} + 1")
                else()
                    math(EXPR passed_count "${passed_count} + 1")
                endif()             
            else()
                math(EXPR failed_count "${failed_count} + 1")
                message("Checksum=${md5sum} FAILED: ${basename}")
            endif()
        endforeach()
    else()
        message("ERROR: ${RTIME_COMPARE}/${libname} does not exist, skipping")
    endif()
    if (${failed_count} GREATER 0)
        message("FAILED: ${lib}: Analyzed file-count=${file_count} Passed=${passed_count} Failed=${failed_count}")
    else()
        message("PASSED: ${lib}: Analyzed file-count=${file_count} Passed=${passed_count} Failed=${failed_count}")
    endif()
endforeach()
