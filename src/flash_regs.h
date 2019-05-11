#ifndef FLASH_REGS_H_
#define FLASH_REGS_H_

#include "autoconf.h"

#define FLASH_FLIP_BASE 0x08000000
#define FLASH_FLOP_BASE 0x01000000

#define FLASH_CTRL      0x40023C00
#define FLASH_CTRL_2    0x40023C00
#define FLASH_SYSTEM    0x1FFF0000
#define FLASH_OTP       0x1FFF7800
#define FLASH_OPB_BK1   0x1FFFC000
#define FLASH_OPB_BK2   0x1FFEC000


#define r_CORTEX_M_FLASH		REG_ADDR(FLASH_CTRL)

#define r_CORTEX_M_FLASH_ACR		(r_CORTEX_M_FLASH + (uint32_t)0x00)	/* FLASH access control register */
#define r_CORTEX_M_FLASH_KEYR		(r_CORTEX_M_FLASH + (uint32_t)0x01)	/* FLASH key register 1*/
#define r_CORTEX_M_FLASH_OPTKEYR	(r_CORTEX_M_FLASH + (uint32_t)0x02)	/* FLASH option key register */
#define r_CORTEX_M_FLASH_SR		(r_CORTEX_M_FLASH + (uint32_t)0x03)	/* FLASH status register */
#define r_CORTEX_M_FLASH_CR		(r_CORTEX_M_FLASH + (uint32_t)0x04)	/* FLASH control register */
#define r_CORTEX_M_FLASH_OPTCR		(r_CORTEX_M_FLASH + (uint32_t)0x05) 	/* FLASH option control register */
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)			/* FLASH option control register 1 (only on f42xxx/43xxx) */
	#define r_CORTEX_M_FLASH_OPTCR1		(r_CORTEX_M_FLASH + (uint32_t)0x06)
#endif

/*******************  FLASH_ACR register  *****************/
#define FLASH_ACR_LATENCY_Pos		0
#define FLASH_ACR_LATENCY_Msk		((uint32_t)7 << FLASH_ACR_LATENCY_Pos)
#define FLASH_ACR_PRFTEN_Pos		8
#define FLASH_ACR_PRFTEN_Msk		((uint32_t)1 << FLASH_ACR_PRFTEN_Pos)
#define FLASH_ACR_ICEN_Pos 		9
#define FLASH_ACR_ICEN_Msk		((uint32_t)1 << FLASH_ACR_ICEN_Pos)
#define FLASH_ACR_DCEN_Pos		10
#define FLASH_ACR_DCEN_Msk		((uint32_t)1 << FLASH_ACR_DCEN_Pos)
#define FLASH_ACR_ICRST_Pos		11
#define FLASH_ACR_ICRST_Msk		((uint32_t)1 << FLASH_ACR_ICRST_Pos)
#define FLASH_ACR_DCRST_Pos		12
#define FLASH_ACR_DCRST_Msk		((uint32_t)1 << FLASH_ACR_DCRST_Pos)

/*******************  Flash key register  ***************/
#define KEY1				((uint32_t)0x45670123)
#define KEY2				((uint32_t)0xCDEF89AB)

/*******************  Flash option key register  ***************/
#define OPTKEY1				((uint32_t)0x08192A3B)
#define OPTKEY2				((uint32_t)0x4C5D6E7F)

/*******************  FLASH_SR register  ******************/
#define FLASH_SR_EOP_Pos		0
#define FLASH_SR_EOP_Msk 		((uint32_t)1 << FLASH_SR_EOP_Pos)
#define FLASH_SR_OPERR_Pos		1
#define FLASH_SR_OPERR_Msk		((uint32_t)1 << FLASH_SR_OPERR_Pos)
#define FLASH_SR_WRPERR_Pos		4
#define FLASH_SR_WRPERR_Msk		((uint32_t)1 << FLASH_SR_WRPERR_Pos)
#define FLASH_SR_PGAERR_Pos		5
#define FLASH_SR_PGAERR_Msk		((uint32_t)1 << FLASH_SR_PGAERR_Pos)
#define FLASH_SR_PGPERR_Pos		6
#define FLASH_SR_PGPERR_Msk		((uint32_t)1 << FLASH_SR_PGPERR_Pos)
#define FLASH_SR_PGSERR_Pos		7
#define FLASH_SR_PGSERR_Msk		((uint32_t)1 << FLASH_SR_PGSERR_Pos)
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)			/* RDERR (only on f42xxx/43xxx) */
	#define FLASH_SR_RDERR_Pos		8
	#define FLASH_SR_RDERR_Msk		((uint32_t)1 << FLASH_SR_RDERR_Pos)
