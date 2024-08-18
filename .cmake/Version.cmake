# MIT License
# Copyright (c) 2022 craig-barecpper@crog.uk
# Distributed under the MIT License. See accompanying LICENSE or https://cmake.org/licensing for details.

# @note 3.20 required for `GENERATED` attribute to be project-wide i.e. Version.h isn't build until build-time
cmake_minimum_required(VERSION 3.20)

#TODO? if ( DEFINED VERSION_SEMANTIC )
    #return()
#endif()

message(CHECK_START "Version.cmake")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

set(VERSION_OUT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Destination directory into which `Version.cmake` shall genrate Versioning header files")
set(VERSION_SOURCE_DIR "${CMAKE_SOURCE_DIR}" CACHE PATH "Repositroy directory used for `Version.cmake` repo versioning")

# Get cmakeVersion information
message(CHECK_START "Find git")
if( NOT DEFINED GIT_EXECUTABLE ) # Find Git or bail out
    find_package( Git )
    if( NOT Git_FOUND )
        message(CHECK_FAIL "Not found in PATH")
    else()
        message(CHECK_PASS "Found: '${GIT_EXECUTABLE}'")
    endif()
else()
    message(CHECK_PASS "Using pre-defined GIT_EXECUTABLE: '${GIT_EXECUTABLE}'")
endif()
    
# Git describe
# @note Exclude 'tweak' tags in the form v0.1.2-30 i.e. with the '-30' to avoid a second suffix being appended e.g v0.1.2-30-12
set(GIT_VERSION_COMMAND "${GIT_EXECUTABLE}" -C "${VERSION_SOURCE_DIR}" --no-pager describe --tags --exclude "v[0-9]*[._][0-9]*[._][0-9]*-[0-9]*" --always --dirty --long)

# Git count
# @note We only count commits on the current branch and not comits in merge branches via '--first-parent'. The count is never unique but the Sha will be!
set(GIT_COUNT_COMMAND "${GIT_EXECUTABLE}" -C "${VERSION_SOURCE_DIR}" rev-list --count  --first-parent HEAD)

# Git cache path
set(GIT_CACHE_PATH_COMMAND "${GIT_EXECUTABLE}" -C "${VERSION_SOURCE_DIR}" rev-parse --git-dir)

macro(version_parseSemantic semVer)
    if( "${semVer}" MATCHES "^v?([0-9]+)[._]([0-9]+)[._]?([0-9]+)?[-]([0-9]+)[-][g]([._0-9A-Fa-f]+)[-]?(dirty)?$")
        set( _VERSION_SET TRUE)
        math( EXPR _VERSION_MAJOR  "${CMAKE_MATCH_1}+0")
        math( EXPR _VERSION_MINOR  "${CMAKE_MATCH_2}+0")
        math( EXPR _VERSION_PATCH  "${CMAKE_MATCH_3}+0")
        math( EXPR _VERSION_COMMIT "${CMAKE_MATCH_4}+0")
        set( _VERSION_SHA   "${CMAKE_MATCH_5}")
        set( _VERSION_DIRTY "${CMAKE_MATCH_6}")
        set( _VERSION_SEMANTIC ${_VERSION_MAJOR}.${_VERSION_MINOR}.${_VERSION_PATCH}.${_VERSION_COMMIT} )    
        set( _VERSION_FULL ${git_describe} )
    else()
        set( _VERSION_SET FALSE)
    endif()
endmacro()

macro(version_export_variables)
    set( VERSION_SET      ${_VERSION_SET} CACHE INTERNAL "" FORCE)
    set( VERSION_MAJOR    ${_VERSION_MAJOR} CACHE INTERNAL "" FORCE)
    set( VERSION_MINOR    ${_VERSION_MINOR} CACHE INTERNAL "" FORCE)
    set( VERSION_PATCH    ${_VERSION_PATCH} CACHE INTERNAL "" FORCE)
    set( VERSION_COMMIT   ${_VERSION_COMMIT} CACHE INTERNAL "" FORCE)
    set( VERSION_SHA      ${_VERSION_SHA} CACHE INTERNAL "" FORCE)
    set( VERSION_DIRTY    ${_VERSION_DIRTY} CACHE INTERNAL "" FORCE)
    set( VERSION_SEMANTIC ${_VERSION_SEMANTIC} CACHE INTERNAL "" FORCE)
    set( VERSION_FULL     ${_VERSION_FULL} CACHE INTERNAL "" FORCE)
endmacro()

message(CHECK_START "Git Cache-Path")
if ( DEFINED GIT_CACHE_PATH)
    message(CHECK_PASS "Using pre-defined GIT_CACHE_PATH '${GIT_CACHE_PATH}'")
else()
    execute_process(
        COMMAND           ${GIT_CACHE_PATH_COMMAND}
        RESULT_VARIABLE   _GIT_RESULT
        OUTPUT_VARIABLE   GIT_CACHE_PATH
        ERROR_VARIABLE    _GIT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
        ${capture_output}
    )
    if( NOT _GIT_RESULT EQUAL 0 )
        message( CHECK_FAIL "Failed: ${GIT_CACHE_PATH_COMMAND}\nRESULT_VARIABLE:'${_GIT_RESULT}' \nOUTPUT_VARIABLE:'${GIT_CACHE_PATH}' \nERROR_VARIABLE:'${_GIT_ERROR}'")
    else()
        file(TO_CMAKE_PATH "${VERSION_SOURCE_DIR}/${GIT_CACHE_PATH}" GIT_CACHE_PATH)
        message(CHECK_PASS "Success '${GIT_CACHE_PATH}'")
    endif()
endif()

message(CHECK_START "Git Describe")
execute_process(
    COMMAND           ${GIT_VERSION_COMMAND}
    RESULT_VARIABLE   _GIT_RESULT
    OUTPUT_VARIABLE   git_describe
    ERROR_VARIABLE    _GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    ${capture_output}
)
if( NOT _GIT_RESULT EQUAL 0 )
    message( CHECK_FAIL "Failed: ${GIT_VERSION_COMMAND}\nResult:'${_GIT_RESULT}' Error:'${_GIT_ERROR}'")
    if ( "${_GIT_ERROR}" STREQUAL "fatal: bad revision 'HEAD'")
        set(_VERSION_NOT_GIT_REPO TRUE) # Flag that we don't have a valid git-repository'
    endif()
else()
    message(CHECK_PASS "Success '${git_describe}'")

    message(CHECK_START "Parse version")
    version_parseSemantic(${git_describe})
    if( ${_VERSION_SET} )
        message(CHECK_PASS "Tag '${git_describe}' is a valid semantic version [${_VERSION_SEMANTIC}]")
    else()
        message(CHECK_FAIL "'${git_describe}' is not a valid semantic-version e.g. 'v0.1.2-30'")
    endif()
endif()
    
if(NOT DEFINED _VERSION_FULL AND NOT _VERSION_NOT_GIT_REPO)
    message(CHECK_START "Fallback as Git-Count")
    execute_process(
        COMMAND           ${GIT_COUNT_COMMAND}
        RESULT_VARIABLE   _GIT_RESULT
        OUTPUT_VARIABLE   git_count
        ERROR_VARIABLE    _GIT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
        ${capture_output}
    )
    if( NOT _GIT_RESULT EQUAL 0 )
        message( CHECK_FAIL "Failed: ${GIT_COUNT_COMMAND}\nResult:'${_GIT_RESULT}' Error:'${_GIT_ERROR}'")
    else()    
        set(git_describe "0.0.0-${git_count}-g${git_describe}")
        version_parseSemantic(${git_describe})
        if( ${VERSION_SET} )
            message(CHECK_PASS "git-tag '${git_describe} is a valid semantic version")
        else()
            message(CHECK_FAIL "'${git_describe}' is not a valid semantic-version e.g. 'v0.1.2-30'")
        endif()
    endif()
endif()

function(gitversion_configure_file VERSION_H_TEMPLATE VERSION_H)
    configure_file (
        "${VERSION_H_TEMPLATE}"
        "${VERSION_H}"
    )
endfunction()

version_export_variables()

if ( VERSION_GENERATE_NOW )
    gitversion_configure_file( ${VERSION_H_TEMPLATE} ${VERSION_H})
else() 
    set(VERSION_H_FILENAME "Version.h")
    set(VERSION_H_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/${VERSION_H_FILENAME}.in")
    set(VERSION_H "${VERSION_OUT_DIR}/${VERSION_H_FILENAME}")

    # If no Version.h.in exists we generate the template with the default
    message(CHECK_START "Find '${VERSION_H_FILENAME}.in'")
    if ( NOT EXISTS ${VERSION_H_TEMPLATE} )
        set(VERSION_H_TEMPLATE "${VERSION_OUT_DIR}/${VERSION_H_FILENAME}.in")
        message( CHECK_FAIL "Not Found. Generating '${VERSION_H_TEMPLATE}'")

        file(WRITE ${VERSION_H_TEMPLATE}
      [=[
#define VERSION_MAJOR @_VERSION_MAJOR@
#define VERSION_MINOR @_VERSION_MINOR@
#define VERSION_PATCH @_VERSION_PATCH@
#define VERSION_COMMIT @_VERSION_COMMIT@
#define VERSION_SHA "@_VERSION_SHA@"
#define VERSION_SEMANTIC "@_VERSION_SEMANTIC@"
#define VERSION_FULL "@_VERSION_FULL@"
      ]=])
        if ( NOT EXISTS ${VERSION_H_TEMPLATE} )
            message( FATAL_ERROR "Failed to create template ${VERSION_H_TEMPLATE}")
        endif()
    else()
        message( CHECK_PASS "Found '${VERSION_H_TEMPLATE}'")
    endif()

    # A custom target is used to update Version.h
    add_custom_target( genCmakeVersion
        ALL
        BYPRODUCTS "${VERSION_H}"
        SOURCES "${VERSION_H_TEMPLATE}"
        DEPENDS "${GIT_CACHE_PATH}/index"
            "${GIT_CACHE_PATH}/HEAD"
        COMMENT "Version.cmake: Generating `${VERSION_H_FILENAME}`"
        COMMAND ${CMAKE_COMMAND}            
            -B "${VERSION_OUT_DIR}"
            -D VERSION_GENERATE_NOW=YES
            -D VERSION_H_TEMPLATE=${VERSION_H_TEMPLATE}
            -D VERSION_H=${VERSION_H}
            -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
            -D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH} 
            -P ${CMAKE_CURRENT_LIST_FILE}
        WORKING_DIRECTORY ${VERSION_SOURCE_DIR}  
        VERBATIM
    )

    add_library( cmakeVersion INTERFACE )
    target_include_directories(cmakeVersion INTERFACE "${VERSION_OUT_DIR}")

    # @note Explicit file-names - prevent Cmake finding `Version.h.in` for `Version.h`
    if (POLICY CMP0115)
        cmake_policy(SET CMP0115 NEW)
    endif()

    target_sources( cmakeVersion 
        INTERFACE 
            "${VERSION_H}")
    add_dependencies( cmakeVersion 
        INTERFACE genCmakeVersion )
        
    add_library( version::version ALIAS cmakeVersion )
endif()

list(POP_BACK CMAKE_MESSAGE_INDENT)

if(VERSION_GENERATE_NOW)
    set(VERSION_H_GENERATED TRUE)
else()
    get_source_file_property(VERSION_H_GENERATED "${VERSION_H}" GENERATED )
endif()

if ( NOT _VERSION_NOT_GIT_REPO )
    if ( NOT VERSION_SET )
        message(CHECK_FAIL "Version.cmake failed - VERSION_SET==false")
    elseif ( ${VERSION_H_GENERATED} )
        message(CHECK_PASS "${VERSION_FULL} [${VERSION_SEMANTIC}] {Generated}")
    elseif(EXISTS ${VERSION_H})
        message(CHECK_PASS "Using pre-defined '${VERSION_H}'")
    else()
        message(CHECK_FAIL "Failed, ${VERSION_H} not available")
    endif()
else()
    message(CHECK_FAIL "Failed, Error reading Git repository")
endif()