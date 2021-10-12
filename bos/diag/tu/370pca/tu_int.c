static char sccsid[] = "@(#)tu_int.c	1.4 2/1/91 14:31:13";
#include <stdio.h>
#include "psca.h"
#include "pscatu.h"
#include "extern.h"
#include "catuser.h"
#include <sys/comio.h>

#ifdef MTEX
#include "pscadefs.h"
extern unsigned char far	*cardarea;
extern STATAREA far		*statarea;
extern BYTE			pscalvl;	/* interrupt level of adapter */
#endif

/*************************************************************************
* Confirm ability to interrupt the microchannel	at various levels	 *
* Requires that the diagnostic microcode has been loaded.		 *
* Requires that the special extender card be inserted on PCA card        *
*									 *
*									 *
* What this really does is:						 *
*       								 *
*	save  the pos5 register value 					 *
*       run the following steps for each of 8 different int levels:      *
*									 *
*         maps each card interrupt output to a fixed level               *
*         executes tu-003 in the microcode to pop the MCI interrupt	 *
*         check to see if the interrupt popped				 *
*         executes tu-003 twice in the microcode			 *
*         check to see that the interrupt counter = 2			 *
*									 *
*	restore the pos5 register value 				 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SYS		One of the ioctl's failed			 *
*	TU_HARD		The diagnostic microcode isn't ready		 *
*	TU_SOFT		The interrupt was not received			 *
*************************************************************************/
tu_intlvl(cb)    
TUTYPE				*cb;		/* control block for necessary info */
{
	int			rc;
	int			int_val;
	int			pos5_save;
	int			pos5_const;
	int			ints_exp;
	int			i;
/***********************************************************/
/* change these declarations to malloc                     */
	cio_query_blk_t		query_blk;
	cat_query_t		query_stats;
/***********************************************************/

	query_blk.bufptr= (caddr_t) &query_stats;          /* set up for CIO_QUERY ioctl */
	query_blk.buflen = sizeof(query_stats);

	read_pos_reg(5,&pos5_save);        /* pca card int level reg */
	pos5_const = pos5_save & 0xF8;     /* part of the register that doesn't control interrupts */
	
	for (i=0; i<8; i++)          		/* loop thru 8 possible int levels from ext card */
	{
		write_pos_reg(5, pos5_const | i);     /* pca card int level reg */
		write_pos_reg(7, 0x80 | (i<<4));      /* int level mapping reg on extender card */
		ints_exp = 0;
		
		query_blk.clearall = CIO_QUERY_CLEAR;            /* clear int count in stats blk */
		if (ioctl(device, CIO_QUERY, &query_blk) == -1)
			{
			sprintf(MSGBUF,"Reset Query Statistics failed.\n");
			return(TU_SYS);
			}
		if (query_blk.status) 
			{
			RPT_VERBOSE("CIO_QUERY status was bad at clearall");
			}
		/*
		 * Set flag to generate interrupts.  (I'm not sure why.)
		 * Run ucode to cause MCI interrupt.
	 	*/
		ROF(write1_sram(INTMCI, 1));
		if ( (rc = execute_ucode(cb,10)) == TU_HARD)
		{
			write_pos_reg(5,pos5_save);          /* cleanup on error */
			write_pos_reg(7,0);
			return(TU_HARD);
		}
#ifdef	MTEX
		mtsleep(5);		  		/* sleep a little */
#endif
		sleep(2);

		query_blk.clearall = 0;                 /* get statistics */
		if (ioctl(device, CIO_QUERY, &query_blk) == -1)   /* get statistics for int count */
			{
			sprintf(MSGBUF,"Query Statistics failed.\n");
			return(TU_SYS);
			}
		if (query_blk.status) 
			{
			RPT_VERBOSE("CIO_QUERY status was bad ");
			}
		int_val = query_stats.ds.total_intr;

		if (int_val != ++ints_exp) {
			sprintf(MSGBUF,"Interrupts expected: 1, received: %d, level index: %d\n",
				int_val,i);
			write_pos_reg(5,pos5_save);             /* cleanup on error condition */
			write_pos_reg(7,0);
			return(TU_SOFT);
		} else
			RPT_VERBOSE("First interrupt succeeded\n");

		/*
	 	* Set flag to generate interrupts.  (I'm not sure why.)
	 	* Do it again to cause the second interrupt.
		* Might need to sleep a tiny bit between them.
	 	*/

		ROF(write1_sram(INTMCI, 1));
		if ( (rc = execute_ucode(cb,10)) == TU_HARD)
		{	
			write_pos_reg(5,pos5_save);              /* cleanup on error condition */
			write_pos_reg(7,0);
			return(TU_HARD);
		}
#ifdef	MTEX
		mtsleep(5);				/* sleep a little */
#endif

		query_blk.clearall = 0; 			/* get statistics on int count */
		if (ioctl(device, CIO_QUERY, &query_blk) == -1)
		{
			sprintf(MSGBUF,"Query Statistics failed.\n");
			return(TU_SYS);
		}
		if (query_blk.status) 
			{
			RPT_VERBOSE("CIO_QUERY status was bad");
			}
		int_val = query_stats.ds.total_intr;

		if (int_val != ++ints_exp) {
			sprintf(MSGBUF,"Interrupts expected: 2, received: %d, level index %d\n",
				int_val,i);
			write_pos_reg(5,pos5_save);		/* cleanup on error */
			write_pos_reg(7,0);
			return(TU_SOFT);
		} else
			RPT_VERBOSE("Second interrupt succeeded\n");
	}
	write_pos_reg(5,pos5_save);              /* restore pos 5 to initial value */
	write_pos_reg(7,0);                      /* set extender back to 1 for 1 int mapping */ 
	return(TU_GOOD);
}

