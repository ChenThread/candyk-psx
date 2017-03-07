# Currently haven't put this with the rest of the fire.

SRCS = \
	main.c

LIBS = -nostdlib -L../../lib -lgcc -lc -lchenboot
INCLUDES = -I../../include

CROSSPREFIX = mipsel-none-elf-
TARGET_CC = $(CROSSPREFIX)gcc
TARGET_CFLAGS = -g -flto -O2 -msoft-float -mips1 $(INCLUDES)
TARGET_LDFLAGS = -g -flto -O2 -msoft-float -mips1 -Wl,-Ttext-segment=0x80010000 $(LIBS)
ELF2PSX = ../../bin/elf2psx
ISO2RAW = ../../bin/iso2raw
MKISOFS = mkisofs
RM_F = rm -f

clean:
	$(RM_F) triangle.elf

all: triangle.iso.bin

triangle.iso.bin: triangle.iso
	$(ISO2RAW) ../../dat/isolicence.pal triangle.iso

triangle.iso: triangle.exe
	$(MKISOFS) -o triangle.iso system.cnf triangle.exe

triangle.exe: triangle.elf
	$(ELF2PSX) -p triangle.elf triangle.exe

triangle.elf: $(SRCS)
	$(TARGET_CC) -o triangle.elf $(SRCS) $(TARGET_CFLAGS) $(TARGET_LDFLAGS)

