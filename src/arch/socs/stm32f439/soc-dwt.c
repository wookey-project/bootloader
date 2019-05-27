/*
 * copyright 2019 the wookey project team <wookey@ssi.gouv.fr>
 *   - ryad     benadjila
 *   - arnauld  michelizza
 *   - mathieu  renard
 *   - philippe thierry
 *   - philippe trebuchet
 * all rights reserved.
 *
 * redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. neither the name of mosquitto nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * this software is provided by the copyright holders and contributors "as is"
 * and any express or implied warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose
 * are disclaimed. in no event shall the copyright owner or contributors be
 * liable for any direct, indirect, incidental, special, exemplary, or
 * consequential damages (including, but not limited to, procurement of
 * substitute goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether in
 * contract, strict liability, or tort (including negligence or otherwise)
 * arising in any way out of the use of this software, even if advised of the
 * possibility of such damage.
 */
#include "soc-dwt.h"
#include "m4-cpu.h"

static uint32_t last_dwt = 0;

static volatile uint64_t cyccnt_loop = 0;

static volatile uint32_t* DWT_CONTROL = (volatile uint32_t*) 0xE0001000;
static volatile uint32_t* SCB_DEMCR = (volatile uint32_t*) 0xE000EDFC;
static volatile uint32_t* LAR = (volatile uint32_t*) 0xE0001FB0;
static volatile uint32_t *DWT_CYCCNT = (volatile uint32_t *) 0xE0001004;

void soc_dwt_reset_timer(void)
{
    *SCB_DEMCR = *(uint32_t *) SCB_DEMCR | 0x01000000;
    *LAR = 0xC5ACCE55;
    *DWT_CYCCNT = 0;   // reset the counter
    *DWT_CONTROL = 0;
   full_memory_barrier();
}

void soc_dwt_start_timer(void)
{
    *DWT_CONTROL = *DWT_CONTROL | 1;  // enable the counter
   full_memory_barrier();
}

uint32_t soc_dwt_getcycles(void)
{
    return *DWT_CYCCNT;
}

uint64_t soc_dwt_getcycles_64(void)
{
    uint64_t val = *DWT_CYCCNT;
    val += cyccnt_loop << 32;
    return val;
}


void soc_dwt_ovf_manage(void)
{
    uint32_t dwt = soc_dwt_getcycles();

    /*
     * DWT cycle count overflow: we increment cyccnt_loop counter.
     */
    if (dwt < last_dwt) {
        cyccnt_loop++;
    }

    last_dwt = dwt;
}

void soc_dwt_init(void)
{
    soc_dwt_reset_timer();
    soc_dwt_start_timer();
}