/*************************************************************************
* Confirm ability to interrupt the microchannel				 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
*									 *
* What this really does is:						 *
*	check to see if there are any spurious interrupts		 *
*	executes tu-003 in the microcode to pop the MCI interrupt	 *
*	check to see if the interrupt popped				 *
*	executes tu-003 twice in the microcode				 *
*	check to see that the interrupt counter = 2			 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SYS		One of the ioctl's failed			 *
*	TU_HARD		The diagnostic microcode isn't ready		 *
*	TU_SOFT		The interrupt was not received			 *
*************************************************************************/
tu_intmci(cb)
TUTYPE				*cb;		/* control block for necessary info */
{
	int			rc;
	int			int_val;
/***********************************************************/
/* change these declarations to malloc                     */
	cio_query_blk_t		query_blk;
	cat_query_t		query_stats;
/***********************************************************/

	query_blk.bufptr= (caddr_t) &query_stats;		/* set up for CIO_QUERY ioctl */ 
	query_blk.buflen = sizeof(query_stats);

	query_blk.clearall = CIO_QUERY_CLEAR;  			/* clear int count in statistics */
	if (ioctl(device, CIO_QUERY, &query_blk) == -1)
		{
		sprintf(MSGBUF,"Reset Query Statistics failed.\n");
		return(TU_SYS);
		}
	if (query_blk.status) 
		{
		RPT_VERBOSE("CIO_QUERY status was bad at clearall");
		}

	/*
	 * Set flag to generate interrupts.  (I'm not sure why.)
	 * Run ucode to cause MCI interrupt.
	 */
	ROF(write1_sram(INTMCI, 1));
	if ( (rc = execute_ucode(cb,10)) == TU_HARD)
		return(TU_HARD);

#ifdef	MTEX
	mtsleep(5);		  		/* sleep a little */
#endif
	sleep(1);

	query_blk.clearall = 0;					/* get statistics */
	if (ioctl(device, CIO_QUERY, &query_blk) == -1)		/* get statistics */
		{
		sprintf(MSGBUF,"Query Statistics failed.\n");
		return(TU_SYS);
		}
	if (query_blk.status) 
		{
		RPT_VERBOSE("CIO_QUERY status was bad");
		}
	int_val = query_stats.ds.total_intr;			/* total int count */

	if (int_val != 1) {
		sprintf(MSGBUF,"Interrupts expected: 1, received: %d\n",
			int_val);
		return(TU_SOFT);
	} else
		RPT_VERBOSE("First interrupt succeeded\n");

	/*
	 * Set flag to generate interrupts.  (I'm not sure why.)
	 * Run ucode to cause MCI interrupt.
	 * Do it again to cause the second interrupt.
	 * Might need to sleep a tiny bit between them.
	 */

	ROF(write1_sram(INTMCI, 1));
	if ( (rc = execute_ucode(cb,10)) == TU_HARD)
		return(TU_HARD);

#ifdef	MTEX
	mtsleep(5);				/* sleep a little */
#endif

	query_blk.clearall = 0;					/* get statistics */
	if (ioctl(device, CIO_QUERY, &query_blk) == -1)
		{
		sprintf(MSGBUF,"Query Statistics failed.\n");
		return(TU_SYS);
		}
	if (query_blk.status) 
		{
		RPT_VERBOSE("CIO_QUERY status was bad");
		}
	int_val = query_stats.ds.total_intr;                    /* get total intr count */

	if (int_val != 2) {
		sprintf(MSGBUF,"Interrupts expected: 2, received: %d\n",
			int_val);
		return(TU_SOFT);
	} else
		RPT_VERBOSE("Second interrupt succeeded\n");

	return(TU_GOOD);
}


