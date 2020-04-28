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
/** @file flash.c
 * \brief File includes functions to manipulate flash memory.
 *
 * See part. 3 (p73) in DocID018909 Rev 13
 */

#include "autoconf.h"
#include "flash.h"
#include "regutils.h"
#include "flash_regs.h"
#include "libc.h"

physaddr_t sectors_toerase[] = {
    /* first, cleaning nominal usersapce content
     * (clear encrypted keybags) */
    FLASH_SECTOR_8,
    FLASH_SECTOR_9,
    FLASH_SECTOR_10,
    FLASH_SECTOR_11,
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)
    FLASH_SECTOR_20,
    FLASH_SECTOR_21,
    FLASH_SECTOR_22,
    FLASH_SECTOR_23,
#endif
    /* now flash DFU content (clear hability to update) */
    FLASH_SECTOR_6,
    FLASH_SECTOR_7,
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)
    FLASH_SECTOR_18,
    FLASH_SECTOR_19,
#endif
    /* now erase kernels */
    FLASH_SECTOR_5,
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)
    FLASH_SECTOR_17,
#endif
    /* and bootinfo (bootloader will fail forever) */
    FLASH_SECTOR_2,
    FLASH_SECTOR_3,
    FLASH_SECTOR_4,
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)
    FLASH_SECTOR_14,
    FLASH_SECTOR_15,
    FLASH_SECTOR_16,
#endif
};


#define FLASH_DEBUG 0

/* Primitive for debug output */
#if FLASH_DEBUG
#define log_printf(...) dbg_llog(__VA_ARGS__)
#else
#define log_printf(...)
#endif

#ifndef assert
#define assert(val) if (!(val)) { log_printf("bkpt"); while(1){}; };
#endif

/* the flash device is mapped in a discretional memory layout:
 * - the flash memory
 *     -> from 0x08000000 to 0x08010000 or 0x08020000 for 2M flash
 * - the flash control register interface
 *     -> from 0x40023C00 to 0x40023FFF
 * - the flash system memory (holding the RO ST bootloader)
 *     -> from 0x1FFF000 to 0x1FFF77FF
 * - the flash One Time Programmable (OTP) region
 *     -> from 0x1FFF7800 to 0x1FFF7A0F
 * - The Bank 1 option bytes
 *     -> from 0x1FFFC000 to 1FFFC00F
 * - the Bank 2 option bytes
 *     -> from 0x1FFEC000 to 1FFEC00F
 *
 *  All these areasare not mapped continuously and request independent
 *  device mapping.
 *  As a consequence, and because it is not possible to map such a number of
 *  device, all devices are mapped VOLUNTARY, and are dynamically mapped/unmapped
 *  as needed by this driver.
 *
 *  At init time, the upper layer specify which device in the above list
 *  is requested to be declared. This permit to declare only the required one, avoiding
 *  the mapping of devices like the OTP area, which is not always needed, depending
 *  on the upper layer needs.
 *
 */

static inline int flash_is_busy(void){
	return !!(read_reg_value(r_CORTEX_M_FLASH_SR) & FLASH_SR_BSY);
}

static inline void flash_busy_wait(void){
	while (flash_is_busy()) {};
}

/**
 * \brief Unlock the flash control register
 *
 * FIXME Check if CR bit is == RESET ?
 */
int flash_unlock(void)
{
    if (get_reg(r_CORTEX_M_FLASH_CR, FLASH_CR_LOCK) == 0) {
        /* already unlocked */
        return 0;
    }
	log_printf("Unlocking flash\n");
	write_reg_value(r_CORTEX_M_FLASH_KEYR, KEY1);
	write_reg_value(r_CORTEX_M_FLASH_KEYR, KEY2);

    /*
     * when unlocking flash for the first time after reset, the PGSERR flag
     * is active and need to be cleared.
     * errata: this is *not* described in the datasheet !
     */
    set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_PGSERR);

    /* checking that flash CR unlock worked */
    if (get_reg(r_CORTEX_M_FLASH_CR, FLASH_CR_LOCK) == 1) {
	   log_printf("flash unlocking failed !\n");
       return 1;
    }
    return 0;
}

/**
 * \brief Unlock the flash option bytes register
 */
