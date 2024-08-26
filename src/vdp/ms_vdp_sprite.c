#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

/*
 	スプライトの処理

	MS.Xは、MSXの256ドットをX68000の512ドットに拡大している
	そのため、MSXの8x8ドットのスプライトは、X68000上は16x16ドットになる
	MSXのスプライトパターンは最大256個定義することができるが、
	X68000は16x16ドットのスプライトパターンを最大128個しか定義できず、数が足りない。
	そこで、MSXのスプライトは最大32個しか表示できないことを利用し、
	X68000のスプライトパターンは、現在表示中のスプライトのみを定義することにする。
	 
 */

/*
 スプライトの初期化
 */
void init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCGバッファの初期化
	for ( i = 0; i < 256 * 32; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern);
void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern);

/*
 スプライトパターンジェネレータテーブルへの書き込み
     offset: パターンジェネレータテーブルのベースアドレスからのオフセットバイト
     pattern: 書き込むパターン(下位8bitのみ使用)
*/
void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_pattern_512(vdp, offset, pattern);
	} else {
		write_sprite_pattern_256(vdp, offset, pattern);
	}
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine; // MSXの1ラインはX68000でも1ライン
	uint32_t pcg_pattern=0; // x68000の16x16のパターンに変換したもの

    // 右端のドットから処理
	for(i =0; i < 8; i++) {
		pcg_pattern >>= 4;
		if(pattern & 1) {
			pcg_pattern |= (0xf0000000);
		}
		pattern >>= 1;
	}
	// パターンジェネレータテーブルのバッファに書き込む
	vdp->x68_pcg_buffer[ptNum * 8 + pcgLine] = pcg_pattern;
}

void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine * 2; // MSXの1ラインはX68000では2ライン
	unsigned int pLeft=0,pRight=0; // 1ラインの左4ドットと右4ドットを X68000の8x8のパターン2つに変換

    // 右端のドットから処理
	for(i =0; i < 4; i++) {
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
	// パターンジェネレータテーブルのバッファに書き込み
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute);
void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute);
void refresh_sprite_256(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode2(ms_vdp_t* vdp);

void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_attribute_512(vdp, offset, attribute);
	} else {
		write_sprite_attribute_256(vdp, offset, attribute);
	}
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y座標
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				update_sprite_visibility(vdp);
			}
			uint8_t scroll_offset = vdp->r23; // 縦スクロール量
			// MSXは1ライン下に表示されるので+1
			// MSXはR23の縦スクロールでスプライトも一緒にスクロールするので、その分を引く
			// 256モードはMSXのY座標の1倍
			// X68000のスプライトの原点は(16,16)なのでずらす
			int y = ((attribute + 1 - scroll_offset) & 0xff) + 16;
			X68_SSR[plNum*4+1] = y;
			break;
		case 1: // X座標
			X68_SSR[plNum*4+0] = (attribute & 0xff) + 16; // MSXのX座標の1倍, X68000のスプライトの原点は(16,16)なのでずらす
			// TODO ECビットによる位置補正処理
			break;
		case 2: // パターン番号
		case 3: // 属性
			// パターン番号、カラーが変更されたら、事前にバッファに展開しておいたパターンを転送し、書き換える
			refresh_sprite_256(vdp, plNum);
			break;
		default:
			break;
	}

	// パターン指定 & パレット指定
	X68_SSR[plNum*4+2] = 0x100 + plNum; // パレット0x10-0x1fを使用するので 0x100を足す
}

void refresh_sprite_256(ms_vdp_t* vdp, int plNum) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1) {
		refresh_sprite_256_mode1(vdp, plNum);
	} else {
		refresh_sprite_256_mode2(vdp);
	}
}

void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	unsigned int ptNum = p[plNum*4+2];
	unsigned int color = p[(plNum*4 & 0x1fffc)+3] & 0xf;
	unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
	if (vdp->sprite_size == 0) {
		// 8x8
		for( i = 0; i < 8; i++) { 
			X68_PCG[plNum*32+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*8+i] & colorex;
		}
	} else {
		// 16x16
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*32+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*8+i] & colorex;
		}
	}
}

