// Data
#define GPU_VERTEX(x, y) (((x)&0xFFFF) | ((y)<<16))
#define GPU_TEXCOORD_UPPER(x, y, upper) (((x)&0xFF) | (((y)&0xFF)<<16) | ((upper)<<16))
#define GPU_TEXCOORD(x, y) GPU_TEXCOORD_UPPER((x), (y), 0)
#define GPU_TEXCOORD_TEXPAGE(x, y, xbase, ybase, blend, bpp) \
	GPU_TEXCOORD_UPPER((x), (y), ( \
	((xbase)&0xF) | (((ybase)&0x1)<<4) | blend | bpp))
#define GPU_TEXCOORD_CLUT(x, y, cx, cy) \
	GPU_TEXCOORD_UPPER((x), (y), ((cx)&0x3F)|(((cy)&0x1FF)<<6))
#define GPU_BOX_OFFS(x, y) (((x)&0xFFFF) | ((y)<<16))
#define GPU_BOX_SIZE(x, y) (((x)&0xFFFF) | ((y)<<16))
#define GPU_RGB8(r, g, b) ((((b)&0xFF)<<16) | (((g)&0xFF)<<8) | ((r)&0xFF))
#define GPU_BGR8(b, g, r) ((((b)&0xFF)<<16) | (((g)&0xFF)<<8) | ((r)&0xFF))

// Misc ops
#define GP0_NOP() (0x00000000)
#define GP0_CLEAR_CACHE() (0x01000000)
#define GP0_INTERRUPT_REQ() (0x1F000000)

// Memory ops
#define GP0_MEM_FILL(bgr) (0x02000000 | ((bgr)&0xFFFFFF))
#define GP0_MEM_COPY_WITHIN() (0x80000000)
#define GP0_MEM_COPY_TO_VRAM() (0xA0000000)
#define GP0_MEM_COPY_FROM_VRAM() (0xC0000000)

// Polygons
#define GP0_TRI_FLAT(bgr) (0x20000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_FLAT_BLEND(bgr) (0x22000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_FLAT_TEX(bgr) (0x24000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_TEX() (0x25000000)
#define GP0_TRI_FLAT_TEX_BLEND(bgr) (0x26000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_TEX_BLEND() (0x27000000)
#define GP0_QUAD_FLAT(bgr) (0x28000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_FLAT_BLEND(bgr) (0x2A000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_FLAT_TEX(bgr) (0x2C000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_TEX() (0x2D000000)
#define GP0_QUAD_FLAT_TEX_BLEND(bgr) (0x2E000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_TEX_BLEND() (0x2F000000)
#define GP0_TRI_GOURAUD(bgr) (0x30000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_GOURAUD_BLEND(bgr) (0x32000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_GOURAUD_TEX(bgr) (0x34000000 | ((bgr)&0xFFFFFF))
#define GP0_TRI_GOURAUD_TEX_BLEND(bgr) (0x36000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_GOURAUD(bgr) (0x38000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_GOURAUD_BLEND(bgr) (0x3A000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_GOURAUD_TEX(bgr) (0x3C000000 | ((bgr)&0xFFFFFF))
#define GP0_QUAD_GOURAUD_TEX_BLEND(bgr) (0x3E000000 | ((bgr)&0xFFFFFF))

// Lines
#define GPU_POLYLINE_END 0x55555555

#define GP0_LINE_FLAT(bgr) (0x40000000 | ((bgr)&0xFFFFFF))
#define GP0_LINE_FLAT_BLEND(bgr) (0x42000000 | ((bgr)&0xFFFFFF))
#define GP0_POLYLINE_FLAT(bgr) (0x48000000 | ((bgr)&0xFFFFFF))
#define GP0_POLYLINE_FLAT_BLEND(bgr) (0x4A000000 | ((bgr)&0xFFFFFF))
#define GP0_LINE_GOURAUD(bgr) (0x50000000 | ((bgr)&0xFFFFFF))
#define GP0_LINE_GOURAUD_BLEND(bgr) (0x52000000 | ((bgr)&0xFFFFFF))
#define GP0_POLYLINE_GOURAUD(bgr) (0x58000000 | ((bgr)&0xFFFFFF))
#define GP0_POLYLINE_GOURAUD_BLEND(bgr) (0x5A000000 | ((bgr)&0xFFFFFF))

// Rects
#define GP0_RECT_FLAT_FLEX(bgr) (0x60000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_FLEX_BLEND(bgr) (0x62000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_1(bgr) (0x68000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_1_BLEND(bgr) (0x6A000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_8(bgr) (0x70000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_8_BLEND(bgr) (0x72000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_16(bgr) (0x78000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_16_BLEND(bgr) (0x7A000000 | ((bgr)&0xFFFFFF))