void flash_unlock_opt(void)
{
	log_printf("Unlocking flash option bytes register\n");
	write_reg_value(r_CORTEX_M_FLASH_OPTKEYR, OPTKEY1);
	write_reg_value(r_CORTEX_M_FLASH_OPTKEYR, OPTKEY2);
}

/**
 * \brief Lock the flash control register
 */
void flash_lock(void)
{
	log_printf("Locking flash\n");
	write_reg_value(r_CORTEX_M_FLASH_CR, 0x00000000);
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_LOCK);	/* Write only to 1, unlock is
							 * done by the previous
							 * sequence (RM0090
							 * DocID018909
							 * Rev 13 3.9.7 p104)
							 */
}

/**
 * \brief Lock the flash option bytes register
 */
void flash_lock_opt(void)
{
	log_printf("Locking flash option bytes register\n");
	set_reg(r_CORTEX_M_FLASH_OPTCR, 1, FLASH_OPTCR_OPTLOCK); /* Same as previously */
}

static bool is_sector_start(physaddr_t addr)
{
    switch (addr) {

        case FLASH_SECTOR_0:
        case FLASH_SECTOR_1:
        case FLASH_SECTOR_2:
        case FLASH_SECTOR_3:
        case FLASH_SECTOR_4:
        case FLASH_SECTOR_5:
        case FLASH_SECTOR_6:
        case FLASH_SECTOR_7:
# if (CONFIG_USR_DRV_FLASH_1M && !CONFIG_USR_DRV_FLASH_DUAL_BANK) || CONFIG_USR_DRV_FLASH_2M
    /* 1MB flash in dual banking doesn't have these 4 sectors */
        case FLASH_SECTOR_8:
        case FLASH_SECTOR_9:
        case FLASH_SECTOR_10:
        case FLASH_SECTOR_11:
    /* 1MB flash in single banking finishes here */
#endif
# if (CONFIG_USR_DRV_FLASH_1M && CONFIG_USR_DRV_FLASH_DUAL_BANK) || CONFIG_USR_DRV_FLASH_2M
        case FLASH_SECTOR_12:
        case FLASH_SECTOR_13:
        case FLASH_SECTOR_14:
        case FLASH_SECTOR_15:
        case FLASH_SECTOR_16:
        case FLASH_SECTOR_17:
        case FLASH_SECTOR_18:
        case FLASH_SECTOR_19:
    /* 1MB flash in dual banking finishes here */
#endif
# if CONFIG_USR_DRV_FLASH_2M
        case FLASH_SECTOR_20:
        case FLASH_SECTOR_21:
        case FLASH_SECTOR_22:
        case FLASH_SECTOR_23:
    /* 2MB flash in dual banking finishes here */
#endif
            return true;
        default:
            return false;
    }
	return false;
}

/**
 * \brief Select the sector to erase
 *
 * Sector address and size size depends on the configured flash device.
 * See flash_regs.h for more information about how these macros are defined.
 *
 * \param   addr Address pointing to sector
 *
 * \return sector number to erase
 */
