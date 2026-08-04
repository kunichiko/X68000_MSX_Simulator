/*

                ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
                                [[スタートアッププログラム ]]

        ver. 0.01	prpgramed by Kuni.
                                                                                1995.9.15

*/

#include <stdio.h>
#include <string.h>
#include <sys/dos.h>
#include <sys/iocs.h>

#include "ms.h"

#define segments 4

int com;

void main() {
    void *MMem, *VideoRAM, *MainROM1, *MainROM2, *SUBROM, *FDCROM, *ROM;
    int a, i;

    if (_iocs_b_super(0) < 0)
        ;

    initialize(); /* システムの初期化				*/

    MMem = new_malloc(64 * 1024 + 8 * segments); /* ６４Ｋ + ８バイト＊総セグメント数	*/
    if (MMem == NULL) {
        printf("メモリが確保できません\n");
        ms_exit();
    }
    MMemSet(MMem, (int)segments); /* アセンブラのルーチンへ引き渡し		*/

    VideoRAM = new_malloc(128 * 1024); /* ＶＲＡＭ １２８Ｋ 					*/
    if (VideoRAM == NULL) {
        printf("メモリが確保できません\n");
        ms_exit();
    }
    ms_vdp_init(VideoRAM); /* アセンブラのルーチンへ引き渡し		*/
                           /* 画面の初期化等						*/

    MainROM1 = new_malloc(16 * 1024 + 8); /* ＭＡＩＮＲＯＭ（前半） １６Ｋ 		*/
    if (MainROM1 == NULL) {
        printf("メモリが確保できません\n");
        ms_exit();
    }
    SetROM(MainROM1, "MAINROM1.ROM", (int)2, (int)0x00, (int)0);

    MainROM2 = new_malloc(16 * 1024 + 8); /* ＭＡＩＮＲＯＭ（前半） １６Ｋ 		*/
    if (MainROM2 == NULL) {
        printf("メモリが確保できません\n");
        ms_exit();
    }
    SetROM(MainROM2, "MAINROM2.ROM", (int)2, (int)0x00, (int)1);

    SUBROM = new_malloc(16 * 1024 + 8); /* ＳＵＢＲＯＭ １６Ｋ 					*/
    if (SUBROM == NULL) {
        printf("メモリが確保できません\n");
        ms_exit();
    }
    SetROM(SUBROM, "SUBROM.ROM", (int)2, (int)0x0d, (int)0);

    FDCROM = new_malloc(16 * 1024 + 8); /* ＦＤＣＲＯＭ １６Ｋ 					*/
    if (FDCROM == NULL) {
        printf("メモリが確保できません\n");
        ms_exit();
    }
    SetROM(FDCROM, "FDC.ROM", (int)2, (int)0x0e, (int)1);

    if (PSG_INIT() != 0) printf("ＰＳＧの初期化に失敗しました\n");

    debugger();
}
