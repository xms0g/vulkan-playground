cmake_minimum_required(VERSION 3.23)
project(abra CXX C)

set(CMAKE_CXX_STANDARD 20)

if (APPLE)
    add_compile_options(-gdwarf-4)
    add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
endif ()

find_package(glfw3 3.2 REQUIRED)
if (glfw3_FOUND)
    include_directories(${glfw3_INCLUDE_DIRS})
endif (glfw3_FOUND)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
endif (Vulkan_FOUND)

set(FILESYSTEM
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/filesystem/filesystem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/filesystem/filesystem.h)

file(GLOB_RECURSE SRC "src/*")
set(COMPILATION_UNIT ${SRC} ${FILESYSTEM})

add_executable(${PROJECT_NAME} ${COMPILATION_UNIT})
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan)
