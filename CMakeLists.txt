cmake_minimum_required(VERSION 3.10)
project(ThING LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

if(TARGET glm::glm)
    set(THING_GLM_TARGET glm::glm)
elseif(TARGET glm)
    set(THING_GLM_TARGET glm)
else()
    message(FATAL_ERROR "There's no (glm::glm o glm).")
endif()

if(WIN32)
    message(STATUS "WINDOWS CONFIG")
    include_directories("$ENV{VULKAN_SDK}/Include")
    link_directories("$ENV{VULKAN_SDK}/Lib")
    set(THING_VULKAN_TARGET vulkan-1)
else()
    message(STATUS "LINUX CONFIG")
    find_package(Vulkan REQUIRED)
    set(THING_VULKAN_TARGET Vulkan::Vulkan)
endif()

# =========================================================
# SHADERS: compile (optional) + embed to generated headers
# Tool: shaders/embed.cpp
# Output headers: build/generated_shaders/*.h
# =========================================================
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_shaders)
file(MAKE_DIRECTORY ${SHADER_GEN_DIR})

# Build embed tool
add_executable(thing_embed_spirv ${SHADER_SRC_DIR}/embed.cpp)
set_target_properties(thing_embed_spirv PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tools
)

# Find shader compiler (glslc preferred, fallback glslangValidator)
find_program(GLSLC glslc)
find_program(GLSLANG_VALIDATOR glslangValidator)

if(GLSLC)
    set(SPIRV_COMPILER_KIND "glslc")
elseif(GLSLANG_VALIDATOR)
    set(SPIRV_COMPILER_KIND "glslangValidator")
else()
    message(FATAL_ERROR "No encontré glslc ni glslangValidator. Instala shaderc o glslang-tools.")
endif()

function(thing_compile_to_spv OUT_SPV IN_SRC)
    if(SPIRV_COMPILER_KIND STREQUAL "glslc")
        add_custom_command(
            OUTPUT  ${OUT_SPV}
            COMMAND ${GLSLC} ${IN_SRC} -o ${OUT_SPV}
            DEPENDS ${IN_SRC}
            COMMENT "Compiling GLSL -> SPIR-V: ${IN_SRC}"
            VERBATIM
        )
    else()
        add_custom_command(
            OUTPUT  ${OUT_SPV}
            COMMAND ${GLSLANG_VALIDATOR} -V ${IN_SRC} -o ${OUT_SPV}
            DEPENDS ${IN_SRC}
            COMMENT "Compiling GLSL -> SPIR-V: ${IN_SRC}"
            VERBATIM
        )
    endif()
endfunction()

function(thing_embed_spv OUT_H IN_SPV SYMBOL)
    add_custom_command(
        OUTPUT  ${OUT_H}
        COMMAND $<TARGET_FILE:thing_embed_spirv> ${IN_SPV} ${OUT_H} ${SYMBOL} ThING::shaders 8
        DEPENDS thing_embed_spirv ${IN_SPV}
        COMMENT "Embedding SPIR-V into header: ${IN_SPV}"
        VERBATIM
    )
endfunction()

# Usage:
#   thing_add_shader(<baseName> <glslExt> <symbol>)
# Examples:
#   thing_add_shader(basicVert vert basicVertSpv)
#   thing_add_shader(basicFrag frag basicFragSpv)
#   thing_add_shader(jfaComp  comp jfaCompSpv)
#
# Behavior:
#   - If shaders/<baseName>.<glslExt> exists -> compile to build/generated_shaders/<baseName>.spv
#   - Else uses shaders/<baseName>.spv
#   - Then embeds to build/generated_shaders/<baseName>_spv.h
#   - Adds the generated header to THING_SHADER_HEADERS (parent scope)
function(thing_add_shader BASE_NAME GLSL_EXT SYMBOL)
    set(GLSL_PATH ${SHADER_SRC_DIR}/${BASE_NAME}.${GLSL_EXT})
    set(SPV_OUT   ${SHADER_GEN_DIR}/${BASE_NAME}.spv)

    if(EXISTS ${GLSL_PATH})
        thing_compile_to_spv(${SPV_OUT} ${GLSL_PATH})
        set(SPV_IN ${SPV_OUT})
    else()
        set(SPV_IN ${SHADER_SRC_DIR}/${BASE_NAME}.spv)
    endif()

    set(H_OUT ${SHADER_GEN_DIR}/${BASE_NAME}_spv.h)
    thing_embed_spv(${H_OUT} ${SPV_IN} ${SYMBOL})

    set(THING_SHADER_HEADERS ${THING_SHADER_HEADERS} ${H_OUT} PARENT_SCOPE)
endfunction()

set(THING_SHADER_HEADERS "")
thing_add_shader(basicVert vert basicVertSpv)
thing_add_shader(basicFrag frag basicFragSpv)
thing_add_shader(jfaComp   comp jfaCompSpv)
thing_add_shader(postVert  vert postVertSpv)
thing_add_shader(postFrag  frag postFragSpv)

add_custom_target(ThING_Shaders ALL
    DEPENDS ${THING_SHADER_HEADERS}
)

file(GLOB_RECURSE THING_SOURCES CONFIGURE_DEPENDS
    core/*.cpp
    extras/*.cpp
)

set(THING_IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

set(THING_IMGUI_SOURCES
    ${THING_IMGUI_DIR}/imgui.cpp
    ${THING_IMGUI_DIR}/imgui_draw.cpp
    ${THING_IMGUI_DIR}/imgui_widgets.cpp
    ${THING_IMGUI_DIR}/imgui_tables.cpp
    ${THING_IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${THING_IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(ThING STATIC
    ${THING_SOURCES}
    ${THING_IMGUI_SOURCES}
)

add_dependencies(ThING ThING_Shaders)
target_include_directories(ThING PRIVATE ${SHADER_GEN_DIR})

target_compile_features(ThING PUBLIC cxx_std_23)
target_compile_definitions(ThING PUBLIC $<$<CONFIG:Debug>:DEBUG>)

target_include_directories(ThING
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external
        ${THING_IMGUI_DIR}
        ${THING_IMGUI_DIR}/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/external/miniaudio
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ThING
    PUBLIC
        glfw
        ${THING_GLM_TARGET}
        ${THING_VULKAN_TARGET}
)
