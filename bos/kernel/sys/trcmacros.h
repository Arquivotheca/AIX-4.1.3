
/* @(#)32	1.12  src/bos/kernel/sys/trcmacros.h, cmdtrace, bos411, 9428A410j 5/13/91 15:02:59 */

#ifndef _H_TRCMACROS
#define _H_TRCMACROS

/*
 * COMPONENT_NAME: (SYSTRACE) system trace facility
 *
 * FUNCTIONS:  macros for trace hooks calls
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Correct usage of trace hook routines:
 *
 * Use the macros: TRCHKL0, TRCHKL0T, ..., TRCHKL5, TRCHKL5T
 *             or: TRCHK, TRCHKT, TRCHKL, TRCHKLT, TRCHKG, TRCHKGT
 *
 * For generic trace use the macros: TRCGEN, TRCGENT.
 *
 *
 * Hookword definition:
 *
 *  0        4        8       12       16       20       24       28       32
 * +--------+--------+--------+--------+--------+--------+--------+--------+
 * |          Hook ID         |  type  |             data field            |
 * +--------+--------+--------+--------+--------+--------+--------+--------+
 *
 * The Hook ID field is the three digit hex representation of the hook id.
 *
 * The type field is further defined as:
 *   high-order bit:  0 = no time stamp, 1 = time stamp
 *   low-order bits:  indicate the number of words to trace
 *		      (hookword and data words but not timestamp)
 *
 * The data field definition depends on the type of trace hook:
 *   for generic trace it is reserved and indicates the length of the
 *      variable buffer
 *   for all others it is available for 2 bytes of additional trace data
 */
#define HKID_MASK   0xFFF00000
#define HKTY_XMASK  0xFFF0FFFF
#define HKTY_TMASK  0x00080000
#define HKTY_Vr     0x0
#define HKTY_Sr     0x1
#define HKTY_Lr     0x2
#define HKTY_Gr     0x6
#define HKTY_VTr    0x8
#define HKTY_STr    0x9
#define HKTY_LTr    0xA
#define HKTY_GTr    0xE

#define HKTY_V      (HKTY_Vr  << 16)
#define HKTY_S      (HKTY_Sr  << 16)
#define HKTY_L      (HKTY_Lr  << 16)
#define HKTY_G      (HKTY_Gr  << 16)
#define HKTY_VT     (HKTY_VTr << 16)
#define HKTY_ST     (HKTY_STr << 16)
#define HKTY_LT     (HKTY_LTr << 16)
#define HKTY_GT     (HKTY_GTr << 16)
#define HKTY_LAST   (HKTY_GTr << 16)
#define HKWDTOMAJOR(hw)   (((hw) >> 24) & 0xFF)
#define HKWDTOHKID(hw)    (((hw) >> 20) & 0xFFF)
#define HKWDTOTYPE(hw)    (((hw) >> 16) & 0xF)
#define HKWDTOLEN(hw)     (((hw) >> 16) & 0x7)
#define HKWDTOWLEN(hw)    ((((hw) & 0xFFFF) + 3 ) / 4)
#define HKWDTOBLEN(hw)    ((hw) & 0xFFFF)
#define ISTIMESTAMPED(hw) ((hw) & 0x00080000)

/* Trace channels.
 * Channel 0 is the kernel trace channel and is used for all non-generic
 * trace hooks.
 * Calls to generic trace routines use channels 1-7 or, to include data
 * in the kernel trace, channel 0.
 */
#define TRC_NCHANNELS 8
extern char Trconflag[TRC_NCHANNELS];
#define TRC_ISON(chan) (Trconflag[(chan)])
#define _TRCTMASK(hw)	((hw) & HKTY_XMASK)

#ifdef _KERNEL

/* This is the common trace hook routine.
 */
extern void trchook();

/* Kernel trace hooks use the common trace hook routine, trchook.
 * The call is avoided if trace is off for channel 0.
 */
#define TRCHK(hw)	(TRC_ISON(0) ? \
			     (void) trchook(_TRCTMASK(hw)|HKTY_S) : \
			     (void) 0)
#define TRCHKT(hw)	(TRC_ISON(0) ? \
			     (void) trchook(_TRCTMASK(hw)|HKTY_ST) : \
			     (void) 0)
#define TRCHKL(hw,a)	(TRC_ISON(0) ? \
			     (void) trchook(_TRCTMASK(hw)|HKTY_L,a) : \
			     (void) 0)
#define TRCHKLT(hw,a)	(TRC_ISON(0) ? \
			     (void) trchook(_TRCTMASK(hw)|HKTY_LT,a) : \
			     (void) 0)
#define TRCHKG(hw,a,b,c,d,e) \
			(TRC_ISON(0) ? \
			     (void) trchook(_TRCTMASK(hw)|HKTY_G,a,b,c,d,e) : \
			     (void) 0)
#define TRCHKGT(hw,a,b,c,d,e) \
			(TRC_ISON(0) ? \
			     (void) trchook(_TRCTMASK(hw)|HKTY_GT,a,b,c,d,e) :\
			     (void) 0)

/* These are the generic trace hook kernel services.
 */
extern void trcgenk();
extern void trcgenkt();

/* Kernel generic trace hooks use the kernel services, trcgenk and trcgenkt.
 * The call is avoided if trace is off for the specified channel.
 */
