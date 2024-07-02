/*

		‚l‚r‚w.‚r‚‰‚‚•‚Œ‚‚”‚…‚’ [[ MS ]]
				[[ƒXƒ^[ƒgƒAƒbƒvƒvƒƒOƒ‰ƒ€ ]]
								
	ver. 0.01	prpgramed by Kuni.
										1995.9.15

*/

#include <stdio.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>
#include "ms.h"

#define	segments	4

void main() {
void *MMem,*VideoRAM,*MainROM1,*MainROM2,*SUBROM;
int a,i;

	if ( _iocs_b_super( 0) < 0)
		;

	initialize();								/* ƒVƒXƒeƒ€‚Ì‰Šú‰»				*/

	MMem = new_malloc( 64*1024 + 8*segments);	/* ‚U‚S‚j + ‚WƒoƒCƒg–‘ƒZƒOƒƒ“ƒg”	*/
	if( MMem > (void *)0x81000000 ) {
 		printf("ƒƒ‚ƒŠ‚ªŠm•Û‚Å‚«‚Ü‚¹‚ñ\n");
		ms_exit();
	}
	MMemSet( MMem, (int)segments);					/* ƒAƒZƒ“ƒuƒ‰‚Ìƒ‹[ƒ`ƒ“‚Öˆø‚«“n‚µ		*/

	VideoRAM = new_malloc( 128*1024);		/* ‚u‚q‚`‚l ‚P‚Q‚W‚j 					*/
	if( VideoRAM > (void *)0x81000000 ) {
 		printf("ƒƒ‚ƒŠ‚ªŠm•Û‚Å‚«‚Ü‚¹‚ñ\n");
		ms_exit();
	}
	VDPSet( VideoRAM);						/* ƒAƒZƒ“ƒuƒ‰‚Ìƒ‹[ƒ`ƒ“‚Öˆø‚«“n‚µ		*/
											/* ‰æ–Ê‚Ì‰Šú‰»“™						*/

	MainROM1 = new_malloc( 16*1024+8);		/* ‚l‚`‚h‚m‚q‚n‚li‘O”¼j ‚P‚U‚j 		*/
	if( MainROM1 > (void *)0x81000000 ) {
 		printf("ƒƒ‚ƒŠ‚ªŠm•Û‚Å‚«‚Ü‚¹‚ñ\n");
		ms_exit();
	}
	SetROM( MainROM1,"MAINROM1.ROM", (int)2, (int)0x00, (int)0 );
	
	MainROM2 = new_malloc( 16*1024+8);		/* ‚l‚`‚h‚m‚q‚n‚li‘O”¼j ‚P‚U‚j 		*/
	if( MainROM2 > (void *)0x81000000 ) {
 		printf("ƒƒ‚ƒŠ‚ªŠm•Û‚Å‚«‚Ü‚¹‚ñ\n");
		ms_exit();
	}
	SetROM( MainROM2,"MAINROM2.ROM", (int)2, (int)0x00, (int)1 );

	SUBROM = new_malloc( 16*1024+8);		/* ‚r‚t‚a‚q‚n‚l ‚P‚U‚j 					*/
	if( SUBROM > (void *)0x81000000 ) {
 		printf("ƒƒ‚ƒŠ‚ªŠm•Û‚Å‚«‚Ü‚¹‚ñ\n");
		ms_exit();
	}
	SetROM( SUBROM,"SUBROM.ROM", (int)2, (int)0x0d, (int)0 );
											
											
	if( PSG_INIT() != 0)
		printf("‚o‚r‚f‚Ì‰Šú‰»‚É¸”s‚µ‚Ü‚µ‚½\n");

	emulate();
	
}
