APP_NAME ?= loader

PROJ_FILES = ../
BIN_NAME = $(APP_NAME).bin
HEX_NAME = $(APP_NAME).hex
ELF_NAME = $(APP_NAME).elf

IMAGE_TYPE = IMAGE_TYPE4
VERSION = 1
#############################

-include $(PROJ_FILES)/m_config.mk
-include $(PROJ_FILES)/m_generic.mk

# use an app-specific build dir
APP_BUILD_DIR = $(BUILD_DIR)/$(APP_NAME)

CFLAGS += $(KERN_CFLAGS)
CFLAGS += $(LIBSIGN_CFLAGS) -I$(PROJ_FILES)/externals/libecc/src
CFLAGS += -Isrc/ -Iinc/ -Isrc/arch -Isrc/arch/cores/$(ARCH) -Isrc/arch/socs/$(SOC)
CFLAGS += -MMD -MP

LDFLAGS := -Tloader.ld $(AFLAGS) -fno-builtin -nostdlib -nostartfiles -Wl,-Map=$(APP_BUILD_DIR)/$(APP_NAME).map
LD_LIBS += -lsign -L$(APP_BUILD_DIR) -L$(BUILD_DIR)/externals

BUILD_DIR ?= $(PROJ_FILE)build

CSRC_DIR = src
SRC := $(wildcard $(CSRC_DIR)/*.c)
OBJ := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(SRC))

SOC_DIR = src/arch/socs/$(SOC)
SOC_SRC := $(wildcard $(SOC_DIR)/*.c)
SOC_OBJ := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(SOC_SRC))

ARCH_DIR = src/arch/cores/$(ARCH)
ARCH_SRC := $(wildcard $(ARCH_DIR)/*.c)
ARCH_OBJ := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(ARCH_SRC))

SOCASM_DIR := src/arch/socs/$(SOC)
SOCASM_SRC := startup_$(SOC).s
SOCASM_OBJ := $(patsubst %.s,$(APP_BUILD_DIR)/asm/%.o,$(SOCASM_SRC))

OUT_DIRS = $(dir $(SOCASM_OBJ)) $(dir $(SOC_OBJ)) $(dir $(ARCH_OBJ)) $(dir $(OBJ))

LDSCRIPT_NAME = $(APP_BUILD_DIR)/$(APP_NAME).ld

# file to (dist)clean
# objects and compilation related
TODEL_CLEAN += $(OBJ) $(SOC_OBJ) $(SOCASM_OBJ) $(ARCH_OBJ) $(DEP) $(SOC_DEP) $(ARCH_DEP)
# targets
TODEL_DISTCLEAN += $(APP_BUILD_DIR)

.PHONY: loader __clean

__clean:
	-rm -rf $(TODEL_CLEAN)

__distclean:
	-rm -rf $(LDSCRIPT_NAME) $(APP_BUILD_DIR)/$(ELF_NAME) $(APP_BUILD_DIR)/$(HEX_NAME) $(APP_BUILD_DIR)/$(BIN_NAME)


all: $(APP_BUILD_DIR) loader

show:
	@echo
	@echo "\t\tAPP_BUILD_DIR\t=> " $(APP_BUILD_DIR)
	@echo
	@echo "C sources files:"
	@echo "\t\tSRC\t=> " $(SRC)
	@echo "\t\tSOC_SRC\t=> " $(SOC_SRC)
	@echo "\t\tARCH_SRC\t=> " $(ARCH_SRC)
	@echo
	@echo "\t\tOBJ\t=> " $(OBJ)
	@echo "\t\tSOC_OBJ\t=> " $(SOC_OBJ)
	@echo "\t\tARCH_OBJ\t=> " $(ARCH_OBJ)
	@echo
	@echo "\t\tBUILD_DIR\t=> " $(BUILD_DIR)
	@echo "\t\tAPP_BUILD_DIR\t=> " $(APP_BUILD_DIR)

loader: $(APP_BUILD_DIR)/$(ELF_NAME) $(APP_BUILD_DIR)/$(HEX_NAME)

#############################################################
# build targets (driver, core, SoC, Board... and local)
# App C sources files
$(APP_BUILD_DIR)/src/%.o: src/%.c
	$(call if_changed,cc_o_c)

# only for ASM startup file
$(APP_BUILD_DIR)/asm/%.o: $(SOCASM_DIR)/$(SOCASM_SRC)
	$(call if_changed,cc_o_c)

$(APP_BUILD_DIR)/arch/socs/$(SOC)/%.o: arch/socs/$(SOC)/%.c
	$(call if_changed,cc_o_c)

$(APP_BUILD_DIR)/arch/cores/$(ARCH)/%.o: arch/cores/$(ARCH)/%.c
	$(call if_changed,cc_o_c)

# Test sources files
$(APP_BUILD_DIR)/tests/%.o: $(TESTSSRC_DIR)/%.c
	$(call if_changed,cc_o_c)

# LDSCRIPT
ifeq (y, $(CONFIG_FIRMWARE_MODE_MONO_BANK))
$(LDSCRIPT_NAME): loader.monobank.ld.in
	$(call if_changed,k_ldscript)
else
$(LDSCRIPT_NAME): loader.dualbank.ld.in
	$(call if_changed,k_ldscript)
endif

# ELF
$(APP_BUILD_DIR)/$(ELF_NAME): $(LDSCRIPT_NAME) $(OBJ) $(ARCH_OBJ) $(SOC_OBJ) $(SOCASM_OBJ)
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
