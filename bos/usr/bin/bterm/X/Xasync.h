/* @(#)79	1.1  src/bos/usr/bin/bterm/X/Xasync.h, libbidi, bos411, 9428A410j 10/5/93 18:03:16 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: LockDisplay
 *		UnlockDisplay
 *
 *   ORIGINS:   16,27,40,42
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if defined(AIXV3) && defined(XASYNC)

#include <sys/signal.h>		/* needed for SIGIO */
#include <sys/ioctl.h>		/* needed for FIONREAD */
#ifdef IBMSHM
#include "shmtrans.h"		/* needed for SMT */
#endif

#ifndef NOFILE
#define NOFILE  (200)
#endif NOFILE

#define MAXSOCKS ( NOFILE - 1 )
#define MSKCNT ((MAXSOCKS + 31) / 32)   /* size of bit array */

#define SIGIO_MASK		(1 << (SIGIO - 1))
#define _XReadEvents(dis)	_XAsyncReadEvents(dis,1)

#define LockDisplay(dis) \
		if ( AIX_ASYNC != NULL ) { \
			(AIX_ASYNC->per_display[dis->fd].Busy)++; \
		}

#ifdef IBMSHM
#define UnlockDisplay(dis) \
		if (AIX_ASYNC != NULL) { \
			if (AIX_ASYNC->per_display[dis->fd].IOpending  && \
			AIX_ASYNC->per_display[dis->fd].Busy == 1 ){ \
				AIX_ASYNC->per_display[dis->fd].Busy = 0; \
				{ \
				int pend; \
					if (dis->shmTrans) {	\
					     ShmTransPtr shmTrans = (ShmTransPtr) dis->shmTrans;	\
					     if (shmTrans->rcvcnt) {	\
						raise(SIGIO);		\
					     } else {				\
						if (ioctl(dis->fd, FIONREAD, 	\
						(char *) &pend) >= 0 && pend > 0) \
							raise(SIGIO); 		\
					     }				\
					} else	{			\
					if (ioctl(dis->fd, FIONREAD, \
					(char *) &pend) >= 0 && pend > 0) \
						raise(SIGIO); 		\
					}				\
				} \
			/* order is important here */ \
			} else if ( AIX_ASYNC->per_display[dis->fd].Busy>1){ \
				AIX_ASYNC->per_display[dis->fd].Busy--; \
			} else { \
				AIX_ASYNC->per_display[dis->fd].Busy = 0; \
			} \
		}
#else

#define UnlockDisplay(dis) \
		if (AIX_ASYNC != NULL) { \
			if (AIX_ASYNC->per_display[dis->fd].IOpending  && \
			AIX_ASYNC->per_display[dis->fd].Busy == 1 ){ \
				AIX_ASYNC->per_display[dis->fd].Busy = 0; \
				{ \
				int pend; \
					if (ioctl(dis->fd, FIONREAD, \
					(char *) &pend) >= 0 && pend > 0) \
						raise(SIGIO); \
				} \
			/* order is important here */ \
			} else if ( AIX_ASYNC->per_display[dis->fd].Busy>1){ \
				AIX_ASYNC->per_display[dis->fd].Busy--; \
			} else { \
				AIX_ASYNC->per_display[dis->fd].Busy = 0; \
			} \
		}
#endif

#define LockMutex(mutex)
#define UnlockMutex(mutex)

#define AIX_DATA_BUFFER_SIZE    BUFSIZE

typedef struct _buffer_data {
  struct _buffer_data *previous;
  struct _buffer_data *next;
  int size;
  int offset;
  int count;
  char *data;
} buffer_data ;

typedef struct _write_buffer {
  buffer_data *data_head;
  buffer_data *data_tail;
} write_buffer;

typedef struct _aix_async_display {
    Display      *display;
    int          Busy;
    int          IOpending;
    int          (*InputHandler)();
    int          async_disabled;
    write_buffer *write_buffered;
} aix_async_display;

typedef struct _aix_async {
  int               sigio_installed;
  void              (*PreviousSignalHandler)();
  int               num_fds;
  aix_async_display per_display[MAXSOCKS];
} aix_async;

extern aix_async *AIX_ASYNC;
extern write_buffer *write_buffer_head;
extern write_buffer *write_buffer_tail;

extern write_buffer *write_buffer_ptr;
extern write_buffer *buffer_data_ptr;

extern write_buffer *tmp_write_buffer_ptr;
extern write_buffer *tmp_buffer_data_ptr;

#endif	/* XASYNC */
