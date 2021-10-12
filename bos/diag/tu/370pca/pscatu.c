static char sccsid[] = "src/bos/diag/tu/370pca/pscatu.c, tu_370pca, bos411, 9428A410j 8/8/91 15:29:33";
/*************************************************************************
* This module is IBM CONFIDENTIAL, and Copyright 1989, IBM Corporation.	 *
*									 *
* This module contains the routines to perform each of the Test Units.	 *
* The correlation between test unit and function names is given		 *
* by the tu array at the bottom of this file.				 *
*									 *
* MODIFICATION HISTORY							 *
* 07/24/89 PCC	V1.0.0	Original untested version (HTX code) TU-5 & 14	 *
* 07/25/89 PCC	V1.0.0	Made ps and pr globals like they should be	 *
* 07/26/89 PCC	V1.2	Code is now HTX independent, made cmds to diag	 *
*			microcode independent of implementation		 *
*			(i.e. simple handshakes vs. complicated FIFO's)	 *
*			Also started using SCCS, now called V1.2	 *
* 07/27/89 PCC	V1.3	Added ps.good, ps.bad, etc., started using SOFT	 *
*			and HARD errors.  Now CHECK even the safe system *
*			calls. etc.					 *
* 07/31/89 PCC	V1.5	Reinstigated exectu and TUTYPE for Rios Diags	 *
* 08/14/89 PCC		Made exectu and the TU's use TU_GOOD, TU_SOFT,	*
*			TU_HARD, TU_SYS, TU_SYS_ERRNO.			 *
* 08/17/89 PCC		Named modules by function rather than test num	 *
* 08/21/89 PCC		Made a tu struct instead of tu_name, tu_func.	 *
* 10/26/89 MWW		Factored out common stuff into execute_ucode	 *
* 01/25/90 SRH		Added code to support TU 26 (tu_func)		 *
* 08/02/90 SRH          added support for tu 8, int level test           *
*************************************************************************/

/*************************************************************************
* When there is no real meaning for hard errors vs. soft errors I have	 *
* decided to use HARD for when a test does not successfully run to	 *
* completion and SOFT for when the test completes but detects a problem. *
*************************************************************************/

#ifdef AIX
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "catuser.h"
#endif

#include <stdio.h>
#include "psca.h"
#include "pscatu.h"
#include "extern.h"

#ifdef MTEX
#include "pscadefs.h"
#endif

int		device;				/* fd for psca, used everywhere */
int		ucode = UNKNOWN;		/* currently downloaded microcode */

#ifdef BOXMFG
TUTYPE		*tucb_ptr;			/* global pointer to test unit control block */
#endif
 
