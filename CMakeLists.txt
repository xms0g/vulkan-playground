cmake_minimum_required(VERSION 3.23)
project(abra CXX C)

set(CMAKE_CXX_STANDARD 20)

set(PROFILE FALSE)
if (PROFILE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    if (APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-omit-leaf-frame-pointer")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-omit-leaf-frame-pointer")
        endif ()
    endif ()
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
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})
