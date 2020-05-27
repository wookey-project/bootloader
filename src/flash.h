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
#ifndef _STM32F4XX_FLASH_H
#define _STM32F4XX_FLASH_H

#include "autoconf.h"
#include "regutils.h"
#include "types.h"


/*
 * This is the stm32f4xx sector mapping, for various STM32F4 SoCs
 */

/* Only STM32F429 and STM32F439 have dual 1MB banks */

# if CONFIG_USR_DRV_FLASH_1M
/* SoC with 1Mbyte flash. For these SoC, there can be two configurations:
 * single-bank of 1Mbytes memory length, or dual-bank of two 512Kbytes
 * memory length. Sectors structure are different
 * (See STM-RM0090, table 7.1)
 */

#define FLASH_SECTOR_0			((uint32_t) 0x08000000) /* 16 kB */
#define FLASH_SECTOR_0_END		((uint32_t) 0x08003FFF)
#define FLASH_SECTOR_1			((uint32_t) 0x08004000) /* 16 kB */
#define FLASH_SECTOR_1_END		((uint32_t) 0x08007FFF)
#define FLASH_SECTOR_2			((uint32_t) 0x08008000) /* 16 kB */
#define FLASH_SECTOR_2_END		((uint32_t) 0x0800BFFF)
#define FLASH_SECTOR_3			((uint32_t) 0x0800C000) /* 16 kB */
#define FLASH_SECTOR_3_END		((uint32_t) 0x0800FFFF)
#define FLASH_SECTOR_4			((uint32_t) 0x08010000) /* 64 kB */
#define FLASH_SECTOR_4_END		((uint32_t) 0x0801FFFF)
#define FLASH_SECTOR_5			((uint32_t) 0x08020000) /* 128 kB */
#define FLASH_SECTOR_5_END		((uint32_t) 0x0803FFFF)
#define FLASH_SECTOR_6			((uint32_t) 0x08040000) /* 128 kB */
#define FLASH_SECTOR_6_END		((uint32_t) 0x0805FFFF)
#define FLASH_SECTOR_7			((uint32_t) 0x08060000) /* 128 kB */
#define FLASH_SECTOR_7_END		((uint32_t) 0x0807FFFF)

#  if CONFIG_USR_DRV_FLASH_DUAL_BANK
/* in dual bank mode, the other bank is autonomous and has the same
 * sector structure as the first one
 */

#define FLASH_SECTOR_12			((uint32_t) 0x08080000) /* 16 kB */
#define FLASH_SECTOR_12_END		((uint32_t) 0x08083FFF)
#define FLASH_SECTOR_13			((uint32_t) 0x08084000) /* 16 kB */
#define FLASH_SECTOR_13_END		((uint32_t) 0x08087FFF)
#define FLASH_SECTOR_14			((uint32_t) 0x08088000) /* 16 kB */
#define FLASH_SECTOR_14_END		((uint32_t) 0x0808BFFF)
#define FLASH_SECTOR_15			((uint32_t) 0x0808C000) /* 16 kB */
#define FLASH_SECTOR_15_END		((uint32_t) 0x0808FFFF)
#define FLASH_SECTOR_16			((uint32_t) 0x08090000) /* 64 kB */
#define FLASH_SECTOR_16_END		((uint32_t) 0x0809FFFF)
#define FLASH_SECTOR_17			((uint32_t) 0x080A0000) /* 128 kB */
#define FLASH_SECTOR_17_END		((uint32_t) 0x080BFFFF)
#define FLASH_SECTOR_18			((uint32_t) 0x080C0000) /* 128 kB */
#define FLASH_SECTOR_18_END		((uint32_t) 0x080DFFFF)
#define FLASH_SECTOR_19			((uint32_t) 0x080E0000) /* 128 kB */
#define FLASH_SECTOR_19_END		((uint32_t) 0x080FFFFF)

#  else /* signe bank, continuing with 128kB sectors */

