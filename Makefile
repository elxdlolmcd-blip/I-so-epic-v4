EE_BIN = CHICKEN.ELF
EE_OBJS = main.o pad.o gfx.o font.o game.o engine.o mapdata.o ui.o audio.o data.o
GSKIT ?= $(PS2DEV)/gsKit
EE_INCS += -I$(GSKIT)/include -I.
EE_CFLAGS += -Wno-missing-braces -Wa,-I$(PS2SDK)/iop/irx
EE_LDFLAGS += -L$(GSKIT)/lib
EE_LIBS = -lgskit -ldmakit -laudsrv -lpatches -lpad -lc -lkernel -lm

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
