static char sccsid[] = "@(#)07	1.41.1.10  src/bos/kernext/mpqp/mpqutil.c, sysxmpqp, bos41B, 412_41B_sync 12/18/94 13:04:23";
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 *  FUNCTIONS:	adap_q_readb, adap_q_readl, adap_q_peekb, adap_q_peekl,
 *		adap_q_writeb, adap_q_writel, bus_copyout, bus_copyin,
 *		que_command, issue_reset, reset_card, download_asw,
 *		reload_asw, PioGet, PioPut, PioBusCopy, PioXchgC,
 *		mlogerr, reset_ndelay, setup_ndlay_timer, start_ndlay_timer,
 *		free_ndlay_timer, sleep_timer, setup_sleep_timer,
 *		start_sleep_timer, free_sleep_timer
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/adspace.h>
#include <sys/types.h>
#include <sys/comio.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <errno.h>
#include <stddef.h>
#include <sys/except.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/mpqpdiag.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysdma.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/xmem.h>

extern t_acb		*acb_dir[];
extern t_mpqp_dds	*dds_dir[];
void   mlogerr();

/*======================================================================*/
/*                        PIO ACCESS PRIMITIVES                         */
/*======================================================================*/

/*
 *  All character, word, and long programmed-I/O (PIO) operations 
 *  to/from the adapter are handled here.  These primitives are not 
 *  typically called directly -- the PIO accessor macros in mpqpdd.h 
 *  resolve at compile time to calls to these routines.
 */

/*---------------------------  P I O G E T  ----------------------------*/
/*                                                                      */
/*  NAME: PioGet                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Performs a PIO read of a byte, short, reversed short, long, 	*/
/*	or reversed long depending on the type of access specified.	*/
/*	If an IO exception occurs during access, it is retried several	*/
/*	times -- a -1 is returned if the retry limit is exceeded.	*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*	-1	If pio error, value of the read otherwise.		*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx, 		*/
/*				BUS_GETC, BUS_GETS, BUS_GETL,		*/
/*				BUS_GETSR, BUS_GETLR.			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

PioGet (
	caddr_t		addr,			/* address to read from */
	int		type)			/* type of access */
{
        int             rc;
        label_t         jump_buf;               /* jump buffer */
        volatile int    retry = PIO_RETRY_COUNT;

        while (TRUE) {                          /* retry loop */
            /*                                                            */
            /*  Set up for context save by doing setjumpx.  If it returns */
            /*  zero, all is well; otherwise, an exception occurred.      */
            /*                                                            */
            if ((rc = setjmpx(&jump_buf)) == 0) {
                if (retry--) {                  /* retry? */
                    /*                                                   */
                    /*  If retries have not been used up, do the read.   */
		    /*  Select the accessor according to the access 	 */
		    /*  type.						 */
                    /*                                                   */
		    switch (type) {
			case C:			/* Character access */
			    rc = (int)BUS_GETC( addr );
			    break;
			case S:			/* Short access */
			    rc = (int)BUS_GETS( addr );
			    break;
			case SR:		/* Short-reversed access */
			    rc = (int)BUS_GETSR( addr );
			    break;
			case L:			/* Long access */
			    rc = (int)BUS_GETL( addr );
			    break;
			case LR:		/* Long-reversed access */
			    rc = (int)BUS_GETLR( addr );
			    break;
			default:		/* unsupported */
			    rc = -1;
			    break;
		    }
                    break;                      /* exit retry loop */
                } else {
                    /*                                                   */
		    /*  Out of retries, so return an error.		 */
                    /*                                                   */
                    rc = -1;
                    break;
                }
            } else {
                /*                                                       */
                /*  An exception has occurred or reoccurred -- if it is  */
                /*  a PIO error, simply retry; else, it is an exception  */
                /*  not handled here so longjmpx to the next handler on  */
                /*  the stack.                                           */
                /*                                                       */
                if (rc != EXCEPT_IO)
                    longjmpx(rc);
            }
        }
        /*  Out of retry loop -- remove jump buffer from exception       */
        /*  stack and return.                                            */
        /*                                                               */
        clrjmpx(&jump_buf);
        return (rc);
}

/*---------------------------  P I O P U T  ----------------------------*/
/*                                                                      */
/*  NAME: PioPut                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Performs a PIO write of a byte, short, reversed short, long, 	*/
/*	or reversed long depending on the type of access specified.	*/
/*	If an IO exception occurs during access, it is retried several	*/
/*	times -- a -1 is returned if the retry limit is exceeded.	*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*	0	Pio write was successful.				*/
/*	-1	Pio error or illegal access type.			*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx, 		*/
/*				BUS_PUTC, BUS_PUTS, BUS_PUTL,		*/
/*				BusPutSR, BusPutLR.			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

PioPut (
	caddr_t		addr,			/* address to write to */
	int		val,			/* value to write */
	int		type)			/* type of access */
{
        int             rc = 0;
        label_t         jump_buf;               /* jump buffer */
        volatile int    retry = PIO_RETRY_COUNT;

        while (TRUE) {                          /* retry loop */
            /*                                                            */
            /*  Set up for context save by doing setjumpx.  If it returns */
            /*  zero, all is well; otherwise, an exception occurred.      */
            /*                                                            */
            if ((rc = setjmpx(&jump_buf)) == 0) {
                if (retry--) {                  /* retry? */
                    /*                                                   */
                    /*  If retries have not been used up, do the write.  */
		    /*  Select the accessor according to the access 	 */
		    /*  type. If we make it out of the switch, we didn't */
		    /*  get a PIO error.				 */
                    /*                                                   */
		    switch (type) {
			case C:			/* Character access */
			    BUS_PUTC( addr, val );
			    break;
			case S:			/* Short access */
			    BUS_PUTS( addr, val );
			    break;
			case SR:		/* Short-reversed access */
			    BusPutSR( addr, val );
			    break;
			case L:			/* Long access */
			    BUS_PUTL( addr, val );
			    break;
			case LR:		/* Long-reversed access */
			    BusPutLR( addr, val );
			    break;
			default:		/* unsupported */
			    rc = -1;
			    break;
		    }
                    break;                      /* exit retry loop */
                } else {
                    /*                                                   */
		    /*  Out of retries, so return an error.		 */
                    /*                                                   */
                    rc = -1;
                    break;
                }
            } else {
                /*                                                       */
                /*  An exception has occurred or reoccurred -- if it is  */
                /*  a PIO error, simply retry; else, it is an exception  */
                /*  not handled here so longjmpx to the next handler on  */
                /*  the stack.                                           */
                /*                                                       */
                if (rc != EXCEPT_IO)
                    longjmpx(rc);
            }
        } 
        /*  Out of retry loop -- remove jump buffer from exception       */
        /*  stack and return.                                            */
        /*                                                               */
        clrjmpx(&jump_buf);
        return (rc);
}

/*------------------------  P I O B U S C O P Y  -----------------------*/
/*                                                                      */
/*  NAME: PioBusCopy                                                    */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Calls BusCpy with pio error retries.  Returns -1 if the		*/
/*      number of retries is exceeded.					*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*	0	Bus copy was successful.				*/
/*	-1	Pio error.                      			*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx, 		*/
/*				BusCpy					*/
/*                                                                      */
/*----------------------------------------------------------------------*/

