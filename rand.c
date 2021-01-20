#include "rand.h"

static unsigned int rand_seed;

void srand(unsigned int val)
{
	rand_seed = val;
}

int rand()
{
	//Tick seed
	rand_seed = (rand_seed*0x41C64E6D)+0x3039;
	//Generate output value
	return ((rand_seed+1) >> 16) & 0xFFFF;
}