/*************************************************************************
* Confirm ability to interrupt the 186					 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* Use the POS register to cause an interrupt of the OBP	in an		 *
* old board, use the memory mapped location on a newer board.		 *
*									 *
* What this really does is:						 *
*	Make sure the diagnostic microcode is running			 *
*	Clear the byte at location ERRCODE				 *
*	Cause an interrupt						 *
*	Wait for ERRCODE to become GOT_INTR				 *
*	Verify that interrupt bit was cleared if possible		 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SYS		One of the ioctl's failed			 *
*	TU_HARD		The diagnostic microcode isn't ready		 *
*	TU_SOFT		The interrupt was not received			 *
*************************************************************************/
tu_int186(cb)
TUTYPE				*cb;		/* control block for necessary info */
{
	struct errdbgblk	results;
	char			*end_str;
	int			tmp;
	BIT16			got;		/* # ints rcvd */
	int			rc;		/* used by ROF macro */
	int			timeout = 2;	/* secs to wait for int */

/*************************************************************************
* Wait for the diagnostic microcode to say "I'm here and waiting"	 *
*************************************************************************/
	if (wait_sram(READY, DIAG_WAITING, 2)) {
		read1_sram(READY,&tmp);
		sprintf(MSGBUF, "Board in wrong state (%s): "
			"Diagnostic Microcode not waiting for cmd\n",
			operlvl_string(tmp));
		ucode = UNKNOWN;
		return(TU_HARD);
	}
	end_str = MSGBUF;
/*************************************************************************
* install the interrupt handler                                          *
*************************************************************************/
	if( (run_cmd(TU_NUM,3)) != TU_GOOD) return(TU_HARD);     
/*************************************************************************
* Reset the count in SRAM, then interrupt the processor			 *
*************************************************************************/
	got = 0;
	/*
	 * Zero is Zero is Zero so no need to do this:
	 *	letni16(&got);
	 */
	ROF(write_sram(ERRCODE, &got, sizeof(got)));
	RPT_VERBOSE("Interrupting the processor now.\n");

#ifdef UCIRQ186
	/*
	 * Use the memory mapped magic address to
	 * interrupt the 186.
	 */
	ROF(write1_sram(UCIRQ186, 0xff));
#else
	/*
	 * Early versions of the board had a bit in POS.
	 * Use the bit in the POS register to
	 * interrupt the 186.
	 */
	ROF(change_pos_reg(4, 0x01, 0x01));
#endif

	/*
	 * Wait for the ucode to receive the interrupt.
	 * Granted this should be almost immediate, but this
	 * code is generalized to wait for timeout seconds.
	 * Note that AIX sleep has a granularity of 1 second.
	 * Thus timeout=1 can sometimes be timeout=0 !
	 */
	ROF(read_sram(ERRCODE, &got, sizeof(got)));	/* get count */
	letni16(&got);					/* byte swap */
	while (timeout-- > 0 && got == 0) {
#ifdef MTEX
		mtsleep(5);				/* only 50 ms -- still safe */
#endif
#ifdef AIX
		sleep(1);
#endif
		ROF(read_sram(ERRCODE, &got, sizeof(got)));	/* get count */
		letni16(&got);				/* byte swap */
	}

	if (got != 1) {
		ADD_SPRINTF(end_str, sprintf(end_str,
		    "OBP interrupts expected: 1, received: %d\n", got));
		return(TU_SOFT);
	}

#ifndef UCIRQ186
	/*
	 * When the Interrupt OBP bit was in POS we could
	 * verify that the OBP successfully cancelled the interrupt.
	 */
	ROF(valcheck_pos(4, 0x01, 0x00));
#endif
	return(TU_GOOD);
}
