static char sccsid[] = "@(#)21	1.3  src/bos/usr/lib/asw/mpqp/acmdvec.c, ucodmpqp, bos411, 9428A410j 11/30/90 10:45:34";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: acmdvec - Vectors to Adapter Diagnostic Commands
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
   This file is invoked when the high nibble of the Command Type field
   in the command block has been deciphered as an adapter command, as
   opposed to a port command which would have gone into a port queue.
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"

typedef struct
{
unsigned char		_typ;		/* Command type (0x??) */
unsigned char		_unused [15];	/* Reserved */
unsigned long		_cardaddr;	/* Card address, base of test */
unsigned short		_cardplgt;	/* Card test paragraph length */
} t_mem_test;

typedef struct
{
unsigned char		_typ;		/* Command type (0x??) */
unsigned char		_port;		/* Port Number */
unsigned char		_unused [14];	/* Reserved */
unsigned char		_parm;		/* Test parameter */
} t_multi_test;

#define	N_VEC_LEV	6

extern int		FE_memtst ( t_mem_test *, char far *( * )() );
extern int		FE_multi ( t_multi_test *, int  ( * )() );

int cmdvec ( unsigned char  buf_no )
{
cmd_blk			*p_cmd;		/* -> to command block of the pair */
unsigned char huge	*p_buf;		/* -> to buffer of the cmd. pair */
register int		( *fnPtr )();	/* Pointer to diagnostic function */
unsigned char		major,minor;

extern int		( *( *fT [] ) )();

	p_cmd = &cmdblk [buf_no];	/* compiler scales automagically */

	p_buf = XMIT_BUFFER( buf_no );

	edrr_0 [0] = 0;			/* Set result length */
	major = 0xF & ( p_cmd->_type >> 4 );
	minor = p_cmd->_type & 0xF;

	if ( major == 0xF )			/* Pure ROS Diagnostic */
	{
		switch ( minor )
		{
		case 0x00 :
		case 0x01 : return ( FE_memtst ( (t_mem_test *)p_cmd,
					(char far *(*)())ad_cntl [minor] ) );
		case 0x04 :
		case 0x05 : return ( FE_multi ( (t_multi_test *)p_cmd,
						ad_cntl [minor] ) );
		}
	}
	else if ( p_cmd->_type == 0xE0 )	/* Extended RAM test */
	{
		return ( FE_memtst ( (t_mem_test *)p_cmd,
				(char far *(*)())ad_diag [minor] ) );
	}
	else if ( major == 0xD )		/* Setup/Status Command */
	{
		switch ( minor )
		{
		case 0x03 :
		case 0x04 :
		case 0x05 :
		case 0x06 : return ( FE_multi ( (t_multi_test *)p_cmd,
						ad_sest [minor] ) );
		}
	}

	major = ~( major | 0xF0 );
 	if ( ( fnPtr = *( fT [major] )[minor] ) != NULL_FN )
		return ( ( *fnPtr )( p_cmd, p_buf ) );
	else
		return ( -2 );
}

/*
   Call the cmdvec function to invoke the diagnostic function.  If a -2
   is returned, the parameter was out of range.  Otherwise, the value of
   the RQE.TYPE is based solely on the Zero/Nonzero return status of the
   diagnostic invoked by cmdvec.
*/

void acmdvec ( unsigned char  buf_no )
{
cmd_blk		*p_cmd;		/* -> to command block of the pair */
t_rqe		l_rqe;		/* Response Queue Element */
int		retval;

	p_cmd = &cmdblk [buf_no];	/* compiler scales automagically */

/* Make the RQE and call the interlocked add routine which will generate
   a host interrupt as required.  Before issuing the cmd, clear EDRR [0]. */

	retval = cmdvec ( buf_no );
	l_rqe.f.rtype    = ( retval ) ? RQE_CMD_NAK : RQE_CMD_ACK;
	l_rqe.f.port     = p_cmd->_port;
	l_rqe.f.status   = p_cmd->_type;
	l_rqe.f.sequence = (unsigned short)retval;
	queue_writel( &arqueue, l_rqe.val );
	host_intr( 0 );
	return;
}

