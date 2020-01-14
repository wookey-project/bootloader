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

#ifndef _RNG_H
#define _RNG_H

#include "types.h"

#define r_CORTEX_M_RNG_BASE             REG_ADDR(0x50060800)

#define r_CORTEX_M_RNG_CR               (r_CORTEX_M_RNG_BASE + (uint32_t)0x00)
#define r_CORTEX_M_RNG_SR               (r_CORTEX_M_RNG_BASE + (uint32_t)0x01)
#define r_CORTEX_M_RNG_DR               (r_CORTEX_M_RNG_BASE + (uint32_t)0x02)

/* RNG control register */
#define RNG_CR_RNGEN_Pos                2
#define RNG_CR_RNGEN_Msk                ((uint32_t)1 << RNG_CR_RNGEN_Pos)
#define RNG_CR_IE_Pos                   3
#define RNG_CR_IE_Msk                   ((uint32_t)1 << RNG_CR_IE_Pos)

/* RNG status register */
#define RNG_SR_DRDY_Pos                 0
#define RNG_SR_DRDY_Msk                 ((uint32_t)1 << RNG_SR_DRDY_Pos)
#define RNG_SR_CECS_Pos                 1
#define RNG_SR_CECS_Msk                 ((uint32_t)1 << RNG_SR_CECS_Pos)
#define RNG_SR_SECS_Pos                 2
#define RNG_SR_SECS_Msk                 ((uint32_t)1 << RNG_SR_SECS_Pos)
#define RNG_SR_CEIS_Pos                 5
#define RNG_SR_CEIS_Msk                 ((uint32_t)1 << RNG_SR_CEIS_Pos)
#define RNG_SR_SEIS_Pos                 6
#define RNG_SR_SEIS_Msk                 ((uint32_t)1 << RNG_SR_SEIS_Pos)

/* RNG data register */
#define RNG_DR_RNDATA_Pos               0
#define RNG_DR_RNDATA_Msk               ((uint32_t)0xFFFF << RNG_DR_RNDATA_Pos)

int rng_manager(uint32_t * random);

#endif                          /* _RNG_H */
