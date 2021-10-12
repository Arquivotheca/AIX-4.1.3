/* @(#)70	1.21  src/bos/kernel/sys/POWER/mpqpdiag.h, sysxmpqp, bos411, 9428A410j 7/17/90 18:28:56 */
#ifndef	_H_MPQPDIAG
#define	_H_MPQPDIAG

/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqpdiag.h - MPQP diagnostics header file
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


#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/types.h>
/**********************************************************************
 *    MPQP Diagnostic Status Defines				      *
 **********************************************************************/

#define	 PORT_STATUS	0x00
#define	 TEST_PASS	0x01
#define	 TEST_FAIL	0x02

/*
 * MPQP	Register I/O Read/Write	Ioctl Command Block
 */
typedef	struct io_cmd
{
    unsigned short	reg_off;	/* Register Offset */

    unsigned char	reg_val;	/* Register Value to be
					   Read/Written	   */
} t_io_cmd;

/*
 *  MPQP Read/Write Ioctl Command Block
 */

typedef	struct
{
	int		length;		/* length of transfer	*/

	char		*usr_buf;	/* address of user buffer */

	unsigned long	mem_off;	/* offset in adapter memory */
					/* where transfer will begin */
}t_rw_cmd;

/*
 *  MPQP User Command Block
 */

typedef	struct
{
	unsigned int	command;	/* Command type	*/

	char		*p_edrr;	/* address of EDRR */

	char		data[48];	/* user	data area */

}t_usr_cmd;

/*
 *  MPQP User Command Block Overlay for	Complex	Commands
 */

typedef	struct
{
	unsigned int	command;	/* Command type	*/

	char		*p_edrr;	/* address of EDRR */

	union
	{
	    struct
	    {
		char		data[48]; /* data area associated with	  */
					  /* this command.		*/
	    }d_ovl;
	   struct
	   {
		unsigned int	tst_addr;
		unsigned short	tst_length;
		unsigned char	fyller[42];
	    }c_ovl;
	}u_data_area;

}t_usr_cmd_ovl;

/*
 *  MPQP Read Ioctl Command Block
 */

typedef	struct
{
	unsigned int	command;	/* Command type	*/

	char		len;		/* length of read */

	char		*buf;		/* user	data area */

}t_read_ioc;

/*
 *  MPQP Write Ioctl Command Block
 */

typedef	struct
{
	unsigned int	command;	/* Command type	*/

	char		len;		/* length of read */

	char		data[48];	/* user	data area */

}t_write_ioc;

/*
 *  MPQP Get Status Ioctl Command Block
 */

typedef	struct GETSTAT
{
    unsigned long	type;		/* Status type */

    unsigned long	status;	       /* status 1 */

    unsigned long	ext_status;	/* extended status */

    char		*p_edrr;	/* pointer to EDRR */

}t_get_stat;

#endif /* _H_MPQPDIAG */
