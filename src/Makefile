CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/HAS/HAS060.X
LD = $(CROSS)gcc

CFLAGS = -c -m68000 -O2 -finput-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

#GCC_OPTS = -c -O -g -finput-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

ASFLAGS = -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -w0

all: ms.x copy_to_target

copy_to_target:
	cp ms.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

ms.x: ms.o ms_R800_mac_30.o ms_vdp_mac.o ms_sysvalue.o ms_sub_mac.o ms_IO_PORT.o ms_PSG_mac.o ms_readcart.o
	$(LD) $(LDFLAGS) -o ms.x ms.o ms_R800_mac_30.o ms_vdp_mac.o ms_sysvalue.o ms_sub_mac.o ms_IO_port.o ms_PSG_mac.o ms_readcart.o

%.o: %.c
	$(CC) $(CFLAGS) $<

%.o: %.has
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
	rm -f *.o ms.x