uint8_t flash_select_sector(physaddr_t addr)
{
	uint8_t sector = 255;
	/* First 8 sectors are the same in single/dual mem config */
	if (addr <= FLASH_SECTOR_0_END) {
		sector = 0;
	}
	else if (addr <= FLASH_SECTOR_1_END) {
		sector = 1;
	}
	else if (addr <= FLASH_SECTOR_2_END) {
		sector = 2;
	}
	else if (addr <= FLASH_SECTOR_3_END) {
		sector = 3;
	}
	else if (addr <= FLASH_SECTOR_4_END) {
		sector = 4;
	}
	else if (addr <= FLASH_SECTOR_5_END) {
		sector = 5;
	}
	else if (addr <= FLASH_SECTOR_6_END) {
		sector = 6;
	}
	else if (addr <= FLASH_SECTOR_7_END) {
		sector = 7;
	}
# if (CONFIG_USR_DRV_FLASH_1M && !CONFIG_USR_DRV_FLASH_DUAL_BANK) || CONFIG_USR_DRV_FLASH_2M
    /* 1MB flash in dual banking doesn't have these 4 sectors */
	else if (addr <= FLASH_SECTOR_8_END) {
		sector = 8;
	}
	else if (addr <= FLASH_SECTOR_9_END) {
		sector = 9;
	}
	else if (addr <= FLASH_SECTOR_10_END) {
		sector = 10;
	}
	else if (addr <= FLASH_SECTOR_11_END) {
		sector = 11;
	}
    /* 1MB flash in single banking finishes here */
#endif
    /* from now on (sector starting with sector 12, are encoded starting with 0b10000 */
# if (CONFIG_USR_DRV_FLASH_1M && CONFIG_USR_DRV_FLASH_DUAL_BANK) || CONFIG_USR_DRV_FLASH_2M
	else if (addr <= FLASH_SECTOR_12_END) {
		sector = 0x10;
	}
	else if (addr <= FLASH_SECTOR_13_END) {
		sector = 0x11;
	}
	else if (addr <= FLASH_SECTOR_14_END) {
		sector = 0x12;
	}
	else if (addr <= FLASH_SECTOR_15_END) {
		sector = 0x13;
	}
	else if (addr <= FLASH_SECTOR_16_END) {
		sector = 0x14;
	}
	else if (addr <= FLASH_SECTOR_17_END) {
		sector = 0x15;
	}
	else if (addr <= FLASH_SECTOR_18_END) {
		sector = 0x16;
	}
	else if (addr <= FLASH_SECTOR_19_END) {
		sector = 0x17;
	}
    /* 1MB flash in dual banking finishes here */
#endif
# if CONFIG_USR_DRV_FLASH_2M
	else if (addr <= FLASH_SECTOR_20_END) {
		sector = 0x18;
	}
	else if (addr <= FLASH_SECTOR_21_END) {
		sector = 0x19;
	}
	else if (addr <= FLASH_SECTOR_22_END) {
		sector = 0x1a;
	}
	else if (addr <= FLASH_SECTOR_23_END) {
		sector = 0x1b;
	}
    /* 2MB flash in dual banking finishes here */
#endif
	else {
		log_printf("Error: %x Wrong address case, can't happen.\n", addr);
		while(1){};
	}
	return sector;
}


/*
 * flash error bits management
 */
static inline bool flash_has_programming_errors(void)
{
    uint32_t reg;
    reg = read_reg_value(r_CORTEX_M_FLASH_SR);
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)      /*  Dual blank only on f42xxx/43xxx */
    uint32_t err_mask = 0x1f2;
#else
    uint32_t err_mask = 0xf2;
#endif
    if (reg & err_mask) {
        if (reg & FLASH_SR_OPERR_Msk) {
            log_printf("flash operation error: OPERR\n");
            set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_OPERR);
            goto err;
        }
        if (reg & FLASH_SR_WRPERR_Msk) {
            log_printf("flash write protection error: WRPERR\n");
            set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_WRPERR);
            goto err;
        }
        if (reg & FLASH_SR_PGAERR_Msk) {
            log_printf("flash write error: PGAERR\n");
            set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_PGAERR);
            goto err;
        }
        if (reg & FLASH_SR_PGPERR_Msk) {
            log_printf("flash write error: PGPERR\n");
            set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_PGPERR);
            goto err;
        }
        if (reg & FLASH_SR_PGSERR_Msk) {
            log_printf("flash write error: PGSERR\n");
            set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_PGSERR);
            goto err;
        }
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)			/* RDERR (only on f42xxx/43xxx) */
        if (reg & FLASH_SR_RDERR_Msk) {
            log_printf("flash write error: RDERR\n");
            set_reg(r_CORTEX_M_FLASH_SR, 1, FLASH_SR_RDERR);
            goto err;
        }
#endif
    }
    return false;
err:
    return true;
}




/**
 * \brief Erase a sector on the flash memory.
 *
 * @param sector Sector to erase (from 16 to 128 kB)
 * @return Erased sector number
 */
uint8_t flash_sector_erase(physaddr_t addr)
{
	uint8_t sector = 255;
	/* Check that we're looking into the flash */
	assert(IS_IN_FLASH(addr));


	/* Select sector to erase */
	sector = flash_select_sector(addr);
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)
    if (sector > 11) {
        /* updating sector number for SNB[4:0] field instead of SNB[3:0] */
        sector = (sector - 12) | 0x10;
    }
