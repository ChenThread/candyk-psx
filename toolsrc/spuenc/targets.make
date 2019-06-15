OUTPUT_TOOLS += $(OUTPUT_BINDIR)spuenc$(EXEPOST)
OUTPUT_TOOLS_OBJS +=

TOOLS_SPUENC_SRCS =
TOOLS_SPUENC_SRCS += toolsrc/spuenc/adpcm.c
TOOLS_SPUENC_SRCS += toolsrc/spuenc/cdrom.c
TOOLS_SPUENC_SRCS += toolsrc/spuenc/filefmt.c
TOOLS_SPUENC_SRCS += toolsrc/spuenc/spuenc.c

TOOLS_SPUENC_INCS =
TOOLS_SPUENC_INCS += toolsrc/spuenc/common.h

$(OUTPUT_BINDIR)spuenc$(EXEPOST): $(TOOLS_SPUENC_SRCS) $(TOOLS_SPUENC_INCS)
	$(NATIVE_CC) -o $@ $(TOOLS_SPUENC_SRCS) $(NATIVE_CFLAGS) $(NATIVE_LDFLAGS) -lavcodec -lavformat -lavutil -lswresample

