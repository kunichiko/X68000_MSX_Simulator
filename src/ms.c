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
#include "disk/ms_disk_container.h"
#include "peripheral/ms_rtc.h"
#include "peripheral/ms_kanjirom12.h"
#include "peripheral/ms_kanjirom_alt.h"

ms_init_params_t default_param;
ms_init_params_t init_param;
ms_init_params_t user_param;

// プロトタイプ宣言
void ms_exit( void);
uint8_t load_user_param();
int search_open(const char *filename, int flag);

// 現在の設定ログレベル
// デバッグテキスト画面が出ている時だけこのログレベルになる
static int _debug_log_level = MS_LOG_INFO;

#ifdef DEBUG
static int textPlaneMode = 1;
#else
static int textPlaneMode = 0;
#endif

// メモリ関連
ms_memmap_t* memmap = NULL;

// I/O関連
ms_iomap_t* iomap = NULL;

// RTC関連
ms_rtc_t* rtc = NULL;

// VDP関連
ms_vdp_t* vdp = NULL;  // ms_vdp_shared と同じになるはず

// PSG関連
int psg_initialized = 0;
int ms_psg_init( void);
void ms_psg_deinit(void);

// Disk関連
ms_disk_container_t* disk_container = NULL;

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

char base_dir[256];
void set_system_roms(void);

void _toggleTextPlane(void);
void _setTextPlane(int textPlaneMode);
void _moveTextPlane(int cursorKeyHit);

unsigned short host_rate = 1;

volatile extern unsigned short ms_vsync_interrupt_tick;
volatile extern unsigned short ms_vdp_vsync_rate;
volatile extern unsigned int int_block_count;
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
	fprintf(stderr, "Usage: %s  [-w MAX_WAIT] [-rm MAINROM] [-rs SUBROM] [-rd DISKBIOS] [-rk KANJIROM] [-r1 ROM_PATH for slot 1][,KIND] [-r2 ROM_PATH for slot 2][,KIND] [-rNM ROM_PATH for slot N page M] [IMAGE1.DSK] [IMAGE2.DSK]..\n", progname);
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
	fprintf(stderr, "    default is 100 cycles.\n");
	fprintf(stderr, " --hostdebug\n");
	fprintf(stderr, "    enable host process debug mode.\n");
	fprintf(stderr, " --disablekanji");
	fprintf(stderr, "    disable kanji ROM.\n");
	fprintf(stderr, " --disablekey\n");
	fprintf(stderr, "    disable key input for performance test.\n");
//	fprintf(stderr, " --debuglevel N\n");
//	fprintf(stderr, "    0: None, 1: Info, 2: Debug, 3: Fine.\n");
	fprintf(stderr, " --safe\n");
	fprintf(stderr, "    safe mode. disable reading MS.INI.\n");
	exit(EXIT_FAILURE);
}

