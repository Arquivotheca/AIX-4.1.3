static char sccsid[] = "@(#)82	1.46  src/bos/kernext/mpqp/mpqwrite.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:22:09";

/*--------------------------------------------------------------------------
*
*				 MPQWRITE.C
*
*  COMPONENT_NAME:  (MPQP) Multiprotocol Quad Port Device Driver.
*
*  FUNCTIONS:	mpqwrite
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1990
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/listmgr.h>
#include <sys/mbuf.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>


extern t_acb		*acb_dir[];	/* ACB directory */
extern t_mpqp_dds	*dds_dir[];	/* DDS directory */

/*******************************************************************
 *    External Function Declarations                               *
 *******************************************************************/

extern void increment_stats ( unsigned int *, unsigned int *, unsigned short );

/*******************************************************************
 *    Internal Declarations                                        *
 *******************************************************************/

/* accumulate transmit counters */

static void	acum_xmit_stats( t_mpqp_dds    *,
				 unsigned char,
				 unsigned short );

/*-------------------------  M P Q W R I T E  --------------------------*/
/*                                                                      */
/*  NAME: mpqwrite                                                      */
/*                                                                      */
/*  FUNCTION:                                                           */
/*  	Provides the write interface to the MPQP device for both	*/
/*	user level and kernel level transmits.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Mpqwrite is part of a loadable device driver which is 		*/
/*	reentrant and always pinned.  This routine cannot be called	*/
/*	from offlevel or interrupt level, it can only be called from	*/
/*	the process environment.					*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*	0	Write succeeded.					*/
/*	n	Error number for error that occurred.			*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: que_command, free_xmit_map, 		*/
/*	trc_xmit_data, que_command, acum_xmit_stats			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

mpqwrite (
	dev_t		devno,			/* device number */
	struct uio	*uiop,			/* user I/O structure */
	int		chan,			/* channel (always zero) */
	t_write_ext	*p_ext,			/* write extension */
	unsigned int	ignore )
{
	register int	i, length;		/* data length of write */
	register int	ndelay, cpulevel;	/* flag, interrupt priority */
	struct mbuf	*m, *n;			/* mbuf pointers */
	struct mbuf	*freem = NULL;		/* mbuf passed from kernel  */
						/* calling routine          */
	t_acb		*p_acb;			/* ptr to adap. ctrl block */
	t_mpqp_dds	*p_dds;			/* ptr to dev data structure */
	char		*p_data, *dma_addr;	/* ptr's to write data */
	t_write_ext	writext;		/* write extension */
	adap_cmd_t	adapcmd;		/* adapter command */
	unsigned long	cio_wrt_flg=0;		/* tmp COMIO flag:free      */
						/* mbuf?: CIO_NOFREE_MBUF=0,*/
						/* report xmit ACK?:        */
						/* CIO_ACK_TX_DONE=0	    */
	int		old_pri;

	DDHKWD5( HKWD_DD_MPQPDD, DD_ENTRY_WRITE, 0, devno, 0, chan, p_ext, 0 );
	MPQTRACE4( "WrtE", devno, uiop, p_ext);
#ifdef _POWER_MPQP
        MPQP_SIMPLE_LOCK( &mpqp_mp_lock );
        MPQP_LOCK_DISABLE(old_pri,&mpqp_intr_lock);
#endif /* _POWER_MPQP */

	ndelay = uiop->uio_fmode & DNDELAY;	/* ndelay flag? */

	if (( minor(devno) >= ( MAX_ADAPTERS * NUM_PORTS )) || chan )
	{
	    MPQTRACE3( "Wdev", devno, chan );
#ifdef _POWER_MPQP
            MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
            MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

	    return( ENXIO );			/* invalid devno or channel */
	}
	p_dds = dds_dir[ minor(devno) ];	/* Get DDS pointer */
	p_acb = acb_dir[ HDW.slot_num ];	/* Get ACB pointer */

	/* As a general rule, when XMITMAP is loaded;  m is the local mbuf   */
	/* allocated and loaded with data by MPQP,  freem in kernel mode is  */
	/* the mbuf passed from DLC kernel process (passed mbuf) and freem   */
	/* in user mode is NULL since a user process can't use mbufs.  The   */
	/* exception is when an mbuf is passed to MPQP in kernel mode and    */
	/* MPQP doesn't have to copy the mbuf;  m is the mbuf passed from    */
	/* the DLC kernel process.  In the exception case, care needs to be  */
	/* taken that m is not freed erroneously.  			     */
	/*								     */
	/* 1. Never free passed mbuf if EAGAIN is to be returned. 	     */
	/* 2. Free passed mbuf according writext.cio_write.flag if EAGAIN is */
	/*    not to be returned.					     */
	/* 3. Always free local mbuf.					     */
	/*--------------------------  USER MODE  -------------------------*/

	if ( uiop->uio_segflg == UIO_USERSPACE )
	{
	    MPQTRACE1( "Wusr" );
	    if ( p_ext )				/* get write ext */
	        copyin( p_ext, &writext, sizeof( t_write_ext ));
	    length = uiop->uio_resid;			/* get data length */
	    if ( !length || ( length > PAGESIZE ))	/* in bounds? */
	    {
		MPQTRACE2( "Wlen", length );
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
            	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return( EINVAL );			/* bad length! */
	    }

	    if ( length <= TX_SHORT_MAX )  /* case 1 */	/* fit in cmd blk? */
	    {						/* then use it: */
		p_data   = adapcmd.cs.data;		/* data goes here */
		dma_addr = NULL;			/* nothing to DMA */
		m = (struct mbuf *)NULL;		/* no mbuf to free */
	    } else {			   /* case 2 */	/* else, use mbuf: */
	        if (!( m = m_getclust
			( ndelay ? M_DONTWAIT : M_WAIT, MT_DATA )))
		{
		    MBUFTRACE1( "Wnom" );		/* no mbuf available */
#ifdef _POWER_MPQP
		    MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
            	    MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		    return( EAGAIN );
		}					
		p_data   = MTOD( m, char * );		/* data goes here */
		dma_addr = p_data;			/* DMA page */
		MBUFTRACE5( "Wusm", m, p_data, writext.cio_write.flag,
		    cio_wrt_flg );
	    }
							/* copy down data: */
	    if ( uiomove( p_data, length, UIO_WRITE, uiop ))
	    {						
	        MPQTRACE3( "Wmem", devno, length );
		if (m) {						
                    m_freem(m);				/* free local mbuf */
	    	}						
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
       	        MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
	        return( EFAULT );			/* bad uio address */
	    }

	    trc_xmit_data ( p_data, length );		/* trace xmit data */

	/*-------------------------  KERNEL MODE  ------------------------*/

	} else {
	    MPQTRACE1( "Wker" );
	    if ( p_ext )				/* get write ext */
	        bcopy( p_ext, &writext, sizeof( t_write_ext ));
	    m = (struct mbuf *)			
			(uiop->uio_iov->iov_base);	/* get passed mbuf */
	    if ( !M_INPAGE( m ))			/* crosses a page? */
	    {
		MBUFTRACE3( "Wpag", devno, m );
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
       	        MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return( EINVAL );			/* then can't do it */
	    }
	    for ( n = m, length = 0; n; n = n->m_next )	/* get data length */
		length += n->m_len;
	    if ( !length || ( length > PAGESIZE ))	/* in bounds? */
	    {
		MPQTRACE2( "Wlen", length );
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
       	        MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return( EINVAL );			/* bad length! */
	    }

	    if ( length <= TX_SHORT_MAX )  /* case 3 */	/* fit in cmd blk? */
	    {						/* then use it */
		m_copydata( m, 0, length, adapcmd.cs.data );
		freem = m;				/* passed mbuf    */
		dma_addr = NULL;			/* nothing to DMA */
		m = (struct mbuf *)NULL;		/* no local mbuf  */
		trc_xmit_data ( adapcmd.cs.data, length ); /* trace xmit data */
		MBUFTRACE4( "Wksm", freem, writext.cio_write.flag,
		    cio_wrt_flg );
	    } else {		                     /* else, use mbuf for DMA*/
	        if ( !M_HASCL( m ))	  /* case 4 */	/* no cluster? */
	        {					/* then get local mbuf*/
	            if (!( n = m_getclust( M_DONTWAIT, MT_DATA ))) 
		    {
		        MBUFTRACE1( "Wnom" );		/* no clust avail */
#ifdef _POWER_MPQP
			MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
       	        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		        return( EAGAIN );
		    }					/* copy data to it */
		    m_copydata( m, 0, length, MTOD( n, char * ));
		    freem = m;				/* passed mbuf */
		    m = n;				/* use new local mbuf */
		    MBUFTRACE2( "Wcls", freem );
		}
		else			  /* case 5 */
		{					/* use passed mbuf */
	    	    cio_wrt_flg = writext.cio_write.flag;
		}
	        dma_addr = MTOD( m, char * );		/* data starts here */
		trc_xmit_data ( dma_addr, length );	/* trace xmit data */
		MBUFTRACE5( "Wkem", m, dma_addr, writext.cio_write.flag,
		    cio_wrt_flg );
	    }
	}
	/*--------------------------------------------------------------*/

	/*  Find a free transmit map element: the efficiency of this	*/
	/*  code scales with increased load -- higher baud rates will	*/
	/*  result in minimal searching (not as many maps in use); 	*/ 
	/*  slower baud rates will use more maps and require more 	*/ 
	/*  searching.  This eliminates the need for slow and 		*/ 
	/*  cumbersome queue management of the transmit map table.	*/ 
	/*  Whether local or passed mbuf, send ACK according to 	*/ 
	/*  caller's instructions in cio_write.flag.			*/ 
	for ( i = 0; i < NUM_PORT_TCWS; i++ )
	{
	    if ( XMITMAP[ i ].map_free )		/* free element? */
	    {
		XMITMAP[ i ].map_free = FALSE;  	/* fill 'er up */	
		XMITMAP[ i ].write_id = writext.cio_write.write_id;
		XMITMAP[ i ].flags = cio_wrt_flg | 
		    		(writext.cio_write.flag & CIO_ACK_TX_DONE);
		XMITMAP[ i ].dma_addr = dma_addr;
		XMITMAP[ i ].m        = m;	
		break;
	    }
	}
	/* When returning EAGAIN, only free local mbuf, don't free passed */
	/* mbuf.							  */
	/*        (m && ( USERSPACE || freem) )			  	  */ 
	/* case 1 (F && (  T        ||   F  ) ) = F : no local mbuf   	  */
	/* case 2 (T && (  T        ||   F  ) ) = T : free local mbuf 	  */
	/* case 3 (F && (  F        ||   T  ) ) = F : no local mbuf   	  */
	/* case 4 (T && (  F        ||   T  ) ) = T : free local mbuf 	  */
	/* case 5 (T && (  F        ||   F  ) ) = F : no local mbuf   	  */
	if ( i == NUM_PORT_TCWS )		/* nothing free? */
	{
	  if ( m && ((uiop->uio_segflg == UIO_USERSPACE) || freem) ) 
	  {
	      MBUFTRACE2( "Wfrr", m );
	      m_freem( m );			/* free local mbuf */
	  }
	  MBUFTRACE1( "Wmap" ); 		/* no map avail */
#ifdef _POWER_MPQP
	  MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
       	  MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
	  return( EAGAIN );			/* try again later */
	}

	/*  If a DMA operation is required, prepare the mbuf page for	*/
	/*  DMA access by the adapter by d_mastering the mbuf to 	*/
	/*  a bus address.						*/

	if ( dma_addr )
	    d_master( p_acb->dma_channel_id, DMA_WRITE_ONLY,
		    dma_addr, length, M_XMEMD( m ), XMITMAP[ i ].bus_addr );

	/*  Build the command block for a transmit operation, if the	*/
	/*  data is in the command block (dma_addr NULL), then it is	*/
	/*  a transmit short, otherwise it is a transmit long.		*/

	adapcmd.type	  = dma_addr ? XMIT_LONG : XMIT_SHORT;
	adapcmd.port	  = DVC.port_num;
	adapcmd.sequence  = SWAPSHORT( i );
	adapcmd.length    = SWAPSHORT( length );
	adapcmd.host_addr = SWAPLONG( XMITMAP[ i ].bus_addr );
	adapcmd.flags     = SWAPSHORT( ADAP_TX_ACK );

	if ( writext.transparent )		/* transparent mode? */ 
	    adapcmd.flags |= SWAPSHORT( ADAP_TRANSP );

	/*  Issue the command block to the adapter; if an error is	*/
	/*  returned, remove the mbuf from DMA access, put the map	*/
	/*  back on the free queue and return an error.			*/
#ifndef _POWER_MPQP
	old_pri = i_disable(INTCLASS1);
#endif
	if ( que_command( p_acb, &adapcmd, NULL, 0, ndelay ) < 0 )
	{
	    MPQTRACE2( "Wcmd", i );
	    /*  If passed mbuf, don't free it when returning EAGAIN.    */
	    /*  (!freem && !USERSPACE) indicates passed mbuf is being   */
	    /*  handled and should not be freed.                        */

            if (!freem && uiop->uio_segflg != UIO_USERSPACE)
		XMITMAP[ i ].flags |= CIO_NOFREE_MBUF;
	    free_xmit_map( p_acb, p_dds, i );  /* free transmit map element */
#ifdef _POWER_MPQP
       	    MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
       	    MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#else
	    i_enable(old_pri);
#endif /* _POWER_MPQP */
	    return( EAGAIN );
	}		
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri,&mpqp_intr_lock);
#else
	i_enable(old_pri);
