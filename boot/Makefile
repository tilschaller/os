DEFAULT_HOST!=../scripts/default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../scripts/target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -march=native -fno-PIC -pedantic -W -Wall -ffreestanding -mno-red-zone -nostdinc -mcmodel=kernel -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-sse4a -mno-3dnow -mno-avx -mno-avx2 -fno-stack-protector -mgeneral-regs-only -mtune=generic -mno-bmi2 -mno-bmi
LDFLAGS= -nostdlib -mcmodel=kernel -fno-PIC -no-pie -Wl,--build-id=none -Wl,-z,max-page-size=0x1000,-n
CPPFLAGS:=$(CPPFLAGS) -D__is_loader -Iinclude
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(HOSTARCH)

LOADER_OBJS=\
$(LOADER_ARCH_OBJS) \

OBJS=\
$(LOADER_OBJS)

LINK_LIST=\
$(LDFLAGS) \
$(LOADER_OBJS) \

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(LOADER_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(LOADER_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(LOADER_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(LOADER_ARCH_LIBS)

.PHONY: all clean install
.SUFFIXES: .o .c .S

all: bootloader.bin

bootloader.bin: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(LDFLAGS) $(LINK_LIST)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -f bootloader.bin
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d	

install: 