PioBusCopy (
	caddr_t		dest,			/* destination address */
	caddr_t		src,			/* source address */
	int		len)			/* length of copy */
{
        int             rc = 0;
        label_t         jump_buf;               /* jump buffer */
        volatile int    retry = PIO_RETRY_COUNT;

        while (TRUE) {                          /* retry loop */
            /*                                                            */
            /*  Set up for context save by doing setjumpx.  If it returns */
            /*  zero, all is well; otherwise, an exception occurred.      */
            /*                                                            */
            if ((rc = setjmpx(&jump_buf)) == 0) {
                if (retry--) {                  /* retry? */
		    /*  						 */
		    /*  Call BusCpy with context saved, if we make it	 */
		    /*	to the break, we didn't get a pio error.	 */
                    /*                                                   */
		    BusCpy( dest, src, len );
                    break;                      /* exit retry loop */
                } else {
                    /*                                                   */
		    /*  Out of retries, so return an error.		 */
                    /*                                                   */
                    rc = -1;
                    break;
                }
            } else {
                /*                                                       */
                /*  An exception has occurred or reoccurred -- if it is  */
                /*  a PIO error, simply retry; else, it is an exception  */
                /*  not handled here so longjmpx to the next handler on  */
                /*  the stack.                                           */
                /*                                                       */
                if (rc != EXCEPT_IO)
                    longjmpx(rc);
            }
        } 
        /*  Out of retry loop -- remove jump buffer from exception       */
        /*  stack and return.                                            */
        /*                                                               */
        clrjmpx(&jump_buf);
        return (rc);
}

/*-------------------------  P I O X C H G C  --------------------------*/
/*                                                                      */
/*  NAME: PioXchgC                                                      */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Calls BusXchgC with pio error retries.  Returns -1 if the	*/
/*      number of retries is exceeded.					*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*	0	Bus character exchange was successful.			*/
/*	-1	Pio error.                      			*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx, 		*/
/*				BusXchgC				*/
/*                                                                      */
/*----------------------------------------------------------------------*/

PioXchgC (
	caddr_t		addr,			/* Bus address */
	unsigned char	val)			/* value to exchange */
{
        int             rc = 0;
        label_t         jump_buf;               /* jump buffer */
        volatile int    retry = PIO_RETRY_COUNT;

        while (TRUE) {                          /* retry loop */
            /*                                                            */
            /*  Set up for context save by doing setjumpx.  If it returns */
            /*  zero, all is well; otherwise, an exception occurred.      */
            /*                                                            */
            if ((rc = setjmpx(&jump_buf)) == 0) {
                if (retry--) {                  /* retry? */
		    /*  						 */
		    /*  Call BusXchgC with context saved, if we make it	 */
		    /*	to the break, we didn't get a pio error.	 */
                    /*                                                   */
		    rc = (int)BusXchgC( addr, val );
                    break;                      /* exit retry loop */
                } else {
                    /*                                                   */
		    /*  Out of retries, so return an error.		 */
                    /*                                                   */
                    rc = -1;
                    break;
                }
            } else {
                /*                                                       */
                /*  An exception has occurred or reoccurred -- if it is  */
                /*  a PIO error, simply retry; else, it is an exception  */
                /*  not handled here so longjmpx to the next handler on  */
                /*  the stack.                                           */
                /*                                                       */
                if (rc != EXCEPT_IO)
                    longjmpx(rc);
            }
        } 
        /*  Out of retry loop -- remove jump buffer from exception       */
        /*  stack and return.                                            */
        /*                                                               */
        clrjmpx(&jump_buf);
        return (rc);
}

/*======================================================================*/
/*                       ADAPTER MEMORY READ/WRITE                      */
/*======================================================================*/

/*
 *  All non-DMA transfers of data blocks to/from the adapter are
 *  handled through these bus-copy routines; these handle bus
 *  attachment, interrupt disabling, and crossing of adapter segments
 *  (cpu page windows).  The caller only needs to provide source and
 *  destination addresses with copy length.  If a hardware PIO error
 *  occurs, the exception is trapped and an error (-1) is returned.
 */

/*------------------------  B U S _ C O P Y  ---------------------------*/
/*                                                                      */
/*  NAME: bus_copyout, bus_copyin                                      	*/
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Copies from/to Rios memory to/from bus (adapter) memory.	*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Can be called from process or offlevel environment.		*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the adapter's memory at address dest.			*/
/*                                                                      */
/*  RETURNS:                                                            */
/*	-1	If the command failed or PIO error.			*/
/*	0	If the command succeeded.				*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, BUSIO_ATT,		*/
/*				PIO_GETC, PIO_PUTC, PioBusCopy,		*/
/*				i_enable.  				*/
/*                                                                      */
/*  ALGORITHM:								*/
/*	For each adapter memory window size (cpu page = segment 	*/
/*	size of 64K), copy up to the window size between Rios memory	*/
/*	and adapter memory.  For lengths crossing a window boundary,	*/
/*	adjust all pointers and lengths before copying from/to the 	*/
/*	next window.  On exit, return the cpu_page register to its	*/
/*	original contents and restore the interrupt level.		*/
/*	If any of the PIO primitives gets an exception, abort the	*/
/*	operation and return an error code (-1);			*/
/*									*/
/*----------------------------------------------------------------------*/

/* Bus Copy Flavor: */

# define BUS_COPY( p_acb, riosmem, adapmem, adapseg, length, to_bus )	\
									\
	unsigned long	segreg;						\
	register int	i_lev, tcount, offset, cpu_pg, err = FALSE;	\
									\
	i_lev  = i_disable( INTCLASS1 );				\
	segreg = BUSIO_ATT( p_acb->io_segreg_val, 0 );			\
	cpu_pg = PIO_GETC( p_acb->io_base | segreg + CPUPAGE );		\
	if ( cpu_pg < 0 ) 						\
	{								\
	    BUSIO_DET( segreg );					\
	    i_enable( i_lev );						\
	    return(-1);							\
	}								\
	while ( length && ( !err ))					\
	{								\
	    offset = ( adapmem % WINDOW_SIZE );				\
	    tcount = MIN( length, WINDOW_SIZE - offset );		\
	    err = PIO_PUTC( p_acb->io_base | 				\
				segreg + CPUPAGE, adapseg );		\
	    if ( err ) break;						\
	    if ( to_bus )						\
		err = PIO_PUTSTR( p_acb->mem_base + 			\
				segreg + offset, riosmem, tcount );	\
	    else							\
		err = PIO_GETSTR( riosmem, p_acb->mem_base +		\
				segreg + offset, tcount );		\
									\
	    length  -= tcount;						\
	    riosmem += tcount;						\
	    adapmem += tcount;						\
	    adapseg++;							\
	}								\
	if ( !err )							\
	    err = PIO_PUTC( p_acb->io_base | 				\
				segreg + CPUPAGE, cpu_pg );		\
	BUSIO_DET( segreg );						\
	i_enable( i_lev );						\
	return( err )


/* Bus Copy Instances: */

bus_copyout (
	t_acb		*p_acb,
	caddr_t		src_ptr,
	unsigned char	dest_seg,
	unsigned long	dest_addr,
	int		length )
{
	BUS_COPY( p_acb, src_ptr, dest_addr, dest_seg, length, TRUE );
}

bus_copyin (
	t_acb		*p_acb,
	unsigned char	src_seg,
	unsigned long	src_addr,
	caddr_t		dest_ptr,
	int		length )
{
	BUS_COPY( p_acb, dest_ptr, src_addr, src_seg, length, FALSE );
}

/*======================================================================*/
/*                       ADAPTER QUEUE ACCESSORS                        */
/*======================================================================*/

