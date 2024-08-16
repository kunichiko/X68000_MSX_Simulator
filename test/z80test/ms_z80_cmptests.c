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
typedef uint8_t (*value_func_t)(void);

char *test_nop();
char *test_neg();

test_func_t test_funcs[] = {test_nop, test_neg, NULL};

int dotest(int max_steps, char* result, bool debug);
int compare(char*);

void dump_cpu() {
	printf("SZ CPU: PC:0x%04x SP:0x%04x CYC:0x%08x\n", sz_cpu.pc, sz_cpu.sp, sz_cpu.cyc);
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", sz_cpu.a, sz_cpu.b, sz_cpu.c, sz_cpu.d, sz_cpu.e, sz_cpu.h, sz_cpu.l, sz_cpu.ix, sz_cpu.iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x F':%02x\n", sz_cpu.a_, sz_cpu.b_, sz_cpu.c_, sz_cpu.d_, sz_cpu.e_, sz_cpu.h_, sz_cpu.l_, sz_cpu.f_);
	printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", sz_cpu.sf, sz_cpu.zf, sz_cpu.yf, sz_cpu.hf, sz_cpu.xf, sz_cpu.pf, sz_cpu.nf, sz_cpu.cf);
	printf(" (HL): %02x %02x\n", sz_memory[(sz_cpu.h << 8) | sz_cpu.l], sz_memory[(sz_cpu.h << 8) | sz_cpu.l + 1]);
	printf("MS CPU: PC:0x%04x SP:0x%04x CYC:0x%08x\n", ms_cpu.pc, ms_cpu.sp, ms_cpu.cyc);
	printf(" A :%02x B :%02x C :%02x D :%02x E :%02x H :%02x L :%02x IX:%04x IY:%04x\n", ms_cpu.a, ms_cpu.b, ms_cpu.c, ms_cpu.d, ms_cpu.e, ms_cpu.h, ms_cpu.l, ms_cpu.ix, ms_cpu.iy);
	printf(" A':%02x B':%02x C':%02x D':%02x E':%02x H':%02x L':%02x F':%02x\n", ms_cpu.a_, ms_cpu.b_, ms_cpu.c_, ms_cpu.d_, ms_cpu.e_, ms_cpu.h_, ms_cpu.l_, ms_cpu.f_);
	printf(" sf:%d zf:%d yf:%d hf:%d xf:%d pf:%d nf:%d cf:%d\n", ms_cpu.sf, ms_cpu.zf, ms_cpu.yf, ms_cpu.hf, ms_cpu.xf, ms_cpu.pf, ms_cpu.nf, ms_cpu.cf);
	printf(" (HL): %02x %02x\n", ms_memory[(ms_cpu.h << 8) | ms_cpu.l], ms_memory[(ms_cpu.h << 8) | ms_cpu.l + 1]);
}

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

	// テストコード
	sz_memory[0x0100] = 0xdd;
	sz_memory[0x0101] = 0x00;
	sz_memory[0x0102] = 0x00;
	sz_memory[0x0103] = 0xce;
	sz_memory[0x0104] = 0x00;
	sz_memory[0x0105] = 0x00;
	sz_memory[0x0106] = 0x00;
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0107] = 0xD3;
	sz_memory[0x0108] = 0x00;

	sz_memory[0xffff] = 0x33;

	printf("Specific Test:\n");
	// テストコードSZ
	sz_cpu.pc = 0x100;
	sz_cpu.sp = 0xffff;
	sz_cpu.a = 0xb6;
	sz_cpu.b = 0x99;
	sz_cpu.c = 0x9a;
	sz_cpu.d = 0x9a;
	sz_cpu.e = 0x9a;
	sz_cpu.h = 0x9a;
	sz_cpu.l = 0x9b;
	sz_cpu.ix = 0xbc01;
	sz_cpu.iy = 0xbcbc;
	sz_cpu.sf = 0;
	sz_cpu.zf = 1;
	sz_cpu.yf = 0;
	sz_cpu.hf = 0;
	sz_cpu.xf = 0;
	sz_cpu.pf = 1;
	sz_cpu.nf = 0;
	sz_cpu.cf = 0;
	sz_cpu.a_ = 0x02;
	sz_cpu.f_ = 0x02;


	char result[256];
	if (dotest(1,result,true) ) {
		printf(" ********************************************************** failed --> %s\n",result);
		dump_cpu();
		dump_mem(0xffff);
		return 1;
	}
	printf(" passed\n");
	return 0;
}

