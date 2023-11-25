cmake_minimum_required(VERSION 3.23)
project(vkREngine CXX C)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.2 REQUIRED)
if (glfw3_FOUND)
    include_directories(${glfw3_INCLUDE_DIRS})
endif (glfw3_FOUND)

find_package (Vulkan REQUIRED)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
endif (Vulkan_FOUND)

file(GLOB_RECURSE SRC "src/*")

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})