#endif
	log_printf("Erasing flash sector #%d\n", sector);

	/* Check that the BSY bit in the FLASH_SR reg is not set */
	if(flash_is_busy()) {
        flash_busy_wait();
    }

    /* check if flash is unlock, and unlock it if needed */
    if (flash_unlock() != 0) {
        log_printf("unable to unlock flash!\n");
    }

	/* Set PSIZE to 0b10 (see STM-RM00090 chap. 3.6.2, PSIZE must be set) */
	set_reg(r_CORTEX_M_FLASH_CR, 2, FLASH_CR_PSIZE);

	/* Set SER bit */
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_SER);

	/* Erase sector */
	set_reg(r_CORTEX_M_FLASH_CR, sector, FLASH_CR_SNB);

	/* Set STRT bit in FLASH_CR reg */
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_STRT);

	/* Wait for BSY bit to be cleared */
	flash_busy_wait();

	/* Clean sector */
	set_reg(r_CORTEX_M_FLASH_CR, 0, FLASH_CR_SNB);

	/* Unset SER bit */
	set_reg(r_CORTEX_M_FLASH_CR, 0, FLASH_CR_SER);

    if (flash_has_programming_errors()) {
        goto err;
    }
	return sector;
err:
    log_printf("error while erasing sector at addr %x\n", addr);
    return 0xff;
}

/**
 * \brief Erase a whole bank
 *
 * @param bank Bank to erase (0 bank 1, 1 bank 2)
 */
void flash_bank_erase(uint8_t bank)
{
	/* Check that the BSY bit in the FLASH_SR reg is not set */
	if(flash_is_busy()){
		log_printf("Flash busy. Should not happen\n");
		while(1){};
	}

	/* Set MER or MER1 bit accordingly */
	if (bank) {
#if !(defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)) /*  Dual blank only on f42xxx/43xxx */
		log_printf("Can't acess bank 2 on a single bank memory!\n");
		while(1){};
#else
		set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_MER1);
#endif
	}
	else
		set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_MER);

	/* Set STRT bit in FLASH_CR reg */
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_STRT);

	/* Wait for BSY bit to be cleared */
	flash_busy_wait();

    if (flash_has_programming_errors()) {
        goto err;
    }
	return;
err:
    log_printf("error while erasing bank\n");
    return;
}



/**
 * \brief Mass erase (erase the whole flash)
 *
 * info: busy check and unlock is done at sector_erase level
 */
void flash_mass_erase(void)
{
    int ret = 0;
#if LOADER_ERASE_WITH_RECOVERY
    uint32_t data[4];
    uint32_t check[4];
    secbool otp_done = secfalse;

    data[0] = 0xDEADCAFE;
    rng_manager((uint32_t*)&data[1]);
    data[2] = 0xCACACACA;
    rng_manager((uint32_t*)&data[3]);
    uint8_t ret = 0;
#endif
    for (uint8_t i = 0; i < (sizeof(sectors_toerase)/sizeof(physaddr_t)); ++i) {
        /* first, cleaning nominal usersapce content
         * (clear encrypted keybags) */
#if LOADER_ERASE_WITH_RECOVERY
        /* first we check if current block is already erased. As this check is critical
         * and is a typical FIA target, checks are multiplied, data set multiple times
         * and random values generated multiple times to make FIA highly complex
         * If the current sector is effectively already erased, pass to next sector */
        flash_read_otp_block(i, &check[0], 4);
        if (check[0] == 0xDEADCAFE &&
                (check[2] == 0xCACACACA)) {

            if (!(check[2] != 0xCACACACA) &&
                    (!(check[0] != 0xDEADCAFE))) {
                /* already erased, continue */
                check[0] = 0;
                check[2] = 0;
                rng_manager((uint32_t*)&check[0]);
                rng_manager((uint32_t*)&check[2]);
                continue;
            }
        }
#endif
        /*effective sector erase, with retry (max 3) */
        uint8_t retry = 3;
        do {
            ret = flash_sector_erase(sectors_toerase[i]);
            retry--;
        } while (ret == 0xff && retry > 0);
#if LOADER_ERASE_WITH_RECOVERY
        do {
            otp_done = sectrue;
            /* write and lock OTP block 0 */
            flash_write_otp_block(i, &data[0], 4);
            /* the b.x here can be faulted (FIA) , making the write otp or otp lock
             * failing. The flash_write and flash_lock must finish with a final
             * flash_read_otp_block(i) to be sure that the block is effectively written
             * (at least). */
            flash_read_otp_block(i, &check[0], 4);
            if ((check[0] != data[0]) ||
                    (check[1] != data[1]) ||
                    (check[2] != data[2]) ||
                    (check[3] != data[3])) {
                log_printf("corruption while setting flash OTP sector !!! FIA ?\n");
                otp_done = secfalse;
            }
        } while (otp_done == secflase);
        flash_lock_otp_block(i);
#endif
    }
	return;
}


