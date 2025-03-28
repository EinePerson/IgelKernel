DEFAULT_HOST!=../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

override CC := "cc"

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -m64 -fno-stack-protector -O0
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -Iinclude -O0
LIBK_CFLAGS:=$(CFLAGS) -I../kernel/include
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk

ARCHDIR=arch/x86_64

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

IGFLAGS = -L "-Wall -Wextra -fno-stack-protector -m64 -O0" \
	--ffreestanding \
	-I include \
    --fno-mangle

LIBK_IGFLAGS = $(IGFLAGS) -I ../kernel/include

CFILES = $(shell find -L * -type f -name '*.c' | LC_ALL=C sort)
IGFILES = $(shell find -L * -type f -name '*.ig' | LC_ALL=C sort)

#$(CFILES) $(IGFILES)


#$(addsuffix .o,$(CFILES)) \
#$(addsuffix .o,$(IGFILES))
#$(CFILES:.c=.o) $(IGFILES:.ig=.o)

FREEOBJS= $(ARCH_FREEOBJS) \
$(CFILES:.c=.o) $(IGFILES:.ig=.o)
#src/stdio/printf.o \
src/stdio/putchar.o \
src/stdio/puts.o \
src/stdlib/abort.o \
src/string/memcmp.o \
src/string/memcpy.o \
src/string/memmove.o \
src/string/memset.o \
src/string/strlen.o \
src/string/strcmp.o \
src/memory/heap.o \
src/memory/same_sized_heap.o \
src/memory/page_manager.o #\
src/lists/linkedList.o

HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S .ig

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)
	ranlib $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.ig.o:
	igc -c $< -o $@  $(IGLAGS)

.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.S.libk.o:
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

.ig.libk.o:
	igc -c $< -o $@ $(LIBK_IGFLAGS)

clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)