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
#include "rng.h"
#include "automaton.h"

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
app_entry_t fw1_main = (app_entry_t) (FW1_START);
app_entry_t dfu1_main = (app_entry_t) (DFU1_START);
#ifdef CONFIG_FIRMWARE_DUALBANK
app_entry_t fw2_main = (app_entry_t) (FW2_START);
app_entry_t dfu2_main = (app_entry_t) (DFU2_START);
#endif

/*
 * definition and declaration of the loader context
 */

typedef struct loader_ctx {
    uint8_t status;
    volatile secbool dfu_mode;
    secbool boot_flip;
#ifdef CONFIG_FIRMWARE_DUALBANK
    secbool boot_flop;
#endif
    volatile uint32_t dfu_waitsec;
    const t_firmware_state *fw;
    app_entry_t  next_stage;
} loader_ctx_t;

static loader_ctx_t ctx = {
    .status = 0,
    .dfu_mode = secfalse,
    .boot_flip = secfalse,
#ifdef CONFIG_FIRMWARE_DUALBANK
    .boot_flop = false,
#endif
    .dfu_waitsec = 2,
    .fw = 0,
    .next_stage = 0
};


/**************************************************************************
 * About generic utility functions, that may be used by the bootloader
 * automaton
 *************************************************************************/

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

/**************************************************************************
 * Successive transition functions, each handling one given transition
 *************************************************************************/


static loader_request_t loader_exec_req_init(loader_state_t nextstate)
{

    /* entering transition target state (here LOADER_INIT) */
    loader_set_state(nextstate);

    /* and execute transition */
    disable_irq();
    core_systick_init();
    // button now managed at kernel boot to detect if DFU mode
    debug_console_init();

    /* self protecting own code write access */
    flash_unlock_opt();
    flash_lock_bootloader();
    flash_lock_opt();

    enable_irq();

    dbg_log("======= Wookey Loader ========\n");
    dbg_log("Built date\t: %s at %s\n", __DATE__, __TIME__);
#if defined(CONFIG_STM32F429)
    dbg_log("Board\t\t: STM32F429\n");
#elif defined(CONFIG_STM32F439)
    dbg_log("Board\t\t: STM32F439\n");
#elif defined(CONFIG_STM32F407)
    dbg_log("Board\t\t: STM32F407\n");
#else
    dbg_log("Board\t\t: Unknown!!\n");
#endif
    dbg_log("==============================\n");
    dbg_flush();

    /* There is no specific error handling in INIT state by now.
     * We can directly request the next transition... */
    return LOADER_REQ_RDPCHECK;
}

static loader_request_t loader_exec_req_rdpcheck(loader_state_t nextstate)
{

    loader_request_t nextreq = LOADER_REQ_SECBREACH;
    loader_state_t prevstate = loader_get_state();
    /* entering RDPCHECK */
    loader_set_state(nextstate);


#if CONFIG_LOADER_FLASH_RDP_CHECK
    /* RDP check */
    switch (flash_check_rdpstate()) {
        case FLASH_RDP_DEACTIVATED:
            flash_mass_erase();
            /* from now on... there is nothing left on the flash. The following
             * code should not be reached */
            NVIC_SystemReset();
            while (1);
        case FLASH_RDP_MEMPROTECT:
            flash_mass_erase();
            /* from now on... there is nothing left on the flash. The following
             * code should not be reached */
            NVIC_SystemReset();
            while (1);
        case FLASH_RDP_CHIPPROTECT:
            dbg_log("Flash is fully protected\n");
            dbg_flush();
            /* valid behavior */
            switch (prevstate) {
                case LOADER_INIT:
                    nextreq = LOADER_REQ_DFUCHECK;
                    break;
                case LOADER_DFUWAIT:
                    nextreq = LOADER_REQ_SELECTBANK;
                    break;
                case LOADER_SELECTBANK:
                    nextreq = LOADER_REQ_CRCCHECK;
                    break;
                case LOADER_HDRCRC:
                    nextreq = LOADER_REQ_INTEGRITYCHECK;
                    break;
                case LOADER_FWINTEGRITY:
                    nextreq = LOADER_REQ_FLASHLOCK;
                    break;
                case LOADER_FLASHLOCK:
                    nextreq = LOADER_REQ_BOOT;
                    break;
                default:
                    nextreq = LOADER_REQ_SECBREACH;
                    break;
            }
            break;
        default:
            /* what the ??? */
            flash_mass_erase();
            /* from now on... there is nothing left on the flash. The following
             * code should not be reached */
            NVIC_SystemReset();
            while (1);
            break;
    }
#else
    switch (prevstate) {
        case LOADER_INIT:
            nextreq = LOADER_REQ_DFUCHECK;
            break;
        case LOADER_DFUWAIT:
            nextreq = LOADER_REQ_SELECTBANK;
            break;
        case LOADER_SELECTBANK:
            nextreq = LOADER_REQ_CRCCHECK;
            break;
        case LOADER_HDRCRC:
            nextreq = LOADER_REQ_INTEGRITYCHECK;
            break;
        case LOADER_FWINTEGRITY:
            nextreq = LOADER_REQ_FLASHLOCK;
            break;
        case LOADER_FLASHLOCK:
            nextreq = LOADER_REQ_BOOT;
            break;
        default:
            nextreq = LOADER_REQ_SECBREACH;
            break;
    }
#endif
    return nextreq;
}

