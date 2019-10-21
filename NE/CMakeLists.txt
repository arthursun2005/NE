cmake_minimum_required(VERSION 2.4)

PROJECT( NE )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Os")

FILE(GLOB sources *.cpp)

ADD_EXECUTABLE( NE ${sources} )