/* Macro for programming factorization */
#define flash_program(addr, elem, elem_cfg) do {\
	/* Check that the BSY bit in the FLASH_SR reg is not set */\
	if (flash_is_busy()) {\
		log_printf("Flash busy. Should not happen\n");\
        flash_busy_wait();\
	}\
	/* Set PSIZE for 64 bits writing */\
	set_reg(r_CORTEX_M_FLASH_CR, (elem_cfg), FLASH_CR_PSIZE);\
	/* Set PG bit */\
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_PG);\
	/* Perform data write op */\
	*(addr) = (elem);\
	/* Wait for BSY bit to be cleared */\
	flash_busy_wait();\
} while(0);

/**
 * \brief Write 64-bit-long data
 *
 * Program erased flash (because successive write op
 * are possible when writing '0' (from a '1')
 * As today, need an extern lock and erase. May be
 * integrated in this function in the future ?
 */
void flash_program_dword(uint64_t *addr, uint64_t value)
{
    if (is_sector_start((physaddr_t)addr) == true) {
        if (flash_sector_erase((physaddr_t)addr) == 0xff) {
            goto err;
        }
    }
	flash_program(addr, value, 3);
    if (flash_has_programming_errors()) {
        goto err;
    }
    return;
err:
    log_printf("error while programming sector at addr %x\n", addr);
    return;
}

/**
 * \brief Write 32-bit-long data
 *
 * Program erased flash (because successive write op
 * are possible when writing '0' (from a '1')
 * As today, need an extern lock and erase. May be
 * integrated in this function in the future ?
 */
void flash_program_word(uint32_t *addr, uint32_t value)
{
    if (is_sector_start((physaddr_t)addr) == true) {
        log_printf("starting programing new sector (@%x)\n", addr);
        if (flash_sector_erase((physaddr_t)addr) == 0xff) {
            goto err;
        }
    }
	flash_program(addr, value, 2);
    if (flash_has_programming_errors()) {
        goto err;
    }
    return;
err:
    log_printf("error while programming sector at addr %x\n", addr);
    return;

}

/**
 * \brief Write 16-bit-long data
 *
 * Program erased flash (because successive write op
 * are possible when writing '0' (from a '1')
 * As today, need an extern lock and erase. May be
 * integrated in this function in the future ?
 */
void flash_program_hword(uint16_t *addr, uint16_t value)
{
    if (is_sector_start((physaddr_t)addr) == true) {
        if (flash_sector_erase((physaddr_t)addr) == 0xff) {
            goto err;
        }
    }
	flash_program(addr, value, 1);
    if (flash_has_programming_errors()) {
        goto err;
    }
    return;
err:
    log_printf("error while programming sector at addr %x\n", addr);
    return;
}

/**
 * \brief Write 8-bit-long data
 *
 * Program erased flash (because successive write op
 * are possible when writing '0' (from a '1')
 * As today, need an extern lock and erase. May be
 * integrated in this function in the future ?
 */
void flash_program_byte(uint8_t *addr, uint8_t value)
{
    if (is_sector_start((physaddr_t)addr) == true) {
        if (flash_sector_erase((physaddr_t)addr) == 0xff) {
            goto err;
        }
    }
	flash_program(addr, value, 0);
    if (flash_has_programming_errors()) {
        goto err;
    }
    return;
err:
    log_printf("error while programming sector at addr %x\n", addr);
    return;
}


/**
 * \brief Read from flash memory
 *
 * @param addr		Adress to read from
 * @param size		Size to read
 * @param buffer	Buffer to write in
 */
void flash_read(uint8_t *buffer, physaddr_t addr, uint32_t size)
{
	if (!IS_IN_FLASH(addr)) {
		log_printf("Read not authorized (not in flash memory)\n");
		while(1){};
	}
	/* Copy data into buffer */
	memcpy(buffer, (void*)addr, size);
}


#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)      /*  Dual blank only on f42xxx/43xxx */
/**
 * \brief Get the current used bank configuration
 *
 * Only for boards with dual bank flah memory
 *
 * @return Bank configuration which is currently used
 */
