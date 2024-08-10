/*s_p_GRAPHIC2

set_CRT_jpt:
	set_GRAPHIC2

VDP_G5_jpt

read_rot:
	.dc.l	r_TEXT1			* ＶＲＡＭ読み込みルーチンのアドレス

write_rot:
	.dc.l	w_TEXT1			* ＶＲＡＭ書き込みルーチンのアドレス

set_gram_add_rot:
	.dc.l	s_TEXT1

VDP_command_jpt_add:
*/


/*
 MSXの画面モードごとに切り替える処理群
 
 * init
 	* X68000側のCRTCパラメータ設定などを行う
 * VRAM読み込み
 * VRAM書き込み
	* VDPレジスタの値とアドレスを比較し、以下を分岐
		* 画面書き換え系
		* スプライトの書き換え
 * VDPコマンドの実行
 * 

 */