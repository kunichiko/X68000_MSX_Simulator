/* *****************************************************************

  汎用入出力インターフェース（ジョイスティック端子）

  AY-3-8910にはPortAとPortBという2つの8bit GPIOがついていて、
   R#14 - PortA
   R#15 - PortB
  というアサインになっている。
  ハードウェア的には読み書きが可能で、R#7のBit6,7によってそれぞれのポートの
  入出力方向を決められるようになっている。
  しかし、MSXの場合は、PortAとPortBの先の接続が以下のようになっていて、
  PortAを入力、PortBを出力に設定して使うことになっている。
    https://www.msx.org/wiki/General_Purpose_port

  本エミュレータは R#7の設定に関わらず、PortAを入力、PortBを出力として扱う。

  PortA,Bの詳細は以下の通り。

  PortA (R#14, 入力)
   bit7 - カセット入力
    bit6 - キーボードレイアウト(0:五十音, 1:JIS)
    bit5 - トリガB (7番ピン)
    bit4 - トリガA (6番ピン)
    bit3 - 方向キー右 (4番ピン)
    bit2 - 方向キー左 (3番ピン)
    bit1 - 方向キー下 (2番ピン)
    bit0 - 方向キー上 (1番ピン)
    ※ bit0-5は74LS157の出力端子が繋がっていて、ジョイスティック端子1もしくは2の
      どちらの端子の状態を読むかを切り替えられるようになっている
      選択の切り替えはPortBのbit6で行う

  PortB (R#15, 出力)
    bit7 - かなLED(0:点灯, 1:消灯)
    bit6 - ジョイスティックポート選択(0:ジョイスティック1, 1:ジョイスティック2)
    bit5 - ジョイスティック端子2の8番ピン
    bit4 - ジョイスティック端子1の8番ピン
    bit3 - ジョイスティック端子2の7番ピン(トリガB)
    bit2 - ジョイスティック端子2の6番ピン(トリガA)
    bit1 - ジョイスティック端子1の7番ピン(トリガB)
    bit0 - ジョイスティック端子1の6番ピン(トリガA)
    ※ bit0-3はバッファICが繋がっているので、PortBを入力モードにして読んでも常に固定値が読める(IC内部でプルアップされているので1のはず)
    ※ bit4,5はバッファがないようなので、PortBを入力モードにすれば 8番ピンの読み込みもできるのではないか?(未確認)

***************************************************************** */
#include "ms_psg_joy.h"

#include <x68k/iocs.h>

uint8_t ms_psg_read_R14_direct();
void ms_psg_write_R15_direct(uint8_t data);

uint16_t ms_psg_port_sel;

/**
 * @brief Port A の読み込み
 *
 * @param shared
 * @return uint8_t
 */
uint8_t ms_psg_read_R14(ms_psg_t* shared) {
    uint8_t ret;
    if (shared->use_iocs) {
        uint8_t data = _iocs_joyget(ms_psg_port_sel);
        // bit7のカセット処理は未実装
        // bit6のキーボードレイアウトは固定でJIS配列
        ret = 0b11000000 |                  //
              ((data & 0b01100000) >> 1) |  //
              ((data & 0b00001111));
    } else {
        ret = ms_psg_read_R14_direct();
    }
    if (shared->swap_AB) {
        return ((ret & 0b11001111)) |       //
               ((ret & 0b00100000) >> 1) |  //
               ((ret & 0b00010000) << 1);
    } else {
        return ret;
    }
}

/**
 * @brief
 *
 * @param shared
 * @return uint8_t
 */
uint8_t ms_psg_read_R15(ms_psg_t* shared) {
    // R15(PortB)は通常書き込みモードで使用しているので、読み出した場合は 最後に書き込んだ値が読めるようにする
    return shared->r15;
}

void ms_psg_write_R15(ms_psg_t* shared, uint8_t data) {
    shared->r15 = data;

    // bit7: かなLEDの状態
    // フラグを保存して、VSYNC期間に反映させる
    ms_peripherals_led_kana = (data & 0x80) ? 0 : 1;  // 論理は逆
    // bit6: ジョイスティックポート選択(0:ジョイスティック1, 1:ジョイスティック2)
    ms_psg_port_sel = (data & 0x40) ? 1 : 0;

    if (shared->use_iocs) {
        // IOCS使用時はR#15 (PortB) の出力は対応しない
    } else {
        ms_psg_write_R15_direct(data);
    }
}