uint8_t flash_get_bank_conf(void)
{
#if CONFIG_USR_DRV_FLASH_1M
	return get_reg(r_CORTEX_M_FLASH_OPTCR, FLASH_OPTCR_DB1M) == 0 ? 0 : 1;
#else
    /* always in dual bank in 2M mode */
    return 1;
#endif
}

/**
 * \brief Set bank memory configuration between single/dual bank organization
 *
 * Only for boards with dual bank flah memory
 *
 * @param conf 0 (default) :single (12 sectors) / 1 : dual (24 sectors)
 */
void flash_set_bank_conf(uint8_t conf __attribute__((unused)))
{
#if CONFIG_USR_DRV_FLASH_1M
	if (conf){
		conf = 1;
	}
	set_reg(r_CORTEX_M_FLASH_OPTCR, conf, FLASH_OPTCR_DB1M);
#endif
    /* with 2Mbytes flash mode, only dual bank mode is supported */
}
#endif

/**
 * \brief Return sector size in bytes
 *
 * FIXME
 *
 * @return Sector size
 */
uint32_t flash_sector_size(uint8_t sector)
{
	switch(sector){
		case 0:
			return FLASH_SECTOR_SIZE(0);
		case 1:
			return FLASH_SECTOR_SIZE(1);
		case 2:
			return FLASH_SECTOR_SIZE(2);
		case 3:
			return FLASH_SECTOR_SIZE(3);
		case 4:
			return FLASH_SECTOR_SIZE(4);
		case 5:
			return FLASH_SECTOR_SIZE(5);
		case 6:
			return FLASH_SECTOR_SIZE(6);
		case 7:
			return FLASH_SECTOR_SIZE(7);
# if (CONFIG_USR_DRV_FLASH_1M && !CONFIG_USR_DRV_FLASH_DUAL_BANK) || CONFIG_USR_DRV_FLASH_2M
    /* 1MB flash in dual banking doesn't have these 4 sectors */
		case 8:
			return FLASH_SECTOR_SIZE(8);
		case 9:
			return FLASH_SECTOR_SIZE(9);
		case 10:
			return FLASH_SECTOR_SIZE(10);
		case 11:
			return FLASH_SECTOR_SIZE(11);
#endif
#if (CONFIG_USR_DRV_FLASH_1M && CONFIG_USR_DRV_FLASH_DUAL_BANK) || CONFIG_USR_DRV_FLASH_2M
		case 12:
			return FLASH_SECTOR_SIZE(12);
		case 13:
			return FLASH_SECTOR_SIZE(13);
		case 14:
			return FLASH_SECTOR_SIZE(14);
		case 15:
			return FLASH_SECTOR_SIZE(15);
		case 16:
			return FLASH_SECTOR_SIZE(16);
		case 17:
			return FLASH_SECTOR_SIZE(17);
		case 18:
			return FLASH_SECTOR_SIZE(18);
		case 19:
			return FLASH_SECTOR_SIZE(19);
    /* 1MB flash in dual banking finishes here */
#endif
# if CONFIG_USR_DRV_FLASH_2M
		case 20:
			return FLASH_SECTOR_SIZE(20);
		case 21:
			return FLASH_SECTOR_SIZE(21);
		case 22:
			return FLASH_SECTOR_SIZE(22);
		case 23:
			return FLASH_SECTOR_SIZE(23);
    /*2MB flash in dual banking finishes here */
#endif
		default:
			log_printf("[Flash] Error: bad sector %d\n", sector);
			return 0;
	}
}


/* [RB] FIXME: do we really need such a complex function here?? */
/**
 * \brief Copy one flash sector into another
 *
 * FIXME Verify size of both sectors accessed !
 *
 * @param dest Destination address
 * @param src Sourcr address
 */
