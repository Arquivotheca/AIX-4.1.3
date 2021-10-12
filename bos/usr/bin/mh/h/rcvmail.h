/* @(#)20	1.3  src/bos/usr/bin/mh/h/rcvmail.h, cmdmh, bos411, 9428A410j 8/3/92 14:47:44 */
/* 
 * COMPONENT_NAME: CMDMH rcvmail.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26  28  35 
 *
 */

/* rcvmail.h - rcvmail hook definitions */

#ifndef	MMDFMTS
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/types.h>
#include "../mts/smail.h"
#else	MMDFMTS
#include "../mts/mmdf/util.h"
#include "../mts/mmdf/mmdf.h"
#endif	MMDFMTS


#ifndef	MMDFI
#define	RCV_MOK	0
#define	RCV_MBX	1
#else	MMDFI
#define	RCV_MOK	RP_MOK
#define	RCV_MBX	RP_MECH
#endif	MMDFI


#ifdef	NRTC			/* sigh */
#undef	RCV_MOK
#undef	RCV_MBX

#define	RCV_MOK	RP_MOK
#define	RCV_MBX	RP_MECH
#endif	NRTC
