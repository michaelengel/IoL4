ARCH=i386
L4DIR=../../../pistachio-0.2

CC=gcc
CFLAGS=-g -O2 -I../../include -nostdinc -I. -Wall -fno-builtin  \
       -I$(L4DIR)/user/include -I$(L4DIR)/user
OBJS=exit.o notmain.o sbrk.o time.o
TARGETLIB=../lib/libCLight.a
AR=ar
RANLIB=ranlib
RM=rm -f

all: $(OBJS)

clean:
	$(RM) $(OBJS)
