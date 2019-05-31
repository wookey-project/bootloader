/*
 * Copyright 2019 The wookey project team <wookey@ssi.gouv.fr>
 *   - Ryad     Benadjila
 *   - Arnauld  Michelizza
 *   - Mathieu  Renard
 *   - Philippe Thierry
 *   - Philippe Trebuchet
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of mosquitto nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/** @file main.c
 *
 */
#include "hash.h"
#include "autoconf.h"
#ifdef CONFIG_ARCH_CORTEX_M4
#include "m4-systick.h"
#else
#error "no systick support for other by now!"
#endif
#include "debug.h"
#include "m4-systick.h"
#include "m4-core.h"
#include "m4-cpu.h"
#include "soc-init.h"
#include "soc-usart.h"
#include "soc-usart-regs.h"
#include "soc-gpio.h"
#include "soc-nvic.h"
#include "soc-rcc.h"
#include "soc-interrupts.h"
#include "boot_mode.h"
#include "shr.h"
#include "crc32.h"
#include "gpio.h"
#include "types.h"
#include "flash.h"

#define COLOR_NORMAL  "\033[0m"
#define COLOR_REVERSE "\033[7m"
#define COLOR_REDBG   "\033[41m"

/**
 *  Ref DocID022708 Rev 4 p.141
 *  BX/BLX cause usageFault if bit[0] of Rm is O
 *  Rm is a reg that indicates addr to branch to.
 *  Bit[0] of the value in Rm must be 1, BUT the addr
 *  to branch to is created by changing bit[0] to 0
 */
//app_entry_t ldr_main = (app_entry_t)(_ldr_base+1);
app_entry_t fw1_main = (app_entry_t) (FW1_START);
app_entry_t dfu1_main = (app_entry_t) (DFU1_START);
#ifdef CONFIG_FIRMWARE_DUALBANK
app_entry_t fw2_main = (app_entry_t) (FW2_START);
app_entry_t dfu2_main = (app_entry_t) (DFU2_START);
#endif

#ifdef CONFIG_FIRMWARE_DFU
dev_gpio_info_t gpio = { 0 };

/* the DFU button GPIO configuration may vary depending on the board
 * by now, both Wookeyv1 & Wookeyv2 hold the DFU button on GPIO PE4 */
# ifdef CONFIG_WOOKEY
#  define DFU_GPIO_PORT GPIO_PE
#  define DFU_GPIO_PIN  4
# else
#  error "Unknown DFU button GPIO configuration! please set it first"
# endif

#endif

volatile secbool dfu_mode = secfalse;

void hexdump(const uint8_t *bin, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++) {
    dbg_log("%x ", bin[i]);
    if ((i % 16 == 0) && (i != 0)) {
      dbg_log("\n");
    }
    dbg_flush();
  }
  dbg_log("\n");
  dbg_flush();
}

void dump_fw_header(const t_firmware_state *fw)
{
    dbg_flush();
    dbg_log("Magic    :  %x\n", fw->fw_sig.magic);
    dbg_log("Version  :  %x\n", fw->fw_sig.version);
    dbg_log("Siglen   :  %x\n", fw->fw_sig.siglen);
    dbg_log("Len      :  %x\n", fw->fw_sig.len);
    dbg_log("Chunksize:  %x\n", fw->fw_sig.chunksize);
    dbg_log("Sig      :\n");
    dbg_flush();
    if (fw->fw_sig.siglen) {
        hexdump(fw->fw_sig.sig, fw->fw_sig.siglen);
    } else {
        hexdump(fw->fw_sig.sig, EC_MAX_SIGLEN);
    }
    dbg_flush();
    dbg_log("Crc32    :  %x\n", fw->fw_sig.crc32);
    dbg_log("Bash     :\n");
        hexdump(fw->fw_sig.hash, SHA256_DIGEST_SIZE);
    dbg_log("Bootable :  %x\n", fw->bootable);
    dbg_flush();
}

extern const shr_vars_t flip_shared_vars;
#ifdef CONFIG_FIRMWARE_DUALBANK
extern const shr_vars_t flop_shared_vars;
#endif
    volatile uint32_t count = 2;

/* NOTE: O0 for fault attacks protections */
#ifdef __GNUC__
#ifdef __clang__
# pragma clang optimize off
#else
# pragma GCC push_options
# pragma GCC optimize("O0")
#endif
#endif
/*
 * We use the local -fno-stack-protector flag for main because
 * the stack protection has not been initialized yet.
 */
