struct state_t;

extern state_t *InitAndResetChip();
extern void destroyChip(state_t *state);
extern void step(state_t *state);
extern void chipStatus(state_t *state);

extern unsigned short readPC(state_t *state);
extern unsigned char readA(state_t *state);
extern unsigned char readX(state_t *state);
extern unsigned char readY(state_t *state);
extern unsigned char readSP(state_t *state);
extern unsigned char readP(state_t *state);
//extern unsigned int readRW(state_t *state);
extern bool readRW(state_t *state);
extern unsigned short readAddressBus(state_t *state);
extern void writeDataBus(state_t *state, unsigned char);
extern unsigned char readDataBus(state_t *state);
extern unsigned char readIR(state_t *state);

extern unsigned char memory[65536];

extern unsigned int cycle;
extern unsigned int transistors;

class Perfect6502
{
public:
	Perfect6502();
	virtual ~Perfect6502();
};
