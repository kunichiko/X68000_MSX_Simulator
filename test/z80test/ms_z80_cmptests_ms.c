#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ms_z80.h"

extern uint8_t *ms_mainmem_ptr;

#define MEMORY_SIZE 0x10000
uint8_t *ms_memory = NULL;
bool ms_test_finished = 0;

static uint8_t ms_rb(void *userdata, uint16_t addr)
{
	return ms_memory[addr];
}

static void ms_wb(void *userdata, uint16_t addr, uint8_t val)
{
	ms_memory[addr] = val;
}

static uint8_t ms_in(ms_z80 *const z, uint8_t port)
{
	uint8_t operation = z->c;

	// print a character stored in E
	if (operation == 2)
	{
		printf("%c", z->e);
	}
	// print from memory at (DE) until '$' char
	else if (operation == 9)
	{
		uint16_t addr = (z->d << 8) | z->e;
		do
		{
			printf("%c", ms_rb(z, addr++));
		} while (ms_rb(z, addr) != '$');
	}
	fflush(stdout);
	return 0xFF;
}

static void ms_out(ms_z80 *const z, uint8_t port, uint8_t val)
{
	ms_test_finished = 1;
}

int init_ms_z80(ms_z80 *const z)
{
	ms_z80_init(z);
	z->read_byte = ms_rb;
	z->write_byte = ms_wb;
	z->port_in = ms_in;
	z->port_out = ms_out;

	ms_memory = ms_mainmem_ptr;

	memset(ms_memory, 0, MEMORY_SIZE);

	// inject "out 1,a" at 0x0000 (signal to stop the test)
	ms_memory[0x0000] = 0xD3;
	ms_memory[0x0001] = 0x00;

	// inject "in a,0" at 0x0005 (signal to output some characters)
	ms_memory[0x0005] = 0xDB;
	ms_memory[0x0006] = 0x00;
	ms_memory[0x0007] = 0xC9;

	return 0;
}

void deinit_ms_z80(ms_z80 *const z)
{
}