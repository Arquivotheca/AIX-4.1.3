static char sccsid[] = "@(#)27	1.3  src/bos/usr/lib/asw/mpqp/dmatest.c, ucodmpqp, bos411, 9428A410j 11/30/90 10:54:07";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS:	dma_test.  See descriptions below.
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
   ENTRY POINT:
	    The Diagnostic Vector function invokes routine dmatest which,
	    in turn, calls the specific test routines for:
		dma_test: Read/Write capability and boundary conditions
			  within a single Host System memory page

   INPUTS : p_cmd   -  Near pointer to the command block causing the call
            p_buf   -  Far pointer to the 4K buffer tied to this c. block

   RETURNS:   0     -  Test succeeded, all passes, all patterns
             !0     -  Test failed

   NOTES:   The caller, i.e. acmdvec, generates the appropriate RQE
            depending on the return value.  No EDRR data is written.

   EXTERNAL INTERFACES:
            The assembler routines start_bm_dma and WaitBM are called to setup
	    Contender and wait for completion from dma_test and quik_test 
	    diagnostic functions.

*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "intzero.h"
#include "iface.def"

typedef struct {
    unsigned char	_type;		/* Command type (0xEC) */
    unsigned char	_empty [15];	/* Reserved */
    unsigned long	_addr [8];	/* Host System addresses (8) */
    unsigned short	_lgt [8];	/* Host Buffer lengths (8) */
} t_dma_test;

bm_dma_ccb		write;		/* Write Host Memory command */
bm_dma_ccb		read;		/* Read Host Memory command */

dmatest( t_dma_test           *p_cmd,
         unsigned char far    *p_buf )
{
	register unsigned	i,j;		/* loop counters */
	unsigned long		sysaddr;	/* Host system address */
	char far		*p1;
	char far		*p_r;
	char far		*p_n;

	unsigned char far	*zbuf;		/* Zeroed storage area */
	unsigned char far	*nbuf;		/* Varied storage area */
	unsigned char far	*rbuf;		/* Read data block */

	/* Setup the local pointers to adapter buffer regions */
	zbuf = p_buf; nbuf = p_buf + 512; rbuf = p_buf + 1024;

	/* Initialize the "read" DMA channel descriptor.  Does not change */
	read.t_count   = 512;
	read.card_addr = rbuf;
	read.host_addr = p_cmd->_addr [0];
	read.cc_reg    = BDCCW_START | BDCCW_FROMHOST | BDCCW_HOSTINCR |
			 BDCCW_CARDINCR | BDCCW_MEMORY | BDCCW_TCINTR;
	if ( WaitBM ( start_bm_dma ( &read )) != 0 )
		return ( -1 );

	/* Setup the local copy of System Address and "write" channel CCW */
	sysaddr = p_cmd->_addr [0];
	write.cc_reg = BDCCW_START | BDCCW_HOSTINCR | BDCCW_TCINTR |
			BDCCW_CARDINCR | BDCCW_MEMORY;

	/* Setup the variable pattern buffer, always read, never written */
	for ( i = 0, p1 = nbuf; i < 512; i++ )
		*p1++ = i;

	/* Setup the zeroed pattern buffer, always read, never written */
	for ( i = 512, p1 = zbuf; i--; )
		*p1++ = 0;

	for ( i = 0; i <= 255; i++ )
	{
		/* Clear the target block (i.e. write all zeroes there) */
		write.card_addr = zbuf;
		write.host_addr = sysaddr;
		write.t_count   = 512;
		if ( WaitBM ( start_bm_dma ( &write )) != 0 )
			return ( -1 );

		/* Write the sequential data pattern, variable length */
		write.card_addr = nbuf + i;
		write.t_count   = 512 - ( i * 2 );
		write.host_addr = sysaddr + i;
		if ( WaitBM ( start_bm_dma( &write )) != 0 )
			return ( -1 );

		/* Read the entire target block, zeroes, sequential, zeroes */
		if ( WaitBM ( start_bm_dma ( &read )) != 0 )
			return ( -1 );

		/* Verify the read data with what we wrote */
		p_r = rbuf;
		p_n = nbuf + i;

		/* Check for "i" leading zeroes */
		for ( j = 0; j < i; j++ )
			if ( *p_r++ != 0 )
				return ( 0x08 );	/* dma test failed */

		/* Check for ( 512 - ( 2 * i ) ) pattern characters */
		for ( ; j < ( 512 - i ); j++ )
			if ( *p_r++ != *p_n++ )
				return ( 0x08 );	/* dma test failed */

		/* Check for "i" trailing zeroes */
		for ( ; j < 512; j++ )
			if ( *p_r++ != 0 )
				return ( 0x08 );	/* dma test failed */
		
	}
	/* Test succeeded if we get this far. */
	return ( 0 );
}
