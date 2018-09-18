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
#include "soc-rcc.h"
#include "soc-interrupts.h"
#include "boot_mode.h"
#include "stack_check.h"
#include "shared.h"
#include "leds.h"

/**
 *  Ref DocID022708 Rev 4 p.141
 *  BX/BLX cause usageFault if bit[0] of Rm is O
 *  Rm is a reg that indicates addr to branch to.
 *  Bit[0] of the value in Rm must be 1, BUT the addr
 *  to branch to is created by changing bit[0] to 0
 */
//app_entry_t ldr_main = (app_entry_t)(_ldr_base+1);
app_entry_t fw1_main = (app_entry_t) (FW1_START);
#ifdef CONFIG_FIRMWARE_DUALBANK
app_entry_t fw2_main = (app_entry_t) (FW2_START);
#endif

extern const shr_vars_t shared_vars;

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

    /* Initialize the stack protection */
    //init_stack_chk_guard();

    //leds_init();
    //leds_on(PROD_LED_STATUS);

    dbg_log("Jumping to : %x\n",
            shared_vars.apps[shared_vars.default_app_index].entry_point);
    dbg_log("Geronimo !\n");
    dbg_flush();

    disable_irq();
    shared_vars.apps[shared_vars.default_app_index].entry_point();

    return 0;
}
