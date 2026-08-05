SRC_DIR = src
VDP_DIR = $(SRC_DIR)/vdp
MEMMAP_DIR = $(SRC_DIR)/memmap
DISK_DIR = $(SRC_DIR)/disk
DISK_9SCDRV_DIR = $(DISK_DIR)/9scdrv
PERIPHERAL_DIR = $(SRC_DIR)/peripheral
UTHASH_DIR = $(SRC_DIR)/submodules/uthash/src

ASMINC_DIR = $(SRC_DIR)/include
BUILD_DIR = build
EXE_DIR = exe

CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/prog/HAS/HAS060.X
LD = $(CROSS)gcc

CFLAGS = -g -std=gnu90 -c -m68000 -O3 -finput-charset=UTF-8 -fexec-charset=CP932 -I${UTHASH_DIR}
CFLAGS_DEBUG = -g -std=gnu90 -c -m68000 -DDEBUG -finput-charset=UTF-8 -fexec-charset=CP932 -I${UTHASH_DIR}
LDFLAGS = src/disk/9scdrv/_X_KPCHK_elf.o -lm -lbas -liocs -ldos -Wl,-e,msboot_start
#LDFLAGS = src/disk/9scdrv/_X_KPCHK_elf.o -lm -lbas -liocs -ldos
#GCC_OPTS = -c -O -g -finput-charset=UTF-8 -fexec-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

-include ~/.human68k.mk

ASFLAGS = -i $(SRC_DIR) -i $(ASMINC_DIR) -i $(VDP_DIR) -i $(MEMMAP_DIR) -i ${DISK_DIR} -i ${PERIPHERAL_DIR} -w0
ASFLAGS_DEBUG = -d -s DEBUG $(ASFLAGS)

# ソースを探索するディレクトリ (vpath でサブフォルダごとの重複ルールを1組にまとめる)
SRC_DIRS = $(SRC_DIR) $(VDP_DIR) $(MEMMAP_DIR) $(DISK_DIR) $(DISK_9SCDRV_DIR) $(PERIPHERAL_DIR)
vpath %.c $(SRC_DIRS)
vpath %.has $(SRC_DIRS)

