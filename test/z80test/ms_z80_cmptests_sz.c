#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "superzazu-z80/z80.h"

// MARK: simple memory interface
#define MEMORY_SIZE 0x10000
uint8_t *sz_memory = NULL;
volatile bool sz_test_finished = 0;

static uint8_t sz_rb(void *userdata, uint16_t addr)
{
	return sz_memory[addr];
}

static void sz_wb(void *userdata, uint16_t addr, uint8_t val)
{
	sz_memory[addr] = val;
}

static uint8_t sz_in(z80 *const z, uint8_t port)
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
			printf("%c", sz_rb(z, addr++));
		} while (sz_rb(z, addr) != '$');
	}
	fflush(stdout);
	return 0xFF;
}

static void sz_out(z80 *const z, uint8_t port, uint8_t val)
{
	sz_test_finished = 1;
}

int init_sz_z80(z80 *const z)
{
	z80_init(z);
	z->read_byte = sz_rb;
	z->write_byte = sz_wb;
	z->port_in = sz_in;
	z->port_out = sz_out;

	sz_memory = malloc(MEMORY_SIZE);
	if (sz_memory == NULL)
	{
		return 1;
	}

	memset(sz_memory, 0, MEMORY_SIZE);

	// inject "out 1,a" at 0x0000 (signal to stop the test)
	sz_memory[0x0000] = 0xD3;
	sz_memory[0x0001] = 0x00;

	// inject "in a,0" at 0x0005 (signal to output some characters)
	sz_memory[0x0005] = 0xDB;
	sz_memory[0x0006] = 0x00;
	sz_memory[0x0007] = 0xC9;

	return 0;
}

void deinit_sz_z80(z80 *const z)
{
	free(sz_memory);
}