/*----------------------  A D A P _ Q _ R E A D  -----------------------*/
/*                                                                      */
/*  NAME: adap_q_readb, adap_q_readl        				*/
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Reads from the queue at q_address on the adapter specified	*/
/*	by the adapter control block pointed to by p_acb.  The value	*/
/*	of the next element to read is returned; the queue is updated	*/
/*	to point to the next read element for read routines, for peek	*/
/*	routines, the queue is not modified.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Can be called from process or offlevel environment.		*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the adapter's queue out pointer.				*/
/*                                                                      */
/*  RETURNS:                                                            */
/*	-1	If the command queue is empty or PIO error.		*/
/*	N	The element read from the queue.			*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  i_disable, i_enable, BUSIO_ATT, 	*/
/*				 PioXchgC, PIO_GETSTR, PIO_PUTC,	*/
/*				 PIO_GETLR, BUSIO_DET			*/
/*									*/
/*  ALGORITHM:								*/
/*	Disable interrupts, get a segment register for the attachment	*/
/*	to the I/O bus, then swap the contents of the CPU register	*/
/*	(on the adapter) to set it to the page containing the queue.	*/
/*	Read the header of the queue into local memory.  If the queue	*/
/*	is empty, return a -1, otherwise, increment the out pointer	*/
/*	to the next element to be read from the queue.  If this is	*/
/*	a "destructive" read (not-peek), update the queue with the	*/
/*	new out pointer.  On return, restore the CPUPAGE, detach from	*/
/*	the bus (return the segment register), then restore interrupts.	*/
/*                                                                      */
/*----------------------------------------------------------------------*/

/* Adapter Queue Read Flavor:	*/

# define ADAP_Q_READ( p_acb, q_addr, val, q_type )			\
									\
	unsigned char	out, page = p_acb->ds_base_page;		\
	unsigned long	segreg, i_lev;					\
	q_type		q_hdr;						\
									\
	i_lev  = i_disable( INTCLASS1 );				\
	segreg = BUSIO_ATT( p_acb->io_segreg_val, 0 );			\
	page   = (unsigned char)					\
		PioXchgC( p_acb->io_base | segreg + CPUPAGE, page );	\
	PIO_GETSTR( &q_hdr, q_addr |= segreg, sizeof( q_hdr ));		\
	if (Q_EMPTY( &q_hdr ))  					\
	    val = -1;							\
	else {								\
	    out = NEXT_OUT( &q_hdr );					\
	    PIO_PUTC( q_addr + offsetof( q_type, out ), out );		\
	    val = PIO_GETLR( q_addr + 					\
			offsetof( q_type, q_elem[ out ] ));		\
	}								\
	PIO_PUTC( p_acb->io_base | segreg + CPUPAGE, page );		\
	BUSIO_DET( segreg );						\
	i_enable( i_lev )


/* Adapter Queue Read Instances: */

char
adap_q_readb ( 
	t_acb		*p_acb,
	unsigned long	addr )
{
	register long	element;

	ADAP_Q_READ( p_acb, addr, element, byte_queue_t );
	return((char)element);
}

long
adap_q_readl ( 
	t_acb		*p_acb,
	unsigned long	addr )
{
	register long	element;

	ADAP_Q_READ( p_acb, addr, element, long_queue_t );
	return( element );
}

/*--------------------  A D A P _ Q _ W R I T E  -----------------------*/
/*                                                                      */
/*  NAME: adap_q_writeb, adap_q_writel         				*/
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Writes to the queue at q_address on the adapter specified	*/
/*	by the adapter control block pointed to by p_acb.  If the	*/
/*	queue is full, -1 is returned; if the queue was empty, 1 is	*/
/*	returned.							*/	
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Can be called from process or offlevel environment.		*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the adapter's queue in pointer and next write slot.	*/
/*                                                                      */
/*  RETURNS:                                                            */
/*	-1	If the command queue is full or PIO error.		*/
/*	 1	The queue was empty before the write.			*/
/*	 0	The queue was non-empty before the write.		*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  i_disable, i_enable, BUSIO_ATT, 	*/
/*				 PioXchgC, PIO_GETSTR, PIO_PUTC,	*/
/*				 PIO_PUTLR, BUSIO_DET			*/
/*									*/
/*  ALGORITHM:								*/
/*	Disable interrupts, get a segment register for the attachment	*/
/*	to the I/O bus, then swap the contents of the CPU register	*/
/*	(on the adapter) to set it to the page containing the queue.	*/
/*	Read the header of the queue into local memory.  If the queue	*/
/*	is full, return a -1, otherwise, save a flag if the queue is	*/
/*	empty.  Increment the in pointer to the next element and place	*/
/*	this value back into the queue.  Write the queue element 	*/
/*	using the appropriate accessor (PIO_PUTC for byte, PIO_PUTLR 	*/
/*	for long).  On exit, restore the CPUPAGE register contents,	*/
/*	detach from the bus (return the segment register), then		*/
/*	enable interrupts.						*/
/*                                                                      */
/*----------------------------------------------------------------------*/

/* Adapter Queue Write Flavor:	*/

# define ADAP_Q_WRITE( p_acb, q_addr, val, q_type, accessor )		\
									\
	unsigned char	in, page = p_acb->ds_base_page;			\
	register int	rc;						\
	unsigned long	segreg, i_lev;					\
	q_type		q_hdr;						\
									\
	i_lev  = i_disable( INTCLASS1 );				\
	segreg = BUSIO_ATT( p_acb->io_segreg_val, 0 );			\
	page   = (unsigned char)					\
		PioXchgC( p_acb->io_base | segreg + CPUPAGE, page );	\
	PIO_GETSTR( &q_hdr, q_addr |= segreg, sizeof( q_hdr ));		\
	if (Q_FULL( &q_hdr )) 						\
	    rc = -1;							\
	else {								\
	    rc = Q_EMPTY( &q_hdr );					\
	    in = NEXT_IN( &q_hdr );					\
	    accessor( q_addr + offsetof( q_type, q_elem[ in ] ),	\
		val );							\
	    PIO_PUTC( q_addr + offsetof( q_type, in ), in );		\
	}								\
	PIO_PUTC( p_acb->io_base | segreg + CPUPAGE, page );		\
	BUSIO_DET( segreg );						\
	i_enable( i_lev );						\
	return( rc )


/* Adapter Queue Write Instances: */

adap_q_writeb ( 
	t_acb		*p_acb,
	unsigned long	addr,
	unsigned char	byte )
{
	ADAP_Q_WRITE( p_acb, addr, byte, byte_queue_t, PIO_PUTC );
}

adap_q_writel ( 
	t_acb		*p_acb,
	unsigned long	addr,
	unsigned long	word )
{
	ADAP_Q_WRITE( p_acb, addr, word, byte_queue_t, PIO_PUTLR );
}


/*---------------------   Q U E _ C O M M A N D   ----------------------*/
/*                                                                      */
/*  NAME: que_command                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Issues a command to the adapter by placing it on the		*/
/*	adapter's command queue and interrupting the card.		*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Can be called from process or offlevel environment.		*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the adapter's command queue and free list.		*/
/*                                                                      */
/*  RETURNS:                                                            */
/*	-1	If the command queue is full or PIO error.		*/
/*	0	If the queue succeeded.					*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  adap_q_readb, adap_q_writeb, 		*/
/*				 i_disable, setup_sleep_timer, 		*/
/*			  	 i_enable, start_sleep_timer, e_sleep,	*/
/*				 free_sleep_timer, lockl, unlockl,	*/
/*				 BUSIO_ATT, PioXchgC, PioBusCopy,	*/
/*				 SET_ACMDREG, PIO_PUTC		        */
/*                                                                      */
/*----------------------------------------------------------------------*/

