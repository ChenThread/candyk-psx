// Data
#define GPU_VERTEX(x, y) (((x)&0xFFFF) | ((y)<<16))
#define GPU_BOX_OFFS(x, y) (((x)&0xFFFF) | ((y)<<16))
#define GPU_BOX_SIZE(x, y) (((x)&0xFFFF) | ((y)<<16))
#define GPU_RGB8(r, g, b) ((((b)&0xFF)<<16) | (((g)&0xFF)<<8) | ((r)&0xFF))
#define GPU_BGR8(b, g, r) ((((b)&0xFF)<<16) | (((g)&0xFF)<<8) | ((r)&0xFF))

// Memory ops
#define GP0_MEM_FILL(bgr) (0x02000000 | ((bgr)&0xFFFFFF))

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

// TODO: rects and lines and whatnot

// Attributes
// TODO: other attributes!
//define GP0_ATTR_TEXPAGE()
//define GP0_ATTR_TEXWINDOW()
#define GP0_ATTR_DRAW_RANGE_MIN(x, y) (0xE3000000 | ((x)&0x3FF) | (((y)&0x3FF)<<10))
#define GP0_ATTR_DRAW_RANGE_MAX(x, y) (0xE4000000 | ((x)&0x3FF) | (((y)&0x3FF)<<10))
#define GP0_ATTR_DRAW_OFFSET(x, y) (0xE5000000 | ((x)&0x7FF) | (((y)&0x7FF)<<11))
//define GP0_ATTR_MASK_BIT()

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

