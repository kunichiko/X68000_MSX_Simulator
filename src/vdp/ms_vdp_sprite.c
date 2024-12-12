#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

/*
 	スプライトの処理

	MSXのスプライトには、
	* 8x8ドット
	* 16x16ドット
	* 8x8ドット(拡大)
	* 16x16ドット(拡大)
	の4つのサイズがあります。

	MSXには256ドットモードと512ドットモードがありますが、スプライトの見た目のサイズは変わらず、
	常に 256ドットモードで表示されます。

	一方、MS.Xは、再現するスクリーンモードによって 256ドットモードと 512ドットモードの
	2つのグラフィック画面を使い分けていますが、X68000のスプライトのドットサイズは
	グラフィック画面と同じであるため、この組み合わせをうまく調整する必要があります。

	組み合わせを列挙すると、以下のようになります。

	* MSX 8x8ドットスプライト & X68000 256ドットモード
		* MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16の1/4を使用)に対応
	* MSX 8x8ドットスプライト & X68000 512ドットモード
		* MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16)に対応
	* MSX 16x16ドットスプライト & X68000 256ドットモード
		* MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16)に対応
	* MSX 16x16ドットスプライト & X68000 512ドットモード
		* MSXの1枚のスプライトを X68000の 4枚 のスプライト(32x32)に対応
	* MSX 8x8ドット(拡大)スプライト & X68000 256ドットモード
		* MSXの1枚のスプライトを X68000の 1枚 のスプライト(16x16)に対応
	* MSX 8x8ドット(拡大)スプライト & X68000 512ドットモード
		* MSXの1枚のスプライトを X68000の 4枚 のスプライト(32x32)に対応
	* MSX 16x16ドット(拡大)スプライト & X68000 256ドットモード
		* MSXの1枚のスプライトを X68000の 4枚 のスプライト(32x32)に対応
	* MSX 16x16ドット(拡大)スプライト & X68000 512ドットモード
		* (対応しない)

	このようにしてみると、最後のパターンをサポートしなければ、MSXの
	スプライトプレーン1つは最大でX68000の4つのスプライトに対応することになります。
	MSXはどのサイズでも32枚のスプライトプレーンを表示でき、X68000は16x16のスプライトを
	128枚表示できるので、ちょうどぴったり収まっています。
	逆にいうと「16x16ドット(拡大)スプライト & X68000 512ドットモード」は、
	X68000のスプライト数が足りないため、対応できません。スプライトダブラを使えば
	不可能ではないかもしれませんが、需要があれば……という感じでしばらくはサポートしません。

	なお、このように画面モードとスプライトのモードによって対応が少しずつずれていき、
	こんがらがりやすいため、以下のように用語を定義します。

	* シングルモード / マルチモード
		* 1スプライトプレーンが1つのスプライトになるケースをシングルモードと呼ぶ
		* 1スプライトプレーンが複数のスプライトになるケースをマルチモードと呼ぶ
	* D1Xモード / D2Xモード / D4Xモード
		* MSXのスプライトパターン1ビットが、X68000の1ドットになる場合を D1Xモードと呼ぶ
		* MSXのスプライトパターン1ビットが、X68000の2ドットになる場合を D2Xモードと呼ぶ
			* 拡大機能で2ドットになる場合もこれに含む
		* MSXのスプライトパターン1ビットが、X68000の4ドットになる場合を D4Xモードと呼ぶ
			* 512ドットモードで拡大スプライトを使う場合4ドット必要になる



	MS.Xでは、どの画面モードにおいても、MSXのスプライトプレーン番号を
	以下のようにマッピングすることにしています。

	MSXのスプライトプレーン  0番 -> X68000のスプライト 0-3番
	MSXのスプライトプレーン  1番 -> X68000のスプライト 4-7番
		:
	MSXのスプライトプレーン 31番 -> X68000のスプライト 124-127番

	このように、1つのスプライトで間に合う場合(シングルモード)でも番号を詰めずに、
	4つ飛ばしで使用します。こうしておくと、画面モードを行き来した時でも対応関係が
	一致するので、管理が楽になるのではと思っています。

	上記はスプライトプレーンの話ですが、スプライトパターン定義のほうは、MSXのスプライトの
	最大4倍の定義をPCG上に展開することができないため、メモリ上にPCGパターンをあらかじめ
	展開しておき(PCGバッファ)、スプライトプレーンの表示時に転送するようにしています。

	この時、PCGバッファには、MSXのスプライト定義(8x8単位)の順番通りに詰めて格納していきます。
	ただ、順番通りと言っても、D2XモードやD4Xモードの場合は、MSXの1スプライト定義が、
	複数枚に膨れるケースがあるので、混乱しないようにしてください。

	●スプライトパターン定義(PCG)について

	画面が256ドットモード、スプライトが16ドットモード、拡大なしの場合とそれ以外の場合で
	スプライトパターン定義の扱いを変えています。

	◯ 「画面幅256、スプライト16x16、拡大なし」の場合
	この場合、MSXのスプライト定義は 16x16ドットの定義が64個作れます。X68000のPCGは
	16x16ドットのスプライトを128個定義できる(BGを使わなければもっとだが)ので、
	MSXのスプライト定義をそのままPCGの定義にマッピングできます。
	512ドットモードや拡大モードになると4倍に膨れてしまい、収まらなくなります。また、
	MSXが8x8ドットモードの場合は定義数が256個になりますが、X68000は16x16のセットで
	しか定義できないので、やはり定義数が4倍に膨れてしまい、収まらなくなります。
	以前はどのモードでも同じやり方（後述する「それ以外の場合」のやりかた）でじっそうして
	いましたが、「画面幅256、スプライト16x16、拡大なし」はゲームで最もよく使われる
	モードなので、このケースだけ特別扱いして高速化することにしました。

	具体的には、MSX側の64個のスプライト定義に色を掛け合わせたもの(色合成後のパターン)を
	X68000のPCG上に最大32個展開します。それぞれのパターンが、MSXのどのパターン番号に
	どの色を掛け合わせたものかを別途記録しておきます。このようにすることで、スプライト
	表示時にすでに色合成済みのものがあればそれをそのまま使うことができるため、高速化
	できます。
	スプライトモード2だと複数枚のパターンを組み合わせた色合成が可能なので、管理テーブル
	は以下のような32bitの情報を32個持つようにします。
    
	31-30 29-20　19-10  9-0
	[ N ][3枚目][2枚目][1枚目]
	  N: 合成したパターンの数.0の時はこのバッファは未使用であることを示します
	  1-3枚目: 10bitのデータで、合成したパターンの番号と色の組み合わせを表します
	     9-6    5-0
        [Col][Pattern]
	
	たとえばMode1の場合は、一枚の合成しかあり得ないので、以下のようになります。

	- 0b01_0000_000000_0000_000000_1111_000000
		パターン番号0番に色コード0xfを掛け合わせたもの

	Mode2の場合は、最大3枚までの色合成を管理できるため、以下のようになります。

	- 0b11_0100_000010_0010_000001_0001_000000
		パターン番号0番に色コード0x1を掛け合わせたもの
		パターン番号1番に色コード0x2を掛け合わせたもの
		パターン番号2番に色コード0x4を掛け合わせたもの
		以上3つを合成したもの

	Mode2はラインごとに色を変更できるのですが、代表色として8ライン目の色を使うことに
	します。たまたま8ライン目の色が同じで他のラインだけ色が異なるケースに対応できない
	こともあり得ますが、諦めます。

	組み合わせのバリエーションは非常に多いですが、そもそもMSXはスプライトプレーンが
	32個しかないので、同時に存在できる組み合わせは高々32通りになります。
	PCGは127個定義できるので多めに持つことも可能ですが、すでに色合成済みのものがあるか
	どうかを調べる際にリニアサーチを行うため、あまり多く持ちすぎるとかえって遅くなる
	可能性があり、32個程度が適切だと思われます。

	以上を踏まえ、垂直帰線期間が終わったあたりのタイミングで、以下の処理を実施します。
	0 : パターンが書き換えられたものにdirtyフラグを立てておきます
	1 : パターン管理テーブルをスキャンし、dirtyなパターンを使っていたものがあれば、
		0クリアします
	2: 	スプライトアトリビュートテーブルを0から順にスキャンします
		2-1: スプライトが非表示であればスキップします
		2-2: 以降のプレーンが非表示の場合はそこで停止し、残りを非表示にします
		2-3: 連続するプレーンを検索し、CC=1のものを最大2つまで見つけます
			3つ以上あった場合は無視します
		2-4: CC=1の組み合わせに応じ、32bitの管理フラグを計算します
		2-5: パターン管理テーブルを先頭からスキャンし、同じ値を持っているものが
			あれば、そのパターンを使用してスプライトを表示します。また、パターンを
			使用したことを32bitの使用済みフラグで覚えておきます
		2-6: パターンが存在しない場合は、その場で色合成し、結果をメモリ上に展開して、
			後の処理に備えます
		2-7: 合成済みパターンを使って表示できなかったものをPCG上に展開します
			このとき、使用済みフラグをみながら、使われなかったパターンを頭から
			上書きしていきます

	このようにすることで、すでに合成済みのパターンを再利用しながら、効率よくスプライトを
	表示することができます。


	◯ それ以外の場合
	X68000のスプライト番号 N番に 実PCGのN版を対応づけるようにします。この対応
	づけは全ての画面モードで変わらないため、4個飛ばしで使われることがあります。

	すると、MSXの8x8モードの時は、PCGバッファ上には連続してパターンが格納されていますが、
	X68000の実PCGに展開する際は、16個飛ばしで展開することになるなど、対応が一対一に
	ならないケースがあるので注意してください。
	(PCGバッファはあくまでPCGを作るときの素材が詰まっていると思ってください)
 */

