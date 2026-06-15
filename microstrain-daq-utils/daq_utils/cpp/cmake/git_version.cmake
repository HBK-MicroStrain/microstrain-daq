# Extracts the version for a project from the latest git tag.
#
# Arguments:
#     PROJECT_PREFIX - The project prefix in the git tag ("v" in "v1.2.3").
#
# Returns:
#        FULL_VERSION_OUT: The extracted full version string (ex: "1.2.3-beta").
#     NUMERIC_VERSION_OUT: The extracted numeric version string (ex: "1.2.3").
#
function(get_project_version_from_git_tag PROJECT_PREFIX FULL_VERSION_OUT NUMERIC_VERSION_OUT)
    execute_process(
        COMMAND git describe --tags --match "${PROJECT_PREFIX}*" --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(GIT_TAG)
        # Extract full version (e.g., "v1.2.3-alpha" -> "1.2.3-alpha")
        string(REGEX REPLACE "^${PROJECT_PREFIX}" "" FULL_VERSION_STRING ${GIT_TAG})

        # Extract just numeric version for CMake project() (ex: "1.2.3-alpha" -> "1.2.3")
        string(REGEX REPLACE "^([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" NUMERIC_VERSION_STRING "${FULL_VERSION_STRING}")

        set(${NUMERIC_VERSION_OUT} ${NUMERIC_VERSION_STRING} PARENT_SCOPE)
        set(${FULL_VERSION_OUT} ${FULL_VERSION_STRING} PARENT_SCOPE)
    else()
        message(WARNING "No git tag found matching '${PROJECT_PREFIX}*', using 0.0.0 as a fallback.")
        set(${NUMERIC_VERSION_OUT} "0.0.0" PARENT_SCOPE)
        set(${FULL_VERSION_OUT} "0.0.0" PARENT_SCOPE)
    endif()
endfunction()
