cmake_minimum_required(VERSION 3.21)
project(udp-game LANGUAGES CXX VERSION 1.0.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(common)
add_subdirectory(tests)
add_subdirectory(client)
add_subdirectory(server)