exectu(fd, cb)
int		fd;				/* fildes of device */
TUTYPE		*cb;				/* control block: all info for tu's */
{
	int		rc = 0;
	int		counter = 0;
	char		*end_of_msg;
	extern struct tu_type	tu[];		/* Defined at the bottom of this file */
	extern int	reset_board();		/* reset the board */
	extern int	dnld_code();		/* download the microcode */

#ifdef BOXMFG
	tucb_ptr = cb;
#endif

	device = fd;				/* why pass it to EVERY routine? */
	if (VERBOSE) {
		sprintf(MSGBUF, "TU-%02d  %s -- Beginning execution\n",
		    TU_NUM, TU_NAME);
		RPT_VERBOSE(MSGBUF);
	}
	MSGBUF[0] = '\0';			/* erase previous message */
/*************************************************************************
* Execute the TU until counter == loop.  If loop == 0 then run forever	 *
* Quitting on error is decided by the severity and the environment.	 *
*************************************************************************/
	do {
		max_errors = cb->pr.maxerrs;	/* reset error msg count */
/*************************************************************************
* Make sure that the currently running microcode matches the		 *
* requirements of the TU before executing it.				 *
*************************************************************************/
		rc = set_ucode(cb);

		if (rc != 0) {
			RPT_HARD(0, MSGBUF);
			ucode = UNKNOWN;
			return(-1);
		}

/**************************************************************************/
/* Run test unit, handle return code					  */
/**************************************************************************/
		rc = (*tu[TU_NUM].func)(cb);
		if (rc) {
			end_of_msg = MSGBUF + strlen(MSGBUF);
			sprintf(end_of_msg, "tu=%d, loop=%d, counter=%d\n",
				TU_NUM, cb->header.loop, counter);
		}
		switch(rc) {
		case TU_GOOD:
#ifdef HTX
			if (TU_NUM != 26) INC_GOOD_OTHERS;
#endif		
			if (VERBOSE) {
				sprintf(MSGBUF, "TU-%02d  %s -- Successful\n",
				    TU_NUM, TU_NAME);
				RPT_VERBOSE(MSGBUF);
			}
			break;
		case TU_SOFT:
			RPT_SOFT(0, MSGBUF);
#ifdef MFG
			return(-1);
#endif
#ifdef HTX
			if (TU_NUM != 26) INC_BAD_OTHERS;
			break;
#endif
#ifdef DIAGS
			return(rc);
#endif
#ifdef MTEX
			return(-1);
			break;
#endif
		case TU_HARD:
			RPT_HARD(0, MSGBUF);
#ifdef MFG
			return(-1);
#endif
#ifdef HTX
			if (TU_NUM != 26) INC_BAD_OTHERS;
			break;
#endif
#ifdef DIAGS
			return(rc);
#endif
#ifdef MTEX
			return(-1);
			break;
#endif
		case TU_SYS:
			RPT_ERR(0, MSGBUF);
#ifdef MTEX
			break;	/* invalid tests return TU_SYS - ignore them */
#endif
#ifdef HTX
			if (TU_NUM != 26) INC_BAD_OTHERS;
#endif		
#ifdef DIAGS
			return(rc);
#endif
			return(-1);
		case TU_SYS_ERRNO:
#ifdef MTEX
			RPT_ERR(0, MSGBUF);
#else
			RPT_ERR(errno, MSGBUF);
#endif
#ifdef HTX
			if (TU_NUM != 26) INC_BAD_OTHERS;
#endif	
#ifdef DIAGS
			return(rc);
#endif	
			return(-1);
		default:
			sprintf(MSGBUF, "TU-%02d  %s -- Unknown TU result code %04x.\n",
			    TU_NUM, TU_NAME, rc);
			RPT_ERR(0, MSGBUF);
			return(-1);
		}
	} while (++counter < cb->header.loop || cb->header.loop == 0);
	return (0);
}


set_ucode(cb)
TUTYPE		*cb;				/* control block: all info for tu's */
{
	int		rc = 0;

	switch(tu[TU_NUM].ucode) {
	case NOCARE:
		break;
	case NONE:				/* NONE really means anything but FUNC */
		if (ucode == FUNC)		/* if FUNC, reset board, ignore errors */
			rc = reset_board();	/* not much you could do anyway */
		break;				/* perhaps I should wait for POST here */
	
	case FUNC:
		if (ucode != FUNC) {
#ifndef DIAGS
			rc = dnld_code(cb->pr.funccode);
#else
			rc = dnld_code(FUNC_UCODE);
#endif
			/*
			 * ucode is not changed here to sense the need to
			 * initialize the functional ucode (run func_prep).
			 * ucode = FUNC;
			 */
		}
		break;
	case DIAG:
		if (ucode != DIAG) {
#ifndef DIAGS
			rc = dnld_code(cb->pr.diagcode);
#else
			rc = dnld_code(DIAG_UCODE);
#endif
			ucode = DIAG;
		}
		break;
	}

/*
 * Run func_prep if this test unit loaded functional ucode above.
 * Don't know how to do this under MTEX.
 */
	if (tu[TU_NUM].ucode == FUNC && ucode != FUNC)
#ifndef MTEX
		if ( func_prep(cb) )
			return(-1);
		else
			ucode = FUNC;
#else
	{
		sprintf(MSGBUF,"Don't know how to initialize functional ucode\n");
		return(-1);
	}
#endif

	return(rc);
}


/*************************************************************************
* No such test - return System Error					 *
*************************************************************************/
no_such_test(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	sprintf(MSGBUF, "Unexpected: Invalid test (%d)\n", TU_NUM);
	return(TU_SYS);
}


/*************************************************************************
* Test not implemented yet - return System Error			 *
*************************************************************************/
not_yet(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	sprintf(MSGBUF, "TU-%02d  %s -- Not yet implemented.\n",
	    TU_NUM, TU_NAME);
	return(TU_SYS);
}


