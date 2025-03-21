cmake_minimum_required(VERSION 3.3)

# Project name and version
project(TaskService VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Add minimum compiler version check
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
        message(FATAL_ERROR "Clang version must be at least 16!")
    endif()
endif()

# Add version checks for system requirements
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++23" COMPILER_SUPPORTS_CXX23)
if(NOT COMPILER_SUPPORTS_CXX23)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++23 support.")
endif()

# Add the main include directory 
include_directories(${CMAKE_SOURCE_DIR}/include) 

# Add the vendor include directory 
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/include/vendor)

# Define source and include directories
set(SRC_DIR "src")
set(INCLUDE_DIR "include")
set(BIN_DIR "build")

file(GLOB SOURCES ${SRC_DIR}/*.cpp)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${BIN_DIR})

# Include directories
include_directories(${INCLUDE_DIR})

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(spdlog REQUIRED)

# Brotli (assumes it is installed using Homebrew or similar package manager)
if(APPLE AND EXISTS "/opt/homebrew")
    set(BROTLI_ROOT "/opt/homebrew")
else()
    set(BROTLI_ROOT "/usr/local")
endif()

find_path(BROTLI_INCLUDE_DIR brotli PATHS ${BROTLI_ROOT}/include)
find_library(BROTLI_COMMON_LIBRARY NAMES brotlicommon PATHS ${BROTLI_ROOT}/lib)
find_library(BROTLI_ENC_LIBRARY NAMES brotlienc PATHS ${BROTLI_ROOT}/lib)
find_library(BROTLI_DEC_LIBRARY NAMES brotlidec PATHS ${BROTLI_ROOT}/lib)

if(NOT BROTLI_INCLUDE_DIR OR NOT BROTLI_COMMON_LIBRARY OR NOT BROTLI_ENC_LIBRARY OR NOT BROTLI_DEC_LIBRARY)
    message(FATAL_ERROR "Brotli libraries not found. Make sure they are installed.")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# Libraries for dependencies
set(DEPENDENT_LIBRARIES
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    spdlog::spdlog
    ${BROTLI_COMMON_LIBRARY}
    ${BROTLI_ENC_LIBRARY}
    ${BROTLI_DEC_LIBRARY}
    pthread
)

# Compiler definitions
add_definitions(
    -DCPPHTTPLIB_OPENSSL_SUPPORT
    -DCPPHTTPLIB_ZLIB_SUPPORT
    -DCPPHTTPLIB_BROTLI_SUPPORT
)

# Add executable targets
add_executable(task-service 
    ${SRC_DIR}/task-service.cpp
    ${SRC_DIR}/logging.cpp
    ${SRC_DIR}/service.cpp
    ${SRC_DIR}/taskdb.cpp
    ${SRC_DIR}/cli.cpp
)

add_executable(task-client
    ${SRC_DIR}/taskdb.cpp
    ${SRC_DIR}/task-client.cpp
)

add_executable(task-unit
    ${SRC_DIR}/task-unit.cpp
    ${SRC_DIR}/logging.cpp
    ${SRC_DIR}/service.cpp
    ${SRC_DIR}/cli.cpp
    ${SRC_DIR}/taskdb.cpp
)


add_executable(integration ${SRC_DIR}/integration.cpp)

# Specify the precompiled header
target_precompile_headers(task-service PRIVATE include/precompiled.hpp)
target_precompile_headers(task-client PRIVATE include/precompiled.hpp)

# turn off optimization to speed compile times
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OPTIM_COMPILE_FLAGS "-O0")
else()
    set(OPTIM_COMPILE_FLAGS "-O0")
endif()

target_compile_options(task-client PRIVATE ${OPTIM_COMPILE_FLAGS})
target_compile_options(task-service PRIVATE ${OPTIM_COMPILE_FLAGS})
target_compile_options(task-unit PRIVATE ${OPTIM_COMPILE_FLAGS})
target_compile_options(integration PRIVATE ${OPTIM_COMPILE_FLAGS})

# Link libraries to executables
target_link_libraries(task-service PRIVATE ${DEPENDENT_LIBRARIES})
target_link_libraries(task-client PRIVATE ${DEPENDENT_LIBRARIES})
target_link_libraries(task-unit PRIVATE ${DEPENDENT_LIBRARIES})
target_link_libraries(integration PRIVATE ${DEPENDENT_LIBRARIES})

# Installation rules
install(TARGETS task-service task-client DESTINATION /usr/local/bin)

# TODO configure for UNITY_BUILD_TARGETS
set(UNITY_BUILD_TARGETS
        task-service
        task-client
        task-unit
        integration
)

foreach(target ${UNITY_BUILD_TARGETS})
    set_target_properties(${target} PROPERTIES
            UNITY_BUILD ON
            UNITY_BUILD_MAX_FILE_COUNT 4
            UNITY_BUILD_BATCH_SIZE 0
    )
endforeach()


# Static linking for standalone executable
if(NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# MacOS universal binary setup
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
    list(APPEND DEPENDENT_LIBRARIES
        "-framework CoreFoundation"
        "-framework Security"
    )
endif()

# Debian container specific setup
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND EXISTS "/etc/debian_version")
    # Debian specific configurations (if any)
endif()