static loader_request_t loader_exec_req_dfucheck(loader_state_t nextstate)
{
    loader_set_state(nextstate);

    /* the DFU support is only handled for Wookey board, which has both DFU
     * button and enough flash memory */
#if CONFIG_WOOKEY

# if CONFIG_LOADER_MOCKUP
#  if CONFIG_LOADER_MOCKUP_DFU
    ctx.dfu_mode = sectrue;
#  endif
# endif

# ifdef CONFIG_FIRMWARE_DFU
    dev_gpio_info_t gpio = { 0 };

    soc_dwt_init();
    dbg_log("Registering button on GPIO E4\n");
    dbg_flush();

    gpio.kref.port = GPIO_PE; /* INFO: this is Wookey board specific */
    gpio.kref.pin = 4;        /* INFO: this is Wookey board specific */
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

    dbg_log("Waiting for DFU jump through button push (%d seconds)\n", ctx.dfu_waitsec);
    uint32_t start, stop;
    uint8_t button_pushed;
    do {
        // in millisecs
        start = soc_dwt_getcycles() / 168000;
        do {
            stop = soc_dwt_getcycles() / 168000;
            button_pushed = soc_gpio_get(gpio.kref);
            if (button_pushed != 0) {
                ctx.dfu_mode = sectrue;
                break;
            }
        } while ((stop - start) < 1000); // < 1s
        dbg_log(".");
        dbg_flush();
        ctx.dfu_waitsec--;
    } while (ctx.dfu_waitsec > 0);
    /* now we have finished with the DFU button, release the GPIO */
    soc_gpio_release(&gpio);
    dbg_log("Booting...\n");
# else
    dbg_log("Booting...\n");
# endif
#endif
    return LOADER_REQ_RDPCHECK;
}

static loader_request_t loader_exec_req_selectbank(loader_state_t nextstate)
{
    loader_set_state(nextstate);


#ifdef CONFIG_FIRMWARE_DUALBANK
    /* both FLIP and FLOP can be started */
    /* FIX: found by LETI: a FIA can be done on the fw bootable flag check which permit to corrupt
     * the bootable firmware flag and change the behavior of the firmware selection, endanger the
     * anti-rollback security function. Here we duplicate the if to avoid single fault attack.
     * A postcheck has also be added just before finishing transition.
     */
    if ((flip_shared_vars.fw.bootable == FW_BOOTABLE && flop_shared_vars.fw.bootable == FW_BOOTABLE) &&
        !(flip_shared_vars.fw.bootable != FW_BOOTABLE || flop_shared_vars.fw.bootable != FW_BOOTABLE)){
        ctx.boot_flip = sectrue;
        ctx.boot_flop = sectrue;
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
            ctx.boot_flop = secfalse;
            ctx.fw = &flip_shared_vars.fw;
            if(!(flip_shared_vars.fw.fw_sig.version > flop_shared_vars.fw.fw_sig.version)){
                goto err;
            }
        }
        if ((ctx.boot_flip == sectrue) && (ctx.boot_flop == sectrue) && (flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version)) {
            /* Sanity check agaist fault on rollback */
            if(!((ctx.boot_flip == sectrue) && (ctx.boot_flop == sectrue) && (flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version))){
                goto err;
            }
            ctx.boot_flip = secfalse;
            ctx.fw = &flop_shared_vars.fw;
            if(!(flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version)){
                goto err;
            }
        }
        /* end of select sanitize... */
        if (!ctx.fw) {
            dbg_log(COLOR_REDBG "Unable to choose! leaving!\n" COLOR_NORMAL);
            dbg_flush();
            goto err;
        }

        /* FIX found by LETI: continuing patch against FIA attack: postcheck here. shared_vars check should be
         * the same as at the begining of the function. */
        if (!(flip_shared_vars.fw.bootable == FW_BOOTABLE && flop_shared_vars.fw.bootable == FW_BOOTABLE)) {
            goto err;
        }
        goto check_crc;
    }
    /* only FLOP can be started */
    if (flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        if(!(flop_shared_vars.fw.bootable == FW_BOOTABLE)){
            goto err;
        }
        ctx.boot_flop = sectrue;
        dbg_log("Flop seems bootable\n");
        dbg_log(COLOR_REVERSE "Flop version: %d\n" COLOR_NORMAL, flop_shared_vars.fw.fw_sig.version);
        dbg_flush();
        ctx.boot_flip = secfalse;
        ctx.fw = &flop_shared_vars.fw;
        /* end of select sanitize... */
        if (!ctx.fw) {
            dbg_log(COLOR_REDBG "Unable to choose! leaving!\n" COLOR_NORMAL);
            dbg_flush();
            goto err;
        }
        /* postcheck: FIA protection */
        if(!(flop_shared_vars.fw.bootable == FW_BOOTABLE)){
            goto err;
        }
        goto check_crc;
    }