/*
 	制限事項

  # 「色コード0」のスプライト
  MSXはパターンの定義と色の定義が別れているため、「パターンは1だが、色コードが0」の
  スプライトというものが描画可能です。一方、X68000はパターンと色が一体となっているため、
  色コード0は透明色として扱われます。そのため、MS.Xでは「パターンは1だが、色コードが0」
  のスプライトは強制的に色コード1に変換されます。
  そのため、色が変わって見えてしまうことがあります。

  # スプライトモード2の色合成

  MSXのスプライトモード2は、スプライトの色合成が行われるモードですが、MS.Xでは
  特定の条件下でしか正しく表示されません。具体的には、以下の条件を満たす場合のみ
  正しく表示されます。

  * 重なったスプライトのXY座標がぴったり一致していること
  * 合性を行うスプライトプレーン番号が連続していること

 
 */

#define COL_SIZE			16				// MSXのスプライトカラーテーブルのサイズ(バイト数)
#define SAT_SIZE			4				// MSXのスプライトアトリビュートテーブルのサイズ(バイト数)
#define PCG_SIZE			32				// X68kのスプライト1つ(16x16)あたりの、PCGパターンのワード数
#define SSR_SIZE			4				// X68kのスプライト1つあたりの、SSRレジスタのワード数
#define PCG_UNIT			(PCG_SIZE * 4)	// MSXのプレーン1つあたりの、PCGのワード長
#define SSR_UNIT			(SSR_SIZE * 4)	// MSXのプレーン1つあたりの、SSRのワード長
#define PCG_BUF_UNIT_D1X	8				// D1Xモードで、MSXの1定義(8x8)がPCGバッファ上で占めるワード数
#define PCG_BUF_UNIT_D2X	32				// D2Xモードで、MSXの1定義(8x8)がPCGバッファ上で占めるワード数
#define PCG_BUF_UNIT_D4X	128				// D4Xモードで、MSXの1定義(8x8)がPCGバッファ上で占めるワード数


// 仮
uint16_t sprite_cc_flags[32]; // スプライトカラーテーブルのCCビットのフラグ

uint8_t sp_pattern_dirty[64];	// 16x16ドットのパターンは64個ある


#define NUM_SP_PAT_BUF 64

uint32_t sp_composition_table[NUM_SP_PAT_BUF];

int sp_new_pattern_count; 
struct sp_pat_buf {
	int32_t plane_num;
	uint32_t composition_flag;
	uint32_t pattern[PCG_SIZE];
} sp_pat_buf[NUM_SP_PAT_BUF];

int lastused_sp_pat_buf = 0;

/*
 スプライトの初期化
 */
void init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCGバッファの初期化
	for ( i = 0; i < 256 * PCG_BUF_UNIT_D4X; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;

	// 初期化処理
	vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_FULL;
	vdp->sprite_composition_flag = 0;

	for (i = 0; i < 64; i++) {
		sp_pattern_dirty[i] = 0;
	}
	for (i = 0; i < 32; i++) {
		sp_composition_table[i] = 0;
	}
	for (i=0; i < 32; i++) {
		sp_pat_buf[i].composition_flag = 0;
	}
	
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern);
void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern);