/*
   The memory test have a different return convention which must be filtered
   here in the front end.  Memory test result -1 = success, anything else is
   considered failure.  The EDRR is set !0 count only on failure.
*/

FE_memtst ( t_mem_test		*p_cmd,
	    char far		*( *p_fn )() )
{
char far	*retval;

	retval = ( *p_fn )( p_cmd->_cardaddr, p_cmd->_cardplgt );
	if ( retval == (char far *)-1 )
		return ( 0 );			/* Success */
/*
   This moves a long to a halfword aligned address.  Naughty naughty.
*/
	*(char far **)&edrr_0 [2] = retval;
	edrr_0 [0] = 4;
	return ( -1 );
}

FE_multi ( t_multi_test		*p_cmd,
	   int			( *p_fn )() )
{
	return ( ( *p_fn )( p_cmd->_parm ) );
}

adreset ( cmd_blk	*p_cmd,
	  char far	*p_buf )
{
register unsigned char	*p_data = (unsigned char *)p_cmd;
int			i;

	for ( i = 16; i; i-- )
		if ( *p_data++ != 0xFF )
			return ( -1 );
	(void)AdapReset();
	p_cmd->_port = 0;
	return ( 0 );
}

typedef struct
{
	unsigned char		cmd_typ;	/* Command Type */
	unsigned char		filler[15];	/* Not Used */
	unsigned char		op_size;	/* 0 = Byte, 1 = Word */
	unsigned char		reserved;	/* Not used */
	unsigned short		io_addr;	/* I/O address to use */
	union
	{
		unsigned char	io_byte,waste;	/* Value to write and filler */
		unsigned short	io_word;	/* Value to write */
	} val;
} t_io_cmd;

int  io_write ( t_io_cmd		*p_cmd )
{
	if ( p_cmd->op_size )			/* word */
		out16( p_cmd->io_addr, p_cmd->val.io_word );
	else					/* byte */
		out08( p_cmd->io_addr, p_cmd->val.io_byte );
	return(0);
}

int  io_read ( t_io_cmd		*p_cmd )
{

	if ( p_cmd->op_size )			/* word */
	{
	unsigned short		s_temp;

		s_temp = in16( p_cmd->io_addr );
		edrr_0 [0] = 2;
		*( (unsigned short *)&edrr_0 [2] ) = s_temp;
	}
	else					/* byte */
	{
	unsigned char		c_temp;

		c_temp = in08( p_cmd->io_addr );
		edrr_0 [0] = 1;
		edrr_0 [2] = c_temp;
	}
	return(0);
}

FE_cpuregs ( t_multi_test	*p_cmd,
	     char far		*p_buf )
{
	return ( CpuRegs( &edrr [p_cmd->_port][0] ) );
}

FE_cputmr ( t_multi_test	*p_cmd,
	    char far		*p_buf )
{
register unsigned int	temp = p_cmd->_parm;

	if ( temp > 2 )
		return ( -2 );
	return ( CpuTmr( &edrr [p_cmd->_port][0], temp ) );
}

FE_cpudma ( t_multi_test	*p_cmd,
	    char far		*p_buf )
{
register unsigned int	temp = p_cmd->_parm;

	if ( temp > 1 )
		return ( -2 );
	return ( CpuDma( &edrr [p_cmd->_port][0], temp ) );
}

FE_segreg ( t_multi_test	*p_cmd,
	    char far		*p_buf )
{
	return ( SegReg( &edrr [p_cmd->_port][0], p_cmd->_parm ) );
}

FE_sccregs ( t_multi_test	*p_cmd,
	     char far		*p_buf )
{
register unsigned int	port = p_cmd->_port;

	return ( SccRegs( &edrr [port][0], pcb[port].scc_base ) );
}

FE_cioregs ( t_multi_test	*p_cmd,
	     char far		*p_buf )
{
register volatile t_pcb		*p_pcb = &pcb [p_cmd->_port];

return ( CioRegs( &edrr [p_cmd->_port][0], p_pcb->cio_base, p_pcb->alt_cio ) );
}

FE_dmaregs( t_multi_test	*p_cmd,
	    char far		*p_buf )
{
	return (DmaRegs( &edrr [p_cmd->_port][0], p_cmd->_parm, p_cmd->_port) );
}