#define FLASH_SECTOR_8			((uint32_t) 0x08080000) /* 128 kB */
#define FLASH_SECTOR_8_END		((uint32_t) 0x0809FFFF)
#define FLASH_SECTOR_9			((uint32_t) 0x080A0000) /* 128 kB */
#define FLASH_SECTOR_9_END		((uint32_t) 0x080BFFFF)
#define FLASH_SECTOR_10			((uint32_t) 0x080C0000) /* 128 kB */
#define FLASH_SECTOR_10_END		((uint32_t) 0x080DFFFF)
#define FLASH_SECTOR_11			((uint32_t) 0x080E0000) /* 128 kB */
#define FLASH_SECTOR_11_END		((uint32_t) 0x080FFFFF)

#  endif /*!banking */

# else /* USR_DRV_FLASH_2M */
/* SoC with 2Mbyte flash. For these SoC, there in only one configuration:
 * a full 2Mbyte flash size in dual banking mode.
 * (See STM-RM0090, table 6)
 */

/* Bank 1 */
#define FLASH_SECTOR_0			((uint32_t) 0x08000000) /* 16 kB */
#define FLASH_SECTOR_0_END		((uint32_t) 0x08003FFF)
#define FLASH_SECTOR_1			((uint32_t) 0x08004000) /* 16 kB */
#define FLASH_SECTOR_1_END		((uint32_t) 0x08007FFF)
#define FLASH_SECTOR_2			((uint32_t) 0x08008000) /* 16 kB */
#define FLASH_SECTOR_2_END		((uint32_t) 0x0800BFFF)
#define FLASH_SECTOR_3			((uint32_t) 0x0800C000) /* 16 kB */
#define FLASH_SECTOR_3_END		((uint32_t) 0x0800FFFF)
#define FLASH_SECTOR_4			((uint32_t) 0x08010000) /* 64 kB */
#define FLASH_SECTOR_4_END		((uint32_t) 0x0801FFFF)
#define FLASH_SECTOR_5			((uint32_t) 0x08020000) /* 128 kB */
#define FLASH_SECTOR_5_END		((uint32_t) 0x0803FFFF)
#define FLASH_SECTOR_6			((uint32_t) 0x08040000) /* 128 kB */
#define FLASH_SECTOR_6_END		((uint32_t) 0x0805FFFF)
#define FLASH_SECTOR_7			((uint32_t) 0x08060000) /* 128 kB */
#define FLASH_SECTOR_7_END		((uint32_t) 0x0807FFFF)
#define FLASH_SECTOR_8			((uint32_t) 0x08080000) /* 128 kB */
#define FLASH_SECTOR_8_END		((uint32_t) 0x0809FFFF)
#define FLASH_SECTOR_9			((uint32_t) 0x080A0000) /* 128 kB */
#define FLASH_SECTOR_9_END		((uint32_t) 0x080BFFFF)
#define FLASH_SECTOR_10			((uint32_t) 0x080C0000) /* 128 kB */
#define FLASH_SECTOR_10_END		((uint32_t) 0x080DFFFF)
#define FLASH_SECTOR_11			((uint32_t) 0x080E0000) /* 128 kB */
#define FLASH_SECTOR_11_END		((uint32_t) 0x080FFFFF)


/* Bank 2 */
#define FLASH_SECTOR_12			((uint32_t) 0x08100000) /* 16 kB */
#define FLASH_SECTOR_12_END		((uint32_t) 0x08103FFF)
#define FLASH_SECTOR_13			((uint32_t) 0x08104000) /* 16 kB */
#define FLASH_SECTOR_13_END		((uint32_t) 0x08107FFF)
#define FLASH_SECTOR_14			((uint32_t) 0x08108000) /* 16 kB */
#define FLASH_SECTOR_14_END		((uint32_t) 0x0810BFFF)
#define FLASH_SECTOR_15			((uint32_t) 0x0810C000) /* 16 kB */
#define FLASH_SECTOR_15_END		((uint32_t) 0x0810FFFF)
#define FLASH_SECTOR_16			((uint32_t) 0x08110000) /* 64 kB */
#define FLASH_SECTOR_16_END		((uint32_t) 0x0811FFFF)
#define FLASH_SECTOR_17			((uint32_t) 0x08120000) /* 128 kB */
#define FLASH_SECTOR_17_END		((uint32_t) 0x0813FFFF)
#define FLASH_SECTOR_18			((uint32_t) 0x08140000) /* 128 kB */
#define FLASH_SECTOR_18_END		((uint32_t) 0x0815FFFF)
#define FLASH_SECTOR_19			((uint32_t) 0x08160000) /* 128 kB */
#define FLASH_SECTOR_19_END		((uint32_t) 0x0817FFFF)
#define FLASH_SECTOR_20			((uint32_t) 0x08180000) /* 128 kB */
#define FLASH_SECTOR_20_END		((uint32_t) 0x0819FFFF)
#define FLASH_SECTOR_21			((uint32_t) 0x081A0000) /* 128 kB */
#define FLASH_SECTOR_21_END		((uint32_t) 0x081BFFFF)
#define FLASH_SECTOR_22			((uint32_t) 0x081C0000) /* 128 kB */
#define FLASH_SECTOR_22_END		((uint32_t) 0x081DFFFF)
#define FLASH_SECTOR_23			((uint32_t) 0x081E0000) /* 128 kB */
#define FLASH_SECTOR_23_END		((uint32_t) 0x081FFFFF)