int disablekanji = 0;
int disablekey = 0;
int safemode = 0;
int hostdebug = 0;

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
	} else {
		*kind = -1;
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
#ifdef DEBUG
	debug_log_level = MS_LOG_DEBUG;
#else 
	debug_log_level = MS_LOG_INFO;
#endif
	// argv[0]から、実行ファイル名を取り除き、ディレクトリ名を取得
	// そのディレクトリをベースディレクトリとして設定
	strncpy(base_dir, argv[0], sizeof(base_dir) - 1);
    base_dir[sizeof(base_dir) - 1] = '\0'; // 文字列の終端を保証
	char *last_separator = strrchr(base_dir, '\\');
	if (last_separator != NULL) {
		// 最後の \ は残す
		*(last_separator+1) = '\0';
	} else {
        printf("ディレクトリ名の取得に失敗しました\n");
        return 1;
    }

	int i, j;
	int opt;
    const char* optstring = "hm:s:w:r:" ; // optstringを定義します
    const struct option longopts[] = {
      //{          *name,           has_arg,       *flag, val },
        {       "vsrate", required_argument,             0, 'A' },
        {     "intblock", required_argument,             0, 'B' },
        {     "hostrate", required_argument,             0, 'C' },
        {    "hostdelay", required_argument,             0, 'D' },
		{    "hostdebug",       no_argument,    &hostdebug,  1  },
		{ "disablekanji",       no_argument, &disablekanji,  1  },
		{   "disablekey",       no_argument,   &disablekey,  1  },
		{         "safe",       no_argument,     &safemode,  1  },
        {              0,                 0,             0,  0  }, // termination
    };
	const struct option* longopt;
    int longindex = 0;

	printf("[[ MSX Simulator MS.X v%s]]\n", MS_dot_X_VERSION);

	unsigned int mpu_type = _iocs_mpu_stat();
	if( (mpu_type & 0xf) < 3) {
		printf("MS.X の動作には 68030以上が必要です\n");
		ms_exit();
	}

	// デフォルトの初期化
	default_param.buf = NULL;
	default_param.diskrom = NULL;
	default_param.kanjirom = NULL;
	for(i=0;i<4;i++) {
		for(j=0;j<4;j++) {
			default_param.slot_path[i][j] = NULL;
		}
	}
	default_param.cartridge_path_slot1 = NULL;
	default_param.cartridge_kind_slot1 = -1;
	default_param.cartridge_path_slot2 = NULL;
	default_param.cartridge_kind_slot2 = -1;
	default_param.max_wait = 0x7fffffff;
	default_param.diskcount = 0;
	for(i=0;i<16;i++) {
		default_param.diskimages[i] = NULL;
	}
	default_param.mainrom = "cbios_main_msx2_jp.rom";
	default_param.subrom = "cbios_sub.rom";
	default_param.slot_path[0][2] = "cbios_logo_msx2.rom";

	// ユーザー設定ファイルの読み込み
	if( load_user_param() ) {
		// 読み込みに成功した場合は、デフォルトの設定をユーザー設定で上書き
		init_param = user_param;
	} else {
		// 読み込みに失敗した場合は、デフォルトの設定を使用
		init_param = default_param;
	}

	// その他ワークエリアの初期化
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
			// もしセーフモードだったら、デフォルトのパラメータに戻す
			if (safemode) {
				printf("セーフモードで起動します。続行する場合は何かキーを押してください。\n");
				_iocs_b_keyinp();
				init_param = default_param;
			}
			break;
		case 'h': // -h オプション
			printHelpAndExit(argv[0]);
			break;
		case 'm': // -m オプション
			if (optarg != NULL)
			{
				init_param.mainrom = optarg;
			}
			break;
		case 'w': // -w オプション
			if (optarg != NULL)
			{
				init_param.max_wait = atoi(optarg);
			}
			break;
		case 's': // -s オプション
			if (optarg != NULL)
			{
				init_param.subrom = optarg;
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
						init_param.mainrom = argv[optind++];
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
						init_param.subrom = argv[optind++];
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
						init_param.diskrom = argv[optind++];
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
					break;
				case 'k':
					// 漢字ROMの指定
					if (argv[optind] != NULL)
					{
						init_param.kanjirom = argv[optind++];
					}
					else
					{
						printf("ROMファイル名が指定されていません\n");
						ms_exit();
					}
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
							init_param.cartridge_path_slot1 = argv[optind++];
							init_param.cartridge_path_slot1 = separate_rom_kind(init_param.cartridge_path_slot1, &init_param.cartridge_kind_slot1);
						} else {
							init_param.cartridge_path_slot2 = argv[optind++];
							init_param.cartridge_path_slot2 = separate_rom_kind(init_param.cartridge_path_slot2, &init_param.cartridge_kind_slot2);
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
						init_param.slot_path[slot][page] = argv[optind++];
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
					init_param.cartridge_path_slot1 = optarg;
					init_param.cartridge_path_slot1 = separate_rom_kind(init_param.cartridge_path_slot1, &init_param.cartridge_kind_slot1);
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
	// init_param.diskcountはこの時点で 1以上になっていることもあるので注意
	for (i = optind; i < argc; i++) {
		if (strcasestr(argv[i], ".dsk") != 0) {
			init_param.diskimages[init_param.diskcount++] = argv[i];
			if(init_param.diskcount >= 16) {
				printf("ディスクイメージの数が多すぎます\n");
				ms_exit();
			}
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
	memmap = ms_memmap_shared_instance();
	if (memmap == NULL)
	{
		printf("メモリシステムの初期化に失敗しました\n");
		ms_exit();
	}

	/*
	 VDPシステムの初期化
	*/
	vdp = ms_vdp_shared_instance();
	if (vdp == NULL)
	{
		printf("VDPシステムの初期化に失敗しました\n");
		ms_exit();
	}

	/*
	 I/Oシステムの初期化
	 */
	iomap = ms_iomap_shared_instance();
	if (iomap == NULL)
	{
		printf("I/Oシステムの初期化に失敗しました\n");
		ms_exit();
	}

	printf("\n\n\n\n\n\n\n\n"); // TEXT画面を上に8ラインくらい上げているので、その分改行を入れる
	printf("\n\n\n\n\n\n\n\n"); // 256ドットモードだとさらに見えなくなるので、もう少し下げる
	printf("[[ MSX Simulator MS.X v%s]]\n", MS_dot_X_VERSION);
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
	 RTCの初期化
	 */
	rtc = ms_rtc_alloc();
	if (rtc == NULL)
	{
		printf("RTCの初期化に失敗しました\n");
		ms_exit();
	}
	ms_rtc_init(rtc, iomap);

	/*
	 漢字ROMのセット
	 */
	if (disablekanji) {
		printf("漢字ROMを無効化します\n");
	} else if (init_param.kanjirom != NULL) {
		ms_kanjirom12_t* k12 = ms_kanjirom12_alloc();
		if (k12 == NULL) {
			printf("漢字ROMの初期化に失敗しました\n");
			ms_exit();
		}
		ms_kanjirom12_init(k12, iomap, init_param.kanjirom);
		printf("KANJIROM: %s\n", init_param.kanjirom);
	} else {
		printf("代替漢字ROMを使用します\n");
		ms_kanjirom_alt_t* k_alt = ms_kanjirom_alt_alloc();
		if (k_alt == NULL) {
			printf("代替漢字ROMの初期化に失敗しました\n");
			ms_exit();
		}
		ms_kanjirom_alt_init(k_alt, iomap);
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
			if ( init_param.slot_path[i][j] != NULL) {
				printf("スロット%d-ページ%dにROMをセットします: %s\n", i, j, init_param.slot_path[i][j]);
				int fh = search_open(init_param.slot_path[i][j], O_BINARY | O_RDONLY);
				if ( fh == -1) {
					printf("ファイルが開けません. %s\n", init_param.slot_path[i][j]);
				} else {
					allocateAndSetROMwithHandle(fh, ROM_TYPE_NORMAL_ROM, i, -1, j);
				}
			}
		}
	}

	// サイズを自動判別してROMをセット
	if (init_param.cartridge_path_slot1 != NULL) {
		printf("Cartridge slot 1: %s\n", init_param.cartridge_path_slot1);
		allocateAndSetROM_Cartridge(init_param.cartridge_path_slot1, 1, init_param.cartridge_kind_slot1);
	}
	if (init_param.cartridge_path_slot2 != NULL) {
		printf("Cartridge slot 2: %s\n", init_param.cartridge_path_slot2);
		allocateAndSetROM_Cartridge(init_param.cartridge_path_slot2, 2, init_param.cartridge_kind_slot2);
	}

	MS_LOG(MS_LOG_DEBUG, "VSYNCレート=%d, ホスト処理レート=%d\n", ms_vdp_vsync_rate, host_rate);
	MS_LOG(MS_LOG_DEBUG, "VSYNC計測中...\n");
	{
		volatile int date,lastdate;
		volatile int start,end;

		date = _iocs_timeget();
		lastdate = date;
		while(date == lastdate) {	// 秒が変わる瞬間を待つ
			lastdate = date;
			date = _iocs_timeget();
		}
		start = ms_vsync_interrupt_tick;		// そのときのtickを取得
		date = _iocs_timeget();
		lastdate = date;
		while(date == lastdate) {	// 秒が変わる瞬間を待つ
			lastdate = date;
			date = _iocs_timeget();
		}
		end = ms_vsync_interrupt_tick;		// そのときのtickを取得

		MS_LOG(MS_LOG_DEBUG, "VSYNC回数は %d です\n", end - start);
	}

	// 	全ページの スロットを０で初期化
	write_port_A8(0);

	// テキスト画面の表示/非表示を設定
	_setTextPlane(textPlaneMode);

	if (1) {
		ms_cpu_emulate(emuLoop, init_param.max_wait);
	} else {
		debugger();
	}

	ms_exit();
}

void ms_exit() {
	printf("終了します。何かキーを押してください。\n");
	_iocs_b_keyinp();

	_iocs_crtmod(0x10);

	if( disk_container != NULL ) {
		ms_disk_container_deinit(disk_container);
		new_free(disk_container);
		//disk_container = NULL;
	}
	if ( psg_initialized ) {
		ms_psg_deinit();
	}
	if ( vdp != NULL ) {
		ms_vdp_shared_deinit();	// singletonは deinit内部でfreeされる
		vdp = NULL;
	}
	if ( rtc != NULL ) {
		ms_rtc_deinit(rtc, iomap); // singletonは deinit内部でfreeされる
		rtc = NULL;
	}
	if ( iomap != NULL ) {
		ms_iomap_shared_deinit(); // singletonは deinit内部でfreeされる
		iomap = NULL;
	}
	if ( memmap != NULL ) {
		ms_memmap_shared_deinit(); // singletonは deinit内部でfreeされる
		memmap = NULL;
	}
	if( user_param.buf != NULL) {
		new_free(user_param.buf);
		user_param.buf = NULL;
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
	//   BIT0  BIT1  BIT2  BIT3  BIT4  BIT5  BIT6  BIT7
	//0 [    ][ESC ][1   ][2   ][3   ][4   ][5   ][6   ] 
	{   0xf00,0xf10,0xf11,0xf12,0xf13,0xf14,0xf15,0xf16},
	//1 [7   ][8   ][9   ][0   ][-   ][^   ][\   ][BS  ] 
	{   0xf17,0xf18,0xf19,0x001,0x104,0x108,0x110,0x720},
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
	{   0xf00,0x710,0xf00,0x620,0x640,0x680,0x701,0x702},
	//d [ F6 ][ F7 ][ F8 ][ F9 ][ F10][    ][    ][    ]  F6=DebugLevel
	{   0xffc,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//e [SHFT][CTRL][OPT1][OPT2][    ][    ][    ][    ]  OPT1=Disk Change
	{   0xff0,0x602,0xff8,0xff9,0xf00,0xf00,0xf00,0xf00},
	//f [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

void dump(unsigned int page, unsigned int pc_8k);

void sync_keyboard_leds();

void willEnterEmuLoop() {
	if( hostdebug ) {
		// デバッグモードの場合は、画面上の色を変更して、どのタイミングで実行されているかを可視化する
		X68_TX_PAL[0] = 0xffff;
	}
}

void didExitEmuLoop() {
	if( hostdebug ) {
		// デバッグモードの場合は、画面上の色を変更して、どのタイミングで実行されているかを可視化する
		X68_TX_PAL[0] = 0x0000;
	}
}

int emuLoopImpl(unsigned int pc, unsigned int counter);

int emuLoop(unsigned int pc, unsigned int counter) {
	willEnterEmuLoop();
	int ret = emuLoopImpl(pc, counter);
	didExitEmuLoop();
	return ret;
}

int emuLoopImpl(unsigned int pc, unsigned int counter) {
	static int emuLoopCounter = 0;
	static uint8_t last_bitsns[16];

	int i,j;
	unsigned short map;
	unsigned char X, Y;
	int hitkey = 0;
	int shiftKeyPress = 0;
	int cursorKeyHit = 0; // 1=LEFT, 2=UP, 3=DOWN, 4=RIGHT
	int f6KeyHit = 0;
	int kigoKeyHit = 0;
	int helpKeyHit = 0;
	int disk_change = -1; // 0=Eject, 1=Disk1, 2=Disk2, 3=Disk3, 4=Disk4

	emuLoopCounter++;

	if( vdp != NULL) {
		ms_vdp_vsync_draw(vdp, hostdebug);
	}

	if(emuLoopCounter % host_rate != 0) {
		return 0;
	}

	sync_keyboard_leds();

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
		int mask = 1;
		for ( j = 0; j < 8; j++) {
			if (v & mask) {
				hitkey = 1;
				map = KEY_MAP[i][j];
				Y = (map & 0xf00) >> 8;
				X = (map & 0xff);
				if (Y == 0xf)
				{
					int opt1_pressed = ((uint8_t*)0x800)[0xe] & 0x04;
					// 特殊キー
					switch (X)
					{
					// *********************************************
					// 0x10-0x1f: Disk Change (Opt.1のPressが必要)
					case 0x10: // ESCキー 0x704 (Eject)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 0;
							}
						} else {
							Y = 0x7;
							X = 0x04;
						}
						break;
					case 0x11: // 1キー 0x002 (Disk1)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 1;
							}
						} else {
							Y = 0x0;
							X = 0x02;
						}
						break;
					case 0x12: // 2キー 0x004 (Disk2)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 2;
							}
						} else {
							Y = 0x0;
							X = 0x04;
						}
						break;
					case 0x13: // 3キー 0x008 (Disk3)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 3;
							}
						} else {
							Y = 0x0;
							X = 0x08;
						}
						break;
					case 0x14: // 4キー 0x010 (Disk4)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 4;
							}
						} else {
							Y = 0x0;
							X = 0x10;
						}
						break;
					case 0x15: // 5キー 0x020 (Disk5)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 5;
							}
						} else {
							Y = 0x0;
							X = 0x20;
						}
						break;
					case 0x16: // 6キー 0x040 (Disk6)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 6;
							}
						} else {
							Y = 0x0;
							X = 0x40;
						}
						break;
					case 0x17: // 7キー 0x080 (Disk7)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 7;
							}
						} else {
							Y = 0x0;
							X = 0x80;
						}
						break;
					case 0x18: // 8キー 0x101 (Disk8)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 8;
							}
						} else {
							Y = 0x1;
							X = 0x01;
						}
						break;
					case 0x19: // 9キー 0x102 (Disk9)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 9;
							}
						} else {
							Y = 0x1;
							X = 0x02;
						}
						break;
					// *********************************************
					case 0xf0: // SHIFTキー 0x601
						Y = 0x6;
						X = 0x01;
						shiftKeyPress = 1;
						break;
					case 0xf1: // LEFTキー 0x810
						Y = 0x8; 
						X = 0x10;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 1;
						}
						break;
					case 0xf2: // UPキー 0x820
						Y = 0x8; 
						X = 0x20;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 2;
						}
						break;
					case 0xf3: // DOWNキー 0x840
						Y = 0x8; 
						X = 0x40;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 3;
						}
						break;
					case 0xf4: // RIGHTキー 0x880
						Y = 0x8; 
						X = 0x80;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 4;
						}
						break;
					case 0xf8: // OPT.1
						break;
					case 0xf9: // OPT.2
						break;
					case 0xfc: // F6キーを押すと、デバッグレベル変更
						if(!(last_bitsns[i] & mask)) { // edge detect
							f6KeyHit = 1;
						}
						break;
					case 0xfd: // 記号キーを押すと、PC周辺のメモリダンプ
						if(!(last_bitsns[i] & mask)) { // edge detect
							kigoKeyHit = 1;
						}
						break;
					case 0xfe: // HELPキーを押すとテキスト表示切り替え
						if(!(last_bitsns[i] & mask)) { // edge detect
							helpKeyHit = 1;
						}
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
			mask <<= 1;
		}
	}
	if (hitkey)
	{
		// 文字入力を読み捨てて、キーバッファを空にする
		_dos_kflushio(0xff);
	}

	if (f6KeyHit) {
		if(shiftKeyPress) {
			// シフトキーと同時押しの場合は、デバッグログレベルを下げる
			debug_log_level = max(0, debug_log_level - 1);
		} else {
			// それ以外の場合は、デバッグログレベルを上げる
			debug_log_level = min(7, debug_log_level + 1);
		}
		printf("デバッグログレベル=%d\n", debug_log_level);
	}

	if (kigoKeyHit) {
		printf("\n");
		printf("loop count=%08d\ncycle=%08ld wait=%ld\n", emuLoopCounter, cpu_cycle_last, cpu_cycle_wait);
		printf("COUNTER=%08x, inttick=%08d\n", counter, ms_vsync_interrupt_tick);
		dump(pc >> 16, pc & 0x1fff);
	}

	if (helpKeyHit) {
		_toggleTextPlane();
	}

	if (shiftKeyPress && cursorKeyHit ) {
		_moveTextPlane(cursorKeyHit);
	}

	if( disk_change == 0) {
		disk_container->eject_disk(disk_container);
		printf("Disk ejected\n");
	} else if (disk_change > 0) {
		disk_container->change_disk(disk_container, disk_change - 1);
		printf("Disk changed: %s\n", disk_container->current_disk->name);
	}

	for(i=0;i<16;i++) {
		last_bitsns[i] = ((uint8_t*)0x800)[i];	// IOCS BITSNSのワークエリア直接参照し、一つ前の値を覚えておく
	}

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
	printf("PC=%04x, EXEC=%04d SKIP=%04d (+%04d)\n", (pc&0xffff), int_exec_counter, current_int_skip_counter,current_int_skip_counter - last_int_skip_counter);
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
static uint8_t last_led_val = 0;