#endif /* _POWER_MPQP */

	/* Mbuf passed from kernel calling routine is not put in XMITMAP,*/
        /* so free it.							 */
	if ( freem && !( writext.cio_write.flag & CIO_NOFREE_MBUF ))
	{
	    MBUFTRACE2( "Wfre", freem );
	    m_freem( freem );			/* free passed mbuf */
	}

        acum_xmit_stats( p_dds, adapcmd.type, length );	/* update stats.*/

	MPQTRACE1( "WrtX" );
	DDHKWD5( HKWD_DD_MPQPDD, DD_EXIT_WRITE, 0, devno, 0, 0, 0, 0 );
#ifdef _POWER_MPQP
       	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
	return( 0 );
}

/*------------------  A C U M _ X M I T _ S T A T S  -------------------*/
/*                                                                      */
/*  NAME: acum_xmit_stats                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Accumlates transmit statistics on each write.			*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Executed from process environment.				*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

static void acum_xmit_stats (
			     t_mpqp_dds *p_dds, 
			     unsigned char  type,
			     unsigned short length )
{
   /* update frame count */
   increment_stats ( &DDS_STATS.tx_frame_lcnt, &DDS_STATS.tx_frame_mcnt, 1 );

   /* update byte count	*/
   increment_stats ( &DDS_STATS.tx_byte_lcnt, &DDS_STATS.tx_byte_mcnt, length );

   if (type == XMIT_SHORT) 		/* short write? */
   {
	DDS_STATS.tx_short++;
	DDS_STATS.tx_shortbytes += length;
   } else
	if ((type == XMIT_LONG) ||
	    (type == XMIT_GATHER))	/* long write */ 
	{
	    DDS_STATS.tx_dma++;
	    DDS_STATS.tx_dmabytes += length;
	}
   return;
}

/*------------------  T R C __ X M I T __ D A T A   --------------------*/
/*                                                                      */
/*  NAME: trc_xmit_data                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*  Tracing the contents of a transmit frame up to 28 bytes.            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*  Executed from process environment.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

trc_xmit_data (
        unsigned int     *p_data,
        	 int     length)
{
	unsigned short	*p_twobytes;		/* 2 bytes of data	*/

	if ( length <= 2 )			/* RR sequence from SDLC */
	{
	   p_twobytes = p_data;
	   MPQTRACE3( "Wdps", length, *p_twobytes );
	}
	else					/* 3-28 bytes of data	*/ 
	{
           MPQTRACE5( "Wdpl", length, *p_data++, *p_data++, *p_data++ );
           MPQTRACE5( "Wdpl", *p_data++, *p_data++, *p_data++, *p_data++ );
	}

        return;
}
