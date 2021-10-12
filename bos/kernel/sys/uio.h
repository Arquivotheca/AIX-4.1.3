/* @(#)63	1.21  src/bos/kernel/sys/uio.h, sysios, bos411, 9431A411a 8/4/94 15:29:26 */
#ifndef _H_UIO_
#define _H_UIO_
/*
 * COMPONENT_NAME: (SYSIOS) User I/O services header file
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)uio.h	7.1 (Berkeley) 6/4/86
 */

#include <sys/xmem.h>
#include <sys/types.h>

struct iovec {
	caddr_t iov_base;	/* base memory address			*/
	int	iov_len;	/* length of transfer for this area	*/
};

struct uio {
	struct	iovec *uio_iov;	/* ptr to array of iovec structs	*/
	struct	xmem  *uio_xmem;/* ptr to array of xmem structs		*/
	int	uio_iovcnt;	/* #iovec elements remaining to be processed*/
	int	uio_iovdcnt;	/* #iovec elements already processed	*/
#ifdef _LONG_LONG
	offset_t uio_offset;	/* byte offset in file/dev to read/write*/
#else
	int	uio_rsvd;	/* ANSI-C does not support long long */ 	
	off_t	uio_offset; 	/* off_t offset for ANSI-C mode      */
#endif 
	int	uio_resid;	/* #bytes left in data area		*/
	short	uio_segflg;	/* see segment flag value defines below */
	long	uio_fmode;	/* copy of file modes from open file struct */
};

/*
 * Control structure used by pinu/unpinu.  Head of list in U.U_pinu_block.
 */
struct pinu_block {
	struct pinu_block *next;	/* pointer to next block in chain    */
	int		seg_num;	/* segment number for this descripter*/
	int		pincount;	/* number of pages pinned in segment */
	struct xmem	xd;		/* cross-memory descripter	     */
};

#ifdef	_SUN
/* SUN -> 4.3 BSD difference */
#define uio_seg uio_segflg
#endif	/* SUN */

enum	uio_rw { UIO_READ, UIO_WRITE, UIO_READ_NO_MOVE, UIO_WRITE_NO_MOVE };

/*
 * Segment flag values 
 */
#define UIO_USERSPACE	USER_ADSPACE	/* user address space */
#define UIO_SYSSPACE	SYS_ADSPACE	/* system address space */
#define UIO_USERISPACE	USERI_ADSPACE	/* user instruction address space */
#define UIO_XMEM	3		/* using cross mem descriptors	*/
#define IOV_MAX		16		/* Maximum number of iov's on readv() */
					/* or writev()			      */
					/* Also Defined in sys/limits.h	      */
#ifndef _KERNEL
#ifdef _NO_PROTO
extern ssize_t	readv();
extern ssize_t	writev();
#else
extern ssize_t	readv(int, struct iovec *, int);
extern ssize_t 	writev(int, const struct iovec *, int);
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#ifdef _KERNEL
#ifndef _NO_PROTO

extern int	ureadc(
	int		c,
	struct uio	*uio);
/* arguments:
 *	int		c;		character to give to user
 *	struct uio	*uio;		user area description
 */

extern int	uwritec(
	struct uio	*uio);
/* arguments:
 *	struct uio	*uio;		user area description
 */

extern int	uiomove(
	caddr_t		cp,
	int		n,
	enum uio_rw	rw,
	struct uio	*uio);
/* arguments:
 *	caddr_t		cp;		address of kernel buffer
 *	int		n;		#bytes to transfer
 *	enum uio_rw	rw;		direction of xfer (read/write)
 *	struct uio	*uio;		user area description
 */

extern int	pinu(
	caddr_t		base,
	int		len,
	short		segflg);
/* arguments:
 *	caddr_t		base;		address of first byte to pin
 *	int		len;		#bytes to pin
 *	short		segflg;		flag that specifies where data is
 */

extern int	unpinu(
	caddr_t		base,
	int		len,
	short		segflg);
/* arguments:
 *	caddr_t		base;		address of first byte to unpin
 *	int		len;		#bytes to unpin
 *	short		segflg;		flag that specifies where data is
 */

extern int	xmempin(
	caddr_t		base,
	int		len,
	struct xmem	*xd);
/* arguments:
 *	caddr_t		base;		address of first byte to pin
 *	int		len;		#bytes to pin
 *	struct xmem	*xd;		cross memory descripter
 */

extern int	xmemunpin(
	caddr_t		base,
	int		len,
	struct xmem	*xd);
/* arguments:
 *	caddr_t		base;		address of first byte to unpin
 *	int		len;		#bytes to unpin
 *	struct xmem	*xd;		cross memory descripter
 */

#else

extern int	ureadc();
/* arguments:
 *	int		c;		character to give to user
 *	struct uio	*uio;		user area description
 */

extern int	uwritec();
/* arguments:
 *	struct uio	*uio;		user area description
 */

extern int	uiomove();
/* arguments:
 *	caddr_t		cp;		address of kernel buffer
 *	int		n;		#bytes to transfer
 *	enum uio_rw	rw;		direction of xfer (read/write)
 *	struct uio	*uio;		user area description
 */

extern int	pinu();
/* arguments:
 *	caddr_t		base;		address of first byte to pin
 *	int		len;		#bytes to pin
 *	short		segflg;		specifies where data to pin is
 */

extern int	unpinu();
/* arguments:
 *	caddr_t		base;		address of first byte to unpin
 *	int		len;		#bytes to unpin
 *	short		segflg;		specifies where data to unpin is
 */

extern int	xmempin();
/* arguments:
 *	caddr_t		base;		address of first byte to pin
 *	int		len;		#bytes to pin
 *	struct xmem	*xd;		cross memory descripter
 */

extern int	xmemunpin();
/* arguments:
 *	caddr_t		base;		address of first byte to unpin
 *	int		len;		#bytes to unpin
 *	struct xmem	*xd;		cross memory descripter
 */

#endif /* not _NO_PROTO */
#endif /* _KERNEL */

#endif /* _H_UIO_ */
