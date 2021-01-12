cmake_minimum_required(VERSION 2.8.11)
SET(CMAKE_C_COMPILER "/usr/bin/gcc")
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")

project(BinaryReader)

find_package(OpenCV REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    "/usr/include/eigen3/"
    ${OpenCV_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} 
    src/database.cc
)


add_executable(database_reader database_reader.cc)
target_link_libraries(database_reader ${PROJECT_NAME} sqlite3 ${OpenCV_LIBS} )