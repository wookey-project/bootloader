APP_NAME ?= loader

PROJ_FILES = ../
BIN_NAME = $(APP_NAME).bin
HEX_NAME = $(APP_NAME).hex
ELF_NAME = $(APP_NAME).elf

IMAGE_TYPE = IMAGE_TYPE4
VERSION = 1
#############################

-include $(PROJ_FILES)/Makefile.conf
-include $(PROJ_FILES)/Makefile.gen

# libbsp includes
-include $(PROJ_FILES)/kernel/arch/socs/$(SOC)/Makefile.objs
-include $(PROJ_FILES)/kernel/arch/cores/$(ARCH)/Makefile.objs
-include $(PROJ_FILES)/kernel/arch/boards/Makefile.objs

# use an app-specific build dir
APP_BUILD_DIR = $(BUILD_DIR)/$(APP_NAME)

CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += $(DEBUG_CFLAGS)
CFLAGS += $(LIBSIGN_CFLAGS)
CFLAGS += -ffreestanding
CFLAGS += -I$(PROJ_FILES)/kernel/shared
CFLAGS += -Isrc/ -Iinc/ -I$(PROJ_FILES)/kernel/arch -I$(PROJ_FILES)/kernel -I$(PROJ_FILES)/externals/libecc/src
CFLAGS += -MMD -MP

LDFLAGS += -Tloader.ld -L$(APP_BUILD_DIR) $(AFLAGS) -fno-builtin -nostdlib -nostartfiles
LD_LIBS += $(APP_BUILD_DIR)/libbsp/libbsp.a $(LIBSIGN) -L$(APP_BUILD_DIR) -L$(APP_BUILD_DIR)/libbsp

BUILD_DIR ?= $(PROJ_FILE)build

CSRC_DIR = src
SRC := $(wildcard $(CSRC_DIR)/*.c)
OBJ := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(SRC))

# startup file, for kernel and loader only
SOC_DIR = $(PROJ_FILES)/kernel/arch/socs/$(SOC)
SOC_SRC := startup_$(SOC).s
SOC_OBJ := $(patsubst %.s,$(APP_BUILD_DIR)/%.o,$(SOC_SRC))

#Rust sources files
RSSRC_DIR=rust/src
RSRC= $(wildcard $(RSRCDIR)/*.rs)
ROBJ = $(patsubst %.rs,$(APP_BUILD_DIR)/%.o,$(RSRC))

#ada sources files
ASRC_DIR = ada/src
ASRC= $(wildcard $(ASRC_DIR)/*.adb)
AOBJ = $(patsubst %.adb,$(APP_BUILD_DIR)/%.o,$(ASRC))

#test sources files
TESTSSRC_DIR = tests
TESTSSRC = tests.c tests_cryp.c tests_dma.c tests_queue.c tests_sd.c tests_systick.c
TESTSOBJ = $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(TESTSSRC))
TESTSDEP = $(TESTSSOBJ:.o=.d)

OUT_DIRS = $(dir $(DRVOBJ)) $(dir $(BOARD_OBJ)) $(dir $(SOC_OBJ)) $(dir $(CORE_OBJ)) $(dir $(AOBJ)) $(dir $(OBJ)) $(dir $(ROBJ))

LDSCRIPT_NAME = $(APP_BUILD_DIR)/$(APP_NAME).ld

# file to (dist)clean
# objects and compilation related
TODEL_CLEAN += $(ROBJ) $(OBJ) $(SOC_OBJ) $(DRVOBJ) $(BOARD_OBJ) $(CORE_OBJ) $(DEP) $(TESTSDEP) $(SOC_DEP) $(DRVDEP) $(BOARD_DEP) $(CORE_DEP) $(LDSCRIPT_NAME)
# targets
TODEL_DISTCLEAN += $(APP_BUILD_DIR)

.PHONY: loader __clean

__clean:
	-rm -rf $(OBJ)

__distclean:
	-rm -rf $(LDSCRIPT_NAME) $(APP_BUILD_DIR)/$(ELF_NAME) $(APP_BUILD_DIR)/$(HEX_NAME) $(APP_BUILD_DIR)/$(BIN_NAME)


all: $(APP_BUILD_DIR) loader

show:
	@echo
	@echo "\t\tAPP_BUILD_DIR\t=> " $(APP_BUILD_DIR)
	@echo
	@echo "C sources files:"
	@echo "\t\tSRC\t=> " $(SRC)
	@echo "\t\tOBJ\t=> " $(OBJ)
	@echo
	@echo "\t\tBUILD_DIR\t=> " $(BUILD_DIR)
	@echo "\t\tAPP_BUILD_DIR\t=> " $(APP_BUILD_DIR)
	@echo
	@echo "Rust sources files:"
	@echo "\t" $(RSRC)
	@echo "\t\t=> " $(ROBJ)

loader: $(APP_BUILD_DIR)/$(ELF_NAME) $(APP_BUILD_DIR)/$(HEX_NAME)

#############################################################
# build targets (driver, core, SoC, Board... and local)
# App C sources files
$(APP_BUILD_DIR)/%.o: %.c
	$(call if_changed,cc_o_c)

# Test sources files
$(APP_BUILD_DIR)/tests/%.o: $(TESTSSRC_DIR)/%.c
	$(call if_changed,cc_o_c)

$(APP_BUILD_DIR)/%.o: $(SOC_DIR)/$(SOC_SRC)
	$(call if_changed,cc_o_c)

# RUST FILES
$(ROBJ): $(RSRC)
	$(call if_changed,rc_o_rs)

# LDSCRIPT
$(LDSCRIPT_NAME):
	$(call if_changed,k_ldscript)

# ELF
$(APP_BUILD_DIR)/$(ELF_NAME): $(LDSCRIPT_NAME) $(ROBJ) $(OBJ) $(SOBJ) $(SOC_OBJ)
	$(call if_changed,link_o_target)

# HEX
$(APP_BUILD_DIR)/$(HEX_NAME): $(APP_BUILD_DIR)/$(ELF_NAME)
	$(call if_changed,objcopy_ihex)

# BIN
$(APP_BUILD_DIR)/$(BIN_NAME): $(APP_BUILD_DIR)/$(ELF_NAME)
	$(call if_changed,objcopy_bin)

$(APP_BUILD_DIR):
	$(call cmd,mkdir)

# TEST TARGETS
tests_suite: CFLAGS += -Itests/ -DTESTS
tests_suite: $(TESTSOBJ) $(ROBJ) $(OBJ) $(SOBJ) $(DRVOBJ)

tests: clean tests_suite
	$(CC) $(LDFLAGS) -o $(APP_NAME).elf $(ROBJ) $(SOBJ) $(OBJ) $(DRVOBJ) $(TESTSOBJ)
	$(GDB) -x gdbfile_run $(APP_NAME).elf

-include $(DEP)
-include $(DRVDEP)
-include $(TESTSDEP)