#ifdef __clang__
/* FIXME */
#else
__attribute__ ((optimize("-fno-stack-protector")))
#endif
int main(void)
{
    disable_irq();

    /* init MSP for loader */
//    asm volatile ("msr msp, %0\n\t" : : "r" (0x20020000) : "r2");

    system_init((uint32_t) LDR_BASE);
    core_systick_init();
    // button now managed at kernel boot to detect if DFU mode
    debug_console_init();

    enable_irq();

    dbg_log("======= Wookey Loader ========\n");
    dbg_log("Built date\t: %s at %s\n", __DATE__, __TIME__);
#ifdef STM32F429
    dbg_log("Board\t\t: STM32F429\n");
#else
    dbg_log("Board\t\t: STM32F407\n");
#endif
    dbg_log("==============================\n");
    dbg_flush();

/* RDP check */
#if CONFIG_LOADER_FLASH_RDP_CHECK
    switch (flash_check_rdpstate()) {
        case FLASH_RDP_DEACTIVATED:
            NVIC_SystemReset();
            goto err;
        case FLASH_RDP_MEMPROTECT:
            NVIC_SystemReset();
            goto err;
        case FLASH_RDP_CHIPPROTECT:
            dbg_log("Flash is fully protected\n");
            dbg_flush();
            break;
        default:
            NVIC_SystemReset();
            goto err;
    }
#endif

#if CONFIG_LOADER_MOCKUP
#if CONFIG_LOADER_MOCKUP_DFU
    dfu_mode = sectrue;
#endif
#endif

#ifdef CONFIG_FIRMWARE_DFU
    soc_dwt_init();
    dbg_log("Registering button on GPIO E4\n");
    dbg_flush();

    gpio.kref.port = DFU_GPIO_PORT;
    gpio.kref.pin = DFU_GPIO_PIN;
    gpio.mask = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
        GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
        GPIO_MASK_SET_EXTI;
    gpio.mode = GPIO_PIN_INPUT_MODE;
    gpio.pupd = GPIO_PULLDOWN;
    gpio.type = GPIO_PIN_OTYPER_PP;
    gpio.speed = GPIO_PIN_LOW_SPEED;
    gpio.afr = 0;
    gpio.bsr_r = 0;
    gpio.lck = 0;

    soc_gpio_set_config(&gpio);

    // wait 1s for button push...

    dbg_log("Waiting for DFU jump through button push (%d seconds)\n", count);
    uint32_t start, stop;
    uint8_t button_pushed;
    do {
        // in millisecs
        start = soc_dwt_getcycles() / 168000;
        do {
            stop = soc_dwt_getcycles() / 168000;
            button_pushed = soc_gpio_get(gpio.kref);
            if (button_pushed != 0) {
                dfu_mode = sectrue;
                break;
            }
        } while ((stop - start) < 1000); // < 1s
        dbg_log(".");
        dbg_flush();
        count--;
    } while (count > 0);

    if (count == 0) {
        dbg_log("Booting...\n");
    }
#else
    dbg_log("Booting...\n");
#endif

    secbool boot_flip = secfalse;
    const t_firmware_state *fw = 0;
#ifdef CONFIG_FIRMWARE_DUALBANK
    secbool boot_flop = secfalse;
    /* both FLIP and FLOP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE && flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        boot_flip = sectrue;
        boot_flop = sectrue;
        dbg_log("Both firwares have FW_BOOTABLE\n");
        dbg_log(COLOR_REVERSE "Flip version: %d\n" COLOR_NORMAL,
            flip_shared_vars.fw.fw_sig.version);
        dbg_log(COLOR_REVERSE "Flop version: %d\n" COLOR_NORMAL,
            flop_shared_vars.fw.fw_sig.version);
        dbg_flush();
        if (flip_shared_vars.fw.fw_sig.version > flop_shared_vars.fw.fw_sig.version) {
            /* Sanity check agaist fault on rollback */
            if(!(flip_shared_vars.fw.fw_sig.version > flop_shared_vars.fw.fw_sig.version)){
                goto err;
            }
            boot_flop = secfalse;
            fw = &flip_shared_vars.fw;
            if(!(flip_shared_vars.fw.fw_sig.version > flop_shared_vars.fw.fw_sig.version)){
                goto err;
            }
        }
        if ((boot_flip == sectrue) && (boot_flop == sectrue) && (flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version)) {
            /* Sanity check agaist fault on rollback */
            if(!((boot_flip == sectrue) && (boot_flop == sectrue) && (flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version))){
                goto err;
            }
            boot_flip = secfalse;
            fw = &flop_shared_vars.fw;
            if(!(flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version)){
                goto err;
            }
        }
        /* end of select sanitize... */
        if (!fw) {
            dbg_log(COLOR_REDBG "Unable to choose! leaving!\n" COLOR_NORMAL);
            dbg_flush();
            goto err;
        }
        goto check_crc;
    }
    /* only FLOP can be started */
    if (flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        if(!(flop_shared_vars.fw.bootable == FW_BOOTABLE)){
            goto err;
        }
        boot_flop = sectrue;
        dbg_log("Flop seems bootable\n");
        dbg_log(COLOR_REVERSE "Flop version: %d\n" COLOR_NORMAL, flop_shared_vars.fw.fw_sig.version);
        dbg_flush();
        boot_flip = secfalse;
        fw = &flop_shared_vars.fw;
        /* end of select sanitize... */
        if (!fw) {
            dbg_log(COLOR_REDBG "Unable to choose! leaving!\n" COLOR_NORMAL);
            dbg_flush();
            goto err;
        }
        goto check_crc;
    }