/*************************************************************************
* POS register test							 *
* The functional microcode should NOT be loaded during this test	 *
*									 *
* exercise the testable bits of the POS registers.			 *
*									 *
* Appropriate tests (under AIX) are:					 *
*	Check reg0 and reg1 against DEV_ID_LSB and DEV_ID_MSB		 *
*	Check reg2 bit 0; R/W bit 0-3; R/W bit 4-7 under MFG		 *
*	Read VPD from reg3 (using reg6) and compare to possible value(s) *
*	Check reg4 bit 0-2, 7; R/W bits 0-7				 *
*	Check reg5 bit 6-7; R/W 0-6; Dag Blokkum says don't write bit 7	 *
*	R/W reg6, all bits						 *
*	R/W reg7, all bits						 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		Any failure of expected vs. received		 *
*	TU_HARD		Any time-out failure				 *
*************************************************************************/
#define DEV_ID_LSB	0x92			/* The PSCA's defined Device ID */
#define DEV_ID_MSB	0xFE			/* The PSCA's defined Device ID */
tu_pos_regs(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	int		rc,i;			/* used in ROF macro */
	char		*s;
	unsigned int	val;
	extern int	valcheck_pos();		/* check POS bit for proper value */
	extern int	rwtest_pos_reg();	/* do R/W test on POS bits */
	extern int	test_vpd();		/* subtest to check VPD */

	RPT_VERBOSE("Initial values of reg 2, 4, and 5 largely unknown and untested.\n");
	if (VERBOSE)
	  {
		s = MSGBUF;
		ADD_SPRINTF(s, sprintf(s, "POS Registers 100h - 107h:  "));
	   for (i=0 ; i < 8; i++)
	      {
			read_pos_reg(i, &val);
			ADD_SPRINTF(s, sprintf(s, "%02x ", val));
		}
		ADD_SPRINTF(s, sprintf(s, "\n"));
		RPT_VERBOSE(MSGBUF);
	}
	ROF(valcheck_pos(0, 0xff, DEV_ID_LSB)); /* check for proper POS values */
	ROF(valcheck_pos(1, 0xff, DEV_ID_MSB));
	ROF(valcheck_pos(2, 0x01, 0x01));	/* this depends on OS initialization */
 
	ROF(read_pos_reg(2,&val));   		/* turn card off line by resetting pos 2 bit 0 */
	val &= 0xfe;
	ROF(write_pos_reg(2,val));
	
	/*
	 * R/W test of POS 2 does not currently change base address bits.
	 * This could be done with the card disabled.
	 */
	ROF(rwtest_pos_reg(2, 0x0f));		/* check ability to R/W POS regs */
	ucode = UNKNOWN;
	RPT_VERBOSE("Board reset, Microcode unknown\n");
#ifdef CARD_POS					/* interrupt bits in POS regs */
	ROF(rwtest_pos_reg(4, 0xfc));		/* this does nasty things to 186 */
#else
	ROF(rwtest_pos_reg(4, 0xff));		/* this does nasty things to 186 */
#endif
	ROF(rwtest_pos_reg(5, 0x3f));
 	
	ROF(read_pos_reg(2,&val));		/* turn card on line by setting pos 2 bit 0 */
	val |= 0x01;
	ROF(write_pos_reg(2,val));
	
	return(TU_GOOD);
}


/*************************************************************************
* Run POST								 *
*									 *
* Run the POST by resetting the board.	Report results of the test.	 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		The POST reported a problem			 *
*	TU_HARD		The test did not complete			 *
*************************************************************************/
tu_post(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	int		rc;			/* used in ROF macro */
	struct errdbgblk	res;		/* results of POST */
	extern int	reset_board();		/* reset the board */
	extern int	read_sram();		/* read block from shared RAM */
	extern int	post_results();		/* Interpret POST error */

	ucode = UNKNOWN;
	ROF(reset_board());				/* reset the board */
	if (wait_post(OPERLVL, DL_WAITING, 30)) {
		sprintf(MSGBUF, "POST did not complete in 30 sec.\n");
		return(TU_HARD);
	}
	ROF(read_dbgblk(&res)); /* get results of POST */
	if (res.errcode) {
		post_results(&res, MSGBUF);
		return(TU_SOFT);
	}
	return(TU_GOOD);
}


/*************************************************************************
* CPU test								 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command to perform a test of the CPU			 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		The test discovered a problem (unlikely IMHO)	 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
tu_cpu(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	return(execute_ucode(cb,15));	/* run ucode for 15 secs */
}

/*************************************************************************
* Wrap test								*
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command to perform a wrap test of the tag&bus lines		 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		The test discovered a problem (unlikely IMHO)	 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
tu_wrap(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	return(execute_ucode(cb,15));	/* run ucode for 15 secs */
}

/*************************************************************************
* SRAM test								 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command to perform a test of the Static RAM			 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		A RAM failure was detected			 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
tu_sram186(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	return(execute_ucode(cb,100));	 /* run ucode for 100 secs */
}