#endif
    /* In one bank configuration, only FLIP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE) {
        dbg_log(COLOR_REVERSE "Flip version: %d\n" COLOR_NORMAL, flip_shared_vars.fw.fw_sig.version);
        ctx.boot_flip = sectrue;
        dbg_log("Flip seems bootable\n");
        dbg_flush();
#ifdef CONFIG_FIRMWARE_DUALBANK
        ctx.boot_flop = secfalse;
#endif
        ctx.fw = &flip_shared_vars.fw;
        /* end of select sanitize... */
        if (!ctx.fw) {
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
    return LOADER_REQ_RDPCHECK;
err:
    return LOADER_REQ_SECBREACH;
}

static loader_request_t loader_exec_req_crccheck(loader_state_t nextstate)
{
    loader_set_state(nextstate);

    {
        /* Sanity check on the current selected partition and the header in flash */
        if (ctx.boot_flip == sectrue) {
            if((ctx.fw)->fw_sig.type != PART_FLIP){
                dbg_log(COLOR_REDBG "Error: FLIP selected, but partition type in flash header is not conforming!\n" COLOR_NORMAL);
                dbg_flush();
                goto err;
            }
        }
#ifdef CONFIG_FIRMWARE_DUALBANK
        else if((ctx.boot_flip == secfalse) && (ctx.boot_flop == sectrue)){
            if((ctx.fw)->fw_sig.type != PART_FLOP){
                dbg_log(COLOR_REDBG "Error: FLOP selected, but partition type in flash header is not conforming!\n" COLOR_NORMAL);
                dbg_flush();
                goto err;
            }
        }
#endif
        else {
           goto err;
        }
    }
    /* sanity check okay, calculating CRC32 */
    {
        uint32_t buf = 0xffffffff;
#if CONFIG_LOADER_EXTRA_DEBUG
        dump_fw_header(ctx.fw);
#endif
        uint32_t crc = 0;
        /* checking CRC32 header check */
        crc = crc32((const uint8_t*)ctx.fw, sizeof(t_firmware_signature) - sizeof(uint32_t) - SHA256_DIGEST_SIZE - EC_MAX_SIGLEN, 0xffffffff);
        crc = crc32((uint8_t*)&buf, sizeof(uint32_t), crc);
        crc = crc32((const uint8_t*)((ctx.fw)->fw_sig.hash), SHA256_DIGEST_SIZE, crc);
        for (uint32_t i = 0; i <  EC_MAX_SIGLEN; ++i) {
            crc = crc32((uint8_t*)&buf, sizeof(uint8_t), crc);
        }
        /* check CRC of padding (fill field) */
        crc = crc32((const uint8_t*)((ctx.fw)->fill), SHR_SECTOR_SIZE - sizeof(t_firmware_signature), crc);
        /* check CRC of bootable flag  */
        crc = crc32((const uint8_t*)&((ctx.fw)->bootable), sizeof(uint32_t), crc);
        crc = crc32((const uint8_t*)&((ctx.fw)->fill2), SHR_SECTOR_SIZE - sizeof(uint32_t), crc);

	/* Double check for faults */
        if (crc != (ctx.fw)->fw_sig.crc32) {
            dbg_log(COLOR_REDBG "Invalid fw header CRC32: %x, %x required!!! leaving...\n" COLOR_NORMAL, crc, (ctx.fw)->fw_sig.crc32);
            dbg_flush();
            goto err;
        }
        if (crc != (ctx.fw)->fw_sig.crc32) {
            dbg_log(COLOR_REDBG "Invalid fw header CRC32: %x, %x required!!! leaving...\n" COLOR_NORMAL, crc, (ctx.fw)->fw_sig.crc32);
            dbg_flush();
            goto err;
        }
    }

    return LOADER_REQ_RDPCHECK;
err:
    return LOADER_REQ_SECBREACH;

}

