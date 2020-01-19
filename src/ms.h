/*

		ＭＳＸ.Ｓｉｍｕｌａｔｅｒ [[ MS ]]
				[[ ヘッダファイル ]]
								
				prpgramed by Kuni.
										1995.12.06

*/

/*	スロットが拡張されているかどうか	*/
struct slot_ex_info_st {
	char slot_0;
	char slot_1;
	char slot_2;
	char slot_3;
};

struct one_slot_st {
	char *page_0;
	char *page_1;
	char *page_2;
	char *page_3;
};

/*	あるページの実体を指すポインタ。バンク切り替えのできるページの場合、現在	*/
/*	選択されているものが入っている。											*/
struct slot_pointer_st	{
	struct one_slot_st s00;
	struct one_slot_st s01;
	struct one_slot_st s02;
	struct one_slot_st s03;

	struct one_slot_st s10;
	struct one_slot_st s11;
	struct one_slot_st s12;
	struct one_slot_st s13;

	struct one_slot_st s20;
	struct one_slot_st s21;
	struct one_slot_st s22;
	struct one_slot_st s23;

	struct one_slot_st s30;
	struct one_slot_st s31;
	struct one_slot_st s32;
	struct one_slot_st s33;
};

void *new_malloc( int );
