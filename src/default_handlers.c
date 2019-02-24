#include "autoconf.h"
#include "soc-interrupts.h"
#include "soc-nvic.h"
#include "debug.h"
#include "soc-scb.h"
#include "main.h"


/*
** Generic handlers. This handlers can be overloaded later if needed.
*/

static inline void exti_handle_line(uint8_t        exti_line,
                                    uint8_t        irq,
                                    stack_frame_t *stack_frame __attribute__((unused)))
{
    gpioref_t   kref;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    /* No possible typecasting from uint8_t to :4 */
    kref.pin = exti_line;
#pragma GCC diagnostic pop

    /* Clear the EXTI pending bit for this line */
    soc_exti_clear_pending(kref.pin);

    /* Get back the configured GPIO port for this line */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    /* No possible typecasting from uint8_t to :4 */
    kref.port = soc_exti_get_syscfg_exticr_port(kref.pin);
#pragma GCC diagnostic pop

    NVIC_ClearPendingIRQ((uint32_t)(irq - 0x10));
#ifdef CONFIG_FIRMWARE_DFU
    exti_button_handler(irq, 0, 0);
#endif

}


stack_frame_t *exti_handler(stack_frame_t * stack_frame)
{
    uint8_t  int_num;
    uint32_t pending_lines = 0;

    /*
     * It's sad to get back again the IRQ here, but as a generic kernel
     * IRQ handler, the IRQ number is not passed as first argument
     * Only postpone_isr get it back.
     * TODO: give irq as first argument of *all* handler ?
     */
    interrupt_get_num(int_num);
    int_num &= 0x1ff;

    switch (int_num) {
        /* EXTI0: pin 0 */
        case EXTI0_IRQ: {
            exti_handle_line(0, int_num, stack_frame);
            break;
        }
        /* EXTI0: pin 1 */
        case EXTI1_IRQ: {
            exti_handle_line(1, int_num, stack_frame);
            break;
        }
        /* EXTI0: pin 2 */
        case EXTI2_IRQ: {
            exti_handle_line(2, int_num, stack_frame);
            break;
        }
        /* EXTI0: pin 3 */
        case EXTI3_IRQ: {
            exti_handle_line(3, int_num, stack_frame);
            break;
        }
        /* EXTI0: pin 4 */
        case EXTI4_IRQ: {
            exti_handle_line(4, int_num, stack_frame);
            break;
        }
        /* EXTI0: pin 5 to 9 */
        case EXTI9_5_IRQ: {
            pending_lines = soc_exti_get_pending_lines(int_num);
            for (uint8_t i = 0; i < 5; ++i) {
                if (pending_lines & (uint32_t)(0x1 << i)) {
                     exti_handle_line((uint8_t)(5 + i), int_num, stack_frame);
                }
            }
            break;
        }
        /* EXTI0: pin 10 to 15 */
        case EXTI15_10_IRQ:
            pending_lines = soc_exti_get_pending_lines(int_num);
            for (uint8_t i = 0; i < 6; ++i) {
                if (pending_lines & (uint32_t)(0x1 << i)) {
                     exti_handle_line((uint8_t)(10 + i), int_num, stack_frame);
                }
            }
            break;
    }
    return stack_frame;
}




stack_frame_t *HardFault_Handler(stack_frame_t * frame)
{
    uint32_t    cfsr = *((uint32_t *) r_CORTEX_M_SCB_CFSR);
    uint32_t    hfsr = *((uint32_t *) r_CORTEX_M_SCB_HFSR);
    uint32_t   *p;
    int         i;
#ifdef KERNEL
    task_t     *current_task = sched_get_current();
    if (!current_task) {
        /* This happend when hardfaulting before sched module initialization */
        dbg_log("\nEarly kernel hard fault\n  scb.hfsr %x  scb.cfsr %x\n", hfsr, cfsr);
    } else {
        dbg_log("\nHard fault from %s\n  scb.hfsr %x  scb.cfsr %x\n", current_task->name, hfsr, cfsr);
    }
#else
    dbg_log("\nHard fault\n  scb.hfsr %x  scb.cfsr %x\n", hfsr, cfsr);
#endif
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
#ifdef KERNEL
    if (frame_is_kernel((physaddr_t)frame)) {
        panic("Oops! Kernel panic!\n");
    }
    /*
     * here current_task can't be null as the frame is user
     * (i.e. the sched module is already started)
     */
    current_task->state[current_task->mode] = TASK_STATE_FAULT;
    request_schedule();
#else
    /* Non kernel mode (e.g. loader mode) */
    panic("Oops! Kernel panic!\n");
#endif
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
    if (int_num == EXTI0_IRQ) {
        exti_handler(stack_frame);
    }
    if (int_num == EXTI1_IRQ) {
        exti_handler(stack_frame);
    }
    if (int_num == EXTI2_IRQ) {
        exti_handler(stack_frame);
    }
    if (int_num == EXTI3_IRQ) {
        exti_handler(stack_frame);
    }
    if (int_num == EXTI4_IRQ) {
        exti_handler(stack_frame);
    }
    if (int_num == EXTI9_5_IRQ) {
        exti_handler(stack_frame);
    }
    if (int_num == EXTI15_10_IRQ) {
        exti_handler(stack_frame);
    }

    asm volatile
       ("mov r1, #0" ::: "r1");

    return stack_frame;
}
#ifdef __clang__
# pragma clang optimize on
# pragma clang diagnostic pop
#endif
