/*
 superzazu/z80 の実装と比較しながら動かすテスト
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "ms_z80.h"
#include "superzazu-z80/z80.h"

int init_sz_z80(z80 *const z);
void deinit_sz_z80(z80 *const z);

int init_ms_z80(ms_z80 *const z);
void deinit_ms_z80(ms_z80 *const z);

z80 sz_cpu;
ms_z80 ms_cpu;

extern uint8_t *sz_memory;
extern uint8_t *ms_memory;

typedef char *(*test_func_t)(void);

char *test_nop();
char *test_neg();

test_func_t test_funcs[] = {test_nop, test_neg, NULL};

void dotest();
int compare(char*);

int main(int argc, char const *argv[])
{
	if (init_sz_z80(&sz_cpu))
	{
		return 1;
	}
	if (init_ms_z80(&ms_cpu))
	{
		return 1;
	}

	test_func_t *p = test_funcs;
	while (*p)
	{
		char *test_name = (*p)();
		printf("Test: %s ... \n", test_name);
		dotest();
		char result[256];
		if (compare(result))
		{
			printf(" failed --> %s\n",result);
			break;
		}
		else
		{
			printf(" passed\n");
		}
		p++;
	}

	deinit_sz_z80(&sz_cpu);
	deinit_ms_z80(&ms_cpu);

	return 0;
}

extern bool sz_test_finished;
extern bool ms_test_finished;

void dotest()
{
	ms_cpu.pc = sz_cpu.pc;
	ms_cpu.sp = sz_cpu.sp;
	ms_cpu.ix = sz_cpu.ix;
	ms_cpu.iy = sz_cpu.iy;
	ms_cpu.a = sz_cpu.a;
	ms_cpu.b = sz_cpu.b;
	ms_cpu.c = sz_cpu.c;
	ms_cpu.d = sz_cpu.d;
	ms_cpu.e = sz_cpu.e;
	ms_cpu.h = sz_cpu.h;
	ms_cpu.l = sz_cpu.l;
	ms_cpu.a_ = sz_cpu.a_;
	ms_cpu.b_ = sz_cpu.b_;
	ms_cpu.c_ = sz_cpu.c_;
	ms_cpu.d_ = sz_cpu.d_;
	ms_cpu.e_ = sz_cpu.e_;
	ms_cpu.h_ = sz_cpu.h_;
	ms_cpu.l_ = sz_cpu.l_;
	ms_cpu.sf = sz_cpu.sf;
	ms_cpu.zf = sz_cpu.zf;
	ms_cpu.yf = sz_cpu.yf;
	ms_cpu.hf = sz_cpu.hf;
	ms_cpu.xf = sz_cpu.xf;
	ms_cpu.pf = sz_cpu.pf;
	ms_cpu.nf = sz_cpu.nf;
	ms_cpu.cf = sz_cpu.cf;

	for (int i = 0; i < 0x200; i++)
	{
		ms_memory[i] = sz_memory[i];
	}

	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x\n", sz_cpu.a, sz_cpu.b, sz_cpu.c, sz_cpu.d, sz_cpu.e, sz_cpu.h, sz_cpu.l);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x\n", sz_cpu.a_, sz_cpu.b_, sz_cpu.c_, sz_cpu.d_, sz_cpu.e_, sz_cpu.h_, sz_cpu.l_);
    printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", sz_cpu.sf, sz_cpu.zf, sz_cpu.yf, sz_cpu.hf, sz_cpu.xf, sz_cpu.pf, sz_cpu.nf, sz_cpu.cf);

	sz_test_finished = 0;
	ms_test_finished = 0;
	while (!sz_test_finished)
	{
		z80_step(&sz_cpu);
	}
	while (!ms_test_finished)
	{
		ms_z80_step(&ms_cpu);
	}
}

int compare(char* result)
{
	// レジスタの比較
	if (sz_cpu.pc != ms_cpu.pc)
	{
		sprintf(result, "pc: %04X != %04X", sz_cpu.pc, ms_cpu.pc);
		return 1;
	}
	if (sz_cpu.sp != ms_cpu.sp)
	{
		sprintf(result, "sp: %04X != %04X\n", sz_cpu.sp, ms_cpu.sp);
		return 1;
	}
	if (sz_cpu.ix != ms_cpu.ix)
	{
		sprintf(result, "ix: %04X != %04X\n", sz_cpu.ix, ms_cpu.ix);
		return 1;
	}
	if (sz_cpu.iy != ms_cpu.iy)
	{
		sprintf(result, "iy: %04X != %04X\n", sz_cpu.iy, ms_cpu.iy);
		return 1;
	}
	if (sz_cpu.a != ms_cpu.a)
	{
		sprintf(result, "a: %02X != %02X\n", sz_cpu.a, ms_cpu.a);
		return 1;
	}
	if (sz_cpu.b != ms_cpu.b)
	{
		sprintf(result, "b: %02X != %02X\n", sz_cpu.b, ms_cpu.b);
		return 1;
	}
	if (sz_cpu.c != ms_cpu.c)
	{
		sprintf(result, "c: %02X != %02X\n", sz_cpu.c, ms_cpu.c);
		return 1;
	}
	if (sz_cpu.d != ms_cpu.d)
	{
		sprintf(result, "d: %02X != %02X\n", sz_cpu.d, ms_cpu.d);
		return 1;
	}
	if (sz_cpu.e != ms_cpu.e)
	{
		sprintf(result, "e: %02X != %02X\n", sz_cpu.e, ms_cpu.e);
		return 1;
	}
	if (sz_cpu.h != ms_cpu.h)
	{
		sprintf(result, "h: %02X != %02X\n", sz_cpu.h, ms_cpu.h);
		return 1;
	}
	if (sz_cpu.l != ms_cpu.l)
	{
		sprintf(result, "l: %02X != %02X\n", sz_cpu.l, ms_cpu.l);
		return 1;
	}
	if (sz_cpu.a_ != ms_cpu.a_)
	{
		sprintf(result, "a': %02X != %02X\n", sz_cpu.a_, ms_cpu.a_);
		return 1;
	}
	if (sz_cpu.b_ != ms_cpu.b_)
	{
		sprintf(result, "b': %02X != %02X\n", sz_cpu.b_, ms_cpu.b_);
		return 1;
	}
	if (sz_cpu.c_ != ms_cpu.c_)
	{
		sprintf(result, "c': %02X != %02X\n", sz_cpu.c_, ms_cpu.c_);
		return 1;
	}
	if (sz_cpu.d_ != ms_cpu.d_)
	{
		sprintf(result, "d': %02X != %02X\n", sz_cpu.d_, ms_cpu.d_);
		return 1;
	}
	if (sz_cpu.e_ != ms_cpu.e_)
	{
		sprintf(result, "e': %02X != %02X\n", sz_cpu.e_, ms_cpu.e_);
		return 1;
	}
	if (sz_cpu.h_ != ms_cpu.h_)
	{
		sprintf(result, "h': %02X != %02X\n", sz_cpu.h_, ms_cpu.h_);
		return 1;
	}
	if (sz_cpu.l_ != ms_cpu.l_)
	{
		sprintf(result, "l': %02X != %02X\n", sz_cpu.l_, ms_cpu.l_);
		return 1;
	}
	if (sz_cpu.sf != ms_cpu.sf)
	{
		sprintf(result, "sf: %d != %d\n", sz_cpu.sf, ms_cpu.sf);
		return 1;
	}
	if (sz_cpu.zf != ms_cpu.zf)
	{
		sprintf(result, "zf: %d != %d\n", sz_cpu.zf, ms_cpu.zf);
		return 1;
	}
	if (sz_cpu.yf != ms_cpu.yf)
	{
		sprintf(result, "yf: %d != %d\n", sz_cpu.yf, ms_cpu.yf);
		return 1;
	}
	if (sz_cpu.hf != ms_cpu.hf)
	{
		sprintf(result, "hf: %d != %d\n", sz_cpu.hf, ms_cpu.hf);
		return 1;
	}
	if (sz_cpu.xf != ms_cpu.xf)
	{
		sprintf(result, "xf: %d != %d\n", sz_cpu.xf, ms_cpu.xf);
		return 1;
	}
	if (sz_cpu.pf != ms_cpu.pf)
	{
		sprintf(result, "pf: %d != %d\n", sz_cpu.pf, ms_cpu.pf);
		return 1;
	}
	if (sz_cpu.nf != ms_cpu.nf)
	{
		sprintf(result, "nf: %d != %d\n", sz_cpu.nf, ms_cpu.nf);
		return 1;
	}
	if (sz_cpu.cf != ms_cpu.cf)
	{
		sprintf(result, "cf: %d != %d\n", sz_cpu.cf, ms_cpu.cf);
		return 1;
	}

	for (int i = 0; i < 0x200; i++)
	{
		if (sz_memory[i] != ms_memory[i])
		{
			sprintf(result, "memory[%04X]: %02X != %02X\n", i, sz_memory[i], ms_memory[i]);
			return 1;
		}
	}
	return 0;
}

//
// Test Cases
//

char *test_nop()
{
	// テストコードSZ
	sz_cpu.pc = 0x100;
	sz_memory[0x0100] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0140] = 0xD3;
	sz_memory[0x0140] = 0x00;

	return "test_nop";
}

char *test_neg()
{
	// テストコードSZ
	sz_cpu.pc = 0x100;
	sz_memory[0x0100] = 0x3e;
	sz_memory[0x0101] = 0xff;
	sz_memory[0x0102] = 0xed; // neg
	sz_memory[0x0103] = 0x44;
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0140] = 0xD3;
	sz_memory[0x0140] = 0x00;

	return "test_neg";
}