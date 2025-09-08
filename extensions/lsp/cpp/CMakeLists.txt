cmake_minimum_required(VERSION 3.20)
project(cpp_lsp_extension)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_DLL "Build as Windows DLL" ON)
option(BUILD_WASM "Build as WebAssembly" OFF)

# LLVM Configuration
include(FetchContent)

FetchContent_Declare(
    llvm_project
    URL https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-21.1.0.tar.gz
    SOURCE_SUBDIR llvm
)

set(LLVM_ENABLE_PROJECTS "clang;clang-tools-extra" CACHE STRING "")
set(LLVM_TARGETS_TO_BUILD "X86" CACHE STRING "")
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "")
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")
set(LLVM_BUILD_TOOLS ON CACHE BOOL "")

# WebAssembly specific settings
if(BUILD_WASM)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/emscripten.cmake")
    set(LLVM_TARGETS_TO_BUILD "WebAssembly" CACHE STRING "" FORCE)
endif()

FetchContent_MakeAvailable(llvm_project)

# Set clangd executable path
set(CLANGD_EXECUTABLE "${CMAKE_BINARY_DIR}/bin/clangd${CMAKE_EXECUTABLE_SUFFIX}")

# LSP Wrapper Library
set(LSP_WRAPPER_SOURCES
    src/lsp_wrapper.cpp
    src/lsp_wrapper.hpp
)

# Create the LSP wrapper library
if(BUILD_DLL)
    add_library(cpp_lsp_wrapper SHARED ${LSP_WRAPPER_SOURCES})
    target_compile_definitions(cpp_lsp_wrapper PRIVATE LSP_WRAPPER_EXPORTS)
    set_target_properties(cpp_lsp_wrapper PROPERTIES
        OUTPUT_NAME "cpp_lsp_wrapper"
        SUFFIX ".dll"
        PREFIX ""
        LINK_DEF_FILE_FLAG "/DEF:"
    )
    # Add DLL export definition file for Windows
    if(WIN32)
        set_target_properties(cpp_lsp_wrapper PROPERTIES
            LINK_FLAGS "/DEF:${CMAKE_SOURCE_DIR}/src/cpp_lsp_wrapper.def"
        )
    endif()
elseif(BUILD_WASM)
    add_library(cpp_lsp_wrapper STATIC ${LSP_WRAPPER_SOURCES})
    set_target_properties(cpp_lsp_wrapper PROPERTIES
        OUTPUT_NAME "cpp_lsp_wrapper"
        SUFFIX ".wasm"
        PREFIX ""
    )
    target_link_options(cpp_lsp_wrapper PRIVATE
        -sEXPORTED_FUNCTIONS=['_lsp_create_server','_lsp_destroy_server','_lsp_initialize','_lsp_shutdown','_lsp_did_open','_lsp_did_change','_lsp_did_save','_lsp_did_close','_lsp_completion','_lsp_hover','_lsp_definition','_lsp_references','_lsp_diagnostics','_lsp_format_document','_lsp_format_range','_lsp_process_message','_lsp_free_string']
        -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
        -sMODULARIZE=1
        -sEXPORT_NAME=LSPModule
        -sALLOW_MEMORY_GROWTH=1
        -sSTACK_SIZE=1MB
        -sINITIAL_MEMORY=16MB
    )
else()
    add_library(cpp_lsp_wrapper STATIC ${LSP_WRAPPER_SOURCES})
endif()

# Link LLVM/Clang libraries
target_link_libraries(cpp_lsp_wrapper PRIVATE
    clangTooling
    clangFrontend
    clangDriver
    clangSerialization
    clangCodeGen
    clangParse
    clangSema
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    clangAnalysis
    clangARCMigrate
    clangRewrite
    clangRewriteFrontend
    clangEdit
    clangAST
    clangLex
    clangBasic
    LLVMSupport
    LLVMMC
    LLVMBitReader
    LLVMOption
    LLVMProfileData
)

# Include directories
target_include_directories(cpp_lsp_wrapper PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${llvm_project_SOURCE_DIR}/clang/include
    ${llvm_project_BINARY_DIR}/clang/include
    ${llvm_project_SOURCE_DIR}/llvm/include
    ${llvm_project_BINARY_DIR}/llvm/include
)

# Generate configuration files
configure_file(
    "${CMAKE_SOURCE_DIR}/config/cpp_lsp_config.json.in"
    "${CMAKE_BINARY_DIR}/cpp_lsp_config.json"
    @ONLY
)

configure_file(
    "${CMAKE_SOURCE_DIR}/config/clangd_config.yaml.in"
    "${CMAKE_BINARY_DIR}/clangd_config.yaml"
    @ONLY
)

configure_file(
    "${CMAKE_SOURCE_DIR}/config/compile_commands_template.json.in"
    "${CMAKE_BINARY_DIR}/compile_commands_template.json"
    @ONLY
)

# Custom target to ensure clangd is built
add_custom_target(build_clangd ALL
    DEPENDS clangd
    COMMENT "Building clangd from LLVM source"
)

# Make sure LSP wrapper depends on clangd
add_dependencies(cpp_lsp_wrapper build_clangd)

# Print completion message
if(BUILD_DLL)
    message(STATUS "C++ LSP extension configured for DLL build")
elseif(BUILD_WASM)
    message(STATUS "C++ LSP extension configured for WASM build")
else()
    message(STATUS "C++ LSP extension configured for static library build")
endif()
message(STATUS "C++ LSP extension configuration completed successfully")