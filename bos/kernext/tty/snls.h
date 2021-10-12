/* @(#)57 1.3 src/bos/kernext/tty/snls.h, sysxtty, bos41B, 9505A 1/20/95 08:30:48 */
/*
 * COMPONENT_NAME: (sysxtty) header file for snls
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _STR_NLS_H
#define _STR_NLS_H

#include <sys/str_tty.h>

#define MODBLKSZ 128				/* size of message block */

#define NLS_NULLPAT	0x02
#define NLS_WARNRULE	0x03
#define NLS_BADRULE	0x04
#define NLS_BADMAP	0x05

#define	NLS_PRI		INTMAX

#define	NLSMAP_MALLOC(p, c, s)	NET_MALLOC(p, c, s, M_STREAMS, M_NOWAIT)
#define	NLSMAP_FREE(p)		NET_FREE(p, M_STREAMS)

typedef struct ttmap *mapp_t;
typedef struct ttmapinfo *infop_t;

typedef struct nls {
	dev_t  devno;			/* for trace tty support		*/
	int    oldpri;			/* for critical section			*/
	int    wbid;			/* bufcall id on write side		*/
	int    rbid;			/* bufcall id on read side		*/
	int    wtid;			/* timeout id on write side		*/
	int    rtid;			/* timeout id on read side		*/ 
	mapp_t nlsmap;			/* for M_COPYIN/M_COPYOUT messages	*/
					/* working tty_map user structure	*/
	struct tty_map nls_tty_map;	/* copied into the kernel		*/
	struct ttmapinfo nls_mapin;	/* input map				*/
	struct ttmapinfo nls_mapout;	/* output map				*/
} STRNLS, *STRNLSP;

typedef struct iocblk	* IOCP;

typedef struct nls_dds {
	enum dds_type	which_dds;	/* type of DDS; value must be NLS_DDS */
};

/*
 * some macros for converting messages
 */
#define NLS_NACK(mp, err_num) \
        { \
        register struct iocblk *_iocp; \
        mp->b_datap->db_type = M_IOCNAK; \
        _iocp = (struct iocblk *)mp->b_rptr; \
        _iocp->ioc_error = err_num; \
        _iocp->ioc_rval = 0; \
        _iocp->ioc_count = 0; \
        if (mp->b_cont) { \
                freemsg(mp->b_cont); \
                mp->b_cont = 0; \
        } \
        }

#define NLS_ACK(mp, err_num) \
        { \
        register struct iocblk *_iocp; \
        mp->b_datap->db_type = M_IOCACK; \
        _iocp = (struct iocblk *)mp->b_rptr; \
        _iocp->ioc_error = err_num; \
        _iocp->ioc_rval = 0; \
        _iocp->ioc_count = 0; \
        if (mp->b_cont) { \
                freemsg(mp->b_cont); \
                mp->b_cont = 0; \
        } \
        }

#endif	/* _STR_NLS_H */
