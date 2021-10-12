/* @(#)55 1.3 src/bos/kernext/tty/stream_sjis.h, sysxldterm, bos412, 9445C412a 9/26/94 14:15:28 */
/*
 * COMPONENT_NAME:
 *
 * FUNCTIONS:
 *
 * ORIGINS: 40, 71, 83
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_STREAM_SJIS
#define _H_STREAM_SJIS

#include <sys/stream.h>
#include <sys/str_tty.h>

/*
 * General definition for sjis converters in configuratio methods for the
 * dds type. This definition uses the include file "str_tty.h".
 */
struct  lc_sjis_dds {
        enum    dds_type        which_dds;      /* dds identifier       */
};

struct  uc_sjis_dds {
        enum    dds_type        which_dds;      /* dds identifier       */
};

/*
 * shift jis conversion module specific structures/definitions
 */
struct sjis_s {
	int flags;		/* conversion flags */
	int flags_save;		/* for EUC_MSAVE/EUC_MREST */
	int c1state;		/* C1 conversion state */
	int rbid;		/* bufcall id on read side */
	int wbid;		/* bufcall id on write side */
	int rtid;		/* timeout id on read side */
	int wtid;		/* timeout id on write side */
	mblk_t *rspare;		/* spare mblk on read side */
	mblk_t *wspare;		/* spare mblk on write side */
	unsigned char sac;	/* char left over sjis->ajec queue */
	unsigned char asc[2];	/* chars left over ajec->sjis queue */
	unsigned char sti_c;	/* TIOCSTI character */
};

#define KS_ICONV	0x0001	/* read conversion on */
#define KS_OCONV	0x0002	/* write conversion on */
#define KS_IEXTEN	0x0004	/* IEXTEN flag is on in line discipline */

#define LC_SJIS_LOWAT	128
#define LC_SJIS_HIWAT	2048

#define UC_SJIS_LOWAT	128
#define UC_SJIS_HIWAT	2048

#define SS2		0x8e
#define SS3		0x8f

#define INRANGE(x1,x2,vv) (((unsigned)((vv)-(x1))) <= ((unsigned)((x2)-(x1))))

/* 
 * For compatibility.
 */
#define	bzero(a, b)	bzero(a, b)
#define bcopy(a, b, c)	bcopy(a, b, c)

/*
 * lower converter module for shift jis
 */
#define LC_SJIS_MODULE_ID    7012
#define LC_SJIS_MODULE_NAME  "lc_sjis"

/*
 * upper converter module for shift jis
 */
#define UC_SJIS_MODULE_ID    7002
#define UC_SJIS_MODULE_NAME  "uc_sjis"

void
conv_sjis2ajec(mblk_t *, mblk_t *, struct sjis_s *);

void
conv_ajec2sjis(mblk_t *, mblk_t *, struct sjis_s *);

int
lc_sjis_close(queue_t *, int, cred_t *);

int
lc_sjis_ioctl(struct sjis_s *, queue_t *, mblk_t *);

int
lc_sjis_open(queue_t *, dev_t *, int, int, cred_t *);

int
lc_sjis_rput(queue_t *, mblk_t *);

int
lc_sjis_rsrv(queue_t *);

int
lc_sjis_wput(queue_t *, mblk_t *);

int
lc_sjis_wsrv(queue_t *);

int
sjis_readdata(struct sjis_s *, queue_t *, mblk_t *, void (*)(), int);

int
sjis_writedata(struct sjis_s *, queue_t *, mblk_t *, void (*)(), int);

int
uc_sjis_close(queue_t *, int, cred_t *);

int
uc_sjis_open(queue_t *, dev_t *, int, int, cred_t *);

int
sjis_proc_sti(struct sjis_s *, unsigned char, queue_t *);

int
uc_sjis_rput(queue_t *, mblk_t *);

int
uc_sjis_rsrv(queue_t *);

int
sjis_send_sti(queue_t *, unsigned char *, int);

int
uc_sjis_wput(queue_t *, mblk_t *);

int
uc_sjis_wsrv(queue_t *);

#endif	/* _H_STREAM_SJIS	*/