#endif
    /* In one bank configuration, only FLIP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE) {
        dbg_log(COLOR_REVERSE "Flip version: %d\n" COLOR_NORMAL, flip_shared_vars.fw.fw_sig.version);
        boot_flip = sectrue;
        dbg_log("Flip seems bootable\n");
        dbg_flush();
#ifdef CONFIG_FIRMWARE_DUALBANK
        boot_flop = secfalse;
#endif
        fw = &flip_shared_vars.fw;
        /* end of select sanitize... */
        if (!fw) {
            dbg_log(COLOR_REDBG "Unable to choose! leaving!\n" COLOR_NORMAL);
            dbg_flush();
            goto err;
        }
        goto check_crc;
    }

    /* fallback, none of the above allows to go to check_crc step */
    dbg_log(COLOR_REDBG "Panic! unable to boot on any firmware! none bootable\n" COLOR_NORMAL);
    dbg_log("Flip header:\n");
    dump_fw_header(&(flip_shared_vars.fw));
#ifdef CONFIG_FIRMWARE_DUALBANK
    dbg_log("------------\n");
    dbg_log("Flop header:\n");
    dump_fw_header(&(flop_shared_vars.fw));
#endif
    dbg_flush();
    goto err;


check_crc:
    dbg_log("entring security part\n");
#ifdef CONFIG_WOOKEY
    {
        /* Sanity check on the current selected partition and the header in flash */
        if(boot_flip == sectrue){
            if(fw->fw_sig.type != PART_FLIP){
                dbg_log(COLOR_REDBG "Error: FLIP selected, but partition type in flash header is not conforming!\n" COLOR_NORMAL);
                dbg_flush();
                goto err;
            }
        }
#ifdef CONFIG_FIRMWARE_DUALBANK
        else if((boot_flip == secfalse) && (boot_flop == sectrue)){
            if(fw->fw_sig.type != PART_FLOP){
                dbg_log(COLOR_REDBG "Error: FLOP selected, but partition type in flash header is not conforming!\n" COLOR_NORMAL);
                dbg_flush();
                goto err;
            }
        }
#endif
        else{
           goto err;
        }
    }
    {
        uint32_t buf = 0xffffffff;
#if LOADER_DEBUG
        dump_fw_header(fw);
#endif
        uint32_t crc = 0;
        /* checking CRC32 header check */
        crc = crc32((const uint8_t*)fw, sizeof(t_firmware_signature) - sizeof(uint32_t) - SHA256_DIGEST_SIZE - EC_MAX_SIGLEN, 0xffffffff);
        crc = crc32((uint8_t*)&buf, sizeof(uint32_t), crc);
        crc = crc32((const uint8_t*)fw->fw_sig.hash, SHA256_DIGEST_SIZE, crc);
        for (uint32_t i = 0; i <  EC_MAX_SIGLEN; ++i) {
            crc = crc32((uint8_t*)&buf, sizeof(uint8_t), crc);
        }
        /* check CRC of padding (fill field) */
        crc = crc32((const uint8_t*)fw->fill, SHR_SECTOR_SIZE - sizeof(t_firmware_signature), crc);
        /* check CRC of bootable flag  */
        crc = crc32((const uint8_t*)&fw->bootable, sizeof(uint32_t), crc);
        crc = crc32((const uint8_t*)&fw->fill2, SHR_SECTOR_SIZE - sizeof(uint32_t), crc);

	/* Double check for faults */
        if (crc != fw->fw_sig.crc32) {
            dbg_log(COLOR_REDBG "Invalid fw header CRC32: %x, %x required!!! leaving...\n" COLOR_NORMAL, crc, fw->fw_sig.crc32);
            dbg_flush();
            goto err;
        }
        if (crc != fw->fw_sig.crc32) {
            dbg_log(COLOR_REDBG "Invalid fw header CRC32: %x, %x required!!! leaving...\n" COLOR_NORMAL, crc, fw->fw_sig.crc32);
            dbg_flush();
            goto err;
        }

    }
