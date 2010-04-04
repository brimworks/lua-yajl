# Copyright (C) 2007-2009 LuaDist.
# Submitted by David Manura
# Redistribution and use of this file is allowed according to the
# terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own
# license.

PROJECT(lua-yajl C)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

# Basic configurations
  SET(INSTALL_CMOD share/lua/cmod CACHE PATH "Directory to install Lua binary modules (configure lua via LUA_CPATH)")
# / configs

# Find yajl
  FIND_LIBRARY (YAJL_LIBRARY NAMES yajl)
  FIND_PATH (YAJL_INCLUDE_DIR yajl/yajl_parse.h
    ) # Find header
  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(yajl  DEFAULT_MSG  YAJL_LIBRARY YAJL_INCLUDE_DIR)
# / Find yajl

# Find lua
  FIND_PACKAGE(Lua51 REQUIRED)
# / Find lua

# Define how to build yajl.so
  INCLUDE_DIRECTORIES(${YAJL_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
  ADD_LIBRARY(cmod_yajl MODULE
    lua_yajl.c yajl.def)
  SET_TARGET_PROPERTIES(cmod_yajl PROPERTIES PREFIX "")
  SET_TARGET_PROPERTIES(cmod_yajl PROPERTIES OUTPUT_NAME yajl)
  TARGET_LINK_LIBRARIES(cmod_yajl ${LUA_LIBRARIES} ${YAJL_LIBRARY})  
# / build yajl.so

# Define how to test yajl.so:
  INCLUDE(CTest)
  FIND_PROGRAM(LUA NAMES lua lua.bat)
  ADD_TEST(basic ${LUA} ${CMAKE_CURRENT_SOURCE_DIR}/test.lua ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/)
  SET_TESTS_PROPERTIES(basic
                       PROPERTIES
                       FAIL_REGULAR_EXPRESSION
                       "not ok")
# / test yajl.so

# Where to install stuff
  INSTALL (TARGETS cmod_yajl DESTINATION ${INSTALL_CMOD})
# / Where to install.
