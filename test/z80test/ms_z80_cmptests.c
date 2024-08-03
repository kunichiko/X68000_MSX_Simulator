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

int dotest(int max_steps, char* result, bool debug);
int compare(char*);

void dump_cpu() {
	printf("SZ CPU: PC:0x%04x SP:0x%04x\n", sz_cpu.pc, sz_cpu.sp);
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", sz_cpu.a, sz_cpu.b, sz_cpu.c, sz_cpu.d, sz_cpu.e, sz_cpu.h, sz_cpu.l, sz_cpu.ix, sz_cpu.iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x\n", sz_cpu.a_, sz_cpu.b_, sz_cpu.c_, sz_cpu.d_, sz_cpu.e_, sz_cpu.h_, sz_cpu.l_);
	printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", sz_cpu.sf, sz_cpu.zf, sz_cpu.yf, sz_cpu.hf, sz_cpu.xf, sz_cpu.pf, sz_cpu.nf, sz_cpu.cf);
	printf("MS CPU: PC:0x%04x SP:0x%04x\n", ms_cpu.pc, ms_cpu.sp);
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", ms_cpu.a, ms_cpu.b, ms_cpu.c, ms_cpu.d, ms_cpu.e, ms_cpu.h, ms_cpu.l, ms_cpu.ix, ms_cpu.iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x\n", ms_cpu.a_, ms_cpu.b_, ms_cpu.c_, ms_cpu.d_, ms_cpu.e_, ms_cpu.h_, ms_cpu.l_);
	printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", ms_cpu.sf, ms_cpu.zf, ms_cpu.yf, ms_cpu.hf, ms_cpu.xf, ms_cpu.pf, ms_cpu.nf, ms_cpu.cf);
}

int run_specific_test()
{
	if (init_sz_z80(&sz_cpu))
	{
		return 1;
	}
	if (init_ms_z80(&ms_cpu))
	{
		return 1;
	}

	// inject "out 1,a" at 0x0000 (signal to stop the test)
	sz_memory[0x0000] = 0xD3;
	sz_memory[0x0001] = 0x00;

	// inject "in a,0" at 0x0005 (signal to output some characters)
	sz_memory[0x0005] = 0xDB;
	sz_memory[0x0006] = 0x00;
	sz_memory[0x0007] = 0xC9;

	//
	sz_memory[0x0100] = 0xcb; // nop
	sz_memory[0x0101] = 0x40; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	printf("Specific Test:\n");
	// テストコードSZ
	sz_cpu.pc = 0x100;
	sz_cpu.a = 0x01;
	sz_cpu.d = 0xe6;
	sz_cpu.cf = 1;

	char result[256];
	if (dotest(20,result,true) ) {
		printf(" ********************************************************** failed --> %s\n",result);
		dump_cpu();
		return 1;
	}
	printf(" passed\n");
	return 0;
}

int run_through_tests(uint8_t seed)
{
	char result[256];

	if (init_sz_z80(&sz_cpu))
	{
		return 1;
	}
	if (init_ms_z80(&ms_cpu))
	{
		return 1;
	}

	// inject "out 1,a" at 0x0000 (signal to stop the test)
	sz_memory[0x0000] = 0xD3;
	sz_memory[0x0001] = 0x00;

	// inject "in a,0" at 0x0005 (signal to output some characters)
	sz_memory[0x0005] = 0xDB;
	sz_memory[0x0006] = 0x00;
	sz_memory[0x0007] = 0xC9;

	//
	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	//
	sz_cpu.a = seed++;
	sz_cpu.b = seed++;
	sz_cpu.c = seed++;
	sz_cpu.d = seed++;
	sz_cpu.e = seed++;
	sz_cpu.h = seed++;
	sz_cpu.l = seed++;
	sz_cpu.a_ = seed++;
	sz_cpu.b_ = seed++;
	sz_cpu.c_ = seed++;
	sz_cpu.d_ = seed++;
	sz_cpu.e_ = seed++;
	sz_cpu.h_ = seed++;
	sz_cpu.l_ = seed++;
	sz_cpu.sf = seed++;
	sz_cpu.zf = seed++;
	sz_cpu.yf = seed++;
	sz_cpu.hf = seed++;
	sz_cpu.xf = seed++;
	sz_cpu.pf = seed++;
	sz_cpu.nf = seed++;
	sz_cpu.cf = seed++;
	sz_cpu.ix = seed++;
	sz_cpu.iy = seed++;

	for(int i=0;i<0x100;i++)
	{
		if (i == 0x76 || i == 0xcb || i == 0xdd || i== 0xed || i == 0xfd || //
			i == 0xd3 || i == 0xdb)
		{
			continue;
		}
		printf("Test: 0x%02x ... ", i);
		fflush(stdout);
		// テストコードSZ
		sz_cpu.pc = 0x100;
		sz_memory[0x0100] = i;

		if (dotest(20,result,false) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			break;
		}
		printf(" passed\n");
	}

	// dd, fd ラインのテスト
	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;
	for(int j=0xdd;j<=0xfd;j+=0x20)
	{
		for(int i=0;i<0x100;i++)
		{
			if (i == 0x76 || i == 0xcb || i == 0xdd || i== 0xed || i == 0xfd) {
				continue;
			}
			printf("Test: 0x%02x 0x%02x ... ", j, i);
			fflush(stdout);
			// テストコードSZ
			sz_cpu.pc = 0x100;
			sz_memory[0x0100] = j;
			sz_memory[0x0101] = i;
			if (dotest(20,result,false) ) {
				printf(" ********************************************************** failed --> %s\n",result);
				dump_cpu();
				break;
			}
			printf(" passed\n");
		}
	}

	// CB ラインのテスト
	sz_memory[0x0100] = 0xCB; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;
	for(int i=0;i<0x100;i++)
	{
		printf("Test: 0xCB 0x%02x ... ", i);
		fflush(stdout);
		// テストコードSZ
		sz_cpu.pc = 0x100;
		sz_memory[0x0101] = i;
		if (dotest(20,result,false) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			break;
		}
		printf(" passed\n");
	}

	deinit_sz_z80(&sz_cpu);
	deinit_ms_z80(&ms_cpu);

	return 0;
}

