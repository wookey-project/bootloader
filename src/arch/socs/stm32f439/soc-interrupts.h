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
#ifndef SOC_IRQ_
#define SOC_IRQ_

#include "types.h"

/*
** That structure points to the saved registers on the caller
** (mostly a user task) stack.
*/

typedef struct {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11, lr;
    uint32_t r0, r1, r2, r3, r12, prev_lr, pc, xpsr;
} __attribute__ ((packed)) stack_frame_t;

/*
** This permit the bellowing table to be used by a given IRQ handler
** e.g. s_irq[USART1_IRQ].fn give the custom handler pointer if it exists
*/
typedef enum {
    ESTACK = 0,
    RESET_IRQ,
    NMI_IRQ,
    HARDFAULT_IRQ,
    MEMMANAGE_IRQ,
    BUSFAULT_IRQ,
    USAGEFAULT_IRQ,
    VOID1_IRQ,
    VOID2_IRQ,
    VOID3_IRQ,
    VOID4_IRQ,
    SVC_IRQ,
    DEBUGON_IRQ,
    VOID5_IRQ,
    PENDSV_IRQ,
    SYSTICK_IRQ,
    WWDG_IRQ,
    PVD_IRQ,
    TAMP_STAMP_IRQ,
    RTC_WKUP_IRQ,
    FLASH_IRQ,
    RCC_IRQ,
    EXTI0_IRQ,
    EXTI1_IRQ,
    EXTI2_IRQ,
    EXTI3_IRQ,
    EXTI4_IRQ,
    DMA1_Stream0_IRQ,
    DMA1_Stream1_IRQ,
    DMA1_Stream2_IRQ,
    DMA1_Stream3_IRQ,
    DMA1_Stream4_IRQ,
    DMA1_Stream5_IRQ,
    DMA1_Stream6_IRQ,
    ADC_IRQ,
    CAN1_TX_IRQ,
    CAN1_RX0_IRQ,
    CAN1_RX1_IRQ,
    CAN1_SCE_IRQ,
    EXTI9_5_IRQ,
    TIM1_BRK_TIM9_IRQ,
    TIM1_UP_TIM10_IRQ,
    TIM1_TRG_COM_TIM11_IRQ,
    TIM1_CC_IRQ,
    TIM2_IRQ,
    TIM3_IRQ,
    TIM4_IRQ,
    I2C1_EV_IRQ,
    I2C1_ER_IRQ,
    I2C2_EV_IRQ,
    I2C2_ER_IRQ,
    SPI1_IRQ,
    SPI2_IRQ,
    USART1_IRQ,
    USART2_IRQ,
    USART3_IRQ,
    EXTI15_10_IRQ,
    RTC_Alarm_IRQ,
    OTG_FS_WKUP_IRQ,
    TIM8_BRK_TIM12_IRQ,
    TIM8_UP_TIM13_IRQ,
    TIM8_TRG_COM_TIM14_IRQ,
    TIM8_CC_IRQ,
    DMA1_Stream7_IRQ,
    FSMC_IRQ,
    SDIO_IRQ,
    TIM5_IRQ,
    SPI3_IRQ,
    UART4_IRQ,
    UART5_IRQ,
    TIM6_DAC_IRQ,
    TIM7_IRQ,
    DMA2_Stream0_IRQ,
    DMA2_Stream1_IRQ,
    DMA2_Stream2_IRQ,
    DMA2_Stream3_IRQ,
    DMA2_Stream4_IRQ,
    ETH_IRQ,
    ETH_WKUP_IRQ,
    CAN2_TX_IRQ,
    CAN2_RX0_IRQ,
    CAN2_RX1_IRQ,
    CAN2_SCE_IRQ,
    OTG_FS_IRQ,
    DMA2_Stream5_IRQ,
    DMA2_Stream6_IRQ,
    DMA2_Stream7_IRQ,
    USART6_IRQ,
    I2C3_EV_IRQ,
    I2C3_ER_IRQ,
    OTG_HS_EP1_OUT_IRQ,
    OTG_HS_EP1_IN_IRQ,
    OTG_HS_WKUP_IRQ,
    OTG_HS_IRQ,
    DCMI_IRQ,
    CRYP_IRQ,
    HASH_RNG_IRQ,
    FPU_IRQ,
} e_irq_id;



#endif /*!SOC_IRQ_ */
