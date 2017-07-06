# Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of libautoupdate, distributed under the MIT License
# For full terms see the included LICENSE file

PROJECT(libautoupdate C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(ZLIB)

INCLUDE_DIRECTORIES(src include ${ZLIB_INCLUDE_DIR})
FILE(GLOB SRC_H include/autoupdate.h)
FILE(GLOB SRC_AUTOUPDATE src/*.c src/*.h)
ADD_LIBRARY(autoupdate ${SRC_H} ${SRC_AUTOUPDATE})

IF(BUILD_TESTS)
  ENABLE_TESTING()
  ADD_TEST(autoupdate_tests autoupdate_tests)
  FILE(GLOB SRC_TEST tests/*.c)
  ADD_EXECUTABLE(autoupdate_tests ${SRC_TEST})
  TARGET_LINK_LIBRARIES(autoupdate_tests autoupdate ${ZLIB_LIBRARIES})
ENDIF()
