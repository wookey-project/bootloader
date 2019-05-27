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
#ifndef _SHARED_H
#define _SHARED_H

#include "libsig.h"
#include "types.h"
#include "autoconf.h"

/* FIXME: migrate to unified layer from json asap */
#define FLIP_BASE       0x08020000
#define FLIP_SIZE       0xe0000
#define FW1_KERN_BASE   0x08020000
#define DFU1_KERN_BASE  0x08030000

#define FLOP_BASE       0x08120000
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
        PART_FLIP = 0,
        PART_FLOP = 1,
} partitions_types;

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
    uint8_t  sig[EC_MAX_SIGLEN];
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
