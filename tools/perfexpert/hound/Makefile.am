#
# Copyright (c) 2013  University of Texas at Austin. All rights reserved.
#
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

bin_PROGRAMS = hound

hound_SOURCES = hound.c

AM_LDFLAGS = 

if ENABLE_DEBUG
    override hound_CFLAGS =
    override AM_CFLAGS = -O0 -fPIC -DDEBUG_PRINT -g
else
    override hound_CFLAGS =
    override AM_CFLAGS = -O0 -fPIC
endif


# EOF