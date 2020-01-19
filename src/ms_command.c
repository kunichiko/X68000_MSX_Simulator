/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]

			ver.0.01	prpgramed by Kuni.
											1994.10.24

*/


#include <stdio.h>
#include <sys/iocs.h>
#include <sys/dos.h>

char opening[5][33] = {	"Msx Simulater For X680x0\n",
						"                   ver 1.00  \n",
						"System Simulation : MSX2      \n",
						" CPU   Simulation : R800      \n",
						"  OS   Simulation : MSX-DOS2  \n"
					};

extern void *commands[];
extern char *command;

extern int ssp;
extern int com;

/*				画面を初期化				*/
void display_set() {
int i,j;

	_iocs_apage( 0);

	_iocs_tgusemd( 0, 2);
	_iocs_crtmod( 4);
	_iocs_b_curoff();
	_iocs_g_clr_on();
	_iocs_sp_init();
	_iocs_sp_on();

	_iocs_skey_mod(0,0,0);
}

/*	ファイル一覧を表示する関数	*/

void dir() {


}

/*
_files:
	link	a6,#0
	movem.l	d0-d7/a0-a6,-(sp)

	movea.l	8(a6),a2	* a2 = 結果を返す配列へのポインタ(ファイルネーム)
	movea.l	12(a6),a4	* a4 = 結果を返す配列へのポインタ(ファイルの種類)
				*	0--- 普通のファイル
				*	1--- cedファイル
				*	2--- ディレクトリ
				*	3--- 全て読み込んだことを示す

	move.w	#%0011_0000,-(sp)	* 通常のファイルとディレクトリを検索
	pea.l	fnamebuf2
	pea.l	filebuf
	DOS	_FILES
	lea.l	10(sp),sp

loopf:	tst.l	d0
	bmi	donef

	lea.l	filebuf+30,a3
	clr.w	d2
	move.w	#23,d0
loopf1:
	move.b	(a3)+,d1
	cmpi.b	#'.',d1			* 拡張子にぶつかった
	beq	exp
	cmpi.b	#0,d1			* ヌル文字にぶつかった
	beq	null
	move.b	d1,(a2)+

	subq.w	#1,d0
	bne	loopf1

	clr.l	d0			* 通常のファイル

nextf:
	move.l	d0,(a4)+
	lea.l	filebuf+21,a3
	move.b	(a3),d1
	and.b	#%0001_0000,d1		* ディレクトリか？
	beq	normal
	move.l	#2,-4(a4)		* ディレクトリだった
normal:
	pea.l	filebuf
	DOS	_NFILES
	addq.l	#4,sp

	bra	loopf

donef:
	move.l	#3,(a4)			* ファイルの種類３は、すべて読み切ったことを示す
	movem.l	(sp)+,d0-d7/a0-a6
	unlk	a6

	rts

*	拡張子の処理
exp:
	subq.w	#5,d0
loope:
	move.b	#' ',(a2)+
	subq.w	#1,d0
	bne	loope

	move.b	d1,(a2)+
	move.w	#4,d0
loopf2:
	move.b	(a3)+,(a2)+
	subq.w	#1,d0
	bne	loopf2

	move.b	-4(a2),d0
	cmpi.b	#'c',d0
	beq	next_e
	cmpi.b	#'C',d0
	bne	not_ced			* １文字目が'c'でも'C'でもない
next_e:
	move.b	-3(a2),d0
	cmpi.b	#'e',d0
	beq	next_d
	cmpi.b	#'E',d0
	bne	not_ced			* １文字目が'e'でも'E'でもない
next_d:
	move.b	-2(a2),d0
	cmpi.b	#'d',d0
	beq	next_ced
	cmpi.b	#'D',d0
	bne	not_ced			* １文字目が'd'でも'D'でもない

next_ced:
	move.l	#1,d0			* ced ファイル
	bra	nextf

not_ced:
	clr.l	d0
	bra	nextf			* 通常のファイル

*	ヌル文字にぶつかった時の処理
null:
	subq.w	#1,d0
loopn:
	move.b	#' ',(a2)+		* ファイルの最後までをスペースで埋める
	subq.w	#1,d0
	bne	loopn

	move.b	#0,(a2)+		* ヌル文字をつける

	clr.l	d0			* 通常のファイル
	bra	nextf

*/

void assign() {
}

void atdir() {
}

void attrib() {
}

void basic() {
}

void buffers() {
}

void chdir() {
}

void chkdsk() {
}


void exit() {
/*	_iocs_b_super( ssp);			*/
	_iocs_b_consol(0, 0, 64,32);
	_dos_exit();
}

void nothing() {						/* なにもしないコマンド			*/
char errmes[] = "\ncommand or file not found\n";

	print( errmes);

	return;
}

/****************** コマンド設定 ******************/
/*
void *commands[64];			 それぞれのコマンドの関数へのポインタ配列	
char command_name[64][10]={"ASSIGN","ATDIR" ,"ATTRIB","BASIC"   ,"BUFFERS","CD",
							"CHDIR" ,"CHKDSK","CLS"   ,"COMMAND2","CONCAT" ,"COPY",
							"DATE"  ,"DEL"   ,"DIR"   ,"ECHO"    ,"ERA"    ,"ERASE",
							"EXIT"  ,"FORMAT","HELP"  ,"IF"      ,"MD"     ,"MKDIR",
							"MODE"  ,"MOVE"  ,"MVDIR" ,"PATH"    ,"PAUSE"  ,"RAMDISK",
							"RD"    ,"RD"    ,"REM"   ,"REN"     ,"RENAME" ,"RMDIR",
							"RNDIR" ,"SET"   ,"TIME"  ,"TYPE"    ,"UNDEL"  ,"VER",
							"VERIFY","VOL"};
*/

void command_init() {
int i;

	for(i=0;i<64;i++)
		commands[i] = nothing;

	commands[ 0] = assign;
	commands[ 1] = atdir;
	commands[ 2] = attrib;
	commands[ 3] = basic;
	commands[ 4] = buffers;
	commands[ 5] = chdir;
	commands[ 6] = chdir;
	commands[18] = exit;
}

void command_ms() {
int i,a;

	command_init();						/* コマンドへのポインタ配列の初期化など	*/

	for( i=0; i<5; i++) {
		print( &opening[i]);
	}

	for(;;) {
		print("A>");
		com = 0;							/* 現在入力されている文字数 	*/

		while(1) {
			a = _dos_inkey();
			if( a >= 32 ) {
				if(com < 255)
					command[com++] = (char)a;
				put( a);					/* 文字表示						*/
			} else {
				switch (a) {
					case  8: if( com != 0) {
								com--;
								put( a);
							}
							break;
					case 13: goto next;
				}
			}
			if ((_iocs_bitsns( 0) & 2) == 2)	/* Ｅｓｃキーが押されていたら終了		*/
				/* exit();*/
				goto end;
		}

	next:
		command[com++] = '\0';				/* 文字列の最後に null をいれる	*/
		print("\n");						/* 改行							*/
		do_command( command);
	}
end:
	_iocs_b_super( ssp);
	_iocs_b_consol(0, 0, 64,32);
}
