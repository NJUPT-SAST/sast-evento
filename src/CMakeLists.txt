add_executable(${PROJECT_NAME} ${VERSION_RC_PATH})

file(GLOB_RECURSE SOURCES Controller/*.cc Infrastructure/*.cc)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} main.cc)

# hide cli in Release Mode for MSVC
if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_HAS_CO_AWAIT)
endif()

# use io_uring and intl on Linux
if (LINUX)
  target_compile_definitions(${PROJECT_NAME} PRIVATE 
    BOOST_ASIO_HAS_IO_URING 
  )
  find_library(URING_LIBRARY NAMES liburing.a liburing.so REQUIRED)
  message(STATUS "Found liburing: ${URING_LIBRARY}")
  find_package(Intl REQUIRED)
  set(INTL_LIBRARY Intl::Intl)
endif()

if (SPEED_UP_DEBUG_BUILD)
  message("Using absolute path of resources in the executable")  
  set(SLINT_GENERATE_COMPILE_UNITS 10)
  set(SLINT_RESOURCES_POLICY "as-absolute-path")
else()
  message("Embedding resources as files")
  set(SLINT_GENERATE_COMPILE_UNITS 1)
  set(SLINT_RESOURCES_POLICY "embed-files")
endif()

slint_target_sources(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/ui/app.slint 
  COMPILATION_UNITS ${SLINT_GENERATE_COMPILE_UNITS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY SLINT_EMBED_RESOURCES ${SLINT_RESOURCES_POLICY})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  set(PLATFORM PLATFORM_WINDOWS)
elseif(APPLE)
  set(PLATFORM PLATFORM_APPLE)
elseif(LINUX)
  set(PLATFORM PLATFORM_LINUX)
endif()

# definitions for Logger
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<CONFIG:Debug>:EVENTO_DEBUG>
    $<$<CONFIG:Release>:EVENTO_RELEASE>
    ${PLATFORM}
    LOCALE_DIR="${SOURCE_LOCALE_DIR}"
    EVENTO_API_V1
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    spdlog::spdlog
    Boost::boost
    Boost::system
    Boost::filesystem
    Boost::url
    OpenSSL::Crypto
    OpenSSL::SSL
    nlohmann_json::nlohmann_json
    sast-link
    keychain
    Slint::Slint
    version::version
    ${INTL_LIBRARY}
    ${URING_LIBRARY}
)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()

if(LINUX)
  find_program(MSGFMT_EXECUTABLE msgfmt REQUIRED)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory en/LC_MESSAGES
    COMMAND ${MSGFMT_EXECUTABLE} en.po -o en/LC_MESSAGES/sast-evento.mo
    COMMAND ${CMAKE_COMMAND} -E make_directory zh/LC_MESSAGES
    COMMAND ${MSGFMT_EXECUTABLE} zh.po -o zh/LC_MESSAGES/sast-evento.mo
    WORKING_DIRECTORY ${SOURCE_LOCALE_DIR}
  )
endif()

add_subdirectory(Tray)