uint8_t ms_fdd_led_1;
uint8_t ms_fdd_led_2;

void sync_keyboard_leds() {
	if (keyboard_led_counter == 0) {
		keyboard_led_counter = 3;
		uint8_t led_val = 0;
		led_val |= ms_peripherals_led_caps ? 0b00001000 : 0;	// CAPS
		led_val |= ms_peripherals_led_kana ? 0b00000001 : 0;	// KANA
		led_val |= ms_fdd_led_1            ? 0b00000100 : 0;	// CODE
		led_val |= ms_fdd_led_2            ? 0b00000010 : 0;	// ROME
		if( last_led_val != led_val) {
			ms_iocs_ledctrl(led_val, ms_peripherals_led_caps);
			last_led_val = led_val;
		}
	} else {
		keyboard_led_counter--;
	}
}

void _toggleTextPlane(void) {
	textPlaneMode = (textPlaneMode + 1) % 2;
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
		// テキスト表示OFFにする時に、デバッグログをINFOにする
		debug_log_level = MS_LOG_INFO;
		// テキスト表示OFF
		vdp->tx_active = 0;
		break;
	case 1:
		// テキスト表示ONにする時に、デバッグログを元に戻す
		debug_log_level = _debug_log_level;
		// テキスト表示ON
		vdp->tx_active = 1;
		break;
	}
	ms_vdp_update_visibility(vdp);
	vdp->ms_vdp_current_mode->update_palette(vdp);
}