void flash_copy_sector(physaddr_t dest, physaddr_t src)
{
	/* Set up variables */
	uint8_t sector = 20;
	uint8_t buffer[64];
	uint32_t i = 0, j = 0, k = 0, sector_size = 0;
	if ((!IS_IN_FLASH(dest)) || (!IS_IN_FLASH(src))) {
		log_printf("Read not authorized (not in flash memory)\n");
		while(1){};
	}
	memset(buffer, 0, 64);
	/* Erase sector */
	sector = flash_sector_erase((physaddr_t)dest);
	/* Get sector size */
	sector_size = flash_sector_size(sector);
	/* Perform copy */
	for (i = 0; i < sector_size; i++) { /* Go through each kB of the sector */
		log_printf("#%d ", i);
		for (j = 0; j < 16; j++) { /* Go through each 64B packet of each kB */
			/* Read packet to copy */
			flash_read(buffer, (src+(i<<8)+(j<<4)), 64); /* !!  sigh 32 bit addr, >>2  */
			log_printf("Buffer #%d:\n", j+i*2);
			for (k = 0; k < 64; k++)
				log_printf("%x ", buffer[k]);
			log_printf("\n");
			/* Write packet by 1B */
			for (k = 0; k < 64; k++)
				flash_program_byte((uint8_t *)(dest)+(i<<10)+(j<<6)+k, buffer[k]);
			flash_read(buffer,(dest+(i<<8)+(j<<4)), 64);
			log_printf("Dest:\n");
			for (k = 0; k < 64; k++)
				log_printf("%x ", buffer[k]);
			log_printf("\n");
		} /* j loop */
	} /* i loop */
	log_printf("End of copy\n");
}

void flash_writelock_bank1(void)
{
#if CONFIG_FIRMWARE_DUALBANK
# if CONFIG_USR_DRV_FLASH_2M
    set_reg(r_CORTEX_M_FLASH_OPTCR, 0x000, FLASH_OPTCR_nWRP);
# else
    log_printf("not yet implemented");
# endif
#endif
}

void flash_writelock_bank2(void)
{
#if CONFIG_FIRMWARE_DUALBANK && CONFIG_USR_DRV_FLASH_2M


    set_reg(r_CORTEX_M_FLASH_OPTCR1, 0x000, FLASH_OPTCR1_nWRP);
#endif
}

void flash_writeunlock_bank1(void)
{
#if CONFIG_FIRMWARE_DUALBANK
# if CONFIG_USR_DRV_FLASH_2M
    set_reg(r_CORTEX_M_FLASH_OPTCR, 0xFFF, FLASH_OPTCR_nWRP);
# else
    log_printf("not yet implemented");
# endif
#endif
}

void flash_writeunlock_bank2(void)
{
#if CONFIG_FIRMWARE_DUALBANK && CONFIG_USR_DRV_FLASH_2M
    set_reg(r_CORTEX_M_FLASH_OPTCR1, 0xFFF, FLASH_OPTCR1_nWRP);
#endif
}

t_flash_rdp_state flash_check_rdpstate(void)
{
    uint32_t val = get_reg(r_CORTEX_M_FLASH_OPTCR, FLASH_OPTCR_RDP);
    if (val == 0xAA) {
        return FLASH_RDP_DEACTIVATED;
    } else if (val == 0xCC) {
        return FLASH_RDP_CHIPPROTECT;
    }
    return FLASH_RDP_MEMPROTECT;
}

void flash_lock_bootloader(void)
{
    /* set write mode protection to write protect (not PCROP) */
#ifdef STM32F429                /* STM32F42xxx/STM32F43xxx only */
    set_reg(r_CORTEX_M_FLASH_OPTCR, 0x0, FLASH_OPTCR_SPRMOD);
#endif
    /* lock bootloader and bootinfo write access on flashbank1 */
    set_reg(r_CORTEX_M_FLASH_OPTCR, 0xFFc, FLASH_OPTCR_nWRP);
#if CONFIG_FIRMWARE_DUALBANK && CONFIG_USR_DRV_FLASH_2M
# ifdef STM32F429                /* STM32F42xxx/STM32F43xxx only */
    set_reg(r_CORTEX_M_FLASH_OPTCR1, 0x0, FLASH_OPTCR_SPRMOD);
# endif
    set_reg(r_CORTEX_M_FLASH_OPTCR1, 0xFFc, FLASH_OPTCR_nWRP);
#endif
}


int flash_lock_otp_block(uint8_t block_id)
{
    if (block_id >= 16) {
        return 1;
    }
    flash_busy_wait();
    flash_unlock();
    /* 1 byte parallelism */
	set_reg(r_CORTEX_M_FLASH_CR, 0x00, FLASH_CR_PSIZE);
    /* programming mode */
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_PG);

    uint8_t *lock_block = FLASH_OTP_LOCK_BLOCK;
    /* set corresponding OTP lock byte */
    lock_block[block_id] = 0x00;
    flash_busy_wait();
    return 0;
}

