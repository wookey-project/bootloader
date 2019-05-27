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
#ifndef REGUTILS_H_
#define REGUTILS_H_
#include "types.h"

#define REG_ADDR(addr)                      ((volatile uint32_t *)(addr))
#define REG_VALUE(reg, value, pos, mask)    ((reg)  |= (((value) << (pos)) & (mask)))

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))

#define ARRAY_SIZE(array, type)	(sizeof(array) / sizeof(type))

/* implicit convertion to int */
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

__INLINE uint8_t min_u8(uint8_t x, uint8_t y)
{
    if (x < y)
        return x;
    else
        return y;
}

/*
 * These macros assume that the coding style (bits_name_Msk and bits_name_Pos)
 * is respected when defining registers bitfields
 */
#define set_reg(REG, VALUE, BITS)	set_reg_value(REG, VALUE, BITS##_Msk, BITS##_Pos)
#define get_reg(REG, BITS)		get_reg_value(REG, BITS##_Msk, BITS##_Pos)

__INLINE uint32_t get_reg_value(volatile const uint32_t * reg, uint32_t mask,
                                uint8_t pos);
__INLINE int8_t set_reg_value(volatile uint32_t * reg, uint32_t value,
                              uint32_t mask, uint8_t pos);

__INLINE uint32_t read_reg_value(volatile uint32_t * reg);
__INLINE uint16_t read_reg16_value(volatile uint16_t * reg);
__INLINE void write_reg_value(volatile uint32_t * reg, uint32_t value);
__INLINE void write_reg16_value(volatile uint16_t * reg, uint16_t value);

__INLINE void set_reg_bits(volatile uint32_t * reg, uint32_t value);
__INLINE void clear_reg_bits(volatile uint32_t * reg, uint32_t value);

__INLINE uint32_t to_big32(uint32_t value);
__INLINE uint16_t to_big16(uint16_t value);
__INLINE uint32_t to_little32(uint32_t value);
__INLINE uint16_t to_little16(uint16_t value);
__INLINE uint32_t from_big32(uint32_t value);
__INLINE uint16_t from_big16(uint16_t value);
__INLINE uint32_t from_little32(uint32_t value);
__INLINE uint16_t from_little16(uint16_t value);

__INLINE uint32_t get_reg_value(volatile const uint32_t * reg, uint32_t mask,
                                uint8_t pos)
{
    if ((mask == 0x00) || (pos > 31))
        return 0;

    return (uint32_t) (((*reg) & mask) >> pos);
}

__INLINE uint16_t get_reg16_value(volatile uint16_t * reg, uint16_t mask,
                                  uint8_t pos)
{
    if ((mask == 0x00) || (pos > 15))
        return 0;

    return (uint16_t) (((*reg) & mask) >> pos);
}

__INLINE int8_t set_reg_value(volatile uint32_t * reg, uint32_t value,
                              uint32_t mask, uint8_t pos)
{
    uint32_t tmp;

    if (pos > 31)
        return -1;

    if (mask == 0xFFFFFFFF) {
        (*reg) = value;
    } else {
        tmp = read_reg_value(reg);
        tmp &= ~mask;
        tmp |= (value << pos) & mask;
        write_reg_value(reg, tmp);
    }

    return 0;
}

__INLINE int8_t set_reg16_value(volatile uint16_t * reg, uint16_t value,
                                uint16_t mask, uint8_t pos)
{
    uint16_t tmp;

    if (pos > 15)
        return -1;

    if (mask == 0xFFFF) {
        (*reg) = value;
    } else {
        tmp = read_reg16_value(reg);
        tmp &= (uint16_t) ~ mask;
        tmp |= (uint16_t) ((value << pos) & mask);
        write_reg16_value(reg, tmp);
    }

    return 0;
}

__INLINE uint32_t read_reg_value(volatile uint32_t * reg)
{
    return (uint32_t) (*reg);
}

__INLINE uint16_t read_reg16_value(volatile uint16_t * reg)
{
    return (uint16_t) (*reg);
}

__INLINE void write_reg_value(volatile uint32_t * reg, uint32_t value)
{
    (*reg) = value;
}

__INLINE void write_reg16_value(volatile uint16_t * reg, uint16_t value)
{
    (*reg) = value;
}

__INLINE void set_reg_bits(volatile uint32_t * reg, uint32_t value)
{
    *reg |= value;
}

__INLINE void set_reg16_bits(volatile uint16_t * reg, uint16_t value)
{
    *reg |= value;
}

__INLINE void clear_reg_bits(volatile uint32_t * reg, uint32_t value)
{
    *reg &= (uint32_t) ~ (value);
}

__INLINE uint32_t to_big32(uint32_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((value & 0xff) << 24) | ((value & 0xff00) << 8)
        | ((value & 0xff0000) >> 8) | ((value & 0xff000000) >> 24);
#else
    return value;
#endif
}

__INLINE uint16_t to_big16(uint16_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t) ((value & 0xff) << 8) | (uint16_t) ((value & 0xff00) >>
                                                          8);
#else
    return value;
#endif
}

__INLINE uint32_t to_little32(uint32_t value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((value & 0xff) << 24) | ((value & 0xff00) << 8)
        | ((value & 0xff0000) >> 8) | ((value & 0xff000000) >> 24);
#else
    return value;
#endif
}

__INLINE uint16_t to_little16(uint16_t value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((value & 0xff) << 8) | ((value & 0xff00) >> 8);
#else
    return value;
#endif
}

__INLINE uint32_t from_big32(uint32_t value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return value;
#else
    return ((value & 0xff) << 24) | ((value & 0xff00) << 8)
        | ((value & 0xff0000) >> 8) | ((value & 0xff000000) >> 24);
#endif
}

__INLINE uint16_t from_big16(uint16_t value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return value;
#else
    return (uint16_t) ((value & 0xff) << 8) | (uint16_t) ((value & 0xff00) >>
                                                          8);
#endif
}

__INLINE uint32_t from_little32(uint32_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#else
    return ((value & 0xff) << 24) | ((value & 0xff00) << 8)
        | ((value & 0xff0000) >> 8) | ((value & 0xff000000) >> 24);
#endif
}

__INLINE uint16_t from_little16(uint16_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#else
    return ((value & 0xff) << 8) | ((value & 0xff00) >> 8);
#endif
}

#endif                          /*!REGUTILS_H_ */
