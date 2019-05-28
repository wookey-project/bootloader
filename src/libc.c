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

#include "types.h"

void *memset(void *s, int c, uint32_t n)
{
    char *bytes = s;
    while (n) {
        *bytes = c;
        bytes++;
        n--;
    }
    return s;
}

void *memcpy(void *dest, const void *src, uint32_t n)
{
    char *d_bytes = dest;
    const char *s_bytes = src;

    while (n) {
        *d_bytes = *s_bytes;
        d_bytes++;
        s_bytes++;
        n--;
    }

    return dest;
}

uint32_t strlen(const char *s)
{
    uint32_t i = 0;
    if (!s) {
        return 0;
    }
    while (*s) {
        i++;
        s++;
    }
    return i;
}

char *strncpy(char *dest, const char *src, uint32_t n)
{
    char *return_value = dest;

    if (!src || !dest) {
        return return_value;
    }
    while (n && *src) {
        *dest = *src;
        dest++;
        src++;
        n--;
    }

    while (n) {
        *dest = 0;
        dest++;
        n--;
    }

    return return_value;
}

int8_t strcmp(const char *a, const char *b)
{
    unsigned char len = 0;

    if (!a || !b) {
       if (!a && !b) {
           return 0;
       }
       if (!a) {
           return -1;
       }
       if (!b) {
           return 1;
       }
    }
    while (1) {
        if (a[len] != b[len] || a[len] == '\0' || b[len] == '\0') {
            return a[len] - b[len];
        }
        len++;
    }
}

char tolower (char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    } else {
        return c;
    }
}

int8_t strcasecmp(const char *a, const char *b)
{
    char c1, c2;
    uint8_t len = 0;

    if (!a || !b) {
       if (!a && !b) {
           return 0;
       }
       if (!a) {
           return -1;
       }
       if (!b) {
           return 1;
       }
    }

    while (1) {
        c1 = tolower(a[len]);
        c2 = tolower(b[len]);

        if (c1 != c2 || c1 == '\0' || c2 == '\0') {
            return c1 - c2;
        }

        len++;
    }
}


void sleep_intern(uint8_t length)
{
    /* FIXME Assert length value */
    int i = 0, j = 0;
    int time_value = (1 << (length * 2));
    for (i = 0; i < time_value; i++) {
        for (j = 0; j < time_value; j++) ;
    }
}
