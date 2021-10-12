/* @(#)60	1.20  src/bos/usr/include/termios.h, cmdtty, bos411, 9428A410j 3/29/94 23:57:27 */

/*
 * COMPONENT_NAME: (CMDTTY)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 9, 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_TERMIOS
#define _H_TERMIOS

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/*
 * POSIX requires that certain values be included in termios.h.  It also 
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 */

#ifdef _POSIX_SOURCE 

typedef unsigned int	tcflag_t;
typedef unsigned char	cc_t;
typedef unsigned int	speed_t;

#define	NCCS 16

/*
 * Ioctl control packet
 */
struct termios {
	tcflag_t	c_iflag;	/* input modes */
	tcflag_t	c_oflag;	/* output modes */
	tcflag_t	c_cflag;	/* control modes */
	tcflag_t	c_lflag;	/* line discipline modes */
	cc_t		c_cc[NCCS];	/* control chars */
};

#ifdef _NO_PROTO

extern int tcgetattr();
extern int tcsetattr();
extern int tcsendbreak();
extern int tcdrain();
extern int tcflush();
extern int tcflow();
extern speed_t cfgetospeed();
extern speed_t cfgetispeed();
extern int cfsetospeed();
extern int cfsetispeed();

#ifdef _ALL_SOURCE
extern int setcsmap();
#endif

#else				/* use POSIX required prototypes */

extern int tcgetattr(int, struct termios *);
extern int tcsetattr(int, int, const struct termios *);
extern int tcsendbreak(int, int);
extern int tcdrain(int);
extern int tcflush(int, int);
extern int tcflow(int, int);
extern speed_t cfgetospeed(const struct termios *);
extern speed_t cfgetispeed(const struct termios *);
extern int cfsetospeed(struct termios *, speed_t);
extern int cfsetispeed(struct termios *, speed_t);
/* Read a terminal code set map file, generate a struct csmap and 	*/
/* install it for stdin.  The pathname of the file is usually composed 	*/
/* by forming a string with the csmap directory and the code set needed,*/
/* e.g.									*/
/*	n = sprintf(path, "%s%s", CSMAP_DIR, nl_langinfo(CODESET));	*/

#ifdef _ALL_SOURCE
extern int 			/* If successful, 0 is returned.  	*/
				/* Otherwise, -1 is returned.		*/
setcsmap(
	const char *path);	/* pathname of the code set map file 	*/
#endif

#endif /* _NO_PROTO */

#ifdef _ALL_SOURCE
#define CSMAP_DIR "/usr/lib/nls/csmap/"
#endif
 
/* mask name symbols for c_lflag */

/* values for optional_actions arguments to tcsetattr() */

#define TCSANOW		0
#define TCSADRAIN	1 
#define TCSAFLUSH	2

/* values for the queue_selector argument to tcflush() */

#define TCIFLUSH	0
#define TCOFLUSH	1
#define TCIOFLUSH	2

/* values for the action argument to tcflow() */

#define TCOOFF		0
#define TCOON		1
#define TCIOFF		2
#define TCION		3

/* control characters */
#define	VINTR	  0
#define	VQUIT	  1
#define	VERASE	  2
#define	VKILL	  3
#define	VEOF	  4
#define	VEOL	  5
#define VSTART    7
#define VSTOP	  8
#define VSUSP	  9
#define	VMIN      4
#define	VTIME     5
#ifdef _ALL_SOURCE
#define VEOL2     6
#define VDSUSP   10
#define VREPRINT 11
#define VDISCRD  12
#define VWERSE   13
#define VLNEXT   14
#define VSTRT   VSTART			/* 5.4 compatability */
#endif /* _ALL_SOURCE */

#define	B0	0x00000000
#define	B50	0x00000001
#define	B75	0x00000002
#define	B110	0x00000003
#define	B134	0x00000004
#define	B150	0x00000005
#define	B200	0x00000006
#define	B300	0x00000007
#define	B600	0x00000008
#define	B1200	0x00000009
#define	B1800	0x0000000a
#define	B2400	0x0000000b
#define	B4800	0x0000000c
#define	B9600	0x0000000d
#define	B19200	0x0000000e
#define	B38400	0x0000000f
#ifdef _ALL_SOURCE
#define EXTA    B19200
#define EXTB    B38400
#endif /* _ALL_SOURCE */

