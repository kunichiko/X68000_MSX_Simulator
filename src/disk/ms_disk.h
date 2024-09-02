#ifndef MS_DISK_H
#define MS_DISK_H

#include <stdint.h>

/**
 * @brief フロッピーディスクの生のトラックデータを扱う構造体です。
 * 2DD 720kBの場合、250kB/Sec で 300rpmとなり、1トラックあたり 6250バイトとなります。
 * IBMフォーマットの場合、6250バイトの内訳は、トラックの先頭から順に、以下のようになっています。
 * * プリアンブル
 * * セクタ1
 * 		* IDフィールド
 * 		* (ギャップ)
 * 		* データフィールド（512バイト)
 * 		* CRC
 * 		* (ギャップ)
 * * セクタ2
 * 		* IDフィールド
 * 		* (ギャップ)
 * 		* データフィールド (512バイト)
 * 		* CRC
 * 		* (ギャップ)
 * * セクタ3-9
 * 		* 同様
 * * ポストアンブル
 * 
 * メディアクラス(ms_disk_media_t)とは、このraw trackでデータをやり取りします。
 * raw trackをセクター情報に変換してアクセスするのは、コントローラーの責務です。
 * 
 * なお、メディアクラスの実装には、セクタベースでイメージを扱う ms_disk_media_disk_t型もありますが、
 * その場合でも、一度 raw track の状態にしたものを再度 controllerでセクタに戻すことになります。
 * 処理としては冗長になりますが、将来的にコピープロテクトの再現などで特殊フォーマットを扱えるようにするためには
 * このようなインターフェースが必要です。
 */
typedef struct ms_disk_raw_track {
	uint8_t data[6250];
} ms_disk_raw_track_t;

typedef struct ms_disk_sector {
	uint8_t track;
	uint8_t head;		// FDDの場合は side
	uint8_t sector;
	uint8_t deleted;
	uint8_t data[512];
	uint16_t crc;
	uint16_t crc_expected;
} ms_disk_sector_t;

#endif