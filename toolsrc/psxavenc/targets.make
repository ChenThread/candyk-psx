OUTPUT_TOOLS += $(OUTPUT_BINDIR)psxavenc$(EXEPOST)
OUTPUT_TOOLS_OBJS +=

TOOLS_PSXAVENC_SRCS =
TOOLS_PSXAVENC_SRCS += toolsrc/psxavenc/cdrom.c
TOOLS_PSXAVENC_SRCS += toolsrc/psxavenc/decoding.c
TOOLS_PSXAVENC_SRCS += toolsrc/psxavenc/filefmt.c
TOOLS_PSXAVENC_SRCS += toolsrc/psxavenc/mdec.c
TOOLS_PSXAVENC_SRCS += toolsrc/psxavenc/psxavenc.c

TOOLS_PSXAVENC_INCS =
TOOLS_PSXAVENC_INCS += toolsrc/psxavenc/common.h

$(OUTPUT_BINDIR)psxavenc$(EXEPOST): $(TOOLS_PSXAVENC_SRCS) $(TOOLS_PSXAVENC_INCS) toolsrc/libpsxav/libpsxav.a
	$(NATIVE_CC) -o $@ $(TOOLS_PSXAVENC_SRCS) $(NATIVE_CFLAGS) $(NATIVE_LDFLAGS) \
		-Itoolsrc/libpsxav -Ltoolsrc/libpsxav \
		-lavcodec -lavformat -lavutil -lswresample -lswscale -lpsxav