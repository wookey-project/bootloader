/*
 *
 * Copyright 2018 The wookey project team <wookey@ssi.gouv.fr>
 *   - Ryad     Benadjila
 *   - Arnauld  Michelizza
 *   - Mathieu  Renard
 *   - Philippe Thierry
 *   - Philippe Trebuchet
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * ur option) any later version.
 *
 * This package is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this package; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
#include "autoconf.h"
#include "soc-rng.h"
#include "automaton.h"
#include "hash.h"
#include "main.h"

volatile uint64_t controlflow;
volatile uint64_t currentflow;


/*
 * Constified (in .rodata) dereferencing structure to associate
 * a secured, discretionary automaton state for if control code
 * with an automaton cell
 */
static const struct automaton_state_correspondency {
    loader_state_t state;
    uint8_t        cell;
} automaton_state_accessor[] = {
    { LOADER_START,       0 },
    { LOADER_INIT,        1 },
    { LOADER_RDPCHECK,    2 },
    { LOADER_DFUWAIT,     3 },
    { LOADER_SELECTBANK,  4 },
    { LOADER_HDRCRC,      5 },
    { LOADER_FWINTEGRITY, 6 },
    { LOADER_FLASHLOCK,   7 },
    { LOADER_BOOTFW,      8 },
    { LOADER_ERROR,       9 },
    { LOADER_SECBREACH,   10 }
};

static uint8_t automaton_get_cell(loader_state_t state)
{
    uint8_t cellid = 10; /* defaulting to LOADER_SECBREACH */
    for (uint8_t i = 0; i < sizeof(automaton_state_accessor)/sizeof(struct automaton_state_correspondency); i++) {
        /* double if protection */
        if (automaton_state_accessor[i].state == state &&
            !(automaton_state_accessor[i].state != state)) {
            cellid = automaton_state_accessor[i].cell;
            goto end;
        }
    }
end:
    return cellid;
}


/****************************************************************
 * loader state automaton formal definition and associate utility
 * functions
 ***************************************************************/

static loader_state_t state;

/*
 * all allowed transitions and target states for each current state
 * empty fields are set as 0xff/0xff for request/state couple, which is
 * an inexistent state and request
 *
 * This table associate each state of the DFU automaton with up to
 * 5 potential allowed transitions/next_state couples. This permit to
 * easily detect:
 *    1) authorized transitions, based on the current state
 *    2) next state, based on the current state and current transition
 *
 * If the next_state for the current transision is keeped to 0xff, this
 * means that the current transition for the current state may lead to
 * multiple next state depending on other informations. In this case,
 * the transition handler has to handle this manually.
 */

#define MAX_TRANSITION_STATE 8

/*
 * Association between a request and a transition to a next state. This couple
 * depend on the current state and is use in the following structure
 */
typedef struct loader_operation_code_transition {
    loader_request_t request;
    loader_state_t   target_state;
} loader_operation_code_transition_t;