# endif

/*
 * System memory and One Time Programmable area are the same for all
 * SoCs
 */
#define FLASH_SECTOR_SYSTEM_MEM		((uint32_t) 0x1FFF0000) /* 30 kB */
#define FLASH_SECTOR_SYSTEM_MEM_END	((uint32_t) 0x1FFF77FF)
#define FLASH_SECTOR_OTP_AREA		((uint32_t) 0x1FFF7800) /* 528 B */
#define FLASH_SECTOR_OTP_AREA_END	((uint32_t) 0x1FFF7A0F)
#define FLASH_OPTION_BYTES		((uint32_t) 0x1FFFC000) /* 16 B */
#define FLASH_OPTION_BYTES_END		((uint32_t) 0x1FFFC00F)


#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)	/*  Only on f42xxx/43xxx */
	#define FLASH_OPTION_BYTES_SGL		((uint32_t) 0x1FFEC000)	/* 16 B */
	#define FLASH_OPTION_BYTES_SGL_END	((uint32_t) 0x1FFEC00F)
#endif

#define FLASH_OPTION_BYTES_BK1		((uint32_t) 0x1FFFC000) /* 16 B */
#define FLASH_OPTION_BYTES_BK1_END	((uint32_t) 0x1FFFC00F)
#define FLASH_OPTION_BYTES_BK2		((uint32_t) 0x1FFEC000)	/* 16 B */
#define FLASH_OPTION_BYTES_BK2_END	((uint32_t) 0x1FFEC00F)


/*
 * About OTP memory
 */
