/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[スタートアッププログラム ]]

	ver. 0.01	prpgramed by Kuni.
										1995.9.15

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <iocslib.h>
#include <doslib.h>
#include <getopt.h>
#include "ms.h"
#include "ms_R800.h"
#include "ms_iomap.h"
#include "memmap/ms_memmap.h"
#include "vdp/ms_vdp.h"

void ms_exit( void);

// メモリ関連
ms_memmap_t* memmap = NULL;

// I/O関連
ms_iomap_t* iomap = NULL;

// VDP関連
ms_vdp_t* vdp = NULL;  // ms_vdp_shared と同じになるはず

// PSG関連
int psg_initialized = 0;
int ms_psg_init( void);
void ms_psg_deinit(void);

/*
  SlotにROMをセットします。
	int ms_memmap_set_rom( (void *)address, (char *)filename, (int)kind, (int)slot, (int)page);

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
void ms_memmap_set_rom( void* address, const char* filename, int kind, int slot, int page);
void ms_memmap_register_rom( void* address, int kind, int slot, int page);

int emuLoop(unsigned int pc, unsigned int counter);

void set_system_roms(void);

void _toggleTextPlane(void);
void _setTextPlane(int textPlaneMode);
void _moveTextPlane(int cursorKeyHit);

unsigned short host_rate = 1;

volatile extern unsigned short ms_vdp_interrupt_tick;
volatile extern unsigned short ms_vdp_vsync_rate;
volatile extern unsigned int int_block_count;
volatile extern unsigned short debug_log_level;
volatile extern unsigned short host_delay;
volatile extern unsigned int int_skip_counter;
volatile extern unsigned int int_exec_counter;
typedef struct interrupt_history_st {
    unsigned short int_tick;
    unsigned short process_type;
    unsigned long emu_counter;
} interrupt_history_t;

volatile extern interrupt_history_t* interrupt_history_ptr;
volatile extern unsigned short interrupt_history_wr;
volatile extern unsigned short interrupt_history_rd;

void printHelpAndExit(char* progname) {
	fprintf(stderr, "Usage: %s  [-w MAX_WAIT] [-rm MAINROM_PATH] [-rs SUBROM_PATH] [-rd DISKBIOS_PATH] [-r1 ROM_PATH for slot 1][,KIND] [-r2 ROM_PATH for slot 2][,KIND] [-rNM ROM_PATH for slot N page M] [IMAGE1.DSK] [IMAGE2.DSK]..\n", progname);
	fprintf(stderr, " KIND is ROM type:\n");
	fprintf(stderr, "    NOR: Normal ROM, G8K: GENERIC 8K, A8K: ASCII 8K, A16: ASCII 16K, KON: Konami, SCC: Konami SCC\n");
	fprintf(stderr, " --vsrate vsync rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 1.\n");
	fprintf(stderr, " --intblock block cycle (1-9999)\n");
	fprintf(stderr, "    default is 2048 cycles.\n");
	fprintf(stderr, " --hostrate host key operation rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 3.\n");
	fprintf(stderr, " --hostdelay host key interruption delay cycle (1-9999)\n");
	fprintf(stderr, "    default is 20 cycles.\n");
	fprintf(stderr, " --disablekey\n");
	fprintf(stderr, "    disable key input for performance test.\n");
//	fprintf(stderr, " --debuglevel N\n");
//	fprintf(stderr, "    0: None, 1: Info, 2: Debug, 3: Fine.\n");
	exit(EXIT_FAILURE);
}

static unsigned char *MainROM1;
static unsigned char *MainROM2;
static unsigned char *SUBROM;
static unsigned char *SLOT1_1;
static unsigned char *SLOT1_2;
static unsigned char *ROM;

uint32_t max_wait = 0xffffffff;
int disablekey = 0;

const char *mainrom_cbios = "cbios_main_msx2_jp.rom";
const char *cbioslogo = "cbios_logo_msx2.rom";
const char *subrom_cbios = "cbios_sub.rom";

char *mainrom_user = "MAINROM.ROM";
char *subrom_user = "SUBROM.ROM";
char *diskbios_user = "DISKBIOS.ROM";

int diskcount = 0;
char *diskimages[16];

char* separate_rom_kind(char* path, int* kind) {
	char* p = strchr(path, ',');
	if (p != NULL) {
		*p = '\0';
		p++;
		if (strcasecmp(p, "NOR") == 0) {
			*kind = ROM_TYPE_NORMAL_ROM;
		} else if (strcasecmp(p, "G8K") == 0) {
			*kind = ROM_TYPE_MEGAROM_GENERIC_8K;
		} else if (strcasecmp(p, "A8K") == 0) {
			*kind = ROM_TYPE_MEGAROM_ASCII_8K;
		} else if (strcasecmp(p, "A16") == 0) {
			*kind = ROM_TYPE_MEGAROM_ASCII_16K;
		} else if (strcasecmp(p, "KON") == 0) {
			*kind = ROM_TYPE_MEGAROM_KONAMI;
		} else if (strcasecmp(p, "SCC") == 0) {
			*kind = ROM_TYPE_MEGAROM_KONAMI_SCC;
		} else {
			*kind = -1;
		}
	}
	return path;
}

/*
	メイン関数

	起動オプション：
		-m filename		: MAIN ROMを指定したものに変更します(32Kバイト)
		-rxx filename	: スロットにfilenameをセットします
			xx: 10の位 = スロット番号, 1の位 = ページ番号
			例：-r11 GAME1.ROM	: スロット1-ページ1にGAME1.ROMをセット				
*/
int main(int argc, char *argv[]) {
	int i, j;
	char *cartridge_path_slot1 = NULL; // カートリッジのパス
	int cartridge_kind_slot1 = -1; // カートリッジの種類
	char *cartridge_path_slot2 = NULL; // カートリッジのパス
	int cartridge_kind_slot2 = -1; // カートリッジの種類
	char *slot_path[4][4]; // 個々のスロットにセットするROMのパス
	int opt;
    const char* optstring = "hm:s:w:r:" ; // optstringを定義します
    const struct option longopts[] = {
      //{        *name,           has_arg,       *flag, val },
        {     "vsrate", required_argument,           0, 'A' },
        {   "intblock", required_argument,           0, 'B' },
        {   "hostrate", required_argument,           0, 'C' },
        {  "hostdelay", required_argument,           0, 'D' },
		{ "disablekey",       no_argument, &disablekey,  1  },
        {            0,                 0,           0,  0  }, // termination
    };
	const struct option* longopt;
    int longindex = 0;

	printf("[[ MSX Simulator MS.X]]\n");

	unsigned int mpu_type = _iocs_mpu_stat();
	if( (mpu_type & 0xf) < 3) {
		printf("MS.X の動作には 68030以上が必要です\n");
		ms_exit();
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			slot_path[i][j] = NULL;
		}
	}

	for(i=0;i<32;i++) {
		interrupt_history_ptr[i].int_tick = 0;
		interrupt_history_ptr[i].process_type = 0;
		interrupt_history_ptr[i].emu_counter = 0;
	}

	// コマンドラインオプションの解析
	while ((opt = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1)
	{
		switch (opt)
		{
		case 0: // フラグがセットされた場合
			break;
		case 'h': // -h オプション
			printHelpAndExit(argv[0]);
			break;
		case 'm': // -m オプション
			if (optarg != NULL)
			{
				mainrom_user = optarg;
			}
			break;
		case 'w': // -w オプション
			if (optarg != NULL)
			{
				max_wait = atoi(optarg);
			}
			break;
		case 's': // -s オプション
			if (optarg != NULL)
			{
				subrom_user = optarg;
			}
			break;
		case 'r': // -rNN オプション
			if (strlen(optarg) == 1 && !isdigit(optarg[0]) ) {
				// -r に数字以外が続く場合
				switch(optarg[0]) {
				case 'm':
					// メインROMの指定
					if (argv[optind] != NULL)
					{
						mainrom_user = argv[optind++];
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
					break;
				case 's':
					// サブROMの指定
					if (argv[optind] != NULL)
					{
						subrom_user = argv[optind++];
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
					break;
				case 'd':
					// ディスクBIOSの指定
					if (argv[optind] != NULL)
					{
						diskbios_user = argv[optind++];
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
					break;
				default:
					printf("不明なオプションです\n");
					printHelpAndExit(argv[0]);
					break;
				}
			} else if (strlen(optarg) == 1 && isdigit(optarg[0]) ) {
				// -r に数字が1桁続く場合
				int slot = atoi(optarg);
				if ( slot >= 1 && slot <= 2) {
					// 次の引数（ROMファイル名）を取得
					if (argv[optind] != NULL)
					{
						if (slot == 1) {
							cartridge_path_slot1 = argv[optind++];
							cartridge_path_slot1 = separate_rom_kind(cartridge_path_slot1, &cartridge_kind_slot1);
						} else {
							cartridge_path_slot2 = argv[optind++];
							cartridge_path_slot2 = separate_rom_kind(cartridge_path_slot2, &cartridge_kind_slot2);
						}
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
				} else {
					printf("スロット番号は1か2を指定してください。\n");
					printHelpAndExit(argv[0]);
				}
			} else if (strlen(optarg) == 2 && isdigit(optarg[0]) && isdigit(optarg[1])) {
				// -r に数字が2桁続く場合
				int num = atoi(optarg);
				int slot = (num / 10);
				int page = num % 10; // 1の位がpage
				if ( slot >= 0 && slot <= 3 && page >= 0 && page <= 3) {
					// 次の引数（ROMファイル名）を取得
					if (argv[optind] != NULL)
					{
						slot_path[slot][page] = argv[optind++];
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
			} else {
				// -r 単独は -r1 と同じとみなす
				// 次の引数（ROMファイル名）を取得
				if (optarg != NULL)
				{
					cartridge_path_slot1 = optarg;
					cartridge_path_slot1 = separate_rom_kind(cartridge_path_slot1, &cartridge_kind_slot1);
				}
				else
				{
					printf("ROMファイル名が指定されていません\n");
					ms_exit();
				}
			}
			break;
		case 'A': // --vsrate N オプション
			// VSYNCレートの設定
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				ms_vdp_vsync_rate = atoi(optarg);
				if (ms_vdp_vsync_rate < 1 || ms_vdp_vsync_rate > 61) {
					printf("VSYNCレートが不正です\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("VSYNCレートが指定されていません\n");
				printHelpAndExit(argv[0]);
			}
			break;
		case 'B': // --intblock N オプション
			// 割り込みブロックカウントの設定
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				int_block_count = atoi(optarg);
				if (int_block_count < 1 || int_block_count > 9999) {
					printf("割り込みブロックカウントが不正です\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("割り込みブロックカウントが指定されていません\n");
				printHelpAndExit(argv[0]);
			}
			break;
		case 'C': // --hostrate N オプション
			// ホスト処理レートの設定
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				host_rate = atoi(optarg);
				if (host_rate < 1 || host_rate > 61) {
					printf("ホスト処理レートが不正です\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("ホスト処理レートが指定されていません\n");
				printHelpAndExit(argv[0]);
			}
			break;
		case 'D': // --hostdelay N オプション
			// ホスト処理遅延カウントの設定
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				host_delay = atoi(optarg);
				if (host_delay < 1 || host_delay > 9999) {
					printf("ホスト処理遅延カウントが不正です\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("ホスト処理遅延カウントが指定されていません\n");
				printHelpAndExit(argv[0]);
			}
			break;
		default: /* '?' */
			printHelpAndExit(argv[0]);
			break;
		}
	}

    // getoptのループが終了した後、通常の引数を処理する
	for (i = optind; i < argc; i++) {
		if (strcasestr(argv[i], ".dsk") != 0) {
			diskimages[diskcount++] = argv[i];
		}
	}

	if (_iocs_b_super(0) < 0)
	{
		printf("スーパーバイザーモードに移行できませんでした\n");
		return -1;
	}

	// CTRL+Cで中断されたときに、ms_exitを呼び出すようにする
	_dos_intvcs(0xfff1, ms_exit);

	/*
	 メモリシステムの初期化
	 */
	memmap = ms_memmap_alloc();
	if (memmap == NULL)
	{
		printf("メモリシステムの初期化に失敗しました\n");
		ms_exit();
	}
	ms_memmap_init(memmap);

	/*
	 VDPシステムの初期化
	*/
	vdp = ms_vdp_alloc();
	if (vdp == NULL)
	{
		printf("VDPシステムの初期化に失敗しました\n");
		ms_exit();
	}
	ms_vdp_init(vdp);

	/*
	 I/Oシステムの初期化
	 */
	iomap = ms_iomap_alloc();
	if (iomap == NULL)
	{
		printf("I/Oシステムの初期化に失敗しました\n");
		ms_exit();
	}
	ms_iomap_init(iomap, vdp);


	printf("\n\n\n\n\n\n\n\n"); // TEXT画面を上に8ラインくらい上げているので、その分改行を入れる
	printf("[[ MSX Simulator MS.X]]\n");
	printf(" この画面は HELP キーで消せます\n");

	/*
	 PSGシステムの初期化
	 */
	psg_initialized = ms_psg_init();
	if (psg_initialized == 0)
	{
		printf("ＰＳＧの初期化に失敗しました\n");
		ms_exit();
	}


	/*
	 SYSTEM ROMのセット
	 */
	set_system_roms();

	/* 基本スロット1-ページ1のROM配置 (32KBytesゲームの前半16Kなど）	*/
	//	allocateAndSetROM( 16 * 1024,"GAME1.ROM", (int)2, (int)0x04, (int)1 );
	/* 基本スロット1-ページ1のROM配置 (32KBytesゲームの後半16Kなど）	*/
	//	allocateAndSetROM( 16 * 1024,"GAME2.ROM", (int)2, (int)0x04, (int)2 );

	for ( i = 0; i < 4; i++) {
		for ( j = 0; j < 4; j++) {
			if ( slot_path[i][j] != NULL) {
				printf("スロット%d-ページ%dにROMをセットします: %s\n", i, j, slot_path[i][j]);
				allocateAndSetROM(slot_path[i][j], ROM_TYPE_NORMAL_ROM, i, -1, j);
			}
		}
	}

	// サイズを自動判別してROMをセット
	if (cartridge_path_slot1 != NULL) {
		allocateAndSetROM_Cartridge(cartridge_path_slot1, 1, cartridge_kind_slot1);
	}
	if (cartridge_path_slot2 != NULL) {
		allocateAndSetROM_Cartridge(cartridge_path_slot2, 2, cartridge_kind_slot2);
	}

	printf("X680x0 MSXシミュレーター with elf2x68k\n");

	printf("VSYNCレート=%d, ホスト処理レート=%d\n", ms_vdp_vsync_rate, host_rate);
	printf("VSYNC計測中...\n");
	{
		volatile int date,lastdate;
		volatile int start,end;

		date = _iocs_timeget();
		lastdate = date;
		while(date == lastdate) {	// 秒が変わる瞬間を待つ
			lastdate = date;
			date = _iocs_timeget();
		}
		start = ms_vdp_interrupt_tick;		// そのときのtickを取得
		date = _iocs_timeget();
		lastdate = date;
		while(date == lastdate) {	// 秒が変わる瞬間を待つ
			lastdate = date;
			date = _iocs_timeget();
		}
		end = ms_vdp_interrupt_tick;		// そのときのtickを取得

		printf("VSYNC回数は %d です\n", end - start);
	}

	// 	全ページの スロットを０で初期化
	write_port_A8(0);

	if (1) {
		ms_cpu_emulate(emuLoop, max_wait);
	} else {
		debugger();
	}

	ms_exit();
}

void ms_exit() {
	printf("終了します。何かキーを押してください。\n");
	_iocs_b_keyinp();

	_iocs_crtmod(0x10);

	if ( psg_initialized ) {
		ms_psg_deinit();
	}
	if ( vdp != NULL ) {
		ms_vdp_deinit(vdp);
		new_free(vdp);
	}
	if ( iomap != NULL ) {
		ms_iomap_deinit(iomap);
		new_free(iomap);
	}
	if ( memmap != NULL ) {
		ms_memmap_deinit(memmap);
		new_free(memmap);
	}
	exit(0);
}

/*
 X68000の _BITSNS で得られるキーマトリクスをMSXのキーマトリクスに変換するためのマッピング

 - X68000側のマトリクス
	- X68000環境ハンドブック 「p287 _BITSNS」 参照
 - MSX側のマトリクス
	- MSXデータパック p12 「2.5 キーボード」 参照
 */
unsigned short KEY_MAP[][8] = {
	//0 [    ][ESC ][1   ][2   ][3   ][4   ][5   ][6   ] 
	{   0xf00,0x704,0x002,0x004,0x008,0x010,0x020,0x040},
	//1 [7   ][8   ][9   ][0   ][-   ][^   ][\   ][BS  ] 
	{   0x080,0x101,0x102,0x001,0x104,0x108,0x110,0x720},
	//2 [TAB ][Q   ][W   ][E   ][R   ][T   ][Y   ][U   ] 
	{   0x708,0x440,0x510,0x304,0x480,0x502,0x540,0x504},
	//3 [I   ][O   ][P   ][@   ][ [  ][RET ][ A  ][ S  ] 
	{   0x340,0x410,0x420,0x120,0x140,0x780,0x240,0x501},
	//4 [ D  ][ F  ][ G  ][ H  ][ J  ][ K  ][ L  ][ ;+ ] 
	{   0x302,0x308,0x310,0x320,0x380,0x401,0x402,0x180},
	//5 [ :* ][ ]  ][ Z  ][ X  ][ C  ][ V  ][ B  ][ N  ] 
	{   0x201,0x203,0x580,0x520,0x301,0x508,0x280,0x408},
	//6 [ M  ][ ,< ][ .> ][ /  ][ _  ][ SP ][HOME][DEL ]  HOME=CLS
	{   0x404,0x204,0x208,0x210,0x220,0x801,0x802,0x808}, 
	//7 [RUP ][RDWN][UNDO][LEFT][UP  ][RIGT][DOWN][CLR ]
	{   0xf00,0xf00,0xf00,0xff1,0xff2,0xff4,0xff3,0xf00},
	//8 [(/) ][(*) ][(-) ][(7) ][(8) ][(9) ][(+) ][(4) ]  TEN KEYs
	{   0x902,0x904,0xa20,0xa04,0xa08,0xa10,0x901,0x980},
	//9 [(5) ][(6) ][(=) ][(1) ][(2) ][(3) ][ENTR][(0) ]  TEN KEYs
	{   0xa01,0xa02,0xf00,0x910,0x920,0x940,0xf00,0x908},
	//a [(,) ][(.) ][KIGO][TORK][HELP][XF1 ][XF2 ][XF3 ]  登録=エミュレータ終了, XF1=GRAPH
	{   0xa40,0xa80,0xffd,0xfff,0xffe,0x604,0xf00,0xf00}, 
	//b [XF4 ][XF5 ][KANA][ROME][CODE][CAPS][INS ][HIRA]
	{   0xf00,0xf00,0x610,0xf00,0xf00,0xf00,0x804,0xf00},
	//c [ZENK][BRAK][COPY][ F1 ][ F2 ][ F3 ][ F4 ][ F5 ]  BRAK=STOP
	{   0xf00,0x710,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//d [ F6 ][ F7 ][ F8 ][ F9 ][ F10][    ][    ][    ]  F6=DebugLevel
	{   0xffc,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//e [SHFT][CTRL][OPT1][OPT2][    ][    ][    ][    ]  OPT1=GRAPH
	{   0xff0,0x602,0x604,0xf00,0xf00,0xf00,0xf00,0xf00},
	//f [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

void dump(unsigned int page, unsigned int pc_8k);

void sync_keyboard_leds();

int emuLoop(unsigned int pc, unsigned int counter) {
	static int emuLoopCounter = 0;
	int i,j;
	unsigned short map;
	unsigned char X, Y;
	int hitkey = 0;
	int shiftKeyHit = 0;
	int cursorKeyHit = 0; // 1=LEFT, 2=UP, 3=DOWN, 4=RIGHT
	static int f6KeyHit = 0, f6KeyHitLast = 0;
	static int kigoKeyHit = 0, kigoKeyHitLast = 0;
	static int helpKeyHit = 0, helpKeyHitLast = 0;
	static int shiftAndCursorKeyHit = 0, shiftAndCursorKeyHitLast = 0;

	emuLoopCounter++;

	if( vdp != NULL) {
		ms_vdp_vsync_draw(vdp);
	}

	if(emuLoopCounter % host_rate != 0) {
		return 0;
	}

	sync_keyboard_leds();

	kigoKeyHit = 0;
	helpKeyHit = 0;
	f6KeyHit = 0;

	for ( i = 0x00; i < 0x0f; i++)
	{
		KEYSNS_tbl_ptr[i] = 0xff;
	}
	for ( i = 0x00; i < 0x0f; i++)
	{
		if (disablekey == 1 && i != 0x0a)
		{
			continue;
		}
		//int v = _iocs_bitsns(i);
		int v = ((uint8_t*)0x800)[i];	// IOCS BITSNSのワークエリア直接参照
		for ( j = 0; j < 8; j++)
		{
			if ((v & 1) == 1)
			{
				hitkey = 1;
				map = KEY_MAP[i][j];
				Y = (map & 0xf00) >> 8;
				X = (map & 0xff);
				if (Y == 0xf)
				{
					// 特殊キー
					switch (X)
					{
					case 0xf0: // SHIFTキー 0x601
						Y = 0x6;
						X = 0x01;
						shiftKeyHit = 1;
						break;
					case 0xf1: // LEFTキー 0x810
						Y = 0x8; 
						X = 0x10;
						cursorKeyHit = 1;
						break;
					case 0xf2: // UPキー 0x820
						Y = 0x8; 
						X = 0x20;
						cursorKeyHit = 2;
						break;
					case 0xf3: // DOWNキー 0x840
						Y = 0x8; 
						X = 0x40;
						cursorKeyHit = 3;
						break;
					case 0xf4: // RIGHTキー 0x880
						Y = 0x8; 
						X = 0x80;
						cursorKeyHit = 4;
						break;
					case 0xfc: // F6キーを押すと、デバッグレベル変更
						f6KeyHit = 1;
						break;
					case 0xfd: // 記号キーを押すと、PC周辺のメモリダンプ
						kigoKeyHit = 1;
						break;
					case 0xfe: // HELPキーを押すとテキスト表示切り替え
						helpKeyHit = 1;
						break;
					case 0xff: // 登録キーによる終了
						_setTextPlane(1);
						return 1;
					default:
						break;
					}
				}
				if (Y != 0xf) { // elseにしてはダメ
					KEYSNS_tbl_ptr[Y] &= ~X;
				}
			}
			v >>= 1;
		}
	}

	if (hitkey)
	{
		// 文字入力を読み捨てて、キーバッファを空にする
		_dos_kflushio(0xff);
	}

	if (f6KeyHit && !f6KeyHitLast)
	{
		debug_log_level = (debug_log_level + 1) & 0x3;
		printf("デバッグログレベル=%d\n", debug_log_level);
	}
	f6KeyHitLast = f6KeyHit;

	if (kigoKeyHit && !kigoKeyHitLast)
	{
		printf("\n");
		printf("loop count=%08d\ncycle=%08ld wait=%ld\n", emuLoopCounter, cpu_cycle_last, cpu_cycle_wait);
		printf("COUNTER=%08x, inttick=%08d\n", counter, ms_vdp_interrupt_tick);
		dump(pc >> 16, pc & 0x1fff);
	}
	kigoKeyHitLast = kigoKeyHit;

	if (helpKeyHit && !helpKeyHitLast)
	{
		_toggleTextPlane();
	}
	helpKeyHitLast = helpKeyHit;

	shiftAndCursorKeyHit = shiftKeyHit && cursorKeyHit;
	if (shiftAndCursorKeyHit && !shiftAndCursorKeyHitLast)
	{
		_moveTextPlane(cursorKeyHit);
	}
	shiftAndCursorKeyHitLast = shiftAndCursorKeyHit;
	return 0;
}

// メモリダンプ
void dump(unsigned int page, unsigned int pc_8k) {
	int i,j;
	int pcbase,pc;
	int data;
	unsigned int current_int_skip_counter = int_skip_counter;
	static unsigned int last_int_skip_counter = 0;
	int rd,wr;
	char process_type_char[] = {'E','S','D','?'};

	wr = interrupt_history_wr;
	for(i=0;i<8;i++) {
		unsigned int tick;
		unsigned int process_type; // 0: EI状態で割り込みがかかった, 1: EIだったがスキップ, 2: DI状態で割り込みがスキップ
		unsigned int counter;
		rd = (interrupt_history_rd + i) % 32;
		if (rd == wr) {
			break;
		}
		tick = interrupt_history_ptr[rd].int_tick;
		process_type = interrupt_history_ptr[rd].process_type;
		if (process_type > 2 ) {
			process_type = 3;
		}
		counter = interrupt_history_ptr[rd].emu_counter;
		printf("[%04x]%c:%06d >",tick,process_type_char[process_type],counter);
	}
	printf("\n");

	pc = (page << 13) | (pc_8k & 0x1fff);
	pcbase = (pc & 0xfff0);
	printf("PAGE=%04x, PC=%04x, EXEC=%04d SKIP=%04d (+%04d)\n", page, (pc&0xffff), int_exec_counter, current_int_skip_counter,current_int_skip_counter - last_int_skip_counter);
	last_int_skip_counter = current_int_skip_counter;
	for (i = -1; i < 2; i++)
	{
		printf("%04x: ", pcbase + i * 16);
		for (j = 0; j < 16; j++)
		{
			data = ms_memmap_read8(pcbase + i * 16 + j);
			printf("%02x ", data);
		}
		printf("\n");
	}
}

static uint8_t keyboard_led_counter = 0;
static uint8_t last_led_caps = 0;
static uint8_t last_led_kana = 0;

void sync_keyboard_leds() {
	if (keyboard_led_counter == 0) {
		keyboard_led_counter = 3;
		if( last_led_caps != ms_peripherals_led_caps) {
			ms_iocs_ledctrl(0x3, ms_peripherals_led_caps);
			last_led_caps = ms_peripherals_led_caps;
		}
		if( last_led_kana != ms_peripherals_led_kana) {
			ms_iocs_ledctrl(0x0, ms_peripherals_led_kana);
			last_led_kana = ms_peripherals_led_kana;
		}
	} else {
		keyboard_led_counter--;
	}
}

// テキスト表示切り替え
unsigned short* VCON_R02 = (unsigned short*)0x00e82600;

static unsigned short debug_log_level_bup;

void _toggleTextPlane(void) {
	static int textPlaneMode = 1;

	textPlaneMode = (textPlaneMode + 1) % 2;
	if (textPlaneMode == 0) {
		// テキスト表示OFFにする時に、デバッグログをOFFにする
		debug_log_level_bup = debug_log_level;
		debug_log_level = 0;
	}
	else {
		debug_log_level = debug_log_level_bup;
	}
	_setTextPlane(textPlaneMode);
}

void _moveTextPlane(int cursorKeyHit) {
	static uint16_t x = 0;
	static uint16_t y = 0;
	int diff = 64;

	switch (cursorKeyHit)
	{
	case 1:
		// 左
		if (x >= diff) {
			x -= diff;
		} else {
			x = 0;
		}
		break;
	case 2:
		// 上
		if (y >= diff) {
			y -= diff;
		} else {
			y = 0;
		}
		break;
	case 3:
		// 下
		if (y <= 512 - 192 - diff) { // 適当
			y += diff;
		} else {
			y = 512 - 192;
		}
		break;
	case 4:
		// 右
		if (x <= 512 - 192 - diff) { // 適当
			x += diff;
		} else {
			x = 512 - 192;
		}
		break;
	}
	CRTR_10 = x;
	CRTR_11 = y;
}

void _setTextPlane(int textPlaneMode) {
	switch (textPlaneMode)
	{
	case 0:
		// テキスト表示OFF
		*VCON_R02 &= 0xffdf;
		vdp->tx_active = 0;
		break;
	case 1:
		// テキスト表示ON
		*VCON_R02 |= 0x0020;
		vdp->tx_active = 1;
		break;
	}
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void set_system_roms() {
	if (file_exists(mainrom_user) && file_exists(subrom_user)) {
		// Load user-provided ROMs
		printf("指定されたBIOS ROMが見つかりました。%s と %sを使用します。\n", mainrom_user, subrom_user);
		allocateAndSetROM(mainrom_user, ROM_TYPE_NORMAL_ROM, 0x00, -1, 0);
		allocateAndSetROM(subrom_user, ROM_TYPE_NORMAL_ROM, 0x03, 1, 0);
		if (file_exists(diskbios_user)) {
			printf("指定されたディスクBIOS ROMが見つかりました。%sを使用します。\n", diskbios_user);
			int i;
			for(i=0;i<diskcount;i++) {
				printf("ディスクイメージ %d: %s\n", i, diskimages[i]);
			}
			allocateAndSetDISKBIOSROM(diskbios_user, diskcount, diskimages);
		}
    } else {
        // Load default CBIOS ROMs
		printf("CBIOS ROMを使用します。\n");
		allocateAndSetROM(mainrom_cbios, ROM_TYPE_NORMAL_ROM, 0x00, -1, 0);
		allocateAndSetROM(cbioslogo, ROM_TYPE_NORMAL_ROM, 0x00, -1, 2);
		allocateAndSetROM(subrom_cbios, ROM_TYPE_NORMAL_ROM, 0x03, 1, 0);
    }
}

extern int filelength(int fh);

/*
	for debug
*/
void dumpsp_c(uint8_t *const registers, uint32_t sp, uint32_t a0, uint32_t a4, uint32_t d7, uint32_t d4, uint32_t d6)
{
	int i;
	for (i = 0; i < 16; i++)
	{
		printf("%02x ", registers[i]);
	}
	printf("   : SP=%04x a0=%08x a4=%08x d7=%08x d4=%08x d6=%08x\n", sp, a0, a4, d7, d4, d6);
}