que_command (
	t_acb		*p_acb,
	unsigned char	*p_cmd,
	unsigned char	*p_cmd_data,
	short		data_len,
	int		ndelay )
{
	unsigned int	segreg;
	register int	rc, addr;
	register char	cmdno;
	unsigned char	page = p_acb->ds_base_page;
	t_mpqp_dds	*p_dds = p_acb->p_port_dds[0];

	MPQTRACE3( "qcmd", *p_cmd, ndelay );

	/* If this is not on offlevel (ndelay is off), then we are a	*/
	/* process competing for the resource (command queue); use	*/
	/* lockl to serialize access.					*/

        if (!ndelay)
	    lockl( &p_acb->cmd_queue_lock, LOCK_SHORT );

	/*  First, acquire a free command block by getting the next	*/
	/*  command number from the free queue.  If none is available,	*/
	/*  sleep until either timer pop or available cmd block.	*/

	while (( cmdno = adap_q_readb( p_acb, p_acb->p_txfree_q )) < 0 )
	{
	    if (!ndelay)
	    {
		setup_sleep_timer( p_acb );
		start_sleep_timer( p_acb );
#ifdef _POWER_MPQP
                e_sleep_thread( &p_acb->txfl_event_lst, &mpqp_intr_lock, EVENT_SHORT );
#else
                e_sleep( &p_acb->txfl_event_lst, EVENT_SHORT );
#endif /* _POWER_MPQP */
		free_sleep_timer( p_acb );
		if ( !p_acb->sleep_timer_pop )
		    continue;
	    }

	    if (!ndelay)
	       unlockl( &p_acb->cmd_queue_lock );
	    MPQTRACE3( "qcm1", *p_cmd, ndelay );
	    return( -1 );
	}
	/*  Copy the command to the adapter's command area; if an error	*/
	/*  is returned, then a PIO exception must have occurred -- 	*/
	/*  log an adapter error for the lowest port and abort.		*/

	if ( bus_copyout( p_acb, p_cmd, page,
			&p_acb->p_cmd_blk[ cmdno ] - p_acb->mem_base, 
					sizeof( t_adap_cmd )) < 0 )
	{
	    /* log error, unlock if ndelay, and re-enable interrupts */

	    mlogerr(ERRID_MPQP_ADPERR,DVC.port_num,__LINE__,__FILE__,0,0,p_dds);
	    if (!ndelay)
	    	unlockl( &p_acb->cmd_queue_lock );
	    MPQTRACE3( "qcm2", *p_cmd, ndelay );
	    return(-1);
	}
	/*  If additional command information is to be sent (dial	*/
	/*  data, for example), copy it to the associated transmit	*/
	/*  buffer (later, this will have to be changed for dynamic	*/
	/*  allocation/deallocation of adapter buffers).		*/

	if ( p_cmd_data )
	{
	    addr = ( cmdno * ADAP_BUF_SIZE ) + ADAP_TX_AREA;
	    if ( bus_copyout( p_acb, p_cmd_data, addr / WINDOW_SIZE, 
					addr % WINDOW_SIZE, data_len ) < 0)
	    {
	        /* log error, unlock if ndelay, and re-enable interupts */
	        mlogerr(ERRID_MPQP_ADPERR, DVC.port_num, 
					__LINE__, __FILE__, 0, 0, p_dds);
		if (!ndelay)
	       	    unlockl( &p_acb->cmd_queue_lock );
	        MPQTRACE3( "qcm3", *p_cmd, ndelay );
	        return(-1);
	    }
	}

	/*  Issue the command to the adapter by writing the command	*/
	/*  number to the adapter command queue.			*/

	if ( adap_q_writeb( p_acb, p_acb->p_adap_cmd_que, cmdno ) <  0 )
	{
	    mlogerr(ERRID_MPQP_ADPERR,DVC.port_num,__LINE__,__FILE__,0,0,p_dds);
	    rc = -1;
	} else {
	    segreg = BUSIO_ATT( p_acb->io_segreg_val, 0 );
	    page = (unsigned char)
		PioXchgC( p_acb->io_base | segreg + CPUPAGE, page );
	    SET_ACMDREG( p_acb, segreg, ACMD_ACQ );
	    PIO_PUTC( p_acb->io_base + segreg + PTRREG, INTCOM );
	    PIO_PUTC( p_acb->io_base | segreg + CPUPAGE, page );
	    BUSIO_DET( segreg );
	    rc = 0;
	}
	if (!ndelay)
	    unlockl( &p_acb->cmd_queue_lock );
	MPQTRACE2( "qcmx", cmdno );
	return( rc );
}

/*======================================================================*/
/*                       ADAPTER DOWNLOAD/RESET				*/
/*======================================================================*/

/*---------------------   I S S U E _ R E S E T   ----------------------*/
/*                                                                      */
/*  NAME: issue_reset                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Changes the adapter state to RESETTING and issues the reset	*/
/*	command to the adapter.  The adapter should respond with an	*/
/*	interrupt -- the interrupt handler changes the state to		*/
/*	RESET upon reception of this interrupt.  If the adapter does	*/
/*	not respond within the reset timeout period, an error is	*/
/*	returned.							*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This is a top-half routine in this driver and can only be       */
/*      executed under a user process.                                  */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Changes the adapter_state in the acb, clears the transmit	*/
/*	free list.							*/
/*                                                                      */
/*  RETURNS:                                                            */
/*	0	If the reset succeeded.					*/
/*	N	Positive error code if the reset failed.		*/
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  delay, SET_CPUPAGE, que_command,       */
/*				 que_stat, bzero.			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

issue_reset (
	t_acb	volatile	*p_acb,
	t_mpqp_dds		*p_dds,
	int			chan,
	unsigned long		bus_sr,
	unsigned long           sleep_flag)
{
	volatile t_chan_info	*p_tmp_chinfo; 
    	volatile unsigned int	old_pri;
	register int		i, ticks = 0;
	t_adap_cmd		cmdblk;
        int                     delay_seg;
        int                     tmp;   /* value read from delay reg (ignored) */
	int			rc;


	/* Clear the local command block, then fill the first 16 bytes	*/
	/* with the card reset command code:				*/

	bzero( ( char *)&cmdblk, sizeof(t_adap_cmd));
	for (i = 0; i < 16; i++)
	    *((caddr_t)((int)&cmdblk + i)) = STRT_CARD_RST;
	    
	/*  Set adapter state to "resetting", set up the memory for	*/
	/*  access to the base page, then queue the reset command to	*/
	/*  the adapter:						*/

#ifdef _POWER_MPQP
	MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#else
	old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
	p_acb->adapter_state = RESETTING;
#ifdef _POWER_MPQP
	MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
	i_enable(old_pri);
#endif /* _POWER_MPQP */
	SET_CPUPAGE( p_acb, bus_sr, p_acb->ds_base_page );
#ifdef _POWER_MPQP
	MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#else
        old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */
	if (rc = que_command( p_acb, &cmdblk, NULL, 0, sleep_flag ) < 0 )
        {
           MPQTRACE4("Res1", p_acb, cmdblk.cmd_typ, rc);
#ifdef _POWER_MPQP
	    MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
	    i_enable( old_pri );
#endif /* _POWER_MPQP */
           return(EIO);
        }


	/*  Wait for the interrupt back from the adapter; the interrupt	*/
	/*  handler will set the adapter state.  If nothing comes back	*/
	/*  after the reset timeout period, return an error.		*/

	/* If NDELAY is set then we must start a timer and check for	*/
        /* the time out value.  If nothing comes back after the reset 	*/
	/* timeout period, return an error.			   	*/

	p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[chan];
	if ( p_tmp_chinfo->devflag & DNDELAY )
	   {
	    /* Enable interrupts before setting timer */
#ifdef _POWER_MPQP
	    MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
	    i_enable( old_pri );
#endif /* _POWER_MPQP */
	    /* NDELAY is on, start the timer */
	    setup_ndlay_timer(p_dds);
	    start_ndlay_timer(p_dds);
	    return(0);
	    free_ndlay_timer(p_dds);
	   }
	else
	   {
	   /* NDELAY is not on, sleep */
	    while ( p_acb->adapter_state != RESET )
                  if ( ++ticks < ((RESET_TIMEOUT * HZ) * 10000) )
                     {
                        /*
                         * Cause process to sleep for 1 microsecond by accessing
                         * the DL_DELAY_REG hardware delay register (0xE0)
                         */

#ifdef _POWER_MPQP
			 MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
                         i_enable( old_pri ); 
#endif /* _POWER_MPQP */
                         delay_seg = (uint)IOCC_ATT(p_acb->iocc_segreg_val,
                           DL_DELAY_REG );
                         tmp = BUSIO_GETC(delay_seg);  /* delay 1 microsecond */
                         IOCC_DET(delay_seg); 
#ifdef _POWER_MPQP
			 MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#else
                         old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

                       } 
                  else
	               {
		         que_stat( p_dds, NULL, CIO_ASYNC_STATUS,
			  MP_ADAP_NOT_FUNC, NULL, NULL, NULL );
	
	    		/* Enable interrupts and return */
#ifdef _POWER_MPQP
			MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
	    		i_enable( old_pri );
#endif /* _POWER_MPQP */
	         	return( EIO );
	               }
	   }

	/*  Clear the free transmit free buffers:	*/
	
	bzero( ( char *)p_acb->p_lcl_txfree_buf_q, (4 + N_TXFREE) );
	/* Enable interrupts and return */