int search_open(const char *filename, int flag) {
	if(filename == NULL) {
		return -1;
	}
    int fh = open(filename, flag);
    if (fh != -1) {
        return fh;
    }
	// ベースディレクトリも検索
	char base_filename[256];
	sprintf(base_filename, "%s%s", base_dir, filename);
	fh = open(base_filename, flag);
	if (fh != -1) {
		return fh;
	}
    return -1;
}

int file_exists(const char *filename) {
	if(filename == NULL) {
		return 0;
	}
	int fh = open(filename, O_RDONLY);
	if (fh != -1) {
		close(fh);
		return 1;
	}
	return 0;
}

void set_system_roms() {
	int fh_mainrom = search_open(init_param.mainrom, O_BINARY | O_RDONLY);
	int fh_subrom = search_open(init_param.subrom, O_BINARY | O_RDONLY);
	if (fh_mainrom != -1 && fh_subrom != -1) {
		// Load user-provided ROMs
		printf("MAIN ROM: %s\n", init_param.mainrom);
		allocateAndSetROMwithHandle(fh_mainrom, ROM_TYPE_NORMAL_ROM, 0x00, -1, 0);
		printf(" SUB ROM: %s\n", init_param.subrom);
		allocateAndSetROMwithHandle(fh_subrom, ROM_TYPE_NORMAL_ROM, 0x03, 1, 0);
		if (file_exists(init_param.diskrom)) {
			printf("DISK ROM: %s\n", init_param.diskrom);
			int i;
			for(i=0;i<init_param.diskcount;i++) {
				printf(" Load disk [%d] : %s\n", i, init_param.diskimages[i]);
			}
			// ディスクコンテナの初期化
			disk_container = ms_disk_container_alloc();
			if (disk_container == NULL) {
				printf("メモリが確保できません。\n");
				ms_exit();
				return;
			}
			ms_disk_container_init(disk_container, init_param.diskcount, init_param.diskimages);

			allocateAndSetDISKBIOSROM(init_param.diskrom, disk_container);
		}
    } else {
		if (fh_mainrom != -1) {
			close(fh_mainrom);
		}
		if (fh_subrom != -1) {
			close(fh_subrom);
		}
        // Load default CBIOS ROMs
		printf("BIOS ROMが見つかりません。ファイルを確認してください。\n");
		printf(" BIOS ROM: %s\n", init_param.mainrom);
		printf(" SUB ROM : %s\n", init_param.subrom);
		ms_exit();
		return;
    }
}

