cmake_minimum_required(VERSION 3.31.5)

project(dxcap_layer VERSION 1.0 LANGUAGES CXX)

# c++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # Use static runtime library (/MT) to avoid dependency on VC++ Redistributable.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

    # --- Release-in-Debug Configuration ---
    # The user intends to use the Debug configuration for release builds.
    # We will apply release-level optimizations and strip debug information for the Debug build type.

    # --- Compiler Flags ---
    # C++ Conformance and Correctness
    # /permissive-: Enforce standard C++ conformance.
    # /Zc:__cplusplus: Ensure __cplusplus macro is correct.
    # /volatile:iso: Use strict volatile semantics for thread safety.
    # /W4: Stricter warnings.
    # /DNDEBUG: Disable asserts and other debug code.
    set(CMAKE_CXX_FLAGS_CONFORMANCE "/permissive- /Zc:__cplusplus /volatile:iso /W4 /DNDEBUG")

    # Performance Optimizations
    # /O2: Optimize for speed.
    # /Ob2: Aggressive inlining.
    # /Ot: Favor fast code over small code.
    # /GL: Whole program optimization.
    # /fp:fast: Fast floating-point model.
    # /Gw: Optimize global data.
    set(CMAKE_CXX_FLAGS_PERFORMANCE "/O2 /Ob2 /Ot /GL /fp:fast /Gw")

    set(CMAKE_CXX_FLAGS_SECURITY "/GS /guard:cf /sdl")

    # Suppress specific warnings encountered during build.
    set(CMAKE_CXX_FLAGS_WARNINGS_OFF "/wd4100 /wd4389 /wd4189 /wd4211 /wd4505")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_CONFORMANCE} ${CMAKE_CXX_FLAGS_PERFORMANCE} ${CMAKE_CXX_FLAGS_SECURITY} ${CMAKE_CXX_FLAGS_WARNINGS_OFF}")

    # --- Linker Flags ---
    # Performance Optimizations
    # /LTCG: Link-time code generation.
    # /INCREMENTAL:NO: Create a non-incremental final image.
    # /OPT:REF: Remove unreferenced functions/data.
    # /OPT:ICF: Fold identical COMDATs.
    # /RELEASE: Sets the checksum in the header, indicating a release build.
    set(CMAKE_LINKER_FLAGS_PERFORMANCE "/LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF /RELEASE")

    set(CMAKE_LINKER_FLAGS_SECURITY "/DYNAMICBASE /HIGHENTROPYVA /NXCOMPAT")
    
    set(CMAKE_LINKER_FLAGS_OBFUSCATION "/MERGE:.rdata=.text /PDB:NONE /DEBUG:NONE")

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_PERFORMANCE} ${CMAKE_LINKER_FLAGS_SECURITY} ${CMAKE_LINKER_FLAGS_OBFUSCATION}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_PERFORMANCE} ${CMAKE_LINKER_FLAGS_SECURITY} ${CMAKE_LINKER_FLAGS_OBFUSCATION}")
endif()

# Set generator platform to x64
set(CMAKE_GENERATOR_PLATFORM x64)

# Define DEBUG macro centrally
# Can be overridden with -DPRINT=1 or -DPRINT=0
# Default: 1 for debug builds, 0 for release builds
if(NOT DEFINED PRINT)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
        set(PRINT 1)
    else()
        set(PRINT 0)
    endif()
endif()

add_compile_definitions(DEBUG=${PRINT})

# src/build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/Release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/Release")

# ------------------------------------------------------
# ImGui — build once, link to both DLLs
# ------------------------------------------------------
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/include/ImGui)

file(GLOB IMGUI_SOURCES
     ${IMGUI_DIR}/imgui.cpp
     ${IMGUI_DIR}/imgui_draw.cpp
     ${IMGUI_DIR}/imgui_tables.cpp
     ${IMGUI_DIR}/imgui_widgets.cpp
     ${IMGUI_DIR}/imgui_impl_dx11.cpp
     ${IMGUI_DIR}/imgui_impl_win32.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR})  # headers already live here

# ------------------------------------------------------
# Combined d3d11.dll and dxgi.dll build (single DLL)
# ------------------------------------------------------
add_library(d3d11_dxgi_layer SHARED src/d3d11.cpp)
set_target_properties(d3d11_dxgi_layer
    PROPERTIES POSITION_INDEPENDENT_CODE ON
               OUTPUT_NAME "d3d11")

target_include_directories(d3d11_dxgi_layer PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
    ${IMGUI_DIR})

target_link_libraries(d3d11_dxgi_layer PRIVATE imgui)

# Create a copy with dxgi name for compatibility
add_custom_command(TARGET d3d11_dxgi_layer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:d3d11_dxgi_layer>
        $<TARGET_FILE_DIR:d3d11_dxgi_layer>/dxgi.dll
    COMMENT "Creating dxgi.dll copy")
