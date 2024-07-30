#ifndef _MS_R800_H_
#define _MS_R800_H_

#include <stdint.h>

typedef struct ms_cpu_state ms_cpu_state;
struct ms_cpu_state {
	void* pc_base_addr;							// 0-3
	uint16_t pc, sp, ix, iy;					// (4,5),(6,7),(8,9),(10,11)
	uint8_t a, b, c, d, e, h, l;				// 12,13,14,15,16,17,18
	uint8_t a_, b_, c_, d_, e_, h_, l_, f_;		// 19,20,21,22,23,24,25,26
	uint8_t i, r;								// 27,28

	uint8_t f;									// 29

	uint16_t wz;								// 30,31

	uint8_t iff_delay;							// 32

	uint8_t interrupt_mode;  // 0: IM0, 1: IM1, 2: IM2

	uint8_t halted;	// 0: normal, 1: halted
};

typedef struct ms_cpu_registers ms_cpu_registers;
struct ms_cpu_registers {
	uint8_t a, f, b, c, d, e, h, l;
	uint8_t ix_h, ix_l, iy_h, iy_l;
	uint16_t sp;
	uint8_t a_, f_, b_, c_, d_, e_, h_, l_;
	uint8_t r,i;
	unit8_t w,z;
};

extern ms_cpu_registers *ms_cpu_registers_ptr;

void ms_cpu_init(void);
int ms_cpu_emulate( int(*)(unsigned int, unsigned int));

void ms_cpu_step(ms_cpu_state* z, int steps);

extern int ms_cpu_step_break;

typedef unsigned char (*ms_io_input_func_ptr)(unsigned char port);
typedef void (*ms_io_output_func_ptr)(unsigned char port, unsigned char data);

extern ms_io_input_func_ptr ms_io_input_table[256];
extern ms_io_output_func_ptr ms_io_output_table[256];

#endif // _MS_R800_H_