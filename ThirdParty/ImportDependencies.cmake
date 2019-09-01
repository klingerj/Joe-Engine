# Verify Git installation
if (WIN32)
    find_package(Git REQUIRED)
    set(GIT_EXECUTABLE "C:/Program Files/Git/cmd/git.exe")
endif()

# Verify Vulkan SDK installation
message(STATUS "Attempting to auto-locate Vulkan...")
    
# Find Vulkan Path using CMake's Vulkan Module
find_package(Vulkan REQUIRED)

# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

message(${VULKAN_PATH})

# Include necessary Vulkan headers and link
if (WIN32)
    include_directories(${VULKAN_PATH}/Include)
else()
    include_directories(${VULKAN_PATH})
endif()
target_link_libraries(JoeEngine ${Vulkan_LIBRARIES})

# Fetch third party libraries
set(THIRDPARTY_SOURCE_DIR "${CMAKE_BINARY_DIR}/ThirdParty")
include(ExternalProject)

# GLFW
ExternalProject_Add(
  GLFW
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3
  PREFIX         ${THIRDPARTY_SOURCE_DIR}/glfw
  BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/lib/libglfw3.a
  BUILD_COMMAND  ""
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR} -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
)
add_library(glfw STATIC IMPORTED GLOBAL)
set_target_properties(glfw PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})
target_link_libraries(JoeEngine glfw)
set(GLFW_INCLUDE_DIR ${CMAKE_BINARY_DIR}/glfw)

# If on macOS, import additional dependencies for GLFW
# Should be located in /System/Library/Frameworks/
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(COCOA Cocoa)
    if (NOT COCOA)
        message(FATAL_ERROR "Cocoa framework not found")
    endif()
    target_link_libraries(JoeEngine ${COCOA})
    
    find_library(IOKIT IOKit)
    if (NOT IOKIT)
        message(FATAL_ERROR "IOKit framework not found")
    endif()
    target_link_libraries(JoeEngine ${IOKIT})
    
    find_library(CORE_VIDEO CoreVideo)
    if (NOT CORE_VIDEO)
        message(FATAL_ERROR "CoreVideo framework not found")
    endif()
    target_link_libraries(JoeEngine ${CORE_VIDEO})
    
    find_library(OPENGL OpenGL)
    if (NOT OPENGL)
        message(FATAL_ERROR "OpenGL framework not found")
    endif()
    target_link_libraries(JoeEngine ${OPENGL})
endif()

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