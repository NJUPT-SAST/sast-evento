add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCES *.cc *.h *.hpp)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# hide cmdline in Release Mode for MSVC
if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_HAS_CO_AWAIT)
endif()

if (APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
  )
endif()

slint_target_sources(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/ui/app.slint)

set_property(TARGET ${PROJECT_NAME} PROPERTY SLINT_EMBED_RESOURCES embed-files)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# definitions for Logger
target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    $<$<CONFIG:Debug>:EVENTO_DEBUG>
    $<$<CONFIG:Release>:EVENTO_RELEASE>
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    spdlog::spdlog
    Boost::boost
    Boost::system
    Boost::url
    Boost::beast
    OpenSSL::Crypto
    OpenSSL::SSL
    nlohmann_json::nlohmann_json
    sast-link
    keychain
    Slint::Slint
    version::version
    tray
)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()