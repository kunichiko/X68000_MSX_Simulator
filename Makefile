SRC_DIR = src
VDP_DIR = $(SRC_DIR)/vdp
MEMMAP_DIR = $(SRC_DIR)/memmap
BUILD_DIR = build
EXE_DIR = exe

CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/HAS/HAS060.X
LD = $(CROSS)gcc

#CFLAGS = -g -std=gnu90 -c -m68000 -O -finput-charset=CP932
# なぜか最適化をかけるとかなり変な動きをするので-Oを外す
CFLAGS = -g -std=gnu90 -c -m68000 -finput-charset=CP932
CFLAGS_DEBUG = -g -std=gnu90 -c -m68000 -finput-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

#GCC_OPTS = -c -O -g -finput-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

ASFLAGS = -i $(SRC_DIR) -i $(VDP_DIR) -i $(MEMMAP_DIR) -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -w0
ASFLAGS_DEBUG = -d -s DEBUG -i $(SRC_DIR) -i $(VDP_DIR) -i $(MEMMAP_DIR) -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -w0

# オブジェクトファイルのリストを変数にまとめる
OBJS = $(BUILD_DIR)/ms.o \
		$(BUILD_DIR)/ms_R800_mac_30.o \
		$(BUILD_DIR)/ms_R800_flag.o \
		$(BUILD_DIR)/ms_iomap.o \
		$(BUILD_DIR)/ms_iomap_mac.o \
		$(BUILD_DIR)/ms_sysvalue.o \
		$(BUILD_DIR)/ms_sub_mac.o \
		$(BUILD_DIR)/ms_peripherals.o \
		$(BUILD_DIR)/ms_PSG_mac.o \
		$(BUILD_DIR)/ms_readcart.o \
		$(BUILD_DIR)/ms_memmap.o \
		$(BUILD_DIR)/ms_memmap_mac.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_8K.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_KONAMI.o \
		$(BUILD_DIR)/ms_vdp.o \
		$(BUILD_DIR)/ms_vdp_mac.o \
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
		$(BUILD_DIR)/ms_vdp_mode_SCREEN12.o

OBJS_DEBUG = $(BUILD_DIR)/ms_d.o \
		$(BUILD_DIR)/ms_R800_mac_30_d.o \
		$(BUILD_DIR)/ms_R800_flag_d.o \
		$(BUILD_DIR)/ms_iomap_d.o \
		$(BUILD_DIR)/ms_iomap_mac_d.o \
		$(BUILD_DIR)/ms_sysvalue_d.o \
		$(BUILD_DIR)/ms_sub_mac_d.o \
		$(BUILD_DIR)/ms_peripherals_d.o \
		$(BUILD_DIR)/ms_PSG_mac_d.o \
		$(BUILD_DIR)/ms_readcart_d.o \
		$(BUILD_DIR)/ms_memmap_d.o \
		$(BUILD_DIR)/ms_memmap_mac_d.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_8K_d.o \
		$(BUILD_DIR)/ms_memmap_MEGAROM_KONAMI_d.o \
		$(BUILD_DIR)/ms_vdp_d.o \
		$(BUILD_DIR)/ms_vdp_mac_d.o \
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
		$(BUILD_DIR)/ms_vdp_mode_SCREEN12_d.o

all: copy_to_target_all

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

${BUILD_DIR}/ms.x: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/ms_debug.x: $(OBJS_DEBUG)
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/%_d.o: $(SRC_DIR)/%.c $(SRC_DIR)/ms_R800.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%_d.o: $(SRC_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/ms_R800.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

# VDP files
${BUILD_DIR}/%.o: $(VDP_DIR)/%.c $(VDP_DIR)/ms_vdp.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(VDP_DIR)/%.c $(VDP_DIR)/ms_vdp.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(VDP_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%_d.o: $(VDP_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

# Memmap files
${BUILD_DIR}/%.o: $(MEMMAP_DIR)/%.c $(MEMMAP_DIR)/ms_memmap.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(MEMMAP_DIR)/%.c $(MEMMAP_DIR)/ms_memmap.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(MEMMAP_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%_d.o: $(MEMMAP_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

# Test program
test/z80test/exe/ms_z80tests.x:
	make -C test/z80test

clean:
	rm -rf $(BUILD_DIR)
	make -C test/z80test clean