/*
　スプライトモード2の色合成処理の実装
　モード2の色合成はドット単位で行われるため、真面目に実装すると非常に複雑になり、負荷が高いので、
　簡略化して対応する（ダメなケースが多いようならな考え直す）
　通常のゲームなどで色合成を使う場合、以下のような使い方をしているはず。
	* X,Y座標は完全に一致させる
	* 連続するスプライトプレーンを使用する
		* モード2の使用上は「同一ライン上に並んでいる物の中で合成」なので必ずしも連続していなくても良い
		* が、スプライトが横切る際などに破綻するので、通常は連続させるはず
	* 最大4枚の合成まで
		* 4ビットあれば16色全てが表現できるので5枚以上重ねるケースはないと想定
　以上を前提として、以下のように合成する。
	* n=0から開始
	* プレーンn番のスプライトのXY座標を取得
	* プレーンn+1番から31のスプライトのうち、XY座標が同一のものかつ、CC=1のラインが一つでもあるものを抽出(連続している物のみ)
		* 連続した最後の番号をmとする
	* y=0から以下を繰り返す
	* プレーンn番のyライン目のパターンに色コードを掛ける
	* プラーンn+1からmまで以下を繰り返す
		* yライン目の色データを取得し、それぞれの色コードを掛ける
		* CC=1の場合は、その色を直前のCC=0のプレーンと合成する
		* CC=0の場合は、それ自身ののプレーン(本来のプレーン)に色を描画する
	OR合成する
	* y=y+1して繰り返す
	* 全てのライン(8x8モードの場合は8ライン、16x16モードの場合は16ライン)に対して繰り返す
	* nを m+1に更新し、nが31を超えるまで繰り返す
　このようにすると、以下のような例外ケースは拾えないが、ある程度再現できるはず。
	* プレーン0番のCCビットが1だった場合、本来は描画しないはず（より若い番号がないので）だが、表示されてしまう
	* XYが微妙にずれて重なっているケースは全て合成されない
　なお、CCビットの判定を高速化するため、以下の仕組みも用意する。
	* 各プレーンに 16bitのビットマップフラグを用意する
	* スプライトカラーテーブルが更新されたら、CC=1の場合はライン番号に対応するビットを1にし、CC=0の場合は0にする
　このようにしておくと、あるプレーンのフラグが0でなければ、いずれかのラインでCC=1になっていることがわかる
 */

uint16_t sprite_cc_flags[32]; // スプライトカラーテーブルのCCビットのフラグ

void refresh_sprite_256_mode2(ms_vdp_t* vdp) {
	int plNum,n,y,i,j;
	uint8_t* pcol = vdp->vram + vdp->sprcolrtbl_baddr;
	uint8_t* patr = vdp->vram + vdp->sprattrtbl_baddr;
	// スプライトモード2の色合成を行う
	for (plNum=0;plNum<32;plNum++) {
		int m = plNum;
		for(n=plNum+1;n<32;n++) {
			// XY座標が同一のものかつ、CC=1のラインが一つでもあるものを抽出(連続している物のみ)
			if((patr[plNum*4+0] == patr[n*4+0]) && (patr[plNum*4+1] == patr[n*4+1]) && //
				(sprite_cc_flags[n] != 0)) {
				m = n;
			} else {
				break;
			}
		}

		// ************
		//m = plNum; // テスト用
		// ************

		// ラインごとの合成処理
		int ymax = vdp->sprite_size == 0 ? 8 : 16;
		int lrmax = vdp->sprite_size == 0 ? 1 : 2;
		int ptNumMask = vdp->sprite_size == 0 ? 0xff : 0xfc;
		int lr;
		for (lr=0;lr < lrmax; lr++) {
			uint16_t mask = 1;
			for	(y=0; y<ymax; y++, mask <<= 1) {
				int yy = y+lr*16;
				i=plNum;
				while(i<=m) {
					uint32_t color = pcol[i*16+y] & 0xf;
					uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
					uint32_t ptNum = patr[i*4+2];
					uint32_t pattern = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*8+yy] & colorex;
					j=i;
					while(j<=m) {
						if( j == m ) {
							X68_PCG[i*32+yy] = pattern;
							i=j+1;
							break;
						}
						j++; // j==mを先に判定しているので、i+1がmをオーバーすることはない
						if( (sprite_cc_flags[j]&mask) == 0) {
							// CC=0に遭遇したら、それ以降は合成しない
							X68_PCG[i*32+yy] = pattern;
							i=j;
							break;
						}
						// CC=1のものが見つかったので合成する
						uint32_t color_add = pcol[j*16+y] & 0xf;
						uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
						uint32_t ptNum_add = patr[j*4+2];
						uint32_t pattern_add = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*8+yy] & colorex_add;
						pattern |= pattern_add;
					}
				}
			}
		}
	}
}


