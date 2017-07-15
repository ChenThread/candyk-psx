LIBS_ORELEI_OBJS = \
	src/orelei/midi.o \
	src/orelei/orelei.o

OUTPUT_LIBS += $(OUTPUT_LIBDIR)liborelei.a
OUTPUT_LIBS_OBJS += $(LIBS_ORELEI_OBJS)

LIBS_ORELEI_INCS =

src/orelei/%.o: src/orelei/%.c
	$(TARGET_CC) -c -o $@ $< $(TARGET_CFLAGS)

$(OUTPUT_LIBDIR)liborelei.a: $(LIBS_ORELEI_OBJS) $(LIBS_ORELEI_INCS)
	$(TARGET_AR) crs $@ $(LIBS_ORELEI_OBJS)

