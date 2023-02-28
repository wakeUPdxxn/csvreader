PROJECT(csvreader)

IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.0)

  MESSAGE(FATAL_ERROR

    "${PROJECT_NAME} requires at least CMake v3.0."

    " You are running v${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}."

    " Please upgrade.")

ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.0)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
SET(CMAKE_CXX_COMPILER g++)
SET(CMAKE_C_COMPILER gcc)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
SET(SOURCES ./sources/main.cpp ./sources/Reader.cpp)
SET(HEADERS ./sources/Exception.h ./sources/Reader.h)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
