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

AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = libmacpo inst libmrt analyze libset
dist_bin_SCRIPTS = macpo.sh

GTEST_DIR = $(srcdir)/../../contrib/gtest
GTEST_INC = $(GTEST_DIR)/include
GTEST_SRC = $(GTEST_DIR)/src

check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = $(GTEST_SRC)/gtest-all.cc $(GTEST_SRC)/gtest_main.cc

MACPO_INCLUDE_FLAGS = -I$(srcdir)/inst/include -I$(srcdir)/tests/libmrt \
    -I$(srcdir)/common

AM_CXXFLAGS = -I$(GTEST_DIR) -isystem $(GTEST_INC) $(MACPO_INCLUDE_FLAGS)
AM_LDFLAGS = -lpthread -lgtest -lrose

MINST_BASE = $(srcdir)/inst
MINST_SOURCE_FILES = $(MINST_BASE)/argparse.cpp $(MINST_BASE)/minst.cpp \
    $(MINST_BASE)/instrumentor.cpp $(MINST_BASE)/streams.cpp \
    $(MINST_BASE)/aligncheck.cpp $(MINST_BASE)/ir_methods.cpp \
    $(MINST_BASE)/loop_traversal.cpp $(MINST_BASE)/tracer.cpp \
    $(MINST_BASE)/generic_vars.cpp $(MINST_BASE)/vector_strides.cpp \
    $(MINST_BASE)/analysis_profile.cpp $(MINST_BASE)/tripcount.cpp \
    $(MINST_BASE)/branchpath.cpp $(MINST_BASE)/pntr_overlap.cpp \
    $(MINST_BASE)/stride_check.cpp $(MINST_BASE)/traversal.cpp \
    $(MINST_BASE)/reuse_dist.cpp $(srcdir)/tests/libmrt/mrt.cpp

utestdir = $(srcdir)/tests/unit-tests
itestdir = $(srcdir)/tests/integration-tests

UTESTS = utest_0001 utest_0002 utest_0003
ITESTS = itest_0001 itest_0002

check_PROGRAMS = $(UTESTS) $(ITESTS)
TESTS = $(check_PROGRAMS)

utest_0001_SOURCES = $(utestdir)/argparse-tests.cpp $(MINST_SOURCE_FILES)
utest_0002_SOURCES = $(utestdir)/irmethods-tests.cpp $(MINST_SOURCE_FILES)
utest_0003_SOURCES = $(utestdir)/libmrt-tests.cpp $(MINST_SOURCE_FILES)

itest_0001_SOURCES = $(itestdir)/basic-tests.cpp $(itestdir)/itest_harness.cpp \
    $(MINST_SOURCE_FILES)
itest_0002_SOURCES = $(itestdir)/compound-tests.cpp \
    $(itestdir)/itest_harness.cpp $(MINST_SOURCE_FILES)
