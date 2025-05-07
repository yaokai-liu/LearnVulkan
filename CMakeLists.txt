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

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
    add_compile_definitions("XGL_SURFACE_WAYLAND")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    add_compile_definitions("XGL_SURFACE_WIN32")
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    MESSAGE(STATUS "current platform: Mac OS X")
else ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
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

add_compile_definitions("XGL_MATH_DEBUG_PRINT")

add_subdirectory(xglm)
include_directories(xglm/src)

find_package(Vulkan REQUIRED)

add_compile_definitions(XGL_WM_USING_GLFW)
add_subdirectory(glfw)
include_directories(glfw/include)

include_directories(src)
aux_source_directory(src SRC)
add_executable(VulkanDemo main.c ${SRC})
target_link_libraries(VulkanDemo PRIVATE xglm meman glfw Vulkan::Vulkan)