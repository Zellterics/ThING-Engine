cmake_minimum_required(VERSION 3.10)
project(VulkanGLFWProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)
option(GLFW_BUILD_SHARED_LIBS OFF)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_compile_definitions(DEBUG=true)

include_directories(
    external/glfw/include
    external/imgui
    external
    app
    app/UI
    app/update
)


if(WIN32)
    message(STATUS "WINDOWS CONFIG")
    include_directories("$ENV{VULKAN_SDK}/Include")
    link_directories("$ENV{VULKAN_SDK}/Lib")
    set(VULKAN_LIB vulkan-1)
else()
    message(STATUS "LINUX CONFIG")
    find_package(Vulkan REQUIRED)
    set(VULKAN_LIB Vulkan::Vulkan)
endif()

file(GLOB_RECURSE APP_SOURCES
    app/*.cpp
    core/*.cpp
)

file(GLOB EXTRAS_SOURCES
    extras/*.cpp
)

# Ejecutable
add_executable(${PROJECT_NAME} 
    main.cpp
    ${CORE_SOURCES}
    ${EXTRAS_SOURCES}
    ${APP_SOURCES}
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MINGW OR WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
else()
    target_link_options(${PROJECT_NAME} PRIVATE
        # -static LINUX DOESN'T LIKE THIS
        -static-libgcc
        -static-libstdc++
    )
endif()

# LINK
if(MINGW OR WIN32)
    target_link_libraries(${PROJECT_NAME}
        glfw
        glm
        ${VULKAN_LIB}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        glfw
        X11
        glm
        ${VULKAN_LIB}
    )
endif()
