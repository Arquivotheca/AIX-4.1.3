/* @(#)05       1.2  src/bos/usr/ccs/lib/libtli/tdbg.h, libtli, bos411, 9428A410j 11/9/93 18:45:52 */
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef	_NO_PROTO
extern void tr_accept (int fd, int resfd, struct t_call *call, int code);
extern void tr_allocate (int fd, int struct_type, int fields, char *code);
extern void tr_bind (int fd, struct t_bind *req, struct t_bind *ret, int code);
extern void tr_close (int fd, int code);
extern void tr_connect (int fd, struct t_call *sndcall, struct t_call *rcvcall,
			int code);
extern void tr_free (int struct_type, int code);
extern void tr_getinfo (int fd, struct t_info *info, int code);
extern void tr_listen (int fd, struct t_call *call, int code);
extern void tr_look (int fd, int code);
extern void tr_open (char* name, int oflag, struct t_info *tinfo, int code);
extern void tr_optmgmt (int fd, struct t_optmgmt *req, struct t_optmgmt *ret, 
			int code);
extern void tr_rcv (int fd, char *buf, unsigned nbytes, int *flags, int code);
extern void tr_rcvconnect (int fd, struct t_call *call, int code);
extern void tr_rcvdis (int fd, struct t_discon *discon, int code);
extern void tr_rcvrel (int fd, int code);
extern void tr_rcvudata (int fd, struct t_unitdata *ud, int *flags, int code);
extern void tr_rcvuderr (int fd, struct t_uderr *uderr, int code);
extern void tr_snd (int fd, char *buf, unsigned nbytes, int flags, int code);
extern void tr_snddis (int fd,struct t_call *call, int code);
extern void tr_sndrel (int fd, int code);
extern void tr_sndudata (int fd, struct t_unitdata *ud, int code);
extern void tr_sync (int fd, int code);
extern void tr_unbind (int fd, int code);
#else
extern void tr_accept ();
extern void tr_allocate ();
extern void tr_bind ();
extern void tr_close ();
extern void tr_connect ();
extern void tr_free ();
extern void tr_getinfo ();
extern void tr_listen ();
extern void tr_look ();
extern void tr_open ();
extern void tr_optmgmt ();
extern void tr_rcv ();
extern void tr_rcvconnect ();
extern void tr_rcvdis ();
extern void tr_rcvrel ();
extern void tr_rcvudata ();
extern void tr_rcvuderr ();
extern void tr_snd ();
extern void tr_snddis ();
extern void tr_sndrel ();
extern void tr_sndudata ();
extern void tr_sync ();
extern void tr_unbind ();
#endif

