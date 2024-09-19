add_executable(${PROJECT_NAME} ${VERSION_RC_PATH})

file(GLOB_RECURSE SOURCES Controller/*.cc Infrastructure/*.cc)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} main.cc)

# hide cmdline in Release Mode for MSVC
if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_HAS_CO_AWAIT)
endif()

slint_target_sources(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/ui/app.slint)

if (CMAKE_CONFIGURATION_TYPES MATCHES "Release")
  message("Embedding resources in the executable")
  set_property(TARGET ${PROJECT_NAME} PROPERTY SLINT_EMBED_RESOURCES embed-files)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

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
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    spdlog::spdlog
    Boost::boost
    Boost::system
    Boost::url
    Boost::beast
    Boost::process
    OpenSSL::Crypto
    OpenSSL::SSL
    nlohmann_json::nlohmann_json
    sast-link
    keychain
    Slint::Slint
    version::version
)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()

add_subdirectory(Tray)
