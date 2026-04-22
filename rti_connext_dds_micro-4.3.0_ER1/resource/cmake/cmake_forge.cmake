###############################################################################
# (c) Copyright, Real-Time Innovations 2016-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description:
# ------------
# Forge CMake Integration
#
# This file defines the ADD_FORGE function to integrate Forge builds into
# CMake projects.
#
################################################################################

# Check for forge and anvil executables
FIND_PROGRAM(FORGE_EXECUTABLE forge)
FIND_PROGRAM(ANVIL_EXECUTABLE anvil)

IF(NOT FORGE_EXECUTABLE OR NOT ANVIL_EXECUTABLE)
    IF(RTIME_DEVTREE)
        MESSAGE(FATAL_ERROR
            "Forge unit tests are enabled (RTI_BUILD_FORGE_UNITTESTS) but required "
            "tools were not found. Install forge and anvil, or configure with "
            "-DRTI_BUILD_FORGE_UNITTESTS=OFF to skip them.")
    ELSE()
        # Not a devtree build: unit tests are not built anyway, silently disable
        # forge so none of the downstream IF(RTI_BUILD_FORGE_UNITTESTS) blocks fire.
        SET(RTI_BUILD_FORGE_UNITTESTS FALSE)
    ENDIF()
ENDIF()

# Define a serial job pool for Forge builds to prevent parallel execution conflicts.
# All forge targets share the same workspace directory (SUBSTRATE parameter), and
# concurrent 'forge run' commands cause race conditions, lock file conflicts, and
# workspace corruption. This pool ensures only one forge build runs at a time while
# allowing other build steps (compilation, linking, profile generation) to remain parallel.
IF(NOT DEFINED RTI_FORGE_POOL_INITIALIZED)
    SET(RTI_FORGE_POOL_INITIALIZED TRUE CACHE INTERNAL "Forge serial pool initialization guard")
    SET_PROPERTY(GLOBAL PROPERTY JOB_POOLS forge_serial=1)
    MESSAGE(STATUS "Forge builds will execute serially to prevent workspace conflicts")

    # Initialize tracking for serial dependency chain
    SET_PROPERTY(GLOBAL PROPERTY RTI_FORGE_LAST_TARGET "")
ENDIF()