#ifdef _POWER_MPQP
	MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
	i_enable( old_pri );
#endif /* _POWER_MPQP */
	return(0);
}

/*
 * reset_card -
 */

unsigned long reset_card( t_acb volatile	*p_acb,
	     		  unsigned long		bus_sr,
			  unsigned long		iob,
			  unsigned long		memb )
{
int			c_i;		/* loop counter	*/
unsigned char		daemon;

union {
	unsigned char	e[4];
	unsigned long	erp;
      } init_err;

	PIO_PUTC( iob + PTRREG, CAD_EN );
	PIO_PUTC( iob + DREG , 0xFF );

	PIO_PUTC( iob + PTRREG, INITREG1 );
	daemon = PIO_GETC( iob + DREG );
	PIO_PUTC( iob + DREG , daemon & ~ROSREADY );

	daemon = PIO_GETC( iob + COMREG );
	PIO_PUTC( iob + COMREG, COM_RC );		/* Reset bit only */
	delay( HZ/10 );					/* wait .1 sec 	  */
	PIO_PUTC( iob + COMREG, daemon );

	for ( c_i = 25 ; c_i; c_i-- )
	{

		delay( HZ );				/* wait 1 sec */

		/* See if the adapter ROS indicates READY */
		PIO_PUTC( iob + PTRREG, INITREG1 );
		daemon = PIO_GETC( iob + DREG );
	        MPQTRACE3("Urc1", c_i, daemon);
		if ( daemon & ROSREADY )
			break;
	}

	/* Early adapter POST errors are reported via TASKREG */
	daemon = PIO_GETC( iob + TASKREG );
	if ( daemon != 0xFF )			/* FF = Normal */
	{
		init_err.e [0] = 0xFF;
		init_err.e [1] = daemon;
		MPQTRACE2("Urc2", daemon);
		return ( init_err.erp );
	}
	if ( !c_i )			/* ROSREADY timed out */
	{
	        MPQTRACE2("Urc3", c_i);
		return ( 0xFE040000 );
	}

	/* Confirm the board has Bus Master Capability */
	PIO_PUTC( iob + PTRREG, GAID );
	daemon = PIO_GETC( iob + DREG );

	/* MPQP Adapters with the latest Bus Master Gate Array must have */
        /* Data Parity enabled in POS5 by the device driver.  Before     */
        /* Contender 5, data parity checking was not working correctly.  */

	if ( daemon > GA_CNTNDR_4 )
	{
	volatile unsigned long	iocc_sr;
	volatile unsigned long	pos_ptr;
        unsigned long           bus_num; 
        unsigned long           slot; 


		MPQTRACE2("Urc4", daemon);
               /* get the slot number */
                bus_num = (((p_acb->iocc_segreg_val >> 20) & 0xff) - 0x20);
                slot = p_acb->slot_num - (bus_num * NUM_SLOTS ); 

		iocc_sr = IOCC_ATT( p_acb->iocc_segreg_val, 0 );
		pos_ptr = iocc_sr + ( slot << 16 ) + IO_IOCC;
		PIO_PUTC( pos_ptr + POS5, p_acb->pos5 | P5_PAREN );
		IOCC_DET( iocc_sr );
	}

        /* Synchronous (Austin) channel check mode must be selected      */
        /* by setting PCP2_SYNC_CHCK.  When PCP2_SYNC_CHCK is set, if a  */
        /* channel check occurs (usually caused by a bad card), a        */
        /* pulse will be generated on the I/O channel check line         */
        /* synchronously with the error.  If not set, channel check      */
        /* would be held low causing the microchannel to lock up.*/

	PIO_PUTC( iob + PTRREG, PCPAR2 );
	daemon  = PIO_GETC( iob + DREG );
	daemon |= PCP2_SYNC_CHCK | PCP2_EN_CHCK;
	PIO_PUTC( iob + DREG, daemon );

	/* View page zero */
	SET_CPUPAGE( p_acb, bus_sr, 0x00 );

	/* Check the adapter POST error reporting areas */
	init_err.e [0] = PIO_GETC( memb + IF_BLK + ERRLOG_PTR + 1 );
	init_err.e [1] = PIO_GETC( memb + IF_BLK + ERRLOG_PTR + 0 );
	init_err.e [2] = PIO_GETC( memb + IF_BLK + STATOFF + 0 );
	init_err.e [3] = PIO_GETC( memb + IF_BLK + STATOFF + 1 );
	
	MPQTRACE3("Urc5",init_err.e[0],init_err.e[1]);
	MPQTRACE3("Urc6",init_err.e[2],init_err.e[3]);

	return ( init_err.erp );
}

int download_asw( t_acb volatile	*p_acb,
		  unsigned long		bus_sr,
		  unsigned long		iob,
		  unsigned long		memb,
		  t_rw_cmd		*p_rld_cmd )
{
int			error = 0;	/* local error flag */
char	 		*asw_buf;	/* local source pointer */
struct xmem		*xmd;		/* pointer to cross mem desc */
unsigned long		tmp_zork;	/* for calculating seg:offset */
#ifdef _POWER_MPQP
int 			old_pri;
#endif /* _POWER_MPQP */


	/* allocate and pin a kernel memory buffer */

	asw_buf = xmalloc( p_rld_cmd->length, 2, pinned_heap );

	/* attach for cross memory services and copy user     */
	/* buffer contents into pinned kernel buffer..ouch    */

	xmd = xmalloc( sizeof(struct xmem), 2, pinned_heap );
	xmd->aspace_id = XMEM_INVAL;
	error =
	    xmattach(p_rld_cmd->usr_buf, p_rld_cmd->length, xmd, USER_ADSPACE);
	error = xmemin( p_rld_cmd->usr_buf, asw_buf, p_rld_cmd->length, xmd );
    
	/* here we zero out the divide by zero entry of the   */
	/* adapter's interrupt vector table.  when the adapter*/
	/* software is loaded and started, it will load the   */
	/* address of the shared data areas into this vector  */

	SET_CPUPAGE( p_acb, bus_sr, 0x00 );
	PIO_PUTL( memb, 0x00 );

	/* we now write the adapter software 			*/
	/* image into the shared memory area on the adapter.    */
#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

	bus_copyout( p_acb, asw_buf, p_rld_cmd->mem_off >> 16, 
				p_rld_cmd->mem_off, p_rld_cmd->length );
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */


	/* To start adapter software, we must set an interrupt */
	/* vector to its entry point and  cause that interrupt */

	SET_CPUPAGE( p_acb, bus_sr, 0x00 );
	tmp_zork = ( ( p_rld_cmd->mem_off & 0xffff0000 ) << 12 );
	tmp_zork = ( ( p_rld_cmd->mem_off & 0x0000ffff ) | tmp_zork );
	PIO_PUTLR( memb + 0x30, tmp_zork );	/* set INT0 vector */

	PIO_PUTC( iob + PTRREG, INTCOM );     /* Interrupt the adapter */

	/* we have to restore the addressing, detach cross memory */
	/* services, and free the buffer before  */
	/* our job is done, regardless of the return code value   */

	error = xmdetach( xmd );
	xmfree( asw_buf, pinned_heap );
	xmfree( xmd, pinned_heap );

	return ( error );
}

