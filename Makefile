OBJS=perfect6502.o netlistsim/netlist_sim.o
OBJS+=cbmbasic/cbmbasic.o cbmbasic/runtime.o cbmbasic/runtime_init.o cbmbasic/plugin.o cbmbasic/console.o cbmbasic/emu.o
;OBJS+=measure.o
CFLAGS=-Werror -Wall -O0 -g -Wno-unused-but-set-variable -I. -Inetlistsim
CC=cc

all: cbmbasic

cbmbasic: $(OBJS)
	$(CC) -o cbmbasic/cbmbasic $(OBJS)

clean:
	rm -f $(OBJS) cbmbasic/cbmbasic

