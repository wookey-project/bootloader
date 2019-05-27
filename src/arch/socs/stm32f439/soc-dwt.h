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
#ifndef SOC_DWT_H
#define SOC_DWT_H

#include "types.h"

void soc_dwt_init(void);

void soc_dwt_reset_timer(void);

void soc_dwt_start_timer(void);

void soc_dwt_stop_timer(void);

uint32_t soc_dwt_getcycles(void);

uint64_t soc_dwt_getcycles_64(void);

void soc_dwt_ovf_manage(void);

#endif /*!SOC_DWT_H */
