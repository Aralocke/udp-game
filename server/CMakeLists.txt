project(server LANGUAGES CXX VERSION 1.0.0)

add_executable(server)
target_include_directories(server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB CLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB CLIENT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group("Source Files" FILES ${CLIENT_SOURCES})
source_group("Header Files" FILES ${CLIENT_HEADERS})

target_link_libraries(server PRIVATE common)
target_sources(server PRIVATE ${CLIENT_HEADERS} ${CLIENT_SOURCES})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTY VS_STARTUP_PROJECT server)
