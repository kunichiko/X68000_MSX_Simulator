CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/HAS/HAS060.X
LD = $(CROSS)gcc

CFLAGS = -g -c -m68000 -finput-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

#GCC_OPTS = -c -O -g -finput-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

ASFLAGS = -d -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -i ../../ -w0

SRC_DIR = .
BUILD_DIR = build
EXE_DIR = exe

all: copy_to_target_all

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copy_to_target_all: copy_to_target

copy_to_target: ${BUILD_DIR} ${BUILD_DIR}/ms_z80_tests.x ${BUILD_DIR}/ms_z80_cmptests.x
	cp ${BUILD_DIR}/ms_z80_tests.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_z80_tests.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_z80_tests.x /Users/ohnaka/work/XEiJ/HFS/MS.X/
	cp ${BUILD_DIR}/ms_z80_cmptests.x ${EXE_DIR}/

${BUILD_DIR}/ms_z80_tests.x: $(BUILD_DIR)/ms_z80_wrapper.o $(BUILD_DIR)/ms_z80_tests.o $(BUILD_DIR)/ms_R800_mac_30.o ${BUILD_DIR}/ms_z80_test_iomap.o ${BUILD_DIR}/ms_z80_test_memmap.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/ms_z80_cmptests.x: $(BUILD_DIR)/ms_z80_cmptests.o $(BUILD_DIR)/ms_z80_cmptests_sz.o $(BUILD_DIR)/ms_z80_cmptests_ms.o $(BUILD_DIR)/ms_z80_wrapper.o  $(BUILD_DIR)/ms_R800_mac_30.o ${BUILD_DIR}/ms_z80_test_iomap.o ${BUILD_DIR}/ms_z80_test_memmap.o $(BUILD_DIR)/z80.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/../../src/ms_R800.h
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/superzazu-z80/%.c
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%.o: $(SRC_DIR)/../src/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

clean:
	rm -rf $(BUILD_DIR)
