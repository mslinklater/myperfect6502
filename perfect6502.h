struct state_t;

namespace Perfect6502
{
	state_t *InitAndResetChip();
	void destroyChip(state_t *state);
	void step(state_t *state);
	void chipStatus(state_t *state);

	unsigned short readPC(state_t *state);
	unsigned char readA(state_t *state);
	unsigned char readX(state_t *state);
	unsigned char readY(state_t *state);
	unsigned char readSP(state_t *state);
	unsigned char readP(state_t *state);
	//extern unsigned int readRW(state_t *state);
	bool readRW(state_t *state);
	unsigned short readAddressBus(state_t *state);
	void writeDataBus(state_t *state, unsigned char);
	unsigned char readDataBus(state_t *state);
	unsigned char readIR(state_t *state);

	extern unsigned char memory[65536];

	extern unsigned int cycle;
	extern unsigned int transistors;
}


