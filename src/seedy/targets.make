LIBS_SEEDY_OBJS = \
	src/seedy/seedy.o

OUTPUT_LIBS += $(OUTPUT_LIBDIR)libseedy.a
OUTPUT_LIBS_OBJS += $(LIBS_SEEDY_OBJS)

LIBS_SEEDY_INCS =

src/seedy/%.o: src/seedy/%.c
	$(TARGET_CC) -c -o $@ $< $(TARGET_CFLAGS)

$(OUTPUT_LIBDIR)libseedy.a: $(LIBS_SEEDY_OBJS) $(LIBS_SEEDY_INCS)
	$(TARGET_AR) crs $@ $(LIBS_SEEDY_OBJS)