/*
 スプライトパターンジェネレータテーブルへの書き込み
	 offset: パターンジェネレータテーブルのベースアドレスからのオフセットバイト
	 pattern: 書き込むパターン(下位8bitのみ使用)
*/
void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern, int32_t old_pattern) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_pattern_512(vdp, offset, pattern);
	} else {
		write_sprite_pattern_256(vdp, offset, pattern);
	}
	// パターンジェネレータテーブルが変更されたら、アトリビュートテーブルの更新をかける
	vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
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
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D1X + pcgLine] = pcg_pattern;

	// パターンが変更されたことを記録
	sp_pattern_dirty[ptNum / 4] = 1;	// 16x16ドット単位で管理
}

void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSXのスプライトパターン番号
	int pLine = offset % 8; // パターンの何行目か 
	int pcgLine = pLine * 2; // MSXの1ラインはX68000では2ライン
	uint32_t pLeft=0,pRight=0; // 1ラインの左4ドットと右4ドットを X68000の8x8のパターン2つに変換

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
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+1 + 16] = pRight;
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute);
void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute);
void refresh_sprite_512(ms_vdp_t* vdp, int plNum);
void refresh_sprite_512_mode1(ms_vdp_t* vdp, int plNum);
void refresh_sprite_512_mode2(ms_vdp_t* vdp);
void refresh_sprite_256(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode2(ms_vdp_t* vdp);

inline int get_sprite_adjustx(ms_vdp_t* vdp) {
	return (vdp->r18 & 0x8) ? 8-(vdp->r18 & 0x7) : -(vdp->r18 & 0x7);
}

void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_attribute_512(vdp, offset, attribute, old_attribute);
	} else {
		write_sprite_attribute_256(vdp, offset, attribute, old_attribute);
	}
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute) {
	int i,j;
	int plNum = (((uint32_t)offset) / SAT_SIZE); // MSXのスプライトプレーン番号
	int type = offset % SAT_SIZE; // 属性の種類
	int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;

	if (plNum >= 32) {
		return;
	}

	switch(type) {
		case 0: // Y座標
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
				return;
			// Y=208/216の対応のため、Y座標の208/216の値が変化したら、スプライトの再配置を行う
			int HY = (vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1 ? 208 : 216;
			if ( attribute == HY || old_attribute == HY) {
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
			}
			uint8_t scroll_offset = vdp->r23; // 縦スクロール量
			// MSXは1ライン下に表示されるので+1
			// MSXはR23の縦スクロールでスプライトも一緒にスクロールするので、その分を引く
			// 256モードはMSXのY座標の1倍
			// X68000のスプライトの原点は(16,16)なのでずらす
			int y = ((attribute + 1 - scroll_offset) & 0xff) + 16;
			X68_SSR[plNum*SSR_UNIT+1] = y;
			break;
		case 1: // X座標
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
				return;
		 	// MSXのX座標の1倍, X68000のスプライトの原点は(16,16)なのでずらす
			// SET ADJUSTのX方向の補正を行う
			int ec = 0;
			switch(spMode) {
			case 1:
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				break;
			case 2:
				uint8_t* pcolr = vdp->vram + vdp->sprcolrtbl_baddr;
				ec = (pcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ラインごとのECはサポートしないので1ライン目だけみる
				break;
			}
			int x = attribute & 0xff;
			int adjustx = get_sprite_adjustx(vdp);
			x = ((x - ec*32) + 16 + adjustx) & 0x3ff; // MSXのX座標の1倍
			X68_SSR[plNum*SSR_UNIT+0] = x;
			break;
		case 2: // パターン番号
		case 3: // 属性
			// パターン番号、カラーが変更されたら、事前にバッファに展開しておいたパターンを転送し、書き換える
			if(spMode == 1) {
				refresh_sprite_256_mode1(vdp, plNum);
				// ECをX座標に反映
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				int x = pattr[plNum*SAT_SIZE+1];
				int ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				int adjustx = get_sprite_adjustx(vdp);
				x = ((x - ec*32) + 16 + adjustx) & 0x3ff; // MSXのX座標の1倍
				X68_SSR[plNum*SSR_UNIT+0] = x;
			} else {
				// モード2は一つ書き換えると全体に影響が出るので、垂直帰線期間にまとめて書き換える
				// TODO: それなりに重いので、もう少し範囲を限定したい
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
			}
			break;
		default:
			break;
	}
}

uint32_t colorex_tbl[16] = {
	0x00000000, 0x11111111, 0x22222222, 0x33333333,
	0x44444444, 0x55555555, 0x66666666, 0x77777777,
	0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb,
	0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff
};

void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	uint32_t* x68_pcg_buffer = vdp->x68_pcg_buffer;
	uint32_t ptNum = p[plNum*SAT_SIZE+2];
	ptNum &= (vdp->sprite_size == 0) ? 0xff : 0xfc;
	uint32_t color = p[plNum*SAT_SIZE+3] & 0xf;
	//color = color == 0 ? vdp->alt_color_zero : color; // 「色コード0」のスプライトが消えてしまう問題への暫定対応
	// Spmode 1はcoloe 0は透明色
	// if (color == 0) {
	// 	// 色コード0のスプライトは表示しない
	// 	for( i=0; i<4; i++) {
	// 		X68_SSR[plNum*SSR_UNIT+i*4+3] = 0;	// スプライトは非表示
	// 	}
	// 	return;
	// } else {
	// 	for( i=0; i<4; i++) {
	// 		X68_SSR[plNum*SSR_UNIT+i*4+3] = 3; // スプライト表示
	// 	}
	// }
	//uint32_t colorex ;= color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
	uint32_t colorex = colorex_tbl[color];
	if (vdp->sprite_size == 0) {
		// 8x8
		for( i = 0; i < 8; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = x68_pcg_buffer[ptNum*PCG_BUF_UNIT_D1X+i] & colorex;
		}
		for(;i < 32; i++) {
			X68_PCG[plNum*PCG_UNIT+i] = 0;
		}
	} else {
		// 16x16
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = x68_pcg_buffer[ptNum*PCG_BUF_UNIT_D1X+i] & colorex;
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
		* モード2の使用上は「同一ライン上に並んでいる物の中で合成」なので必ずしも連続していなくても良いが、
		* スプライトが横切る際などに破綻するので、通常は連続させるはず
	* 最大3枚の合成まで
		* 4ビットあれば16色全てが表現できるので5枚以上重ねるケースはないと想定
		* 実際には3枚もあれば8色が使えるので、管理の都合上もあり、3枚までとする
　以上を前提として、以下のように合成する。
	* n=0から開始
	* プレーンn番のスプライトのXY座標を取得
	* プレーンn+1番から31のスプライトのうち、XY座標が同一のものかつ、CC=1のラインが一つでもあるものを抽出(連続している物のみ)
		* 連続した最後の番号をmとする
		* 合成がない場合は m == n となる
	* ラインごとに合成するためy=0から以下を繰り返す
	* プレーンn番のyライン目のパターンに色コードを掛ける
	* プレーンn+1からmまで以下を繰り返し、すべてをorで合成する
		* yライン目の色データを取得し、それぞれの色コードを掛ける
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

int find_free_pcg(uint8_t* sp_pattern_used) {
	// 今回新たに合成したものを、空いているPCGに書き込む
	int i,j;
	int new=0;
/*	for(i=0;i<NUM_SP_PAT_BUF;i++) {
		if( sp_new_pattern_count <= new) {
			break;
		}
		if( sp_composition_table[i] != 0) {
			continue;
		}
		return i;
		// 空いているところが見つかったのでパターンを転送する
		for(j=0;j<32;j++) {
			X68_PCG[i*PCG_SIZE+j] = sp_pat_buf[new].pattern[j];
		}
		// プレーンにパターンを割り当てる
		X68_SSR[sp_pat_buf[new].plane_num * SSR_UNIT + 2] = 0x100 + i; // パレット0x10-0x1fを使用するので 0x100を足す
		//
		sp_composition_table[i] = sp_pat_buf[new].composition_flag;
		new++;
	}*/
	//空いているところがなかったら、今回使わなかったパターンをクリア
	for(i=(lastused_sp_pat_buf+1) % NUM_SP_PAT_BUF ;;i= (i+1) % NUM_SP_PAT_BUF) {
		if( sp_pattern_used[i] ) {
			continue;
		}
		return i;
	}
}

void refresh_sprite_256_mode2(ms_vdp_t* vdp) {
	int plNum,n,y,i,j;
	uint8_t* pcol = vdp->vram + vdp->sprcolrtbl_baddr;
	uint8_t* patr = vdp->vram + vdp->sprattrtbl_baddr;

	// フラグクリア
	vdp->sprite_composition_flag = 0;
	sp_new_pattern_count = 0;

	// dirtyフラグの処理
	for (i=0;i<NUM_SP_PAT_BUF;i++) {
		uint32_t flag = sp_composition_table[i];
		if (flag == 0) {
			continue;
		}
		for(j=0;j<3;j++) {
			if ( j == (flag >> 30) ) {
				break;
			}
			int ptNum = flag & 0x3f;
			flag >>= 10;
			if (sp_pattern_dirty[ptNum]) {
				// パターンが書き換えられたので、合成済みのパターンをクリア
				sp_composition_table[i] = 0;
				break;
			}
		}
	}
	// ダーティフラグをクリア
	for (i = 0; i < 64; i++) {
		sp_pattern_dirty[i] = 0;
	}

	uint8_t sp_pattern_used[NUM_SP_PAT_BUF];
	for(i=0;i<NUM_SP_PAT_BUF;i++) {
		sp_pattern_used[i] = 0;
	}

	uint8_t scroll_offset = vdp->r23; // 縦スクロール量

	// プレーンごとに処理
	for (plNum=0;plNum<32;) {
		if ( ((patr[plNum*SAT_SIZE+0] - scroll_offset) & 0xff) > 191) {
			// Y座標が範囲外のものは無視
			plNum++;
			continue;
		}
		int m = plNum;
		uint32_t composition_flag = 0;
		uint32_t color = pcol[plNum*COL_SIZE+8] & 0xf; // 8ライン目を代表色として使用
		if (color == 0) {
			//色コード0は無視
			plNum++;
			continue;
		}
		uint32_t ptnum = (patr[plNum*SAT_SIZE+2] / 4) & 0x3f; // 16x16ドットモードのときは4つで1つのパターン
		uint32_t flag = (color << 6) | ptnum;
		composition_flag = flag; 
		vdp->sprite_composition_flag &= ~(1 << plNum);
		for(i=1,n=plNum+1; i<3 && n<32; i++,n++) {
			// XY座標が同一のものかつ、CC=1のラインが一つでもあるものを抽出(連続している物のみ)
			if((patr[plNum*SAT_SIZE+0] == patr[n*SAT_SIZE+0]) && (patr[plNum*SAT_SIZE+1] == patr[n*SAT_SIZE+1]) && //
				(sprite_cc_flags[n] != 0)) {
				// 連続したものが見つかったので、mを更新
				m = n;
				vdp->sprite_composition_flag |= 1 << m;
				color = pcol[plNum*COL_SIZE+8] & 0xf; // 8ライン目を代表色として使用
				ptnum = patr[plNum*SAT_SIZE+2] / 4;	// 16x16ドットモードのときは4つで1つのパターン
				flag = ((color << 6) | (ptnum & 0x3f)) << (i*10);
				composition_flag |= flag;
			} else {
				break;
			}
		}
		composition_flag |= i << 30;	// 合成枚数をbit31-30に記録

		// この組み合わせがすでにPCG上に存在するかどうかを確認
		int found = 0;
		for(i=0;i<NUM_SP_PAT_BUF;i++) {
			if( sp_composition_table[i] == composition_flag) {
				// すでに合成済みのパターンが存在するので、それを使う
				X68_SSR[plNum*SSR_UNIT+2] = 0x100 + i; // パレット0x10-0x1fを使用するので 0x100を足す					
				X68_SSR[plNum*SSR_UNIT+3] = 3;	// スプライトは表示
				sp_pattern_used[i] = 1;
				found = 1;
				break;
			}
		}
		if (found) {
			plNum = m+1;
			continue;
		}

		// 合成済みのものが見つからなかった場合は、新たに合成する

		for(i=(lastused_sp_pat_buf+1) % NUM_SP_PAT_BUF ;;i= (i+1) % NUM_SP_PAT_BUF) {
			if( sp_pattern_used[i] ) {
				continue;
			}
			// 必ず一つは空いているはず
			break;
		}
		// このパターン番号に展開する
		int pat_num = i;
		lastused_sp_pat_buf = i;
		sp_pattern_used[pat_num] = 1;
		sp_composition_table[pat_num] = composition_flag;

		// sp_pat_buf[sp_new_pattern_count].plane_num = plNum;
		// sp_pat_buf[sp_new_pattern_count].composition_flag = composition_flag;

		// ラインごとの合成処理
		// 256モードの場合、16x16のスプライトがそのまま16x16のスプライトになるので、
		// 以下のように合成する
		// 一つの箱が X68000の 8x8のパターン(1ラインが32bit)を表す
		// X68000はスプライトサイズは16x16だが、定義は8x8の箱が4つ、
		// 左上→左下→右上→右下の順に集まっているので注意が必要
		//
		//  lr: 0   1  
		//    +---+---+	y=0		CCの0ライン目
		//    | 0 | 2 |
		//  　+---+---+	y=8		CCの8ライン目
		//    | 1 | 3 |
		//    +---+---+	y=15	CCの15ライン目
		// ※ 8x8のスプライトの場合は左上の0の部分だけが使われる
		int ymax = vdp->sprite_size == 0 ? 8 : 16;
		int lrmax = vdp->sprite_size == 0 ? 1 : 2;
		int ptNumMask = vdp->sprite_size == 0 ? 0xff : 0xfc;
		int lr;
		uint16_t mask = 1;
		for	(y=0; y<ymax; y++, mask <<= 1) {
			// yybase は上記パターンの一番左の列(0,15) の中で見た、X68000側のライン番号
			int yybase = y;
			i=plNum;
			while(i<=m) {
				uint32_t color = pcol[i*COL_SIZE+y] & 0xf;
				color = color == 0 ? vdp->alt_color_zero : color; // 「色コード0」のスプライトが消えてしまう問題への暫定対応
				uint32_t colorex = colorex_tbl[color];
				uint32_t ptNum = patr[i*SAT_SIZE+2];
				uint32_t pattern0 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D1X+yybase+8*0 ] & colorex;	// lr=0
				uint32_t pattern1 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D1X+yybase+8*2 ] & colorex;	// lr=1

				j=i;
				while(j<=m) {
					if( j == m ) {
						// 1ライン合成終了
//						sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*0] = pattern0;
//						sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*2] = (lrmax==2) ? pattern1 : 0;
						X68_PCG[pat_num*PCG_SIZE+yybase+8*0] = pattern0;
						X68_PCG[pat_num*PCG_SIZE+yybase+8*2] = (lrmax==2) ? pattern1 : 0;
						i=j+1;
						break;
					}
					j++; // j==mを先に判定しているので、j+1がmをオーバーすることはない
					// CC=1のものを合成する
					uint32_t color_add = pcol[j*COL_SIZE+y] & 0xf;
					color_add = color_add == 0 ? vdp->alt_color_zero : color_add; // 「色コード0」のスプライトが消えてしまう問題への暫定対応
					//uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
					uint32_t colorex_add = colorex_tbl[color_add];
					uint32_t ptNum_add = patr[j*SAT_SIZE+2];
					uint32_t pattern_add0 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D1X+yybase+8*0 ] & colorex_add;
					uint32_t pattern_add1 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D1X+yybase+8*2 ] & colorex_add;
					pattern0 |= pattern_add0;
					pattern1 |= pattern_add1;
				}
			}
		}
		// 8x8の時は残りを0で埋める
		for	(; y<16; y++) {
			int yybase = y;
			i=plNum;
			//sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*0] = 0;
			//sp_pat_buf[sp_new_pattern_count].pattern[yybase+8*2] = 0;
			X68_PCG[pat_num*PCG_SIZE+yybase+8*0] = 0;
			X68_PCG[pat_num*PCG_SIZE+yybase+8*2] = 0;
		}


		//sp_new_pattern_count++;

/*		// 次のプレーンへ
		X68_SSR[plNum*SSR_UNIT+3] = 3;	// スプライトは表示
		vdp->sprite_composition_flag &= ~(1 << plNum);
		while(plNum < m) {
			plNum++;
			X68_SSR[plNum*SSR_UNIT+3] = 0;	// 合成したスプライトは非表示
			vdp->sprite_composition_flag |= 1 << plNum;
		}*/
		plNum = m+1;
	}
	// 今回新たに合成したものを、空いているPCGに書き込む
