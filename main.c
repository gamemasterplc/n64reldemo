//Library includes
#include <nusys.h>
//Non-Library includes
#include "reloc.h"
#include "rand.h"
#include "main.h"
//Stage includes
#include "stage00.h"
#include "stage01.h"

//File specific macros

//Declare extern symbols for stage generated from spec file
#define STAGE_DECLARE(name) \
extern u8 _##name##SegmentRomStart[]; \
extern u8 _##name##SegmentRomEnd[]; \
extern u8 _##name##SegmentStart[]; \
extern u8 _##name##SegmentEnd[]; \
extern u8 _##name##SegmentReloc[]

//Define stage data
#define STAGE_DEFINE(name, init, update, draw) \
{ \
	(u32)_##name##SegmentRomStart, (u32)_##name##SegmentRomEnd, _##name##SegmentStart, _##name##SegmentEnd, \
	_##name##SegmentReloc, NULL, init, update, draw \
}

//Declarations
STAGE_DECLARE(stage00);
STAGE_DECLARE(stage01);

extern u8 _codeSegmentEnd[];

//Types

//Function types
typedef void (*InitFunc)();
typedef void (*UpdateFunc)();
typedef void (*DrawFunc)();

typedef struct stage_entry {
	//ROM address symbols
	u32 rom_start;
	u32 rom_end;
	//Virtual RAM address symbols
	void *virt_start;
	void *virt_end;
	//Relocation Data Symbol
	void *reloc;
	void *ram_addr; //Load Address
	//Function Symbols
	InitFunc init;
	UpdateFunc update;
	DrawFunc draw;
} StageEntry;

//Definitions

static StageEntry stage_list[] = {
	STAGE_DEFINE(stage00, Stage00Init, Stage00Update, Stage00Draw),
	STAGE_DEFINE(stage01, Stage01Init, Stage01Update, Stage01Draw)
};

static volatile int stage_num = 0;
static volatile int last_stage_num = -1;
static InitFunc stage_init;
static UpdateFunc stage_update;
static DrawFunc stage_draw;

NUContData contdata[NU_CONT_MAXCONTROLLERS];

static void VsyncCB(u32 num_tasks)
{
	rand(); //Tick RNG every frame
	//Only update if no tasks remain
	if(num_tasks == 0) {
		nuContDataGetExAll(contdata); //Read controller
		//Update the stage
		stage_update();
		//Render the stage
		GfxInit();
		stage_draw();
		GfxEnd();
	}
}

static void LoadStage()
{
	//Generate a random load address for the stage overlay
	void *load_addr = _codeSegmentEnd+((rand() & 0xFFF)*0x100);
	//Load Stage Overlay
	stage_list[stage_num].ram_addr = load_addr;
	RelocSegLoad(stage_list[stage_num].rom_start, stage_list[stage_num].rom_end, stage_list[stage_num].virt_start,
		stage_list[stage_num].virt_end, stage_list[stage_num].reloc, load_addr);
	//Grab pointers to functions in stage overlay
	stage_init = (InitFunc)RELOC_GET_RAM_ADDR(stage_list[stage_num].init, stage_list[stage_num].virt_start, load_addr);
	stage_update = (UpdateFunc)RELOC_GET_RAM_ADDR(stage_list[stage_num].update, stage_list[stage_num].virt_start, load_addr);
	stage_draw = (DrawFunc)RELOC_GET_RAM_ADDR(stage_list[stage_num].draw, stage_list[stage_num].virt_start, load_addr);
	stage_init(); //Initialize stage
}

void ToggleStage()
{
	//Flip stage number
	stage_num = last_stage_num^1;
}

void mainproc(void * dummy)
{
	//Initialize program
	srand(osGetCount());
	nuContInit();
    nuGfxInit();
    nuGfxFuncSet((NUGfxFunc)VsyncCB);
    while(1) {
		//Begin stage
		LoadStage();
		//Transition stage
		stage_list[last_stage_num].ram_addr = NULL;
		last_stage_num = stage_num;
		stage_num = -1;
		nuGfxDisplayOn();
		while(stage_num == -1); //Wait for stage transition
		//Turn off screen to hide transition
		nuGfxDisplayOff();
	}
}
