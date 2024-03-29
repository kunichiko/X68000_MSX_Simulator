/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[ カートリッジ読み込みルーチン ]]
											programed by Kuni.
														1995.12.06
*/

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/iocs.h>
#include <sys/dos.h>

#define h_length 8

void read_cartridge( char *, void *, int, int, int);

/*	カートリッジファイルを検索するルーチン			*/

void mount_cartridge( ) {
DIR directory;
DIR *p_directory;

/*	p_directory = opendir();	*/

/*	if( (ccf_buff = _dos_malloc( ccf_length=filelength( ccf_fh))) >(void *)0x81000000 ) { */
/*		printf("メモリが確保できません。\n");	*/
/*		return;	*/
/*	}	*/

	
}

void mount_cartridge_zantei() {
	read_cartridge( "MAINROM1.ROM", 0, 0x00, 0, 2);
	read_cartridge( "MAINROM2.ROM", 0, 0x00, 1, 2);
	read_cartridge( "SUBROM.ROM", 0, 0x0d, 0, 2);
}

/*																		*/
/*				サーチされたファイルを実際に読み込むルーチン			*/
/*																		*/
/*	path_crt	カートリッジファイルのパス名							*/
/*	ccf_buff	読み込んだコンフィグファイルのアドレス。０の時コンフィ	*/
/*				グファイルはない。										*/
/*	location	読み込むスロット位置。基本スロット×４＋拡張スロット	*/
/*	page		読み込むページ。ccfファイルがある場合はそちらの指定を	*/
/*				優先する。												*/
/*	kind		カートリッジの種類。ccfファイルがあればそちらが優先		*/
/*																		*/
void read_cartridge(char *path_crt, void *ccf_buff, int location, int page, int kind){
int crt_fh;									/* カートリッジファイルの fh が入る	*/
int ccf_fh;									/* コンフィグファイルの fh が入る	*/
int crt_length;
void *crt_buff;

	crt_fh = open( path_crt, O_RDONLY | O_BINARY);

	if( ccf_buff == 0) {
		crt_length = filelength(crt_fh);
		if( ( crt_buff = _dos_malloc( crt_length + h_length ) ) > (void *)0x81000000) {
			printf("メモリが確保できません。\n");
			return;
		}
		read( crt_fh, crt_buff + h_length, crt_length);
		set_cartridge( crt_buff, location, page, kind);	/* アセンブラのルーチンへ	*/

		close( crt_fh);

		return;
	}

}
