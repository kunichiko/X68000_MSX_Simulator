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
#include <ctype.h>
#include <iocslib.h>
#include <doslib.h>
#include <getopt.h>
#include "ms.h"
#include "ms_R800.h"

#define NUM_SEGMENTS 4

extern void writeSpritePattern(unsigned char* p, int offset, unsigned int pattern);
extern void writeSpriteAttribute(unsigned char* p, int offset, unsigned int attribute);
extern int readMemFromC(int address);

void ms_exit( void);

// メモリ関連
int mem_initialized = 0;
int ms_memmap_init();
void ms_memmap_deinit(void);

void ms_memmap_set_main_mem( void *, int);

// I/O関連
int io_initialized = 0;
int ms_iomap_init();
void ms_iomap_deinit(void);

// VDP関連
int vdp_initialized = 0;
int ms_vdp_init( void *);
void ms_vdp_deinit(void);

void initSprite(void);

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


int emuLoop(unsigned int pc, unsigned int counter);
void allocateAndSetROM(size_t size, const char* romFileName, int param1, int param2, int param3);

void _toggleTextPlane(void);
void _setTextPlane(int textPlaneMode);

volatile extern unsigned short ms_vdp_interrupt_tick;
volatile extern unsigned short ms_vdp_vsync_rate;
volatile extern unsigned int int_block_count;
volatile extern unsigned short debug_log_level;
volatile extern unsigned short host_rate;
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
	fprintf(stderr, "Usage: %s [-m1 MAINROM1_PATH] [-m2 MAINROM2_PATH] [-r11 ROM1_PATH] [-r12 ROM2_PATH]\n", progname);
	fprintf(stderr, " --vsrate vsync rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 1.\n");
	fprintf(stderr, " --intblock block cycle (1-9999)\n");
	fprintf(stderr, "    default is 2048 cycles.\n");
	fprintf(stderr, " --hostrate host key operation rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 3.\n");
	fprintf(stderr, " --hostdelay host key interruption delay cycle (1-999)\n");
	fprintf(stderr, "    default is 20 cycles.\n");
	fprintf(stderr, " --disablekey\n");
	fprintf(stderr, "    disable key input for performance test.\n");
//	fprintf(stderr, " --debuglevel N\n");
//	fprintf(stderr, "    0: None, 1: Info, 2: Debug, 3: Fine.\n");
	exit(EXIT_FAILURE);
}

static unsigned char *MMem;
static unsigned char *VideoRAM;
static unsigned char *MainROM1;
static unsigned char *MainROM2;
static unsigned char *SUBROM;
static unsigned char *SLOT1_1;
static unsigned char *SLOT1_2;
static unsigned char *ROM;