static const struct {
    loader_state_t state;
    loader_operation_code_transition_t req_trans[MAX_TRANSITION_STATE];
} loader_automaton[] = {
    {LOADER_START, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_INIT, LOADER_INIT},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                  }
    },
    {LOADER_INIT, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_RDPCHECK, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_DFUCHECK, LOADER_DFUWAIT},
                 {LOADER_REQ_SELECTBANK, LOADER_SELECTBANK},
                 {LOADER_REQ_CRCCHECK, LOADER_HDRCRC},
                 {LOADER_REQ_INTEGRITYCHECK, LOADER_FWINTEGRITY},
                 {LOADER_REQ_FLASHLOCK, LOADER_FLASHLOCK},
                 {LOADER_REQ_BOOT, LOADER_BOOTFW},
                 }
     },
    {LOADER_DFUWAIT, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_SELECTBANK, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_HDRCRC, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_FWINTEGRITY, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_FLASHLOCK, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_BOOTFW, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_ERROR, {
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_SECBREACH, {
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
};

/*
 * control flow sequence that must be respected by the automaton
 */
#define LOADER_CONTROLFLOW_LENGTH 14

static const uint32_t loader_controlflow[] = {
    LOADER_START,
    LOADER_INIT,
    LOADER_RDPCHECK,
    LOADER_DFUWAIT,
    LOADER_RDPCHECK,
    LOADER_SELECTBANK,
    LOADER_RDPCHECK,
    LOADER_HDRCRC,
    LOADER_RDPCHECK,
    LOADER_FWINTEGRITY,
    LOADER_RDPCHECK,
    LOADER_FLASHLOCK,
    LOADER_RDPCHECK,
    LOADER_BOOTFW,
};

/**********************************************
 * loader getters and setters
 *********************************************/

loader_state_t loader_get_state(void)
{
    return state;
}


void loader_init_controlflow(void)
{
    union u_controlflow {
        volatile uint64_t *cf64;
        volatile uint32_t *cf32;
    };
    union u_controlflow cf_init;
    cf_init.cf64 = &controlflow;

    /* set the all 64 bits of the controlflow variable */
    soc_get_random(&(cf_init.cf32[0]));
    soc_get_random(&(cf_init.cf32[1]));

#if CONFIG_LOADER_EXTRA_DEBUG
    dbg_log("initialize controlflow to (long long) %ll\n", controlflow);
    dbg_flush();
#endif
    /* depending on the value of controlflow at start, the value of the
     * successive calculation of currentflow may generate an uint64_t overflow,
     * but this is not a problem, because the value itself means nothing while
     * the two calculated values are the same. If FC detect the overflow, we
     * cas set the MBS bits of controlflow to 0 to avoid any overflow risk. */
    currentflow = controlflow;
}

static inline void update_controlflowvar(volatile uint64_t *var, uint32_t value)
{
    /* atomic local update of the control flow var.
     * FIXME: addition is problematic because it does not ensure that
     * previous states has been executed in the correct order, but only
     * that they **all** have been executed. Though, as each state check for
     * the control flow, the order is checked. Althgouh, a mathematical
     * primitive ensuring variation (successive CRC ? other ?) would be better. */
    *var += hash_state(value);

}



secbool loader_calculate_flowstate(loader_state_t prevstate,
                                   loader_state_t nextstate)
{
    /*
     * Here, we recalculate from scratch, based on the prevstate/nextstate pair
     * (which **must** be unique) to current controlflow value.
     * We use the flow control sequence set in .rodata to successively increment
     * myflow up to the state pair we should be on.
     * The caller function can then compare the result of this function with
     * loader_update_flowstate(). If the results are the same, the control flow is
     * keeped. If not, the control flow is corrupted.
     */
    /* 1. init myflow to seed */
    uint8_t i = 0;
    uint64_t myflow = (uint64_t)controlflow;
    /* 2. starting to init state, for each state pairs different from current one, add
     * first state of the pair to myflow */
    for (i = 0; i < LOADER_CONTROLFLOW_LENGTH - 1; ++i) {
        update_controlflowvar(&myflow, loader_controlflow[i]);
        if (loader_controlflow[i] == prevstate && loader_controlflow[i + 1] == nextstate) {
            break;
        }
    }
    /* 3. found state pair ? add nextstate en return */
    update_controlflowvar(&myflow, nextstate);

#if CONFIG_LOADER_EXTRA_DEBUG
    dbg_log("%s: result of online calculation (%d sequences) is (long long) %ll\n", __func__, i, myflow);
    dbg_flush();
#endif
    /* TODO: how to harden u64 comparison ? */
    if (myflow != currentflow) {
        /* control flow error detected */
        dbg_log("Error in control flow ! Fault injection detected !\n");
        return secfalse;
    }
    return sectrue;
}

void loader_update_flowstate(loader_state_t nextstate)
{
    /* here, we update current flow in order to add the current state to the previous
     * flow sequence, by adding its state value to the currentflow variable */
    /* calculation may be more complex to avoid any collision risk. Is there a way
     * through cryptographic calculation on state value concatenation to avoid any
     * risk ? */
    update_controlflowvar(&currentflow, nextstate);
#if CONFIG_LOADER_EXTRA_DEBUG
    dbg_log("%s: update controlflow to (long long) %ll\n", __func__, currentflow);
    dbg_flush();
#endif

}

/*
 * This function is eligible in both main thread and ISR
 * mode (through trigger execution).
 */
void loader_set_state(const loader_state_t new_state)
{

    /* double if protection */
    if (new_state == 0xff &&
        !(new_state != 0xff)) {
        dbg_log("%s: PANIC! this should never arise !", __func__);
        dbg_flush();
        loader_set_state(LOADER_ERROR);
        return;
    }
#if CONFIG_LOADER_EXTRA_DEBUG
    dbg_log("%s: state: %x => %x\n", __func__, state, new_state);
    dbg_flush();
#endif
    state = new_state;
}

/******************************************************
 * loader automaton management function (transition and
 * state check)
 *****************************************************/

/*!
 * \brief return the next automaton state
 *
 * The next state is returned depending on the current state
 * and the current request. In some case, it can be 0xff if multiple
 * next states are possible.
 *
 * \param current_state the current automaton state
 * \param request       the current transition request
 *
 * \return the next state, or 0xff
 */
loader_state_t loader_next_state(const loader_state_t current_state,
                                 const loader_request_t request)
{
    for (uint8_t i = 0; i < MAX_TRANSITION_STATE; ++i) {
        /* double if protection */
        if (loader_automaton[automaton_get_cell(current_state)].req_trans[i].request == request &&
            !(loader_automaton[automaton_get_cell(current_state)].req_trans[i].request != request)) {
            return (loader_automaton[automaton_get_cell(current_state)].req_trans[i].target_state);
        }
    }
    /* fallback, no corresponding request found for  this state */
    return 0xff;
}

/*!
 * \brief Specify if the current request is valid for the current state
 *
 * \param current_state the current automaton state
 * \param request       the current transition request
 *
 * \return true if the transition request is allowed for this state, or false
 */
secbool loader_is_valid_transition(const loader_state_t current_state,
                                   const loader_request_t request)
{
    for (uint8_t i = 0; i < MAX_TRANSITION_STATE; ++i) {
        /* double if protection */
        if (loader_automaton[automaton_get_cell(current_state)].req_trans[i].request == request &&
            !(loader_automaton[automaton_get_cell(current_state)].req_trans[i].request != request)) {
            return sectrue;
        }
    }
    /*
     * Didn't find any request associated to current state. This is not a
     * valid transition. We should stall the request.
     */
    dbg_log("%s: invalid transition from state %d, request %d\n", __func__,
           current_state, request);
    loader_set_state(LOADER_ERROR);
    return secfalse;
}
