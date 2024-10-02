cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC Runtime Library Selection
if (VCPKG_TARGET_TRIPLET MATCHES "-windows-static$")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/.cmake/)
include(Version)

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

project(sast-evento VERSION ${VERSION_SEMANTIC} LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # That optimization causes a crash when re-logging in, so we disable it.
  # Additional research is needed to determine the root cause, but wait...
  # HELP WANTED! I do not know about GNU compiler at all! - @alampy
  add_compile_options(-fno-ipa-sra -fno-ipa-cp-clone)
endif()

set(SLINT_STYLE "material" CACHE STRING "Slint style" FORCE)
set(SLINT_FEATURE_GETTEXT ON CACHE BOOL "Slint gettext feature" FORCE)

if (WIN32)
  # Target Windows 7
  add_definitions(-D_WIN32_WINNT=0x0601)

  set(VERSION_RC_PATH ${CMAKE_SOURCE_DIR}/sast-evento.rc)
  configure_file(
      ${CMAKE_SOURCE_DIR}/.cmake/version_exe.rc.in
      ${VERSION_RC_PATH}
  )
  set(WIN32_MANIFEST_PATH ${CMAKE_SOURCE_DIR}/app.manifest)

  set(BUILD_SAST_LINK_SHARED OFF)
endif()

# find Slint
find_package(Slint QUIET)
if (NOT Slint_FOUND)
  message("Slint could not be located in the CMake module search path. Downloading it from Git and building it locally")
  include(FetchContent)
  if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
    set(FETCHCONTENT_SYSTEM_IF_AVAILABLE SYSTEM)
  else()
    set(FETCHCONTENT_SYSTEM_IF_AVAILABLE)
  endif()
  FetchContent_Declare(
    Slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    # `release/1` will auto-upgrade to the latest Slint >= 1.0.0 and < 2.0.0
    # `release/1.0` will auto-upgrade to the latest Slint >= 1.0.0 and < 1.1.0
    GIT_TAG release/1
    SOURCE_SUBDIR api/cpp
    ${FETCHCONTENT_SYSTEM_IF_AVAILABLE}
  )
  FetchContent_MakeAvailable(Slint)
endif (NOT Slint_FOUND)

# find vcpkg-installed
find_package(spdlog REQUIRED)
find_package(Boost REQUIRED COMPONENTS system url beast process)
find_package(OpenSSL 3.3.0 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(tomlplusplus REQUIRED IMPORTED_TARGET tomlplusplus)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/sast-link-cxx-sdk/CMakeLists.txt" OR 
    NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ui/modules/surrealism-ui/index.slint" OR
    NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/keychain/CMakeLists.txt")
  message(FATAL_ERROR "Git submodule not found. Run `git submodule update --init` from the source tree to fetch the submodule contents.")
endif()

# i18n
set(SOURCE_LOCALE_DIR "${CMAKE_SOURCE_DIR}/ui/locale")

# source code
add_subdirectory(src)
# sast-link-sdk
add_subdirectory(3rdpart/sast-link-cxx-sdk)

# credential storage
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(3rdpart/keychain)
set(BUILD_SHARED_LIBS ON)

# install
file(WRITE ${CMAKE_BINARY_DIR}/sast-evento-version.txt ${VERSION_SEMANTIC})
install(FILES ${CMAKE_BINARY_DIR}/sast-evento-version.txt DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION .)
install(TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(LINUX)
  install(FILES ${SOURCE_LOCALE_DIR}/en/LC_MESSAGES/sast-evento.mo DESTINATION ./locale/en/LC_MESSAGES)
  install(FILES ${SOURCE_LOCALE_DIR}/zh/LC_MESSAGES/sast-evento.mo DESTINATION ./locale/zh/LC_MESSAGES)
endif()