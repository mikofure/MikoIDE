cmake_minimum_required(VERSION 3.19)
project(Hyperion)

# -------------------------------------------------
# General settings
# -------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# -------------------------------------------------
# Dependencies - SDL3
# -------------------------------------------------

set(SDL_SHARED ON  CACHE BOOL "Build SDL3 as shared library")
set(SDL_STATIC OFF CACHE BOOL "Build SDL3 as static library")
set(SDL_TEST   OFF CACHE BOOL "Build SDL3 test programs")

add_subdirectory(external/SDL3)

# -------------------------------------------------
# Dependencies - CEF
# -------------------------------------------------
set(CEF_VERSION "138.0.27+g0b28f18+chromium-138.0.7204.158")

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CEF_PLATFORM "windows64")
    else()
        set(CEF_PLATFORM "windows32")
    endif()
elseif(APPLE)
    set(CEF_PLATFORM "macosx64")
else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CEF_PLATFORM "linux64")
    else()
        set(CEF_PLATFORM "linux32")
    endif()
endif()

set(CEF_DOWNLOAD_URL "https://cef-builds.spotifycdn.com/cef_binary_${CEF_VERSION}_${CEF_PLATFORM}.tar.bz2")
set(CEF_ROOT         "${CMAKE_BINARY_DIR}/cef")
set(CEF_ARCHIVE      "${CMAKE_BINARY_DIR}/cef_binary.tar.bz2")
set(CEF_EXTRACTED_DIR "${CMAKE_BINARY_DIR}/cef_binary_${CEF_VERSION}_${CEF_PLATFORM}")

function(download_cef)
    if(NOT EXISTS "${CEF_ROOT}")
        message(STATUS "Downloading CEF from ${CEF_DOWNLOAD_URL}")

        file(DOWNLOAD "${CEF_DOWNLOAD_URL}" "${CEF_ARCHIVE}"
             SHOW_PROGRESS
             STATUS download_status
             LOG download_log)

        list(GET download_status 0 status_code)
        if(NOT status_code EQUAL 0)
            message(FATAL_ERROR "Failed to download CEF: ${download_log}")
        endif()

        message(STATUS "Extracting CEF...")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xjf "${CEF_ARCHIVE}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            RESULT_VARIABLE extract_result
        )
        if(NOT extract_result EQUAL 0)
            message(FATAL_ERROR "Failed to extract CEF archive")
        endif()

        if(EXISTS "${CEF_ROOT}")
            file(REMOVE_RECURSE "${CEF_ROOT}")
        endif()

        if(WIN32)
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E rename "${CEF_EXTRACTED_DIR}" "${CEF_ROOT}"
                RESULT_VARIABLE rename_result
            )
            if(NOT rename_result EQUAL 0)
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CEF_EXTRACTED_DIR}" "${CEF_ROOT}"
                    RESULT_VARIABLE copy_result
                )
                if(copy_result EQUAL 0)
                    file(REMOVE_RECURSE "${CEF_EXTRACTED_DIR}")
                else()
                    message(FATAL_ERROR "Failed to move CEF directory")
                endif()
            endif()
        else()
            file(RENAME "${CEF_EXTRACTED_DIR}" "${CEF_ROOT}")
        endif()

        file(REMOVE "${CEF_ARCHIVE}")
        message(STATUS "CEF downloaded and extracted successfully")
    else()
        message(STATUS "CEF already exists at ${CEF_ROOT}")
    endif()
endfunction()

download_cef()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

# -------------------------------------------------
# Miniz
# -------------------------------------------------
add_subdirectory(external/miniz)

# -------------------------------------------------
# Output Directories
# -------------------------------------------------
set(CEF_OUT_DIR_RELEASE "${CMAKE_BINARY_DIR}/Release/bin/cef/${CEF_PLATFORM}")
set(CEF_OUT_DIR_DEBUG   "${CMAKE_BINARY_DIR}/Debug/bin/cef/${CEF_PLATFORM}")
set(TOOLS_OUT_DIR_DEBUG "${CMAKE_BINARY_DIR}/Debug/tools")
set(TOOLS_OUT_DIR_RELEASE "${CMAKE_BINARY_DIR}/Release/tools")
set(CORE_BINARY_DEBUG "${CMAKE_BINARY_DIR}/Debug/bin")
set(CORE_BINARY_RELEASE "${CMAKE_BINARY_DIR}/Release/bin")

if(CMAKE_CONFIGURATION_TYPES)
    set(CEF_TARGET_OUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
else()
    set(CEF_TARGET_OUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})
endif()

# -------------------------------------------------
# Logical CEF targets
# -------------------------------------------------
if(OS_WINDOWS)
    add_library(libcef_lib SHARED IMPORTED)
    set_target_properties(libcef_lib PROPERTIES
        IMPORTED_LOCATION          "${CEF_ROOT}/Release/libcef.dll"
        IMPORTED_LOCATION_DEBUG    "${CEF_ROOT}/Debug/libcef.dll"
        IMPORTED_LOCATION_RELEASE  "${CEF_ROOT}/Release/libcef.dll"
        IMPORTED_IMPLIB            "${CEF_ROOT}/Release/libcef.lib"
        IMPORTED_IMPLIB_DEBUG      "${CEF_ROOT}/Debug/libcef.lib"
        IMPORTED_IMPLIB_RELEASE    "${CEF_ROOT}/Release/libcef.lib"
    )
    add_subdirectory("${CEF_ROOT}/libcef_dll" libcef_dll_wrapper)
