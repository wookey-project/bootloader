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
#ifndef AUTOMATON_H_
#define AUTOMATON_H_

#include "arch/cores/armv7-m/types.h"
#include "debug.h"

/*
 * Here we use discretionary values with at least 2 non-contigous separated
 * bits to handle loader state. The goal here is to protect the loader
 * automaton against physical attacks (corruption, bit flip...)
 * The effective association between the state and the automaton state is
 * hold in flash memory (.rodata) and not in RAM.
 * The goal is to physically lock RO the flash memory in order to protect
 * these data properly at early boot time.
 */
typedef enum loader_state {
    LOADER_START       = 0x00000003,
    LOADER_INIT        = 0x0000000c,
    LOADER_RDPCHECK    = 0x00000035,
    LOADER_DFUWAIT     = 0x000000ca,
    LOADER_SELECTBANK  = 0x00000350,
    LOADER_HDRCRC      = 0x00000ca3,
    LOADER_FWINTEGRITY = 0x000035cf,
    LOADER_FLASHLOCK   = 0x0000ca0c,
    LOADER_BOOTFW      = 0x00035c30,
    LOADER_ERROR       = 0x000ca3f3,
    LOADER_SECBREACH   = 0x0035cfcf,
} loader_state_t;

/*
 * transition requests, like states, there is multiple bits to
 * flip in order to switch from one transition to another.
 *
 * Swithching state and transitions together is requested to
 * avoid automaton transition validation autocheck making
 * physical attacks (bit flips) harder and harder.
 */
typedef enum loader_request {
    LOADER_REQ_INIT            = 0x03000003,
    LOADER_REQ_RDPCHECK        = 0x0500000c,
    LOADER_REQ_DFUCHECK        = 0x3a000035,
    LOADER_REQ_SELECTBANK      = 0x5c0000ca,
    LOADER_REQ_CRCCHECK        = 0xaf000350,
    LOADER_REQ_INTEGRITYCHECK  = 0xc3000ca3,
    LOADER_REQ_FLASHLOCK       = 0xf50035cf,
    LOADER_REQ_BOOT            = 0xfa00ca0c,
    LOADER_REQ_ERROR           = 0xfc0ca3f3,
    LOADER_REQ_SECBREACH       = 0xff35cfcf,
} loader_request_t;

loader_state_t loader_get_state(void);

void           loader_set_state(const loader_state_t new_state);

loader_state_t loader_next_state(const loader_state_t current_state,
                                 const loader_request_t request);

secbool        loader_is_valid_transition(const loader_state_t current_state,
                                          const loader_request_t request);

void loader_init_controlflow(void);

void loader_update_flowstate(loader_state_t nextstate);

secbool loader_calculate_flowstate(loader_state_t prevstate,
                                   loader_state_t nextstate);

#endif /*!LOADER_AUTOMATON_H_ */
