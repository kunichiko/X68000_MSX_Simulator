#include <stdio.h>
#include "ms_z80.h"
#include "../../src/ms_R800.h"

extern ms_z80* context;
extern int ms_test_finished;

ms_io_input_func_ptr ms_io_input_table[256];
ms_io_output_func_ptr ms_io_output_table[256];

unsigned char input_default(unsigned char port) {
	if (context) {
		context->b = ms_cpu_registers_ptr->b;
		context->c = ms_cpu_registers_ptr->c;
		context->d = ms_cpu_registers_ptr->d;
		context->e = ms_cpu_registers_ptr->e;
		return context->port_in(context, port);
	} else {
		return 0xff;
	}
}

void output_default(unsigned char port, unsigned char data) {
	if (context) {
		context->b = ms_cpu_registers_ptr->b;
		context->c = ms_cpu_registers_ptr->c;
		context->d = ms_cpu_registers_ptr->d;
		context->e = ms_cpu_registers_ptr->e;
		context->port_out(context, port, data);
		if (ms_test_finished) {
			ms_cpu_step_break = 1;
		}
	}
}

int ms_iomap_init(void) {
	for (int i = 0; i < 256; i++) {
		ms_io_input_table[i] = input_default;
		ms_io_output_table[i] = output_default;
	}
	return 1;
}

void ms_iomap_deinit(void) {
}