uint8_t defaultvalue()
{
	return 0x00;
}

void set_registers(value_func_t value_func)
{
	//
	sz_cpu.a = (*value_func)();
	sz_cpu.b = (*value_func)();
	sz_cpu.c = (*value_func)();
	sz_cpu.d = (*value_func)();
	sz_cpu.e = (*value_func)();
	sz_cpu.h = (*value_func)();
	sz_cpu.l = (*value_func)();
	sz_cpu.a_ = (*value_func)();
	sz_cpu.b_ = (*value_func)();
	sz_cpu.c_ = (*value_func)();
	sz_cpu.d_ = (*value_func)();
	sz_cpu.e_ = (*value_func)();
	sz_cpu.h_ = (*value_func)();
	sz_cpu.l_ = (*value_func)();
	sz_cpu.sf = (*value_func)();
	sz_cpu.zf = (*value_func)();
	sz_cpu.yf = (*value_func)();
	sz_cpu.hf = (*value_func)();
	sz_cpu.xf = (*value_func)();
	sz_cpu.pf = (*value_func)();
	sz_cpu.nf = (*value_func)();
	sz_cpu.cf = (*value_func)();
	sz_cpu.ix = (*value_func)();
	sz_cpu.iy = (*value_func)();
}

// 通常の命令のテスト
int through_test_normal(value_func_t value_func)
{
	char result[256];

	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = (*value_func)();
	sz_memory[0x0102] = (*value_func)();
	sz_memory[0x0103] = (*value_func)();
	sz_memory[0x0104] = (*value_func)();
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	set_registers(value_func);

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
		bool debug = false;
		if ( i == 0x27 ) {
			debug = false;
		}
		if (dotest(20,result,debug) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			return 1;
		}
		printf(" passed\n");
	}
	return 0;
}

// バグの多いDAA命令のテスト
int through_test_DAA()
{
	char result[256];

	sz_memory[0x0100] = 0x27; // DAA
	sz_memory[0x0101] = 0x00;
	sz_memory[0x0102] = 0x00;
	sz_memory[0x0103] = 0x00;
	sz_memory[0x0104] = 0x00;
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	for(int i=0;i<0x100;i++)
	{
		for (int c=0;c<4;c++) {
			for (int n=0;n<2;n++) {
				printf("Test DAA: A=0x%02x C=%d H=%d n=%d... ", i, c % 2, c / 2, n);
				fflush(stdout);
				// テストコードSZ
				sz_cpu.pc = 0x100;
				sz_cpu.a = i;
				sz_cpu.c = c % 2;
				sz_cpu.hf = c / 2;
				sz_cpu.nf = n;
				bool debug = false;
				if (dotest(20,result,debug) ) {
					printf(" ********************************************************** failed --> %s\n",result);
					dump_cpu();
					return 1;
				}
				printf(" passed\n");
			}
		}
	}
	return 0;
}

// CB ラインのテスト
int through_test_cb(value_func_t value_func)
{
	char result[256];

	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	set_registers(value_func);

	for(int i=0;i<0x100;i++)
	{
		printf("Test: 0xCB 0x%02x ... ", i);
		fflush(stdout);
		// テストコードSZ
		sz_cpu.pc = 0x100;
		sz_memory[0x0100] = 0xCB; // 破壊されることがあるので書き直す
		sz_memory[0x0101] = i;
		bool debug = false;
		if ( i == 0xfd75 | i == 0x76 ) {
			debug = false;
		}
		if (dotest(20,result,debug) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			return 1;
		}
		printf(" passed\n");
	}
	return 0;
}

