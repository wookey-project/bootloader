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
#ifndef M4_CPU
#define M4_CPU

#include "types.h"

__INLINE __attribute__ ((always_inline))
void core_write_psp(void *ptr)
{
    asm volatile ("MSR psp, %0\n\t"::"r" (ptr));
}

__INLINE __attribute__ ((always_inline))
void core_write_msp(void *ptr)
{
    asm volatile ("MSR msp, %0\n\t"::"r" (ptr));
}

__INLINE __attribute__ ((always_inline))
void *core_read_psp(void)
{
    void *result = NULL;
    asm volatile ("MRS %0, psp\n\t":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
void *core_read_msp(void)
{
    void *result = NULL;
    asm volatile ("MRS %0, msp\n\t":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
void enable_irq(void)
{
    __ASM volatile ("cpsie i; isb":::"memory");
}

__INLINE __attribute__ ((always_inline))
void disable_irq(void)
{
    __ASM volatile ("cpsid i":::"memory");

}

__INLINE __attribute__ ((always_inline))
void full_memory_barrier(void)
{
    __ASM volatile ("dsb; isb":::);
}

__INLINE __attribute__ ((always_inline))
uint32_t __get_CONTROL(void)
{
    uint32_t result;

    __ASM volatile ("MRS %0, control":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
void __set_CONTROL(uint32_t control)
{
    __ASM volatile ("MSR control, %0"::"r" (control));
}

__INLINE __attribute__ ((always_inline))
uint32_t __get_IPSR(void)
{
    uint32_t result;

    __ASM volatile ("MRS %0, ipsr":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
uint32_t __get_APSR(void)
{
    uint32_t result;

    __ASM volatile ("MRS %0, apsr":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
uint32_t __get_xPSR(void)
{
    uint32_t result;

    __ASM volatile ("MRS %0, xpsr":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
uint32_t __get_PRIMASK(void)
{
    uint32_t result;

    __ASM volatile ("MRS %0, primask":"=r" (result));
    return (result);
}

__INLINE __attribute__ ((always_inline))
void __set_PRIMASK(uint32_t priMask)
{
    __ASM volatile ("MSR primask, %0"::"r" (priMask));
}

__INLINE void wait_for_interrupt(void)
{
    asm volatile ("wfi");
}

#if defined(__CC_ARM)
/* No Operation */
#define __NOP		__nop
/* Instruction Synchronization Barrier */
#define __ISB()	__isb(0xF)
/* Data Synchronization Barrier */
#define __DSB()	__dsb(0xF)
/* Data Memory Barrier */
#define __DMB()	__dmb(0xF)
/* Reverse byte order (32 bit) */
#define __REV		__rev
/* Reverse byte order (16 bit) */
static __INLINE __ASM uint32_t __REV16(uint32_t value)
{
rev16 r0, r0 bx lr}
/* Breakpoint */
#define __BKPT	__bkpt
#elif defined(__GNUC__)

static inline __attribute__ ((always_inline))
void __NOP(void)
{
    __asm__ volatile ("nop");
}

static inline __attribute__ ((always_inline))
void __ISB(void)
{
    __asm__ volatile ("isb");
}

static inline __attribute__ ((always_inline))
void __DSB(void)
{
    __asm__ volatile ("dsb");
}

static inline __attribute__ ((always_inline))
void __DMB(void)
{
    __asm__ volatile ("dmb");
}

static inline __attribute__ ((always_inline))
uint32_t __REV(uint32_t value)
{
    uint32_t result;
    __asm__ volatile ("rev %0, %1":"=r" (result):"r"(value));
    return result;
}

static inline __attribute__ ((always_inline))
uint32_t __REV16(uint32_t value)
{
    uint32_t result;
    __asm__ volatile ("rev16 %0, %1":"=r" (result):"r"(value));
    return result;
}

static inline __attribute__ ((always_inline))
void __BKPT(void)
{
    __asm__ volatile ("bkpt");
}
#endif

#endif                          /*!M4_CPU */
