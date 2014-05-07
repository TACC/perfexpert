#
# Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# This file is part of PerfExpert.
#
# PerfExpert is free software: you can redistribute it and/or modify it under
# the terms of the The University of Texas at Austin Research License
# 
# PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.
# 
# Authors: Leonardo Fialho and Ashay Rane
#
# $HEADER$
#

AM_CPPFLAGS = -g -pthread
AM_CXXFLAGS = -Wno-deprecated -I$(srcdir)/../../include -I$(GTEST_DIR) \
                -isystem $(GTEST_DIR)/include -I$(srcdir)/../../libmrt
AM_LDFLAGS = -lpthread -lgtest -lrose -ljvm

GTEST_DIR = $(srcdir)/../../../../contrib/gtest

check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = $(GTEST_DIR)/src/gtest-all.cc \
                        $(GTEST_DIR)/src/gtest_main.cc

check_PROGRAMS = test_0001 test_0002 test_0003
TESTS = $(check_PROGRAMS)

test_0001_SOURCES = $(srcdir)/../../inst/argparse.cpp \
    $(srcdir)/argparse-tests.cpp
test_0002_SOURCES = $(srcdir)/../../inst/ir_methods.cpp \
    $(srcdir)/irmethods-tests.cpp
test_0003_SOURCES = $(srcdir)/libmrt-tests.cpp