/*
 * reload_asw - this function is called only from within mpqioctl
 *		and will download the provided adapter software to
 *		the adapter, set up pointers in the acb and start
 *		the adapter software running.
 */

int reload_asw( t_acb volatile		*p_acb,
		t_mpqp_dds		*p_dds,
		int                     chan,
		t_rw_cmd		*p_rld_cmd,
		unsigned long		bus_sr,
		unsigned long		iob,
		unsigned long		memb,
		unsigned int            sleep_flag )

{
    int			c_i;		/* loop counter	*/
    register int	ticks;		/* clock tick counter */
    unsigned short	ds_base;	/* offset in adapter memory */
					/* to start of data structures*/
    unsigned long	ds_cur_addr;	/* work address of structures */
    unsigned char  	ds_page;	/* page number for data */
					/* structures on Typhoon */
    int			error = 0;	/* local error flag */
    int			port_num;	/* port number loop control */
    unsigned long	post_result;	/* results of POST */
    t_byte_queue	*tmp_byte_que;  /* temporary pointer for    */
					/* calculating offsets	    */
    t_word_queue	*tmp_word_que;	/* temporary pointer for    */
					/* calculating offsets      */
    unsigned long	tmp_zork;	/* All purpose temporary */
    int			asw_ON;		/* Adapter SW Running the card? */
    int                 i;		/* counter value for for loop   */
    t_mpqp_dds          *p_tmp_dds;     /* temp pointer to port dds	*/
    int			old_pri;
/****************************************************************************/

/* Begin reload adapter software code */
    MPQTRACE4("UasW", p_acb, p_dds, p_rld_cmd);
    MPQTRACE4("Uas1", bus_sr, iob, memb );

    if  /* only one port may be open */
      (p_acb->n_open_ports > 1)
    {
	return(EINVAL);
    }

    SET_CPUPAGE( p_acb, bus_sr, 0x00);

    /* check to determine whether card is already up and running */

    asw_ON = (((PIO_GETLR(memb)) & 0xFFFF0000) == 0x10000000) ? 1:0;

    MPQTRACE2("Uas2", asw_ON);
    if /* adapter software is running, reset it */
       ( asw_ON )
    {
	if (error = issue_reset( p_acb, p_dds, chan, bus_sr, sleep_flag )) {
	    MPQTRACE2("Uas3", error);
	    return(error);
        }
    }

    MPQTRACE3("Uasx", p_acb->asw_load_flag, p_acb->num_starts);
    if /* adapter loaded flag is set */
      ( p_acb->asw_load_flag >= 1)
    {    
	if /* any starts have occurred on this adapter */
	   ( p_acb->num_starts )
	{
	    free_recv( p_acb, p_dds );
	    p_acb->num_starts = 0;
	}
	MPQTRACE2("Uas4", port_num);
	for /* number of ports on board */
	   ( port_num = 0 ; port_num < NUM_PORTS ; port_num++ )
	{
	    if /* dds pointer exists, go free up the resources */
	       ( p_acb->p_port_dds[port_num] )
	    {
		free_xmit( p_acb, p_acb->p_port_dds[port_num] );
		(p_acb->p_port_dds[port_num])->dds_wrk.num_starts = 0;
	    }
	}


	/* set the pos register before reset the card */
	set_pos (p_acb);
	for ( c_i = 5; c_i; c_i-- )
	{
	    if /* POST passed */
	       ((post_result = reset_card( p_acb, bus_sr, iob, memb )) == 0)
	    {
	                MPQTRACE1("UasA");
			break;
	    }
			/* Log a reset error */
	}
	if ( !c_i )			/* ROS Tests failed... */
	{
	    /* first save the POST results */

    	    p_rld_cmd->mem_off = post_result;
	    return (ENXIO);
	}

	p_acb->asw_load_flag = 0;

    } /* end of if software already loaded */

    /* enable the adapter to cause system unit interrupts */

    PIO_PUTC(iob + COMREG, 0x10);
    MPQTRACE4("Uas6", iob, memb, p_rld_cmd);
    if ( download_asw( p_acb, bus_sr, iob, memb, p_rld_cmd ) )
    {
	MPQTRACE1("Uas7");
	/* Log an error */
	return ( ENXIO );
    }

    /* now that we have downloaded the software we can put the POST results */
    /* into the offset field of the user command block */
    MPQTRACE2("Uas8", post_result);
    p_rld_cmd->mem_off = post_result;

    /* At this point we will enter a loop querying adapter   */
    /* memory location 0 for a 32 bit , non-zero value which */
    /* will be the pointer to the beginning of the data area */
    /* for the adapter.  We must add the memory base and	 */
    /* offsets to get the base of the command block area and */
    /* save it in the Adapter Control Block			 */

    SET_CPUPAGE( p_acb, bus_sr, 0x00);

    ticks = 0;
    while (TRUE) {
	if (ds_base = PIO_GETSR( memb )) 
	{
	    ds_page = PIO_GETC( memb + 0x02 );
	    break;
	} else {
	    delay(1);
	    if (++ticks > (RESET_TIMEOUT * HZ)) 
	    {
#ifdef _POWER_MPQP
    		MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

		que_stat( p_dds, NULL, CIO_ASYNC_STATUS,
			  MP_ADAP_NOT_FUNC, NULL, NULL, NULL );
#ifdef _POWER_MPQP
    		MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

		return( EIO );
	    }
	}
    }

    MPQTRACE3("Uas9", ds_base, ds_page);

    /* Restart the adapter software by causing an INT0 interrupt. */
    PIO_PUTC( iob + PTRREG, INTCOM );

    /* here we switch pages based on the segment value we   */
    /* received from the adapter software in the divide by  */
    /* zero exception area of the adapter's IVT		    */

    p_acb->ds_base_page = ds_page;	/* store CPUPAGE   */
    					/* for future use  */

    SET_CPUPAGE( p_acb, bus_sr, p_acb->ds_base_page ); 

    /* set up bus memory address of first data structure */

    ds_cur_addr = memb + (unsigned int)ds_base;
    		
    /* mask off segment register value */

    ds_cur_addr &= 0x0fffffff;

    tmp_zork = ds_cur_addr + bus_sr;

    tmp_byte_que = (t_byte_queue *)ds_cur_addr;
    p_acb->p_txfree_q = tmp_byte_que; 		/* set TX Free */
    					    	/* pointer in  */
    					    	/* acb	   */

    tmp_byte_que = (t_byte_queue *)tmp_zork;

    ds_cur_addr +=
    	    4 + 3 + PIO_GETC(&tmp_byte_que->length);/* add size of */
    						/* TX Free queue */
					/* hdr + q len + 3 extra at end */

    tmp_zork = ds_cur_addr + bus_sr;

    tmp_byte_que = (t_byte_queue *)ds_cur_addr;

    p_acb->p_adap_cmd_que = tmp_byte_que;	/* set Adapter */
    					   	/* Command Queue */
    					   	/* pointer in acb */

    tmp_byte_que = (t_byte_queue *)tmp_zork;

    ds_cur_addr +=
    	4 + 4 + PIO_GETC(&tmp_byte_que->length); 	/* add size of */
    					 	/* Adapter Command Queue */ 
					/* hdr + q len + 4 extra at end */

    tmp_word_que = (t_word_queue *)ds_cur_addr;

    p_acb->p_adap_rsp_que=tmp_word_que; 	/* set Adapter */
    					   	/* Response Queue */
    					   	/* pointer in acb */

    tmp_zork = ds_cur_addr + bus_sr;

    tmp_word_que = (t_word_queue *)tmp_zork;

    ds_cur_addr +=
	4 + (PIO_GETC(&tmp_word_que->length) * 4);	/*add size*/
    				 		/* of Adapter Response */
    				 		/* Queue */ 

    p_acb->p_adap_cmd_reg = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of ACMDREG */

    p_acb->p_num_cmd = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of number */
						/* of adapter commands	   */

    p_acb->p_num_rcv_buf = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of number */
						/* of receive buffers 	   */

    p_acb->p_rcv_buf_siz = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of        */
						/* receive buffer size     */

    p_acb->p_rcv_buf_para_num = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of        */
						/* receive buffer paragraph */
						/* number		   */

    p_acb->p_num_xmit_buf = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of number */
						/* of transmit buffers 	   */

    p_acb->p_xmit_buf_siz = (unsigned short *)ds_cur_addr;

    ds_cur_addr += 2;				/* add in length of        */
						/* transmit buffer size     */

    p_acb->p_xmit_buf_para_num = (unsigned short *)ds_cur_addr;

    ds_cur_addr = ((ds_cur_addr + 63) >> 6); 	/* round up to */
    ds_cur_addr = ds_cur_addr << 6;    		/* next 64 byte */
    				     		/* boundary */

    for /* for number of EDRR pointers, set pointers */
       (c_i = 0; c_i <4 ; c_i++)
    {
	p_acb->p_edrr[c_i] = (unsigned char *)ds_cur_addr;

	ds_cur_addr += 64;
    }

    p_acb->p_adap_trc_data = (char *) ds_cur_addr;

    ds_cur_addr += 256;

    for /* for number of port trace log pointers, set pointers */
       (c_i = 0; c_i <4 ; c_i++)
    {
	p_acb->p_port_trc_data[c_i] = (unsigned char *)ds_cur_addr;

	ds_cur_addr += 256;
    }

    p_acb->p_cmd_blk = (t_adap_cmd *) ds_cur_addr;

    /* now we go get the most current new copy of the transmit */
    /* free buffer list (where the free command pair numbers are) */
    /* Also, rewind the adapter command queue "In" index. */

    SET_CPUPAGE( p_acb, bus_sr, 0x00);		 /* CPUPAGE reg = zero */
    p_acb->adap_cmd_que_in = 0;

    /* set adapter software is loaded flag */
    p_acb->asw_load_flag = 1;

    /* set adapter state in acb to initialization complete */
#ifdef _POWER_MPQP
    MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#else
    old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
    p_acb->adapter_state = INITIALIZED;
#ifdef _POWER_MPQP
    MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#else
    i_enable(old_pri);
#endif /* _POWER_MPQP */

    MPQTRACE3("UasA", p_acb->adapter_state, p_acb->asw_load_flag);
    for ( i = 0; i < NUM_PORTS; i++ ) {
	if (p_acb->p_port_dds[i] != NULL) {
		p_tmp_dds = p_acb->p_port_dds[i]; 
		/* if port already started, port is not open */
		MPQTRACE2("UasB", p_tmp_dds->dds_dvc.port_state);
		if ( p_tmp_dds->dds_dvc.port_state > OPEN )
			p_tmp_dds->dds_dvc.port_state = OPEN;
	}
    }
    MPQTRACE2("UasC", error);
    return(error);
}



 /*******************************************************************
 **
 **      Function Name:  mlogerr
 **
 **      Description:    log an error to the error logger
 **
 **      Inputs:         err_id  type of error ( see errids.h )
 **                      dev     minor dev_no
 **                      line    line of code in file
 **                      file    source code file where err occured
 **                      data1   piece 1 of detail data, 0 if n/a
 **                      data2   piece 2 of detail data, 0 if n/a
 **
 **      Outputs:        void
 **
 **      Externals       none
 **      Referenced
 **
 **      Externals       none
 **      Modified
 **
 *******************************************************************/

 void
 mlogerr (int err_id, unsigned int port_num, 
	  int line, char *file, uint data1, uint data2, t_mpqp_dds *p_dds)
 {
	 errmsg  	msglog;
	 char 		errbuf[300];

	 msglog.err.error_id = err_id;

	 if (port_num == ADAPTER_LCK) {
		 sprintf(msglog.err.resource_name, "%s :%d",
			 p_dds->dds_vpd.adpt_name, data1);
		 data1 = 0;
	 }
	 else
		 sprintf(msglog.err.resource_name, "%s", 
			 p_dds->dds_vpd.devname);

	 sprintf(errbuf, "line: %d file: %s", line, file);

	 strncpy(msglog.file, errbuf, (size_t)sizeof(msglog.file));

	 msglog.data1 = data1;
	 msglog.data2 = data2;

	 errsave(&msglog, (uint)sizeof(errmsg));
 }


