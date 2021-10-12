/* @(#)11       1.1.2.3  src/bos/usr/ccs/lib/libtli/tlistate.h, libtli, bos411, 9428A410j 7/12/94 12:23:15 */
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 18 27 63
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1990  Mentat Inc.
 ** tlistate.h 1.1, last change 4/14/90
 **/

#ifndef	_TLISTATE_
#define	_TLISTATE_

struct tli_st {
	struct  tli_st  *tlis_next;
	int     tlis_fd;
	int	tlis_state;	/* state */
	int	tlis_servtype;	/* server type */
	int	tlis_flags;	/* BLC sez: now we're real! */
	long    tlis_etsdu_size;        /* Transport service data unit size */
	long    tlis_tsdu_size;         /* Transport service data unit size */
	long    tlis_tidu_size;         /* Transport individual data unit size*/
	char    *tlis_proto_buf;
	void    *tlis_lock;
	int     tlis_sequence;
};

/* Flags! */
#define	TLIS_DATA_STOPPED	0x0001
#define	TLIS_EXDATA_STOPPED	0x0002
#define	TLIS_SAVED_PROTO	0x0004
#define	TLIS_MORE_RUDATA	0x0008
#define TLIS_MORE_DATA		0x0010
#define TLIS_SAVED_DATA		0x0020
#define TLIS_SAVED_EXDATA	0x0040

#define	IOSTATE_VERIFY		0x0001
#define	IOSTATE_SYNC		0x0002
#define	IOSTATE_FREE		0x0004

extern  struct tli_st * iostate_lookup(int, int);


#endif	/*_TLISTATE_*/
