/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[スタートアッププログラム ]]

	ver. 0.01	prpgramed by Kuni.
										1995.9.15

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <iocslib.h>
#include <doslib.h>
#include <x68k/iocs.h>
#include <x68k/dos.h>
#include "ms.h"

#define NUM_SEGMENTS 4

void emulater_ini(void);
int emulate( int(*)(int));
void ms_init( void);
void ms_exit( void);
void MMemSet( void *, int);
void VDPSet( void *);

/*
  SlotにROMをセットします。
	int SetROM( (void *)address, (char *)filename, (int)kind, (int)slot, (int)page);

  引数：
	void *ROM		: ROMのアドレス
	char *ROMName	: ROMファイル名
	int size		: ROMのサイズ
	int slot		: スロット番号
	int page		: ページ番号

  スロット番号の詳細：
	slot %0000ssxx
		ss: 基本スロット番号
		xx: 拡張スロット番号

	戻り値：(ちゃんと実装されていないかもしれません)
		0	: 正常終了
		-1	: エラー
*/
void SetROM( void* address, const char* filename, int kind, int slot, int page);
int PSG_INIT( void);

int emuLoop(int);
void allocateAndSetROM(size_t size, const char* romFileName, int param1, int param2, int param3);

void printHelpAndExit(char* progname) {
	fprintf(stderr, "Usage: %s [-m1 MAINROM1_PATH] [-m2 MAINROM2_PATH] [-r11 ROM1_PATH] [-r12 ROM2_PATH]\n", progname);
	exit(EXIT_FAILURE);
}

