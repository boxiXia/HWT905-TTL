cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17) # set c++ standard to 17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(CMakeDemo VERSION 1.0)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# find_package(LAPACK REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
add_executable(demo src/UARTTest.cpp src/Com.cpp src/Com.h src/JY901.cpp src/JY901.h)

