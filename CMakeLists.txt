# Project settings
cmake_minimum_required (VERSION 2.4)
project (oit-ll)

# Compilation flags
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
add_definitions(-Wall)

# LIBRARIES
# 		glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE STRING "" FORCE)
set(GLFW_INSTALL OFF CACHE STRING "" FORCE)
add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)

# 		glew
add_definitions(-DGLEW_STATIC)
option(glew-cmake_BUILD_SHARED "" OFF)
add_subdirectory(lib/glew)
include_directories(lib/glew/include lib/glew/src)

# 		glm
include_directories(lib/glm)

# BUILD
file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR}/src/*)
add_executable(main ${source_files})
target_link_libraries(main glfw ${GLFW_LIBRARIES} libglew_static)
