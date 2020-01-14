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
#include "soc-rcc.h"
#include "regutils.h"
#include "debug.h"
#include "rng.h"

#define RNG_DEBUG 0

/**
 * @brief Initialize RNG (mainly initialize it clock).
 *
 * @param nothing
 * @return nothing
 */
static void rng_init(void)
{
        set_reg_bits(r_CORTEX_M_RCC_AHB2ENR, RCC_AHB2ENR_RNGEN);

        return;
}


/**
 * @brief Run the random number genrator.
 *
 * Run the RNG to get a random number. return 0 if
 * generation is completed, or an error code if not.
 *
 * As explained in FIPS PUB, we discard the first
 * random number generated and compare each generation
 * to the next one. Each number has to be compared to
 * previous one and generation fails if they are equal.
 *
 * @param  random Random number buffer.
 * @return 0 if success, error code is failure.
 */
static volatile bool rng_enabled = false;
static volatile bool not_first_rng = false;
static volatile uint32_t last_rng = 0;

static int rng_run(uint32_t *random)
{
        /* Enable RNG clock if needed */
        if(rng_enabled == false){
                rng_init(); 
                rng_enabled = true;
        }
        /* Enable random number generation */
        set_reg(r_CORTEX_M_RNG_CR, 1, RNG_CR_RNGEN);
        /* Wait for the RNG to be ready */
        while (!(read_reg_value(r_CORTEX_M_RNG_SR) & RNG_SR_DRDY_Msk)) {};
        /* Check for error */
        if (read_reg_value(r_CORTEX_M_RNG_SR) & RNG_SR_CEIS_Msk) {
                return 1;
        }
        else if (read_reg_value(r_CORTEX_M_RNG_SR) & RNG_SR_SEIS_Msk) {
                return 2;
        }
        /* Read random number */
        else if (read_reg_value(r_CORTEX_M_RNG_SR) & RNG_SR_DRDY_Msk) {
                *random = read_reg_value(r_CORTEX_M_RNG_DR);
                if((not_first_rng == false) || (last_rng == *random)){
                        /* FIPS PUB test of current with previous random
                         * and discard the first random.
                         */
                        last_rng = *random;
                        not_first_rng = true;
                        return 4;
                }
                else{
                        last_rng = *random;
                        return 0;
                }
        }
        else {
                return 3;
        }
}


/**
 * \brief Handles clock error (CEIS bit read as '1').
 */
static void rng_ceis_error(void)
{
    /* Check that clock controller is correctly configured */
#if RNG_DEBUG == 1
    dbg_log("[Clock error\n");
#endif
    /* Clear error */
    set_reg(r_CORTEX_M_RNG_SR, 0, RNG_SR_CEIS);
}

/**
 * \brief Handles seed error (SEIS bit read as '1').
 *
 * Seed error, we should not read the random number provided.
 */
static void rng_seis_error(void)
{
#if RNG_DEBUG == 1
    dbg_log("SEIS (seed) error\n");
#endif
    /* Clear error */
    set_reg(r_CORTEX_M_RNG_SR, 0, RNG_SR_SEIS);
    /* Clear and set RNGEN bit to restart the RNG */
    set_reg(r_CORTEX_M_RNG_CR, 0, RNG_CR_RNGEN);
    set_reg(r_CORTEX_M_RNG_CR, 1, RNG_CR_RNGEN);
}

static void rng_fips_error(void)
{
#if RNG_DEBUG == 1
    dbg_log("FIPS PUB warning: current random is the same as the previous one (or it is the first one)\n");
#endif
}

static void rng_unknown_error(void)
{
#if RNG_DEBUG == 1
    dbg_log("Unknown error happened (maybe data wasn't ready?)\n");
#endif
}

/**
 * @brief Launch a random number generation and handles errors.
 *
 * @param random Random number buffer
 */
int rng_manager(uint32_t *random)
{
        int ret;
again:
        ret = rng_run(random);
        switch (ret) {
                case 0:
                        break;
                case 1:
                        rng_ceis_error();
                        goto again;
                case 2:
                        rng_seis_error();
                        /* We have a seed error, discard the random and run again! */
                        goto again;
                case 3:
                        rng_unknown_error();
                        return -1;
                case 4:
                        rng_fips_error();
                        goto again;
                default:
                        return -1;
        }
        return 0;
}

