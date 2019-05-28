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
#ifndef SOC_TYPES_H
#define SOC_TYPES_H

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
/* fully typed log buffer size */
typedef uint8_t logsize_t;

typedef enum {false = 0, true = 1} bool;
typedef enum {SUCCESS, FAILURE} retval_t;

/* Secure boolean against fault injections for critical tests */
typedef enum {secfalse = 0x55aa55aa, sectrue = 0xaa55aa55} secbool;

#define KBYTE 1024
#define MBYTE 1048576
#define GBYTE 1073741824

#define NULL				((void *)0)

/* 32bits targets specific */
typedef uint32_t physaddr_t;
typedef uint8_t svcnum_t;

#if defined(__CC_ARM)
# define __ASM            __asm  /* asm keyword for ARM Compiler    */
# define __INLINE         static __inline    /* inline keyword for ARM Compiler */
# define __ISR_HANDLER           /* [PTH] todo: find the way to deactivate localy frame pointer or use rx, x<4 for it */
# define __NAKED                 /* [PTH] todo: find the way to set the function naked (without pre/postamble) */
# define __UNUSED                /* [PTH] todo: find the way to set a function/var unused */
# define __WEAK                  /* [PTH] todo: find the way to set a function/var weak */
#elif defined(__GNUC__)
# define __ASM            __asm  /* asm keyword for GNU Compiler    */
# define __INLINE        static inline
#ifdef __clang__
  # define __ISR_HANDLER  __attribute__((interrupt("IRQ")))
#else
  # define __ISR_HANDLER   __attribute__((optimize("-fomit-frame-pointer")))
#endif
# define __NAKED         __attribute__((naked))
# define __UNUSED        __attribute__((unused))
# define __WEAK          __attribute__((weak))
# define __packed		__attribute__((__packed__))
#endif

#endif