// DD CB ラインのテスト
int through_test_ddcb(value_func_t value_func)
{
	char result[256];

	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	set_registers(value_func);

	int d = 0;

	for(int i=0;i<0x100;i++)
	{
		if ( (i & 0x0f) != 0x06 && (i & 0x0f) != 0x0e)
		{
			continue;
		}
		printf("Test: 0xDD 0xCB 0x%02x 0x%02x ... ", d, i);
		fflush(stdout);
		// テストコードSZ
		sz_cpu.pc = 0x100;
		sz_memory[0x0100] = 0xDD;
		sz_memory[0x0101] = 0xCB;
		sz_memory[0x0102] = d;
		sz_memory[0x0103] = i;
		bool debug = false;
		if ( i == 0xc6 | i == 0xce ) {
			debug = false;
		}
		if (dotest(20,result,debug) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			return 1;
		}
		printf(" passed\n");
	}
	return 0;
}

// dd, fd ラインのテスト
int through_test_ddfd(value_func_t value_func)
{
	char result[256];

	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	set_registers(value_func);

	for(int j=0xdd;j<=0xfd;j+=0x20)
	{
		for(int i=0;i<0x100;i++)
		{
			if (i == 0x76 || i == 0xcb || i == 0xdb || i == 0xdd || i== 0xed || i == 0xfd) {
				continue;
			}
			printf("Test: 0x%02x 0x%02x ... ", j, i);
			fflush(stdout);
			// テストコードSZ
			sz_cpu.pc = 0x100;
			sz_memory[0x0100] = j;
			sz_memory[0x0101] = i;
			bool debug = false;
			if ( j == 0xdd && i == 0x00 ) {
				debug = false;
			}
			if (dotest(20,result,debug) ) {
				printf(" ********************************************************** failed --> %s\n",result);
				dump_cpu();
				dump_mem(0xffff);
				dump_mem(0x0100);
				return 1;
			}
			printf(" passed\n");
		}
	}
	return 0;
}

// dd, fd ラインの a,ixhや a,ixlのフルレンジテスト
int through_test_ddfd_alu(value_func_t value_func)
{
	char result[256];
	uint8_t opcodes[] = //
	{
		0x84,0x85,0x8c,0x8d,//
		0x94,0x95,0x9c,0x9d,//
		0xa4,0xa5,0xac,0xad,//
		0xb4,0xb5,0xbc,0xbd,//
		0xff // STOP
	};

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
		int i=0;
		uint8_t opcode;
		while( (opcode = opcodes[i++]) != 0xff )
		{
			for(int k=0;k<0x100;k++)
			{
				int a = (*value_func)() & 0xff;
				int idx = ((*value_func)() & 0xff);
				bool cf = ((*value_func)() & 0x05);
				printf("Test: 0x%02x 0x%02x A=%02x Idx=%02x Cf=%d... ", j, opcode, a, idx, cf);
				fflush(stdout);
				// テストコードSZ
				sz_cpu.pc = 0x100;
				sz_cpu.a = a;
				sz_cpu.cf = cf;
				if(j==0xdd) {
					if((opcode % 2) == 0) {
						sz_cpu.ix = idx << 8;	//IXh
					} else {
						sz_cpu.ix = idx;		//IXl
					}
				} else {
					if((opcode % 2) == 0) {
						sz_cpu.iy = idx << 8;	//IYh
					} else {
						sz_cpu.iy = idx;		//IYl
					}
				}
				sz_memory[0x0100] = j;
				sz_memory[0x0101] = opcode;
				bool debug = false;
				if ( opcode == 0xb9 ) {
					debug = false;
				}
				if (dotest(20,result,debug) ) {
					printf(" ********************************************************** failed --> %s\n",result);
					dump_cpu();
					return 1;
				}
				printf(" passed\n");
			}
		}
	}
	return 0;
}