/*	int new=0;
	for(i=0;i<NUM_SP_PAT_BUF;i++) {
		if( sp_new_pattern_count <= new) {
			break;
		}
		if( sp_composition_table[i] != 0) {
			continue;
		}
		// 空いているところが見つかったのでパターンを転送する
		for(j=0;j<32;j++) {
			X68_PCG[i*PCG_SIZE+j] = sp_pat_buf[new].pattern[j];
		}
		// プレーンにパターンを割り当てる
		X68_SSR[sp_pat_buf[new].plane_num * SSR_UNIT + 2] = 0x100 + i; // パレット0x10-0x1fを使用するので 0x100を足す
		//
		sp_composition_table[i] = sp_pat_buf[new].composition_flag;
		new++;
	}
	//空いているところがなかったら、今回使わなかったパターンをクリア
	for(i=(lastused_sp_pat_buf+1) % NUM_SP_PAT_BUF ;;i= (i+1) % NUM_SP_PAT_BUF) {
		if( sp_new_pattern_count <= new) {
			break;
		}
		if( sp_pattern_used[i] ) {
			continue;
		}
		// 空いているところが見つかったのでパターンを転送する
		lastused_sp_pat_buf = i;
		for(j=0;j<32;j++) {
			X68_PCG[i*PCG_SIZE+j] = sp_pat_buf[new].pattern[j];
		}
		// プレーンにパターンを割り当てる
		X68_SSR[sp_pat_buf[new].plane_num * SSR_UNIT + 2] = 0x100 + i; // パレット0x10-0x1fを使用するので 0x100を足す
		//
		sp_composition_table[i] = sp_pat_buf[new].composition_flag;
		new++;
	}*/


}

