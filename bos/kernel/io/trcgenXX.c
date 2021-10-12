static char sccsid[] = "@(#)00  1.8  src/bos/kernel/io/trcgenXX.c, systrace, bos411, 9438B411a 9/21/94 06:46:13";
/*
 *   COMPONENT_NAME: SYSTRACE
 *
 *   FUNCTIONS: trcgen
 *		trcgenk
 *		trcgenkt
 *		trcgent
 *
 *   ORIGINS: 27 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *                                                                 
 */

#ifdef _POWER_MP
#include <sys/inline.h>
#endif

/* 
 * These function pointers are declared in trc_ptr.c 
 */
extern void (*_trcgenfp)();
extern void (*_trcgentfp)();
extern void (*_trcgenkfp)();
extern void (*_trcgenktfp)();

/*
 * Lock used to synchronize with the unloading of the driver
 * ( declared in trc_ptr.c )
 */
extern int initfp_lock;

/* 
 * trcgen(), trcgent(), trcgenk(),trcgenkt() are merely the interface 
 * using their peer function pointers 
 *	_ which are initialized to dummy function when 
 * 	the trace driver is not loaded in the kernel 
 *	- which are initialized to their real peer function by trcconfig()
 *	 when the trace driver is loaded into the kernel
 */
void trcgen(chan,hookword,d1,len,buf)
char *buf;
{
	if ( (fetch_and_add(&initfp_lock,1) & (1<<29) ) == 0 ) {
#ifdef _POWER_MP
		isync();
#endif
		_trcgenfp(chan,hookword,d1,len,buf);
	}
#ifdef _POWER_MP
	__iospace_sync();
#endif
	fetch_and_add(&initfp_lock,-1);
}

void trcgent(chan,hookword,d1,len,buf)
char *buf;
{
	if ( (fetch_and_add(&initfp_lock,1) & (1<<29) ) == 0 ) {
#ifdef _POWER_MP
		isync();
#endif
		_trcgentfp(chan,hookword,d1,len,buf);
	}
#ifdef _POWER_MP
	__iospace_sync();
#endif
	fetch_and_add(&initfp_lock,-1);
}

void trcgenk(chan,hookword,d1,len,buf)
char *buf;
{
	if ( (fetch_and_add(&initfp_lock,1) & (1<<29) ) == 0 ) {
#ifdef _POWER_MP
		isync();
#endif
		_trcgenkfp(chan,hookword,d1,len,buf);
	}
#ifdef _POWER_MP
	__iospace_sync();
#endif
	fetch_and_add(&initfp_lock,-1);
}

void trcgenkt(chan,hookword,d1,len,buf)
char *buf;
{
	if ( (fetch_and_add(&initfp_lock,1) & (1<<29) ) == 0 ) {
#ifdef _POWER_MP
		isync();
#endif
		_trcgenktfp(chan,hookword,d1,len,buf);
	}
#ifdef _POWER_MP
	__iospace_sync();
#endif
	fetch_and_add(&initfp_lock,-1);
}