#define TRCGEN(ch,hw,d1,len,buf)   (TRC_ISON(ch) ? \
				    (void) trcgenk(ch,hw,d1,len,buf) : \
				    (void) 0)

#define TRCGENT(ch,hw,d1,len,buf)  (TRC_ISON(ch) ? \
				    (void) trcgenkt(ch,hw,d1,len,buf) : \
				    (void) 0)

/* Redefine uses of the old macros to call the correct kernel service.
 */
#define TRCGENK(a,b,c,d,e)	TRCGEN(a,b,c,d,e)
#define TRCGENKT(a,b,c,d,e)	TRCGENT(a,b,c,d,e)

/*
 * Easy to use macros for hooks in system calls.
 * TRCHKT_SYSC(UNAMEX)
 *   expands to:
 * trchook(HKWD_SYSC_UNAMEX | HKTY_T)
 */
#define TRCHKT_SYSC(x)             TRCHKT(HKWD_SYSC_##x)
#define TRCHKLT_SYSC(x,a)          TRCHKLT(HKWD_SYSC_##x,a)
#define TRCHKGT_SYSC(x,a,b,c,d,e)  TRCHKGT(HKWD_SYSC_##x,a,b,c,d,e)

#else /* _KERNEL */

/* This is the fast-SVC trace hook routine.
 */
extern void utrchook();

/* User-level trace hooks use the fast-SVC trace hook routine, utrchook.
 * The call is avoided if trace is off for channel 0.
 */
#define TRCHK(hw)	(TRC_ISON(0) ? \
			     (void) utrchook(_TRCTMASK(hw)|HKTY_S) : \
			     (void) 0)
#define TRCHKT(hw)	(TRC_ISON(0) ? \
			     (void) utrchook(_TRCTMASK(hw)|HKTY_ST) : \
			     (void) 0)
#define TRCHKL(hw,a)	(TRC_ISON(0) ? \
			     (void) utrchook(_TRCTMASK(hw)|HKTY_L,a) : \
			     (void) 0)
#define TRCHKLT(hw,a)	(TRC_ISON(0) ? \
			     (void) utrchook(_TRCTMASK(hw)|HKTY_LT,a) : \
			     (void) 0)
#define TRCHKG(hw,a,b,c,d,e) \
			(TRC_ISON(0) ? \
			     (void) utrchook(_TRCTMASK(hw)|HKTY_G,a,b,c,d,e) : \
			     (void) 0)
#define TRCHKGT(hw,a,b,c,d,e) \
			(TRC_ISON(0) ? \
			     (void) utrchook(_TRCTMASK(hw)|HKTY_GT,a,b,c,d,e) :\
			     (void) 0)

/* These are the generic trace hook system calls.
 */
extern void trcgen();
extern void trcgent();
extern int trcon();
extern int trcoff();
extern int trcstart();
extern int trcstop();

/* User-level generic trace hooks use the system calls, trcgen and trcgent.
 * The call is avoided if trace is off for the specified channel.
 */
#define TRCGEN(ch,hw,d1,len,buf)   (TRC_ISON(ch) ? \
				    (void) trcgen(ch,hw,d1,len,buf) : \
				    (void) 0)
#define TRCGENT(ch,hw,d1,len,buf)  (TRC_ISON(ch) ? \
			 	    (void) trcgent(ch,hw,d1,len,buf) : \
				    (void) 0)

#endif /* _KERNEL */

/* These are the preferred macros.
 */
#define TRCHKL0(hw)                 TRCHK(hw)
#define TRCHKL1(hw,D1)              TRCHKL(hw,D1)
#define TRCHKL2(hw,D1,D2)           TRCHKG(hw,D1,D2,0,0,0)
#define TRCHKL3(hw,D1,D2,D3)        TRCHKG(hw,D1,D2,D3,0,0)
#define TRCHKL4(hw,D1,D2,D3,D4)     TRCHKG(hw,D1,D2,D3,D4,0)
#define TRCHKL5(hw,D1,D2,D3,D4,D5)  TRCHKG(hw,D1,D2,D3,D4,D5)

#define TRCHKL0T(hw)                TRCHKT(hw)
#define TRCHKL1T(hw,D1)             TRCHKLT(hw,D1)
#define TRCHKL2T(hw,D1,D2)          TRCHKGT(hw,D1,D2,0,0,0)
#define TRCHKL3T(hw,D1,D2,D3)       TRCHKGT(hw,D1,D2,D3,0,0)
#define TRCHKL4T(hw,D1,D2,D3,D4)    TRCHKGT(hw,D1,D2,D3,D4,0)
#define TRCHKL5T(hw,D1,D2,D3,D4,D5) TRCHKGT(hw,D1,D2,D3,D4,D5)

/* Redefine calls to the old interfaces to call the correct routine.
 */
#define trchk(hw)		TRCHK(hw)
#define trchkt(hw)		TRCHKT(hw)
#define trchkl(hw,a)		TRCHKL(hw,a)
#define trchklt(hw,a)		TRCHKLT(hw,a)
#define trchkg(hw,a,b,c,d,e)	TRCHKG(hw,a,b,c,d,e)
#define trchkgt(hw,a,b,c,d,e)	TRCHKGT(hw,a,b,c,d,e)

#endif /* _H_TRCMACROS */