/*--------------------  R E S E T _ N D E L A Y   ----------------------*/
/*  NAME:  reset_ndelay							*/
/* 									*/
/*  FUNCTION: 								*/
/* 	Does the work of the timers in mpqutil for resetting the 	*/
/*	adapter.							*/
/*									*/
/*  RETURNS:  Status of adapter reset.  One of the following:		*/
/*	      MP_ADAP_NOT_FUNC						*/
/*	      MP_RESET_CMPL						*/
/*									*/
/*  EXTERNAL FUNCTIONS CALLED:						*/
/* 	que_stat							*/
/*----------------------------------------------------------------------*/


int reset_ndelay (
 	t_acb   		*p_acb,
  	t_mpqp_dds		*p_dds)	

{
#ifdef _POWER_MPQP
    	 int			old_pri; 
#endif /* _POWER_MPQP */

	MPQTRACE1( "QrnE");
	if ( p_acb->adapter_state != RESET )
	   {
#ifdef _POWER_MPQP
    	    MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

	    que_stat(p_dds,NULL, CIO_ASYNC_STATUS, MP_ADAP_NOT_FUNC,
	             NULL, NULL, NULL);
#ifdef _POWER_MPQP
    	    MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
	    p_dds->dds_wrk.ndelay_timer_pop = 1;
	    return(EIO);
	   }

	if ( p_acb->adapter_state == RESET )
	   {
#ifdef _POWER_MPQP
    	    MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
	    que_stat(p_dds,NULL, CIO_ASYNC_STATUS, MP_RESET_CMPL,
	             NULL, NULL, NULL);
#ifdef _POWER_MPQP
    	    MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
	    p_dds->dds_wrk.ndelay_timer_pop = 1;
	   }
	MPQTRACE1( "QrnX");
}


/*-------------------  S E T U P _ N D L A Y _ T I M E R  --------------*/
/*                                                                      */
/*  NAME: setup_ndlay_timer                                             */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Allocates and initializes the timer.                            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed by the top half of the driver.                         */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the DDS timer variables.                               */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_alloc                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

 int setup_ndlay_timer (
  	t_mpqp_dds		*p_dds)	
{
	MPQTRACE1( "sntE");
        if ( p_dds->dds_wrk.ndelay_timer == NULL )
           {
            p_dds->dds_wrk.ndelay_timer = (struct trb *)talloc();
            p_dds->dds_wrk.ndelay_timer->flags &= ~(T_ABSOLUTE);
            p_dds->dds_wrk.ndelay_timer->func
                                = (void (*)())reset_ndelay;
            p_dds->dds_wrk.ndelay_timer->func_data
                                = (unsigned long)p_dds;
            p_dds->dds_wrk.ndelay_timer->ipri
                                = INTCLASS3;
            p_dds->dds_wrk.ndelay_timer = FALSE;
            p_dds->dds_wrk.ndelay_timer_pop = 0;
           }
	MPQTRACE1( "sntX");
}

