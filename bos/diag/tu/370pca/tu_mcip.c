static char sccsid[] = "@(#)tu_mcip.c	1.4 6/14/91 14:46:29";
/*****************************************************************************
* TU_MCI_PAR.C:		 test unit code for mci parity test    	4/19/90	     *
*									     *
* functions:		 tu_mci_parity					     *
*									     *
*									     *
* Origins:		 27 (IBM)					     *
*									     *
* (C) Copyright International Business Machines Corp. 1990		     *
* All Rights Reserved							     *
* Licensed Materials - Property of IBM					     *
*									     *
* US Government Users Restricted Rights - Use, duplication or		     *
* disclosure resticted by GSA ADP Schedule Contract with IBM Corp.	     *
*****************************************************************************/
#ifdef AIX
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "catuser.h"
#endif

#include "psca.h"
#include "pscatu.h"
#include "extern.h"

#ifdef MTEX
#include "pscadefs.h"
extern STATAREA far		*statarea;
extern BYTE far			*cardarea;
extern unsigned int		numints;	    /* number of interrupts */
extern unsigned int		cardslot;
#endif
 
static int	table[] = {
		0x0002,
		0x0001,
		0x0008,
		0x0004,
		0x0005,
		0x0020,
		0x0010,
		0x0080,
		0x0040,
		0x0050,
		0x0055,
		0x0200,
		0x0100,
		0x0800,
		0x0400,
		0x0500,
		0x2000,
		0x1000,
	/*	0x8000,  Hardware bug, pattern wont work */
        /*	0x4000,        */
	/*	0x5000,        */
	/*	0x5500,        */
	/*	0x5555,        */  
		0x0000
        /*	0xAAAA };      */  
			};
static int      tablesize = (sizeof(table) / sizeof(int));

change_forcing(force_a_par,force_d_par)
int	force_a_par,force_d_par;
{
	int	rc;
	int	temp,temp1;
	ROF(read_pos_reg(7,&temp));
	if (force_a_par) temp |= 0x0001; else temp &= 0xFFFE;

	if (force_d_par) temp |= 0x0002; else temp &= 0xFFFD;
	ROF(write_pos_reg(7,temp));
	ROF(read_pos_reg(7,&temp1));
	if (temp != temp1)
	{
		sprintf(MSGBUF,"Pos reg 7 wrote %x readback %x\n",temp,temp1);
		return(TU_SYS_ERRNO);
	}
 	return(0);
}

change_checking(check_a_par,check_d_par)
int	check_a_par,check_d_par;
{
	int	rc;
	int	temp,temp1;
	ROF(read_pos_reg(2,&temp));
	if (check_a_par) temp |= 0x0002; else temp &= 0xFFFD;

	if (check_d_par) temp |= 0x0004; else temp &= 0xFFFB;
	ROF(write_pos_reg(2,temp));
	ROF(read_pos_reg(2,&temp1));
	if (temp != temp1)
	{
		sprintf(MSGBUF,"Pos reg 2 wrote %x readback %x\n",temp,temp1);
		return(TU_SYS_ERRNO);
	}
 	return(0);
}

read_sram_ck(addr, buf, stat)
SRAM		addr;
BYTE		*buf;
unsigned int	*stat;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = 4;
	cmd.str_ptr = buf;
	cmd.rd_wrt = CAT_CHNL_RD_NO_FAIL;
	cmd.dma_mode = CAT_USE_PIO;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	*stat = cmd.status;
	return(TU_GOOD);
#endif
#ifdef MTEX
	mcopy(cardarea + addr, (BYTE far *) buf, 4);
	return(TU_GOOD);
#endif
}


write_sram_ck(addr, buf, stat)
SRAM		addr;
BYTE		*buf;
unsigned int	*stat;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = 4;
	cmd.rd_wrt = CAT_CHNL_WR_NO_FAIL;
	cmd.dma_mode = CAT_USE_PIO;
	cmd.str_ptr = buf;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	*stat = cmd.status;
	return(TU_GOOD);
