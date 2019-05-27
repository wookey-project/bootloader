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
#include "soc-interrupts.h"
#include "soc-nvic.h"
#include "debug.h"
#include "soc-scb.h"
#include "main.h"


/*
** Generic handlers. This handlers can be overloaded later if needed.
*/
#define interrupt_get_num(intr) { asm volatile ("mrs r1, ipsr\n\t" \
                                                "mov %0, r1\n\t" \
                                              : "=r" (intr) :: "r1" ); }



stack_frame_t *HardFault_Handler(stack_frame_t * frame)
{
    uint32_t    cfsr = *((volatile uint32_t *) r_CORTEX_M_SCB_CFSR);
    uint32_t    hfsr = *((volatile uint32_t *) r_CORTEX_M_SCB_HFSR);
    uint32_t   *p;
    int         i;

    dbg_log("\nHard fault\n  scb.hfsr %x  scb.cfsr %x\n", hfsr, cfsr);
    dbg_flush();

    dbg_log("-- registers (frame at %x, EXC_RETURN  %x)\n", frame, frame->lr);
    dbg_log("  r0  %x\t r1  %x\t r2  %x\t r3  %x\n",
        frame->r0, frame->r1, frame->r2, frame->r3);
    dbg_log("  r4  %x\t r5  %x\t r6  %x\t r7  %x\n",
        frame->r4, frame->r5, frame->r6, frame->r7);
    dbg_log("  r8  %x\t r9  %x\t r10 %x\t r11 %x\n",
        frame->r8, frame->r9, frame->r10, frame->r11);
    dbg_log("  r12 %x\t pc  %x\t lr %x\n",
        frame->r12, frame->pc, frame->prev_lr);
    dbg_flush();

    p = (uint32_t*) ((uint32_t) frame & 0xfffffff0);
    dbg_log("-- stack trace\n");
    for (i=0;i<8;i++) {
        dbg_log("  %x: %x  %x  %x  %x\n", p, p[0], p[1], p[2], p[3]);
        dbg_flush();
        p = p + 4;
    }
    /* Non kernel mode (e.g. loader mode) */
    panic("Oops! Kernel panic!\n");
    return frame;
}

/*
 * To avoid purging the Default_Handler stack (making irq_enter/irq_return
 * fail), all the default handler algorithmic MUST be done in a subframe (i.e.
 * in a child function)
 */
/* [RB] FIXME: clang's warning about called function from IRQ could hide something ...
 * [RB] TODO: investigate, related to https://bugs.llvm.org/show_bug.cgi?id=35527
 *            https://bugs.llvm.org/show_bug.cgi?id=35528
 *            https://reviews.llvm.org/D28820
 */
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wextra"
# pragma clang optimize off
#endif
__ISR_HANDLER stack_frame_t *Default_SubHandler(stack_frame_t * stack_frame)
{
    uint8_t         int_num;

    /* Getting the IRQ number */
    interrupt_get_num(int_num);
    int_num &= 0x1ff;

    if (int_num == 3) {
        HardFault_Handler(stack_frame);
    }

    asm volatile
       ("mov r1, #0" ::: "r1");

    return stack_frame;
}
#ifdef __clang__
# pragma clang optimize on
# pragma clang diagnostic pop
#endif