#endif
#define FLASH_SR_BSY_Pos   		16
#define FLASH_SR_BSY_Msk  		((uint32_t)1 << FLASH_SR_BSY_Pos)

/*******************  FLASH_CR register  ******************/
#define FLASH_CR_PG_Pos 		0
#define FLASH_CR_PG_Msk  		((uint32_t)1 << FLASH_CR_PG_Pos)
#define FLASH_CR_SER_Pos 		1
#define FLASH_CR_SER_Msk		((uint32_t)1 << FLASH_CR_SER_Pos)
#define FLASH_CR_MER_Pos		2
#define FLASH_CR_MER_Msk		((uint32_t)1 << FLASH_CR_MER_Pos)

#define FLASH_CR_SNB_Pos		3
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK) /*  Dual blank only on f42xxx/43xxx */
#define FLASH_CR_SNB_Msk		((uint32_t)0x1F << FLASH_CR_SNB_Pos)
#else
#define FLASH_CR_SNB_Msk		((uint32_t)0xF << FLASH_CR_SNB_Pos)
#endif
#define FLASH_CR_PSIZE_Pos		8
#define FLASH_CR_PSIZE_Msk		((uint32_t)3 << FLASH_CR_PSIZE_Pos)
#if defined(CONFIG_USR_DRV_FLASH_DUAL_BANK)			/* MER1 (only on f42xxx/43xxx) */
	#define FLASH_CR_MER1_Pos		15
	#define FLASH_CR_MER1_Msk		((uint32_t)1 << FLASH_CR_MER1_Pos)
#endif
#define FLASH_CR_STRT_Pos		16
#define FLASH_CR_STRT_Msk		((uint32_t)1 << FLASH_CR_STRT_Pos)
#define FLASH_CR_EOPIE_Pos		24
#define FLASH_CR_EOPIE_Msk		((uint32_t)1 << FLASH_CR_EOPIE_Pos)
#define FLASH_CR_ERRIE_Pos		25
#define FLASH_CR_ERRIE_Msk		((uint32_t)1 << FLASH_CR_ERRIE_Pos)
#define FLASH_CR_LOCK_Pos 		31
#define FLASH_CR_LOCK_Msk 		((uint32_t)1 << FLASH_CR_LOCK_Pos)

/*******************  FLASH_OPTCR register  ***************/
#define FLASH_OPTCR_OPTLOCK_Pos		0
#define FLASH_OPTCR_OPTLOCK_Msk		((uint32_t)1 << FLASH_OPTCR_OPTLOCK_Pos)
#define FLASH_OPTCR_OPTSTRT_Pos		1
#define FLASH_OPTCR_OPTSTRT_Msk		((uint32_t)1 << FLASH_OPTCR_OPTSTRT_Pos)
#define FLASH_OPTCR_BOR_LEV_Pos		2
#define FLASH_OPTCR_BOR_LEV_Msk		((uint32_t)3 << FLASH_OPTCR_BOR_LEV_Pos)
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)			/* BFB2 (only on f42xxx/43xxx) */
	#define FLASH_OPTCR_BFB2_Pos		4
	#define FLASH_OPTCR_BFB2_Msk		((uint32_t)1 << FLASH_OPTCR_BFB2_Pos)
