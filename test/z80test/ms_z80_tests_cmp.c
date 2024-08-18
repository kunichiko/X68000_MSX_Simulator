#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ms_z80.h"
#include "superzazu-z80/z80.h"

// MARK: simple memory interface
#define MEMORY_SIZE 0x10000
static uint8_t *ms_memory = NULL;
static uint8_t *sz_memory = NULL;
bool ms_test_finished = 0;
bool sz_test_finished = 0;

// addrで指定された前後16バイトをダンプする
void dump_mem(int addr) {
	for(int j=-1;j<2;j++)
	{
		printf("SZ 0x%04x: ",(addr+j*16)&0xfff0);
		for(int k=0;k<16;k++)
		{
			printf("%02x ",sz_memory[((addr+j*16)&0xfff0)+k]);
		}
		printf("\n");
	}
	printf("--------\n");
	for(int j=-1;j<2;j++)
	{
		printf("MS 0x%04x: ",(addr+j*16)&0xfff0);
		for(int k=0;k<16;k++)
		{
			printf("%02x ",ms_memory[((addr+j*16)&0xfff0)+k]);
		}
		printf("\n");
	}
}

void dump_cpu(z80 *const sz_z, ms_z80 *const ms_z) {
	printf("[[ Next ]] OPCODE=0x%02X 0x%02X 0x%02X 0x%02X\n", sz_memory[sz_z->pc], sz_memory[(sz_z->pc+1)&0xffff], sz_memory[(sz_z->pc+2)&0xffff], sz_memory[(sz_z->pc+3)&0xffff]);
	printf("SZ CPU: PC:0x%04x SP:0x%04x CYC:0x%08x\n", sz_z->pc, sz_z->sp, sz_z->cyc);
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", sz_z->a, sz_z->b, sz_z->c, sz_z->d, sz_z->e, sz_z->h, sz_z->l, sz_z->ix, sz_z->iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x F':%02x\n", sz_z->a_, sz_z->b_, sz_z->c_, sz_z->d_, sz_z->e_, sz_z->h_, sz_z->l_, sz_z->f_);
	printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", sz_z->sf, sz_z->zf, sz_z->yf, sz_z->hf, sz_z->xf, sz_z->pf, sz_z->nf, sz_z->cf);
	printf(" (HL): %02x %02x\n", sz_memory[(sz_z->h << 8) | sz_z->l], sz_memory[(sz_z->h << 8) | sz_z->l + 1]);
	printf("MS CPU: PC:0x%04x SP:0x%04x CYC:0x%08x\n", ms_z->pc, ms_z->sp, ms_z->cyc);
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", ms_z->a, ms_z->b, ms_z->c, ms_z->d, ms_z->e, ms_z->h, ms_z->l, ms_z->ix, ms_z->iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x F':%02x\n", ms_z->a_, ms_z->b_, ms_z->c_, ms_z->d_, ms_z->e_, ms_z->h_, ms_z->l_, ms_z->f_);
	printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", ms_z->sf, ms_z->zf, ms_z->yf, ms_z->hf, ms_z->xf, ms_z->pf, ms_z->nf, ms_z->cf);
	printf(" (HL): %02x %02x\n", ms_memory[(ms_z->h << 8) | ms_z->l], ms_memory[(ms_z->h << 8) | ms_z->l + 1]);
}

static uint8_t ms_rb(void *userdata, uint16_t addr)
{
	return ms_memory[addr];
}

static uint8_t sz_rb(void *userdata, uint16_t addr)
{
	return sz_memory[addr];
}

static void ms_wb(void *userdata, uint16_t addr, uint8_t val)
{
	ms_memory[addr] = val;
}

static void sz_wb(void *userdata, uint16_t addr, uint8_t val)
{
	sz_memory[addr] = val;
}

