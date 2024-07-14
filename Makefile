CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/HAS/HAS060.X
LD = $(CROSS)gcc

CFLAGS = -g -std=gnu90 -c -m68000 -O2 -finput-charset=CP932
CFLAGS_DEBUG = -g -std=gnu90 -c -m68000 -finput-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

#GCC_OPTS = -c -O -g -finput-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

ASFLAGS = -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -w0

SRC_DIR = src
BUILD_DIR = build
EXE_DIR = exe

all: copy_to_target_all

debug: copy_to_target_debug

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copy_to_target_all: copy_to_target copy_to_target_debug

copy_to_target: ${BUILD_DIR} ${BUILD_DIR}/ms.x
	cp ${BUILD_DIR}/ms.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

copy_to_target_debug: ${BUILD_DIR} ${BUILD_DIR}/ms_debug.x
	cp ${BUILD_DIR}/ms_debug.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_debug.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

${BUILD_DIR}/ms.x: $(BUILD_DIR)/ms.o $(BUILD_DIR)/ms_R800_mac_30.o $(BUILD_DIR)/ms_vdp_mac.o $(BUILD_DIR)/ms_sysvalue.o $(BUILD_DIR)/ms_sub_mac.o $(BUILD_DIR)/ms_IO_PORT.o $(BUILD_DIR)/ms_PSG_mac.o $(BUILD_DIR)/ms_readcart.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/ms_debug.x: $(BUILD_DIR)/ms_debug.o $(BUILD_DIR)/ms_R800_mac_30.o $(BUILD_DIR)/ms_vdp_mac.o $(BUILD_DIR)/ms_sysvalue.o $(BUILD_DIR)/ms_sub_mac.o $(BUILD_DIR)/ms_IO_PORT.o $(BUILD_DIR)/ms_PSG_mac.o $(BUILD_DIR)/ms_readcart.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/%_debug.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

#ms_R800_mac_30.o: ms_R800_mac_30.has
#	m68k-xelf-as $(AS_OPTS) ms_R800_mac_30.has

#ms_vdp_mac.o: ms_vdp_mac.has
#	m68k-xelf-as $(AS_OPTS) ms_vdp_mac.has

#ms_sysvalue.o: ms_sysvalue.has
#	m68k-xelf-as $(AS_OPTS) ms_sysvalue.has

#ms_sub_mac.o: ms_sub_mac.has
#	m68k-xelf-as $(AS_OPTS) ms_sub_mac.has

#ms_IO_port.o: ms_IO_port.has
#	m68k-xelf-as $(AS_OPTS) ms_IO_port.has

#ms_PSG_mac.o: ms_PSG_mac.has
#	m68k-xelf-as $(AS_OPTS) ms_PSG_mac.has

#ms_readcart.o: ms_readcart.c
#	$(CC) $(GCC_OPTS) ms_readcart.c

clean:
	rm -rf $(BUILD_DIR)
