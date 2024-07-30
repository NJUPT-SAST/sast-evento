file(GLOB_RECURSE SOURCES *.cc *.h *.hpp)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog::spdlog
    Boost::boost
    Boost::system
    Boost::url
    Boost::beast
    OpenSSL::Crypto
    OpenSSL::SSL
    nlohmann_json::nlohmann_json
    sast-link
    Slint::Slint
    version::version
)

target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_HAS_CO_AWAIT)