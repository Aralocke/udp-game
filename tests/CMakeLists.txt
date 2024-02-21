project(tests LANGUAGES CXX VERSION 1.0.0)

add_library(tests STATIC)
target_include_directories(tests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB TESTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB TESTS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group("Source Files" FILES ${TESTS_SOURCES})
source_group("Header Files" FILES ${TESTS_HEADERS})

target_sources(tests PRIVATE ${TESTS_HEADERS} ${TESTS_SOURCES})

target_link_libraries(tests
    PUBLIC
        common
)
