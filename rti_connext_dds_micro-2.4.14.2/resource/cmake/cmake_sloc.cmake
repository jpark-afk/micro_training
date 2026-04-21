###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Determine the SLOC count for each library being built.
################################################################################
set(sloc_modules)
set(preproc_modules)
set(objdump_modules)
set(objchksum_modules)

foreach(m ${MODULE_LIST})
    if (DEFINED ${m}_EXPORT_NAME)
        set(${${m}_EXPORT_NAME}_PREPROCESSED_FILES)
        
        set(lib_name ${${m}_EXPORT_NAME})
        
        list(APPEND sloc_modules ${${m}_EXPORT_NAME}_sloc)
        list(APPEND preproc_modules ${${m}_EXPORT_NAME}_preproc)
        list(APPEND objdump_modules ${${m}_EXPORT_NAME}_objdump)
        list(APPEND objchksum_modules ${${m}_EXPORT_NAME}_objchksum)
                
        foreach(f ${${m}_SOURCE})
            if (NOT ${f} MATCHES ".*/IDL_*")        
                string(REGEX REPLACE "^${RTIME_SOURCE_ROOT}/(.*)\\.(c$|cxx$)" 
                                     "\\1" target_file ${f}
                )
         
                string(REGEX REPLACE "^${RTIME_SOURCE_ROOT}/.*\\.(c$|cxx$)" 
                                     "\\1" target_file_suffix ${f}
                )
                
                add_custom_command(OUTPUT ${target_file}.i
                                   COMMAND ${CMAKE_COMMAND} 
                                           --build ${RTIME_CMAKE_ROOT} 
                                           --config Debug -- ${target_file}.i
                                   DEPENDS ${f}
                )
        
                add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.objdump
                                   COMMAND ${RTIME_OBJDUMP} ${RTIME_OBJDUMP_OPT} ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX} > ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.objdump
                                   DEPENDS ${f}
                )

                IF (NOT RTI_NO_SHARED_LIB)
                    add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.objdump
                                       COMMAND ${RTIME_OBJDUMP} ${RTIME_OBJDUMP_OPT} ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX} > ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.objdump
                                       DEPENDS ${f}
                    )
                ENDIF()

                add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.chksum
                                   COMMAND ${CMAKE_COMMAND}
                                           -DRTIME_OBJDUMPCHKSUM=${RTIME_OBJDUMPCHKSUM} 
                                           -DRTIME_OBJDUMPCHKSUM_OPT="${RTIME_OBJDUMPCHKSUM_OPT}" 
                                           -DRTIME_OBJFILE=${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX}
                                           -DRTIME_OBJCHKSUMFILE=${target_file}.${target_file_suffix}.objchksum 
                                           -DRTIME_WORKING_DIR=${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir
                                           -DRTIME_OUTPUT_FILE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_STATIC} -P ${RESOURCEHOME}/cmake/cmake_chksum_make.cmake
                                   WORKING_DIRECTORY ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir                              
                                   DEPENDS ${f}
                )
                 
                IF (NOT RTI_NO_SHARED_LIB)
                    add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.chksum
                                       COMMAND ${CMAKE_COMMAND}
                                               -DRTIME_OBJDUMPCHKSUM=${RTIME_OBJDUMPCHKSUM} 
                                               -DRTIME_OBJDUMPCHKSUM_OPT="${RTIME_OBJDUMPCHKSUM_OPT}" 
                                               -DRTIME_OBJFILE=${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX}
                                               -DRTIME_OBJCHKSUMFILE=${target_file}.${target_file_suffix}.objchksum 
                                               -DRTIME_WORKING_DIR=${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir
                                               -DRTIME_OUTPUT_FILE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_SHARED} -P ${RESOURCEHOME}/cmake/cmake_chksum_make.cmake
                                       WORKING_DIRECTORY ${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir                              
                                       DEPENDS ${f}
                    )                
                ENDIF()
                            
                list(APPEND ${lib_name}_PREPROCESSED_FILES "${target_file}.i")
                list(APPEND ${lib_name}_OBJECTDUMP_FILES "${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.objdump")
                list(APPEND ${lib_name}_OBJECTCHKSUM_FILES "${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.chksum")
                IF (NOT RTI_NO_SHARED_LIB)
                    list(APPEND ${lib_name}_OBJECTCHKSUM_FILES_SHARED "${RTIME_CMAKE_ROOT}/CMakeFiles/${${m}_EXPORT_NAME}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.chksum")
                ENDIF()
            endif()        
        endforeach()
        
        foreach(m ${${m}_OBJECT_DEPENDS})
            foreach(f ${${m}_SOURCE})
                if (NOT ${f} MATCHES ".*/IDL_*")
                    string(REGEX REPLACE "^${RTIME_SOURCE_ROOT}/(.*)\\.(c$|cxx$)" 
                                         "\\1" target_file ${f}
                    )
        
                    string(REGEX REPLACE "^${RTIME_SOURCE_ROOT}/.*\\.(c$|cxx$)" 
                                         "\\1" target_file_suffix ${f}
                    )
                     
                    add_custom_command(OUTPUT ${target_file}.i
                                       COMMAND ${CMAKE_COMMAND} 
                                               --build ${RTIME_CMAKE_ROOT} 
                                               --config Debug -- ${target_file}.i
                                       DEPENDS ${f}
                    )                    
                    
                    add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.objdump
                                       COMMAND ${RTIME_OBJDUMP} ${RTIME_OBJDUMP_OPT} ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX} > ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.objdump
                                       DEPENDS ${f}
                    )

                    IF (NOT RTI_NO_SHARED_LIB)
                        add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.objdump
                                           COMMAND ${RTIME_OBJDUMP} ${RTIME_OBJDUMP_OPT} ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX} > ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.objdump
                                           DEPENDS ${f}
                        )
                    ENDIF()

                    add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.chksum
                                       COMMAND ${CMAKE_COMMAND} 
                                               -DRTIME_OBJDUMPCHKSUM=${RTIME_OBJDUMPCHKSUM} 
                                               -DRTIME_OBJDUMPCHKSUM_OPT="${RTIME_OBJDUMPCHKSUM_OPT}"
                                               -DRTIME_OBJFILE=${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX}
                                               -DRTIME_OBJCHKSUMFILE=${target_file}.${target_file_suffix}.objchksum 
                                               -DRTIME_WORKING_DIR=${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir
                                               -DRTIME_OUTPUT_FILE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_STATIC} -P ${RESOURCEHOME}/cmake/cmake_chksum_make.cmake
                                       WORKING_DIRECTORY ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir                                  
                                       DEPENDS ${f}
                    )
        
                    IF (NOT RTI_NO_SHARED_LIB)
                        add_custom_command(OUTPUT ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.chksum
                                           COMMAND ${CMAKE_COMMAND} 
                                                   -DRTIME_OBJDUMPCHKSUM=${RTIME_OBJDUMPCHKSUM} 
                                                   -DRTIME_OBJDUMPCHKSUM_OPT="${RTIME_OBJDUMPCHKSUM_OPT}"
                                                   -DRTIME_OBJFILE=${target_file}.${target_file_suffix}.${RTIME_OBJ_SUFFIX}
                                                   -DRTIME_OBJCHKSUMFILE=${target_file}.${target_file_suffix}.objchksum 
                                                   -DRTIME_WORKING_DIR=${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir
                                                   -DRTIME_OUTPUT_FILE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_SHARED} -P ${RESOURCEHOME}/cmake/cmake_chksum_make.cmake
                                           WORKING_DIRECTORY ${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir                                  
                                           DEPENDS ${f}
                        )                    
                    ENDIF()
                    
                    list(APPEND ${lib_name}_PREPROCESSED_FILES "${target_file}.i")
                    list(APPEND ${lib_name}_OBJECTDUMP_FILES "${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.objdump")
                    IF (NOT RTI_NO_SHARED_LIB)
                        list(APPEND ${lib_name}_OBJECTDUMP_FILES_SHARED "${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.objdump")
                    ENDIF()                    
                    list(APPEND ${lib_name}_OBJECTCHKSUM_FILES "${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_STATIC}.dir/${target_file}.${target_file_suffix}.chksum")
                    IF (NOT RTI_NO_SHARED_LIB)
                        list(APPEND ${lib_name}_OBJECTCHKSUM_FILES_SHARED "${RTIME_CMAKE_ROOT}/CMakeFiles/${lib_name}${RTI_LIB_SUFFIX_SHARED}.dir/${target_file}.${target_file_suffix}.chksum")
                    ENDIF()                    
                endif()
            
            endforeach()            
        endforeach()
              
        add_custom_target(${${m}_EXPORT_NAME}_sloc
                          COMMAND perl ${RESOURCEHOME}/perl/collect_sloc.pl 
                            --module ${lib_name} 
                            --arch ${RTIME_TARGET_NAME} 
                            --build_root ${RTIME_CMAKE_ROOT}/CMakeFiles
                            --objdir ${CMAKE_BUILD_TYPE}
                            --src_c \"${${lib_name}_PREPROCESSED_FILES}\" 
                            --lua_file ${RTIME_MODULE_EXPORT_ROOT}/benchmark/${RTIME_TARGET_NAME}/${lib_name}${RTI_LIB_SUFFIX_DYNAMIC}_sloc
                          DEPENDS  ${${${m}_EXPORT_NAME}_PREPROCESSED_FILES}
                          WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
        )
    
        add_custom_target(${${m}_EXPORT_NAME}_preproc
                          DEPENDS  ${${${m}_EXPORT_NAME}_PREPROCESSED_FILES}
                          WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
        )
    
        add_custom_target(${${m}_EXPORT_NAME}_objdump
                          DEPENDS  ${${${m}_EXPORT_NAME}_OBJECTDUMP_FILES} ${${${m}_EXPORT_NAME}_OBJECTDUMP_FILES_SHARED}
                          WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}                      
        )

        add_custom_target(${${m}_EXPORT_NAME}_objchksum
                          DEPENDS  ${${${m}_EXPORT_NAME}_OBJECTCHKSUM_FILES} ${${${m}_EXPORT_NAME}_OBJECTCHKSUM_FILES_SHARED}
                          WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
        )
    endif()
