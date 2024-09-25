add_executable(${PROJECT_NAME} ${VERSION_RC_PATH})

file(GLOB_RECURSE SOURCES Controller/*.cc Infrastructure/*.cc)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} main.cc)

# hide cli in Release Mode for MSVC
if (MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_HAS_CO_AWAIT)
endif()

# use io_uring on Linux
if (LINUX)
  target_compile_definitions(${PROJECT_NAME} PRIVATE 
    BOOST_ASIO_HAS_IO_URING 
    BOOST_ASIO_DISABLE_EPOLL
  )
  set(URING_LIBRARIES ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/liburing.a)
endif()

if (CMAKE_CONFIGURATION_TYPES MATCHES "Release")
  set(SLINT_GENERATE_COMPILE_UNITS 1)
  set(SLINT_RESOURCES_POLICY "embed-files")
else()
  message("Embedding resources in the executable")  
  set(SLINT_GENERATE_COMPILE_UNITS 10)
  set(SLINT_RESOURCES_POLICY "as-absolute-path")
endif()

slint_target_sources(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/ui/app.slint 
  COMPILATION_UNITS ${UI_UNITS}
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
    LOCALE_DIR="${CMAKE_SOURCE_DIR}/ui/locale"
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
    Intl::Intl
    ${URING_LIBRARIES}
)

# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/ui/locale/en/LC_MESSAGES
  COMMAND msgfmt ${CMAKE_SOURCE_DIR}/ui/locale/en.po -o ${CMAKE_SOURCE_DIR}/ui/locale/en/LC_MESSAGES/sast-evento.mo
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/ui/locale/en/LC_MESSAGES
  COMMAND msgfmt ${CMAKE_SOURCE_DIR}/ui/locale/zh.po -o ${CMAKE_SOURCE_DIR}/ui/locale/zh/LC_MESSAGES/sast-evento.mo
)

add_subdirectory(Tray)
