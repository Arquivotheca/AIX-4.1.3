/* @(#)52 1.1 src/bos/kernel/sys/termiox.h, sysxtty, bos411, 9428A410j 10/15/93 11:58:00 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS :
 *
 * ORIGINS: 71, 83
 *
 */
/*
 * OSF/1 1.1
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_TERMIOX
#define _H_TERMIOX

#include <sys/ioctl.h>
/*
 * This is the SYSVR4 termiox interface
 */
#define NFF             5

struct termiox {
    unsigned short    x_hflag;        /* Hardware flow control modes */
    unsigned short    x_cflag;        /* Reserved for future use */
    unsigned short    x_rflag[NFF];   /* Reserved for future use */
    unsigned short    x_sflag;        /* Hardware open modes */
};

/*
 * For x_hflag
 */
#define RTSXOFF         0x00000001
#define CTSXON          0x00000002
#define DTRXOFF         0x00000004
#define CDXON           0x00000008

/*
 * For x_sflag
 */
#define DTR_OPEN        0x00000001
#define WT_OPEN         0x00000002
#define RI_OPEN         0x00000004

#define	TCGETX	_IOR('B', 1, struct termiox)
#define	TCSETX	_IOW('B', 2, struct termiox)
#define	TCSETXW	_IOW('B', 3, struct termiox)
#define	TCSETXF	_IOW('B', 4, struct termiox)


#endif /* _H_TERMIOX */
