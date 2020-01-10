OUTPUT_TOOL_LIBS =
OUTPUT_TOOL_LIBS_OBJS =

include toolsrc/libpsxav/targets.make

OUTPUT_TOOLS =
OUTPUT_TOOLS_OBJS =

include toolsrc/elf2psx/targets.make
include toolsrc/pscd-new/targets.make
include toolsrc/psxavenc/targets.make
include toolsrc/xainterleave/targets.make

