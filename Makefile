N64KITDIR	= c:/nintendo/n64kit

TARGET		= reldemo
CODEFILES	= main.c text.c graphic.c rand.c reloc.c 
#List of files that generate relocation data
RELCODEFILES = stage00.c stage01.c

include $(ROOT)/usr/include/make/PRdefs

#Compiler settings
DEBUGSYM	= -g
OPTIMIZER	= -O0

#nusys library definitions
NUSYSDIR	= $(N64KITDIR)/nusys
NUSYSINC	= $(NUSYSDIR)/include
NUSYSLIB	= $(NUSYSDIR)/lib
NUOBJ		= $(NUSYSLIB)/nusys.o

#Objects linked into code segment
CODEOBJECTS	= $(CODEFILES:.c=.o) $(NUOBJ)

#Allow F3DEX2 usage
CUSTFLAGS	= -DF3DEX_GBI_2
LCINCS		= -I$(NUSYSINC)
LCOPTS		= -G 0 $(DEBUGSYM) $(CUSTFLAGS)

#Symbols that must not be deleted when linking code segment
FORCELINK = -u nuContDataGetExAll \
-u sprintf

LDFLAGS		= $(FORCELINK) -L$(ROOT)/usr/lib -L$(ROOT)/usr/lib/PR -L$(NUSYSLIB) -lnusys_d -lgultra_d -L$(GCCDIR)/mipse/lib -lkmc

CODESEGMENT	= codesegment.o
RELOBJECTS =  $(RELCODEFILES:.c=.o)
#.rel files are treated as objects here
RELFILES = $(RELOBJECTS:.o=.rel)
OBJECTS		= $(CODESEGMENT) $(RELOBJECTS) $(RELFILES)

SYMBOL	= $(TARGET).out
ROM	    = $(TARGET).n64

#Add suffix for relocation data
.SUFFIXES: .rel

default: $(ROM)

#Code segment object file rule
$(CODESEGMENT):	$(CODEOBJECTS)
		$(LD) -o $(CODESEGMENT) -r $(CODEOBJECTS) $(LDFLAGS)

#ROM build rule
$(ROM):	$(OBJECTS)
	$(MAKEROM) spec -I$(NUSYSINC) -r $(ROM) -e $(SYMBOL)
	makemask $(ROM)
	
#Relocation data generation
.o.rel:
	tools/makereloc.exe $< $@
	
.c.o:
	$(CC) $(CFLAGS) $<

#Extended clean and clobber rule to delete .rel files

clean:
ifeq	($(GCC_CELF),ON)
#	deltree /Y *.o $(LDIRT) __elfcom.tbl __elfcom.elf
	command.com /c for %i in (*.o *.rel __elfcom.elf __elfcom.tbl) do if exist %i del %i
	command.com /c for %i in ($(LDIRT)) do if exist %i del %i
else
#	deltree /Y *.o $(LDIRT)
	command.com /c for %i in (*.o *.rel) do if exist %i del %i
	command.com /c for %i in ($(LDIRT)) do if exist %i del %i
endif

clobber:
ifeq	($(GCC_CELF),ON)
#	deltree /Y *.o $(LDIRT) __elfcom.tbl __elfcom.elf
	command.com /c for %i in (*.o *.rel *.n64 *.out __elfcom.elf __elfcom.tbl) do if exist %i del %i
	command.com /c for %i in ($(LDIRT)) do if exist %i del %i
else
#	deltree /Y *.o $(LDIRT)
	command.com /c for %i in (*.o *.rel *.n64 *.out) do if exist %i del %i
	command.com /c for %i in ($(LDIRT)) do if exist %i del %i
endif
