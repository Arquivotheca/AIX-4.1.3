/* @(#)00	1.3  src/bos/kernext/tok/tr_mon_dds.h, sysxtok, bos411, 9428A410j 11/8/93 11:03:21 */
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TR_MON_DDS
#define _H_TR_MON_DDS

#include <sys/cdli_tokuser.h>

/*
 *	DDS structure
 */

struct tr_mon_dds {
	uchar	lname[8];		/* Logical name in ASCII characters */
	uchar	alias[8];		/* Alias name in ASCII characters */

	int	bus_type;		/* for use with i_init */
	int	bus_id;			/* for use with i_init */
	int	intr_level;		/* for use with i_init */
	int	intr_priority;		/* for use with i_init */

	uchar	*tcw_bus_mem_addr;	/* tcw bus memory base address */
	int	dma_arbit_lvl;		/* DMA Arbitration Level */
	ulong	io_port;		/* starting address of I/O registers */
	int	slot;			/* slot for this device */

	int	xmt_que_size;		/* 1 queue/device for xmit buffering */
	int	ring_speed;		/* ring data rate: 0==4M/1==16M */
	int	use_alt_addr;		/* use alt network addr: 1==yes */
	int	attn_mac;		/* rcv attention MAC frames: 1==yes */
	int	beacon_mac;		/* rcv beacon MAC frames: 1==yes */
	uchar	alt_addr[CTOK_NADR_LENGTH];	/* alternate network address */
};
typedef struct tr_mon_dds tr_mon_dds_t;

/*
 *      Structure for downloading adapter microcode
 */

struct tok_download {
	char	*p_mcload;		/* microcode loader image pointer */
	int	l_mcload;		/* microcode loader length */
	char	*p_mcode;		/* microcode image pointer */
	int	l_mcode;		/* microcode length */
};
typedef struct tok_download tok_download_t;

/*
 *	Structure for reading VPD from the device driver
 */

#define TOK_VPD_VALID      0x00		/* VPD obtained is valid */
#define TOK_VPD_INVALID    0x02		/* VPD obtained is invalid */
#define TOK_VPD_LENGTH     0x67		/* VPD length of  bytes */

struct tok_vpd {
	ulong	status;			/* status of VPD */
	ulong	l_vpd;			/* length of VPD returned */
					/* (may be <= TOK_VPD_LENGTH) */
	uchar	vpd[TOK_VPD_LENGTH];    /* VPD */
};
typedef struct tok_vpd tok_vpd_t;

#endif /* _H_TR_MON_DDS */
