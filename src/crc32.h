/* \file fw_crc32.h
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
#ifndef CRC32_H_
#define CRC32_H_

#include "types.h"

/*
 * @brief Linux-compatible CRC32 implementation
 *
 * Return the CRC32 of the current buffer
 * @param buf  the buffer on which the CRC32 is calculated
 * @param len  the buffer len
 * @param init when calculating CRC32 on successive chunk to get back
 *             the CRC32 of the whole input content, contains the previous
 *             chunk CRC32, or 0xffffffff for the first one
 */

uint32_t crc32 (const unsigned char *buf, uint32_t len, uint32_t init);

#endif/*!CRC32_H_*/