int run_func_tests()
{
	if (init_sz_z80(&sz_cpu))
	{
		return 1;
	}
	if (init_ms_z80(&ms_cpu))
	{
		return 1;
	}

	char result[256];
	test_func_t *p = test_funcs;
	while (*p)
	{
		char *test_name = (*p)();
		printf("Test: %s ... \n", test_name);
		if (dotest(0x200,result,false) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			break;
		}
		printf(" passed\n");
		p++;
	}

	deinit_sz_z80(&sz_cpu);
	deinit_ms_z80(&ms_cpu);

	return 0;
}

int main(int argc, char const *argv[])
{
	run_specific_test();
	run_through_tests(0x01);
	run_func_tests();
}

extern volatile bool sz_test_finished;
extern volatile bool ms_test_finished;

int dotest(int max_steps, char* result, bool debug)
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
	ms_cpu.mem_ptr = sz_cpu.mem_ptr;

	for (int i = 0; i < 0x200; i++)
	{
		ms_memory[i] = sz_memory[i];
	}

	/*printf("[initial state]\n");
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", sz_cpu.a, sz_cpu.b, sz_cpu.c, sz_cpu.d, sz_cpu.e, sz_cpu.h, sz_cpu.l, sz_cpu.ix, sz_cpu.iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x\n", sz_cpu.a_, sz_cpu.b_, sz_cpu.c_, sz_cpu.d_, sz_cpu.e_, sz_cpu.h_, sz_cpu.l_);
    printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", sz_cpu.sf, sz_cpu.zf, sz_cpu.yf, sz_cpu.hf, sz_cpu.xf, sz_cpu.pf, sz_cpu.nf, sz_cpu.cf);
	printf("\n");*/

	sz_test_finished = 0;
	ms_test_finished = 0;
	for (int i=0; i< max_steps; i++)
	{
		if (debug)
		{
			printf("[[ Step: %d ]]\n", i);
			dump_cpu();
		}
		z80_step(&sz_cpu);
		ms_z80_step(&ms_cpu);
		if (compare(result))
		{
			return 1;
		}

		if(sz_test_finished || ms_test_finished) break;
	}
	if (debug)
	{
		printf("\n");
	}
	if (sz_test_finished != ms_test_finished)
	{
		sprintf(result, "Test failed: sz_test_finished=%d, ms_test_finished=%d\n", sz_test_finished, ms_test_finished);
		return 1;
	}
	return 0;
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
/*	if (sz_cpu.mem_ptr != ms_cpu.mem_ptr)
	{
		sprintf(result, "mem_ptr (WZ): %04X != %04X\n", sz_cpu.mem_ptr, ms_cpu.mem_ptr);
		return 1;
	}*/

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
	sz_memory[0x0101] = 0xD3;
	sz_memory[0x0102] = 0x00;

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
	sz_memory[0x0104] = 0xD3;
	sz_memory[0x0105] = 0x00;

	return "test_neg";
}