/* c_iflag bits */
#define	IGNBRK	0x00000001
#define	BRKINT	0x00000002
#define	IGNPAR	0x00000004
#define	PARMRK	0x00000008
#define	INPCK	0x00000010
#define	ISTRIP	0x00000020
#define	INLCR	0x00000040
#define	IGNCR	0x00000080
#define	ICRNL	0x00000100
#define	IXON	0x00000200
#define	IXOFF	0x00000400
#ifdef _XOPEN_SOURCE
#define IUCLC   0x00000800
#define IXANY   0x00001000
#ifdef _ALL_SOURCE
#define IMAXBEL 0x00010000
#endif /* _ALL_SOURCE */
#endif /* _XOPEN_SOURCE */

/* c_oflag bits */
#define	OPOST	0x00000001
#ifdef _XOPEN_SOURCE
#define	OLCUC	0x00000002
#define	ONLCR	0x00000004
#define	OCRNL	0x00000008
#define	ONOCR	0x00000010
#define	ONLRET	0x00000020
#define	OFILL	0x00000040
#define	OFDEL	0x00000080
#define	CRDLY	0x00000300
#define	CR0	0x00000000
#define	CR1	0x00000100
#define	CR2	0x00000200
#define	CR3	0x00000300
#define	TABDLY	0x00000c00
#define	TAB0	0x00000000
#define	TAB1	0x00000400
#define	TAB2	0x00000800
#define	TAB3	0x00000c00
#define	BSDLY	0x00001000
#define	BS0	0x00000000
#define	BS1	0x00001000
#define	FFDLY	0x00002000
#define	FF0	0x00000000
#define	FF1	0x00002000
#define	NLDLY	0x00004000
#define	NL0	0x00000000
#define	NL1	0x00004000
#define	VTDLY	0x00008000
#define	VT0	0x00000000
#define	VT1	0x00008000
#ifdef _ALL_SOURCE
#define DLY_MASK (NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY)
#define OXTABS  0x00040000      /* expand tabs to spaces added     	*/
                                /* on 08/05/92.                         */
#define ONOEOT  0x00080000      /* discard EOT's (^D) on output    	*/
                                /* added on 08/05/92.                   */
#endif /* _ALL_SO0URCE */
#endif /* _XOPEN_SOURCE */

/* c_cflag bits */
#ifdef _ALL_SOURCE
#define CBAUD  _CBAUD
#endif /* _ALL_SOURCE */
#define _CBAUD  0x0000000f
#define	CSIZE	0x00000030
#define	CS5	0x00000000
#define	CS6	0x00000010
#define	CS7	0x00000020
#define	CS8	0x00000030
#define	CSTOPB	0x00000040
#define	CREAD	0x00000080
#define	PARENB	0x00000100
#define	PARODD	0x00000200
#define	HUPCL	0x00000400
#define	CLOCAL	0x00000800
#define _CIBAUD 0x000f0000
#define _IBSHIFT 16
#ifdef _ALL_SOURCE
#define CIBAUD  _CIBAUD
#define IBSHIFT _IBSHIFT
#define PAREXT  0x00100000
#endif /* _ALL_SOURCE */

/* c_lflag bits */
#define	ISIG	0x00000001
#define	ICANON	0x00000002
#ifdef _XOPEN_SOURCE
#define XCASE   0x00000004
#endif /* _XOPEN_SOURCE */
#define	ECHO	0x00000008
#define	ECHOE	0x00000010
#define	ECHOK	0x00000020
#define	ECHONL	0x00000040
#define	NOFLSH	0x00000080
#define TOSTOP  0x00010000
#ifdef _ALL_SOURCE
#define ECHOCTL 0x00020000
#define ECHOPRT 0x00040000
#define ECHOKE  0x00080000
#define FLUSHO  0x00100000
#define ALTWERASE       0x00400000      /* use alternate WERASE    	*/
                                        /* algorithm, added ALTWERASE   */
                                        /* on 08/05/92.                 */
#define PENDIN  0x20000000
#endif /* ALL_SOURCE */
#define IEXTEN  0x00200000
 
#endif /* _POSIX_SOURCE */

#endif /* _H_TERMIOS */
