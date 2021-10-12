/* @(#)54	1.26.1.14  src/bos/kernel/db/POWER/dbfunc.h, sysdb, bos411, 9439C411a 9/30/94 12:58:12 */
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 83
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
 * LEVEL 1,  5 Years Bull Confidential Information
*/

/* declarations for all the functions that are called by driver */
int     Help(),alter(),Back(),Break(),display_watch_brat_break(),drivers();
int     Clear(),Display(),Go(), Fmtu(),fmts(),Find(),display_map();
int     Loop(),Next(),Origin(),Quit(),reset_var();
int     Restore(),Screen(),Set(),Sregs();
int     St(),Stc(),Step(),Sth(),Swap();
int     fpregs(),list_vars(),Xlate(),Proc();
int     trbdb(), tty_dump(), trace_disp();
int     pr_vmm(), Bdisplay(), Brat(), Watch(), Udisplay(), Reason();
int     mblk(), fmodsw(), dmodsw(), Stream(), strqueue(), buckets();
int	prtcpcb(), prmbuf(), prinpcb(), showsock(), prndd();
#ifdef _POWER_MP
int     Switch(), Cpu(), LBreak(), Ppd();
#endif /* #ifdef _POWER_MP */
#ifdef _THREADS
int     Thread(), Fmtut(), stack_traceback();
#endif /* #ifdef _THREADS */
int	Sysinfo();

int     fpregs();
#ifdef DEBUG
int	bpr();
#endif /* #ifdef DEBUG */

