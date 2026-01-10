cmake_minimum_required(VERSION 3.31)
project(OpenGL)

set(CMAKE_CXX_STANDARD 20)

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

# Include GLFW and GLEW headers
include_directories(${GLFW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})

# Explicitly set source files
set(SOURCES src/main.cpp src/textInfo.cpp)

# Verify source files exist
foreach(source_file ${SOURCES})
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${source_file})
        message(FATAL_ERROR "Source file ${source_file} not found")
    endif()
endforeach()

add_executable(OpenGL ${SOURCES})

# Link GLFW libraries with Windows-specific configuration
if(WIN32)
    # For Windows, link against the dynamic library
    target_link_directories(OpenGL PRIVATE ${GLFW_LIB_DIR} ${GLEW_LIB_DIR})
    target_link_libraries(OpenGL ${GLFW_LIB_DIR}/glfw3.dll ${GLEW_LIB_DIR}/glew32.lib)
    
    # Copy the DLLs to the output directory
    add_custom_command(TARGET OpenGL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_LIB_DIR}/glfw3.dll"
        "$<TARGET_FILE_DIR:OpenGL>"
        COMMENT "Copying glfw3.dll to output directory"
    )
    add_custom_command(TARGET OpenGL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/bin/Release/x64/glew32.dll"
        "$<TARGET_FILE_DIR:OpenGL>"
        COMMENT "Copying glew32.dll to output directory"
    )
else()
    # For other platforms, use the static library
    target_link_directories(OpenGL PRIVATE ${GLFW_LIB_DIR})
    target_link_libraries(OpenGL glfw3)
endif()

# Add required Windows libraries
target_link_libraries(OpenGL opengl32 gdi32)
