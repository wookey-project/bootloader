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
#include "libsig.h"

/* FIXME: migrate to unified layer from json asap */
#define FLIP_BASE       0x08020000
#define FLIP_SIZE       0xe0000
#define FW1_KERN_BASE   0x08020000
#define DFU1_KERN_BASE  0x08030000

#define FLOP_BASE       0x08020000
#define FLOP_SIZE       0xe0000
#define FW2_KERN_BASE   0x08120000
#define DFU2_KERN_BASE  0x08130000

#define FW1_START FW1_KERN_BASE + VTORS_SIZE + 1
#define DFU1_START DFU1_KERN_BASE + VTORS_SIZE + 1

#define FW2_START FW2_KERN_BASE + VTORS_SIZE + 1
#define DFU2_START DFU2_KERN_BASE + VTORS_SIZE + 1

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
    FW_BOOTABLE = 0x53747421,
    FW_NOT_BOOTABLE = 0x5e19be55
} t_bootable_state;

typedef struct __packed {
    uint32_t magic;
    uint32_t type;
    uint32_t version;
    uint32_t len;
    uint32_t siglen;
    uint32_t chunksize;
    uint32_t crc32;
    uint8_t hash[SHA256_DIGEST_SIZE];
    uint8_t  sig[EC_STRUCTURED_SIG_EXPORT_SIZE(EC_MAX_SIGLEN)];
} t_firmware_signature;


typedef struct __packed {
    t_firmware_signature   fw_sig;
    uint8_t                fill[SHR_SECTOR_SIZE - sizeof(t_firmware_signature)];
    uint32_t               bootable;
    uint8_t                fill2[SHR_SECTOR_SIZE - sizeof(uint32_t)];
} t_firmware_state;

typedef struct __packed {
        t_firmware_state fw;
} shr_vars_t;

#endif
