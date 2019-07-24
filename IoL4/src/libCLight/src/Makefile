ARCH=i386

CC=gcc
CFLAGS=-g -O2 -I../include -nostdinc -I. -Wall -fno-builtin \
       -I../../pistachio-0.2/user/include
OBJS=memory.o strings.o file.o search.o rand.o qsort.o sscanf.o \
     cmpdi2.o udivdi3.o qdivrem.o errno.o env.o strtoul.o strtol.o \
     sprintf.o fixunsdfdi.o strlen.o strcmp.o strcpy.o strncpy.o \
     strncmp.o malloc.o sysdep/$(ARCH)/setjmp.o
L4OBJS=sysdep/exit.o sysdep/notmain.o sysdep/sbrk.o sysdep/time.o
CRTOBJ=sysdep/$(ARCH)/crt0.o
CRT0=../lib/crt0.o
TARGETLIB=../lib/libCLight.a
AR=ar
RANLIB=ranlib
RM=rm -f

all: l4stuff $(TARGETLIB) $(CRT0)

$(TARGETLIB): $(OBJS) 
	$(AR) ruv $(TARGETLIB) $(OBJS) $(L4OBJS)
	$(RANLIB) $(TARGETLIB)

l4stuff:
	$(MAKE) -C sysdep

clean:
	$(RM) $(OBJS) $(TARGETLIB)
	$(RM) $(L4OBJS)
	$(RM) $(CRTOBJ) $(CRT0)

$(CRT0): $(CRTOBJ)
	cp $(CRTOBJ) $(CRT0)
