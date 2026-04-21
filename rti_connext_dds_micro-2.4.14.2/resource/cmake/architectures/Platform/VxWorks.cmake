###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2020
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.6)

INCLUDE(CMakeForceCompiler)
IF (${RTIME_TARGET_NAME} MATCHES "_face")
# Currently support FACE conformance with POSIX only
    SET(_RTIME_OSAPI_PLATFORM posix)
ELSEIF (${RTIME_TARGET_NAME} MATCHES "Vx653")
    SET(_RTIME_OSAPI_PLATFORM "arinc653:vxworks653")
    SET(RTIME_VXWORKS_DISABLE_MUNCHING TRUE)
ELSE()
    SET(_RTIME_OSAPI_PLATFORM vxworks)
ENDIF()

IF (NOT ${RTIME_TARGET_NAME} MATCHES "gcc")
    MESSAGE(FATAL_ERROR "The VxWorks toolchain-file used for ${RTIME_TARGET_NAME} only supports GCC.\n"
                        "To use diab or icc it is necessary to write a custom cmake toolchain file")
ENDIF()

# Setup munching configuration for C++ only if munching is enabled, available and not compiling for RTP
# NOTE: Use -DRTIME_VXWORKS_ENABLE_MUNCHING:BOOL=TRUE on cmake to enable munching
IF (RTIME_VXWORKS_ENABLE_MUNCHING)
    IF (EXISTS "$ENV{WIND_BASE}/host/resource/hutils/tcl/munch.tcl"
            AND NOT ${RTIME_TARGET_NAME} MATCHES "_rtp")
        set(MUNCH "$ENV{WIND_FOUNDATION_PATH}/$ENV{WIND_HOST_TYPE}/bin/wtxtcl.ex \
                $ENV{WIND_BASE}/host/resource/hutils/tcl/munch.tcl -c arm")
        set(MUNCH_FLAGS "-T $ENV{WIND_BASE}/target/h/tool/gnu/ldscripts/link.OUT")
        set(MUNCH_NM "$ENV{WIND_BASE}/host/binutils/$ENV{WIND_HOST_TYPE}/bin/nmarm")
    ELSE()
        MESSAGE(WARNING "Munching enabled, but munching $ENV{WIND_BASE}/host/resource/hutils/tcl/munch.tcl not found")
    ENDIF()
ENDIF()

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/platform.tc)