int flash_read_otp_block(uint8_t block_id, uint32_t *data, uint32_t data_len)
{
    uint32_t *otp_block;
    if (block_id >= 16) {
        return 1;
    }
    if (data == NULL) {
        return 2;
    }
    if (data_len > 4) {
        return 3;
    }
    flash_busy_wait();
    switch (block_id) {
        case 0:
            otp_block = FLASH_OTP_BLOCK_0;
            break;
        case 1:
            otp_block = FLASH_OTP_BLOCK_1;
            break;
        case 2:
            otp_block = FLASH_OTP_BLOCK_2;
            break;
        case 3:
            otp_block = FLASH_OTP_BLOCK_3;
            break;
        case 4:
            otp_block = FLASH_OTP_BLOCK_4;
            break;
        case 5:
            otp_block = FLASH_OTP_BLOCK_5;
            break;
        case 6:
            otp_block = FLASH_OTP_BLOCK_6;
            break;
        case 7:
            otp_block = FLASH_OTP_BLOCK_7;
            break;
        case 8:
            otp_block = FLASH_OTP_BLOCK_8;
            break;
        case 9:
            otp_block = FLASH_OTP_BLOCK_9;
            break;
        case 10:
            otp_block = FLASH_OTP_BLOCK_10;
            break;
        case 11:
            otp_block = FLASH_OTP_BLOCK_11;
            break;
        case 12:
            otp_block = FLASH_OTP_BLOCK_12;
            break;
        case 13:
            otp_block = FLASH_OTP_BLOCK_13;
            break;
        case 14:
            otp_block = FLASH_OTP_BLOCK_14;
            break;
        case 15:
            otp_block = FLASH_OTP_BLOCK_15;
            break;
        default:
            return 1;
    }

    for (uint8_t i = 0; i < data_len; ++i) {
        data[i] = otp_block[i];
    }
    return 0;
}

int flash_write_otp_block(uint8_t block_id, uint32_t *data, uint32_t data_len)
{
    if (block_id >= 16) {
        return 1;
    }
    if (data == NULL) {
        return 2;
    }
    if (data_len > 4 || data_len < 1) {
        return 3;
    }
    flash_busy_wait();
    flash_unlock();
    /* 32 bits parallelism */
	set_reg(r_CORTEX_M_FLASH_CR, 0x10, FLASH_CR_PSIZE);
    /* programming mode */
	set_reg(r_CORTEX_M_FLASH_CR, 1, FLASH_CR_PG);
    uint32_t *otp_block = NULL;
    switch (block_id) {
        case 0:
            otp_block = FLASH_OTP_BLOCK_0;
            break;
        case 1:
            otp_block = FLASH_OTP_BLOCK_1;
            break;
        case 2:
            otp_block = FLASH_OTP_BLOCK_2;
            break;
        case 3:
            otp_block = FLASH_OTP_BLOCK_3;
            break;
        case 4:
            otp_block = FLASH_OTP_BLOCK_4;
            break;
        case 5:
            otp_block = FLASH_OTP_BLOCK_5;
            break;
        case 6:
            otp_block = FLASH_OTP_BLOCK_6;
            break;
        case 7:
            otp_block = FLASH_OTP_BLOCK_7;
            break;
        case 8:
            otp_block = FLASH_OTP_BLOCK_8;
            break;
        case 9:
            otp_block = FLASH_OTP_BLOCK_9;
            break;
        case 10:
            otp_block = FLASH_OTP_BLOCK_10;
            break;
        case 11:
            otp_block = FLASH_OTP_BLOCK_11;
            break;
        case 12:
            otp_block = FLASH_OTP_BLOCK_12;
            break;
        case 13:
            otp_block = FLASH_OTP_BLOCK_13;
            break;
        case 14:
            otp_block = FLASH_OTP_BLOCK_14;
            break;
        case 15:
            otp_block = FLASH_OTP_BLOCK_15;
            break;
        default:
            return 1;
    }
    /* set corresponding OTP lock byte */
    for (uint8_t i = 0; i < data_len; ++i) {
        /* 128 bytes to write */
        otp_block[i] = data[i];
        flash_busy_wait();
    }
    return 0;
}
