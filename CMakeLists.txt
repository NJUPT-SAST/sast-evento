cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/.cmake/)
include(Version)

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

project(sast-evento VERSION ${VERSION_SEMANTIC} LANGUAGES CXX)

# find Slint
find_package(Slint QUIET)
if (NOT Slint_FOUND)
  message("Slint could not be located in the CMake module search path. Downloading it from Git and building it locally")
  include(FetchContent)
  FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    # `release/1` will auto-upgrade to the latest Slint >= 1.0.0 and < 2.0.0
    # `release/1.0` will auto-upgrade to the latest Slint >= 1.0.0 and < 1.1.0
    GIT_TAG release/1
    SOURCE_SUBDIR api/cpp
  )
  FetchContent_MakeAvailable(Slint)
endif (NOT Slint_FOUND)

# find vcpkg-installed
find_package(spdlog REQUIRED)
find_package(Boost REQUIRED COMPONENTS system url beast)
find_package(OpenSSL 3.3.0 REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME})

# definitions for Logger
target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    $<$<CONFIG:Debug>:EVENTO_DEBUG>
    $<$<CONFIG:Release>:EVENTO_RELEASE>
)

# source code
add_subdirectory(src)
# sast-link-sdk
add_subdirectory(3rdpart/sast-link-cxx-sdk)

slint_target_sources(${PROJECT_NAME} ui/app.slint
  LIBRARY_PATHS surrealism=${CMAKE_SOURCE_DIR}/ui/modules/surrealism-ui/index.slint
)

set_property(TARGET ${PROJECT_NAME} PROPERTY SLINT_EMBED_RESOURCES embed-files)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()