#define GP0_RECT_FLAT_TEX_FLEX(bgr) (0x64000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_FLEX(bgr) (0x65000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_FLEX_BLEND(bgr) (0x66000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_FLEX_BLEND(bgr) (0x67000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_1(bgr) (0x6C000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_1(bgr) (0x6D000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_1_BLEND(bgr) (0x6E000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_1_BLEND(bgr) (0x6F000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_8(bgr) (0x74000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_8(bgr) (0x75000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_8_BLEND(bgr) (0x76000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_8_BLEND(bgr) (0x77000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_16(bgr) (0x7C000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_16(bgr) (0x7D000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_FLAT_TEX_16_BLEND(bgr) (0x7E000000 | ((bgr)&0xFFFFFF))
#define GP0_RECT_TEX_16_BLEND(bgr) (0x7F000000 | ((bgr)&0xFFFFFF))

// Attributes
#define GP0_ATTR_TEXPAGE(xbase, ybase, blend, bpp, flags) \
	(0xE1000000 | ((xbase)&0xF) | (((ybase)&0x1)<<4) | blend | bpp | flags)

#define GPU_TEXPAGE_BLEND_HALF 0x00
#define GPU_TEXPAGE_BLEND_ADD 0x20
#define GPU_TEXPAGE_BLEND_SUB 0x40
#define GPU_TEXPAGE_BLEND_ADDQUARTER 0x60

#define GPU_TEXPAGE_TEXBPP_4 0x000
#define GPU_TEXPAGE_TEXBPP_8 0x080
#define GPU_TEXPAGE_TEXBPP_15 0x100

#define GPU_TEXPAGE_DITHER 0x0200
#define GPU_TEXPAGE_DRAWTODISPLAY 0x0400
#define GPU_TEXPAGE_TEXOFFDEBUG 0x0800
#define GPU_TEXPAGE_XFLIP 0x1000
#define GPU_TEXPAGE_YFLIP 0x2000

#define GP0_ATTR_TEXWINDOW(xmask, ymask, xoffs, yoffs) \
	(0xE2000000 \
		| ((xmask)&0x1F) \
		| (((ymask)&0x1F)<<5) \
		| (((xoffs)&0x1F)<<10) \
		| (((yoffs)&0x1F)<<15) \
	)

#define GP0_ATTR_DRAW_RANGE_MIN(x, y) (0xE3000000 | ((x)&0x3FF) | (((y)&0x3FF)<<10))
#define GP0_ATTR_DRAW_RANGE_MAX(x, y) (0xE4000000 | ((x)&0x3FF) | (((y)&0x3FF)<<10))
#define GP0_ATTR_DRAW_OFFSET(x, y) (0xE5000000 | ((x)&0x7FF) | (((y)&0x7FF)<<11))

#define GP0_ATTR_MASKBIT(flags) (0xE6000000 | (flags))
#define GPU_MASKBIT_ALWAYS1 0x1
#define GPU_MASKBIT_DRAWIF0 0x2

// GP1 ops
#define GP1_RESET_GPU() 0x00000000
#define GP1_RESET_COMMAND_BUFFER() 0x01000000
#define GP1_INTERRUPT_ACK() 0x02000000
#define GP1_DISPLAY_ENABLE() 0x03000000
#define GP1_DISPLAY_DISABLE() 0x03000001

#define GP1_DMA_DIRECTION(dmadir) (0x04000000 | ((dmadir)&3))
#define GPU_DMADIR_OFF    0x0
#define GPU_DMADIR_FIFO   0x1
#define GPU_DMADIR_TO_GPU 0x2
#define GPU_DMADIR_TO_CPU 0x3

#define GP1_DISPLAY_START(x, y) \
	(0x05000000 | ((x)&0x3FF) | (((y)&0x1FF)<<10))
#define GP1_DISPLAY_RANGE_X(xbeg, xend) \
	(0x06000000 | ((xbeg)&0xFFF) | (((xend)&0xFFF)<<12))
#define GP1_DISPLAY_RANGE_Y(ybeg, yend) \
	(0x07000000 | ((ybeg)&0x3FF) | (((yend)&0x3FF)<<10))

// GP1: Display modes
#define GP1_DISPLAY_MODE(w, h, bpp, std) \
	(0x08000000 | (w) | (h) | (bpp) | (std))

#define GPU_DISPMODE_W_256 0x00
#define GPU_DISPMODE_W_320 0x01
#define GPU_DISPMODE_W_512 0x02
#define GPU_DISPMODE_W_640 0x03
#define GPU_DISPMODE_W_368 0x40

#define GPU_DISPMODE_H_240P 0x00
#define GPU_DISPMODE_H_240I 0x20
#define GPU_DISPMODE_H_480I 0x24

#define GPU_DISPMODE_BPP_15 0x00
#define GPU_DISPMODE_BPP_24 0x10

#define GPU_DISPMODE_STD_NTSC 0x00
#define GPU_DISPMODE_STD_PAL  0x08


//define GP0_INTERRUPT_REQ() 0x