#endif
    /* check firmware integrity if activated */

# ifdef CONFIG_LOADER_FW_HASH_CHECK
    uint32_t partition_addr;
    uint32_t partition_size;
    if (boot_flip == sectrue){
        partition_addr = FLIP_BASE;
        partition_size = FLIP_SIZE;
    }
#ifdef CONFIG_FIRMWARE_DUALBANK
    else if ((boot_flip == secfalse) && (boot_flop == sectrue)){
        partition_addr = FLOP_BASE;
        partition_size = FLOP_SIZE;
    }
#endif
    else{
        goto err;
    }
    if (check_fw_hash(fw, partition_addr, partition_size) != sectrue)
    {
        dbg_log(COLOR_REDBG "Error while checking firmware integrity! Leaving \n" COLOR_NORMAL);
        dbg_flush();
        goto err;
    }

# endif

    app_entry_t  next_level = 0;

    if (dfu_mode == sectrue) {
        if (boot_flip == sectrue) {
            dbg_log("Locking local bank write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writeunlock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLIP in DFU mode\n" COLOR_NORMAL);
            dbg_log("Jumping to DFU mode: %x\n", DFU1_START);
            next_level = (app_entry_t)DFU1_START;
        }
#ifdef CONFIG_FIRMWARE_DUALBANK
        else if ((boot_flip == secfalse) && (boot_flop == sectrue)) {
            dbg_log("locking local bank write\n");
            flash_unlock_opt();
            flash_writeunlock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLOP in DFU mode\n" COLOR_NORMAL);
            dbg_log("Jumping to DFU mode: %x\n", DFU2_START);
            next_level = (app_entry_t)DFU2_START;
        }
#endif
        else{
            goto err;
        }
        dbg_flush();
    } else if (dfu_mode == secfalse) {
        if (boot_flip == sectrue) {
            dbg_log("Locking flash write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLIP in nominal mode\n" COLOR_NORMAL);
            dbg_log("Jumping to FW mode: %x\n", FW1_START);
            next_level = (app_entry_t)FW1_START;
        }
#ifdef CONFIG_FIRMWARE_DUALBANK
        else if ((boot_flip == secfalse) && (boot_flop == sectrue)) {
            dbg_log("Locking flash write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLOP in nominal mode\n" COLOR_NORMAL);
            dbg_log("Jumping to FW mode: %x\n", FW2_START);
            next_level = (app_entry_t)FW2_START;
        }
#endif
        else{
            goto err;
        }
        dbg_flush();
    }
    else{
        goto err;
    }
    dbg_log("Geronimo !\n");
    dbg_flush();
    disable_irq();

    /* Sanity check */
    if(  (next_level != (app_entry_t)DFU1_START) && (next_level != (app_entry_t)DFU2_START)
      && (next_level != (app_entry_t)FW1_START) && (next_level != (app_entry_t)FW2_START)){
        goto err;
    }

    if (next_level) {
        next_level();
    }

    dbg_log(COLOR_REDBG "Error while selecting next level! leaving!\n" COLOR_NORMAL);

err:
    /* FIXME: reset the platform. Infinite loop is error prone. */
    dbg_log(COLOR_REDBG "Loader inifinite loop due to an error ...\n" COLOR_NORMAL);
    dbg_flush();
    while(1);
    return 0;
}
#ifdef __GNUC__
#ifdef __clang__
# pragma clang optimize on
#else
# pragma GCC pop_options
#endif
#endif
