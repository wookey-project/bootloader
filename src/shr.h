/* \file shr.h
 *
 * Copyright 2018 The wookey project team <wookey@ssi.gouv.fr>
 *   - Ryad     Benadjila
 *   - Arnauld  Michelizza
 *   - Mathieu  Renard
 *   - Philippe Thierry
 *   - Philippe Trebuchet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 *
 */
#ifndef _SHARED_H
#define _SHARED_H

#include "types.h"
#include "autoconf.h"

#define MAX_SIG_LEN 64 /* 512bit sig len */

#ifndef CONFIG_FIRMWARE_DUALBANK
# ifndef CONFIG_FIRMWARE_DFU
#  define MAX_APP_INDEX 1
# else
#  define MAX_APP_INDEX 2
# endif
#else
# ifndef CONFIG_FIRMWARE_DFU
#  define MAX_APP_INDEX 3
# else
#  define MAX_APP_INDEX 4
# endif
#endif

typedef int (* app_entry_t)(void);

#define SHR_SECTOR_SIZE 16384

/* flash erase generate 0xfffffffff content */
#define ERASE_VALUE 0xffffffff

typedef enum {
    FW_BOOTABLE = 0x00110100,
    FW_NOT_BOOTABLE = 0x11011101
} t_bootable_state;

typedef struct __packed {
    uint32_t bootable;
    uint32_t version;
    uint32_t siglen;
    uint8_t  sig[MAX_SIG_LEN];
    uint32_t crc32;
} t_firmware_state;

typedef struct __packed {
        t_firmware_state fw;
} shr_vars_t;

#endif
