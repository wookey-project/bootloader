#include "hash.h"
#include "regutils.h"
#include "debug.h"
#include "main.h"

# ifdef CONFIG_LOADER_FW_HASH_CHECK

secbool check_fw_hash(const t_firmware_state *fw, uint32_t partition_base_addr, uint32_t partition_size)
{
    sha256_context sha256_ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];
    uint32_t tmp;

#if __GNUG__
# pragma GCC push_options
# pragma GCC optimize("O0")
#endif
#if __clang__
# pragma clang optimize off
#endif
    /* Double sanity check (for faults) */
    if(fw->fw_sig.len > partition_size){
        goto err;
    }
    if(fw->fw_sig.len > partition_size){
        goto err;
    }
#if __clang__
# pragma clang optimize on
#endif
#if __GNUG__
# pragma GCC pop_options
#endif

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

#if __GNUG__
# pragma GCC push_options
# pragma GCC optimize("O0")
#endif
#if __clang__
# pragma clang optimize off
#endif
    /* Double check for faults */
    if(!are_equal(digest, fw->fw_sig.hash, SHA256_DIGEST_SIZE)){
        goto err;
    }
    if(!are_equal(digest, fw->fw_sig.hash, SHA256_DIGEST_SIZE)){
        goto err;
    }
#if __clang__
# pragma clang optimize on
#endif
#if __GNUG__
# pragma GCC pop_options
#endif


    return sectrue;

err:
    return secfalse;
}

# endif
