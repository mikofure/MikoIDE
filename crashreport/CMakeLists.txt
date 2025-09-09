cmake_minimum_required(VERSION 3.25)
project(crashreport)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Determine the target output directory based on platform and generator
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generators (Visual Studio, Xcode)
    set(CRASHPAD_TARGET_OUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")
else()
    # Single-config generators (Ninja, Make)
    set(CRASHPAD_TARGET_OUT_DIR "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

# Create crashpad_handler executable
add_executable(crashpad_handler
    src/main.cpp
    src/crash_handler.cpp
    src/minidump_writer.cpp
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(crashpad_handler
        PRIVATE
        dbghelp
        psapi
        version
        wininet
        ws2_32
    )
endif()

# Set target properties
set_target_properties(crashpad_handler PROPERTIES
    OUTPUT_NAME "crashpad_handler"
    RUNTIME_OUTPUT_DIRECTORY "${CRASHPAD_TARGET_OUT_DIR}"
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(crashpad_handler PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

# Set compiler flags
if(MSVC)
    target_compile_options(crashpad_handler PRIVATE
        /W3
        /permissive-
    )
else()
    target_compile_options(crashpad_handler PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()