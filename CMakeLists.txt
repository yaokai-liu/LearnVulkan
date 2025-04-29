cmake_minimum_required(VERSION 3.28)
project(VulkanDemo C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output)

set(XGL_SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(XGL_SHADER_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)

set(XGL_DEFAULT_VERT_SHADER ${XGL_SHADER_OUTPUT_DIR}/vert.spv)
set(XGL_DEFAULT_FRAG_SHADER ${XGL_SHADER_OUTPUT_DIR}/frag.spv)
if(NOT EXISTS ${XGL_SHADER_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${XGL_SHADER_OUTPUT_DIR})
endif ()

if(NOT EXISTS ${XGL_DEFAULT_VERT_SHADER})
    message(STATUS "${XGL_DEFAULT_VERT_SHADER} doesn't exist, compile it.")
    execute_process(COMMAND
            glslc
            -fshader-stage=vertex --target-env=vulkan1.3 --target-spv=spv1.3
            ${XGL_SHADER_SOURCE_DIR}/vert-default.glsl
            -o ${XGL_DEFAULT_VERT_SHADER}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(STATUS "${XGL_DEFAULT_VERT_SHADER} exists, skip to compile it.")
endif()

if(NOT EXISTS ${XGL_DEFAULT_FRAG_SHADER})
    message(STATUS "${XGL_DEFAULT_FRAG_SHADER} doesn't exist, compile it.")
    execute_process(COMMAND
            glslc
            -fshader-stage=fragment --target-env=vulkan1.3 --target-spv=spv1.3
            ${XGL_SHADER_SOURCE_DIR}/frag-default.glsl
            -o ${XGL_DEFAULT_FRAG_SHADER}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(STATUS "${XGL_DEFAULT_FRAG_SHADER} exists, skip to compile it.")
endif()

include_directories(meman)
aux_source_directory(meman MEMAN_SRC)
add_library(meman STATIC ${MEMAN_SRC})

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)
include_directories(glfw/include)

add_executable(VulkanDemo main.c)
target_link_libraries(VulkanDemo PRIVATE meman glfw Vulkan::Vulkan)