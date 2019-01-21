/* \file shr.c
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

/*!
 * \file shr.c
 *
 * This file handle the SHR section structure. This content is out of
 * the basic layout and is shared beetween the kernel and the loader
 * for bootloading information such as flip/flop state
 */
#include "autoconf.h"
#include "shr.h"

#define VTORS_SIZE 392 /* 0x188 */
#define FW1_KERN_BASE   0x08020000
#define DFU1_KERN_BASE  0x08030000

#define FW2_KERN_BASE   0x08120000
#define DFU2_KERN_BASE  0x08130000

#define FW1_START FW1_KERN_BASE + VTORS_SIZE + 1
#define DFU1_START DFU1_KERN_BASE + VTORS_SIZE + 1

#define FW2_START FW2_KERN_BASE + VTORS_SIZE + 1
#define DFU2_START DFU2_KERN_BASE + VTORS_SIZE + 1

/* these data (.shared content) is mapped in SHR region (due to loader
 * ldscript) only when flashing the loader through the JTAG interface.
 * When using DFU and during all system boot, the SHR region is not overriden
 * by these initial configuration and is updated by DFUSMART as needed. */
__attribute__((section(".shared_flip")))
    const shr_vars_t flip_shared_vars = {
        .fw = {
            .fw_sig = {
                .magic = 0x0,
                .version = 0,
                .siglen = 0,
                .chunksize = 0,
                .sig = { 0x0 },
                .crc32 = 0x0
            },
            .fill = { 0xff },
            .bootable = FW_BOOTABLE
        },
    };

#if CONFIG_FIRMWARE_DUALBANK
__attribute__((section(".shared_flop")))
    const shr_vars_t flop_shared_vars = {
        .fw = {
            .fw_sig = {
                .magic = 0x0,
                .version = 0,
                .siglen = 0,
                .chunksize = 0,
                .sig = { 0x0 },
                .crc32 = 0x0
            },
            .fill = { 0xff },
            .bootable = FW_NOT_BOOTABLE
        },
    };
#endif
