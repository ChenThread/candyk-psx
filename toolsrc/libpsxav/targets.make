TOOLS_LIBPSXAV_OBJS =
TOOLS_LIBPSXAV_OBJS += toolsrc/libpsxav/adpcm.o
TOOLS_LIBPSXAV_OBJS += toolsrc/libpsxav/cdrom.o

TOOLS_LIBPSXAV_INCS =
TOOLS_LIBPSXAV_INCS += toolsrc/libpsxav/libpsxav.h

OUTPUT_TOOL_LIBS += toolsrc/libpsxav/libpsxav$(LIBPOST)
OUTPUT_TOOL_LIBS_OBJS += $(TOOLS_LIBPSXAV_OBJS)

toolsrc/libpsxav/%.o: toolsrc/libpsxav/%.c
	$(NATIVE_CC) -c -o $@ $< $(NATIVE_CFLAGS)

toolsrc/libpsxav/libpsxav$(LIBPOST): $(TOOLS_LIBPSXAV_OBJS) $(TOOLS_LIBPSXAV_INCS)
	$(NATIVE_AR) crs $@ $(TOOLS_LIBPSXAV_OBJS)