/*************************************************************************
* PSCA Hardware Register test						 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command to perform a test of the Hardware Registers		 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		A register failure was detected			 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
tu_regs(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	return(execute_ucode(cb,15));	/* run ucode for 15 secs */
}

/*************************************************************************
* PSCA 186 FIFO test.							 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command to perform a test of the 186 FIFO			 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		A register failure was detected			 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
tu_fifo_186(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	return(execute_ucode(cb,15));	/* run ucode for 15 secs */
}


/*************************************************************************
* DRAM test								 *
* Requires that the diagnostic microcode has been loaded		 *
*									 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command to perform a test of the Dynamic RAM		 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		A RAM failure was detected			 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
tu_dram(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	int	rc;

	if ( (rc=execute_ucode(cb,90)) != TU_GOOD)  /* run ucode for 60 secs */
	   ucode = UNKNOWN;			/* if DRAM is bad,  ucode may be corrupted. */
	return(rc);
}


extern int	tu_srammci();		/* moved to tu_sram.c */

/*************************************************************************
* execute_ucode								 *
* Requires that the diagnostic microcode has been loaded		 *
* request the execution and report the results of the Diagnostic	 *
* Microcode command passed in cb					 *
* MWW 891026 this just factors out the common stuff from a bunch of	 *
* similar tests.							 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SOFT		A register failure was detected			 *
*	TU_HARD		Test did not complete				 *
*************************************************************************/
execute_ucode(cb, timeout)
TUTYPE		*cb;				/* control block for necessary info */
int		timeout;			/* timeout for ucode */
{
	struct errdbgblk results;
	char		*end_str;
	int		rc;			/* used by ROF macro */
	extern int	run_cmd();		/* pass cmd to diag ucode */
	extern int	mem_results();		/* interpret error */

	if (run_cmd(TU_NUM, timeout)) {
		end_str = MSGBUF;
		ADD_SPRINTF(end_str, sprintf(end_str, "TU-%02d	%s -- TIMED OUT.\n",
		    TU_NUM, TU_NAME));
		if (VERBOSE) {
			ADD_SPRINTF(end_str, sprintf(end_str,
			    "Diagnostic Microcode did not finish normally.\n"));
			ROF(read_dbgblk(&results));
			mem_results(&results, end_str);
		}
		ucode = UNKNOWN;
		return(TU_HARD);
	}
	ROF(read_dbgblk(&results));
	if (results.errcode) {
		end_str = MSGBUF;
		ADD_SPRINTF(end_str, sprintf(MSGBUF, "%s: FAILED: ", TU_NAME));
		mem_results(&results, end_str);
		return(TU_SOFT);
	}
	return(TU_GOOD);
}


extern int	tu_srammci();		/* moved to tu_sram.c */


/*************************************************************************
* Download microcode to the PSCA					 *
*									 *
* Use the CIO_DNLD command of the ioctl mechanism to download		 *
* the contents of the file named by DOWNLOAD to the board.		 *
*									 *
* What this really does is:						 *
*	reset the board,						 *
*	wait for the POST to complete					 *
*	check the completion status of the POST				 *
*	copy the microcode into an area of the shared Static RAM	 *
*	set a specific bit to tell the ROM code running on the board	 *
*		that there is code to be downloaded.			 *
*	We then wait around for the ROM to confirm that the microcode	 *
*		has been copied into the DRAM.				 *
* The possible results are:						 *
*	Everything went fine						 *
*	The POST failed							 *
*	We timed-out waiting for the POST to complete			 *
*	We timed-out waiting for confirmation of the download (There	 *
*		really isn't any excuse for this)			 *
*									 *
* return values								 *
*	TU_GOOD		Everything went fine				 *
*	TU_SYS_ERRNO	A system error occured and errno applies	 *
*	TU_SYS		A system error occured and errno does not apply	 *
*	TU_HARD		The Download failed				 *
*	TU_SOFT		The Download succeeded but the ucode didn't run	 *
*************************************************************************/
#define DOWNLOAD	(cb->pr.diagcode)

tu_download(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	int		rc;

	ROF(dnld_code(DOWNLOAD));
	ucode = DIAG;
	return(TU_GOOD);
}

