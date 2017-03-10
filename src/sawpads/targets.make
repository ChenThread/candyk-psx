LIBS_SAWPADS_OBJS = \
	src/sawpads/sawpads.o

OUTPUT_LIBS += $(OUTPUT_LIBDIR)libsawpads.a
OUTPUT_LIBS_OBJS += $(LIBS_SAWPADS_OBJS)

LIBS_SAWPADS_INCS =

src/sawpads/%.o: src/sawpads/%.c
	$(TARGET_CC) -c -o $@ $< $(TARGET_CFLAGS)

$(OUTPUT_LIBDIR)libsawpads.a: $(LIBS_SAWPADS_OBJS) $(LIBS_SAWPADS_INCS)
	$(TARGET_AR) crs $@ $(LIBS_SAWPADS_OBJS)

