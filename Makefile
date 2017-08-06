CROSSPREFIX = mipsel-none-elf-
RM = rm
RM_F = $(RM) -f
NATIVE_CC = $(CC)
NATIVE_CFLAGS = -O2 -g
NATIVE_LDFLAGS = -O2 -g
TARGET_AR = $(CROSSPREFIX)gcc-ar
TARGET_CC = $(CROSSPREFIX)gcc
TARGET_LD = $(CROSSPREFIX)ld
TARGET_RANLIB = $(CROSSPREFIX)ranlib
TARGET_CFLAGS = -flto -O2 -g -msoft-float -mips1 -Iinclude
TARGET_LDFLAGS = -flto -O2 -g -msoft-float -mips1

EXEPOST=
OUTPUT_BINDIR = bin/
OUTPUT_LIBDIR = lib/

fake_all: all

include src/targets.make
include toolsrc/targets.make

all: tools libs

clean: clean_tools clean_libs

clean_tools:
	$(RM_F) $(OUTPUT_TOOLS) $(OUTPUT_TOOLS_OBJS) || true

clean_libs:
	$(RM_F) $(OUTPUT_LIBS) $(OUTPUT_LIBS_OBJS) || true

libs: $(OUTPUT_LIBS)

tools: $(OUTPUT_TOOLS)

.DUMMY: fake_all all tools libs