#define FLASH_OTP_BLOCK_0			((uint32_t*) 0x1FFF7800) /* 32 By */
#define FLASH_OTP_BLOCK_0_END		((uint32_t) 0x1FFF781F)
#define FLASH_OTP_BLOCK_1			((uint32_t*) 0x1FFF7820) /* 32 By */
#define FLASH_OTP_BLOCK_1_END		((uint32_t) 0x1FFF783F)
#define FLASH_OTP_BLOCK_2			((uint32_t*) 0x1FFF7840) /* 32 By */
#define FLASH_OTP_BLOCK_2_END		((uint32_t) 0x1FFF785F)
#define FLASH_OTP_BLOCK_3			((uint32_t*) 0x1FFF7860) /* 32 By */
#define FLASH_OTP_BLOCK_3_END		((uint32_t) 0x1FFF787F)
#define FLASH_OTP_BLOCK_4			((uint32_t*) 0x1FFF7880) /* 32 By */
#define FLASH_OTP_BLOCK_4_END		((uint32_t) 0x1FFF789F)
#define FLASH_OTP_BLOCK_5			((uint32_t*) 0x1FFF78A0) /* 32 By */
#define FLASH_OTP_BLOCK_5_END		((uint32_t) 0x1FFF78BF)
#define FLASH_OTP_BLOCK_6			((uint32_t*) 0x1FFF78C0) /* 32 By */
#define FLASH_OTP_BLOCK_6_END		((uint32_t) 0x1FFF78DF)
#define FLASH_OTP_BLOCK_7			((uint32_t*) 0x1FFF78E0) /* 32 By */
#define FLASH_OTP_BLOCK_7_END		((uint32_t) 0x1FFF78FF)
#define FLASH_OTP_BLOCK_8			((uint32_t*) 0x1FFF7900) /* 32 By */
#define FLASH_OTP_BLOCK_8_END		((uint32_t) 0x1FFF791F)
#define FLASH_OTP_BLOCK_9			((uint32_t*) 0x1FFF7920) /* 32 By */
#define FLASH_OTP_BLOCK_9_END		((uint32_t) 0x1FFF793F)
#define FLASH_OTP_BLOCK_10  		((uint32_t*) 0x1FFF7940) /* 32 By */
#define FLASH_OTP_BLOCK_10_END		((uint32_t) 0x1FFF795F)
#define FLASH_OTP_BLOCK_11  		((uint32_t*) 0x1FFF7960) /* 32 By */
#define FLASH_OTP_BLOCK_11_END		((uint32_t) 0x1FFF797F)
#define FLASH_OTP_BLOCK_12  		((uint32_t*) 0x1FFF7980) /* 32 By */
#define FLASH_OTP_BLOCK_12_END		((uint32_t) 0x1FFF799F)
#define FLASH_OTP_BLOCK_13  		((uint32_t*) 0x1FFF79A0) /* 32 By */
#define FLASH_OTP_BLOCK_13_END		((uint32_t) 0x1FFF79BF)
#define FLASH_OTP_BLOCK_14  		((uint32_t*) 0x1FFF79C0) /* 32 By */
#define FLASH_OTP_BLOCK_14_END		((uint32_t) 0x1FFF79DF)
#define FLASH_OTP_BLOCK_15  		((uint32_t*) 0x1FFF79E0) /* 32 By */
#define FLASH_OTP_BLOCK_15_END		((uint32_t) 0x1FFF79FF)


/* array of 16 bytes (from 0 to 15). When the corresponding cell of the lock block
 * is set to 0x00, the associated OTP block is no more writeable */
#define FLASH_OTP_LOCK_BLOCK        ((uint8_t*) 0x1FFF7A00)

typedef enum {
    FLASH_RDP_DEACTIVATED = 0x85606b8c,
    FLASH_RDP_MEMPROTECT  = 0x75909b7c,
    FLASH_RDP_CHIPPROTECT = 0x8a6f6483
} t_flash_rdp_state;

/******* Flash operations **********/
int flash_unlock(void);

void flash_unlock_opt(void);

void flash_lock(void);

void flash_lock_opt(void);

uint8_t flash_select_sector(physaddr_t addr);

uint8_t flash_sector_erase(physaddr_t addr);

#ifdef CONFIG_LOADER_ERASE_WITH_RECOVERY
secbool flash_mass_erase_ongoing(void);
#endif

void flash_mass_erase(void);

void flash_program_dword(uint64_t *addr, uint64_t value);

void flash_program_word(uint32_t *addr, uint32_t word);

void flash_program_byte(uint8_t *addr, uint8_t value);

void flash_read(uint8_t *buffer, physaddr_t addr, uint32_t size);

#if defined(USR_DRV_FLASH_DUAL_BANK)	/*  Only on f42xxx/43xxx */
uint8_t flash_get_bank_conf(void);

void flash_set_bank_conf(uint8_t conf);
#endif

void flash_copy_sector(physaddr_t dest, physaddr_t src);

uint32_t flash_sector_size(uint8_t sector);

void flash_writelock_bank1(void);

void flash_writelock_bank2(void);

void flash_writeunlock_bank1(void);

void flash_writeunlock_bank2(void);

t_flash_rdp_state flash_check_rdpstate(void);

void flash_writelock_bootloader(void);

void flash_writeunlock_bootloader(void);

int flash_read_otp_block(uint8_t block_id, uint32_t *data, uint32_t data_len);

int flash_write_otp_block(uint8_t block_id, uint32_t *data, uint32_t data_len);

int flash_lock_otp_block(uint8_t block_id);

#endif /* _STM32F4XX_FLASH_H */

