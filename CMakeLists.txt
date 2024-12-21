cmake_minimum_required(VERSION 3.16)

# Project name and version
project(TaskService VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
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
find_package(GTest REQUIRED)
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
    ${SRC_DIR}/tasks.cpp
    ${SRC_DIR}/cli.cpp
)

add_executable(integration ${SRC_DIR}/integration.cpp)

# Link libraries to executables
target_link_libraries(task-service PRIVATE ${DEPENDENT_LIBRARIES})
target_link_libraries(integration PRIVATE ${DEPENDENT_LIBRARIES})

# Installation rules
install(TARGETS task-service integration DESTINATION ${BIN_DIR})

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

