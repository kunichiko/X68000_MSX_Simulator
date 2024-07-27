//
// Modified from: https://github.com/superzazu/z80/blob/master/z80.h
//
#ifndef MS_Z80_H_
#define MS_Z80_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ms_z80 ms_z80;
struct ms_z80 {
  uint8_t (*read_byte)(void*, uint16_t);
  void (*write_byte)(void*, uint16_t, uint8_t);
  uint8_t (*port_in)(ms_z80*, uint8_t);
  void (*port_out)(ms_z80*, uint8_t, uint8_t);
  void* userdata;

  unsigned long cyc; // cycle count (t-states)

  uint16_t pc, sp, ix, iy; // special purpose registers
  uint16_t mem_ptr; // "wz" register
  uint8_t a, b, c, d, e, h, l; // main registers
  uint8_t a_, b_, c_, d_, e_, h_, l_, f_; // alternate registers
  uint8_t i, r; // interrupt vector, memory refresh

  // flags: sign, zero, yf, half-carry, xf, parity/overflow, negative, carry
  bool sf : 1, zf : 1, yf : 1, hf : 1, xf : 1, pf : 1, nf : 1, cf : 1;

  uint8_t iff_delay;
  uint8_t interrupt_mode;
  uint8_t int_data;
  bool iff1 : 1, iff2 : 1;
  bool halted : 1;
  bool int_pending : 1, nmi_pending : 1;
};

void ms_z80_init(ms_z80* const z);
void ms_z80_step(ms_z80* const z);
void ms_z80_debug_output(ms_z80* const z);
void ms_z80_gen_nmi(ms_z80* const z);
void ms_z80_gen_int(ms_z80* const z, uint8_t data);

#endif // MS_Z80_H_
