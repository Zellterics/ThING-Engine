cmake_minimum_required(VERSION 3.10)
project(ThING_Demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(THING_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

add_subdirectory("${THING_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/thing-build")

file(GLOB_RECURSE DEMO_SOURCES CONFIGURE_DEPENDS
    globals.cpp
    UI/*.cpp
    update/*.cpp
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${DEMO_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ThING)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/update
)

if(MINGW OR WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)
else()
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()