void refresh_sprite_512_mode2(ms_vdp_t* vdp) {
	int plNum,n,y,i,j;
	uint8_t* pcol = vdp->vram + vdp->sprcolrtbl_baddr;
	uint8_t* patr = vdp->vram + vdp->sprattrtbl_baddr;
	// スプライトモード2の色合成を行う
	for (plNum=0;plNum<32;plNum++) {
		int m = plNum;
		for(n=plNum+1;n<32;n++) {
			// XY座標が同一のものかつ、CC=1のラインが一つでもあるものを抽出(連続している物のみ)
			if((patr[plNum*SAT_SIZE+0] == patr[n*SAT_SIZE+0]) && (patr[plNum*SAT_SIZE+1] == patr[n*SAT_SIZE+1]) && //
				(sprite_cc_flags[n] != 0)) {
				m = n;
			} else {
				break;
			}
		}

		// ************
		//m = plNum; // テスト用
		// ************

		// TODO 512ドットモードでスプライトを縦に2倍に引き伸ばすことはできるっぽいので、それができると書き込み量が半分になる　

		// ラインごとの合成処理
		// 512モードの場合、1ラインが2ラインになるので、同じラインを2回描く
		// 16x16のスプライトの場合、以下のように合成する
		// 一つの箱が X68000の 8x8のパターン(1ラインが32bit)を表す
		// X68000はスプライトサイズは16x16だが、定義は8x8の箱が4つ、
		// 左上→左下→右上→右下の順に集まっているので注意が必要
		//
		//  lr: 0   1      2   3
		//    +---+---+  +---+---+	y=0		CCの0ライン目
		//    | 0 | 2 |  | 8 | A |
		//  　+---+---+  +---+---+	y=4		CCの4ライン目
		//    | 1 | 3 |  | 9 | B |	
		//    +---+---+  +---+---+	y=7		CCの7ライン目
		//  lr: 0   1      2   3
		//    +---+---+  +---+---+	y=8		CCの8ライン目
		//    | 4 | 6 |  | C | E |
		//    +---+---+  +---+---+
		//    | 5 | 7 |  | D | F |
		//    +---+---+  +---+---+	y=15	CCの15ライン目
		// ※ 8x8のスプライトの場合は左上の0,1,2,3の部分だけが使われる

		// 各yに対してX68000側は2ラインになるが、ループはMSX側のライン数でおこない、
		// X68000側に同じパターンを連続で書くようにしている。よってこの yは 8 or 16でOK。
		int ymax = vdp->sprite_size == 0 ? 8 : 16;
		// lrは 8ドットサイズの場合は2回、16ドットサイズの場合は4回繰り返す
		int lrmax = vdp->sprite_size == 0 ? 2 : 4;
		int ptNumMask = vdp->sprite_size == 0 ? 0xff : 0xfc;
		int lr;
		uint16_t ccmask = 1;
		for	(y=0; y<ymax; y++, ccmask <<= 1) {
			// yybase は上記パターンの一番左の列(0,1,4,5) の中で見た、X68000側のライン番号
			// ここで出てくるyybaseは必ず偶数になります。
			int yybase = ( y & 0x8)/8*4*8 +	// 外の上段下段
						 ( y & 0x4)/4*1*8 +	// 中の上段下段
						 ( y & 0x3)  *2;	// 512モードの場合、1ラインが2ラインになる
			i=plNum;
			while(i<=m) {
				uint32_t color = pcol[i*COL_SIZE+y] & 0xf;
				color = color == 0 ? vdp->alt_color_zero : color; // 「色コード0」のスプライトが消えてしまう問題への暫定対応
				//uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
				uint32_t colorex = colorex_tbl[color];
				uint32_t ptNum = patr[i*SAT_SIZE+2];
				uint32_t pattern0 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*0 ] & colorex;	// lr=0
				uint32_t pattern1 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*2 ] & colorex;	// lr=1
				uint32_t pattern2 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*8 ] & colorex;	// lr=2
				uint32_t pattern3 = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D2X+yybase+8*10] & colorex;	// lr=3

				j=i;
				while(j<=m) {
					if( j == m ) {
						X68_PCG[i*PCG_UNIT+yybase+8*0 +0] = pattern0;
						X68_PCG[i*PCG_UNIT+yybase+8*0 +1] = pattern0;	//奇数ラインも同じものになる(yyは必ず偶数)
						X68_PCG[i*PCG_UNIT+yybase+8*2 +0] = pattern1;
						X68_PCG[i*PCG_UNIT+yybase+8*2 +1] = pattern1;	//奇数ラインも同じものになる(yyは必ず偶数)
						X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = (lrmax==4) ? pattern2 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*8 +1] = (lrmax==4) ? pattern2 : 0;	//奇数ラインも同じものになる(yyは必ず偶数)
						X68_PCG[i*PCG_UNIT+yybase+8*10+0] = (lrmax==4) ? pattern3 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*10+1] = (lrmax==4) ? pattern3 : 0;	//奇数ラインも同じものになる(yyは必ず偶数)
						i=j+1;
						break;
					}
					j++; // j==mを先に判定しているので、i+1がmをオーバーすることはない
					if( (sprite_cc_flags[j]&ccmask) == 0) {
						// CC=0に遭遇したら、それ以降は合成しない
						X68_PCG[i*PCG_UNIT+yybase+8*0 +0] = pattern0;
						X68_PCG[i*PCG_UNIT+yybase+8*0 +1] = pattern0;	//奇数ラインも同じものになる(yyは必ず偶数)
						X68_PCG[i*PCG_UNIT+yybase+8*2 +0] = pattern1;
						X68_PCG[i*PCG_UNIT+yybase+8*2 +1] = pattern1;	//奇数ラインも同じものになる(yyは必ず偶数)
						X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = (lrmax==4) ? pattern2 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*8 +1] = (lrmax==4) ? pattern2 : 0;	//奇数ラインも同じものになる(yyは必ず偶数)
						X68_PCG[i*PCG_UNIT+yybase+8*10+0] = (lrmax==4) ? pattern3 : 0;
						X68_PCG[i*PCG_UNIT+yybase+8*10+1] = (lrmax==4) ? pattern3 : 0;	//奇数ラインも同じものになる(yyは必ず偶数)
						i=j;
						break;
					}
					// CC=1のものが見つかったので合成する
					uint32_t color_add = pcol[j*COL_SIZE+y] & 0xf;
					color_add = color_add == 0 ? vdp->alt_color_zero : color_add; // 「色コード0」のスプライトが消えてしまう問題への暫定対応
					uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
					uint32_t ptNum_add = patr[j*SAT_SIZE+2];
					uint32_t pattern_add0 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*0 ] & colorex_add;
					uint32_t pattern_add1 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*2 ] & colorex_add;
					uint32_t pattern_add2 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*8 ] & colorex_add;
					uint32_t pattern_add3 = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D2X+yybase+8*10] & colorex_add;
					pattern0 |= pattern_add0;
					pattern1 |= pattern_add1;
					pattern2 |= pattern_add2;
					pattern3 |= pattern_add3;
				}
			}
		}
		// 8x8の時は残りを0で埋める
		for	(; y<16; y++) {
			int yybase = ( y & 0x8)/8*4*8 +	// 外の上段下段
						 ( y & 0x4)/4*1*8 +	// 中の上段下段
						 ( y & 0x3)  *2;	// 512モードの場合、1ラインが2ラインになる
			i=plNum;
			X68_PCG[i*PCG_UNIT+yybase+8*0 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*0 +1] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*2 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*2 +1] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*8 +0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*8 +1] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*10+0] = 0;
			X68_PCG[i*PCG_UNIT+yybase+8*10+1] = 0;
		}

		// 次のプレーンへ
		X68_SSR[plNum*SSR_UNIT+4*0+3] = 3;	// スプライトは表示
		X68_SSR[plNum*SSR_UNIT+4*1+3] = 3;	// スプライトは表示
		X68_SSR[plNum*SSR_UNIT+4*2+3] = 3;	// スプライトは表示
		X68_SSR[plNum*SSR_UNIT+4*3+3] = 3;	// スプライトは表示
		vdp->sprite_composition_flag &= ~(1 << plNum);
		while(plNum < m) {
			plNum++;
			X68_SSR[plNum*SSR_UNIT+4*0+3] = 0;	// 合成したスプライトは非表示
			X68_SSR[plNum*SSR_UNIT+4*1+3] = 0;	// 合成したスプライトは非表示
			X68_SSR[plNum*SSR_UNIT+4*2+3] = 0;	// 合成したスプライトは非表示
			X68_SSR[plNum*SSR_UNIT+4*3+3] = 0;	// 合成したスプライトは非表示
			vdp->sprite_composition_flag |= 1 << plNum;
		}
	}
}

