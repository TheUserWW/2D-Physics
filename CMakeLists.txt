cmake_minimum_required(VERSION 3.31)
project(2DPhysics)

set(CMAKE_CXX_STANDARD 23)

# Set optimization level to O2 for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

# Enable static linking for Windows to avoid DLL dependencies
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
endif()

# Enable C++20 modules support for GCC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
endif()

# Set GLFW dependencies paths
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Dependencies/include")
set(GLFW_LIB_DIR "${CMAKE_SOURCE_DIR}/Dependencies/lib-mingw-w64")

# Set GLEW dependencies paths
set(GLEW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/include")
set(GLEW_LIB_DIR "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64")

# Set ImGui dependencies paths
set(IMGUI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Dependencies/include/imgui")
set(IMGUI_BACKENDS_DIR "${CMAKE_SOURCE_DIR}/Dependencies/include/imgui/backends")

# Set cPhysics dependencies path
set(CPHYSICS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/Dependencies/cPhysics/include")
set(CPHYSICS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Dependencies/cPhysics/src")

# Include GLFW, GLEW, ImGui and cPhysics headers
include_directories(${GLFW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR} ${IMGUI_BACKENDS_DIR} ${CPHYSICS_INCLUDE_DIR})

# Add ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_INCLUDE_DIR}/imgui.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_demo.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp
    ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
)

# Add cPhysics source files
set(CPHYSICS_SOURCES
    ${CPHYSICS_SOURCE_DIR}/field.c
    ${CPHYSICS_SOURCE_DIR}/entity.c
    ${CPHYSICS_SOURCE_DIR}/plog.c
)

# Check if ImGui source files exist
foreach(IMGUI_SOURCE ${IMGUI_SOURCES})
    if(NOT EXISTS ${IMGUI_SOURCE})
        message(WARNING "ImGui source file not found: ${IMGUI_SOURCE}")
    endif()
endforeach()

# Set project sources
set(SOURCES src/main.cpp ${IMGUI_SOURCES} ${CPHYSICS_SOURCES} 2DPhysics.rc)

add_executable(2DPhysics ${SOURCES})

# Set resource file working directory to find icon files
set_source_files_properties(2DPhysics.rc PROPERTIES
    COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}"
)

# 设置为Windows子系统，隐藏控制台窗口
set_target_properties(2DPhysics PROPERTIES WIN32_EXECUTABLE TRUE)

# Add GLFW library directory
target_link_directories(2DPhysics PRIVATE ${GLFW_LIB_DIR})

# Link GLEW library with Windows-specific configuration
if(WIN32)
    # For Windows, link against the static library for pure static compilation
    target_link_directories(2DPhysics PRIVATE ${GLEW_LIB_DIR})
    target_link_libraries(2DPhysics ${GLEW_LIB_DIR}/glew32s.lib)
    
    # Link GLFW and other required libraries
    target_link_libraries(2DPhysics ${GLFW_LIB_DIR}/libglfw3.a opengl32 gdi32 stdc++exp)
    
    # Add static compilation flags for GLEW
    target_compile_definitions(2DPhysics PRIVATE GLEW_STATIC)
    
    # Check for GLFW library and provide download instructions if missing
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/Dependencies/lib-mingw-w64/libglfw3.a")
        message(WARNING "GLFW library files are missing. Please download GLFW binaries from https://www.glfw.org/download.html")
        message(STATUS "For Windows, download the MinGW-w64 binaries and extract to Dependencies/lib-mingw-w64/")
    endif()
    
    # Check for GLEW static library
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib")
        message(WARNING "GLEW static library (glew32s.lib) is required for static compilation")
    endif()
else()
    # For other platforms, use the static library
    target_link_libraries(2DPhysics glfw)
endif()
