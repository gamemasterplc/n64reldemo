//Library includes
#include <nusys.h>
//Non-Library includes
#include "graphic.h"
#include "main.h"

//Definitions
static u8 bg_tex[] __attribute__((aligned(8))) = {
	//Include background texture data
	#include "stage00bg.inc"
};
static float scroll_x, scroll_y;
static float zoom;

void Stage00Init()
{
	//Reset background view
	scroll_x = scroll_y = 0;
	zoom = 1.0f;
}

void Stage00Update()
{
	//Exit when A button is pressed
	if(contdata[0].trigger & A_BUTTON) {
		ToggleStage();
		return;
	}
	//Zoom with Z and R trigger
	if(contdata[0].button & Z_TRIG) {
		zoom -= 0.01f*zoom;
		if(zoom < 0.25) {
			zoom = 0.25;
		}
	}
	if(contdata[0].button & R_TRIG) {
		zoom += 0.01f*zoom;
		if(zoom > 4) {
			zoom = 4;
		}
	}
	//Scroll background with analog stick
	if(contdata[0].stick_x > 16 || contdata[0].stick_x < -16) {
		scroll_x += contdata[0].stick_x*0.01;
	}
	if(contdata[0].stick_y > 16 || contdata[0].stick_y < -16) {
		scroll_y -= contdata[0].stick_y*0.01;
	}
}

void Stage00Draw()
{
	static Gfx bg_init[] = {
		//Initialize background render settings
		gsDPSetCycleType(G_CYC_1CYCLE),
		gsDPSetRenderMode(G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF),
		gsDPSetDepthSource(G_ZS_PIXEL),
		gsDPSetTexturePersp(G_TP_NONE),
		gsDPSetPrimColor(0 ,0, 255, 255, 255, 255),
		gsDPSetCombineMode(G_CC_MODULATERGBA_PRIM, G_CC_MODULATERGBA_PRIM),
		gsDPSetTextureLUT(G_TT_NONE),
		//Load background texture
		gsDPLoadTextureBlock(bg_tex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 0, G_TX_WRAP, G_TX_WRAP, 5, 5, G_TX_NOLOD, G_TX_NOLOD),
		gsSPEndDisplayList(),
	};
	char text_buf[24];
	GfxClear(0, 0, 0); //Clear screen
	//Print background texture address to show relocation working
	sprintf(text_buf, "bg_tex=0x%08x", bg_tex);
	TextDraw(24, 24, text_buf);
	//Print stage number
	TextDraw(24, SCREEN_HT-32, "stage00");
	//Draw background
	gSPDisplayList(glistp++, bg_init);
    gSPTextureRectangle(glistp++, 0 << 2, 32 << 2, SCREEN_WD << 2, (SCREEN_HT-32) << 2,  G_TX_RENDERTILE,
		(scroll_x-((SCREEN_WD/2)/zoom))*32, (scroll_y-(((SCREEN_HT/2)-32)/zoom))*32, 1024/zoom, 1024/zoom);
    gDPPipeSync(glistp++);
}