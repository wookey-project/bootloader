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

volatile bool dfu_mode = false;


void dump_fw_header(const t_firmware_state *fw)
{
    dbg_flush();
    dbg_log("bootable:  %x\n", fw->bootable);
    dbg_log("version :  %d\n", fw->version);
    dbg_log("siglen  :  %d\n", fw->siglen);
    dbg_flush();
    dbg_log("sig     :  %x %x .. %x %x\n", fw->sig[0], fw->sig[1], fw->sig[fw->siglen - 2], fw->sig[fw->siglen - 1]);
    dbg_log("crc32   :  %x\n", fw->crc32);
    dbg_flush();
}

void exti_button_handler(uint8_t irq __attribute__((unused)),
                         uint32_t sr __attribute__((unused)),
                         uint32_t dr __attribute__((unused)))
{
  dfu_mode = true;
}

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
    dev_gpio_info_t gpio = {
        .kref.port = GPIO_PE,
        .kref.pin = 4,
        .mask = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
            GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
            GPIO_MASK_SET_EXTI,
        .mode = GPIO_PIN_INPUT_MODE,
        .pupd = GPIO_PULLDOWN,
        .type = GPIO_PIN_OTYPER_PP,
        .speed = GPIO_PIN_LOW_SPEED,
        .afr = 0,
        .bsr_r = 0,
        .lck = 0,
        .exti_trigger = GPIO_EXTI_TRIGGER_RISE,
        .exti_lock = GPIO_EXTI_LOCKED,
        .exti_handler = (user_handler_t) exti_button_handler
    };
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

    bool boot_flip = true;
    bool boot_flop = true;
    const t_firmware_state *fw = 0;

#ifdef CONFIG_FIRMWARE_DUALBANK
    /* both FLIP and FLOP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE && flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        dbg_log("both firwares seems bootable\n");
        dbg_flush();
        if (flip_shared_vars.fw.version == ERASE_VALUE) {
            boot_flip = false;
        }
        if (flop_shared_vars.fw.version == ERASE_VALUE) {
            boot_flop = false;
        }
        if (boot_flip && boot_flop && flip_shared_vars.fw.version > flop_shared_vars.fw.version) {
            boot_flop = false;
            fw = &flip_shared_vars.fw;
        }
        if (boot_flip && boot_flop && flop_shared_vars.fw.version > flip_shared_vars.fw.version) {
            boot_flip = false;
            fw = &flop_shared_vars.fw;
        }
        /* end of select sanitize... */
        if (!fw) {
            dbg_log("Unable to choose ! leaving!\n");
            dbg_flush();
            return 0;
        }
        goto check_crc;
    }

    /* only FLOP can be started */
    if (flop_shared_vars.fw.bootable == FW_BOOTABLE) {
        dbg_log("flop seems bootable\n");
        dbg_flush();
        boot_flip = false;
        if (flop_shared_vars.fw.version == ERASE_VALUE) {
            boot_flop = false;
            dbg_log("invalid version value for lonely bootable FLIP ! leaving\n");
            dbg_flush();
            return 0;
        }
        fw = &flop_shared_vars.fw;
        /* end of select sanitize... */
        if (!fw) {
            dbg_log("Unable to choose ! leaving!\n");
            dbg_flush();
            return 0;
        }
        goto check_crc;
    }

#endif
    /* only FLIP can be started */
    if (flip_shared_vars.fw.bootable == FW_BOOTABLE) {
        dbg_log("flip seems bootable\n");
        dbg_flush();
        boot_flop = false;
        if (flip_shared_vars.fw.version == ERASE_VALUE) {
            boot_flip = false;
            dbg_log("invalid version value for lonely bootable FLIP ! leaving\n");
            dbg_flush();
            return 0;
        }
        fw = &flip_shared_vars.fw;
        /* end of select sanitize... */
        if (!fw) {
            dbg_log("Unable to choose ! leaving!\n");
            dbg_flush();
            return 0;
        }
        goto check_crc;
    }

    /* fallback, none of the above permits to go to check_crc step */
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

    /* checking CRC32 header check */
    if (fw->version != 0) {
        uint32_t crc;
        /* when version is 0, this means that this is the initial JTAG
         * flashed firmware. This firmaware doesn't have (yet) a header
         * signature */
        crc = crc32((uint8_t*)fw, sizeof(t_firmware_state) - sizeof(uint32_t), 0xffffffff);
        if (crc != fw->crc32) {
            dbg_log("invalid fw header CRC32!!! leaving...\n");
            dbg_flush();
            return 0;
        }
    }

#if 0
    dbg_log("boot structure: default: %d\n", shared_vars.default_app_index);
    dbg_log("boot structure: default_dfu: %d\n", shared_vars.default_dfu_index);
    dbg_log("boot structure: fw entrypoint %x\n", shared_vars.apps[shared_vars.default_app_index].entry_point);
    dbg_log("boot structure: dfu entrypoint %x\n", shared_vars.apps[shared_vars.default_dfu_index].entry_point);
#endif

    app_entry_t  next_level = 0;

    if (dfu_mode) {
        if (boot_flip) {
            dbg_log("Jumping to DFU mode: %x\n", DFU1_START);
            next_level = (app_entry_t)DFU1_START;
        }
        if (boot_flop) {
            dbg_log("Jumping to DFU mode: %x\n", DFU2_START);
            next_level = (app_entry_t)DFU2_START;
        }
        dbg_flush();
    } else {
        if (boot_flip) {
            dbg_log("Jumping to FW mode: %x\n", FW1_START);
            next_level = (app_entry_t)FW1_START;
        }
        if (boot_flop) {
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

    return 0;
}
