/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[スタートアッププログラム ]]
								
	ver. 0.01	prpgramed by Kuni.
										1995.9.15

*/

#include <stdio.h>
#include <string.h>
#include <sys/iocs.h>
#include <sys/dos.h>

#define	segments	4

int	ssp;						/* スーパーバイザーモードへ行く時に使用 	*/

void main() {
void *MMem,*VideoRAM,*MainROM1,*MainROM2,*SUBROM;
int a,i;

	ssp = _iocs_b_super( 0);

	initialize();								/* システムの初期化				*/

	MMem = _dos_malloc( 64*1024 + 8*segments);	/* ６４Ｋ + ８バイト＊総セグメント数	*/
	if( MMem > (void *)0x81000000 ) {
 		printf("メモリが確保できません\n");
		ms_exit();
	}
	MMemSet( MMem, (int)segments);			/* アセンブラのルーチンへ引き渡し		*/

	VideoRAM = _dos_malloc( 128*1024);		/* ＶＲＡＭ １２８Ｋ 					*/
	if( VideoRAM > (void *)0x81000000 ) {
 		printf("メモリが確保できません\n");
		ms_exit();
	}

	VDPSet( VideoRAM);						/* アセンブラのルーチンへ引き渡し		*/
											/* 画面の初期化等						*/

	mount_cartridge_zantei();
					
	if( PSG_INIT() != 0)
		printf("ＰＳＧの初期化に失敗しました\n");

	emulate();
	
}
