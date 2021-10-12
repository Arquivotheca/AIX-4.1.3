/* @(#)55	1.1  src/bos/kernel/sys/aio.h, sysxaio, bos411, 9428A410j 5/28/91 01:31:38 */
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: aio.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_AIO

#define _H_AIO

#define _POSIX_ASYNCHRONOUS_IO

#include <sys/errno.h>
#include <sys/file.h>
#include <sys/signal.h>

typedef struct aiocb *aio_handle_t;

#define EINPROG EINPROGRESS

struct aiocb
{
	int		aio_whence;	/* lseek whence argument       */
	off_t		aio_offset;	/* lseek offset argument       */
	char	       *aio_buf;	/* Pointer to R/W buffer       */
	int		aio_return;	/* return code of i/o          */
	int		aio_errno;	/* Error number, or EINPROG    */
	size_t		aio_nbytes;	/* Number of bytes to r/w      */
	int		aio_reqprio;	/* Not used		       */
	struct sigevent	aio_event;	/* Not used		       */
	int		aio_flag;	/* Checked for AIO_SIGNAL only */
	struct file    *aio_fp;		/* File fp, used internally    */
	aio_handle_t	aio_handle;	/* Pointer back to self	       */
	int		aio_pad[4];	/* leave some extra space      */
};

struct liocb
{
	int		lio_opcode;	/* list member opcode	       */
	int		lio_fildes;	/* file descriptor	       */
	struct aiocb	lio_aiocb;	/* aiocb for request	       */
};

/*
 * bits in aio_flag field of aiocb struct
 */

#define AIO_EVENT	0x001		/* Event notification not implemented  */
#define AIO_SIGNAL	0x002

/*
 * return values from aio_cancel()
 */
#define AIO_CANCELED	0	/* all i/o requested was cancelled */
#define AIO_NOTCANCELED	1	/* some i/o could not be cancelled */
#define AIO_ALLDONE	2	/* all i/o was already completed   */

#define LIO_WAIT	1
#define LIO_ASYNC	2  /* LIO Event notification not implemented */
#define LIO_ASIG	3  /* temporary replacement until Event notification */
#define LIO_NOWAIT	0

#define LIO_READ 1
#define LIO_WRITE 2
#define LIO_NOP 0

/*
 * Macros
 *
 * No error checking.  The user is free to pass us nonsense,
 * and get nonsense back (or core dump).
 */
#define aio_return(h)	((h)->aio_return)
#define aio_error(h)	((h)->aio_errno)

/*
 * subroutine declarations
 */

#ifdef _NO_PROTO

int aio_read();
int aio_write();
int lio_listio();
int aio_cancel();
int aio_suspend();

#else

int aio_read(int filedes, struct aiocb *aiocbp);
int aio_write(int fildes, struct aiocb *aiocbp);
int lio_listio(int cmd, struct liocb *list[], int nent, struct sigevent *eventp);
int aio_cancel(int fildes, struct aiocb *aiocbp);
int aio_suspend(int cnt, struct aiocb *aiocbpa[]);

#endif /* _NO_PROTO */

#endif /* _H_AIO */
