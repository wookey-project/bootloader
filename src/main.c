/** @file main.c
 *
 */
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
#include "soc-layout.h"
#include "soc-gpio.h"
#include "soc-exti.h"
#include "soc-nvic.h"
#include "soc-rcc.h"
#include "soc-interrupts.h"
#include "boot_mode.h"
#include "stack_check.h"
#include "shr.h"
#include "leds.h"
#include "crc32.h"
#include "exported/gpio.h"
#include "hash.h"
#include "flash.h"

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
#endif

volatile bool dfu_mode = false;

void hexdump(const uint8_t *bin, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++) {
    dbg_log("%x ", bin[i]);
    if (i % 16 == 0 && i != 0) {
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
    dbg_log("magic    :  %x\n", fw->fw_sig.magic);
    dbg_log("version  :  %x\n", fw->fw_sig.version);
    dbg_log("siglen   :  %x\n", fw->fw_sig.siglen);
    dbg_log("len      :  %x\n", fw->fw_sig.len);
    dbg_log("chunksize:  %x\n", fw->fw_sig.chunksize);
    dbg_log("sig      :\n");
    dbg_flush();
    if (fw->fw_sig.siglen) {
        hexdump(fw->fw_sig.sig, fw->fw_sig.siglen);
    } else {
        hexdump(fw->fw_sig.sig, EC_MAX_SIGLEN);
    }
    dbg_flush();
    dbg_log("crc32    :  %x\n", fw->fw_sig.crc32);
    dbg_log("bash     :\n");
        hexdump(fw->fw_sig.hash, SHA256_DIGEST_SIZE);
    dbg_log("bootable :  %x\n", fw->bootable);
    dbg_flush();
}

#ifdef CONFIG_FIRMWARE_DFU
void exti_button_handler(uint8_t irq __attribute__((unused)),
                         uint32_t sr __attribute__((unused)),
                         uint32_t dr __attribute__((unused)))
{
    dfu_mode = true;
    /* Security info:
     * DFU mode request is registered. Just don't try to freeze
     * the boot sequence by generating EXTI interrupt burst
     * The Exti-lock feature is an EwoK feature, not a loader feature
     */
    soc_exti_disable(gpio.kref);
}
#endif

extern const shr_vars_t flip_shared_vars;
#ifdef CONFIG_FIRMWARE_DUALBANK
extern const shr_vars_t flop_shared_vars;
#endif
    volatile uint32_t count = 2;

/*
 * We use the local -fno-stack-protector flag for main because
 * the stack protection has not been initialized yet.
 */
__attribute__ ((optimize("-fno-stack-protector")))
int main(void)
{
    disable_irq();

    /* init MSP for loader */
//    asm volatile ("msr msp, %0\n\t" : : "r" (0x20020000) : "r2");

    system_init((uint32_t) LDR_BASE);
    core_systick_init();
    // button now managed at kernel boot to detect if DFU mode
    //d_button_init();
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
            dbg_lush();
            break;
        default:
            NVIC_SystemReset();
            goto err;
    }
#endif

#if CONFIG_LOADER_MOCKUP
#if CONFIG_LOADER_MOCKUP_DFU
    dfu_mode = true;
#endif
#endif

    //leds_init();
    //leds_on(PROD_LED_STATUS);
    // button is on GPIO E4
#ifdef CONFIG_FIRMWARE_DFU
    soc_dwt_init();
    soc_exti_init();
    dbg_log("registering button on GPIO E4\n");
    dbg_flush();

    gpio.kref.port = GPIO_PE;
    gpio.kref.pin = 4;
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
    gpio.exti_trigger = GPIO_EXTI_TRIGGER_RISE;
    gpio.exti_lock = GPIO_EXTI_LOCKED;
    gpio.exti_handler = (user_handler_t) exti_button_handler;

    soc_gpio_set_config(&gpio);
    soc_exti_config(&gpio);
    soc_exti_enable(gpio.kref);
#endif

    // wait 1s for button push...

    dbg_log("Waiting for DFU jump through button push (%d seconds)\n", count);
    uint32_t start, stop;
    do {
        // in millisecs
        start = soc_dwt_getcycles() / 168000;
        do {
        stop = soc_dwt_getcycles() / 168000;
        } while (stop - start < 1000); // < 1s
        dbg_log(".");
        dbg_flush();
        count--;
    } while (count > 0);

    if (count == 0) {
        dbg_log("Booting...\n");
    }

    bool boot_flip = false;
    bool boot_flop = false;
    const t_firmware_state *fw = 0;
#ifdef CONFIG_FIRMWARE_DUALBANK
    /* both FLIP and FLOP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE && flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        boot_flip = boot_flop = true;
        dbg_log("Both firwares have FW_BOOTABLE\n");
        dbg_log("\x1b[33;43mflip version: %d\x1b[37;40m\n", flip_shared_vars.fw.fw_sig.version);
        dbg_log("\x1b[33;43mflop version: %d\x1b[37;40m\n", flop_shared_vars.fw.fw_sig.version);
        dbg_flush();
        if (flip_shared_vars.fw.fw_sig.version > flop_shared_vars.fw.fw_sig.version) {
            boot_flop = false;
            fw = &flip_shared_vars.fw;
        }
        if (boot_flip && boot_flop && flop_shared_vars.fw.fw_sig.version > flip_shared_vars.fw.fw_sig.version) {
            boot_flip = false;
            fw = &flop_shared_vars.fw;
        }
        /* end of select sanitize... */
        if (!fw) {
            dbg_log("Unable to choose! leaving!\n");
            dbg_flush();
            goto err;
        }
        goto check_crc;
    }

    /* only FLOP can be started */
    if (flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        boot_flop = true;
        dbg_log("flop seems bootable\n");
        dbg_log("\x1b[37;43mflop version: %d\x1b[37;40m\n", flop_shared_vars.fw.fw_sig.version);
        dbg_flush();
        boot_flip = false;
        fw = &flop_shared_vars.fw;
        /* end of select sanitize... */
        if (!fw) {
            dbg_log("Unable to choose! leaving!\n");
            dbg_flush();
            goto err;
        }
        goto check_crc;
    }
