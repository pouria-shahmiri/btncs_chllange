cmake_minimum_required(VERSION 3.24)
project(data_streaming)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find FastDDS and FastCDR packages
find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)

# Include directories
include_directories(
    ${FASTRTPS_INCLUDE_DIRS}
    ${FASTCDR_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include  # <-- headers here
    ${CMAKE_SOURCE_DIR}/src
)

# Add executable with source files
add_executable(data_streaming
    src/main.cpp
    src/MBOPublisher.cpp
)

# Link FastDDS and FastCDR libraries
target_link_libraries(data_streaming
    ${FASTRTPS_LIBRARIES}
    ${FASTCDR_LIBRARIES}
)

# Set output directory
set_target_properties(data_streaming PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
