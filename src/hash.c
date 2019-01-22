#include "hash.h"
#include "regutils.h"
#include "debug.h"
#include "main.h"

# if CONFIG_LOADER_FW_HASH_CHECK

bool check_fw_hash(const t_firmware_state *fw, uint32_t partition_base_addr, uint32_t partition_size)
{
    sha256_context sha256_ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];
    uint32_t tmp;

    /* Sanity check */
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

    if(!are_equal(digest, fw->fw_sig.hash, SHA256_DIGEST_SIZE)){
        goto err;
    }

    return true;

err:
    return false;
}

# endif
