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

set(SLINT_STYLE "material" CACHE STRING "Slint style" FORCE)

if (MSVC)
  set(SLINT_BACKEND "winit" CACHE STRING "Slint backend" FORCE)
endif()

if (WIN32)
  set(VERSION_RC_PATH ${CMAKE_SOURCE_DIR}/sast-evento.rc)
  configure_file(
      ${CMAKE_SOURCE_DIR}/.cmake/version_exe.rc.in
      ${VERSION_RC_PATH}
  )
  configure_file(
      ${CMAKE_SOURCE_DIR}/.cmake/InstallerScript.iss.in
      ${CMAKE_SOURCE_DIR}/action-cli/InstallerScript.iss
  )
  set(WIN32_MANIFEST_PATH ${CMAKE_SOURCE_DIR}/app.manifest)
endif()

if (APPLE)
  set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})
  set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
  set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})

  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${spectral_VERSION})
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${spectral_VERSION})

  set(ICON_NAME "app.icns")
  set(${PROJECT_NAME}_MAC_ICON "${CMAKE_SOURCE_DIR}/ui/assets/image/icon/${ICON_NAME}")
  set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
  set_property(SOURCE "${${PROJECT_NAME}_MAC_ICON}" PROPERTY
      MACOSX_PACKAGE_LOCATION Resources)
endif()

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

# source code
add_subdirectory(src)
# sast-link-sdk
add_subdirectory(3rdpart/sast-link-cxx-sdk)

# credential storage
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(3rdpart/keychain)
set(BUILD_SHARED_LIBS ON)

# install
install(TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)