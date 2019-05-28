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
#ifndef M4_CORE_
#define M4_CORE_

#define MAIN_CLOCK_FREQUENCY 168000000
#define MAIN_CLOCK_FREQUENCY_MS 168000
#define MAIN_CLOCK_FREQUENCY_US 168

#define INITIAL_STACK 0x1000b000

#define INT_STACK_BASE KERN_STACK_BASE - 8192   /* same for FIQ & IRQ by now */
#define ABT_STACK_BASE INT_STACK_BASE - 4096
#define SYS_STACK_BASE ABT_STACK_BASE - 4096
#define UDF_STACK_BASE SYS_STACK_BASE - 4096

#define MODE_CLEAR 0xffffffe0

static inline void core_processor_init_modes(void)
{
    /*
     * init msp for kernel, this is needed in order to make IT return to SVC mode
     * in thread mode (LR=0xFFFFFFF9) working (loading the good msp value from the SPSR)
     */
    asm volatile ("msr msp, %0\n\t"::"r" (INITIAL_STACK):);
}

#endif                          /*!M4_CORE_ */