void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute, int32_t old_attribute) {
	int i,j;
	int plNum = (offset / SAT_SIZE) % 32; // MSXのスプライトプレーン番号
	int type = offset % SAT_SIZE; // 属性の種類

	switch(type) {
		case 0: // Y座標
			// Y=208/216の対応のため、Y座標の208/216の値が変化したら、スプライトの再配置を行う
			int HY = (vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1 ? 208 : 216;
			if ( attribute == HY || old_attribute == HY) {
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
			}
			uint8_t scroll_offset = vdp->r23; // 縦スクロール量
			// MSXは1ライン下に表示されるので+1
			// MSXはR23の縦スクロールでスプライトも一緒にスクロールするので、その分を引く
			// 512モードはMSXのY座標の2倍
			// X68000のスプライトの原点は(16,16)なのでずらす
			int y = ((attribute + 1 - scroll_offset) & 0xff)*2 + 16;
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+1] = y + (i%2)*16;
			}
			break;
		case 1: // X座標
			int ec = 0;
			int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;
			switch(spMode) {
			case 1:
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				break;
			case 2:
				uint8_t* pcolr = vdp->vram + vdp->sprcolrtbl_baddr;
				ec = (pcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ラインごとのECはサポートしないので1ライン目だけみる
				break;
			}
			int x = attribute & 0xff;
			int adjustx = get_sprite_adjustx(vdp);
			x = ((x - ec*32)*2 + 16 + adjustx*2) & 0x3ff; // MSXのX座標の2倍
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+0] = x + (i/2)*16;
			}
			break;
		case 2: // パターン番号
		case 3: // 属性
			// パターン番号、カラーが変更されたら、事前にバッファに展開しておいたパターンを転送
			refresh_sprite_512(vdp, plNum);
			break;
		default:
			break;
	}
}


