# Verify Git installation
if (WIN32)
    # By default, ExternalProject finds the wrong git
    find_package(Git REQUIRED)
    set(GIT_EXECUTABLE "C:/Program Files/Git/cmd/git.exe")
endif()

# Verify Vulkan SDK installation

#message(STATUS "Attempting to auto-locate Vulkan...")
    
# Find Vulkan Path using CMake's Vulkan Module
find_package(Vulkan REQUIRED)

# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

#if(NOT Vulkan_FOUND)
    # CMake may fail to locate the libraries but could be able to 
    # provide some path in Vulkan SDK include directory variable
    # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
#    #message(STATUS "Failed to locate Vulkan SDK, retrying")
#    if(EXISTS "${VULKAN_PATH}")
#       message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
#    else()
#        message("Error: Unable to locate Vulkan SDK")
#        return()
#    endif()
#endif()

# Include necessary Vulkan headers
include_directories(${VULKAN_PATH}/Include)

# Link
target_link_libraries(JoeEngine ${Vulkan_LIBRARIES})

# Grab third party libraries

set(THIRDPARTY_SOURCE_DIR "${CMAKE_BINARY_DIR}/ThirdParty")

include(ExternalProject)

# GLFW
ExternalProject_Add(
  GLFW
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3
  PREFIX         ${THIRDPARTY_SOURCE_DIR}/glfw
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
)
add_library(glfw STATIC IMPORTED GLOBAL)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(JoeEngine glfw)
set(GLFW_INCLUDE_DIR ${CMAKE_BINARY_DIR}/glfw)

# GLM
ExternalProject_Add(
  GLM
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.5
  PREFIX         ${THIRDPARTY_SOURCE_DIR}/glm
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -DGLM_TEST_ENABLE=OFF
)
set(GLM_INCLUDE_DIR ${CMAKE_BINARY_DIR}/glm)

# TinyOBJ
ExternalProject_Add(
  TINYOBJLOADER
  GIT_REPOSITORY https://github.com/syoyo/tinyobjloader.git
  GIT_TAG        v1.0.6
  PREFIX         ${THIRDPARTY_SOURCE_DIR}/tinyobjloader
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
)
set(TINYOBJLOADER_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)

# STB
ExternalProject_Add(
  STB
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        master
  PREFIX         ${THIRDPARTY_SOURCE_DIR}/stb
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
set(STB_INCLUDE_DIR ${THIRDPARTY_SOURCE_DIR}/stb/src/stb)

include_directories("${GLFW_INCLUDE_DIR}")
include_directories("${GLM_INCLUDE_DIR}")
include_directories("${TINYOBJLOADER_INCLUDE_DIR}")
include_directories("${STB_INCLUDE_DIR}")

add_dependencies(JoeEngine GLFW)
add_dependencies(JoeEngine GLM)
add_dependencies(JoeEngine TINYOBJLOADER)
add_dependencies(JoeEngine STB)