#endif
#define FLASH_OPTCR_WDG_SW_Pos		5
#define FLASH_OPTCR_WDG_SW_Msk		((uint32_t)1 << FLASH_OPTCR_WDG_SW_Pos)
#define FLASH_OPTCR_nRST_STOP_Pos	6
#define FLASH_OPTCR_nRST_STOP_Msk	((uint32_t)1 << FLASH_OPTCR_nRST_STOP_Pos)
#define FLASH_OPTCR_nRST_STDBY_Pos	7
#define FLASH_OPTCR_nRST_STDBY_Msk	((uint32_t)1 << FLASH_OPTCR_nRST_STDBY_Pos)
#define FLASH_OPTCR_RDP_Pos		8
#define FLASH_OPTCR_RDP_Msk		((uint32_t)0xFF << FLASH_OPTCR_RDP_Pos)
#define FLASH_OPTCR_nWRP_Pos		16
#define FLASH_OPTCR_nWRP_Msk		((uint32_t)0x0FFF << FLASH_OPTCR_nWRP_Pos)
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)			/*  Only on f42xxx/43xxx */
	#define FLASH_OPTCR_DB1M_Pos		30
	#define FLASH_OPTCR_DB1M_Msk		((uint32_t)1 << FLASH_OPTCR_DB1M_Pos)
	#define FLASH_OPTCR_SPRMOD_Pos		31
	#define FLASH_OPTCR_SPRMOD_Msk		((uint32_t)1 << FLASH_OPTCR_SPRMOD_Pos)
#endif

/*******************  FLASH_OPTCR1 register  ***************/
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)			/*  Only on f42xxx/43xxx */
	#define FLASH_OPTCR1_nWRP_Pos		16
	#define FLASH_OPTCR1_nWRP_Msk		((uint32_t)0x0FFF << FLASH_OPTCR1_nWRP_Pos)
#endif


/* return true if the the address is in the flash memory */
#if CONFIG_USR_DRV_FLASH_1M
# if CONFIG_USR_DRV_FLASH_DUAL_BANK
#  define IS_IN_FLASH(addr)		((addr) >= FLASH_SECTOR_0) && \
					((addr) <= FLASH_SECTOR_19_END)
# else
#  define IS_IN_FLASH(addr)		((addr) >= FLASH_SECTOR_0) && \
					((addr) <= FLASH_SECTOR_11_END)
# endif
#elif CONFIG_USR_DRV_FLASH_2M
#  define IS_IN_FLASH(addr)		((addr) >= FLASH_SECTOR_0) && \
					((addr) <= FLASH_SECTOR_23_END)
#else
# error "Unkown flash size!"
#endif

#define FLASH_SECTOR_SIZE(sector)  (FLASH_SECTOR_##sector##_END-FLASH_SECTOR_##sector)

/*******************  Bits definition for FLASH_ACR register  *****************/
#define FLASH_ACR_LATENCY                    ((uint32_t)0x00000007)
#define FLASH_ACR_LATENCY_0WS                ((uint32_t)0x00000000)
#define FLASH_ACR_LATENCY_1WS                ((uint32_t)0x00000001)
#define FLASH_ACR_LATENCY_2WS                ((uint32_t)0x00000002)
#define FLASH_ACR_LATENCY_3WS                ((uint32_t)0x00000003)
#define FLASH_ACR_LATENCY_4WS                ((uint32_t)0x00000004)
#define FLASH_ACR_LATENCY_5WS                ((uint32_t)0x00000005)
#define FLASH_ACR_LATENCY_6WS                ((uint32_t)0x00000006)
#define FLASH_ACR_LATENCY_7WS                ((uint32_t)0x00000007)

#define FLASH_ACR_PRFTEN                     ((uint32_t)0x00000100)
#define FLASH_ACR_ICEN                       ((uint32_t)0x00000200)
#define FLASH_ACR_DCEN                       ((uint32_t)0x00000400)
#define FLASH_ACR_ICRST                      ((uint32_t)0x00000800)
#define FLASH_ACR_DCRST                      ((uint32_t)0x00001000)
#define FLASH_ACR_BYTE0_ADDRESS              ((uint32_t)0x40023C00)
#define FLASH_ACR_BYTE2_ADDRESS              ((uint32_t)0x40023C03)