static int load_file(const char *filename, uint16_t addr, uint8_t *memory)
{
	FILE *f = fopen(filename, "rb");
	if (f == NULL)
	{
		fprintf(stderr, "error: can't open file '%s'.\n", filename);
		return 1;
	}

	// file size check:
	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	rewind(f);

	if (file_size + addr >= MEMORY_SIZE)
	{
		fprintf(stderr, "error: file %s can't fit in memory.\n", filename);
		return 1;
	}

	// copying the bytes in memory:
	size_t result = fread(&memory[addr], sizeof(uint8_t), file_size, f);
	if (result != file_size)
	{
		fprintf(stderr, "error: while reading file '%s'\n", filename);
		return 1;
	}

	fclose(f);
	return 0;
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

static void ms_out(ms_z80 *const z, uint8_t port, uint8_t val)
{
	ms_test_finished = 1;
}

static void sz_out(z80 *const z, uint8_t port, uint8_t val)
{
	sz_test_finished = 1;
}

int compare(ms_z80 *const ms_z, z80 *const sz_z, char* result)
{
	// レジスタの比較
	if (sz_z->pc != ms_z->pc)
	{
		sprintf(result, "pc: %04X != %04X", sz_z->pc, ms_z->pc);
		return 1;
	}
	if (sz_z->sp != ms_z->sp)
	{
		sprintf(result, "sp: %04X != %04X\n", sz_z->sp, ms_z->sp);
		return 1;
	}
	if (sz_z->ix != ms_z->ix)
	{
		sprintf(result, "ix: %04X != %04X\n", sz_z->ix, ms_z->ix);
		return 1;
	}
	if (sz_z->iy != ms_z->iy)
	{
		sprintf(result, "iy: %04X != %04X\n", sz_z->iy, ms_z->iy);
		return 1;
	}
	if (sz_z->a != ms_z->a)
	{
		sprintf(result, "a: %02X != %02X\n", sz_z->a, ms_z->a);
		return 1;
	}
	if (sz_z->b != ms_z->b)
	{
		sprintf(result, "b: %02X != %02X\n", sz_z->b, ms_z->b);
		return 1;
	}
	if (sz_z->c != ms_z->c)
	{
		sprintf(result, "c: %02X != %02X\n", sz_z->c, ms_z->c);
		return 1;
	}
	if (sz_z->d != ms_z->d)
	{
		sprintf(result, "d: %02X != %02X\n", sz_z->d, ms_z->d);
		return 1;
	}
	if (sz_z->e != ms_z->e)
	{
		sprintf(result, "e: %02X != %02X\n", sz_z->e, ms_z->e);
		return 1;
	}
	if (sz_z->h != ms_z->h)
	{
		sprintf(result, "h: %02X != %02X\n", sz_z->h, ms_z->h);
		return 1;
	}
	if (sz_z->l != ms_z->l)
	{
		sprintf(result, "l: %02X != %02X\n", sz_z->l, ms_z->l);
		return 1;
	}
	if (sz_z->a_ != ms_z->a_)
	{
		sprintf(result, "a': %02X != %02X\n", sz_z->a_, ms_z->a_);
		return 1;
	}
	if (sz_z->b_ != ms_z->b_)
	{
		sprintf(result, "b': %02X != %02X\n", sz_z->b_, ms_z->b_);
		return 1;
	}
	if (sz_z->c_ != ms_z->c_)
	{
		sprintf(result, "c': %02X != %02X\n", sz_z->c_, ms_z->c_);
		return 1;
	}
	if (sz_z->d_ != ms_z->d_)
	{
		sprintf(result, "d': %02X != %02X\n", sz_z->d_, ms_z->d_);
		return 1;
	}
	if (sz_z->e_ != ms_z->e_)
	{
		sprintf(result, "e': %02X != %02X\n", sz_z->e_, ms_z->e_);
		return 1;
	}
	if (sz_z->h_ != ms_z->h_)
	{
		sprintf(result, "h': %02X != %02X\n", sz_z->h_, ms_z->h_);
		return 1;
	}
	if (sz_z->l_ != ms_z->l_)
	{
		sprintf(result, "l': %02X != %02X\n", sz_z->l_, ms_z->l_);
		return 1;
	}
	if (sz_z->sf != ms_z->sf)
	{
		sprintf(result, "sf: %d != %d\n", sz_z->sf, ms_z->sf);
		return 1;
	}
	if (sz_z->zf != ms_z->zf)
	{
		sprintf(result, "zf: %d != %d\n", sz_z->zf, ms_z->zf);
		return 1;
	}
	if (sz_z->yf != ms_z->yf)
	{
		sprintf(result, "yf: %d != %d\n", sz_z->yf, ms_z->yf);
		return 1;
	}
	if (sz_z->hf != ms_z->hf)
	{
		sprintf(result, "hf: %d != %d\n", sz_z->hf, ms_z->hf);
		return 1;
	}
	if (sz_z->xf != ms_z->xf)
	{
		sprintf(result, "xf: %d != %d\n", sz_z->xf, ms_z->xf);
		return 1;
	}
	if (sz_z->pf != ms_z->pf)
	{
		sprintf(result, "pf: %d != %d\n", sz_z->pf, ms_z->pf);
		return 1;
	}
	if (sz_z->nf != ms_z->nf)
	{
		sprintf(result, "nf: %d != %d\n", sz_z->nf, ms_z->nf);
		return 1;
	}
	if (sz_z->cf != ms_z->cf)
	{
		sprintf(result, "cf: %d != %d\n", sz_z->cf, ms_z->cf);
		return 1;
	}
/*	if (sz_z->mem_ptr != ms_z->mem_ptr)
	{
		sprintf(result, "mem_ptr (WZ): %04X != %04X\n", sz_z->mem_ptr, ms_z->mem_ptr);
		return 1;
	}*/
	if (sz_z->cyc != ms_z->cyc)
	{
		sprintf(result, "cyc: %08X != %08X\n", sz_z->cyc, ms_z->cyc);
		return 1;
	}

	for (int i = -0x100; i < 0x100; i++)
	{
		int addr = (((unsigned int)i) & 0xffff);
		if (sz_memory[addr] != ms_memory[addr])
		{
			sprintf(result, "memory[%04X]: %02X != %02X\n", i, sz_memory[addr], ms_memory[addr]);
			dump_mem(i);
			return 1;
		}

	}
	for (int i = - 0x40; i < 0x40; i++)
	{
		int addr = (((unsigned int)(sz_z->sp + i)) & 0xffff);
		if (sz_memory[addr] != ms_memory[addr])
		{
			sprintf(result, "memory[%04X]: %02X != %02X\n", i, sz_memory[addr], ms_memory[addr]);
			dump_mem(i);
			return 1;
		}
	}
	return 0;
}

// MARK: test runner
static int run_test(
	z80 *const sz_z, ms_z80 *const ms_z, const char *filename, unsigned long cyc_expected)
{
	z80_init(sz_z);
	sz_z->read_byte = sz_rb;
	sz_z->write_byte = sz_wb;
	sz_z->port_in = sz_in;
	sz_z->port_out = sz_out;
	memset(sz_memory, 0, MEMORY_SIZE);
	if (load_file(filename, 0x100, sz_memory) != 0)
	{
		return 1;
	}

	ms_z80_init(ms_z);
	ms_z->read_byte = ms_rb;
	ms_z->write_byte = ms_wb;
	ms_z->port_in = ms_in;
	ms_z->port_out = ms_out;
	memset(ms_memory, 0, MEMORY_SIZE);
	if (load_file(filename, 0x100, ms_memory) != 0)
	{
		return 1;
	}


	printf("*** TEST: %s\n", filename);

	sz_z->pc = 0x100;
	sz_z->sp = 0xF000;
	sz_z->cyc = 0;

	ms_z->pc = sz_z->pc;
	ms_z->sp = sz_z->sp;
	ms_z->ix = sz_z->ix;
	ms_z->iy = sz_z->iy;
	ms_z->a = sz_z->a;
	ms_z->b = sz_z->b;
	ms_z->c = sz_z->c;
	ms_z->d = sz_z->d;
	ms_z->e = sz_z->e;
	ms_z->h = sz_z->h;
	ms_z->l = sz_z->l;
	ms_z->a_ = sz_z->a_;
	ms_z->b_ = sz_z->b_;
	ms_z->c_ = sz_z->c_;
	ms_z->d_ = sz_z->d_;
	ms_z->e_ = sz_z->e_;
	ms_z->h_ = sz_z->h_;
	ms_z->l_ = sz_z->l_;
	ms_z->f_ = sz_z->f_;
	ms_z->sf = sz_z->sf;
	ms_z->zf = sz_z->zf;
	ms_z->yf = sz_z->yf;
	ms_z->hf = sz_z->hf;
	ms_z->xf = sz_z->xf;
	ms_z->pf = sz_z->pf;
	ms_z->nf = sz_z->nf;
	ms_z->cf = sz_z->cf;
	ms_z->mem_ptr = sz_z->mem_ptr;
	ms_z->cyc = sz_z->cyc;

	// inject "out 1,a" at 0x0000 (signal to stop the test)
	sz_memory[0x0000] = 0xD3;
	sz_memory[0x0001] = 0x00;
	ms_memory[0x0000] = 0xD3;
	ms_memory[0x0001] = 0x00;

	// inject "in a,0" at 0x0005 (signal to output some characters)
	sz_memory[0x0005] = 0xDB;
	sz_memory[0x0006] = 0x00;
	sz_memory[0x0007] = 0xC9;
	ms_memory[0x0005] = 0xDB;
	ms_memory[0x0006] = 0x00;
	ms_memory[0x0007] = 0xC9;

	long nb_instructions = 0;

	char result[256];

	ms_test_finished = 0;
	sz_test_finished = 0;
	while (!ms_test_finished && !sz_test_finished)
	{
		nb_instructions += 1;

		// warning: the following line will output dozens of GB of data.
		// dump_cpu(sz_z, ms_z);

		z80_step(sz_z);
		ms_z80_step(ms_z, 1);

		if (compare(ms_z, sz_z, result) != 0)
		{
			printf("error: %s\n", result);
			dump_cpu(sz_z, ms_z);
			return 1;
		}

		if (nb_instructions % 10000000 == 0)
		{
			//printf("nb_instructions=%ld\n", nb_instructions);
			printf(".");
			fflush(stdout);
		}
	}

	long long diff = cyc_expected - ms_z->cyc;
	printf("\n*** %lu instructions executed on %lu cycles"
		   " (expected=%lu, diff=%lld)\n\n",
		   nb_instructions, ms_z->cyc, cyc_expected, diff);

	return cyc_expected != ms_z->cyc;
}

extern void *ms_mainmem_ptr;
int main(void)
{
	sz_memory = malloc(MEMORY_SIZE);
	if (sz_memory == NULL) {
		return 1;
	 }
	ms_memory = ms_mainmem_ptr;

	z80 sz_z;
	ms_z80 ms_z;

	// the following cycle counts have been retrieved from z80emu
	// (https://github.com/anotherlin/z80emu) for those exact roms
	int r = 0;
	//r += run_test(&sz_z, &ms_z, "roms/prelim.com", 8721LU);
	r += run_test(&sz_z, &ms_z, "roms/zexdoc.cim", UINT32_MAX);
	// r += run_test(&cpu, "roms/zexall.cim", UINT32_MAX);

	//  free(memory);
	return r != 0;
}
