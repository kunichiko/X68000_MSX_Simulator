/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[ 総合システム ]]
								
	ver. 0.01	prpgramed by Kuni.
										1995.3.5

*/

#include <stdio.h>
#include <string.h>
#include <sys/iocs.h>
#include <sys/dos.h>
#include "ms.h"

void *MMem;						/* メインメモリの先頭アドレス			*/
void *VRAM;						/* ＶＲＡＭの先頭アドレス				*/

char fontfile[] = "font.fon";
					
char command[256] = "";
void (*commands[64])( int, char *[]);	/* それぞれのコマンドの関数へのポインタ配列	*/
char command_name[64][10]={"ASSIGN","ATDIR" ,"ATTRIB","BASIC"   ,"BUFFERS","CD",
							"CHDIR" ,"CHKDSK","CLS"   ,"COMMAND2","CONCAT" ,"COPY",
							"DATE"  ,"DEL"   ,"DIR"   ,"ECHO"    ,"ERA"    ,"ERASE",
							"EXIT"  ,"FORMAT","HELP"  ,"IF"      ,"MD"     ,"MKDIR",
							"MODE"  ,"MOVE"  ,"MVDIR" ,"PATH"    ,"PAUSE"  ,"RAMDISK",
							"RD"    ,"RD"    ,"REM"   ,"REN"     ,"RENAME" ,"RMDIR",
							"RNDIR" ,"SET"   ,"TIME"  ,"TYPE"    ,"UNDEL"  ,"VER",
							"VERIFY","VOL"};

int	com;
int	ssp;						/* スーパーバイザーモードへ行く時に使用 	*/

int screen_mode;				/* ＭＳＸでのスクリーンモード				*/
								/* ６８では、0～7 の時 512*512    16c		*/
								/* 			 8～12の時 512*512 65536cにする	*/

int width_size;					/* 文字表示幅								*/
								
void do_command( char *command ) {
int i,j;
char *arguments[128];			/* 入力されたコマンドのスペースをヌル文字変え、	*/
								/* それぞれの文字列の先頭アドレスを arguments[]	*/
	j = 0;						/* というポインタ配列へ入れる。					*/
	i = 0;
	while( command[i] == ' ' ) {
		i++;						/* 連続するスペースをスキップ				*/
	}
	arguments[j++] = command + i;	/* arguments[0] は第一引数へのポインタ		*/
	for(;i<com; i++) {
		if(command[i] == ' ') {
			command[i++] ='\0';		/* とりあえず引数の終りにヌル文字を付ける	*/ 
			while( command[i] == ' ' ) {
				i++;				/* 連続するスペースをスキップ				*/
			}
			arguments[j++] = command + i;
		}
	}
	arguments[j] = 0;				/* 最後のポインタはヌルポインタ				*/
									/* j は引数の数								*/
	if( *arguments[0] == '\0')		/* スペース以外何も入力されていない時は		*/
		return;						/* リターン									*/

	for(i=0; i<64; i++) {
		if( stricmp( arguments[0], command_name[i]) == 0) {
			(*commands[i])( j, arguments);
			return;
		}
	}
	nothing();
}

void screen( int mode) {
	set_screen( mode);			/* アセンブラのルーチンを呼び出し	*/
	screen_mode = mode;
}

void width( int size) {
	set_width( size);
	width_size = size;
}

/* ==========================  メイン  ==================================== */

void main() {
int crtmod;							/* 現在の画面状態 0=256*256 1=512*512	*/
int a,i;
unsigned char sym[2] = "0";
unsigned char hex[17] = "0123456789ABCDEF";

	MMem = _dos_malloc( 0x10000);
	if( MMem > (void *)0x81000000 ) {		/* ６４Ｋのメインメモリを確保			*/
 		printf("メモリが確保できません\n");
		return;
	}

	VRAM = vdp_init( fontfile);			/* ＶＲＡＭ用のメモリのの確保など		*/

	ssp = _iocs_b_super( 0);

	login( MMem, VRAM);					/* システムの初期化				*/

	display_set();

	screen( 0);							/* スクリーンモードを０に		*/

	width( 80);

	command_ms();						/* command.ms の実行			*/

}
