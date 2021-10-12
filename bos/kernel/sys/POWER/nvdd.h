/* @(#)58	1.7  src/bos/kernel/sys/POWER/nvdd.h, machdd, bos411, 9428A410j 4/19/94 18:26:05 */
#ifndef _H_NVDD
#define _H_NVDD
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/lockl.h> 

struct md_lock_t {
    int lock_ret;
    lock_t lock_val;
};

extern struct md_lock_t lock_md;

/*-------------------------------------------------------------------------*/
/* Definitions of NVRAM addresses                                          */

#define NVRAM_BASE		0xA00000

#define SCSI_BASE_ADDR 		0x0010	/* SCSI init slot 0       	*/
#define TR_BASE_ADDR		0x0102	/* Token Ring info		*/
#define IPLIST_PREVBOOT         0x0200	/* Previous boot dev descriptor */
#define IPLIST_NORMAL           0x0224	/* Normal boot list		*/
#define IPLIST_SERVICE          0x0278	/* Service mode boot list	*/
#define LED_ADDRESS             0x0300	/* LED data mirrored      	*/
#define OFF_CHKCOUNT 		0x0308	/* Checkstop count         	*/
#define OFF_CHKPTR   		0x030C	/* Checkstop logout ptr     	*/
#define OCS_EC_LEVEL		0x0310	/* OCS Code E/C Level		*/
#define OCS_EPROM_EC_LEVEL	0x0314
#define ROS_DISPLAY_LEDS        0x0315
#define LED_STRING_OUTPUT_AREA  0x0320
#define LED_STRING_DIAG_ADDR    0x0328
#define LED_STRING_OUTPUT_END   0x0362
#define MC_ERROR_SAVE           0x0368	/* Machine check save area	*/
#define OCS_COMMAND_INTF        0x037C
#define	OCS_INFO_AREA		0x0380
#define	SWDATA_BASE		0x4400
#define	SWDATA_BASE_8K		0x0800
/*-------------------------------------------------------------------------*/

/* These define the reserved blocks of nvram: */
#define CRC_SIZE		4	  /* crc takes up 4 bytes */
#define NVRAM_ERRLOG_BASE	0
#define NVRAM_ERRLOG_SIZE	0x400	  /* block size in bytes */
#define NVRAM_DUMP_BASE		(NVRAM_ERRLOG_BASE+NVRAM_ERRLOG_SIZE+CRC_SIZE)
#define NVRAM_DUMP_SIZE		0x200	  /* block size in bytes */

#define DUMP_NVBLOCK		2	  /* Index of dump block */
#define ERRLOG_NVBLOCK		3	  /* Index of error log block */
#define MAX_NVBLOCK		3	  /* Index of last reserved block */

/* This structure identifies all of the fixed blocks of NVRAM
   that are used.  The machdd needs to know this to handle crcs.
   The crc is stored in the last word of the block; the block is 
   actually len+4 bytes in size. */
struct nvblock {
	unsigned long base;		/* Base address of block */
	unsigned long len;		/* Length of block */
};

/*
 *      LEDADDR(n) : returns addr appropriate for MACH_DD_IO md_addr
 *              n = LED number, valid (0-7), effective (0-2)
 */
#define LEDADDR(n)	(LED_ADDRESS | (n))

#endif /* _H_NVDD */
