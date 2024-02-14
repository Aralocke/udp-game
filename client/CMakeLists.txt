project(client LANGUAGES CXX VERSION 1.0.0)

add_executable(client)
target_include_directories(client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB CLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB CLIENT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group("Source Files" FILES ${CLIENT_SOURCES})
source_group("Header Files" FILES ${CLIENT_HEADERS})

target_link_libraries(client PRIVATE common)
target_sources(client PRIVATE ${CLIENT_HEADERS} ${CLIENT_SOURCES})
