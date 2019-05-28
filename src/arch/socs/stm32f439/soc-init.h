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
#ifndef SOC_INIT_H
#define SOC_INIT_H

#include "types.h"
#include "soc-rcc.h"

#define RESET	0
#define SET	1

#if !defined (HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT	((uint16_t)0x0500)
#endif                          /* !HSE_STARTUP_TIMEOUT */

#if !defined (HSI_STARTUP_TIMEOUT)
#define HSI_STARTUP_TIMEOUT	((uint16_t)0x0500)
#endif                          /* !HSI_STARTUP_TIMEOUT */

#define STM32F429

//#define PROD_ENABLE_HSE
#define PROD_ENABLE_PLL 1

#define PROD_PLL_M     16
#define PROD_PLL_N     336
#define PROD_PLL_P     2
#define PROD_PLL_Q     7

#define PROD_HCLK      RCC_CFGR_HPRE_DIV1
#define PROD_PCLK2     RCC_CFGR_HPRE2_DIV2
#define PROD_PCLK1     RCC_CFGR_HPRE1_DIV4

#define PROD_CLOCK_APB1  42000000 // MHz
#define PROD_CLOCK_APB2  84000000 // MHz

#define PROD_CORE_FREQUENCY 168000

#define LDR_BASE 0x08000000
#define VTORS_SIZE 0x188

void set_vtor(uint32_t);
void system_init(uint32_t);

#endif/*!SOC_INIT_H*/