/*******************  Bits definition for FLASH_SR register  ******************/
#define FLASH_SR_EOP                         ((uint32_t)0x00000001)
#define FLASH_SR_SOP                         ((uint32_t)0x00000002)
#define FLASH_SR_WRPERR                      ((uint32_t)0x00000010)
#define FLASH_SR_PGAERR                      ((uint32_t)0x00000020)
#define FLASH_SR_PGPERR                      ((uint32_t)0x00000040)
#define FLASH_SR_PGSERR                      ((uint32_t)0x00000080)
#if defined(CONFIG_STM32F439) || defined(CONFIG_STM32F429)      /*  Dual blank only on f42xxx/43xxx */
#define FLASH_SR_RDERR                       ((uint32_t)0x00000100)
#endif
#define FLASH_SR_BSY                         ((uint32_t)0x00010000)

/*******************  Bits definition for FLASH_CR register  ******************/
#define FLASH_CR_PG                          ((uint32_t)0x00000001)
#define FLASH_CR_SER                         ((uint32_t)0x00000002)
#define FLASH_CR_MER                         ((uint32_t)0x00000004)
#define FLASH_CR_SNB_0                       ((uint32_t)0x00000008)
#define FLASH_CR_SNB_1                       ((uint32_t)0x00000010)
#define FLASH_CR_SNB_2                       ((uint32_t)0x00000020)
#define FLASH_CR_SNB_3                       ((uint32_t)0x00000040)
#define FLASH_CR_PSIZE_0                     ((uint32_t)0x00000100)
#define FLASH_CR_PSIZE_1                     ((uint32_t)0x00000200)
#define FLASH_CR_STRT                        ((uint32_t)0x00010000)
#define FLASH_CR_EOPIE                       ((uint32_t)0x01000000)
#define FLASH_CR_LOCK                        ((uint32_t)0x80000000)

/*******************  Bits definition for FLASH_OPTCR register  ***************/
#define FLASH_OPTCR_OPTLOCK                  ((uint32_t)0x00000001)
#define FLASH_OPTCR_OPTSTRT                  ((uint32_t)0x00000002)
#define FLASH_OPTCR_BOR_LEV_0                ((uint32_t)0x00000004)
#define FLASH_OPTCR_BOR_LEV_1                ((uint32_t)0x00000008)
#define FLASH_OPTCR_BOR_LEV                  ((uint32_t)0x0000000C)
#define FLASH_OPTCR_WDG_SW                   ((uint32_t)0x00000020)
#define FLASH_OPTCR_nRST_STOP                ((uint32_t)0x00000040)
#define FLASH_OPTCR_nRST_STDBY               ((uint32_t)0x00000080)
#define FLASH_OPTCR_RDP_0                    ((uint32_t)0x00000100)
#define FLASH_OPTCR_RDP_1                    ((uint32_t)0x00000200)
#define FLASH_OPTCR_RDP_2                    ((uint32_t)0x00000400)
#define FLASH_OPTCR_RDP_3                    ((uint32_t)0x00000800)
#define FLASH_OPTCR_RDP_4                    ((uint32_t)0x00001000)
#define FLASH_OPTCR_RDP_5                    ((uint32_t)0x00002000)
#define FLASH_OPTCR_RDP_6                    ((uint32_t)0x00004000)
#define FLASH_OPTCR_RDP_7                    ((uint32_t)0x00008000)
#define FLASH_OPTCR_nWRP_0                   ((uint32_t)0x00010000)
#define FLASH_OPTCR_nWRP_1                   ((uint32_t)0x00020000)
#define FLASH_OPTCR_nWRP_2                   ((uint32_t)0x00040000)
#define FLASH_OPTCR_nWRP_3                   ((uint32_t)0x00080000)
#define FLASH_OPTCR_nWRP_4                   ((uint32_t)0x00100000)
#define FLASH_OPTCR_nWRP_5                   ((uint32_t)0x00200000)
#define FLASH_OPTCR_nWRP_6                   ((uint32_t)0x00400000)
#define FLASH_OPTCR_nWRP_7                   ((uint32_t)0x00800000)
#define FLASH_OPTCR_nWRP_8                   ((uint32_t)0x01000000)
#define FLASH_OPTCR_nWRP_9                   ((uint32_t)0x02000000)
#define FLASH_OPTCR_nWRP_10                  ((uint32_t)0x04000000)
#define FLASH_OPTCR_nWRP_11		             ((uint32_t)0x08000000)


#endif/*!FLASH_REGS_H_*/