static loader_request_t loader_exec_req_integritycheck(loader_state_t nextstate)
{
    loader_set_state(nextstate);

#ifdef CONFIG_LOADER_FW_HASH_CHECK
    uint32_t partition_addr;
    uint32_t partition_size;
    if (ctx.boot_flip == sectrue){
        partition_addr = FLIP_BASE;
        partition_size = FLIP_SIZE;
    }
# ifdef CONFIG_FIRMWARE_DUALBANK
    else if ((ctx.boot_flip == secfalse) && (ctx.boot_flop == sectrue)){
        partition_addr = FLOP_BASE;
        partition_size = FLOP_SIZE;
    }
# endif
    else{
        goto err;
    }
    if (check_fw_hash(ctx.fw, partition_addr, partition_size) != sectrue)
    {
        dbg_log(COLOR_REDBG "Error while checking firmware integrity! Leaving \n" COLOR_NORMAL);
        dbg_flush();
        goto err;
    }

#endif
    return LOADER_REQ_RDPCHECK;
#ifdef CONFIG_LOADER_FW_HASH_CHECK
err:
    return LOADER_REQ_SECBREACH;
#endif
}

static loader_request_t loader_exec_req_flashlock(loader_state_t nextstate)
{

    loader_set_state(nextstate);

    if (ctx.dfu_mode == sectrue) {
        if (ctx.boot_flip == sectrue) {
            dbg_log("Locking local bank write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writeunlock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLIP in DFU mode\n" COLOR_NORMAL);
            dbg_log("Jumping to DFU mode: %x\n", DFU1_START);
            ctx.next_stage = (app_entry_t)DFU1_START;
        }
#ifdef CONFIG_FIRMWARE_DUALBANK
        else if ((ctx.boot_flip == secfalse) && (ctx.boot_flop == sectrue)) {
            dbg_log("locking local bank write\n");
            flash_unlock_opt();
            flash_writeunlock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLOP in DFU mode\n" COLOR_NORMAL);
            dbg_log("Jumping to DFU mode: %x\n", DFU2_START);
            ctx.next_stage = (app_entry_t)DFU2_START;
        }
#endif
        else{
            goto err;
        }
        dbg_flush();
    } else if (ctx.dfu_mode == secfalse) {
        if (ctx.boot_flip == sectrue) {
            dbg_log("Locking flash write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLIP in nominal mode\n" COLOR_NORMAL);
            dbg_log("Jumping to FW mode: %x\n", FW1_START);
            ctx.next_stage = (app_entry_t)FW1_START;
        }
#ifdef CONFIG_FIRMWARE_DUALBANK
        else if ((ctx.boot_flip == secfalse) && (ctx.boot_flop == sectrue)) {
            dbg_log("Locking flash write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log(COLOR_REVERSE "Booting FLOP in nominal mode\n" COLOR_NORMAL);
            dbg_log("Jumping to FW mode: %x\n", FW2_START);
            ctx.next_stage = (app_entry_t)FW2_START;
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

    return LOADER_REQ_RDPCHECK;
err:
    return LOADER_REQ_ERROR;
}

static loader_request_t loader_exec_req_boot(loader_state_t nextstate)
{
    loader_set_state(nextstate);

    dbg_log("Geronimo !\n");
    dbg_flush();
    disable_irq();

    /* Sanity check */
    if(  (ctx.next_stage != (app_entry_t)DFU1_START) && (ctx.next_stage != (app_entry_t)DFU2_START)
      && (ctx.next_stage != (app_entry_t)FW1_START) && (ctx.next_stage != (app_entry_t)FW2_START)){
        goto err;
    }

    if (ctx.next_stage) {
        /* clear debug device if activated */
        debug_release();
        ctx.next_stage();
    }
    /* this part of the code should never be reached */
    return LOADER_REQ_SECBREACH;

err:
    return LOADER_REQ_ERROR;
}

static loader_request_t loader_exec_error(loader_state_t state)
{
    dbg_log("ERROR! entering error from state %x!\n", state);
    dbg_flush();
    NVIC_SystemReset();
    while (1); /* waiting for reset */
    return LOADER_REQ_ERROR;
}

static loader_request_t loader_exec_secbreach(loader_state_t state)
{
    dbg_log("ERROR! entering Security breach from state %x!\n", state);
    dbg_flush();
    /*In case of security breach, we may react differently before reseting */
    /* let's lock both flash bank*/
#if CONFIG_LOADER_ERASE_ON_SECBREACH
    flash_mass_erase();
#endif
    NVIC_SystemReset();
    while (1);
    return LOADER_REQ_ERROR;
}



/*
 * transition switch head function.
 */
static loader_request_t loader_exec_automaton_transition(const loader_request_t req)
{
    loader_state_t state = loader_get_state();
    loader_request_t nextreq = LOADER_REQ_ERROR;
    /* FIX: found by LETI: weakness in previous boolean handling (implicit comparison)
     * which may lead to successful FIA */
    if (loader_is_valid_transition(state, req) != sectrue) {
        loader_set_state(LOADER_ERROR);
        goto end_transition;
    }
    loader_state_t nextstate = loader_next_state(state, req);
    /* nextstate must always be valid, considering the automaton defined
     * in automaton.c */

    switch(req) {
        case LOADER_REQ_INIT:
            nextreq = loader_exec_req_init(nextstate);
            break;
        case LOADER_REQ_RDPCHECK:
            nextreq = loader_exec_req_rdpcheck(nextstate);
            break;
        case LOADER_REQ_DFUCHECK:
            nextreq = loader_exec_req_dfucheck(nextstate);
            break;
        case LOADER_REQ_SELECTBANK:
            nextreq = loader_exec_req_selectbank(nextstate);
            break;
        case LOADER_REQ_CRCCHECK:
            nextreq = loader_exec_req_crccheck(nextstate);
            break;
        case LOADER_REQ_INTEGRITYCHECK:
            nextreq = loader_exec_req_integritycheck(nextstate);
            break;
        case LOADER_REQ_FLASHLOCK:
            nextreq = loader_exec_req_flashlock(nextstate);
            break;
        case LOADER_REQ_BOOT:
            nextreq = loader_exec_req_boot(nextstate);
            break;
        case LOADER_REQ_ERROR:
            nextreq = loader_exec_error(state);
            break;
        case LOADER_REQ_SECBREACH:
            nextreq = loader_exec_secbreach(state);
            break;
        default:
            nextreq = LOADER_REQ_ERROR;
    }
end_transition:
    return nextreq;
}

/* execute each requested transition. The automaton must finish in one of
 *    - LOADER_BOOTFW,
 *    - LOADER_ERROR,
 *    - LOADER_SECBREACH
 * states, from which it never goes out.
 */
static void loader_exec_automaton(loader_request_t req)
{
    while (true) {
        req = loader_exec_automaton_transition(req);
    }
}


#if __GNUC__
#pragma GCC push_options
#pragma GCC optimize("-fno-stack-protector")
#endif

#if __clang__
#pragma clang optimize off
  /* Well, clang support local stack protection deactivation only since v8 :-/ */
#if __clang_major__ > 7
#pragma clang attribute push(__attribute__((no_stack_protector)), apply_to = do_starttask)
#endif
#endif

/* The stack check guard value */
volatile uint32_t __stack_chk_guard = 0;

/*
 * This function handles stack check error, corresponding to canary corruption
 * detection
 */
void __stack_chk_fail(void)
{
    /* We have failed to check our stack canary: go to panic! */
    panic("Failed to check the stack guard! Stack corruption!");

    while (1) {};
}

int main(void)
{
    system_init((uint32_t) LDR_BASE);
    /* Initialize the stack guard */
    if(rng_manager((uint32_t*)&__stack_chk_guard)){
        panic("Failed to init stack guard with RNG!");
        goto err;
    }
    loader_set_state(LOADER_START);
    loader_request_t initial_req = LOADER_REQ_INIT;

    loader_exec_automaton(initial_req);

    dbg_log(COLOR_REDBG "Error while selecting next level! leaving!\n" COLOR_NORMAL);
    dbg_flush();
    loader_exec_error(loader_get_state());
    return 0;
err:
    /* End here in case of critical error */
    while(1){};
}

#if __clang__
#pragma clang optimize on
#if __clang_major__ > 7
#pragma clang attribute pop
#endif
#endif

#if __GNUC__
#pragma GCC pop_options
#endif
