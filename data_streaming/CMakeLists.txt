cmake_minimum_required(VERSION 3.10)
project(DataStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find FastDDS
find_package(fastrtps REQUIRED)
find_package(fastcdr REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/dbn_reader.cpp
    src/market_data_publisher.cpp
)

# Create executable
add_executable(dbn_reader ${SOURCES})

# Link FastDDS libraries
target_link_libraries(dbn_reader fastrtps fastcdr)

# Compiler flags
if(MSVC)
    target_compile_options(dbn_reader PRIVATE /W4)
else()
    target_compile_options(dbn_reader PRIVATE -Wall -Wextra -pedantic)
endif()
