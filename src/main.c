// MSX.Simulator [[ MS ]]
//   [[ bootstrap ]]
//
// Copyright (C)2024 TcbnErik
// License: MIT

#include <stdio.h>
#include <stdlib.h>
#include <x68k/iocs.h>
#include "ms.h"

#if defined(__mc68020__) || defined(__mc68030__) || defined(__mc68040__) || defined(__mc68060__)
#error "please compile this file with `-m68000` option."
#endif

// IOCS work
#define MPUTYPE ((unsigned char*)0xcbc)

// ms.c
extern int msx_main(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	printf("[[ MSX Simulator MS.X %s]]\n", MS_dot_X_VERSION);

	unsigned int mpu_type = (unsigned int)_iocs_b_bpeek(MPUTYPE);
	if (mpu_type < 3) {
		printf("MS.X ‚Ì“®ì‚É‚Í68030ˆÈã‚ª•K—v‚Å‚·\n");
		return EXIT_FAILURE;
	}

	return msx_main(argc, argv);
}

