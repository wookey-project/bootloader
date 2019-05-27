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
#include "autoconf.h"
#include "m4-cpu.h"
#include "soc-init.h"
#include "soc-flash.h"
#include "soc-pwr.h"
#include "soc-scb.h"
#include "soc-rcc.h"
#include "debug.h"

/*
 * \brief Configure the Vector Table location and offset address
 *
 * WARNING : No interrupts here => IRQs disabled
 *				=> No LOGs here
 */
void set_vtor(uint32_t addr)
{

    __DMB();                    /* Data Memory Barrier */
#ifdef CONFIG_STM32F4
    write_reg_value(r_CORTEX_M_SCB_VTOR, addr);
#endif
    __DSB();                    /*
                                 * Data Synchronization Barrier to ensure all
                                 * subsequent instructions use the new configuration
                                 */
}

/* void system_init(void)
 * \brief  Setup the microcontroller system
 *
 *         Initialize the Embedded Flash Interface, the PLL and update the
 *         SystemFrequency variable.
 */
void system_init(uint32_t addr)
{
#ifdef PROD_ENABLE_HSE
    bool enable_hse = true;
#else
    bool enable_hse = false;
#endif

#ifdef PROD_ENABLE_PLL
    bool enable_pll = true;
#else
    bool enable_hse = false;
#endif

#ifdef CONFIG_STM32F4
    soc_rcc_reset();
    /*
     * Configure the System clock source, PLL Multiplier and Divider factors,
     * AHB/APBx prescalers and Flash settings
     */
    soc_rcc_setsysclock(enable_hse, enable_pll);
#endif

    //set_vtor(FLASH_BASE|VECT_TAB_OFFSET);
    set_vtor(addr);
}