endforeach()

add_custom_target(clean_sloc
                  COMMAND ${CMAKE_COMMAND} -E echo HELLO
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${RTIME_MODULE_EXPORT_ROOT}/benchmark/${RTIME_TARGET_NAME}
                  WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
)

add_custom_target(sloc
                  DEPENDS clean_sloc ${sloc_modules}
                  WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
                  
)

add_custom_target(preproc
                  DEPENDS ${preproc_modules}
                  WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
)

add_custom_target(objdump
                  DEPENDS ${objdump_modules}
                  WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}                  
)

add_custom_target(objchksum_clean
                  COMMAND ${CMAKE_COMMAND} -E remove -f ${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_STATIC}
                  COMMAND ${CMAKE_COMMAND} -E remove -f ${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_SHARED}
)

add_custom_target(objchksum
                  COMMAND ${CMAKE_COMMAND}
                    -DRTIME_SORT_RESULT=true
                    -DRTIME_OUTPUT_FILE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_SHARED}
                    -P ${RESOURCEHOME}/cmake/cmake_chksum_make.cmake
                  COMMAND ${CMAKE_COMMAND}
                    -DRTIME_SORT_RESULT=true
                    -DRTIME_OUTPUT_FILE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME}/checksum${RTI_LIB_SUFFIX_STATIC}
                    -P ${RESOURCEHOME}/cmake/cmake_chksum_make.cmake
                  DEPENDS ${objchksum_modules}
                  WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}
)

