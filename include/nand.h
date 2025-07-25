/*
 * Copyright (C) 2006 Microchip Technology Inc. and its subsidiaries
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef __NAND_H__
#define __NAND_H__

#define MAX_ECC_BYTES		512 /* maximum bytes of ecc */

/* Maximum size of the data area of one page, in bytes. */
#define NAND_MAX_PAGE_DATA_SIZE          8192

/* Maximum size of the spare area of one page, in bytes. */
#define NAND_MAX_PAGE_SPARE_SIZE         512


#if defined(CONFIG_SAMA5D2) || defined(CONFIG_SAMA5D3) ||\
    defined(CONFIG_SAMA5D4) || defined(CONFIG_SAMA7G5) ||\
    defined(CONFIG_SAMA7D65)
#define SMC_BASE	ATMEL_BASE_SMC
#else
#define SMC_BASE	AT91C_BASE_SMC
#endif

#define TIMING_MODE_0	0
#define TIMING_MODE_1	1
#define TIMING_MODE_2	2
#define TIMING_MODE_3	3

struct nand_timing {
	unsigned int tCS;
	unsigned int tRC;
	unsigned int tREH;
	unsigned int tRHOH;
	unsigned int tRP;
	unsigned int tWC;
	unsigned int tWH;
	unsigned int tWP;
	unsigned int tRHZ;
	unsigned int tCLR;
	unsigned int tADL;
	unsigned int tAR;
	unsigned int tREA;
	unsigned int tRR;
	unsigned int tWB;
};

struct nand_ooblayout {
	unsigned short	badblockpos;
	unsigned short	eccbytes;
	unsigned short	eccpos[MAX_ECC_BYTES];
};

struct nand_chip {
	unsigned short	chip_id; /* Nand Chip ID */
	unsigned short	numblocks;
	unsigned int	blocksize;
	unsigned short	pagesize;
	unsigned short	oobsize;
	unsigned char	buswidth;
	unsigned char	eccbits;
	unsigned int	eccwordsize;
	unsigned short  opt_cmd;
	unsigned short  timingmode;
};

struct nand_info {
	unsigned int	blocksize;	/* size of a block */
	unsigned int	pagesize;	/* size of a page */
	unsigned int	oobsize;	/* size of a oob */
	unsigned int	sectorsize;	/* size of a sector */

	unsigned int	numblocks;	/* number of blocks in device */
	unsigned int	pages_device;	/* number of pages in device */
	unsigned int	pages_block;	/* number of pages in block */

	unsigned int	buswidth;	/* data bus width (8/16 bits) */

	void (*command)(unsigned char cmd);
	void (*address)(unsigned char addr);

	struct nand_ooblayout	*ecclayout;
	/* Used by PMECC */
	int			ecc_sector_size;
	int			ecc_err_bits;
};

#define ZONE_DATA			0x01    /* Sector data zone */
#define ZONE_INFO			0x02    /* Sector info zone */

/* Nand flash chip status codes */
#define STATUS_READY			(0x01 << 6)   /* Status code for Ready */
#define STATUS_ERROR			(0x01 << 0)   /* Status code for Error */

/* Nand flash commands */
#define CMD_READ_1			0x00
#define CMD_READ_2			0x30

#define CMD_READID			0x90

#define CMD_WRITE_1			0x80
#define CMD_WRITE_2			0x10

#define CMD_ERASE_1			0x60
#define CMD_ERASE_2			0xD0

#define CMD_STATUS			0x70
#define CMD_RESET			0xFF

/* Nand flash commands (small blocks) */
#define CMD_READ_A0			0x00
#define CMD_READ_A1			0x01
#define CMD_READ_C			0x50
#define CMD_READ_R			0x85
#define CMD_WRITE_A			0x00
#define CMD_WRITE_C			0x50

/* NandFlash ONFI */
#define CMD_READ_ONFI			0xEC

/* Feature Operations */
#define CMD_SET_FEATURE			0xEF
#define CMD_GET_FEATURE			0xEE

extern void nandflash_smc_conf(unsigned int mode, unsigned int cs);
#ifdef CONFIG_FAST_BOOT
extern int nand_flash_read(struct nand_info *nand, unsigned int address, unsigned int length,
	void *buf);
extern int nand_flash_write(struct nand_info *nand, unsigned int address, unsigned int length,
	const void *buf);
#endif
#endif /* #ifndef __NAND_H__ */
