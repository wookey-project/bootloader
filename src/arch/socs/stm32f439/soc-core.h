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

#ifndef SOC_CORE_H
#define SOC_CORE_H

#include "soc-dwt.h"

#if !defined  (__FPU_PRESENT)
#define __FPU_PRESENT             1 /*!< FPU present                                   */
#endif                          /* __FPU_PRESENT */
#define __CM4_REV                 0x0001    /*!< Core revision r0p1                            */
#define __MPU_PRESENT             1 /*!< STM32F4XX provides an MPU                     */
#define __NVIC_PRIO_BITS          4 /*!< STM32F4XX uses 4 Bits for the Priority Levels */

/* Memory mapping of Cortex-M3/M4 Hardware */
#define CODE_BASE                           ((uint32_t) 0x00000000) /* Internal Flash Base Address */
#define FLASH_BASE                          ((uint32_t) 0x08000000)
#define SRAM_BASE                           ((uint32_t) 0x20000000) /* Internal SRAM Base Address */
#define SRAM_BASE_BITBANG_BASE              ((uint32_t) 0x22000000) /* From (uint32_t)0x22000000UL to (uint32_t)0x23FFFFFFUL */
#define PERIPH_BASE                         ((uint32_t) 0x40000000) /* Peripheral Base Address  */
#define PERIPH_BASE_BITBANG1_BASE           ((uint32_t) 0x40000000) /* From (uint32_t)0x40000000UL to (uint32_t)0x400FFFFFUL */
#define PERIPH_BASE_BITBANG2_BASE           ((uint32_t) 0x42000000) /* From (uint32_t)0x42000000UL to (uint32_t)0x43FFFFFFUL */
#define EXTERNAL_RAM                        ((uint32_t) 0x60000000) /* From (uint32_t)0x60000000UL to (uint32_t)0x9FFFFFFFUL */
#define EXTERNAL_DEVICE                     ((uint32_t) 0xA0000000) /* From (uint32_t)0xA0000000UL to (uint32_t)0xDFFFFFFFUL */
#define SCS_BASE                            ((uint32_t) 0xE000E000) /* System Control Space Base Address                 */
#define ITM_BASE                            ((uint32_t) 0xE0000000) /* TODO ITM Base Address                             */

#define CoreDebug_BASE                      ((uint32_t) 0xE000EDF0) /* Core Debug Base Address                           */

#define SysTick_BASE                        (SCS_BASE + (uint32_t) 0x010)   /* SysTick Base Address                              */
#define NVIC_BASE                           (SCS_BASE + (uint32_t) 0x100)   /* NVIC Base Address                                 */
#define SCB_BASE                            (SCS_BASE + (uint32_t) 0xd00)   /* System Control Block Base Address                 */

#if (__FPU_PRESENT == 1)
#define CPACR_BASE                          (SCB_BASE + (uint32_t) 0xd88)   /* Floating point unit coprocessor access control
                                                                               register Base Address  */
#else
#warning "Compiler generates FPU instructions for a device without an FPU (check __FPU_PRESENT)"
#define __FPU_USED       0
#endif

#if (__MPU_PRESENT == 1)
#define MPU_BASE          (SCS_BASE +  (uint32_t) 0x0D90)   /* Memory Protection Unit */
#else
#warning "Using an MPU on device with no MPU (check __MPU_PRESENT)"
#define __MPU_USED       0
#endif

#define NVIC_STIR_BASE                      (SCS_BASE + (uint32_t) 0xf00)   /* NVIC_STIR Base Address                            */

#if (__FPU_PRESENT == 1)
#define FPU_BASE          (SCS_BASE +  (uint32_t) 0x0F30)
#else
#warning "Compiler generates FPU instructions for a device without an FPU (check __FPU_PRESENT)"
#define __FPU_USED       0
#endif

/*Peripheral memory map */
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000)
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x10000000)

#define RCC_BASE              (AHB1PERIPH_BASE + 0x3800)

/*APB1 peripherals */
#define TIM2_BASE             (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x0400)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x0800)
#define TIM5_BASE             (APB1PERIPH_BASE + 0x0C00)
#define TIM6_BASE             (APB1PERIPH_BASE + 0x1000)
#define TIM7_BASE             (APB1PERIPH_BASE + 0x1400)
#define TIM12_BASE            (APB1PERIPH_BASE + 0x1800)
#define TIM13_BASE            (APB1PERIPH_BASE + 0x1C00)
#define TIM14_BASE            (APB1PERIPH_BASE + 0x2000)
#define RTC_BASE              (APB1PERIPH_BASE + 0x2800)
#define WWDG_BASE             (APB1PERIPH_BASE + 0x2C00)
#define IWDG_BASE             (APB1PERIPH_BASE + 0x3000)
#define I2S2ext_BASE          (APB1PERIPH_BASE + 0x3400)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800)
#define SPI3_BASE             (APB1PERIPH_BASE + 0x3C00)
#define I2S3ext_BASE          (APB1PERIPH_BASE + 0x4000)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800)
#define I2C3_BASE             (APB1PERIPH_BASE + 0x5C00)
#define CAN1_BASE             (APB1PERIPH_BASE + 0x6400)
#define CAN2_BASE             (APB1PERIPH_BASE + 0x6800)
#define PWR_BASE              (APB1PERIPH_BASE + 0x7000)
#define DAC_BASE              (APB1PERIPH_BASE + 0x7400)

/*APB2 peripherals */
#define TIM1_BASE             (APB2PERIPH_BASE + 0x0000)
#define TIM8_BASE             (APB2PERIPH_BASE + 0x0400)
#define USART1_BASE           (APB2PERIPH_BASE + 0x1000)
#define USART6_BASE           (APB2PERIPH_BASE + 0x1400)
#define ADC1_BASE             (APB2PERIPH_BASE + 0x2000)
#define ADC2_BASE             (APB2PERIPH_BASE + 0x2100)
#define ADC3_BASE             (APB2PERIPH_BASE + 0x2200)
#define ADC_BASE              (APB2PERIPH_BASE + 0x2300)
//#define SDIO_BASE             (APB2PERIPH_BASE + 0x2C00)
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000)
#define SYSCFG_BASE           (APB2PERIPH_BASE + 0x3800)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x3C00)
#define TIM9_BASE             (APB2PERIPH_BASE + 0x4000)
#define TIM10_BASE            (APB2PERIPH_BASE + 0x4400)
#define TIM11_BASE            (APB2PERIPH_BASE + 0x4800)

/*AHB1 peripherals */
#define GPIOA_BASE            (AHB1PERIPH_BASE + 0x0000)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400)
#define GPIOC_BASE            (AHB1PERIPH_BASE + 0x0800)
#define GPIOD_BASE            (AHB1PERIPH_BASE + 0x0C00)
#define GPIOE_BASE            (AHB1PERIPH_BASE + 0x1000)
#define GPIOF_BASE            (AHB1PERIPH_BASE + 0x1400)
#define GPIOG_BASE            (AHB1PERIPH_BASE + 0x1800)
#define GPIOH_BASE            (AHB1PERIPH_BASE + 0x1C00)
#define GPIOI_BASE            (AHB1PERIPH_BASE + 0x2000)

#endif                          /* SOC_CORE_H */