/*-------------------  S T A R T _ N D L A Y  _ T I M E R  -------------*/
/*                                                                      */
/*  NAME: start_ndlay_timer                                             */
/*                                                                      */
/*  FUNCTION:                                                          	*/
/*      Initiates the ndelay timer.					*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed by the top half of the driver.                         */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the DDS timer variables.                               */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_start                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

 int start_ndlay_timer (
  	t_mpqp_dds		*p_dds)	
{
        /*  If the timer exists and is not already running, set up the  */
        /*  ndelay milliseconds wait, mark the timer as running, then   */
        /*  start it.                                                   */
        /*                                                              */

	    MPQTRACE1( "stnE");
            if ( p_dds->dds_wrk.ndelay_timer )
               {
                p_dds->dds_wrk.ndelay_timer->timeout.it_value.tv_sec
                   = RESET_TIMEOUT * 2;
                p_dds->dds_wrk.ndelay_timer  = TRUE;
                tstart(p_dds->dds_wrk.ndelay_timer);
               }
	MPQTRACE1( "stnX");
        return;
}

/*-------------------  F R E E _ N D L A Y  _ T I M E R  ----------------*/
/*                                                                       */
/*  NAME: free_ndlay_timer                                               */
/*                                                                       */
/*  FUNCTION:                                                            */
/*      Disables and releases the ndelay timer.                          */
/*                                                                       */
/*  EXECUTION ENVIRONMENT:                                               */
/*      Executed by the top half of the driver.                          */
/*                                                                       */
/*  DATA STRUCTURES:                                                     */
/*      Modifies the DDS ndelay timer variables ndelay_timer and         */
/*      ndelay_timer_on.                                                 */
/*                                                                       */
/*  RETURNS: Nothing                                                     */
/*                                                                       */
/*  EXTERNAL PROCEDURES CALLED:  t_stop, t_free                          */
/*                                                                       */
/*-----------------------------------------------------------------------*/

 int free_ndlay_timer (
  	t_mpqp_dds		*p_dds)	
{
        /*                                                              */
        /*  If the ndelay timer exists, free it (if it is running, stop */
        /*  it first).  Clear ndelay timer variables.                   */
        /*                                                              */

	MPQTRACE1( "QfnE");
        if ( p_dds->dds_wrk.ndelay_timer )
           {
#ifdef _POWER_MPQP
              while(tstop(p_dds->dds_wrk.ndelay_timer));
#else
              tstop(p_dds->dds_wrk.ndelay_timer);
#endif
              tfree(p_dds->dds_wrk.ndelay_timer);
           }
        p_dds->dds_wrk.ndelay_timer = NULL;
        p_dds->dds_wrk.ndelay_timer = FALSE;
	MPQTRACE1( "QfnX");
}

/*----------------------S L E E P _ T I M E R --------------------------*/
/*  NAME:  sleep_timer							*/
/* 									*/
/*  FUNCTION: 								*/
/* 	Does the work of the timers in mpqutil for sleep in 		*/
/*      que_command. 	 						*/
/*									*/
/*  RETURNS:  Status of adapter reset.  One of the following:		*/
/*	      MP_ADAP_NOT_FUNC						*/
/*	      MP_RESET_CMPL						*/
/*									*/
/*  EXTERNAL FUNCTIONS CALLED:						*/
/* 	que_stat							*/
/*----------------------------------------------------------------------*/


int sleep_timer (t_acb   		*p_acb)
 	

{
	MPQTRACE1( "QstE");

	if ( p_acb->sleep_timer = TRUE )
	   {
	    p_acb->sleep_timer = FALSE;
	    p_acb->sleep_timer_pop = 1; 
	    e_wakeup( &p_acb->txfl_event_lst);
	   }
	MPQTRACE1( "QstX");

}


/*-------------------  S E T U P _ S L E E P _ T I M E R  --------------*/
/*                                                                      */
/*  NAME: setup_sleep_timer                                             */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Allocates and initializes the timer.                            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed by the top half of the driver.                         */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the ACB timer variables.                               */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_alloc                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

 int setup_sleep_timer (t_acb		*p_acb)	
  	
{
	MPQTRACE1( "sstE");
        if ( p_acb->sleep_timer == NULL )
           {
            p_acb->sleep_timer = (struct trb *)talloc();
            p_acb->sleep_timer->flags &= ~(T_ABSOLUTE);
            p_acb->sleep_timer->func
                                = (void (*)())sleep_timer;
            p_acb->sleep_timer->func_data
                                = (unsigned long) p_acb;
            p_acb->sleep_timer->ipri
                                = INTCLASS3;
            p_acb->sleep_timer = FALSE;
	    p_acb->sleep_timer_pop = 0;
           }
	MPQTRACE1( "sstX");
}

/*-------------------  S T A R T _ S L E E P  _ T I M E R  -------------*/
/*                                                                      */
/*  NAME: start_sleep_timer                                             */
/*                                                                      */
/*  FUNCTION:                                                          	*/
/*      Initiates the sleep timer.					*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Executed by the top half of the driver.                         */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the ACB timer variables.                               */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  t_start                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

 int start_sleep_timer (t_acb		*p_acb)	
  	
{
        /*  If the timer exists and is not already running, set up the  */
        /*  ndelay milliseconds wait, mark the timer as running, then   */
        /*  start it.                                                   */
        /*                                                              */

	    MPQTRACE1( "SstE");
            if ( p_acb->sleep_timer == FALSE )
               {
                p_acb->sleep_timer->timeout.it_value.tv_sec
                   = HZ * 30;
                p_acb->sleep_timer  = TRUE;
                tstart(p_acb->sleep_timer);
               }
	MPQTRACE1( "SstX");
        return;
}


/*-------------------  F R E E _ S L E E P  _ T I M E R  ----------------*/
/*                                                                       */
/*  NAME: free_sleep_timer                                               */
/*                                                                       */
/*  FUNCTION:                                                            */
/*      Disables and releases the sleep timer.                           */
/*                                                                       */
/*  EXECUTION ENVIRONMENT:                                               */
/*      Executed by the top half of the driver.                          */
/*                                                                       */
/*  DATA STRUCTURES:                                                     */
/*      Modifies the ACB ndelay timer variables ndelay_timer and         */
/*      ndelay_timer_on.                                                 */
/*                                                                       */
/*  RETURNS: Nothing                                                     */
/*                                                                       */
/*  EXTERNAL PROCEDURES CALLED:  t_stop, t_free                          */
/*                                                                       */
/*-----------------------------------------------------------------------*/

 int free_sleep_timer (t_acb		*p_acb)	
  	
{
	int	old_pri;

        /*                                                              */
        /*  If the sleep timer exists, free it (if it is running, stop  */
        /*  it first).  Clear sleep timer variables.                    */
        /*                                                              */

	MPQTRACE1( "FstE");
        if ( p_acb->sleep_timer )
           {
#ifdef _POWER_MPQP
            while(tstop(p_acb->sleep_timer))
	    {
		MPQP_UNLOCK_ENABLE( INTBASE, &mpqp_intr_lock );
		MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
	    }
#else
	    tstop(p_acb->sleep_timer);
#endif /* _POWER_MPQP */
            tfree(p_acb->sleep_timer); 
           }
        p_acb->sleep_timer = NULL;
        p_acb->sleep_timer = FALSE;
	MPQTRACE1( "FstX");
}

/*------------------  I N C R E M E N T _ S T A T S  -------------------*/
/*                                                                      */
/*  NAME: increment_stats                                               */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Accumlates statistics counters which have a low order integer	*/
/*	and a high order integer.					*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	Executed from process environment.				*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:  Nothing                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

void increment_stats ( 
	unsigned int *lcnt, 
	unsigned int *mcnt,
	unsigned short length ) 

{
	if (ULONG_MAX - length <= *lcnt) 
 	{
		*mcnt = *mcnt + 1;
		*lcnt = ULONG_MAX - *lcnt;
		*lcnt = length - *lcnt;
	} else {
		*lcnt += length;
	}
	return;
}

