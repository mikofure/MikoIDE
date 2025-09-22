cmake_minimum_required(VERSION 3.20)
project(miko_c_lsp)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_DLL "Build as Windows DLL" ON)
option(BUILD_WASM "Build as WebAssembly" OFF)

# Include FetchContent for downloading LLVM
include(FetchContent)

# Download and build LLVM 21.1.0
FetchContent_Declare(
    llvm_project
    URL https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-21.1.0.tar.gz
    SOURCE_SUBDIR llvm
)

# Configure LLVM build options
set(LLVM_ENABLE_PROJECTS "clang;clang-tools-extra" CACHE STRING "")
set(LLVM_TARGETS_TO_BUILD "X86" CACHE STRING "")
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "")
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "")
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")
set(LLVM_BUILD_TOOLS ON CACHE BOOL "")
set(CLANG_BUILD_TOOLS ON CACHE BOOL "")

# WebAssembly specific settings
if(BUILD_WASM)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/emscripten.cmake")
    set(LLVM_TARGETS_TO_BUILD "WebAssembly" CACHE STRING "" FORCE)
endif()

message(STATUS "Downloading and configuring LLVM 21.1.0...")
FetchContent_MakeAvailable(llvm_project)

# Set clangd executable path
set(CLANGD_EXECUTABLE "${CMAKE_BINARY_DIR}/_deps/llvm_project-build/bin/clangd${CMAKE_EXECUTABLE_SUFFIX}")
message(STATUS "LLVM clangd will be built at: ${CLANGD_EXECUTABLE}")

# LSP Wrapper Library
set(LSP_WRAPPER_SOURCES
    src/lsp_wrapper.cpp
    src/lsp_wrapper.hpp
)

# Create the LSP wrapper library
if(BUILD_DLL)
    add_library(c_lsp_wrapper SHARED ${LSP_WRAPPER_SOURCES})
    target_compile_definitions(c_lsp_wrapper PRIVATE LSP_WRAPPER_EXPORTS)
    set_target_properties(c_lsp_wrapper PROPERTIES
        OUTPUT_NAME "c_lsp_wrapper"
        SUFFIX ".dll"
        PREFIX ""
        LINK_DEF_FILE_FLAG "/DEF:"
    )
    # Add DLL export definition file for Windows
    if(WIN32)
        set_target_properties(c_lsp_wrapper PROPERTIES
            LINK_FLAGS "/DEF:${CMAKE_SOURCE_DIR}/src/c_lsp_wrapper.def"
        )
    endif()
elseif(BUILD_WASM)
    add_library(c_lsp_wrapper STATIC ${LSP_WRAPPER_SOURCES})
    set_target_properties(c_lsp_wrapper PROPERTIES
        OUTPUT_NAME "c_lsp_wrapper"
        SUFFIX ".wasm"
        PREFIX ""
    )
    target_link_options(c_lsp_wrapper PRIVATE
        -sEXPORTED_FUNCTIONS=['_lsp_create_server','_lsp_destroy_server','_lsp_initialize','_lsp_shutdown','_lsp_did_open','_lsp_did_change','_lsp_did_save','_lsp_did_close','_lsp_completion','_lsp_hover','_lsp_definition','_lsp_references','_lsp_diagnostics','_lsp_format_document','_lsp_format_range','_lsp_process_message','_lsp_free_string']
        -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
        -sMODULARIZE=1
        -sEXPORT_NAME=LSPModule
        -sALLOW_MEMORY_GROWTH=1
        -sSTACK_SIZE=1MB
        -sINITIAL_MEMORY=16MB
    )
else()
    add_library(c_lsp_wrapper STATIC ${LSP_WRAPPER_SOURCES})
endif()

