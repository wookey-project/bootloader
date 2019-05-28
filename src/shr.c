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

/*!
 * \file shr.c
 *
 * This file handle the SHR section structure. This content is out of
 * the basic layout and is shared beetween the kernel and the loader
 * for bootloading information such as flip/flop state
 */
#include "autoconf.h"
#include "shr.h"

/* these data (.shared content) is mapped in SHR region (due to loader
 * ldscript) only when flashing the loader through the JTAG interface.
 * When using DFU and during all system boot, the SHR region is not overriden
 * by these initial configuration and is updated by DFUSMART as needed. */
__attribute__((section(".shared_flip")))
    const shr_vars_t flip_shared_vars = {
        .fw = {
            .fw_sig = {
                .magic = 0x0,
                .type = 0x0,
                .version = 0,
                .siglen = 0,
                .len = 0,
                .chunksize = 0,
                .sig = { 0x0 },
                .crc32 = 0x0,
		.hash = { 0x0 }
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
                .type = 0x0,
                .version = 0,
                .siglen = 0,
                .len = 0,
                .chunksize = 0,
                .sig = { 0x0 },
                .crc32 = 0x0,
		.hash = { 0x0 }
            },
            .fill = { 0xff },
            .bootable = FW_NOT_BOOTABLE
        },
    };
#endif