#endif
    /* In one bank configuration, only FLIP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE) {
        dbg_log("\x1b[37;43mflip version: %d\x1b[37;40m\n", flip_shared_vars.fw.fw_sig.version);
        boot_flip = true;
        dbg_log("flip seems bootable\n");
        dbg_flush();
        boot_flop = false;
        fw = &flip_shared_vars.fw;
        /* end of select sanitize... */
        if (!fw) {
            dbg_log("Unable to choose! leaving!\n");
            dbg_flush();
            goto err;
        }
        goto check_crc;
    }

    /* fallback, none of the above allows to go to check_crc step */
    dbg_log("panic! unable to boot on any firmware ! none bootable\n");
    dbg_log("flip header:\n");
    dump_fw_header(&(flip_shared_vars.fw));
#ifdef CONFIG_FIRMWARE_DUALBANK
    dbg_log("------------\n");
    dbg_log("flop header:\n");
    dump_fw_header(&(flop_shared_vars.fw));
#endif
    dbg_flush();
    return 0;


check_crc:
    {
        uint32_t buf = 0xffffffff;
#if LOADER_DEBUG
        dump_fw_header(fw);
#endif
        uint32_t crc = 0;
        /* checking CRC32 header check */
        crc = crc32((uint8_t*)fw, sizeof(t_firmware_signature) - sizeof(uint32_t) - SHA256_DIGEST_SIZE - EC_MAX_SIGLEN, 0xffffffff);
        crc = crc32((uint8_t*)&buf, sizeof(uint32_t), crc);
        crc = crc32((uint8_t*)fw->fw_sig.hash, SHA256_DIGEST_SIZE, crc);
        for (uint32_t i = 0; i <  EC_MAX_SIGLEN; ++i) {
            crc = crc32((uint8_t*)&buf, sizeof(uint8_t), crc);
        }
        /* check CRC of padding (fill field) */
        crc = crc32((uint8_t*)fw->fill, SHR_SECTOR_SIZE - sizeof(t_firmware_signature), crc);
        /* check CRC of bootable flag  */
        crc = crc32((uint8_t*)&fw->bootable, sizeof(uint32_t), crc);
        crc = crc32((uint8_t*)&fw->fill2, SHR_SECTOR_SIZE - sizeof(uint32_t), crc);

        if (crc != fw->fw_sig.crc32) {
            dbg_log("invalid fw header CRC32: %x, %x required!!! leaving...\n", crc, fw->fw_sig.crc32);
            dbg_flush();
            goto err;
        }
    }

    /* check firmware integrity if activated */

# ifdef CONFIG_LOADER_FW_HASH_CHECK
    uint32_t partition_addr;
    uint32_t partition_size;
    if (boot_flip){
        partition_addr = FLIP_BASE;
        partition_size = FLIP_SIZE;
    }
    else if (boot_flop){
        partition_addr = FLOP_BASE;
        partition_size = FLOP_SIZE;
    }
    else{
        goto err;
    }
    if (!check_fw_hash(fw, partition_addr, partition_size))
    {
        dbg_log("Error while checking firmware integrity! Leaving \n");
        dbg_flush();
        goto err;
    }
# endif

    app_entry_t  next_level = 0;

    if (dfu_mode) {
        if (boot_flip) {
            dbg_log("locking local bank write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writeunlock_bank2();
            flash_lock_opt();
            dbg_log("\x1b[37;41mbooting FLIP in DFU mode\x1b[37;40m\n");
            dbg_log("Jumping to DFU mode: %x\n", DFU1_START);
            next_level = (app_entry_t)DFU1_START;
        }
        if (boot_flop) {
            dbg_log("locking local bank write\n");
            flash_unlock_opt();
            flash_writeunlock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log("\x1b[37;41mbooting FLOP in DFU mode\x1b[37;40m\n");
            dbg_log("Jumping to DFU mode: %x\n", DFU2_START);
            next_level = (app_entry_t)DFU2_START;
        }
        dbg_flush();
    } else {
        if (boot_flip) {
            dbg_log("locking flash write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log("\x1b[37;41mbooting FLIP in nominal mode\x1b[37;40m\n");
            dbg_log("Jumping to FW mode: %x\n", FW1_START);
            next_level = (app_entry_t)FW1_START;
        }
        if (boot_flop) {
            dbg_log("locking flash write\n");
            flash_unlock_opt();
            flash_writelock_bank1();
            flash_writelock_bank2();
            flash_lock_opt();
            dbg_log("\x1b[37;41mbooting FLOP in nominal mode\x1b[37;40m\n");
            dbg_log("Jumping to FW mode: %x\n", FW2_START);
            next_level = (app_entry_t)FW2_START;
        }
        dbg_flush();
    }
    dbg_log("Geronimo !\n");
    dbg_flush();
    disable_irq();


    if (next_level) {
        next_level();
    }

    dbg_log("error while selecting next level! leaving!\n");

err:
    while(1);
    return 0;
}
