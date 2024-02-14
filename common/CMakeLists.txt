project(common LANGUAGES CXX VERSION 1.0.0)

add_library(common STATIC)
target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB COMMON_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

source_group("Source Files" FILES ${COMMON_SOURCES})
source_group("Header Files" FILES ${COMMON_HEADERS})

target_sources(common PRIVATE ${COMMON_HEADERS} ${COMMON_SOURCES})

# We need these for winsock so just add them here. Make them PUBLIC
# so that client / server inherits them too.
target_link_libraries(common
    PUBLIC
        bcrypt ws2_32
)

# These definitions should be application wide, again PUBLIC so that
# client/server inherits their values.
target_compile_definitions(common
    PUBLIC
        -DUNICODE=1 -D_UNIICODE=1
        -DNOMINMAX=1
        -DWIN32_LEAN_AND_MEAN=1
        -D_CRT_SECURE_NO_WARNINGS=1
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING=1
)

# Lastly these are some common sense compile options for windows. Adds
# buffer safety and some things like that.
target_compile_options(common
    PUBLIC
        /Z7 /FC /DEBUG /MP /GR- /GS
        /EHs  # disable exceptions
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(common PRIVATE /Od /RTC1)
endif()
