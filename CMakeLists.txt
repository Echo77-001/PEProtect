cmake_minimum_required(VERSION 3.12)

project(PEProtect LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 20)

enable_language(ASM_MASM)

set(ZSTD_BUILD_STATIC ON CACHE INTERNAL "")
set(ZSTD_BUILD_SHARED OFF CACHE INTERNAL "")
set(ZSTD_BUILD_PROGRAMS OFF CACHE INTERNAL "")
set(ZSTD_BUILD_TESTS OFF CACHE INTERNAL "")

include(FetchContent)

FetchContent_Declare(
    zstd
    URL "https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz"
    SOURCE_SUBDIR build/cmake
)

FetchContent_MakeAvailable(zstd)

add_executable(PEProtect
    src/main.cpp
    src/core/BasicPe.cpp
    src/core/BasicPe.h
    src/core/PEBuilder.cpp
    src/core/PEBuilder.h
    src/core/PEParser.cpp
    src/core/PEParser.h
    src/core/Utils.cpp
    src/core/Utils.h
    src/disassembler/disassembler.cpp
    src/disassembler/disassembler.h
    src/enums/Enums.h
    src/vm/VMInstruction.h
    src/vm/VMInstruction.cpp
    src/vm/VMCodeBlock.h
    src/vm/VMCodeBlock.cpp
)

add_library(PEProtect_core SHARED
    core/main.cpp
    core/Structs.h
    core/func.asm
)

target_include_directories(PEProtect PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${zstd_SOURCE_DIR}/lib
)

target_include_directories(PEProtect_core PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${zstd_SOURCE_DIR}/lib
)

target_link_libraries(PEProtect PRIVATE 
    libzstd_static
)

target_link_libraries(PEProtect_core PRIVATE 
    libzstd_static
)