# ヘッダを更新したら全オブジェクトを作り直す (依存指定を簡潔にするため一括)
HEADERS = $(wildcard $(SRC_DIR)/*.h $(VDP_DIR)/*.h $(MEMMAP_DIR)/*.h $(DISK_DIR)/*.h $(DISK_9SCDRV_DIR)/*.h $(PERIPHERAL_DIR)/*.h)

# アセンブラソースは UTF-8 で管理し、HAS060 が要求する SJIS(CP932) に変換した
# シャドウツリー ($(ASMSRC_DIR)) を作ってからアセンブルする。.include も元のパス
# 基準で読まれるため、ディレクトリ構造ごと複製する。
# (macOS の iconv は CP932 変換が不安定なため Python の cp932 コーデックを使用)
UTF8TOCP932 = python3 -c 'import sys;sys.stdout.buffer.write(open(sys.argv[1],encoding="utf-8").read().encode("cp932"))'
ASMSRC_DIR = $(BUILD_DIR)/asmsrc
ASM_SOURCES = $(shell find $(SRC_DIR) -type f \( -name '*.has' -o -name '*.mac' -o -name '*.MAC' -o -name '*.equ' \))
ASM_STAMP = $(ASMSRC_DIR)/.stamp

# オブジェクトファイルのリストを変数にまとめる
OBJS =	$(BUILD_DIR)/msboot_mac.o \
		$(BUILD_DIR)/ms.o \
		$(BUILD_DIR)/ms_R800_30_mac.o \
		$(BUILD_DIR)/ms_R800_flag_mac.o \
		$(BUILD_DIR)/ms_iomap.o \
		$(BUILD_DIR)/ms_iomap_mac.o \
		$(BUILD_DIR)/ms_sysvalue_mac.o \
		$(BUILD_DIR)/ms_sub_mac.o \
		$(BUILD_DIR)/ms_peripherals_mac.o \
		$(BUILD_DIR)/ms_rtc.o \
		$(BUILD_DIR)/ms_psg.o \
		$(BUILD_DIR)/ms_psg_mac.o \
		$(BUILD_DIR)/ms_psg_joy.o \
		$(BUILD_DIR)/ms_psg_joy_mac.o \
		$(BUILD_DIR)/ms_kanjirom12.o \
		$(BUILD_DIR)/ms_kanjirom_alt.o \
		$(BUILD_DIR)/ms_memmap.o \
		$(BUILD_DIR)/ms_memmap_mac.o \
		$(BUILD_DIR)/ms_memmap_util.o \
		$(BUILD_DIR)/ms_memmap_driver.o \
		$(BUILD_DIR)/ms_memmap_NOTHING.o \
		$(BUILD_DIR)/ms_memmap_MAINRAM.o \
		$(BUILD_DIR)/ms_memmap_NORMALROM.o \
		$(BUILD_DIR)/ms_memmap_MIRROREDROM.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_GENERIC_8K.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_ASCII_8K.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_KONAMI.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_KONAMI_SCC.o \
		$(BUILD_DIR)/ms_memmap_PAC.o \
		$(BUILD_DIR)/ms_memmap_ESE_RAM.o \
		$(BUILD_DIR)/ms_memmap_ESE_SCC.o \
		$(BUILD_DIR)/ms_vdp.o \
		$(BUILD_DIR)/ms_vdp_mac.o \
		$(BUILD_DIR)/ms_vdp_sprite.o \
		$(BUILD_DIR)/ms_vdp_command.o \
		$(BUILD_DIR)/ms_vdp_mode_DEFAULT.o \
		$(BUILD_DIR)/ms_vdp_mode_TEXT1.o \
		$(BUILD_DIR)/ms_vdp_mode_TEXT2.o \
		$(BUILD_DIR)/ms_vdp_mode_MULTICOLOR.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC1.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC2.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC3.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC4.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC5.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC6.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC7.o \
		$(BUILD_DIR)/ms_vdp_mode_SCREEN10.o \
		$(BUILD_DIR)/ms_vdp_mode_SCREEN12.o \
		$(BUILD_DIR)/ms_disk_media.o \
		$(BUILD_DIR)/ms_disk_media_sectorbase.o \
		$(BUILD_DIR)/ms_disk_media_dskformat.o \
		$(BUILD_DIR)/ms_disk_media_9scdrv.o \
		$(BUILD_DIR)/ms_disk_container.o \
		$(BUILD_DIR)/ms_disk_drive.o \
		$(BUILD_DIR)/ms_disk_drive_floppy.o \
		$(BUILD_DIR)/ms_disk_controller_TC8566AF.o \
		$(BUILD_DIR)/ms_disk_bios_Panasonic.o \
		$(BUILD_DIR)/ms_disk_controller_WD2793.o \
		$(BUILD_DIR)/ms_disk_bios_Sony.o \
		$(BUILD_DIR)/ms_disk_9scdrv.o \
		$(BUILD_DIR)/ms_disk_9scdrv_mac.o

OBJS_DEBUG = $(BUILD_DIR)/msboot_mac.o \
		$(BUILD_DIR)/ms_d.o \
		$(BUILD_DIR)/ms_R800_30_mac_d.o \
		$(BUILD_DIR)/ms_R800_flag_mac_d.o \
		$(BUILD_DIR)/ms_iomap_d.o \
		$(BUILD_DIR)/ms_iomap_mac_d.o \
		$(BUILD_DIR)/ms_sysvalue_mac_d.o \
		$(BUILD_DIR)/ms_sub_mac_d.o \
		$(BUILD_DIR)/ms_peripherals_mac_d.o \
		$(BUILD_DIR)/ms_rtc_d.o \
		$(BUILD_DIR)/ms_psg_d.o \
		$(BUILD_DIR)/ms_psg_mac_d.o \
		$(BUILD_DIR)/ms_psg_joy_d.o \
		$(BUILD_DIR)/ms_psg_joy_mac_d.o \
		$(BUILD_DIR)/ms_kanjirom12_d.o \
		$(BUILD_DIR)/ms_kanjirom_alt_d.o \
		$(BUILD_DIR)/ms_memmap_d.o \
		$(BUILD_DIR)/ms_memmap_mac_d.o \
		$(BUILD_DIR)/ms_memmap_util_d.o \
		$(BUILD_DIR)/ms_memmap_driver_d.o \
		$(BUILD_DIR)/ms_memmap_NOTHING_d.o \
		$(BUILD_DIR)/ms_memmap_MAINRAM_d.o \
		$(BUILD_DIR)/ms_memmap_NORMALROM_d.o \
		$(BUILD_DIR)/ms_memmap_MIRROREDROM_d.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_GENERIC_8K_d.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_ASCII_8K_d.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_KONAMI_d.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_KONAMI_SCC_d.o \
		$(BUILD_DIR)/ms_memmap_PAC_d.o \
		$(BUILD_DIR)/ms_memmap_ESE_RAM_d.o \
		$(BUILD_DIR)/ms_memmap_ESE_SCC_d.o \
		$(BUILD_DIR)/ms_vdp_d.o \
		$(BUILD_DIR)/ms_vdp_mac_d.o \
		$(BUILD_DIR)/ms_vdp_sprite_d.o \
		$(BUILD_DIR)/ms_vdp_command_d.o \
		$(BUILD_DIR)/ms_vdp_mode_DEFAULT_d.o \
		$(BUILD_DIR)/ms_vdp_mode_TEXT1_d.o \
		$(BUILD_DIR)/ms_vdp_mode_TEXT2_d.o \
		$(BUILD_DIR)/ms_vdp_mode_MULTICOLOR_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC1_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC2_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC3_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC4_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC5_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC6_d.o \
		$(BUILD_DIR)/ms_vdp_mode_GRAPHIC7_d.o \
		$(BUILD_DIR)/ms_vdp_mode_SCREEN10_d.o \
		$(BUILD_DIR)/ms_vdp_mode_SCREEN12_d.o \
		$(BUILD_DIR)/ms_disk_media_d.o \
		$(BUILD_DIR)/ms_disk_media_sectorbase_d.o \
		$(BUILD_DIR)/ms_disk_media_dskformat_d.o \
		$(BUILD_DIR)/ms_disk_media_9scdrv_d.o \
		$(BUILD_DIR)/ms_disk_container_d.o \
		$(BUILD_DIR)/ms_disk_drive_d.o \
		$(BUILD_DIR)/ms_disk_drive_floppy_d.o \
		$(BUILD_DIR)/ms_disk_controller_TC8566AF_d.o \
		$(BUILD_DIR)/ms_disk_bios_Panasonic_d.o \
		$(BUILD_DIR)/ms_disk_controller_WD2793_d.o \
		$(BUILD_DIR)/ms_disk_bios_Sony_d.o \
		$(BUILD_DIR)/ms_disk_9scdrv_d.o \
		$(BUILD_DIR)/ms_disk_9scdrv_mac_d.o

all: update_version copy_to_target_all 

update_version:
	@./update_version.sh

debug: copy_to_target_debug

test: src/z80test/exe/ms_z80tests.x
	cd src/z80test/exe ; run68 ms_z80tests.x

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copy_to_target_all: copy_to_target copy_to_target_debug

copy_to_target: ${BUILD_DIR} ${BUILD_DIR}/ms.x
	cp ${BUILD_DIR}/ms.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

copy_to_target_debug: ${BUILD_DIR} ${BUILD_DIR}/ms_debug.x
	cp ${BUILD_DIR}/ms_debug.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_debug.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_debug.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

$(SRC_DIR)/ms.h: $(SRC_DIR)/../version.h
	touch $(SRC_DIR)/ms.h

${BUILD_DIR}/ms.x: $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) 

${BUILD_DIR}/ms_debug.x: $(OBJS_DEBUG)
	$(LD) -o $@ $^ $(LDFLAGS) 

# --- C ソース (全サブフォルダ共通。vpath でソースを探索) ---
${BUILD_DIR}/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: %.c $(HEADERS)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

# --- アセンブラソース (CP932 シャドウツリーを作ってからアセンブル) ---
# 全アセンブラソースを CP932 に変換して $(ASMSRC_DIR)/ にツリー複製する
$(ASM_STAMP): $(ASM_SOURCES)
	@rm -rf $(ASMSRC_DIR)
	@set -e; for f in $(ASM_SOURCES); do \
		mkdir -p "$(ASMSRC_DIR)/$$(dirname $$f)"; \
		$(UTF8TOCP932) "$$f" > "$(ASMSRC_DIR)/$$f"; \
	done
	@mkdir -p $(ASMSRC_DIR)
	@touch $@

# シャドウツリー内でアセンブルする (cd するので include の相対解決も従来どおり)
${BUILD_DIR}/%_mac.o: %.has $(ASM_STAMP)
	cd $(ASMSRC_DIR) && $(AS) $(ASFLAGS) $< -o $(notdir $@).tmp
	x68k2elf.py $(ASMSRC_DIR)/$(notdir $@).tmp $@
	rm $(ASMSRC_DIR)/$(notdir $@).tmp

${BUILD_DIR}/%_mac_d.o: %.has $(ASM_STAMP)
	cd $(ASMSRC_DIR) && $(AS) $(ASFLAGS_DEBUG) $< -o $(notdir $@).tmp
	x68k2elf.py $(ASMSRC_DIR)/$(notdir $@).tmp $@
	rm $(ASMSRC_DIR)/$(notdir $@).tmp

# Test program
test/z80test/exe/ms_z80tests.x:
	make -C test/z80test

clean:
	rm -rf $(BUILD_DIR)
	make -C test/z80test clean
