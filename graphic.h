#ifndef GRAPHIC_H
#define GRAPHIC_H

//Required library include
#include <ultra64.h>

//Defines
#define SCREEN_WD 320
#define SCREEN_HT 240
#define GLIST_SIZE 2048

//Declarations
void GfxInit();
void GfxClear(u8 r, u8 g, u8 b);
void GfxEnd();

extern Gfx *glistp;
extern Gfx glist[GLIST_SIZE];

#endif