#endif
#ifdef MTEX
	mcopy((BYTE far *) buf, cardarea + addr, 4);
	return(TU_GOOD);
#endif
}

validate_data(stat_expected, stage)
unsigned int	stat_expected;
int		stage;

{
int		rc;
unsigned int	k, valueback;
unsigned int	stat;

for (k=0 ; k<tablesize ; k++)
{
	ROF (write_sram_ck((SRAM)0x1000, &table[k], &stat));
	if (stat != stat_expected)
	{
		sprintf(MSGBUF,"validate data par write op, stage %3d, pattern %x\n stat expected %d, stat actual %d\n",stage,table[k],stat_expected,stat);
		return(TU_SYS_ERRNO);
	}
	ROF (read_sram_ck((SRAM)0x1000, &valueback, &stat));
	if (stat != stat_expected)
	{
		sprintf(MSGBUF,"validate data par read op, stage %3d, pattern %x\n stat expected %d, stat actual %d\n",stage,table[k],stat_expected,stat);
		return(TU_SYS_ERRNO);
	}
	if ((table[k] != valueback) && (stat_expected == 0))
	{
		sprintf(MSGBUF,"Stage %3d, validate data readback of SRAM is incorrect. Expected %d,received %d \n",stage,table[k],valueback);
		return(TU_SYS_ERRNO);
	}
}
return(0);
}

validate_addr(stat_expected, stage)
unsigned int    stat_expected;
int		stage;
{
int		rc;
int		k;
unsigned int	valueback;
unsigned int	stat;
int		pattern;

for (k=0 ; k<11 ; k++)
{
	pattern = 0x12340567;
	ROF (write_sram_ck((SRAM) table[k] << 2, &pattern, &stat));
        if (stat != stat_expected)
	{
		sprintf(MSGBUF,"validate addr par write op, stage %3d, pattern %x\n stat expected %d, stat actual %d\n",stage,table[k],stat_expected,stat);
		return(TU_SYS_ERRNO);
	}
	ROF (read_sram_ck((SRAM) table[k] << 2, &valueback, &stat));
	


        if (stat != stat_expected)
	{
		sprintf(MSGBUF,"validate addr par read op, stage %3d, pattern %x\n stat expected %d, stat actual %d\n",stage,table[k],stat_expected,stat);
		return(TU_SYS_ERRNO);
	}
	if ((pattern != valueback) && (stat_expected == 0))
	{
		sprintf(MSGBUF,"Stage %3d, validate addr readback of SRAM is incorrect. Expected %d,received %d \n",stage,table[k],valueback);
		return(TU_SYS_ERRNO);
	}
}
return(0);
}

tu_mci_parity(cb)
TUTYPE	 *cb;
{
int		rc;

/* make sure no parity errors are forced */
	ROF(change_forcing(0,0));
/* turn on parity checking */
	ROF(change_checking(1,1));
/* test all patterns for address, data, and return if an error */
	ROF(validate_addr(CIO_OK, 1));
	ROF(validate_data(CIO_OK, 2));

/* this section forces data parity errors */
	ROF(change_forcing(0,1)); 		/* force data par errors */
	ROF(validate_data(CIO_HARD_FAIL,3));	
	ROF(change_checking(1,0));	/* turn off data parity checking */	
	ROF(validate_data(CIO_OK, 4));
	ROF(change_checking(1,1));      /* turn parity checkers back on */

/* this section forces addr parity errors */
	ROF(change_forcing(1,0)); 		/* force addr par errors */
	ROF(validate_addr(CIO_HARD_FAIL,5));	
	ROF(change_checking(0,1));	/* turn off addr parity checking */	
	ROF(validate_addr(CIO_OK, 6));
	ROF(change_checking(1,1));      /* turn parity checkers back on */
	ROF(change_forcing(0,0));	/* turn off error forcing */

        return(TU_GOOD);
}
