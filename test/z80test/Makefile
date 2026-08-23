REPO_DIR = ../..
SRC_DIR = .
MS_SRC_DIR = $(REPO_DIR)/src
BUILD_DIR = build
EXE_DIR = exe

# 環境依存の設定はルートの Makefile と同じ仕組みで読み込む (詳細はそちらのコメント参照)
-include $(HOME)/.human68k.mk
-include $(REPO_DIR)/local.mk

HAS060 ?= $(HOME)/work/XEiJ/HFS/prog/HAS/HAS060.X
INSTALL_DIR ?=

CROSS = m68k-xelf-
CC = $(CROSS)gcc
AS = run68 $(HAS060)
LD = $(CROSS)gcc

CFLAGS = -g -c -m68000 -finput-charset=UTF-8 -fexec-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

ASFLAGS = -d -i src -i src/include -w0

# アセンブラソースは UTF-8 で管理し、HAS060 が要求する SJIS(CP932) に変換した
# シャドウツリー ($(ASMSRC_DIR)) を作ってからアセンブルする (ルートの Makefile と同じ方式)。
# .include はリポジトリルート基準で解決されるため、ディレクトリ構造ごと複製する。
# (macOS の iconv は CP932 変換が不安定なため Python の cp932 コーデックを使用)
UTF8TOCP932 = python3 -c 'import sys;sys.stdout.buffer.write(open(sys.argv[1],encoding="utf-8").read().encode("cp932"))'
ASMSRC_DIR = $(BUILD_DIR)/asmsrc
ASM_SOURCES = $(shell cd $(REPO_DIR) && find src test/z80test -type f \( -name '*.has' -o -name '*.mac' -o -name '*.MAC' -o -name '*.equ' \))
ASM_DEPS = $(addprefix $(REPO_DIR)/,$(ASM_SOURCES))
ASM_STAMP = $(ASMSRC_DIR)/.stamp

all: copy_to_target_all

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copy_to_target_all: copy_to_target

copy_to_target: ${BUILD_DIR} ${BUILD_DIR}/ms_z80_tests.x ${BUILD_DIR}/ms_z80_tests_cmp.x ${BUILD_DIR}/ms_z80_cmptests.x
	cp ${BUILD_DIR}/ms_z80_tests.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_z80_tests.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_z80_tests_cmp.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_z80_tests_cmp.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_z80_cmptests.x ${EXE_DIR}/
	$(if $(INSTALL_DIR),cp ${BUILD_DIR}/ms_z80_tests.x $(INSTALL_DIR)/)
	$(if $(INSTALL_DIR),cp ${BUILD_DIR}/ms_z80_tests_cmp.x $(INSTALL_DIR)/)
	$(if $(INSTALL_DIR),cp ${BUILD_DIR}/ms_z80_cmptests.x $(INSTALL_DIR)/)

${BUILD_DIR}/ms_z80_tests.x: $(BUILD_DIR)/ms_z80_wrapper.o $(BUILD_DIR)/ms_z80_tests.o $(BUILD_DIR)/ms_R800_30.o $(BUILD_DIR)/ms_R800_flag.o ${BUILD_DIR}/ms_z80_test_iomap.o ${BUILD_DIR}/ms_z80_test_memmap.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/ms_z80_tests_cmp.x: $(BUILD_DIR)/ms_z80_wrapper.o $(BUILD_DIR)/ms_z80_tests_cmp.o $(BUILD_DIR)/ms_R800_30.o $(BUILD_DIR)/ms_R800_flag.o ${BUILD_DIR}/ms_z80_test_iomap.o ${BUILD_DIR}/ms_z80_test_memmap.o $(BUILD_DIR)/z80.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/ms_z80_cmptests.x: $(BUILD_DIR)/ms_z80_cmptests.o $(BUILD_DIR)/ms_z80_cmptests_sz.o $(BUILD_DIR)/ms_z80_cmptests_ms.o $(BUILD_DIR)/ms_z80_wrapper.o  $(BUILD_DIR)/ms_R800_30.o $(BUILD_DIR)/ms_R800_flag.o ${BUILD_DIR}/ms_z80_test_iomap.o ${BUILD_DIR}/ms_z80_test_memmap.o $(BUILD_DIR)/z80.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/%.o: $(SRC_DIR)/%.c $(MS_SRC_DIR)/ms_R800.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/superzazu-z80/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# --- アセンブラソース (CP932 シャドウツリーを作ってからアセンブル) ---
# 全アセンブラソースを CP932 に変換して $(ASMSRC_DIR)/ にツリー複製する
$(ASM_STAMP): $(ASM_DEPS)
	@rm -rf $(ASMSRC_DIR)
	@set -e; for f in $(ASM_SOURCES); do \
		mkdir -p "$(ASMSRC_DIR)/$$(dirname $$f)"; \
		$(UTF8TOCP932) "$(REPO_DIR)/$$f" > "$(ASMSRC_DIR)/$$f"; \
	done
	@mkdir -p $(ASMSRC_DIR)
	@touch $@

# HAS060.X のパスが違うと HAS の usage が延々流れるだけで原因が分かりにくいため、
# アセンブルの前に存在確認する
.PHONY: check-has060
check-has060:
	@test -f "$(HAS060)" || { \
		echo "エラー: HAS060.X が見つかりません: $(HAS060)"; \
		echo "       local.mk (local.mk.example を参照) か環境変数 HAS060 で指定してください"; \
		exit 1; \
	}

# シャドウツリー内でアセンブルする (cd するので include の相対解決も従来どおり)
${BUILD_DIR}/%.o: $(SRC_DIR)/%.has $(ASM_STAMP) | $(BUILD_DIR) check-has060
	cd $(ASMSRC_DIR) && $(AS) $(ASFLAGS) test/z80test/$*.has -o $(notdir $@).tmp
	x68k2elf.py $(ASMSRC_DIR)/$(notdir $@).tmp $@
	rm $(ASMSRC_DIR)/$(notdir $@).tmp

${BUILD_DIR}/%.o: $(MS_SRC_DIR)/%.has $(ASM_STAMP) | $(BUILD_DIR) check-has060
	cd $(ASMSRC_DIR) && $(AS) $(ASFLAGS) src/$*.has -o $(notdir $@).tmp
	x68k2elf.py $(ASMSRC_DIR)/$(notdir $@).tmp $@
	rm $(ASMSRC_DIR)/$(notdir $@).tmp

clean:
	rm -rf $(BUILD_DIR)