endif()

# -------------------------------------------------
# Targets
# -------------------------------------------------

if(MSVC)
    add_compile_options(/wd4505) # disable C4505 warning
    add_compile_options(/W3)     # optional: lower warning level
endif()

add_executable(${PROJECT_NAME} WIN32
    app/main.cpp
    app/utils/logger.cpp
    app/client/client.cpp
    app/client/app.cpp
    app/resources/webapp.cpp
    app/resources/editor.cpp
    app/resources/menuoverlay.cpp
    app/resources/resources.cpp
    app/resources/resourceutil.cpp
    app/resources/splash.cpp
    app/terminal/renderer.cpp
    app/terminal/processmanager.cpp
    app/terminal/terminalbuffer.cpp
    app/renderer/dx11_renderer.cpp
    app/internal/simpleipc.cpp
    app/bootstrap/bootstrap.cpp
)

# Add platform-specific renderer sources
if(UNIX AND NOT APPLE)
    target_sources(${PROJECT_NAME} PRIVATE
        app/renderer/linux/vulkan_renderer.cpp
    )
endif()

add_executable(mikowebhelper WIN32
    app/webhelper/main.cpp
    app/utils/logger.cpp
    app/client/app.cpp
)

add_executable(mikoterminal
    app/terminal/renderer.cpp
    app/terminal/processmanager.cpp
    app/terminal/terminalbuffer.cpp
    test/terminal/main.cpp
    test/terminal/windowed.cpp
)

add_executable(termibench
    test/termibench/termibench.c
)
add_executable(hyprn
    app/cli/main.c
    app/cli/core/parser.c
    app/cli/core/file_ops.c
    app/cli/core/workspace.c
    app/cli/core/extensions.c
    app/cli/core/server.c
    app/cli/core/executor.c
)

# -------------------------------------------------
# Resources
# -------------------------------------------------
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE shared/windows/assets/app.rc)
    target_sources(mikowebhelper   PRIVATE shared/windows/assets/webhelper.rc)
endif()

SET_EXECUTABLE_TARGET_PROPERTIES(${PROJECT_NAME})
SET_EXECUTABLE_TARGET_PROPERTIES(mikowebhelper)
SET_EXECUTABLE_TARGET_PROPERTIES(mikoterminal)
SET_EXECUTABLE_TARGET_PROPERTIES(termibench)
SET_EXECUTABLE_TARGET_PROPERTIES(hyprn)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}"
    OUTPUT_NAME "${PROJECT_NAME}"
)

set_target_properties(mikoterminal PROPERTIES
    OUTPUT_NAME "mikoterminal"
)

set_target_properties(hyprn PROPERTIES
    OUTPUT_NAME "hyprn"
    RUNTIME_OUTPUT_DIRECTORY "${CORE_BINARY_DEBUG}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CORE_BINARY_RELEASE}"
)

set_target_properties(termibench PROPERTIES
    OUTPUT_NAME "termibench"
    RUNTIME_OUTPUT_DIRECTORY "${TOOLS_OUT_DIR_DEBUG}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${TOOLS_OUT_DIR_RELEASE}"
)

set_target_properties(mikowebhelper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CEF_OUT_DIR_DEBUG}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CEF_OUT_DIR_RELEASE}"
    OUTPUT_NAME "mikowebhelper"
)

# -------------------------------------------------
# Linking
# -------------------------------------------------
target_link_libraries(${PROJECT_NAME}
    SDL3::SDL3
    SDL3::SDL3-shared
    libcef_lib
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    miniz
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        d2d1 dwrite d3d11 dxgi windowscodecs
        dwmapi shell32 ole32
        msvcrt
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}
        vulkan
    )   
endif()

target_link_libraries(mikowebhelper
    libcef_lib
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
)

target_link_libraries(mikoterminal
    SDL3::SDL3
    SDL3::SDL3-shared
)

if(WIN32)
    target_link_libraries(mikoterminal d2d1 dwrite)
endif()

# -------------------------------------------------
# Includes
# -------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE ${CEF_ROOT})
target_include_directories(mikowebhelper   PRIVATE ${CEF_ROOT})

# -------------------------------------------------
# Post-build
# -------------------------------------------------
if(OS_WINDOWS)
    COPY_FILES("mikowebhelper" "${CEF_BINARY_FILES}"  "${CEF_BINARY_DIR}" "${CEF_OUT_DIR_DEBUG}")
    COPY_FILES("mikowebhelper" "${CEF_BINARY_FILES}"  "${CEF_BINARY_DIR}" "${CEF_OUT_DIR_RELEASE}")
    COPY_FILES("mikowebhelper" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_OUT_DIR_DEBUG}")
    COPY_FILES("mikowebhelper" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_OUT_DIR_RELEASE}")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3-shared>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying SDL3 DLL to output directory"
    )
endif()

# -------------------------------------------------
# Extra
# -------------------------------------------------

if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

PRINT_CEF_CONFIG()
