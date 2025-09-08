# Emscripten toolchain file for WebAssembly builds

set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

# Find Emscripten SDK
if(DEFINED ENV{EMSDK})
    set(EMSCRIPTEN_ROOT_PATH $ENV{EMSDK}/upstream/emscripten)
elseif(EXISTS "/usr/lib/emscripten")
    set(EMSCRIPTEN_ROOT_PATH "/usr/lib/emscripten")
elseif(EXISTS "/usr/local/lib/emscripten")
    set(EMSCRIPTEN_ROOT_PATH "/usr/local/lib/emscripten")
else()
    message(FATAL_ERROR "Could not find Emscripten. Please install Emscripten SDK and set EMSDK environment variable.")
endif()

# Set compilers
set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++")
set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar" CACHE FILEPATH "Emscripten ar")
set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib" CACHE FILEPATH "Emscripten ranlib")

# Set system processor
set(CMAKE_SYSTEM_PROCESSOR x86)

# Set compilation flags
set(CMAKE_C_FLAGS_INIT "-s USE_PTHREADS=0")
set(CMAKE_CXX_FLAGS_INIT "-s USE_PTHREADS=0")

# Set link flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "-s WASM=1 -s USE_PTHREADS=0")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-s WASM=1 -s USE_PTHREADS=0")

# Don't run the linker on compiler check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Set find root path
set(CMAKE_FIND_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set executable suffix
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Disable some features that don't work well with Emscripten
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "")

# Set build type specific flags
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

# Platform-specific settings
set(CMAKE_SYSTEM_INCLUDE_PATH "${EMSCRIPTEN_ROOT_PATH}/system/include")

# Emscripten-specific cache variables
set(EMSCRIPTEN 1 CACHE BOOL "Emscripten build")
set(EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES 1 CACHE BOOL "Generate bitcode static libraries")

message(STATUS "Emscripten toolchain configured for WebAssembly build")
message(STATUS "Emscripten root: ${EMSCRIPTEN_ROOT_PATH}")