# Link LLVM/Clang libraries
target_link_libraries(c_lsp_wrapper PRIVATE
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
target_include_directories(c_lsp_wrapper PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${llvm_project_SOURCE_DIR}/clang/include
    ${llvm_project_BINARY_DIR}/clang/include
    ${llvm_project_SOURCE_DIR}/llvm/include
    ${llvm_project_BINARY_DIR}/llvm/include
)

# Custom target to ensure clangd is built
add_custom_target(build_clangd ALL
    DEPENDS clangd
    COMMENT "Building clangd from LLVM source"
)

# Make sure LSP wrapper depends on clangd
add_dependencies(c_lsp_wrapper build_clangd)

# Create C LSP extension configuration
set(C_LSP_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/c_lsp_config.json")
file(WRITE ${C_LSP_CONFIG_FILE} "{
  \"name\": \"miko-c-lsp\",
  \"version\": \"1.0.0\",
  \"description\": \"C Language Server Protocol extension for Miko IDE\",
  \"language\": \"c\",
  \"server\": {
    \"executable\": \"${CLANGD_EXECUTABLE}\",
    \"args\": [
      \"--background-index\",
      \"--clang-tidy\",
      \"--completion-style=detailed\",
      \"--header-insertion=iwyu\",
      \"--pch-storage=memory\",
      \"--log=error\"
    ],
    \"initializationOptions\": {
      \"clangdFileStatus\": true,
      \"usePlaceholders\": true,
      \"completeUnimported\": true,
      \"semanticHighlighting\": true
    }
  },
  \"fileExtensions\": [\"c\", \"h\"],
  \"capabilities\": {
    \"textDocumentSync\": 2,
    \"completionProvider\": {
      \"resolveProvider\": true,
      \"triggerCharacters\": [\".\", \"->\", \":\", \"<\"]
    },
    \"hoverProvider\": true,
    \"signatureHelpProvider\": {
      \"triggerCharacters\": [\"(\", \",\"]
    },
    \"definitionProvider\": true,
    \"referencesProvider\": true,
    \"documentHighlightProvider\": true,
    \"documentSymbolProvider\": true,
    \"workspaceSymbolProvider\": true,
    \"codeActionProvider\": true,
    \"documentFormattingProvider\": true,
    \"documentRangeFormattingProvider\": true,
    \"renameProvider\": true,
    \"foldingRangeProvider\": true,
    \"semanticTokensProvider\": {
      \"legend\": {
        \"tokenTypes\": [\"namespace\", \"type\", \"class\", \"enum\", \"interface\", \"struct\", \"typeParameter\", \"parameter\", \"variable\", \"property\", \"enumMember\", \"event\", \"function\", \"method\", \"macro\", \"keyword\", \"modifier\", \"comment\", \"string\", \"number\", \"regexp\", \"operator\"],
        \"tokenModifiers\": [\"declaration\", \"definition\", \"readonly\", \"static\", \"deprecated\", \"abstract\", \"async\", \"modification\", \"documentation\", \"defaultLibrary\"]
      },
      \"range\": true,
      \"full\": {
        \"delta\": true
      }
    }
  }
}")

# Install configuration file
install(FILES ${C_LSP_CONFIG_FILE} DESTINATION extensions/lsp/c/)

# Create compile_commands.json template
set(COMPILE_COMMANDS_TEMPLATE "${CMAKE_CURRENT_BINARY_DIR}/compile_commands_template.json")
file(WRITE ${COMPILE_COMMANDS_TEMPLATE} "[
  {
    \"directory\": \"/path/to/project\",
    \"command\": \"clang -I/usr/include -std=c11 -Wall -Wextra -c file.c\",
    \"file\": \"file.c\"
  }
]")

install(FILES ${COMPILE_COMMANDS_TEMPLATE} DESTINATION extensions/lsp/c/)

# Create .clangd configuration template
set(CLANGD_CONFIG_TEMPLATE "${CMAKE_CURRENT_BINARY_DIR}/clangd_config.yaml")
file(WRITE ${CLANGD_CONFIG_TEMPLATE} "CompileFlags:
  Add:
    - -std=c11
    - -Wall
    - -Wextra
    - -Wpedantic
  Remove:
    - -W*
    - -f*

Index:
  Background: Build
  StandardLibrary: Yes

Completion:
  AllScopes: Yes

Hover:
  ShowAKA: Yes

InlayHints:
  Enabled: Yes
  ParameterNames: Yes
  DeducedTypes: Yes

Diagnostics:
  ClangTidy:
    Add:
      - readability-*
      - performance-*
      - modernize-*
      - bugprone-*
    Remove:
      - modernize-use-trailing-return-type
")

install(FILES ${CLANGD_CONFIG_TEMPLATE} DESTINATION extensions/lsp/c/)

# Configuration complete
if(BUILD_DLL)
    message(STATUS "C LSP extension configured for DLL build")
elseif(BUILD_WASM)
    message(STATUS "C LSP extension configured for WASM build")
else()
    message(STATUS "C LSP extension configured for static library build")
endif()
message(STATUS "C LSP extension configuration completed successfully")