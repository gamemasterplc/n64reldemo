#include <nusys.h>

//Define Code Segment
beginseg
    name    "code"
    flags   BOOT OBJECT
    entry   nuBoot
    address NU_SPEC_BOOT_ADDR
    stack   NU_SPEC_BOOT_STACK
    include "codesegment.o"
	include "$(ROOT)/usr/lib/PR/rspboot.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
    include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
    include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
	include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"
endseg

//Start relocatable segment definitions

//The first relocatable segment must have an address of 0x80800000
//Relocatable modules Must have a rel file Included as it Contains the Relocation Data
beginseg
    name "stage00"
	flags OBJECT
	address 0x80800000
	include "stage00.o"
	include "stage00.rel"
endseg

//Each relocatable segment must have a unique virtual address
//This is accomplished here with the after keyword
beginseg
    name "stage01"
	flags OBJECT
	after stage00
	include "stage01.o"
	include "stage01.rel"
endseg

//Define Symbol Table for ELF
beginwave
    name    "rom"
    include "code"
	include "stage00"
	include "stage01"
endwave