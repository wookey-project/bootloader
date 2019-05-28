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
#ifndef DEBUG_H_
#define DEBUG_H_

#include "autoconf.h"
#ifdef CONFIG_ARCH_CORTEX_M4
#include "m4-systick.h"
#else
#error "no systick support for other by now!"
#endif
#include "soc-usart.h"

/**
 * This is the DBGLOG log levels definition. This is syslog compatible
 */
typedef enum {
    DBG_EMERG = 0,
    DBG_ALERT = 1,
    DBG_CRIT = 2,
    DBG_ERR = 3,
    DBG_WARN = 4,
    DBG_NOTICE = 5,
    DBG_INFO = 6,
    DBG_DEBUG = 7,
} e_dbglevel_t;

/**
 * dbg_log - log strings in ring buffer
 * @fmt: format string
 */
int dbg_log(const char *fmt, ...);

/**
 * menuconfig controlled debug print
 */
#define DEBUG(level, fmt, ...) {  \
  if (level <= CONFIG_DBGLEVEL) {  dbg_log(fmt, __VA_ARGS__); dbg_flush(); } \
}

void debug_console_init(void);

/**
 * dbg_flush - flush the ring buffer to UART
 */
void dbg_flush(void);

/**
 * panic - output string on UART, flush ring buffer and stop
 * @fmt: format string
 */
void panic(char *fmt, ...);

#define assert(EXP)									\
	do {										\
		if (!(EXP))								\
			panic("Assert in file %s on line %d\n", __FILE__, __LINE__);	\
	} while (0)

#if DEBUG_LVL >= 3
#define LOG(fmt, ...) dbg_log("%lld: [II] %s:%d, %s:"fmt, get_ticks(), __FILE__, __LINE__,  __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG(fmt, ...) do {} while (0)
#endif

#if DEBUG_LVL >= 2
#define WARN(fmt, ...) dbg_log("%lld: [WW] %s:%d, %s:"fmt, get_ticks(), __FILE__, __LINE__,  __FUNCTION__, ##__VA_ARGS__)
#else
#define WARN(fmt, ...) do {} while (0)
#endif

#if DEBUG_LVL >= 1
extern volatile int logging;
#define ERROR(fmt, ...)							\
	do {									\
		dbg_log("%lld: [EE] %s:%d, %s:"fmt, get_ticks(), __FILE__, __LINE__,  __FUNCTION__, ##__VA_ARGS__);	\
		/*if (logging)*/							\
			dbg_flush();						\
	} while (0)
#else
#define ERROR(fmt, ...) do {} while (0)
#endif

#define LOG_CL(fmt, ...) dbg_log(""fmt, ##__VA_ARGS__)

void init_ring_buffer(void);

#endif /* !DEBUG_H_ */