void refresh_sprite_512(ms_vdp_t* vdp, int plNum) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1) {
		refresh_sprite_512_mode1(vdp, plNum);
	} else {
		// モード2は一つ書き換えると全体に影響が出るので、垂直帰線期間にまとめて書き換える
		// TODO: それなりに重いので、もう少し範囲を限定したい
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
	}
}

void refresh_sprite_512_mode1(ms_vdp_t* vdp, int plNum) {
	int i;
	uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;

	unsigned int ptNum = pattr[plNum*SAT_SIZE+2];
	uint32_t color = pattr[plNum*SAT_SIZE+3] & 0xf;
	color = color == 0 ? vdp->alt_color_zero : color; // 「色コード0」のスプライトが消えてしまう問題への暫定対応
	// MSXの4ドット分(X68000だと2倍の8ドットに拡大)
	//uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
	uint32_t colorex = colorex_tbl[color];
	if (vdp->sprite_size == 0) { // 8x8
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*PCG_BUF_UNIT_D2X+i] & colorex;
		}
	} else { // 16x16
		for( i = 0; i < 32*4; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*PCG_BUF_UNIT_D2X+i] & colorex;
		}
	}
}

/*
 スプライトカラーテーブルへの書き込み
	 offset: カラーテーブルのベースアドレスからのオフセットバイト
	 pattern: 書き込む値(下位8bitのみ使用)
*/
void write_sprite_color(ms_vdp_t* vdp, int offset, uint32_t color, int32_t old_color) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) != 2) {
		//　スプライトモード2以外は何もしない
		return;
	} else {
		// 毎回反映するのは大変なので、VRAMにだけ書いておき、フラグを立て、垂直帰線期間にまとめた処理する
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_CC;

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

/**
 * @brief VSYNC期間にスプライトの更新をまとめて行う
 * 
 * @param vdp 
 */
void ms_vdp_sprite_vsync_draw(ms_vdp_t* vdp) {
	//vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;	// 全書き換え
	//vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;	// アトリビュートテーブルのみ再検査
	//vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;	// 実験的に、位置調整は毎回行うようにしてみる

	if (!vdp->sprite_refresh_flag) {
		return;
	}

	int hostdebugmode = vdp->hostdebugmode;

	uint8_t* vram = vdp->vram;

	int spSize = vdp->sprite_size == 0 ? 8 : 16;
	int mag512 = (vdp->ms_vdp_current_mode->sprite_mode & 0x80) ? 2 : 1;
	int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;

	int i,j;
	int plNum;
	uint16_t flag = vdp->sprite_refresh_flag;
	if (flag & SPRITE_REFRESH_FLAG_FULL) {
		vdp->sprite_composition_flag = 1;
		for(plNum = 0; plNum < 32; plNum++) {
			if( mag512 == 2 && spSize == 16) {
				// 512ドットモード、16x16サイズの時
				for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+2] = 0x100 + plNum*4+i; // パレット0x10-0x1fを使用するので 0x100を足す
				}
			} else {
				// 256ドットモードの時、512ドットモードで8x8サイズの時
				X68_SSR[plNum*SSR_UNIT+2] = 0x100 + plNum*4; // パレット0x10-0x1fを使用するので 0x100を足す					
				for( i=1; i<4; i++) {
					X68_SSR[plNum*SSR_UNIT+i*4+3] = 0;	// 8x8の時は後ろ3枚のスプライトは非表示
				}
			}
		}
		flag |= SPRITE_REFRESH_FLAG_PGEN;	// 以降の処理も全て行う
	}
	if (flag & SPRITE_REFRESH_FLAG_PGEN) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 11;	// Green
		}
		// パターンジェネレータテーブルからPCGバッファの再構築を行います
		uint32_t sprpgenaddr = vdp->sprpgentbl_baddr & 0x1fe00; // 下位9ビットをクリア
		for(i=0;i<256;i++) {
			for(j=0;j<8;j++) {
				write_sprite_pattern(vdp, i*8+j, vram[sprpgenaddr + i*8 + j], -1);
			}
		}
		flag |= SPRITE_REFRESH_FLAG_CC;	// CCフラグ更新、PCG更新、スプライトアトリビュートテーブルの更新も行う
	}
	if (flag & SPRITE_REFRESH_FLAG_CC) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 6;	// Red
		}
		// スプライトカラーテーブルのCCビットマップフラグを再作成
		if (spMode == 2) {
			uint8_t* sprcolr = vram + vdp->sprcolrtbl_baddr;
			for(plNum=0; plNum<32; plNum++) {
				uint16_t ccflag = 0;
				for(i=0; i<16; i++) {
					if (*sprcolr++ & 0x40) { // CCビットを検査
						ccflag |= (1 << i);
					}
				}
				sprite_cc_flags[plNum] = ccflag;
			}
		}
		flag |= SPRITE_REFRESH_FLAG_ATTR;	// スプライトアトリビュートテーブル(パターン番号)の更新も行う
	}
	if (flag & SPRITE_REFRESH_FLAG_ATTR) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 1;	// Blue
		}
		// PCG更新処理
		if (mag512 == 2 ) {
			// 512ドットモードの時
			if (spMode == 1) {
				for(i=0;i<32;i++) {
					refresh_sprite_512(vdp, i);
				}
			} else {
				refresh_sprite_512_mode2(vdp);
			}
		} else {
			// 256ドットモードの時
			if (spMode == 1) {
				for(i=0;i<32;i++) {
					refresh_sprite_256_mode1(vdp, i);
				}
			} else {
				refresh_sprite_256_mode2(vdp);
			}
		}
		flag |= SPRITE_REFRESH_FLAG_COORD;	// スプライトアトリビュートテーブル(表示位置)の更新も行う
	}
	if (flag & SPRITE_REFRESH_FLAG_COORD) {
		if(hostdebugmode) {
			X68_TX_PAL[0] = 0x1f << 11 | 0x1f << 6;	// Yellow
		}
		// スプライトアトリビュートテーブルのXY座標のみの更新
		int HY = (vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1 ? 208 : 216;
		uint8_t* sprattr = vram + vdp->sprattrtbl_baddr;
		uint8_t* sprcolr = vram + vdp->sprcolrtbl_baddr;
		uint8_t scroll_offset = vdp->r23; // 縦スクロール量
		int visible_sprite_planes = 0;
		plNum = 0;
		int x,y,ec = 0;
		int adjustx = get_sprite_adjustx(vdp) * mag512;
		if ( (vdp->r08 & 0x02) == 0 ) {
			// スプライト非表示ではない時
			for(;plNum<32;plNum++) {
				y = sprattr[plNum*SAT_SIZE+0];
				x = sprattr[plNum*SAT_SIZE+1];
				if ( y == HY) {
					// このプレーン以降は描画しない
					break;
				}
				if(spMode == 1) {
					if( (sprattr[plNum*SAT_SIZE+3] & 0x04) == 0) {
						// 色コード0のスプライトは表示しない
						for( i=0; i<4; i++) {
						//	X68_SSR[plNum*SSR_UNIT+i*4+3] = 0;	// スプライトは非表示
						}
						//continue;
					}
					ec = (sprattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				} else {
					if(vdp->sprite_composition_flag & (1 << plNum)) {
						// 合成されたスプライトプレーンは手前のプレーンに合成済みなのでスキップ
						X68_SSR[plNum*SSR_UNIT+3] = 0; // スプライト悲表示
						continue;
					}
					ec = (sprcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ラインごとのECはサポートしないので1ライン目だけみる
				}
				y = ((y + 1 - scroll_offset) & 0xff) * mag512 + 16;
				x = ((x  - ec*32)* mag512 + 16 + adjustx) & 0x3ff;
				if( mag512 == 2 && spSize == 16) {
					// 512ドットモード、16x16サイズの時
					for( i=0; i<4; i++) {
						X68_SSR[plNum*SSR_UNIT+i*4+0] = x + (i/2)*16;
						X68_SSR[plNum*SSR_UNIT+i*4+1] = y + (i%2)*16;
						X68_SSR[plNum*SSR_UNIT+i*4+3] = 3; // スプライト表示
					}
				} else {
					// 256ドットモードの時、512ドットモードで8x8サイズの時
					X68_SSR[plNum*SSR_UNIT+0] = x;
					X68_SSR[plNum*SSR_UNIT+1] = y;
					X68_SSR[plNum*SSR_UNIT+3] = 3; // スプライト表示
				}
			}
		}
		visible_sprite_planes = plNum;
		// 以下のスプライトプレーンは非表示にする
		for(;plNum<32;plNum++) {
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+3] = 0; // スプライト非表示
			}
		}
		vdp->last_visible_sprite_planes = visible_sprite_planes;
		vdp->last_visible_sprite_size = vdp->sprite_size;
	}

	// フラグクリア
	vdp->sprite_refresh_flag = 0;
}