/*************************************************************************
* Find str in a table of strings					 *
* return values								 *
*	n	found string at index n in table			 *
*	-1	string not found					 *
*************************************************************************/
table_cmp(table, str)
char		*table[];
char		*str;
{
	int		i = 0;

	while (table[i] != NULL) {
		if (strcmp(table[i], str) == MATCH)
			return(i);		/* found it */
		i++;
	}
	return(-1);				/* not found */
}

extern int	tu_fifo_mci();			/* code moved to tu_fifo.c */

extern int	tu_baseaddr();			/* code moved to tu_sram.c */

extern int	tu_int186();			/* code moved to tu_int.c */

extern int	tu_intmci();			/* code moved to tu_int.c */

#ifndef BOXMFG
#ifndef DIAGS
extern int	tu_intlvl();	        	/* code in tu_int.c */
#endif
#endif

extern int	test_vpd();			/* code moved to misc.c */

extern int	tu_dma();			/* code is in tu_dma.c */

#ifndef BOXMFG
#ifndef DIAGS
extern int	tu_func();			/* code in tu_func.c */

extern int	tu_mci_parity();		/* code in tu_mcip.c */
#endif
#endif
/*************************************************************************
* End of Test Unit code							 *
*************************************************************************/

/*************************************************************************
* Declare the array that defines the test units				 *
*	ucode		What microcode is required for the test		 *
*	func		A pointer to the function for the test		 *
*	name		A textual name for the test			 *
*************************************************************************/
struct tu_type	tu[] = {
{ /*  0 */	NOCARE, no_such_test,	"No test"					},
{ /*  1 */	NOCARE,	tu_post,	"Power on Self Test"				},
{ /*  2 */	NOCARE,	tu_pos_regs,	"Write/Read POS Register"			},
{ /*  3 */	DIAG,	tu_intmci,	"Interrupt Test (to MCI)"			},
{ /*  4 */	DIAG,	tu_cpu,		"Advanced CPU Test"				},
{ /*  5 */	DIAG,	tu_dram,	"Advanced DRAM Test"				},
{ /*  6 */	DIAG,	tu_sram186,	"Advanced Static Ram test (From 186)"		},
{ /*  7 */	NOCARE,	test_vpd,	"Individual VPD test"				},
#ifdef HTX
{ /*  8 */	DIAG,	tu_intlvl,	"Microchannel Interrupt Level Test"		},
#else
{ /*  8 */	NOCARE, no_such_test,   "only for stand-alone HTX (cardmfg)" 		},
#endif
{ /*  9 */	DIAG,	tu_wrap,	"Internal Wrap"					},
{ /* 10 */	DIAG,	tu_wrap,	"Wrap Plug at the Card"				},
{ /* 11 */	NOCARE, no_such_test,	"Wrap Plug Cable: Cancelled"			},
{ /* 12 */	DIAG,	tu_regs,	"PSCA Hardware Register Test"			},
{ /* 13 */	NOCARE,	not_yet,	"OLTS"						},
{ /* 14 */	NOCARE,	tu_download,	"Load Diagnostic Task"				},
{ /* 15 */	DIAG,	tu_wrap,	"Wrap Plug at box (tailgate)"			},
{ /* 16 */	FUNC,	not_yet,	"IO CTCL"					},
{ /* 17 */	DIAG,	tu_fifo_186,	"Hardware FIFO test (From 186)"			},
{ /* 18 */	DIAG,	tu_fifo_mci,	"Hardware FIFO test (From MCI)"			},
{ /* 19 */	NOCARE,	not_yet,	"SAK IVORIE"					},
{ /* 20 */	NOCARE, not_yet,	"***Removed Gate Array Loopback setup"		},
{ /* 21 */	NOCARE,	tu_baseaddr,	"Base Address Switching Test"			},
#ifdef HTX
{ /* 22 */	NOCARE,	tu_mci_parity,	"MicroChannel Parity test"			},
#else
{ /* 22 */      NOCARE, no_such_test,   "used only HTX stand-alone (cardmfg)"		},
#endif
{ /* 23 */	NOCARE,	tu_srammci,	"Advanced Static RAM Test (From MCI)"		},
{ /* 24 */	DIAG,	tu_dma,		"DMA test"					},
{ /* 25 */	DIAG,	tu_int186,	"Interrupt Test (to 186)"			}
#ifdef HTX
,
{ /* 26 */	FUNC,	tu_func,	"370 Data Loop Back Test"			}
#else
,
{ /* 26 */      NOCARE, no_such_test,   "used only HTX stand-alone (cardmfg)"		}
#endif

};

int max_tu = sizeof(tu) / sizeof(*tu) - 1;	/* oddball external */
