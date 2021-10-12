/* @(#)65	1.10  src/bos/kernel/net/spl.h, sockinc, bos412, 9445C412a 10/28/94 09:53:23 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: spl0
 *		splhi
 *		splhigh
 *		splimp
 *		splnet
 *		splx
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * spl.h -	processor level macros for 4.3 style networking code
 */
#include "sys/intr.h"

#define	PL_BASE		INTBASE		/* allow all interrupts 	*/
#define	PL_IMP		INTCLASS2	/* block all net adapters	*/
#define	PL_NET		INTCLASS2	/* block out network int level	*/
#define	PL_HI		INTMAX		/* block out all interrupts	*/

#define	spl0()		i_enable(PL_BASE)
#define	splnet()	i_disable(PL_NET)
#define	splimp()	i_disable(PL_IMP)
#define	splhi()		i_disable(PL_HI)
#define	splhigh()	i_disable(PL_HI)
#define	splx(s)		i_enable(s)
