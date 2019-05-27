#include "autoconf.h"
#include "soc-interrupts.h"
#include "soc-nvic.h"
#include "debug.h"
#include "soc-scb.h"
#include "main.h"


/*
** Generic handlers. This handlers can be overloaded later if needed.
*/


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
