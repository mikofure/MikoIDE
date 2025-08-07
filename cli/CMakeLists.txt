cmake_minimum_required(VERSION 3.10)
project(mikoide-cli VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DDEBUG")
endif()

# Source files
set(SOURCES
    src/miko.c
)

# Create executable
add_executable(miko ${SOURCES})

# Set output name
set_target_properties(miko PROPERTIES
    OUTPUT_NAME "mikoide"
)

# Platform-specific settings
if(WIN32)
    # Windows specific settings
    target_compile_definitions(miko PRIVATE _WIN32)
    
    # Link with Windows libraries if needed
    # target_link_libraries(miko PRIVATE kernel32 user32)
else()
    # Unix/Linux specific settings
    target_compile_definitions(miko PRIVATE _GNU_SOURCE)
endif()

# Installation
install(TARGETS miko
    RUNTIME DESTINATION bin
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "mikoide-cli")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MikoIDE Command Line Interface")
set(CPACK_PACKAGE_VENDOR "MikoIDE")
set(CPACK_PACKAGE_CONTACT "contact@mikoide.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)

# Testing (optional)
enable_testing()

# Add a simple test
add_test(NAME version_test
    COMMAND miko --version
)

add_test(NAME help_test
    COMMAND miko --help
)

# Custom targets
add_custom_target(format
    COMMAND clang-format -i ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting source code"
)

add_custom_target(lint
    COMMAND cppcheck --enable=all --std=c99 ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running static analysis"
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C flags: ${CMAKE_C_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug flags: ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release flags: ${CMAKE_C_FLAGS_RELEASE}")
endif()