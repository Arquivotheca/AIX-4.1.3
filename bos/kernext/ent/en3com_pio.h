/* @(#)36  1.3  src/bos/kernext/ent/en3com_pio.h, sysxent, bos411, 9428A410j 11/10/93 11:10:42 */
/*
 * COMPONENT_NAME: sysxent --  High Performance Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_EN3COM_PIO
#define _H_EN3COM_PIO

#include <sys/ioacc.h>

/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, then   */
/*  exception code is ORed into a variable called pio_rc.  This variable  */
/*  must declared in the routine that will be using these macros.         */
/*------------------------------------------------------------------------*/

enum pio_func
{
	GETC, GETS, GETSR, GETL, GETLR, PUTSTR, GETSTR,
	PUTC, PUTS, PUTSR, PUTL, PUTLR
};

#define PIO_RETRY_COUNT		3

#define ENT_GETPOS(addr, c) \
{ \
	uchar pos_c; \
	uint pos_i; \
	 	    \
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) { \
		BUS_GETCX((char *)(addr), (char *)(c)); \
		BUS_GETCX((char *)(addr), &pos_c); \
		if (*(char *)(c) == pos_c) \
			break; \
	} \
	if (pos_i >= PIO_RETRY_COUNT) \
		pio_rc = TRUE; \
}
	
#define ENT_PUTPOS(addr, c) \
{ \
	uchar pos_c; \
	uint pos_i; \
		    \
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) { \
		BUS_PUTCX((char *)(addr), (char)(c)); \
		BUS_GETCX((char *)(addr), &pos_c); \
		if ((char)(c) == pos_c) \
			break; \
	} \
	if (pos_i >= PIO_RETRY_COUNT) \
		pio_rc = TRUE; \
}
	
#define ENT_GETCX(addr, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETCX((char *)(addr), (char *)(c))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				GETC, (int)(addr), (long)(c), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETSX(addr, s) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETSX((short *)(addr), (short *)(s))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				GETS, (int)(addr), (long)(s), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETSRX(addr, sr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETSRX((short *)(addr), (short *)(sr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				GETSR, (int)(addr), (long)(sr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETLX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETLX((long *)(addr), (long *)(lr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				GETL, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETLRX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETLRX((long *)(addr), (long *)(lr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				GETLR, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTCX(addr, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTCX((char *)(addr), (char)(c))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				PUTC, (int)(addr), (long)(c), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTSX(addr, sr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTSX((short *)(addr), (short)(sr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				PUTS, (int)(addr), (long)(sr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTSRX(addr, sr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTSRX((short *)(addr), (short)(sr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				PUTSR, (int)(addr), (long)(sr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTLX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTLX((long *)(addr), (long)(lr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				PUTL, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTLRX(addr, lr) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTLRX((long *)(addr), (long)(lr))) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				PUTLR, (int)(addr), (long)(lr), 0); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_PUTSTRX(addr, src, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_PUTSTRX((char *)(addr), (char *)(src), (int)c)) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
				PUTSTR, (int)(addr), (long)(src), (int)c); \
		pio_rc |= excpt_code; \
	} \
}
#define ENT_GETSTRX(addr, src, c) \
{ \
	int excpt_code; \
			\
	if (excpt_code = BUS_GETSTRX((char *)(addr), (char *)(src), (int)c)) { \
		excpt_code = en3com_pio_retry(p_dev_ctl, excpt_code, \
					   GETSTR, (int)(addr), (long)(src), \
					   (int) c); \
		pio_rc |= excpt_code; \
	} \
}

#endif /* _H_EN3COM_PIO */
