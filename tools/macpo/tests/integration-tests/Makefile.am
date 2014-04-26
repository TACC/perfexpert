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

SUBDIRS = libmrt

AM_CPPFLAGS = -g -pthread
AM_CXXFLAGS = -Wno-deprecated -I$(srcdir)/../../include -I$(GTEST_DIR) \
                -isystem $(GTEST_DIR)/include
AM_LDFLAGS = -lpthread -lgtest -lrose -ljvm

GTEST_DIR = $(srcdir)/../../../../contrib/gtest

check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = $(GTEST_DIR)/src/gtest-all.cc \
                        $(GTEST_DIR)/src/gtest_main.cc

check_PROGRAMS = test_0001
TESTS = $(check_PROGRAMS)

MINST_BASE = $(srcdir)/../..
MINST_SOURCE_FILES = $(MINST_BASE)/argparse.cpp $(MINST_BASE)/minst.cpp \
    $(MINST_BASE)/instrumentor.cpp $(MINST_BASE)/streams.cpp \
    $(MINST_BASE)/aligncheck.cpp $(MINST_BASE)/ir_methods.cpp \
    $(MINST_BASE)/loop_traversal.cpp $(MINST_BASE)/tracer.cpp \
    $(MINST_BASE)/generic_vars.cpp $(MINST_BASE)/vector_strides.cpp \
    $(MINST_BASE)/analysis_profile.cpp $(MINST_BASE)/tripcount.cpp \
    $(MINST_BASE)/branchpath.cpp $(MINST_BASE)/pntr_overlap.cpp \
    $(MINST_BASE)/stride_check.cpp $(MINST_BASE)/traversal.cpp \
    $(MINST_BASE)/reuse_dist.cpp

test_0001_SOURCES = $(srcdir)/basic-tests.cpp $(srcdir)/itest_harness.cpp $(MINST_SOURCE_FILES)
