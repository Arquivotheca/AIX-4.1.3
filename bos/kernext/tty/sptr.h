/* @(#)58 1.6 src/bos/kernext/tty/sptr.h, sysxtty, bos411, 9428A410j 7/4/94 17:18:35 */
/*
 * COMPONENT_NAME: SYSXTTY (header file for sptr).
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _SPTR_H
#define _SPTR_H

#include <sys/types.h>
#include <sys/str_tty.h>
#include <sys/lpio.h>
#include <sys/devinfo.h>
#include <sys/stream.h>

#define	SPTR_PRI	INTMAX

/* local data structure attached to one open stream with sptr pushed */

typedef	struct line		line_t;
typedef	struct sptr_config	sptr_config_t;

struct  line {
					/* pointer to the corresponding	      */
	sptr_config_t	*config_p;	/* sptr_config structure	      */
					/* pointer to a message block needed  */
	mblk_t		*sendff_mp;	/* in the str_sp_sendff routine	      */
					/* pointer to an M_DATA mess. with    */
	mblk_t		*write_mdata_mp; /* data that arrived from upstream   */
					/* pointer to an LPWRITE_REQ M_IOCTL  */
					/* message which have to be	      */
	mblk_t		*write_mioctl_mp; /* acknowledged later		      */
					/* pointer to an LPRGETA/LPRSETA      */
					/* M_IOCTL message, which have to be  */
	mblk_t		*mioctl_mp;	/* acknowledged at the read side      */
					/* pointer to data arrived from driver*/
	mblk_t		*input_data_mp; /* pointer to data arrived from driver*/
	int		ind;		/* indent level			      */
	int		col;		/* columns per line, including indent */
	int		line;		/* lines per page		      */
	int		v_timout;	/* number of seconds before timing out*/
	int		ihog;		/* input hog limit		      */
	int		ccc;		/* current character count	      */
	int		mcc;		/* max character count		      */
	int		mlc;		/* max line count		      */
	int		chars_sent;	/* Characters Sent Counter	      */
	unsigned int	sp_modes;	/* printer modes		      */
	unsigned long	flags;		/* state flags			      */
#define	SP_OPEN_READ		0x01	/* device open for reading	      */
#define	SP_OPEN_WRITE		0x02	/* device open for writing	      */
#define	SP_IODONE		0x04	/* last i/o done was not in PLOT mode */
#define	SP_DUMMYSEND		0x10	/* DO NOT send data downstream	      */
	unsigned int	write_status;	/* different status of writing	      */
#define	SP_WRITE_IN_PROCESS	0x01	/* sptr has sent data to the driver   */
					/* it is not possible to write because*/
#define	SP_WRITE_NOT_POSSIBLE	0x02	/* SP_CTS_OFF2 is set!!!!	      */
#define	SP_WRITE_TIMEDOUT	0x04	/* write timed out		      */
#define	SP_WRITE_FINISHED	0x08	/* write to hardware succeeded	      */
					/* an M_DATA message is queued at     */
#define	SP_WRITE_DATA_QUEUED	0x10	/* the write side		      */
					/* an M_DATA message of the write     */
					/* queue or pointed to by 	      */
#define	SP_WRITE_DATA_FLUSHED	0x20	/* write_mdata_mp has been flushed    */
					/* indicates which message type       */
	unsigned int	mp_stored_status; /*  is locally stored		      */
					/* M_DATA message with data to be     */
					/* written is locally stored and      */
#define	SP_OUTPUT_DATA_STORED	0x01	/* pointed to by write_mdata_mp	      */
					/* M_IOCTL message of type LPWRITE_REQ*/
					/* is locally stored and waits to be  */
#define	SP_IOCTL_WREQ_STORED	0x02	/* acknowledged			      */
					/* M_IOCTL message of type LPRSETA    */
					/* is locally stored and waits to be  */
#define	SP_IOCTL_LPRSETA_STORED	0x04	/* acknowledged			      */
					/* M_IOCTL message of type LPRGETA    */
					/* is locally stored and waits to be  */
#define	SP_IOCTL_LPRGETA_STORED	0x08	/* acknowledged			      */
#define	SP_IOCTL_TIOCMGET_STORED 0x10	/* TIOCMGET for CTS to be acked       */
					/* M_IOCTL message of type TCSETA     */
					/* is locally stored and waits to be  */
#define	SP_IOCTL_TCSETA_STORED	0x20	/* acknowledged			      */
					/* M_IOCTL message of type TCGETA     */
					/* is locally stored and waits to be  */
#define	SP_IOCTL_TCGETA_STORED	0x40	/* acknowledged			      */
	unsigned char	sp_timerset;	/* indicates write timer turned on    */
	int		sp_timerid;	/* ident. for the write timer request */
	unsigned char	sp_timeout;	/* write timeout has occurred	      */
	int		sp_ctsid;	/* ident. for the CTS timer request m */
	unsigned long	sp_cts;		/* status of CTS		      */
#define	SP_CTS_ON		0x01	/* CTS is on			      */
#define	SP_CTS_OFF1		0x02	/* CTS has changed from on to off     */
#define	SP_CTS_OFF2		0x04	/* CTS is off since at least 10 sec.  */
	int		wbid;		/* bufcall id on write side	      */
	int		rbid;		/* bufcall id on read side	      */
	int		wtid;		/* timeout id on write side	      */
	int		rtid;		/* timeout id on read side	      */
};

/* configuration structure per line/Stream attached to the sptr module	*/

struct sptr_config  {   
					/* pointer to next sptr_config struct.*/
	sptr_config_t	*next;		/* in linked list		      */
					/* pointer to the corresponding line  */
	line_t		*line_p;	/*  structure in use		      */
	dev_t		devno;		/* devno of printer device	      */
	unsigned int	modes;		/* printer modes		      */
					/* number of seconds before writing   */
	int		timout;		/* times out			      */
	int		ind;		/* indent level			      */
	int		col;		/* columns per line, including indent */
	int		line;		/* lines per page		      */
};

/* Miscellaneous Constants  */

#define	FF		0x0c		/* Form Feed			      */
 
#define	MODBLKSZ	 128		/* size of message block	      */

#define	SENDFFBLKSZ	1024		/* size of sendff message block	      */

#define	SPTR_IHIWAT	 512		/* input hog limit		      */
#define	SPTR_ILOWAT	 128		

					/* delay to wait that CTS changes from*/
#define	NO_CTS_DELAY	  10		/* off to on			      */

struct sptr_dds {
	enum dds_type  which_dds; /* type of DDS; value must be SPTR_DDS */
	dev_t          devno;     /* Device number */
	struct lprio   plpst;     /* Margin parameters used with LPRGET&LPRSET*/
	struct lptimer plptimer;  /* Timeout used with LPRGTOV and LPRSTOV    */
	struct lprmod  plpmod;    /* Printer mode used with LPRMODG&LPRMODS   */
};

/* begin : macros for formatting routines */

#define	ADDC(nmp, line_p, c)			\
	if (!addc(nmp, line_p, c)) {		\
		return 0;			\
	}

#define	SPTR_PRNFORMAT(q, nmp, c)		\
	if (!sptr_prnformat(q, nmp, c)) {	\
		return 0;			\
	}

/* end : macros for formatting routines */

#endif	/* _SPTR_H */
