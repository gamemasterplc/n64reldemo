//Library includes
#include <ultra64.h>
//Non-Library includes
#include "text.h"
#include "graphic.h"

static u8 font_tex_data[] __attribute__((aligned (8))) = {
	//Include font texture data
    #include "font.inc"
};

void TextDraw(int x, int y, char *str)
{
	static Gfx font_begin[] = {
		//Setup render Settings for drawing text
		gsDPSetCycleType(G_CYC_1CYCLE),
		gsDPSetTexturePersp(G_TP_NONE),
		gsDPSetTextureLUT(G_TT_NONE),
		gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
		gsDPSetRenderMode(G_RM_TEX_EDGE, G_RM_TEX_EDGE),
		//Load font texture
		gsDPLoadTextureTile_4b(font_tex_data, G_IM_FMT_I, 128, 48, 0, 0, 128, 48, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD),
		//Get ready to render text
		gsDPPipeSync(),
		gsSPEndDisplayList(),
	};
	//Call text initializing display list
	gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(font_begin));
	while(*str) {
		char c = *str++;
		//Draw valid characters
		if(x > -8 && y > -8 && c >= 32) {
			gSPScisTextureRectangle(glistp++, x*4, y*4, (x+8)*4, (y+8)*4, 0, ((c-32)%16)*256, ((c-32)/16)*256, 1024, 1024);
		}
		//Move right a character
		x += 8;
	}
}