void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSXのスプライトプレーン番号
	int type = offset % 4; // 属性の種類

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y座標
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				update_sprite_visibility(vdp);
			}
			uint8_t scroll_offset = vdp->r23; // 縦スクロール量
			// MSXは1ライン下に表示されるので+1
			// MSXはR23の縦スクロールでスプライトも一緒にスクロールするので、その分を引く
			// 512モードはMSXのY座標の2倍
			// X68000のスプライトの原点は(16,16)なのでずらす
			int y = ((attribute + 1 - scroll_offset) & 0xff)*2 + 16;
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = y + (i%2)*16;
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
			if (vdp->sprite_size == 0) { // 8x8
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32*4+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*32+i] & colorex;
				}
			} else { // 16x16
				for( i = 0; i < 32*4; i++) { 
					X68_PCG[plNum*32*4+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*32+i] & colorex;
				}
			}
			break;
		default:
			break;
	}
	if (vdp->sprite_size == 0) {
		// 8x8モード
		X68_SSR[plNum*4+2] = 0x100 + plNum; // パレット0x10-0x1fを使用するので 0x100を足す
	} else {
		// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
		for( i=0; i<4; i++) {
			X68_SSR[plNum*16+i*4+2] = 0x100 + plNum*4+i; // パレット0x10-0x1fを使用するので 0x100を足す
		}
	}

}

/*
 スプライトカラーテーブルへの書き込み
     offset: カラーテーブルのベースアドレスからのオフセットバイト
     pattern: 書き込む値(下位8bitのみ使用)
*/
void write_sprite_color(ms_vdp_t* vdp, int offset, uint32_t color) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) != 2) {
		//　スプライトモード2以外は何もしない
		return;
	} else {
		// TODO 毎回反映するのは大変なので、VRAMにだけ書いておき、フラグを立て、垂直基線機関にまとめた処理する
		
		// CCビットのビットマップフラグを更新
		int plNum = offset / 16; // MSXのスプライトプレーン番号
		int line = offset & 0x0f; // 今回更新するライン番号
		if (color & 0x40) { // CCビットを検査
			sprite_cc_flags[plNum] |= (1 << line);
		} else {
			sprite_cc_flags[plNum] &= ~(1 << line);
		}
	}
}

/*
 スプライトの表示/非表示を更新する
 関連する仕様は以下の2つ
  * R#8のbit1が1のときはスプライトを非表示にする
  * あるスプライトプレーンのY座標を208にすると、そのスプライトプレーン以降は全て非表示になる
 */
void update_sprite_visibility(ms_vdp_t* vdp) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;

	int	visible_sprite_planes; // 画面に表示するスプライトプレーンの数。0にすると非表示になる
	if (vdp->r08 & 0x01) {
		visible_sprite_planes = 0;
	} else {
		visible_sprite_planes = 32;
		for (i = 0; i < 32; i++) {
			if (p[i*4+0] == 208) {
				visible_sprite_planes = i;
				break;
			}
		}
	}
	if ((vdp->last_visible_sprite_planes == visible_sprite_planes) && //
		(vdp->last_visible_sprite_size == vdp->sprite_size)) {
		return;
	}

	vdp->last_visible_sprite_planes = visible_sprite_planes;
	vdp->last_visible_sprite_size = vdp->sprite_size;
	for (i = 0; i < visible_sprite_planes; i++) {
		if (vdp->sprite_size == 0) {
			// 8x8モード
			X68_SSR[i*4+3] = 3; // スプライト表示
		} else {
			// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
			for( j=0; j<4; j++) {
				X68_SSR[i*16+j*4+3] = 3; // スプライト表示
			}
		}
	}
	for (; i < 32; i++) {
		if (vdp->sprite_size == 0) {
			// 8x8モード
			X68_SSR[i*4+3] = 0; // スプライト非表示
		} else {
			// 16x16モードは X68000上で 32x32になるので、16x16のスプライトを4つ並べて表現する
			for( j=0; j<4; j++) {
				X68_SSR[i*16+j*4+3] = 0; // スプライト非表示
			}
		}
	}
}

void rewrite_all_sprite(ms_vdp_t* vdp) {
	if(0) {
		printf(".");
		fflush(stdout); 
	}
	int i,j;
	uint32_t sprpgenaddr = vdp->sprpgentbl_baddr & 0x1fe00; // 下位9ビットをクリア
	for(i=0;i<256;i++) {
		for(j=0;j<8;j++) {
			write_sprite_pattern(vdp, i*8+j, vdp->vram[sprpgenaddr + i*8 + j]);
		}
	}
	uint32_t sprattraddr = vdp->sprattrtbl_baddr; // & 0x1f80; // 下位7ビットをクリア
	for(i=0;i<32;i++) {
		write_sprite_attribute(vdp, i*4+0, vdp->vram[sprattraddr + i*4 + 0]);
		write_sprite_attribute(vdp, i*4+1, vdp->vram[sprattraddr + i*4 + 1]);
		//write_sprite_attribute(vdp, i*4+3, vdp->vram[sprattraddr + i*4 + 3]);
	}
	refresh_sprite_256_mode2(vdp);
}