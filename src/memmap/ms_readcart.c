/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[ カートリッジ読み込みルーチン ]]
											programed by Kuni.
														1995.12.06
*/

#include <stdio.h>
#include <string.h>
//#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <iocslib.h>
#include <doslib.h>
#include "ms_memmap.h"

void read_cartridge( char *, void *, int, int, int);
void ms_memmap_set_cartridge( void *, int, int, int);

/*	カートリッジファイルを検索するルーチン			*/

void mount_cartridge( ) {
//DIR directory;
//DIR *p_directory;

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

int filelength(int fh) {
	struct stat st;
	if( fstat(fh, &st) == -1) {
		return -1;
	}
	return st.st_size;
}

/*
 *	サーチされたファイルを実際に読み込むルーチン
 *
 *	path_crt	カートリッジファイルのパス名
 *	ccf_buff	読み込んだコンフィグファイルのアドレス。０の時コンフィグファイルはない。
 *	location	読み込むスロット位置。基本スロット×４＋拡張スロット
 *	page		読み込むページ。ccfファイルがある場合はそちらの指定を優先する。
 *	kind		カートリッジの種類。ccfファイルがあればそちらが優先	
 */
void read_cartridge(char *path_crt, void *ccf_buff, int location, int page, int kind){
	int crt_fh;									/* カートリッジファイルの fh が入る	*/
	int ccf_fh;									/* コンフィグファイルの fh が入る	*/
	int crt_length;
	void *crt_buff;

	crt_fh = open( path_crt, O_RDONLY | O_BINARY);
	if( ccf_fh == -1) {
		printf("ファイルが見つかりません。\n");
		return;
	}
	if( ccf_buff == 0) {
		crt_length = filelength(crt_fh);
		if(crt_length == -1) {
			printf("ファイルの長さが取得できません。\n");
			return;
		}
		if( ( crt_buff = (void*)_dos_malloc( crt_length + MS_MEMMAP_HEADER_LENGTH ) ) == NULL) {
			printf("メモリが確保できません。\n");
			return;
		}
		read( crt_fh, crt_buff + MS_MEMMAP_HEADER_LENGTH, crt_length);
		ms_memmap_set_cartridge( crt_buff, location, page, kind);	/* アセンブラのルーチンへ	*/

		close( crt_fh);

		return;
	}

}
