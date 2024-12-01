SRC_DIR = src
VDP_DIR = $(SRC_DIR)/vdp
MEMMAP_DIR = $(SRC_DIR)/memmap
DISK_DIR = $(SRC_DIR)/disk
PERIPHERAL_DIR = $(SRC_DIR)/peripheral
ASMINC_DIR = $(SRC_DIR)/include
BUILD_DIR = build
EXE_DIR = exe

CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/HAS/HAS060.X
LD = $(CROSS)gcc

CFLAGS = -g -std=gnu90 -c -m68000 -O2 -finput-charset=CP932
CFLAGS_DEBUG = -g -std=gnu90 -c -m68000 -DDEBUG -finput-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

#GCC_OPTS = -c -O -g -finput-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

-include ~/.human68k.mk

ASFLAGS = -i $(SRC_DIR) -i $(ASMINC_DIR) -i $(VDP_DIR) -i $(MEMMAP_DIR) -i ${DISK_DIR} -i ${PERIPHERAL_DIR} -w0
ASFLAGS_DEBUG = -d -s DEBUG $(ASFLAGS)

# オブジェクトファイルのリストを変数にまとめる
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/ms.o \
		$(BUILD_DIR)/ms_R800_mac_30.o \
		$(BUILD_DIR)/ms_R800_flag.o \
		$(BUILD_DIR)/ms_iomap.o \
		$(BUILD_DIR)/ms_iomap_mac.o \
		$(BUILD_DIR)/ms_sysvalue.o \
		$(BUILD_DIR)/ms_sub_mac.o \
		$(BUILD_DIR)/ms_peripherals.o \
		$(BUILD_DIR)/ms_rtc.o \
		$(BUILD_DIR)/ms_psg.o \
		$(BUILD_DIR)/ms_psg_mac.o \
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
		$(BUILD_DIR)/ms_disk_container.o \
		$(BUILD_DIR)/ms_disk_drive.o \
		$(BUILD_DIR)/ms_disk_drive_floppy.o \
		$(BUILD_DIR)/ms_disk_controller_TC8566AF.o \
		$(BUILD_DIR)/ms_disk_bios_Panasonic.o

OBJS_DEBUG = $(BUILD_DIR)/main_d.o $(BUILD_DIR)/ms_d.o \
		$(BUILD_DIR)/ms_R800_mac_30_d.o \
		$(BUILD_DIR)/ms_R800_flag_d.o \
		$(BUILD_DIR)/ms_iomap_d.o \
		$(BUILD_DIR)/ms_iomap_mac_d.o \
		$(BUILD_DIR)/ms_sysvalue_d.o \
		$(BUILD_DIR)/ms_sub_mac_d.o \
		$(BUILD_DIR)/ms_peripherals_d.o \
		$(BUILD_DIR)/ms_rtc_d.o \
		$(BUILD_DIR)/ms_psg_d.o \
		$(BUILD_DIR)/ms_psg_mac_d.o \
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
		$(BUILD_DIR)/ms_disk_container_d.o \
		$(BUILD_DIR)/ms_disk_drive_d.o \
		$(BUILD_DIR)/ms_disk_drive_floppy_d.o \
		$(BUILD_DIR)/ms_disk_controller_TC8566AF_d.o \
		$(BUILD_DIR)/ms_disk_bios_Panasonic_d.o

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

${BUILD_DIR}/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/ms_R800.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(SRC_DIR)/%.c $(SRC_DIR)/ms_R800.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%_d.o: $(SRC_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS) -m68000 $< -o $@

${BUILD_DIR}/main_d.o: $(SRC_DIR)/main.c $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS_DEBUG) -m68000 $< -o $@


# VDP files
${BUILD_DIR}/%.o: $(VDP_DIR)/%.c $(VDP_DIR)/ms_vdp.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(VDP_DIR)/%.c $(VDP_DIR)/ms_vdp.h $(SRC_DIR)/ms.h
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
${BUILD_DIR}/%.o: $(MEMMAP_DIR)/%.c $(MEMMAP_DIR)/ms_memmap.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(MEMMAP_DIR)/%.c $(MEMMAP_DIR)/ms_memmap.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(MEMMAP_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%_d.o: $(MEMMAP_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

# Disk files
${BUILD_DIR}/%.o: $(DISK_DIR)/%.c $(DISK_DIR)/ms_disk.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(DISK_DIR)/%.c $(DISK_DIR)/ms_disk.h $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(DISK_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%_d.o: $(DISK_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

# Peripheral files
${BUILD_DIR}/%.o: $(PERIPHERAL_DIR)/%.c $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%_d.o: $(PERIPHERAL_DIR)/%.c $(SRC_DIR)/ms.h
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(PERIPHERAL_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%_d.o: $(PERIPHERAL_DIR)/%.has $(SRC_DIR)/ms.mac
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

# Test program
test/z80test/exe/ms_z80tests.x:
	make -C test/z80test

clean:
	rm -rf $(BUILD_DIR)
	make -C test/z80test clean
