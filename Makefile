#
# File          : Makefile
# Description   : Build file for CMSC312 Assignment 4


# Environment Setup
LIBDIRS=-L. 
INCLUDES=-I.
CC=gcc 
CFLAGS=-c $(INCLUDES) -g -Wall
LINK=gcc -g
LDFLAGS=$(LIBDIRS)
AR=ar rc
RANLIB=ranlib

# Suffix rules
.c.o :
	${CC} ${CFLAGS} $< -o $@

#
# Setup builds

PT-TARGETS=cmsc312-p4
CMSC312LIB=
CMSC312LIBOBJS=

# proj lib
LIBS=

#
# Project Protections

p4 : $(PT-TARGETS)

cmsc312-p4 : cmsc312-p4.o cmsc312-p4-mfu.o cmsc312-p4-second.o cmsc312-p4-lfu.o
	$(LINK) $(LDFLAGS) cmsc312-p4.o cmsc312-p4-mfu.o cmsc312-p4-second.o cmsc312-p4-lfu.o -o $@

lib$(CMSC312LIB).a : $(CMSC312LIBOBJS)
	$(AR) $@ $(CMSC312LIBOBJS)
	$(RANLIB) $@

clean:
	rm -f *.o *~ $(TARGETS) $(LIBOBJS) lib$(CMSC312LIB).a

