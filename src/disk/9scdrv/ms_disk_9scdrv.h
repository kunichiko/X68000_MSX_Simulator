#ifndef MS_DISK_9SCDRV_H
#define MS_DISK_9SCDRV_H

#include <stdint.h>

typedef struct xkpchk_result {
    // 常駐している 9SCDRV.X のバージョンコード（アスキーコードで 'v' + ３桁の数字）。常駐していない時は -1 (0xffffffff)。
    uint32_t version;
    // 常駐している 9SCDRV.X のリビジョンコード（アスキーコードで 'r' + ３桁の数字）。
    uint32_t revision;
    // 9SCCALL エントリテーブル先頭アドレス。テーブル構造につきましては ｢_X_VECTOR｣ の項目をご覧下さい。
    void* table;
} xkpchk_result_t;

void ms_disk_9scdrv_init(xkpchk_result_t* result);

/* 共通パラメータ
    PDA  : 0x90～0x93 (ドライブ0-3)

    MODE : bit6  0=FM, 1=MFM
           bit5  0=リトライしない, 1=10回リトライする
           bit4  0=シークせずに実行, 1=シーク後に実行
           bit3-0  0固定

    D2.L : 位置指定
           bit31-24 セクタ長(0=128, 1=256, 2=512, 3=1024)
                   bit23-16 トラック番号(0～)
                   bit15-8  サイド(0,1)
                   bit7-0   セクタ番号(1～)

    メディアバイト : 2HD/2HC=$FE,2HS=$FB,2HDE=$F8,2DD=$F9,任意=$FF(1.44M)）
  */

typedef enum {
    MS_DISK_9SCDRV_DRV0 = 0x90,
    MS_DISK_9SCDRV_DRV1 = 0x91,
    MS_DISK_9SCDRV_DRV2 = 0x92,
    MS_DISK_9SCDRV_DRV3 = 0x93
} ms_disk_9scdrv_drive_t;

typedef enum {
    MS_DISK_9SCDRV_SECTOR_SIZE_128 = 0,
    MS_DISK_9SCDRV_SECTOR_SIZE_256 = 1,
    MS_DISK_9SCDRV_SECTOR_SIZE_512 = 2,
    MS_DISK_9SCDRV_SECTOR_SIZE_1024 = 3
} ms_disk_9scdrv_sector_size_t;

typedef enum {
    MS_DSK_9SCDRV_MEDIABYTE_2HD_2HC = 0xFE,
    MS_DSK_9SCDRV_MEDIABYTE_2HS = 0xFB,
    MS_DSK_9SCDRV_MEDIABYTE_2HDE = 0xF8,
    MS_DSK_9SCDRV_MEDIABYTE_2DD = 0xF9,
    MS_DSK_9SCDRV_MEDIABYTE_OTHER = 0xFF
} ms_disk_9scdrv_media_byte_t;

uint32_t convert_to_position(ms_disk_9scdrv_sector_size_t sector_size_code, uint8_t track, uint8_t side, uint8_t sector);

/**
===< $201C _X_MEDIA >=================================================

機能	指定ドライブのディスクの種類を判定します。

引数	D1.W	上位８ビット PDA/下位８ビット MODE

返り値	D2.L	メディアの調査結果

        ┌───┬───┬───┬───┐
 D2.L   │ $00  │*ﾄﾗｯｸ │ｽﾃｰﾀｽ │ﾒﾃﾞｨｱ │
        └───┴───┴───┴───┘
        1F       17       F      7     0

        メディア　:　9SCDRV.X 内の該当ドライブのメディアコード
    ┌───────┬───────┬───────┬───────┐
    ｜-1：NO DISK   │４：2HT       │９：2HQ(1.44M)│14：2D(320K)  │
    │０：2HD       │５：2DD(720K) │10：User-Media│15：1D(180K)  │
    │１：2HS       │６：2DD(640K) │11：2DD(800K) │16：1D(160K)  │
    │２：2HC       │７：1DD(360K) │12：2HQ(1.6M) │17：DMF(1.68M)│
    │３：2HDE      │８：1DD(320K) │13：2D(360K)  ｜-128:Lock状態 │
    └───────┴───────┴───────┴───────┘

        ステータス:　通常0、無効なメディアの時-1。

        トラック　:　通常0。オーバートラックディスクに対してオーバート
                　　 ラック判定を行った時、そのトラック数が入る。但し、
                　　 9SCDRV.X の常駐オプションに /E が指定されている時
                     のみ有効。
*/
uint32_t ms_disk_9scdrv_media(ms_disk_9scdrv_drive_t pda, uint8_t mode);

/**
===< $2008 _X_READ >==================================================

機能	フロッピーディスクのデータを読み出します。

引数	D1.W	上位８ビット PDA/下位８ビット MODE
        D2.L	目的読み込み位置
        D3.L	読み込みバイト数
        D4.B	メディアバイト
        A1.L	読み込みデータ先頭アドレス

返り値	D0.L	FDC からのステータス
                    bit 0～7	コマンド終了時のトラックナンバー
                    bit 8～$F	ステータスレジスタ２
                    bit $10～$17	ステータスレジスタ１
                    bit $18～$1F	ステータスレジスタ０
        D2.L	最後に読み込んだセクタの位置
        D3.L	０になる
        A1.L	読み込みデータの次のデータアドレス
 */
typedef struct ms_disk_9scdrv_read_result {
    uint32_t status;       // D0.L
    uint32_t last_sector;  // D2.L
    uint32_t zero;         // D3.L
    void* next_addr;       // A1.L
} ms_disk_9scdrv_read_result_t;

ms_disk_9scdrv_read_result_t* ms_disk_9scdrv_read(ms_disk_9scdrv_drive_t pda, uint8_t mode, uint32_t position, uint32_t size,
                                                  ms_disk_9scdrv_media_byte_t media_byte, uint8_t* buffer);
#endif