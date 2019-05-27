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
#include "regutils.h"
#include "autoconf.h"
#include "soc-rcc.h"
#include "soc-pwr.h"
#include "soc-flash.h"
#include "m4-cpu.h"

/*
 * TODO: some of the bellowing code should be M4 generic. Yet, check if all
 * these registers are M4 generic or STM32F4 core specific
 */
void soc_rcc_reset(void)
{
    /* Reset the RCC clock configuration to the default reset state */
    /* Set HSION bit */
    set_reg_bits(r_CORTEX_M_RCC_CR, RCC_CR_HSION);

    /* Reset CFGR register */
    write_reg_value(r_CORTEX_M_RCC_CFGR, 0x00000000);

    /* Reset HSEON, CSSON and PLLON bits */
    clear_reg_bits(r_CORTEX_M_RCC_CR,
                   RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLLON);

    /* Reset PLLCFGR register */
    write_reg_value(r_CORTEX_M_RCC_PLLCFGR, 0x24003010);

    /* Reset HSEBYP bit */
    clear_reg_bits(r_CORTEX_M_RCC_CR, RCC_CR_HSEBYP);

    /* Reset all interrupts */
    write_reg_value(r_CORTEX_M_RCC_CIR, 0x00000000);

    full_memory_barrier();
}

void soc_rcc_setsysclock(bool enable_hse, bool enable_pll)
{
    uint32_t StartUpCounter = 0, status = 0;

    /*
     * PLL (clocked by HSE/HSI) used as System clock source
     */

    if (enable_hse) {
        /* Enable HSE */
        set_reg_bits(r_CORTEX_M_RCC_CR, RCC_CR_HSEON);
        do {
            status = read_reg_value(r_CORTEX_M_RCC_CR) & RCC_CR_HSERDY;
            StartUpCounter++;
        } while ((status == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));
    } else {
        /* Enable HSI */
        set_reg_bits(r_CORTEX_M_RCC_CR, RCC_CR_HSION);
        do {
            status = read_reg_value(r_CORTEX_M_RCC_CR) & RCC_CR_HSIRDY;
            StartUpCounter++;
        } while ((status == 0) && (StartUpCounter != HSI_STARTUP_TIMEOUT));
    }

    if (status != RESET) {
        /* Enable high performance mode, System frequency up to 168 MHz */
        set_reg_bits(r_CORTEX_M_RCC_APB1ENR, RCC_APB1ENR_PWREN);
        /*
         * This bit controls the main internal voltage regulator output
         * voltage to achieve a trade-off between performance and power
         * consumption when the device does not operate at the maximum
         * frequency. (DocID018909 Rev 15 - page 141)
         * PWR_CR_VOS = 1 => Scale 1 mode (default value at reset)
         */
        set_reg_bits(r_CORTEX_M_PWR_CR, PWR_CR_VOS_Msk);

        /* Set clock dividers */
        set_reg_bits(r_CORTEX_M_RCC_CFGR, PROD_HCLK);
        set_reg_bits(r_CORTEX_M_RCC_CFGR, PROD_PCLK2);
        set_reg_bits(r_CORTEX_M_RCC_CFGR, PROD_PCLK1);

        if (enable_pll) {
            /* Configure the main PLL */
            if (enable_hse) {
                write_reg_value(r_CORTEX_M_RCC_PLLCFGR, PROD_PLL_M | (PROD_PLL_N << 6)
                    | (((PROD_PLL_P >> 1) - 1) << 16)
                    | (RCC_PLLCFGR_PLLSRC_HSE) | (PROD_PLL_Q << 24));
            } else {
                write_reg_value(r_CORTEX_M_RCC_PLLCFGR, PROD_PLL_M | (PROD_PLL_N << 6)
                    | (((PROD_PLL_P >> 1) - 1) << 16)
                    | (RCC_PLLCFGR_PLLSRC_HSI) | (PROD_PLL_Q << 24));
            }

            /* Enable the main PLL */
            set_reg_bits(r_CORTEX_M_RCC_CR, RCC_CR_PLLON);

            /* Wait till the main PLL is ready */
            while ((read_reg_value(r_CORTEX_M_RCC_CR) & RCC_CR_PLLRDY) == 0)
                continue;
        }

        /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
        write_reg_value(r_CORTEX_M_FLASH_ACR, FLASH_ACR_ICEN
                        | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS);

        if (enable_pll) {
            /* Select the main PLL as system clock source */
            clear_reg_bits(r_CORTEX_M_RCC_CFGR, RCC_CFGR_SW);
            set_reg_bits(r_CORTEX_M_RCC_CFGR, RCC_CFGR_SW_PLL);

            /* Wait till the main PLL is used as system clock source */
            while ((read_reg_value(r_CORTEX_M_RCC_CFGR) & (uint32_t) RCC_CFGR_SWS)
                    != RCC_CFGR_SWS_PLL)
                continue;
        }

    } else {
        /* If HSE/I fails to start-up, the application will have wrong
         * clock configuration. User can add here some code to deal
         * with this error.
         */
    }
}