// ed ラインのテスト
int through_test_ed(value_func_t value_func)
{
	char result[256];
	uint8_t opcodes[] = //
	{
		//0x04,0x0c, //
		//0x14,0x1c, //
		//0x24,0x2c, //
		//0x34,0x3c, //
		0x42,0x43,0x44,0x45,0x46,0x47,0x4a,0x4b,     0x4d,     0x4f, //
		0x52,0x53,          0x56,     0x5a,0x5b,          0x5e,//
		0x62,                    0x67,0x6a,                    0x6f, //
		0x72,0x73,                    0x7a,0x7b,//
		0xa0,0xa1,0xa8,0xa9,//
		0xb0,0xb1,0xb8,0xb9,//
		0xff // STOP
	};
	sz_memory[0x0100] = 0x00; // nop
	sz_memory[0x0101] = 0x00; // nop
	sz_memory[0x0102] = 0x00; // nop
	sz_memory[0x0103] = 0x00; // nop
	sz_memory[0x0104] = 0x00; // nop
	// inject "out 1,a" (signal to stop the test)
	sz_memory[0x0105] = 0xD3;
	sz_memory[0x0106] = 0x00;

	set_registers(value_func);

	int i=0;
	uint8_t opcode;
	while( (opcode = opcodes[i++]) != 0xff )
	{
		printf("Test: 0xED 0x%02x ... ", opcode);
		fflush(stdout);
		// テストコードSZ
		sz_cpu.pc = 0x100;
		sz_memory[0x0100] = 0xed; // 破壊されることがあるので書き直す
		sz_memory[0x0101] = opcode;
		bool debug = false;
		if ( opcode == 0xb9 ) {
			debug = false;
		}
		if (dotest(20,result,debug) ) {
			printf(" ********************************************************** failed --> %s\n",result);
			dump_cpu();
			return 1;
		}
		printf(" passed\n");
	}
	return 0;
}

int run_through_tests(value_func_t value_func)
{
	if (value_func == NULL)
	{
		value_func = defaultvalue;
	} else {
		value_func = value_func;
	}

	printf("Run Through Tests:\n");

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

	if(through_test_ddfd(value_func)) {
		return 1;
	}
	if(through_test_normal(value_func)) {
		return 1;
	}
	if(through_test_cb(value_func)) {
		return 1;
	}
	if(through_test_ed(value_func)) {
		return 1;
	}
	if(through_test_ddcb(value_func)) {
		return 1;
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

uint8_t randomvalue()
{
	static uint8_t value = 0x00;
	do {
		value = (value + 5) + 1;
	} while (value == 0x76 || value == 0xcb || value == 0xdd || value== 0xed || value == 0xfd || //
			value == 0xd3 || value == 0xdb);
	return value;
}

int main(int argc, char const *argv[])
{
	run_specific_test();

	if(run_through_tests(NULL)) {
		return 1;
	}

	if(through_test_ddfd_alu(randomvalue)) {
		return 1;
	}
	if(through_test_DAA()) {
		return 1;
	}
	for( int i=0; i<100; i++) {
		if( run_through_tests(randomvalue) ) {
			break;
		}
	}
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
	ms_cpu.f_ = sz_cpu.f_;
	ms_cpu.sf = sz_cpu.sf;
	ms_cpu.zf = sz_cpu.zf;
	ms_cpu.yf = sz_cpu.yf;
	ms_cpu.hf = sz_cpu.hf;
	ms_cpu.xf = sz_cpu.xf;
	ms_cpu.pf = sz_cpu.pf;
	ms_cpu.nf = sz_cpu.nf;
	ms_cpu.cf = sz_cpu.cf;
	ms_cpu.mem_ptr = sz_cpu.mem_ptr;
	ms_cpu.cyc = sz_cpu.cyc;

	for (int i = 0; i <= 0x1ff; i++)
	{
		ms_memory[i] = sz_memory[i];
	}
	for (int i = 0xff00; i <= 0xffff; i++)
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
			printf("[[ Step: %d ]] OPCODE=0x%02X 0x%02X 0x%02X 0x%02X\n", i, sz_memory[sz_cpu.pc], sz_memory[(sz_cpu.pc+1)&0xffff], sz_memory[(sz_cpu.pc+2)&0xffff], sz_memory[(sz_cpu.pc+3)&0xffff]);
			dump_cpu();
		}
		z80_step(&sz_cpu);
		ms_z80_step(&ms_cpu, 1);
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
	if (sz_cpu.cyc != ms_cpu.cyc)
	{
		sprintf(result, "cyc: %08X != %08X\n", sz_cpu.cyc, ms_cpu.cyc);
		return 1;
	}

	for (int i = 0; i < 0x200; i++)
	{
		if (sz_memory[i] != ms_memory[i])
		{
			sprintf(result, "memory[%04X]: %02X != %02X\n", i, sz_memory[i], ms_memory[i]);
			dump_mem(i);
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