/**
 * @brief MS.INI ファイルを読み込み、初期化パラメータを設定します
 * 
 * @return uint8_t ロードできた場合1、失敗した場合0
 */
uint8_t load_user_param() {
	user_param = default_param;

	int fh = search_open("MS.INI", O_RDONLY); // TEXT MODE で開く
	if(fh == -1) {
		printf("MS.INI ファイルを開けませんでした。\n");
		return 0;
	}
	int size = filelength(fh);
	user_param.buf = (uint8_t*)new_malloc(size + 1);
	if(user_param.buf == NULL) {
		printf("メモリが確保できません。\n");
		close(fh);
		return 0;
	}
	read(fh, user_param.buf, size);
	close(fh);
	user_param.buf[size] = '\0';

	user_param.diskcount = 0;
	// 1行ずつ読みながらパラメータを設定
	// Read each line from the MS.INI file
	int pos = 0;
	while (pos < size) {
		while(pos<size) {
			if( user_param.buf[pos] == '\n' || user_param.buf[pos] == '\r') {
				// 連続する改行をスキップ
				pos++;
			} else {
				break;
			}
		}
		if(pos >= size) {
			break;
		}
		char* line = user_param.buf + pos;
		while(pos<size && user_param.buf[pos] != '\n') {
			pos++;
		}
		user_param.buf[pos++] = '\0';

		// 行がコメント行かどうかをチェック
		if (line[0] == ';' || line[0] == '#') {
			continue;
		}

		// Parse the line to extract the parameter and value
		char* param = strtok(line, "=");
		char* value = strtok(NULL, "=");

		// Check if the parameter is "mainrom"
		if (strcmp(param, "mainrom") == 0) {
			user_param.mainrom = value;
			if( user_param.slot_path[0][2] == default_param.slot_path[0][2] ) {
				// C-BIOSのロゴを外す
				user_param.slot_path[0][2] = NULL;
			}
		}
		// Check if the parameter is "subrom"
		else if (strcmp(param, "subrom") == 0) {
			user_param.subrom = value;
		}
		// Check if the parameter is "diskrom"
		else if (strcmp(param, "diskrom") == 0) {
			user_param.diskrom = value;
		}
		// Check if the parameter is "kanjirom"
		else if (strcmp(param, "kanjirom") == 0) {
			user_param.kanjirom = value;
		}
		// Check if the parameter starts with "slot"
		else if (strncmp(param, "slot", 4) == 0) {
			// Extract the slot number and page number from the parameter
			int slot = param[4] - '0';
			int page = param[6] - '0';

			// Set the ROM path for the specified slot and page
			user_param.slot_path[slot][page] = value;
		}
		// Check if the parameter starts with "r"
		else if (strncmp(param, "r", 1) == 0) {
			// Extract the slot number and page number from the parameter
			int slot = param[1] - '0';
			int page = param[2] - '0';

			// Set the ROM path for the specified slot and page
			user_param.slot_path[slot][page] = value;
		}
		// Check if the parameter is "cart1"
		else if (strcmp(param, "cart1") == 0) {
			user_param.cartridge_path_slot1 = separate_rom_kind(value, &user_param.cartridge_kind_slot1);
		}
		// Check if the parameter is "cart2"
		else if (strcmp(param, "cart2") == 0) {
			user_param.cartridge_path_slot2 = separate_rom_kind(value, &user_param.cartridge_kind_slot2);
		}
		// Check if the parameter is "diskimage"
		else if (strcmp(param, "diskimage") == 0) {
			if(user_param.diskcount >= 16) {
				printf("ディスクイメージの数が多すぎます\n");
			} else {
				user_param.diskimages[user_param.diskcount++] = value;
			}
		}
	}

	return 1;
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