cmake_minimum_required (VERSION 3.16)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(mimir)

set(PUBLIC_HEADERS
  include/mimir.h
  include/block.h
  include/util.h
)

set(SRCS
  src/block.cpp
  src/util.cpp
)

add_library(mimir ${SRCS} ${PUBLIC_HEADERS})
SET_TARGET_PROPERTIES(
  hydra
PROPERTIES
  VERSION 0.1.0
  SOVERSION 0.1
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

install(
  TARGETS mimir
    LIBRARY
      DESTINATION lib
    ARCHIVE 
      DESTINATION lib
    PUBLIC_HEADER
      DESTINATION include/mimir
)