#===============================================================================
# FUNCTION ..: ADD_FORGE
# SYNOPSIS ..: Add a Forge build target
# USAGE .....: ADD_FORGE(target
#                  PROGRAM <program_name>
#                  ARCH <architecture>
#                  SUBSTRATE <substrate_dir>
#                  [OUTPUT_DIR <output_dir>]
#                  [PROFILE_OUTPUT <profile_path>]
#                  [EXTENDS <base_profile>]
#                  [LIBS <lib1> <lib2> ...]
#                  [LIB_DIRS <dir1> <dir2> ...]
#                  [DEPENDS <file_dep1> <file_dep2> ...]
#                  [TARGET_DEPENDS <target_dep1> <target_dep2> ...]
#              )
#
# This function:
# 1. Generates a Forge profile using 'anvil create-profile'
# 2. Runs 'forge run' to build the firmware
# 3. Creates a custom target that depends on the build output
#
# Arguments:
#   PROGRAM: The name of the program to build (e.g., 'utest').
#   ARCH: The target architecture string.
#   SUBSTRATE: Path to the substrate directory (workspace).
#   OUTPUT_DIR: Directory where build artifacts will be placed. Defaults to ${CMAKE_CURRENT_BINARY_DIR}/${target}_forge.
#   PROFILE_OUTPUT: Path where the generated profile YAML will be saved. Defaults to ${OUTPUT_DIR}/${target}_profile.yaml.
#   EXTENDS: Path to a base profile to extend.
#   LIBS: List of libraries to link against.
#   LIB_DIRS: List of directories to search for libraries.
#   DEPENDS: List of file dependencies (e.g. profiles, source files) that trigger a rebuild.
#   TARGET_DEPENDS: List of CMake targets (e.g. libraries) that must be built before this target.
#
#===============================================================================
FUNCTION(ADD_FORGE target)
    SET(options DEBUG)
    SET(oneValueArgs PROGRAM ARCH SUBSTRATE OUTPUT_DIR PROFILE_OUTPUT EXTENDS)
    SET(multiValueArgs LIBS LIB_DIRS DEPENDS TARGET_DEPENDS)
    CMAKE_PARSE_ARGUMENTS(FORGE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    IF(NOT FORGE_EXECUTABLE OR NOT ANVIL_EXECUTABLE)
        MESSAGE(FATAL_ERROR "Forge or Anvil not found, cannot add forge target ${target}")
    ENDIF()

    # Default output directory
    IF(NOT FORGE_OUTPUT_DIR)
        SET(FORGE_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target}_forge")
    ENDIF()

    # Default profile output path
    IF(NOT FORGE_PROFILE_OUTPUT)
        SET(FORGE_PROFILE_OUTPUT "${FORGE_OUTPUT_DIR}/${target}_profile.yaml")
    ENDIF()

    # Ensure output directory exists (CMake will create it at build time if we use it in COMMAND,
    # but good to be explicit if needed, though COMMAND usually handles it or we rely on forge/anvil)
    # Actually, 'anvil' might expect the dir to exist or create it. 'forge run' creates output dir.
    # We'll let the tools handle it or create it via command.
    FILE(MAKE_DIRECTORY ${FORGE_OUTPUT_DIR})

    # 1. Generate Profile
    SET(ANVIL_CMD ${ANVIL_EXECUTABLE} create-profile
        --name ${target}
        --output ${FORGE_PROFILE_OUTPUT}
    )

    IF(FORGE_EXTENDS)
        LIST(APPEND ANVIL_CMD --extends ${FORGE_EXTENDS})
    ENDIF()

    IF(FORGE_LIBS)
        LIST(APPEND ANVIL_CMD --libs ${FORGE_LIBS})
    ENDIF()

    IF(FORGE_LIB_DIRS)
        LIST(APPEND ANVIL_CMD --lib-dirs ${FORGE_LIB_DIRS})
    ENDIF()

    ADD_CUSTOM_COMMAND(OUTPUT ${FORGE_PROFILE_OUTPUT}
        COMMAND ${ANVIL_CMD}
        DEPENDS ${FORGE_EXTENDS} ${FORGE_DEPENDS}
        COMMENT "Generating Forge profile for ${target}"
    )

    # 2. Run Forge
    # The primary output artifact extension is declared per-architecture in cmake_forge_tests.txt
    # as "<arch>_primary_artifact" ("bin" or "elf"). Defaults to "bin" if not set.
    SET(FORGE_PRIMARY_EXT "${${FORGE_ARCH}_primary_artifact}")
    IF(NOT FORGE_PRIMARY_EXT)
        SET(FORGE_PRIMARY_EXT "bin")
    ENDIF()
    SET(FORGE_OUTPUT_PRIMARY "${FORGE_OUTPUT_DIR}/firmware.${FORGE_PRIMARY_EXT}")

    SET(FORGE_CMD ${FORGE_EXECUTABLE} run
        --arch ${FORGE_ARCH}
        --program ${FORGE_PROGRAM}
        --profile ${FORGE_PROFILE_OUTPUT}
        --substrate ${FORGE_SUBSTRATE}
        --output ${FORGE_OUTPUT_DIR}
    )

    IF(FORGE_DEBUG)
        LIST(APPEND FORGE_CMD --debug)
    ENDIF()

    ADD_CUSTOM_COMMAND(OUTPUT ${FORGE_OUTPUT_PRIMARY}
        COMMAND ${FORGE_CMD}
        DEPENDS ${FORGE_PROFILE_OUTPUT} ${FORGE_DEPENDS}
        COMMENT "Building with Forge for ${target}"
    )

    # 3. Create Target
    ADD_CUSTOM_TARGET(${target} ALL DEPENDS ${FORGE_OUTPUT_PRIMARY})

    # Assign this target to the serial pool to prevent parallel forge execution
    SET_PROPERTY(TARGET ${target} PROPERTY JOB_POOL forge_serial)

    # Add dependencies to the target as well, to ensure they are built before this target runs
    IF(FORGE_TARGET_DEPENDS)
        ADD_DEPENDENCIES(${target} ${FORGE_TARGET_DEPENDS})
    ENDIF()

    # CRITICAL: Create dependency chain to enforce serial execution
    # Make this forge target depend on the previous forge target (if any)
    GET_PROPERTY(LAST_FORGE_TARGET GLOBAL PROPERTY RTI_FORGE_LAST_TARGET)
    IF(LAST_FORGE_TARGET)
        ADD_DEPENDENCIES(${target} ${LAST_FORGE_TARGET})
        MESSAGE(STATUS "Forge target ${target} will wait for ${LAST_FORGE_TARGET} to complete")
    ENDIF()

    # Update the last forge target to this one
    SET_PROPERTY(GLOBAL PROPERTY RTI_FORGE_LAST_TARGET ${target})

ENDFUNCTION(ADD_FORGE)
