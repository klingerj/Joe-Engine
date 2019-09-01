cmake_minimum_required (VERSION 3.15.1)
project(JoeEngine)
# The version number.
set(JOE_ENGINE_VERSION_MAJOR 1)
set(JOE_ENGINE_VERSION_MINOR 0)

if (WIN32)
    set(JOE_ENGINE_PLATFORM_WINDOWS ON)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(JOE_ENGINE_PLATFORM_APPLE ON)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
    if (COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17")
    else()
        message(FATAL_ERROR "Compiler does not support C++17!")
    endif()
endif()

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/JoeEngineConfig.h.in"
  "${PROJECT_BINARY_DIR}/JoeEngineConfig.h"
  )

# add the binary tree to the search path for include files
# so that we will find JoeEngineConfig.h
include_directories("${PROJECT_BINARY_DIR}")

# Source files
set(SOURCE_LIST
    "Source/main.cpp"
    "Source/EngineApplication.cpp"
    "Source/EngineApplication.h"
    "Source/Io/IOHandler.cpp"
    "Source/Io/IOHandler.h"
    "Source/Physics/PhysicsManager.cpp"
    "Source/Physics/PhysicsManager.h"
    "Source/Rendering/Texture.cpp"
    "Source/Rendering/Texture.h"
    "Source/Rendering/VulkanQueue.cpp"
    "Source/Rendering/VulkanQueue.h"
    "Source/Rendering/VulkanRenderer.cpp"
    "Source/Rendering/VulkanRenderer.h"
    "Source/Rendering/VulkanShader.cpp"
    "Source/Rendering/VulkanShader.h"
    "Source/Rendering/VulkanSwapChain.cpp"
    "Source/Rendering/VulkanSwapChain.h"
    "Source/Rendering/VulkanWindow.cpp"
    "Source/Rendering/VulkanWindow.h"
    "Source/Scene/MeshDataManager.cpp"
    "Source/Scene/MeshDataManager.h"
    "Source/Scene/SceneManager.cpp"
    "Source/Scene/SceneManager.h"
    "Source/Scene/Camera.h"
    "Source/Utils/Common.cpp"
    "Source/Utils/Common.h"
    "Source/Utils/MemAllocUtils.cpp"
    "Source/Utils/MemAllocUtils.h"
    "Source/Utils/ThreadPool.cpp"
    "Source/Utils/ThreadPool.h"
    "Source/Utils/VulkanValidationLayers.cpp"
    "Source/Utils/VulkanValidationLayers.h"
)

add_executable(JoeEngine ${SOURCE_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

include("./ThirdParty/ImportDependencies.cmake")

# Run pre-build scripts
if (WIN32)
    set(MAKE_CMD "${CMAKE_CURRENT_SOURCE_DIR}/Source/Scripts/compileShaders.bat")
else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MAKE_CMD "${CMAKE_CURRENT_SOURCE_DIR}/Source/Scripts/compileShaders.sh")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "pwd")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "ls ../")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "ls ../Source/")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "ls ../Source/Scripts/")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "cat ${CMAKE_CURRENT_BINARY_DIR}/../Source/Scripts/compileShaders.sh")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "cat ../Source/Scripts/compileShaders.sh")
    add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND "chmod +x ${CMAKE_CURRENT_BINARY_DIR}/../Source/Scripts/compileShaders.sh")
endif()
add_custom_command(TARGET JoeEngine PRE_BUILD COMMAND ${MAKE_CMD})

# add the install targets
install (TARGETS JoeEngine DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/JoeEngineConfig.h" DESTINATION include)