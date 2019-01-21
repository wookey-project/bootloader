#ifndef HASH_H_
#define HASH_H_

#include "autoconf.h"
#include "types.h"
#include "libsig.h"

# if CONFIG_LOADER_FW_HASH_CHECK
bool check_fw_hash(t_firmware_state *fw);

# endif

#endif
