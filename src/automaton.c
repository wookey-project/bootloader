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
#include "automaton.h"
#include "main.h"


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

#define MAX_TRANSITION_STATE 3

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
                  }
    },
    {LOADER_INIT, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_RDPCHECK, LOADER_RDPCHECK},
                 }
     },
    {LOADER_RDPCHECK, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_DFUCHECK, LOADER_DFUWAIT},
                 }
     },
    {LOADER_DFUWAIT, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_SELECTBANK, LOADER_SELECTBANK},
                 }
     },
    {LOADER_SELECTBANK, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_CRCCHECK, LOADER_HDRCRC},
                 }
     },
    {LOADER_HDRCRC, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_INTEGRITYCHECK, LOADER_FWINTEGRITY},
                 }
     },
    {LOADER_FWINTEGRITY, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_FLASHLOCK, LOADER_FLASHLOCK},
                 }
     },
    {LOADER_FLASHLOCK, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {LOADER_REQ_BOOT, LOADER_BOOTFW},
                 }
     },
    {LOADER_BOOTFW, {
                 {LOADER_REQ_ERROR, LOADER_ERROR},
                 {LOADER_REQ_SECBREACH, LOADER_SECBREACH},
                 {0xff, 0xff},
                 }
     },
    {LOADER_ERROR, {
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
    {LOADER_SECBREACH, {
                 {0xff, 0xff},
                 {0xff, 0xff},
                 {0xff, 0xff},
                 }
     },
};

/**********************************************
 * loader getters and setters
 *********************************************/

loader_state_t loader_get_state(void)
{
    return state;
}


/*
 * This function is eligible in both main thread and ISR
 * mode (through trigger execution).
 */
void loader_set_state(const loader_state_t new_state)
{
    if (new_state == 0xff) {
        dbg_log("%s: PANIC! this should never arise !", __func__);
        dbg_flush();
        loader_set_state(LOADER_ERROR);
        return;
    }
#if LOADER_DEBUG
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
        if (loader_automaton[current_state].req_trans[i].request == request) {
            return (loader_automaton[current_state].req_trans[i].target_state);
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
        if (loader_automaton[current_state].req_trans[i].request == request) {
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
