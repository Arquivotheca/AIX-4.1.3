static char sccsid[] = "@(#)32	1.3  src/bos/usr/lib/asw/mpqp/memtest.c, ucodmpqp, bos411, 9428A410j 11/30/90 11:02:06";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 	memtest - See Description below.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
   Test adapter memory regions.  Tests performed:
	Bit ripple, word access, Write/Read/Verify (odd parity)
	Zero ripple, word access, Write/Read/Verify (odd parity)
	0x5A5A, 0xA5A5, word access, Write/Read/Verify (even parity)
	0xFFFF, 0x0000, word access, Write/Read/Verify (even parity)

   RETURNS :  -1     -  Test succeeded, No EDRR data of significance
              [other]-  Test failed, [other] is the failing address

   EXTERNAL INTERFACES: None

   CODE CAVEATS:
   1)	This code must be made smart enough to move itself out of
	the test region if necessary.  It presently is not.
   2)	Since the "volatile" variable descriptor is used and not
 	implemented, the optimization must be checked, as it could
	completely remove the test read following the write.
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"

/* Note that Memvfy accepts a number of Unsigneds to test, not Chars */

extern unsigned far *MemVfy( unsigned huge *, unsigned, unsigned );
extern unsigned far *Log2Seg( unsigned long );
extern unsigned far *AddrInc2( unsigned far *, unsigned );


unsigned far *
memtest ( unsigned long		k,
          unsigned		plgt )
{
#define NUM_PAT	37

extern unsigned		patterns [];

register unsigned	i;		/* loop counter */
register unsigned	*p_pattern;	/* data pattern pointer */

unsigned unsigned	j;		/* Current pass length, <= F000h */

unsigned far		* volatile p_dst;	/* Destination pointer */
unsigned far		* volatile p_tmp;	/* Temporary, return value */

	p_dst = Log2Seg( k );

	/* Test Write/Read/Verify on all patterns in the test array. */
	/* Test length is paragraph length (cardplgt) * 8 for words. */
	for ( k = ( (unsigned long)plgt << 3 ); k; )
	{
		j = ( k > 0xF000 ) ? 0xF000 : k;
		p_pattern = &patterns [0];	/* fast access to pattern */
		for ( i = NUM_PAT; i; i-- )
		{
			if ( ( p_tmp = MemVfy ( p_dst, *p_pattern, j ) ) != NULL )
				return ( (unsigned far *)p_tmp );
			++p_pattern;
		}
		p_dst = AddrInc2( p_dst, j );
		k -= j;
	}
	return ( (unsigned far *)-1 );		/* Successful completion */
}
