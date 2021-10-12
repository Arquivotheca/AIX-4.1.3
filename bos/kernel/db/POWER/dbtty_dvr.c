static char sccsid[] = "@(#)51	1.30  src/bos/kernel/db/POWER/dbtty_dvr.c, sysdb, bos411, 9428A410j 4/17/94 21:16:42";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:   d_ttyopen, d_ttyclose, d_ttyget, d_ttyput, d_ttybinput,
 *              d_tty_ischar, machine_detect
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file contains redirection code for the polled mode tty access routines
 * used by the kernel debugger.  When a machine with a different kind of
 * console I/O is supported by AIX, a debugger file must be created that
 * exports the same interfaces as this routine.  This file must be modified to
 * make the machine detection routine correctly identify the new machine and
 * update the branch table correctly.
 */

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>

/* TTY I/O routines for Dakota architecture boxes                            */
extern void dsf_ttybinput(unsigned char);
extern void dsf_ttyclose();
extern unsigned char dsf_ttyget();
extern int dsf_ttyopen(int);
extern void dsf_ttyput(unsigned char);

/* TTY I/O routines for RS/6000 boxes                                        */
extern void drs_ttybinput(unsigned char);
extern void drs_ttyclose();
extern unsigned char drs_ttyget();
extern int drs_ttyopen(int);
extern void drs_ttyput(unsigned char);
#ifdef _PEGASUS
extern int drs_tty_ischar(int);
#endif /* _PEGASUS */


#define MAXTYPES 2                 /* How many machines to select between    */
#define TYPE_RS  0                 /* Machine type original RS/6000          */
#define TYPE_SF  1                 /* Machine type Sandalfoot                */

static mtype = -1;                 /* What machine type is this?             */

/* Structure containing pointers to the machine-specific versions of the */
/* tty driver functions used by the debugger. */
typedef struct ttyfunctab
{
  void (*ttybinput)();             /* d_ttybinput                            */
  void (*ttyclose)();              /* d_ttyclose                             */
  unsigned char (*ttyget)();       /* d_ttyget                               */
  int (*ttyopen)();                /* d_ttyopen                              */
  void (*ttyput)();                /* d_ttyput                               */
  int (*ttyischar)();              /* d_tty_ischar  PEGASUS only             */
} TTYFUNCTAB;

/* This constant array contains the TTYFUNCTAB structures used to initialize */
/* the static TTYFUNCTAB structure ttyfunc. */
const TTYFUNCTAB ttyfuncs[MAXTYPES] =
{
  {drs_ttybinput, drs_ttyclose, drs_ttyget, drs_ttyopen, drs_ttyput,
#ifdef _PEGASUS		/* Because drs_tty_ischar is similarly bracketed. */
   drs_tty_ischar},
#else /* not _PEGASUS */
   NULL},
#endif /* _PEGASUS */
  {dsf_ttybinput, dsf_ttyclose, dsf_ttyget, dsf_ttyopen, dsf_ttyput,NULL}
};

/* Static variable containing fcn ptrs to the routines we will be using on this
   system */
static TTYFUNCTAB ttyfunc;

/* machine_detect - This routine must figure out what type of system we are	*/
/*                  running on W.R.T. ttys.  It will return a non-negative	*/
/*                  integer less than MAXTYPES.  Internally it will select	*/
/*                  between all the TYPE_* #defined machines and return one	*/
/*                  of them.							*/
/* ASSUMPTION: At the time this routine runs hardinit() has already run and	*/
/*             filled in the system_config structure.  This assumption is	*/
/*             can be checked by looking at main.c and making sure hardinit()	*/
/*             is still called prior to debugger_init().			*/
/* NOTE: This routine should only need to be called from the d_ttyopen routine.	*/
/*	No one should be doing tty I/O without opening the tty first.		*/
static void machine_detect()
{
  if (mtype < 0) {			/* Have we tried this before?		*/
	if (__rspc())			/* Are we running on a Dakota box?	*/
		mtype = TYPE_SF;	/* YES - update the type field		*/
	else				/* No, 					*/
		mtype = TYPE_RS;	/*   assume we are on an RS/6000	*/
	ttyfunc = ttyfuncs[mtype];	/* Copy in proper function pointers	*/
  }
  assert((mtype >= 0) && (mtype < MAXTYPES));
}


/* Routine: d_ttyopen
 * Function: sets up tty for debugger use
 * Parameters: number of serial port to use
 * Returns: 1 if tty opened successfully, 0 otherwise
 * Note: since this routine must be called before any use of the tty by the
 *	debugger, this is the only one of these routines that calls
 *	machine_detect.
 */
int d_ttyopen(int port)
{
  if (mtype<0) machine_detect();	/* Call machine detect if not called b4	*/

  return((*ttyfunc.ttyopen)(port));	/* Call machine-dependent ttyopen */
}

/* Routine: d_ttyput
 * Function: Calls machine-dependent ttyput routine to write data to tty
 * Parameters: data to write to tty
 * Returns: nothing
 * Note: d_ttyopen must be called before this routine is called.  Uses the 
 *	ttyfunc static data structure to retrieve the function pointer to
 *	be used.
 */
void d_ttyput(unsigned char data)
{
  (*ttyfunc.ttyput)(data);		/* Call machine-dependent ttyput */
}

/* Routine: d_ttybinput
 * Function: Calls machine-dependent ttyput routine to write data to tty
 *	in binary mode
 * Parameters: data to write to tty
 * Returns: nothing
 * Note: d_ttyopen must be called before this routine is called.  Uses the 
 *	ttyfunc static data structure to retrieve the function pointer to
 *	be used.
 */
void d_ttybinput(unsigned char data)
{
  (*ttyfunc.ttybinput)(data);
}

/* Routine: d_ttyget
 * Function: Calls machine-dependent ttyget routine to read a character
 * from the tty
 * Parameters: none
 * Returns: character read from tty
 * Note: d_ttyopen must be called before this routine is called.  Uses the 
 *	ttyfunc static data structure to retrieve the function pointer to
 *	be used.
 */
unsigned char d_ttyget()
{
  return((*ttyfunc.ttyget)());
}

/* Routine: d_ttyclose
 * Function: Terminates debugger use of tty
 * Parameters: none
 * Returns: none
 * Note: d_ttyopen must be called before this routine is called.  Uses the 
 *	ttyfunc static data structure to retrieve the function pointer to
 *	be used.
 */
void d_ttyclose()
{
  (*ttyfunc.ttyclose)();
}

/* Routine: d_tty_ischar
 * Function: Checks whether a character is available in the tty for pegasus
 *	machines.
 * Parameters: only for compatibility
 * Returns: 1 if char available, 0 otherwise
 * Note: d_ttyopen must be called before this routine is called.  Uses the 
 *	ttyfunc static data structure to retrieve the function pointer to
 *	be used.
 */
#ifdef _PEGASUS
int d_tty_ischar(int line)
{
  return((*ttyfunc.ttyischar)(line));
}
#endif /* _PEGASUS */
