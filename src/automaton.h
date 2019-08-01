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

typedef enum loader_state {
    LOADER_START       = 0x0,
    LOADER_INIT,
    LOADER_RDPCHECK,
    LOADER_DFUWAIT,
    LOADER_SELECTBANK,
    LOADER_HDRCRC,
    LOADER_FWINTEGRITY,
    LOADER_FLASHLOCK,
    LOADER_BOOTFW,
    LOADER_ERROR,
    LOADER_SECBREACH,
} loader_state_t;

/*
 * transition requests, like states, should be fault resilient.
 * TODO: this has to be added.
 */
typedef enum loader_request {
    LOADER_REQ_INIT,
    LOADER_REQ_RDPCHECK,
    LOADER_REQ_DFUCHECK,
    LOADER_REQ_SELECTBANK,
    LOADER_REQ_CRCCHECK,
    LOADER_REQ_INTEGRITYCHECK,
    LOADER_REQ_FLASHLOCK,
    LOADER_REQ_BOOT,
    LOADER_REQ_ERROR,
    LOADER_REQ_SECBREACH,
} loader_request_t;

loader_state_t loader_get_state(void);

void           loader_set_state(const loader_state_t new_state);

loader_state_t loader_next_state(const loader_state_t current_state,
                                 const loader_request_t request);

secbool        loader_is_valid_transition(const loader_state_t current_state,
                                          const loader_request_t request);

#endif /*!LOADER_AUTOMATON_H_ */