int disablekey = 0;

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
	int i, j;
	char *mainrom1_path = "MAINROM1.ROM"; // デフォルトのMAIN ROM1パス
	char *mainrom2_path = "MAINROM2.ROM"; // デフォルトのmAIN ROM2パス
	char *cartridge_path[4][4]; // カートリッジのパス
	int opt;
    const char* optstring = "hm:r:" ; // optstringを定義します
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

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			cartridge_path[i][j] = NULL;
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
				if (host_delay < 1 || host_delay > 999) {
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

	if (_iocs_b_super(0) < 0)
	{
		printf("スーパーバイザーモードに移行できませんでした\n");
		return -1;
	}

	/*
	 メモリシステムの初期化
	 */
	mem_initialized = ms_memmap_init();
	if (mem_initialized == 0)
	{
		printf("メモリシステムの初期化に失敗しました\n");
		ms_exit();
	}

	MMem = new_malloc(64 * 1024 + 8 * NUM_SEGMENTS); /* ６４Ｋ + ８バイト＊総セグメント数	*/
	if (MMem > (unsigned char *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	ms_memmap_set_main_mem(MMem, (int)NUM_SEGMENTS); /* アセンブラのルーチンへ引き渡し		*/

	/*
	 I/Oシステムの初期化
	 */
	io_initialized = ms_iomap_init();
	if (io_initialized == 0)
	{
		printf("I/Oシステムの初期化に失敗しました\n");
		ms_exit();
	}

	/*
	 VDPシステムの初期化
	*/
	VideoRAM = new_malloc(128 * 1024); /* ＶＲＡＭ １２８Ｋ 					*/
	if (VideoRAM > (unsigned char *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	vdp_initialized = ms_vdp_init(VideoRAM);
	if (vdp_initialized == 0)
	{
		printf("VDPシステムの初期化に失敗しました\n");
		ms_exit();
	}

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
	 ROMのセット
	 */

	/* ＭＡＩＮＲＯＭ（前半:16K, 後半:16K */
	allocateAndSetROM(16 * 1024, mainrom1_path, 2, 0x00, 0);
	allocateAndSetROM(16 * 1024, mainrom2_path, 2, 0x00, 1);
	/* ＳＵＢＲＯＭ 16K */
	allocateAndSetROM(16 * 1024, "SUBROM.ROM", 2, 0x0d, 0);

	/* 基本スロット1-ページ1のROM配置 (32KBytesゲームの前半16Kなど）	*/
	//	allocateAndSetROM( 16 * 1024,"GAME1.ROM", (int)2, (int)0x04, (int)1 );
	/* 基本スロット1-ページ1のROM配置 (32KBytesゲームの後半16Kなど）	*/
	//	allocateAndSetROM( 16 * 1024,"GAME2.ROM", (int)2, (int)0x04, (int)2 );

	for ( i = 0; i < 4; i++) {
		for ( j = 0; j < 4; j++) {
			if ( cartridge_path[i][j] != NULL) {
				allocateAndSetROM(16 * 1024, cartridge_path[i][j], 2, i<<2, j);
			}
		}
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

	printf("emu-ini\n");
	ms_cpu_init();
	printf("emu-ini exit\n");

	initSprite();

	if (1) {
		ms_cpu_emulate(emuLoop);
	} else {
		debugger();
	}

	printf("終了します\n");

	_iocs_crtmod(0x10);

	ms_exit();
}

void ms_exit() {
	if ( psg_initialized ) {
		ms_psg_deinit();
	}
	if ( vdp_initialized ) {
		ms_vdp_deinit();
	}
	if ( io_initialized ) {
		ms_iomap_deinit();
	}
	if ( mem_initialized ) {
		ms_memmap_deinit();
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
	//7 [RUP ][RDWN][UNDO][LEFT][UkP  ][RIGT][DOWN][CLR ]
	{   0xf00,0xf00,0xf00,0x810,0x820,0x880,0x840,0xf00},
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
	{   0x601,0x602,0x604,0xf00,0xf00,0xf00,0xf00,0xf00},
	//f [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

void dump(unsigned int page, unsigned int pc_16k);

int emuLoop(unsigned int pc, unsigned int counter) {
	static int emuLoopCounter = 0;
	int i,j;
	unsigned short map;
	unsigned char X, Y;
	int hitkey = 0;
	static int f6KeyHit = 0, f6KeyHitLast = 0;
	static int kigoKeyHit = 0, kigoKeyHitLast = 0;
	static int helpKeyHit = 0, helpKeyHitLast = 0;
	

	emuLoopCounter++;

	kigoKeyHit = 0;
	helpKeyHit = 0;

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
		int v = _iocs_bitsns(i);
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
				else
				{
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
		debug_log_level = (debug_log_level + 1) % 4;
		printf("デバッグログレベル=%d\n", debug_log_level);
	}
	f6KeyHitLast = f6KeyHit;

	if (kigoKeyHit && !kigoKeyHitLast)
	{
		printf("\n");
		printf("emu loop counter=%08d\n", emuLoopCounter);
		printf("COUNTER=%08x, inttick=%08d\n", counter, ms_vdp_interrupt_tick);
		dump(pc >> 16, pc & 0x3fff);
	}
	kigoKeyHitLast = kigoKeyHit;

	if (helpKeyHit && !helpKeyHitLast)
	{
		_toggleTextPlane();
	}
	helpKeyHitLast = helpKeyHit;

	return 0;
}

// メモリダンプ
void dump(unsigned int page, unsigned int pc_16k) {
	int i,j;
	int pcbase,pc;
	int data;
	unsigned int current_int_skip_counter = int_skip_counter;
	static unsigned int last_int_skip_counter = 0;
	int rd,wr;
	char process_type_char[] = {'E','S','D','?'};

	wr = interrupt_history_wr;
	for(i=0;i<32;i++) {
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

	pc = (page << 14) | (pc_16k & 0x3fff);
	pcbase = (pc & 0xfff0);
	printf("PAGE=%04x, PC=%04x, EXEC=%04d SKIP=%04d (+%04d)\n", page, (pc&0xffff), int_exec_counter, current_int_skip_counter,current_int_skip_counter - last_int_skip_counter);
	last_int_skip_counter = current_int_skip_counter;
	for (i = -1; i < 2; i++)
	{
		printf("%04x: ", pcbase + i * 16);
		for (j = 0; j < 16; j++)
		{
			data = readMemFromC(pcbase + i * 16 + j);
			printf("%02x ", data);
		}
		printf("\n");
	}
}

// テキスト表示切り替え
unsigned short* VCON_R02 = (unsigned short*)0x00e82600;

void _toggleTextPlane(void) {
	static int textPlaneMode = 1;

	textPlaneMode = (textPlaneMode + 1) % 2;
	_setTextPlane(textPlaneMode);
}

extern short tx_active;

void _setTextPlane(int textPlaneMode) {
	switch (textPlaneMode)
	{
	case 0:
		// テキスト表示OFF
		*VCON_R02 &= 0xffdf;
		tx_active = 0;
		break;
	case 1:
		// テキスト表示ON
		*VCON_R02 |= 0x0020;
		tx_active = 1;
		break;
	}
}

void allocateAndSetROM(size_t size, const char *romFileName, int kind, int slot, int page)
{
	void *ROM = new_malloc(size + 8);
	if (ROM > (void *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	ms_memmap_set_rom(ROM, romFileName, kind, slot, page);
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

unsigned short* X68_SP_PALETTE = (unsigned short*)0x00e82200; // スプライト/テキストパレット
unsigned short* X68_SSR = (unsigned short*)0x00eb0000; // スプライトスクロールレジスタ
unsigned int* X68_PCG = (unsigned int*)0x00eb8000;

unsigned int* X68_PCG_buffer;
extern unsigned char sprite_size;	// 0: 8x8, 1: 16x16

/*
 スプライトの初期化
 TODO: VDP関連のソースを分離する
 */
void initSprite(void) {
	int i;

	// X68000は 1スプライト(16x16)パターンあたり128バイトが必要
	// MSXは 256個定義できるが、X68000は128個しか定義できないため、メモリ上に定義領域を作っておき
	// 表示時に転送するようにしている
	X68_PCG_buffer = (unsigned int*)malloc( 256 * 32 * sizeof(unsigned int)); 
	if (X68_PCG_buffer > (unsigned int *)0x81000000)
	{
		printf("メモリが確保できません\n");
		ms_exit();
	}
	// PCGバッファの初期化
	for ( i = 0; i < 256 * 32; i++) {
		X68_PCG_buffer[i] = 0;
	}
	// スプライトパレットの初期化
//	for (int i = 1; i < 256; i++) {
//		X68_SP_PALETTE[i] = i;
//	}
}

/*
 スプライトパターンジェネレータテーブルへの書き込み
     offset: パターンジェネレータテーブルのベースアドレスからのオフセットバイト
     pattern: 書き込むパターン(下位8bitのみ使用)
*/
void writeSpritePattern(unsigned char* p, int offset, unsigned int pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine * 2; // MSXの1ラインはX68000では2ライン
	unsigned int pLeft=0,pRight=0; // 1ラインの左4ドットと右4ドットを X68000の8x8のパターン2つに変換

    // 右端のドットから処理
	for(i =0; i < 4;i++) {
		pRight >>= 8;
		if(pattern & 1) {
			pRight |= (0xff000000);
		}
		pattern >>= 1;
	}
    // 残りの左4ドットの処理
	for(i =0; i < 4;i++) {
		pLeft >>= 8;
		if(pattern & 1) {
			pLeft |= (0xff000000);
		}
		pattern >>= 1;
	}
	// パターンジェネレータテーブルへの書き込み
	X68_PCG_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	X68_PCG_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	X68_PCG_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	X68_PCG_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void writeSpriteAttribute(unsigned char* p, int offset, unsigned int attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	switch(type) {
		case 0: // Y座標
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = (((attribute + 1 ) & 0xff) * 2) + (i%2)*16 + 16; // MSXのY座標の2倍, MSXは1ライン下に表示されるので+1, X68000のスプライトの原点は(16,16)なのでずらす
			}
			break;
		case 1: // X座標
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+0] = ((attribute & 0xff) * 2) + (i/2)*16 + 16; // MSXのX座標の2倍
			}
			// TODO ECビットによる位置補正処理
			break;
		case 2: // パターン番号
		case 3: // 属性
			// パターン番号、カラーが変更されたら、事前にバッファに展開しておいたパターンを転送
			unsigned int ptNum = p[(offset & 0x1fffc)+2];
			unsigned int color = p[(offset & 0x1fffc)+3] & 0xf;
			unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color; // MSXの4ドット分(X68000だと2倍の8ドットに拡大)
			if (sprite_size == 0) { // 8x8
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32*4+i] = X68_PCG_buffer[(ptNum & 0xff)*32+i] & colorex;
				}
			} else { // 16x16
				for( i = 0; i < 32*4; i++) { 
					X68_PCG[plNum*32*4+i] = X68_PCG_buffer[(ptNum & 0xfc)*32+i] & colorex;
				}
			}
			break;
		default:
			break;
	}
	if (sprite_size == 0) {
		// 8x8モード
		X68_SSR[plNum*4+2] = 0x100 + plNum; // パレット0x10-0x1fを使用するので 0x100を足す
		X68_SSR[plNum*4+3] = 0x0003; // スプライト表示ON
	} else {
		// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
		for( i=0; i<4; i++) {
			X68_SSR[plNum*16+i*4+2] = 0x100 + plNum*4+i; // パレット0x10-0x1fを使用するので 0x100を足す
			X68_SSR[plNum*16+i*4+3] = 0x0003; // スプライト表示ON
		}
	}

}

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