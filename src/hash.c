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
#include "hash.h"
#include "regutils.h"
#include "debug.h"
#include "main.h"

# ifdef CONFIG_LOADER_FW_HASH_CHECK

/* NOTE: O0 for fault attacks protections */
#ifdef __GNUC__
#ifdef __clang__
# pragma clang optimize off
#else
# pragma GCC push_options
# pragma GCC optimize("O0")
#endif
#endif

uint64_t hash_state(uint64_t val)
{
    sha256_context sha256_ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];
    union u_digest {
        uint64_t *u64;
        uint8_t  *digest;
    };

    sha256_init(&sha256_ctx);
    uint64_t to_hash;

    /* make val a big endian value */
    to_hash = to_big32((uint32_t)val);
    to_hash |= (32 << to_big32((uint32_t)(val >> 32)));

    sha256_update(&sha256_ctx, (uint8_t*)&to_hash, sizeof(to_hash));

    sha256_final(&sha256_ctx, digest);

    /* truncate the result to an uint64_t */
    union u_digest result;
    result.digest = &(digest[0]);

    return *(result.u64);
}

secbool check_fw_hash(const t_firmware_state *fw, uint32_t partition_base_addr, uint32_t partition_size)
{
    sha256_context sha256_ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];
    uint32_t tmp;

    /* Double sanity check (for faults) */
    if(fw->fw_sig.len > partition_size){
        goto err;
    }
    if(fw->fw_sig.len > partition_size){
        goto err;
    }

    sha256_init(&sha256_ctx);
    /* Begin to hash the header */
    tmp = to_big32(fw->fw_sig.magic);
    sha256_update(&sha256_ctx, (uint8_t*)&tmp, sizeof(tmp));
    tmp = to_big32(fw->fw_sig.type);
    sha256_update(&sha256_ctx, (uint8_t*)&tmp, sizeof(tmp));
    tmp = to_big32(fw->fw_sig.version);
    sha256_update(&sha256_ctx, (uint8_t*)&tmp, sizeof(tmp));
    tmp = to_big32(fw->fw_sig.len);
    sha256_update(&sha256_ctx, (uint8_t*)&tmp, sizeof(tmp));
    tmp = to_big32(fw->fw_sig.siglen);
    sha256_update(&sha256_ctx, (uint8_t*)&tmp, sizeof(tmp));
    tmp = to_big32(fw->fw_sig.chunksize);
    sha256_update(&sha256_ctx, (uint8_t*)&tmp, sizeof(tmp));

    /* Then hash the flash content */
    sha256_update(&sha256_ctx, (uint8_t*)partition_base_addr, partition_size);

    sha256_final(&sha256_ctx, digest);

    /* Multiple checks for faults */
    if(!are_equal(digest, fw->fw_sig.hash, SHA256_DIGEST_SIZE)){
        goto err;
    }
    if(!are_equal(fw->fw_sig.hash, digest, SHA256_DIGEST_SIZE)){
        goto err;
    }
    if(!are_equal(digest, fw->fw_sig.hash, SHA256_DIGEST_SIZE)){
        goto err;
    }

    return sectrue;

err:
    return secfalse;
}
#ifdef __GNUC__
#ifdef __clang__
# pragma clang optimize on
#else
# pragma GCC pop_options
#endif
#endif

# endif