add_custom_target(objchksumcmp
                  COMMAND ${CMAKE_COMMAND} 
                          -DRTIME_OBJDUMPCHKSUM=${RTIME_OBJDUMPCHKSUM} 
                          -DRTIME_OBJDUMPCHKSUM_OPT="${RTIME_OBJDUMPCHKSUM_OPT}" 
                          -DRTIME_COMPARE=${RTIME_MODULE_EXPORT_ROOT}/lib/${RTIME_TARGET_NAME} 
                          -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
                          -DRTIME_COMPARE_WITH=${RTIME_COMPARE_WITH}
                          -DCMAKE_STATIC_LIBRARY_PREFIX=${CMAKE_STATIC_LIBRARY_PREFIX}
                          -DRTIME_LIB_SUFFIX_STATIC=${RTI_LIB_SUFFIX_STATIC}
                          -DCMAKE_STATIC_LIBRARY_SUFFIX=${CMAKE_STATIC_LIBRARY_SUFFIX} 
                          -DRTIME_BUILD_SINGLE_LIB=${RTIME_BUILD_SINGLE_LIB}
                          -DRTIME_OBJ_SUFFIX=${RTIME_OBJ_SUFFIX}
                          -DRTIME_AR=${RTIME_AR}
                          -DRTIME_AR_EXRACT=${RTIME_AR_EXRACT}
                          -P ${RESOURCEHOME}/cmake/cmake_chksum_cmp.cmake
                  WORKING_DIRECTORY ${RTIME_SOURCE_ROOT}                  
)

add_dependencies(sloc clean_sloc)
