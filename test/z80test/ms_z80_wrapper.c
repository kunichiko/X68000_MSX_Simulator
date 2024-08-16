#include <stdio.h>
#include <stdlib.h>

#include "ms_z80.h"
#include "../../src/ms_R800.h"


unsigned char VDP_R_01;

unsigned char p_FC;
unsigned char p_FD;
unsigned char p_FE;
unsigned char p_FF;

int ms_iomap_init(void);

extern void* ms_mainmem_ptr;
extern int* ms_instructions_ptr;
extern uint8_t *page0;
extern uint8_t *page1;
extern uint8_t *page2;
extern uint8_t *page3;

ms_z80* context;
ms_cpu_state state;

uint8_t *page0_save;
uint8_t *page1_save;
uint8_t *page2_save;
uint8_t *page3_save;

void ms_z80_init(ms_z80* const z) {
	ms_iomap_init();
	context = z;

	page0_save = page0;
	page1_save = page1;
	page2_save = page2;
	page3_save = page3;

	state.pc_base_addr = ms_mainmem_ptr;
/*
	unsigned int* base_ptr = ms_instructions_ptr;
	printf("instructions:\n");
	for(int i=0; i< 64; i++) {
		printf("%08x: ", (int)(base_ptr)+i*16);
		for(int j=0; j< 4; j++) {
			printf("%08x", (unsigned int)(*(base_ptr+j+i*4)));
			printf(" ");
		}
		printf("        : %02x-%02x\n", i*4, i*4+3);
	}*/
}

void ms_z80_step(ms_z80* const z, int steps) {
	//state.pc
	state.pc = z->pc;
	state.sp = z->sp;
	state.ix = z->ix;
	state.iy = z->iy;
	state.wz = z->mem_ptr;
	state.a = z->a;
	state.b = z->b;
	state.c = z->c;
	state.d = z->d;
	state.e = z->e;
	state.h = z->h;
	state.l = z->l;
	state.a_ = z->a_;
	state.b_ = z->b_;
	state.c_ = z->c_;
	state.d_ = z->d_;
	state.e_ = z->e_;
	state.h_ = z->h_;
	state.l_ = z->l_;
	state.f_ = z->f_;
	state.i = z->i;
	state.r = z->r;
	state.f = (z->sf << 7) | (z->zf << 6) | (z->yf << 5) | (z->hf << 4) | (z->xf << 3) | (z->pf << 2) | (z->nf << 1) | (z->cf << 0);
	state.iff_delay = z->iff_delay;
	state.interrupt_mode = z->interrupt_mode;
	ms_cpu_step(&state, steps);
	z->pc = state.pc;
	z->sp = state.sp;
	z->ix = state.ix;
	z->iy = state.iy;
	z->mem_ptr = state.wz;
	z->a = state.a;
	z->b = state.b;
	z->c = state.c;
	z->d = state.d;
	z->e = state.e;
	z->h = state.h;
	z->l = state.l;
	z->a_ = state.a_;
	z->b_ = state.b_;
	z->c_ = state.c_;
	z->d_ = state.d_;
	z->e_ = state.e_;
	z->h_ = state.h_;
	z->l_ = state.l_;
	z->f_ = state.f_;
	z->i = state.i;
	z->r = state.r;
	z->sf = (state.f & 0x80) >> 7;
	z->zf = (state.f & 0x40) >> 6;
	z->yf = (state.f & 0x20) >> 5;
	z->hf = (state.f & 0x10) >> 4;
	z->xf = (state.f & 0x08) >> 3;
	z->pf = (state.f & 0x04) >> 2;
	z->nf = (state.f & 0x02) >> 1;
	z->cf = (state.f & 0x01) >> 0;
	z->iff_delay = state.iff_delay;
	z->interrupt_mode = state.interrupt_mode;

	if(page0 != page0_save) {
		printf("page0 changed: %p -> %p\n", page0_save, page0);
		printf("page0: %p\n", page0);
		printf("page1: %p\n", page1);
		printf("page2: %p\n", page2);
		printf("page3: %p\n", page3);
		page0 = page0_save;
	}
	if(page1 != page1_save) {
		printf("page1 changed: %p -> %p\n", page1_save, page1);
		printf("page0: %p\n", page0);
		printf("page1: %p\n", page1);
		printf("page2: %p\n", page2);
		printf("page3: %p\n", page3);
		//page1 = page1_save;
	}
	if(page2 != page2_save) {
		printf("page2 changed: %p -> %p\n", page2_save, page2);
		printf("page0: %p\n", page0);
		printf("page1: %p\n", page1);
		printf("page2: %p\n", page2);
		printf("page3: %p\n", page3);
		//page2 = page2_save;
	}
	if(page3 != page3_save) {
		printf("page3 changed: %p -> %p\n", page3_save, page3);
		printf("page0: %p\n", page0);
		printf("page1: %p\n", page1);
		printf("page2: %p\n", page2);
		printf("page3: %p\n", page3);
		//page3 = page3_save;
	}
}

void ms_z80_debug_output(ms_z80* const z) {
	int f;
	f = (z->sf << 7) | (z->zf << 6) | (z->yf << 5) | (z->hf << 4) | (z->xf << 3) | (z->pf << 2) | (z->nf << 1) | (z->cf);
	printf("PC: %04x\t%02x\tSP:%04x F:%02x\n", z->pc, z->read_byte(z->userdata, z->pc), z->sp, f);
}

void ms_z80_gen_nmi(ms_z80* const z) {
	printf("z80_gen_nmi: not implemented yet.\n");
	exit(1);
}

void ms_z80_gen_int(ms_z80* const z, uint8_t data) {
	printf("z80_gen_int: not implemented yet.\n");
	exit(1);
};

// ****************:

void read_vdp_0() {}
void read_vdp_1() {}
void read_vdp_2() {}
void read_vdp_3() {}

void write_vdp_0() {}
void write_vdp_1() {}
void write_vdp_2() {}
void write_vdp_3() {}

void r_port_90() {}
void r_port_A0() {}
void r_port_A2() {}
void r_port_A8() {}
void r_port_A9() {}
void r_port_AA() {}
void r_port_AB() {}
void r_port_B4() {}
void r_port_B5() {}
void r_port_FC() {}
void r_port_FD() {}
void r_port_FE() {}
void r_port_FF() {}

void w_port_90() {}
void w_port_91() {}
void w_port_A0() {}
void w_port_A1() {}
void w_port_A2() {}
void w_port_A8() {}
void w_port_A9() {}
void w_port_AA() {}
void w_port_AB() {}
void w_port_B4() {}
void w_port_B5() {}
void w_port_FC() {}
void w_port_FD() {}
void w_port_FE() {}
void w_port_FF() {}



void dumpsp_c(uint8_t *const registers, uint32_t sp, uint32_t a0, uint32_t a4, uint32_t d7, uint32_t d4, uint32_t d6)
{
	for (int i = 0; i < 16; i++)
	{
		printf("%02x ", registers[i]);
	}
	printf("   : SP=%04x a0=%08x a4=%08x d7=%08x d4=%08x d6=%08x\n", sp, a0, a4, d7, d4, d6);
}

