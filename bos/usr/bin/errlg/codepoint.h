/* @(#)84	1.3  src/bos/usr/bin/errlg/codepoint.h, cmderrlg, bos411, 9428A410j 6/11/91 07:57:23 */

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: Format of codepoint.cat file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * PROGRAM NAME:   errupdate/raslib.a
 * FILE NAME:      codepoint.h
 *
 *   20 byte header.
 *   array of cp_entry's.
 *   0-terminated text data.
 */

#define MAGIC "codepoint.cat456"	/* 16 characters */

#define BACKUP_CODE_PT	"/usr/lib/ras/codepoint.cat"

struct cp_hdr {
	char   cp_magic[16];			/* magic text string */
	int    cp_count;				/* number of entries in cp_cat */
	int    cp_fill;
};

struct cp_entry {
	unsigned short ce_alertid;		/* alert id */
	unsigned short ce_set;			/* set for this file */
	union {
		int   _ce_offset;			/* offset into file for text string */
		char *_ce_text;				/* in-core text pointer */
	} _c;
};
#define ce_offset _c._ce_offset
#define ce_text   _c._ce_text