/*
	メイン関数

	起動オプション：
		-m1 filename	: MAIN ROMを指定したものに変更します(前半16K)
		-m2 filename	: MAIN ROMを指定したものに変更します(後半16K)
		-rxx filename	: スロットにfilenameをセットします
			xx: 10の位 = スロット番号, 1の位 = ページ番号
			例：-r11 GAME1.ROM	: スロット1-ページ1にGAME1.ROMをセット				
*/
int main(int argc, char *argv[]) {
	void *MMem, *VideoRAM, *MainROM1, *MainROM2, *SUBROM, *SLOT1_1, *ROM, *SLOT1_2;
	int a, i;
	char *mainrom1_path = "MAINROM1.ROM"; // デフォルトのMAIN ROM1パス
	char *mainrom2_path = "MAINROM2.ROM"; // デフォルトのmAIN ROM2パス
	char *cartridge_path[4][4]; // カートリッジのパス
	int opt;

	printf("MSX Simulator\n");

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cartridge_path[i][j] = NULL;
		}
	}

	// コマンドラインオプションの解析
	while ((opt = getopt(argc, argv, "m:r:")) != -1)
	{
		switch (opt)
		{
		case 'm': // -mN オプション
			if (strlen(optarg) == 1 && isdigit(optarg[0]))
			{ 
				if (argv[optind] != NULL)
				{
					if (optarg[0] == '1')
					{
						mainrom1_path = argv[optind++];
					} else if (optarg[0] == '2')
					{
						mainrom2_path = argv[optind++];
					}
				}
			}
			break;
		case 'r': // -rNN オプション
			if (strlen(optarg) == 2 && isdigit(optarg[0]) && isdigit(optarg[1])) {
				// -r に続く2桁の数字を取得
				int num = atoi(optarg);
				int slot = (num / 10);
				int page = num % 10; // 1の位がpage
				if ( slot >= 0 && slot <= 3 && page >= 0 && page <= 3) {
					// 次の引数（ROMファイル名）を取得
					if (argv[optind] != NULL)
					{
						cartridge_path[slot][page] = argv[optind++];
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
				} else {
					printf("スロット番号が不正です\n");
					printHelpAndExit(argv[0]);
				}
			}
			break;
		default: /* '?' */
			printHelpAndExit(argv[0]);
			break;
		}
	}

	if (_iocs_b_super(0) < 0)
	{
		printf("スーパーバイザーモードに移行できませんでした\n");
		return -1;
	}

	printf("ms_init: enter\n");
	ms_init(); /* システムの初期化				*/
	printf("ms_init: exit\n");

	MMem = new_malloc(64 * 1024 + 8 * NUM_SEGMENTS); /* ６４Ｋ + ８バイト＊総セグメント数	*/
	if (MMem > (void *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	MMemSet(MMem, (int)NUM_SEGMENTS); /* アセンブラのルーチンへ引き渡し		*/

	VideoRAM = new_malloc(128 * 1024); /* ＶＲＡＭ １２８Ｋ 					*/
	if (VideoRAM > (void *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	printf("VDPSet: enter\n");
	VDPSet(VideoRAM); /* アセンブラのルーチンへ引き渡し		*/
					  /* 画面の初期化等						*/
	printf("VDPSet: exit\n");

	/* ＭＡＩＮＲＯＭ（前半:16K, 後半:16K */
	allocateAndSetROM(16 * 1024, mainrom1_path, 2, 0x00, 0);
	allocateAndSetROM(16 * 1024, mainrom2_path, 2, 0x00, 1);
	/* ＳＵＢＲＯＭ 16K */
	allocateAndSetROM(16 * 1024, "SUBROM.ROM", 2, 0x0d, 0);

	/* 基本スロット1-ページ1のROM配置 (32KBytesゲームの前半16Kなど）	*/
	//	allocateAndSetROM( 16 * 1024,"GAME1.ROM", (int)2, (int)0x04, (int)1 );
	/* 基本スロット1-ページ1のROM配置 (32KBytesゲームの後半16Kなど）	*/
	//	allocateAndSetROM( 16 * 1024,"GAME2.ROM", (int)2, (int)0x04, (int)2 );

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if ( cartridge_path[i][j] != NULL) {
				allocateAndSetROM(16 * 1024, cartridge_path[i][j], 2, i<<2, j);
			}
		}
	}


	printf("PSG\n");
	if (PSG_INIT() != 0)
	{
		printf("ＰＳＧの初期化に失敗しました\n");
		ms_exit();
	}

	printf("X680x0 MSXシミュレーター with elf2x68k\n");

	printf("emu-ini\n");
	emulater_ini();
	printf("emu-ini exit\n");
	emulate(emuLoop);

	printf("終了します\n");

	ms_exit();
}

/*
 X68000の _BITSNS で得られるキーマトリクスをMSXのキーマトリクスに変換するためのマッピング

 - X68000側のマトリクス
	- X68000環境ハンドブック 「p287 _BITSNS」 参照
 - MSX側のマトリクス
	- MSXデータパック p12 「2.5 キーボード」 参照
 */
unsigned short KEY_MAP[][8] = {
	//  [    ][ESC ][1   ][2   ][3   ][4   ][5   ][6   ] 
	{   0xf00,0x704,0x002,0x004,0x008,0x010,0x020,0x040},
	//  [7   ][8   ][9   ][0   ][-   ][^   ][\   ][BS  ] 
	{   0x080,0x101,0x102,0x001,0x104,0x108,0x110,0x720},
	//  [TAB ][Q   ][W   ][E   ][R   ][T   ][Y   ][U   ] 
	{   0x708,0x440,0x510,0x304,0x480,0x502,0x540,0x504},
	//  [I   ][O   ][P   ][@   ][ [  ][RET ][ A  ][ S  ] 
	{   0x340,0x410,0x420,0x120,0x140,0x780,0x240,0x501},
	//  [ D  ][ F  ][ G  ][ H  ][ J  ][ K  ][ L  ][ ;+ ] 
	{   0x302,0x308,0x310,0x320,0x380,0x401,0x402,0x180},
	//  [ :* ][ ]  ][ Z  ][ X  ][ C  ][ V  ][ B  ][ N  ] 
	{   0x201,0x203,0x580,0x520,0x301,0x508,0x280,0x408},
	//  [ M  ][ ,< ][ .> ][ /  ][ _  ][ SP ][HOME][DEL ]  HOME=CLS
	{   0x404,0x204,0x208,0x210,0x220,0x801,0x802,0x808}, 
	//  [RUP ][RDWN][UNDO][LEFT][UP  ][RIGT][DOWN][CLR ]
	{   0xf00,0xf00,0xf00,0x810,0x820,0x880,0x840,0xf00},
	//  [(/) ][(*) ][(-) ][(7) ][(8) ][(9) ][(+) ][(4) ]  TEN KEYs
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [(5) ][(6) ][(=) ][(1) ][(2) ][(3) ][ENTR][(0) ]  TEN KEYs
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [(,) ][(.) ][KIGO][TORK][HELP][XF1 ][XF2 ][XF3 ]  登録=エミュレータ終了
	{   0xf00,0xf00,0xf00,0xfff,0xf00,0xf00,0xf00,0xf00},
	//  [XF4 ][XF5 ][KANA][ROME][CODE][CAPS][INS ][HIRA]
	{   0xf00,0xf00,0x610,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [ZENK][BRAK][COPY][ F1 ][ F2 ][ F3 ][ F4 ][ F5 ]  BRAK=STOP
	{   0xf00,0x710,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [ F6 ][ F7 ][ F8 ][ F9 ][ F10][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [SHFT][CTRL][OPT1][OPT2][    ][    ][    ][    ]  OPT1=GRAPH
	{   0x601,0x602,0x604,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

int emuLoop(int pc) {
	unsigned short map;
	unsigned char X, Y;

	//if (divider++ < 2) {
	//	return 0;
	//}
	//divider = 0;
	// キースキャン
	for (int i = 0; i < 0x0f; i++)
	{
		KEYSNS_tbl_ptr[i] = 0xff;
	}
	for (int i = 0; i < 0x0f; i++)
	{
		int v = _iocs_bitsns(i);
		//int v = 0;
		for (int j = 0; j < 8; j++)
		{
			if ((v & 1) == 1)
			{
				map = KEY_MAP[i][j];
				Y = (map & 0xf00) >> 8;
				X = (map & 0xff);
				if (Y == 0xf)
				{
					// 特殊キー
					switch (X)
					{
					case 0xff: // 登録キーによる終了
						return 1;
					default:
						break;
					}
				}
				else
				{
					KEYSNS_tbl_ptr[Y] &= ~X;
				}
			}
			v >>= 1;
		}
	}

	return 0;
}

void allocateAndSetROM(size_t size, const char *romFileName, int kind, int slot, int page)
{
	void *ROM = new_malloc(size + 8);
	if (ROM > (void *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	SetROM(ROM, romFileName, kind, slot, page);
}

/*
 	スプライトの処理

	MS.Xは、MSXの256ドットをX68000の512ドットに拡大している
	そのため、MSXの8x8ドットのスプライトは、X68000上は16x16ドットになる
	MSXのスプライトパターンは最大256個定義することができるが、
	X68000は16x16ドットのスプライトパターンを最大128個しか定義できず、数が足りない。
	そこで、MSXのスプライトは最大32個しか表示できないことを利用し、
	X68000のスプライトパターンは、現在表示中のスプライトのみを定義することにする。
	 
 */

unsigned int* X68_SSR = (unsigned short*)0x00eb0000; // スプライトスクロールレジスタ
unsigned int* X68_PCG = (unsigned int*)0x00eb8000;

unsigned int* X68_PCG_buffer;

void initSprite(void) {
	X68_PCG_buffer = (unsigned int*)malloc( 256 * 128); // 1スプライト(16x16)あたり128バイト
	// PCGバッファの初期化
	for (int i = 0; i < 256 * 128; i++) {
		X68_PCG_buffer[i] = 0;
	}
}

/*
 スプライトパターンジェネレータテーブルへの書き込み
     offset: パターンジェネレータテーブルのベースアドレスからのオフセットバイト
     pattern: 書き込むパターン
*/
void writeSpritePattern(int offset, unsigned char pattern) {
	int pNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine * 2; // MSXの1ラインはX68000では2ライン
	unsigned int pLeft=0,pRight=0; // 1ラインの左4ドットと右4ドット (X68000の8x8のパターンの左側と右側)

    // 右端のドットから処理
	for(int i =0; i < 4;i++) {
		if(pattern & 1) {
			pRight |= (0xf0000000);
			pRight >>= 4;
		}
		pattern >>= 1;
	}
    // 残りの左4ドットの処理
	for(int i =0; i < 4;i++) {
		if(pattern & 1) {
			pLeft |= (0xf0000000);
			pLeft >>= 4;
		}
		pattern >>= 1;
	}
	// パターンジェネレータテーブルへの書き込み
	X68_PCG_buffer[pNum * 32 + pcgLine+0 + 0] = pLeft;
	X68_PCG_buffer[pNum * 32 + pcgLine+1 + 0] = pLeft;
	X68_PCG_buffer[pNum * 32 + pcgLine+0 + 16] = pRight;
	X68_PCG_buffer[pNum * 32 + pcgLine+1 + 16] = pRight;
}

void writeSpriteAttribute(int offset, unsigned char attribute) {
	int pNum = offset / 4; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	switch(type) {
		case 0: // Y座標
			X68_SSR[pNum*4+1] = attribute * 2; // MSXのY座標は2倍
			break;
		case 1: // X座標
			X68_SSR[pNum*4+0] = attribute * 2; // MSXのX座標は2倍
			// TODO ECビットによる位置補正処理
			break;
		case 2: // パターン番号
			// 事前にバッファに展開しておいたパターンを転送
			for(int i = 0; i < 32; i++) { 
				X68_PCG[pNum*32+i] = X68_PCG_buffer[attribute*32+i];
			}
			break;
		case 3: // 属性
			// TODO 属性の処理
			break;
	}
}