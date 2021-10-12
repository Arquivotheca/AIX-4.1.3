/* @(#)17       1.5.1.4  src/bos/kernext/rcm/inc/gscsubr.h, rcm, bos41J, 9519B_all 5/10/95 16:34:39 */

#ifndef _H_GSCSUBR
#define _H_GSCSUBR

/*
 * COMPONENT_NAME: (rcm) Internal functions externs
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*

	in rcmgp.c

*/


extern void fix_mstsave ();
/*
        rcmProcPtr  	    pproc;
	devDomainPtr        pdom;
	int         	    flags;
*/
#define FIX_MSTSAVE_SUB     0         /* subtract authority  */
#define FIX_MSTSAVE_ADD     1         /* add authority */


/*

	in rcmrcx.c

*/

extern void make_null_current ();
/*      
    	rcmProcPtr  pproc;
	int	    domain;
*/


/*

	in rcmwin.c

*/

extern int free_win_geom();
/*      
    	rcmWGPtr     pWG;
	int          flags;
*/
#define FREE_WG_WG      1             /* free wg */
#define FREE_WG_REGION  2             /* free clip region */
#define FREE_WG_VIS	4	      /* free visibility region */


extern int free_win_attr();
/*      
    	rcmWAPtr    pWA;
	int         flags;
*/
#define FREE_WA_WA          1         /* free wa */
#define FREE_WA_REGION      2         /* free region */
#define FREE_WA_PIXMAP      4         /* free pixmap */


extern void unlink_win_attr ();
/*      
    	rcmWAPtr    pwa;
*/


extern int rcm_delete_win_geom ();
/*      
    	gscDevPtr       pdev;
	rcmWGPtr        pwg;
	int             perror;
*/


extern int rcm_delete_win_attr ();
/*      
    	gscDevPtr        pdev;
	rcmProcPtr       pproc;
	rcmWAPtr         pwa;
	int              *perror;
*/


/*

	    in rcmswitch.c

*/

extern int gp_fault ();
/*
        struct uexcepth         *pexcp;
	int                     type;
	pid_t                   pid;
	struct mstsave          *pmst;
*/


extern void gp_dispatch ();
/*
    	struct      trb     *ptimer;
*/


extern void rcx_switch ();
/*      
    	rcxPtr  old, new;
	ulong   switches;
	ulong   mask;
	int	old_int;
*/


void rcx_switch_done ();
/*
        devDomainPtr        pdom;
	rcxPtr              prcx;
	ulong		    switches;
*/

#define RCX_SWITCH_DOIT       1       /* do all switching in-line */
#define RCX_SWITCH_QUEUE      2       /* queue to heavy switch contr, if nec */
#define RCX_SWITCH_NO_ENABLE  4       /* don't reenable interrupts */


extern void rcx_fault_list ();
/*  
    	devDomainPtr    pdom;                 domain
	rcxPtr          prcx;                 rcx to put on list
	ulong           switches;             switches for action
*/

#define RCX_FAULT_WAIT      2         /* wait rather than block */
#define RCX_FAULT_OVER      4         /* override the normal priority and
	                                 put the rcx at the head of the list */
#define RCX_FAULT_BLOCK     8         /* block process */

/*

	in rcmlock.c

*/

extern void guard_dom ();
/*  
    	devDomainPtr    pdom;         domain to guard
	rcmProcPtr      pproc;        process to guard for
	ulong   	in_int;	      input interrupt mask, if enter disabled
	ulong   	flags;        flags to indicate alternative action 
*/
#define GUARD_ONLY	    0	      /* only guard the domain */
#define GUARD_LOCK	    1	      /* domain lock the domain */
#define GUARD_DEV_LOCK	    2	      /* device lock the domain */
#define GUARD_DISABLED	    4	      /* enter with interrupts disabled */
#define GUARD_NO_ENABLE	    8	      /* enter with interrupts disabled,
					 and do not enable on exit */

extern void unguard_dom ();
/*  
    	devDomainPtr    pdom;
	ulong   	flags;
*/
#define UNGUARD_ONLY	    0	      /* only unguard the domain */
#define UNGUARD_UNLOCK	    1	      /* domain unlock the domain */
#define UNGUARD_DEV_UNLOCK  2	      /* device unlock the domain */

#endif
