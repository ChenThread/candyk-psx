# Currently haven't put this with the rest of the fire.

SRCS = \
	main.c

LIBS = -L../../lib -lchenboot 

CROSSPREFIX = mipsel-none-elf-
TARGET_CC = $(CROSSPREFIX)gcc
TARGET_CFLAGS = -g -O1 -msoft-float -mips1
TARGET_LDFLAGS = -g -O1 -msoft-float -mips1 -Wl,-Ttext-segment=0x80010000 $(LIBS)
ELF2PSX = ../../bin/elf2psx
ISO2RAW = ../../bin/iso2raw
MKISOFS = mkisofs

triangle.iso.bin: triangle.iso
	$(ISO2RAW) ../../dat/isolicence.pal triangle.iso

triangle.iso: triangle.exe
	$(MKISOFS) -o triangle.iso system.cnf triangle.exe

triangle.exe: triangle.elf
	$(ELF2PSX) -p triangle.elf triangle.exe

triangle.elf: $(SRCS)
	$(TARGET_CC) -o triangle.elf $(SRCS) $(TARGET_CFLAGS) $(TARGET_LDFLAGS)