IF (${RTIME_TARGET_NAME} MATCHES "ppc")
    SET(_RTIME_CPU_SUFFIX ppc)
    SET(CMAKE_C_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ccppc)
    SET(CMAKE_CXX_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/c++ppc)
    SET(CMAKE_LINKER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ldppc)
    SET(CMAKE_C_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/arppc r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibppc <TARGET> ")
    SET(CMAKE_CXX_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/arppc r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibppc <TARGET> ")
ELSEIF (${RTIME_TARGET_NAME} MATCHES "arm")
    SET(_RTIME_CPU_SUFFIX arm)
    SET(CMAKE_C_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ccarm)
    SET(CMAKE_CXX_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/c++arm)
    SET(CMAKE_LINKER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ldarm)
    SET(CMAKE_C_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ararm r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibarm <TARGET> ")
    SET(CMAKE_CXX_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ararm r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibarm <TARGET> ")
ELSEIF (${RTIME_TARGET_NAME} MATCHES "mips")
    SET(_RTIME_CPU_SUFFIX mips)
    SET(CMAKE_C_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ccmips)
    SET(CMAKE_CXX_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/c++mips)
    SET(CMAKE_LINKER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ldmips)
    SET(CMAKE_C_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/armips r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibmips <TARGET> ")
    SET(CMAKE_CXX_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/armips r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibmips <TARGET> ")
ELSEIF (${RTIME_TARGET_NAME} MATCHES "pentium")
    SET(_RTIME_CPU_SUFFIX pentium)
    SET(CMAKE_C_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ccpentium)
    SET(CMAKE_CXX_COMPILER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/c++pentium)
    SET(CMAKE_LINKER $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ldpentium)
    SET(CMAKE_C_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/arpentium r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibpentium <TARGET> ")
    SET(CMAKE_CXX_CREATE_STATIC_LIBRARY
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/arpentium r <TARGET> <LINK_FLAGS> <OBJECTS> "
        "$ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ranlibpentium <TARGET> ")
ELSE()
    IF (NOT ${TARGET_HELP})
        MESSAGE(FATAL_ERROR "Did not find a known CPU architecture in ${RTIME_TARGET_NAME}, expecting one of arm,ppc,pentium, or mips")
    ELSE()
        SET(CC_SUFFIX "dummy")
    ENDIF()
ENDIF()

FOREACH(lang C CXX)
  SET(CMAKE_${lang}_FLAGS_INIT "")
  SET(CMAKE_${lang}_FLAGS_DEBUG_INIT "-g")
  SET(CMAKE_${lang}_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
  IF (RTI_ME_CMAKE_${lang}_FLAGS_RELEASE_INIT)
    SET(CMAKE_${lang}_FLAGS_RELEASE_INIT "${RTI_ME_CMAKE_${lang}_FLAGS_RELEASE_INIT} -DNDEBUG")
  ELSE()
    SET(CMAKE_${lang}_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
  ENDIF()
  SET(CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT "-O2 -g -DNDEBUG")
ENDFOREACH()

if (NOT VXWORKS_WRGNU)
    set(VXWORKS_WRGNU 1)

    IF (NOT DEFINED VX_PPC_ARCH)
        SET(VX_PPC_ARCH "PPC32")
    ENDIF()
    IF (NOT DEFINED VX_ARM_ARCH)
        SET(VX_ARM_ARCH "ARMARCH7")
    ENDIF()
    IF (NOT DEFINED VX_MIPS_ARCH)
        SET(VX_MIPS_ARCH "MIPSI32")
    ENDIF()
    IF (NOT DEFINED VX_PENTIUM_ARCH)
        SET(VX_PENTIUM_ARCH "PENTIUM")
    ENDIF()

    SET(VX_C_FLAGS "-ansi -std=c99  -Wextra -Wall -Winit-self  ")
    SET(VX_CXX_FLAGS "-ansi -Wextra -Wall -Winit-self ")

    IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
        SET(VX_C_FLAGS "${VX_C_FLAGS} -mrtp -DRTI_RTP")
        SET(VX_CXX_FLAGS "${VX_CXX_FLAGS} -mrtp -DRTI_RTP")
    ELSE(NOT ${RTIME_TARGET_NAME} STREQUAL "")
        SET(VX_C_FLAGS "${VX_C_FLAGS} -D_WRS_KERNEL ")
        SET(VX_CXX_FLAGS "${VX_CXX_FLAGS} -D_WRS_KERNEL")
    ENDIF()

    IF (${TARGET_HELP})
    ELSEIF (${RTIME_TARGET_NAME} MATCHES "Vx7")

        IF (${RTIME_TARGET_NAME} MATCHES "pentium64")
            SET(VX_VSB_DIR "$ENV{WIND_HOME}/workspace/vsb-itl_generic-64-nehalem")
        ELSEIF (NOT DEFINED VX_VSB_DIR)
            SET(VX_VSB_DIR "$ENV{WIND_HOME}/workbench-4/workspace/VSB")
        ENDIF()

        IF (${RTIME_TARGET_NAME} MATCHES "ppc")
            set(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_ENDIAN_BIG -DCPU=PPC32 -mlongcall -mstrict-align -m32")
            set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_ENDIAN_BIG -DCPU=PPC32 -mlongcall -mstrict-align -m32")
            IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
                set(VX_C_FLAGS "${VX_C_FLAGS} -mhard-float -mregnames")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -mhard-float -mregnames")
            ENDIF()
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "arm")
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "mips")
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "pentium64")
            IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
                set(VX_C_FLAGS "${VX_C_FLAGS} -D_VX_CPU=_VX_NEHALEM -DCPU=_VX_NEHALEM -m64 ")
                set(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_ENDIAN_LITTLE -D_VX_TOOL_FAMILY=gnu -D_VX_TOOL=gnu ")
                set(VX_C_FLAGS "${VX_C_FLAGS} -march=corei7 -mpopcnt -m64 -mcmodel=small -fno-builtin ")
                set(VX_C_FLAGS "${VX_C_FLAGS} -fno-omit-frame-pointer -DRTI_64BIT -fno-strict-aliasing ")
                set(VX_C_FLAGS "${VX_C_FLAGS} -D_C99 -D_HAS_C9X -MD -MP -fpic -D__SO_PICABILINUX__ ")
                set(VX_C_FLAGS "${VX_C_FLAGS} -D__SO64_SMALL__ ")

                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -D_VX_CPU=_VX_NEHALEM -DCPU=_VX_NEHALEM -m64 ")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_ENDIAN_LITTLE -D_VX_TOOL_FAMILY=gnu -D_VX_TOOL=gnu ")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -march=corei7 -mpopcnt -m64 -mcmodel=small -fno-builtin ")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -fno-omit-frame-pointer -DRTI_64BIT -fno-strict-aliasing ")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -D_C99 -D_HAS_C9X -MD -MP -fpic -D__SO_PICABILINUX__ ")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -D__SO64_SMALL__ ")

            ELSE()
                set(VX_C_FLAGS "${VX_C_FLAGS} -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_CONFIG_LP64 -fno-builtin -m64 -DRTI_ENDIAN_LITTLE -DCPU=_VX_NEHALEM -march=core2 ")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_CONFIG_LP64 -fno-builtin -m64 -DRTI_ENDIAN_LITTLE -DCPU=_VX_NEHALEM -march=core2 ")
            ENDIF()
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "pentium")
            set(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_ENDIAN_LITTLE -DCPU=PENTIUM -march=pentium ")
            set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_ENDIAN_LITTLE -DCPU=PENTIUM -march=pentium ")
        ELSE()
            IF (NOT ${TARGET_HELP})
                MESSAGE(FATAL_ERROR "Did not find a known CPU architecture in ${RTIME_TARGET_NAME}, expecting one of arm,ppc,pentium, or mips")
            ELSE()
                SET(CC_SUFFIX "dummy")
            ENDIF()
        ENDIF()

        IF (NOT DEFINED VX_VSB_CONFIG)
            ADD_DEFINITIONS(-D_VSB_CONFIG_FILE=\"${VX_VSB_DIR}/h/config/vsbConfig.h\")
        ELSE()
            ADD_DEFINITIONS(-D_VSB_CONFIG_FILE=\"${VX_VSB_CONFIG}\")
        ENDIF()

        IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
            INCLUDE_DIRECTORIES(${VX_VSB_DIR}/usr/h/public ${VX_VSB_DIR}/usr/h ${VX_VSB_DIR}/share/h)
 #           LINK_DIRECTORIES(${VX_VSB_DIR}/usr/lib/common/PIC)
            LINK_DIRECTORIES(${VX_VSB_DIR}/usr/lib/common)
            SET(LINK_DIRECTORIES "-L${VX_VSB_DIR}/usr/lib/common/PIC -Wl,--defsym,__wrs_rtp_base=0x200000 -L${VX_VSB_DIR}/usr/lib/common")
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "pentium64")
            INCLUDE_DIRECTORIES(${VX_VSB_DIR}/krnl/h/system ${VX_VSB_DIR}/krnl/h/public ${VX_VSB_DIR}/share/h)
            SET(CMAKE_EXECUTABLE_SUFFIX "so")
        ELSE(NOT ${RTIME_TARGET_NAME} STREQUAL "")
            INCLUDE_DIRECTORIES(${VX_VSB_DIR}/krnl/h/public ${VX_VSB_DIR}/share/h)
            SET(CMAKE_EXECUTABLE_SUFFIX "so")
        ENDIF()

    # Match both Vx6.X.X and Vx653
    ELSEIF (${RTIME_TARGET_NAME} MATCHES "Vx6")

        IF (${RTIME_TARGET_NAME} MATCHES "ppc")
           LINK_DIRECTORIES($ENV{WIND_BASE}/target/lib/usr/lib/ppc/${VX_PPC_ARCH}/common)
           SET(LINK_DIRECTORIES "-L$ENV{WIND_BASE}/target/lib/usr/lib/ppc/${VX_PPC_ARCH}/common")
            set(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_ENDIAN_BIG -DCPU=PPC32 -mlongcall -mstrict-align -m32")
            set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_ENDIAN_BIG -DCPU=PPC32 -mlongcall -mstrict-align -m32")
            IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
                set(VX_C_FLAGS "${VX_C_FLAGS} -mhard-float -mregnames")
                set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -mhard-float -mregnames")
            ENDIF()
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "arm")
            LINK_DIRECTORIES($ENV{WIND_BASE}/target/lib/usr/lib/arm/${VX_ARM_ARCH}/common)
            SET(LINK_DIRECTORIES "-L$ENV{WIND_BASE}/target/lib/usr/lib/arm/${VX_ARM_ARCH}/common")
            SET(VX_C_FLAGS "${VX_C_FLAGS} -mlong-calls")
            SET(VX_CXX_FLAGS "${VX_CXX_FLAGS} -mlong-calls")
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "mips")
            LINK_DIRECTORIES($ENV{WIND_BASE}/target/lib/usr/lib/mips/${VX_MIPS_ARCH}/common)
            SET(LINK_DIRECTORIES "-L$ENV{WIND_BASE}/target/lib/usr/lib/mips/${VX_MIPS_ARCH}/common")
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "pentium64")
            LINK_DIRECTORIES($ENV{WIND_BASE}/target/lib/usr/lib/pentium/NEHALEM/common)
            SET(LINK_DIRECTORIES "-L$ENV{WIND_BASE}/target/lib/usr/lib/pentium/NEHALEM/common")
#            set(VX_C_FLAGS "${VX_C_FLAGS} -m64 -DRTI_ENDIAN_LITTLE -DCPU=_VX_NEHALEM -march=core2 ")
#            set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -m64 -DRTI_ENDIAN_LITTLE -DCPU=_VX_NEHALEM -march=core2 ")
        ELSEIF (${RTIME_TARGET_NAME} MATCHES "pentium")
            LINK_DIRECTORIES($ENV{WIND_BASE}/target/lib/usr/lib/pentium/${VX_PENTIUM_ARCH}/common)
            SET(LINK_DIRECTORIES "-L$ENV{WIND_BASE}/target/lib/usr/lib/pentium/${VX_PENTIUM_ARCH}/common")
            set(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_ENDIAN_LITTLE -DCPU=PENTIUM -march=pentium ")
            set(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_ENDIAN_LITTLE -DCPU=PENTIUM -march=pentium ")
        ELSE()
            IF (NOT ${TARGET_HELP})
                MESSAGE(FATAL_ERROR "Did not find a known CPU architecture in ${RTIME_TARGET_NAME}, expecting one of arm,ppc,pentium, or mips")
            ELSE()
                SET(CC_SUFFIX "dummy")
            ENDIF()
        ENDIF()

        # Vx 6.x
        IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
            INCLUDE_DIRECTORIES($ENV{WIND_BASE}/target/usr/h $ENV{WIND_BASE}/target/usr/h/wrn/coreip)
        ELSE(NOT ${RTIME_TARGET_NAME} STREQUAL "")
            INCLUDE_DIRECTORIES($ENV{WIND_BASE}/target/h $ENV{WIND_BASE}/target/h/wrn/coreip)
        ENDIF()
    ENDIF()

    IF (${RTIME_TARGET_NAME} MATCHES "Vx653")
        # Set required ARINC flags
        SET(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_VXWORKS -DRTI_ARINC653=1")
        SET(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_VXWORKS -DRTI_ARINC653=1")
        IF (${RTIME_TARGET_NAME} MATCHES ".*Vx653-([0-9])\\.([0-9])\\.([0-9])\\.([0-9]).*")
            SET(RTI_VX653 "${CMAKE_MATCH_1}${CMAKE_MATCH_2}${CMAKE_MATCH_3}${CMAKE_MATCH_4}")
            SET(VX_C_FLAGS "${VX_C_FLAGS} -DRTI_VX653=${RTI_VX653}")
            SET(VX_CXX_FLAGS "${VX_CXX_FLAGS} -DRTI_VX653=${RTI_VX653}")
        ENDIF()

        # Fix multiple definitions of GETOPT_S
        SET(VX_C_FLAGS "${VX_C_FLAGS} -Wl,-z,muldefs")
        SET(VX_CXX_FLAGS "${VX_CXX_FLAGS} -Wl,-z,muldefs")

        SET(RTIME_INCLUDE_ARINC TRUE)
    ENDIF()

    IF (${TARGET_HELP})
    ELSEIF(NOT ${RTIME_TARGET_NAME} STREQUAL "")
        IF (${RTIME_TARGET_NAME} MATCHES ".*Vx([0-9]+)\\.([0-9]+).*")
            ADD_DEFINITIONS(-DVXWORKS_MAJOR_VERSION=${CMAKE_MATCH_1} -DVXWORKS_MINOR_VERSION=${CMAKE_MATCH_2})
        ENDIF()
    ENDIF()

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${VX_C_FLAGS}" CACHE INTERNAL "CMAKE_C_FLAGS" FORCE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VX_CXX_FLAGS}" CACHE INTERNAL "CMAKE_CXX_FLAGS" FORCE)

    IF (${RTIME_TARGET_NAME} MATCHES "Vx7")
        IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
            SET(CMAKE_FIND_ROOT_PATH ${VX_VSB_DIR}/usr/h ${VX_VSB_DIR}/share/h)
        ELSE(NOT ${RTIME_TARGET_NAME} STREQUAL "")
            SET(CMAKE_FIND_ROOT_PATH ${VX_VSB_DIR}/krnl/h/public ${VX_VSB_DIR}/share/h)
        ENDIF()
    # Match both Vx6.X.X and Vx653
    ELSEIF (${RTIME_TARGET_NAME} MATCHES "Vx6")
        IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
            SET(CMAKE_FIND_ROOT_PATH $ENV{WIND_BASE}/target/usr/h $ENV{WIND_BASE}/target/usr/h/wrn/coreip)
        ELSE(NOT ${RTIME_TARGET_NAME} STREQUAL "")
            SET(CMAKE_FIND_ROOT_PATH $ENV{WIND_BASE}/target/h $ENV{WIND_BASE}/target/h/wrn/coreip)
        ENDIF()
    ENDIF()
ENDIF(NOT VXWORKS_WRGNU)

IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
    set(CMAKE_C_COMPILE_OPTIONS_PIC "-fPIC" CACHE INTERNAL "CMAKE_C_COMPILE_OPTIONS_PIC" FORCE)
    set(CMAKE_CXX_COMPILE_OPTIONS_PIC "-fPIC" CACHE INTERNAL "CMAKE_CXX_COMPILE_OPTIONS_PIC" FORCE)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC " CACHE INTERNAL "CMAKE_SHARED_LIBRARY_C_FLAGS" FORCE)
    set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-fPIC " CACHE INTERNAL "CMAKE_SHARED_LIBRARY_CXX_FLAGS" FORCE)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-fPIC -shared" CACHE INTERNAL "CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS" FORCE)
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-fPIC -shared" CACHE INTERNAL "CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS" FORCE)
ELSE()
    set(CMAKE_C_COMPILE_OPTIONS_PIC "" CACHE INTERNAL "CMAKE_C_COMPILE_OPTIONS_PIC" FORCE)
    set(CMAKE_CXX_COMPILE_OPTIONS_PIC "" CACHE INTERNAL "CMAKE_CXX_COMPILE_OPTIONS_PIC" FORCE)
    set(CMAKE_SHARED_LIBRARY_C_FLAGS "" CACHE INTERNAL "CMAKE_SHARED_LIBRARY_C_FLAGS" FORCE)
    set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "" CACHE INTERNAL "CMAKE_SHARED_LIBRARY_CXX_FLAGS" FORCE)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-Bsymbolic-functions -r" CACHE INTERNAL "CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS" FORCE)
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-Bsymbolic-functions -r" CACHE INTERNAL "CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS" FORCE)
ENDIF()

SET(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> ${CMAKE_C_FLAGS} <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
SET(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER> ${CMAKE_CXX_FLAGS} <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
SET(CMAKE_C_CREATE_SHARED_LIBRARY "<CMAKE_C_COMPILER> <LANGUAGE_COMPILE_FLAGS> ${LINK_DIRECTORIES} <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> ${VX_C_FLAGS} <LINK_FLAGS> -o <TARGET> <SONAME_FLAG><TARGET_SONAME> <OBJECTS>")
SET(CMAKE_C_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -C -E <SOURCE> > <PREPROCESSED_SOURCE>" CACHE INTERNAL "CMAKE_C_CREATE_PREPROCESSED_SOURCE" FORCE)
SET(CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -C -E <SOURCE> > <PREPROCESSED_SOURCE>" CACHE INTERNAL "CMAKE_CXX_CREATE_PREPROCESSED_SOURCE"  FORCE)

# Apply munching only if we have found the munching tools.
# Note that to link C++ target agains the Micro libraries we are using (as a wrapper for the linker)
# the C compiler instead of the C++ compiler. Apparently, when linking with the C++ compiler,
# the target is also linked against C++ libraries that are not properly compiled for some architectures
# (e.g. missing -mlong-calls on arm targets), thus the compiled target cannot be loaded on VxWorks.
IF (MUNCH)
    MESSAGE(STATUS "Found munch tool: ${MUNCH}")

    IF (WIN32)
        SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
            "${CMAKE_C_COMPILER} ${VX_C_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET>.munch <LINK_LIBRARIES>"
            "${CMAKE_CURRENT_LIST_DIR}/../../../scripts/rtime-munch.bat ${MUNCH_NM} <TARGET> ${MUNCH}"
            "${CMAKE_C_COMPILER} ${LINK_DIRECTORIES} ${VX_C_FLAGS} -fdollars-in-identifiers -c <TARGET>_ctordtor.c -o <TARGET>_ctordtor.o"
            "${CMAKE_LINKER} -r -nostdlib ${MUNCH_FLAGS} <TARGET>.munch <TARGET>_ctordtor.o -o <TARGET>"
        )
    ELSE()
        SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
            "${CMAKE_C_COMPILER} ${VX_C_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET>.munch <LINK_LIBRARIES>"
            "${CMAKE_CURRENT_LIST_DIR}/../../../scripts/rtime-munch.sh ${MUNCH_NM} <TARGET> ${MUNCH}"
            "${CMAKE_C_COMPILER} ${LINK_DIRECTORIES} ${VX_C_FLAGS} -fdollars-in-identifiers -c <TARGET>_ctordtor.c -o <TARGET>_ctordtor.o"
            "${CMAKE_LINKER} -r -nostdlib ${MUNCH_FLAGS} <TARGET>.munch <TARGET>_ctordtor.o -o <TARGET>"
        )
    ENDIF()
ELSE()
    SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_CXX_COMPILER} ${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} ${VX_CXX_FLAGS} <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
ENDIF()

IF (${RTIME_TARGET_NAME} MATCHES "arm")
    IF (${RTIME_TARGET_NAME} MATCHES "_rtp")
    ELSE()
        SET(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_CREATE_SHARED_LIBRARY})
    ENDIF()
ENDIF()

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

FOREACH(type SHARED_LIBRARY SHARED_MODULE EXE)
  set(CMAKE_${type}_LINK_STATIC_C_FLAGS "-Wl,-Bstatic")
  set(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-Wl,-Bdynamic")
ENDFOREACH()

SET(PLATFORM_LIBS )

SET(RTIME_OBJDUMP_OPT --disassemble-all)
SET(RTIME_OBJDUMPCHKSUM $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/objdump${_RTIME_CPU_SUFFIX})
SET(RTIME_OBJDUMP $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/objdump${_RTIME_CPU_SUFFIX})
SET(RTIME_AR $ENV{WIND_GNU_PATH}/$ENV{WIND_HOST_TYPE}/bin/ar${_RTIME_CPU_SUFFIX})
SET(RTIME_OBJDUMPCHKSUM_OPT --section=.text --section=.data --section=.rodata --disassemble-all)
SET(RTIME_OBJ_SUFFIX obj)
SET(RTIME_AR_EXRACT -x)

IF (${TARGET_HELP})
    MESSAGE("PLATFORM NOTES\n")
    MESSAGE(" For VxWorks (RTIME_TARGET_NAME includes Vx) the ")
    MESSAGE(" following libraries are included:")
    MESSAGE(" - ${PLATFORM_LIBS}")
    MESSAGE("\n")
ENDIF()
