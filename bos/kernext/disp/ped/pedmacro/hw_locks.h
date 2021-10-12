/* @(#)98       1.1  src/bos/kernext/disp/ped/pedmacro/hw_locks.h, pedmacro, bos411, 9428A410j 3/24/94 13:59:32 */

/*
 * COMPONENT_NAME: PEDMACRO
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_MID_HW_LOCKS
#define _H_MID_HW_LOCKS

#include <mid/mid.h>

/*---------------------------------------------------------------------------*/
/* The macros in this module protect the PCB and Indirect addressing         */
/* resources against contention between the multiple users.  Access to the   */
/* PCB is serialized by using fast domain locks.  Access to the Indirect     */
/* Registers is searialized by using the fast domain locks in user space     */
/* and by saving and restoring the state of the Indirect registers with      */
/* interrupts disabled in kernel space.                                      */
/*---------------------------------------------------------------------------*/

#ifndef MID_DD

#define LOCK_USER_ACCESS(lockptr)                                             \
	lockptr->DomainLocked = TRUE;

#define UNLOCK_USER_ACCESS(lockptr,domain_num)                                \
{                                                                             \
	lockptr->DomainLocked = FALSE;                                        \
	if (lockptr->TimeSliceExpired)                                        \
	{                                                                     \
	        give_up_timeslice  notify;                                    \
	                                                                      \
	        notify.error  = 0;                                            \
	        notify.domain = domain_num;                                   \
	        aixgsc(MID_GSC_HANDLE,GIVE_UP_TIMESLICE,&notify);             \
	}                                                                     \
}

#else /* MID_DD */

#if 0 /* Set to 0 for debug until MID_DD locks are working, Set to 1 to ship */

#define  Bool  unsigned /* Required by midksr.h */

#include <keytab.h>     /* define kdd_data for vt.h */
#include <sys/hft.h>    /* define COLORPAL, FONTPAL & keystroke for vt.h */
#include <vt.h>         /* define KSR_MODE */
#include <sys/lockl.h>
#include <mid/hw_io_kproc.h>
#include "midddf.h"             /* define MID_INTR_PRIORITY */
#include "midksr.h"             /* define pIo_kproc */

#define MID_PCB_LOCKWORD        (MID_DDF->io_Q.pcb_stack.pcb_ind_lockword)
#define MID_IND_LOCKWORD        (MID_DDF->io_Q.pcb_stack.pcb_ind_lockword)
#define MID_DD_MOM              (MID_DDF->io_Q.flags & MID_IOK_MOM)

#define MID_CALLBACK            (MID_DDF->pdev->devHead.pCom->rcm_callback)
#define MID_DOMAIN_PTR(dom_num) (&(MID_DDF->pdev->domain[dom_num]))
#define MID_RCM_KPPROC                                                        \
	((struct middata *)(MID_DDF->phys_disp->visible_vt->vttld))->pIo_kproc

#endif /* 0 */


/*---------------------------------------------------------------------------*/
/* The following macros are used by the device driver to determine if        */
/* access to the PCB and Indirect Addressing domains are locked.  The type   */
/* of test depends on whether the adapter is in KSR mode or Monitor mode.    */
/*                                                                           */
/* If indirect access is locked, then it is necessary for the device driver  */
/* to save and restore the Indirect Addressing registers when using them.    */
/* In KSR mode, only the device driver uses Indirect Access, and use of      */
/* these registers is protected by disabling interrupts.  Consequently,      */
/* the test for a locked domain always returns false, since it is never      */
/* necessary to save and restore these registers.  In Monitor mode,          */
/* Indirect Access may be locked by non-kernel users, so it is necessary     */
/* to test for a guarded domain.                                             */
/*                                                                           */
/* If pcb access is locked, then it is necessary for some kernel users of    */
/* the pcb to stack the pcb commands, rather than wait for the pcb to        */
/* become available.  In KSR mode, only the device driver uses the PCB, so   */
/* a conventional lock is tested to determine if the PCB is busy.  In        */
/* monitor mode, PCB access may be locked by non-kernel users, so an         */
/* additional test for a guarded domain is necessary.                        */
/*---------------------------------------------------------------------------*/

#if 0 /* Set to 0 for debug until MID_DD locks are working, Set to 1 to ship */

#define INDIRECT_ACCESS_IS_LOCKED                                            \
	((MID_DD_MOM) ?                                                      \
	   ((*(MID_CALLBACK->guard_dom_sleep_test))                          \
	        (MID_DOMAIN_PTR(MID_IND_DOMAIN), MID_RCM_KPPROC) != FALSE) : \
	   FALSE)


#ifdef  MID_INTR

#define PCB_ACCESS_IS_NOT_LOCKED                                              \
	((MID_PCB_LOCKWORD == LOCK_AVAIL) &&                                  \
	((MID_DD_MOM) ?                                                       \
	   ((*(MID_CALLBACK->guard_dom_sleep_test))                           \
	        (MID_DOMAIN_PTR(MID_IND_DOMAIN), MID_RCM_KPPROC) == FALSE) :  \
	   TRUE))


#endif

#else /* For debug purposes until MID_DD locks are working */

#define INDIRECT_ACCESS_IS_LOCKED       (TRUE)
#define PCB_ACCESS_IS_NOT_LOCKED        (FALSE)

#endif

/*---------------------------------------------------------------------------*/
/* These macros protect kernel access to the PCB.  Different protection      */
/* mechanisms are used depending upon whether the adapter is in KSR mode     */
/* or Monitor mode.  In KSR mode, only the kernel should be accesing the     */
/* PCB, and therefore conventional locks are used to prevent contentions     */
/* between multiple kernel processes.  In Monitor mode, both kernel          */
/* processes and user processes may access the PCB, and therefore fast       */
/* domain locks are also used to prevent contention between the users.       */
/*                                                                           */
/* Prerequisites:                                                            */
/*   The global variables used in these macros are defined in HWPDDFSetup.   */
/*---------------------------------------------------------------------------*/

#define LOCK_KERNEL_ACCESS(lockword,domain_num)                               \
{                                                                             \
	while (lockl(&(lockword),LOCK_SIGRET) != LOCK_SUCC) ;                 \
	                                                                      \
	_mid_lock_mode = MID_DD_MOM ;                                         \
	if (_mid_lock_mode)                                                   \
	        (*(MID_CALLBACK->guard_domain))                               \
	               (MID_DOMAIN_PTR(domain_num), MID_RCM_KPPROC) ;         \
}

#define UNLOCK_KERNEL_ACCESS(lockword,domain_num)                             \
{                                                                             \
	if (_mid_lock_mode)                                                   \
	    (*(MID_CALLBACK->unguard_domain))(MID_DOMAIN_PTR(domain_num));    \
	                                                                      \
	unlockl (&(lockword)) ;                                               \
}

/*---------------------------------------------------------------------------*/
/* These macros protect the indirect address registers, if necessary, by     */
/* saving the indirect address and control registers prior to using them     */
/* and restoring the indirect address and control register after using them. */
/* The registers are only saved and restored if the corresponding fast       */
/* domain lock is set indicating that they are currently in use.             */
/*                                                                           */
/* These macros disable interrupts prior to saving the registers and         */
/* enable interrupts after restoreing the registers.  This prevents any      */
/* other user from changing the value of these registers until the current   */
/* user is done with them.  Consequently, it is VERY important that any      */
/* save is followed by a restore.                                            */
/*                                                                           */
/* There is a BIM bug whereby if the indirect control register is written    */
/* and then read back within 60 microseconds, the value read back may not    */
/* equal the value written.  This bug does not occur if any other BIM        */
/* registers are read or written between the write and the read for the      */
/* indirect control register.  Since it is possible for the following macro  */
/* which reads the indirect control register to quickly follow a write of    */
/* the indirect control register (due to interrupts or process switching),   */
/* we read the indirect address register before reading the indirect         */
/* control register.  Since interrupts are disabled, this prevents a write   */
/* of the indirect control register from being directly followed by a read   */
/* and thus prevents the BIM bug from occurring.                             */
/*                                                                           */
/* When restoring the indirect address and control registers, if the control */
/* register is programmed for a read with auto increment, the first          */
/* word of data will get fetched and the address will be incremented at      */
/* the time the address is loaded.  Therefore, the address which is restored */
/* must be one less than the value actually read.  Also, we must restore     */
/* the indirect control register before the indirect address register so     */
/* that by knowing the value of the indirect control register we can         */
/* determine if the indirect address value needs to be decremented.  This    */
/* order is also good for preventing the above BIM bug, because it keeps the */
/* write of the indirect control register from being followed directly by a  */
/* read.                                                                     */
/*                                                                           */
/* Prerequisites:                                                            */
/*   The global variables used in these macros are defined in HWPDDFSetup.   */
/*---------------------------------------------------------------------------*/

#define SAVE_INDIRECT_REGISTERS                                               \
{                                                                             \
	_mid_ind_old_priority = i_disable(MID_INTR_PRIORITY(MID_DDF) );       \
	_mid_ind_access_is_locked = INDIRECT_ACCESS_IS_LOCKED;                \
	if (_mid_ind_access_is_locked)                                        \
	{                                                                     \
	        _MID_RD_VALUE( MID_PTR_IND_ADDRESS, _mid_ind_save_adr )       \
	        _MID_RD_VALUE( MID_PTR_IND_CONTROL, _mid_ind_save_ctl )       \
	        if ((_mid_ind_save_ctl & MID_IND_MODE_MASK) == MID_IND_RD_AI) \
	                _mid_ind_save_adr--;                                  \
	}                                                                     \
}

#define RESTORE_INDIRECT_REGISTERS                                            \
{                                                                             \
	if (_mid_ind_access_is_locked)                                        \
	{                                                                     \
	        _MID_WR_VALUE( MID_PTR_IND_CONTROL, _mid_ind_save_ctl )       \
	        _MID_WR_VALUE( MID_PTR_IND_ADDRESS, _mid_ind_save_adr )       \
	}                                                                     \
	i_enable(_mid_ind_old_priority);                                      \
}

#endif /* MID_DD */

/*---------------------------------------------------------------------------*/
/* The following defines determine what protection each type of user has     */
/* for the PCB and Indirect Addressing domain.                               */
/*                                                                           */
/* The MID_CDD define is for the common character device driver which        */
/* does not need any protection since it is single threaded.                 */
/*                                                                           */
/* The MID_INTR is used to distiguish a module that cannot use a locking     */
/* technique to gain access to the PCB, but must, instead, use the stacking  */
/* mechanism.  This is certainly true for any code in the interrupt handling */
/* path.                                                                     */
/*                                                                           */
/* In addition, we use the MID_INTR define in paths which, for any other     */
/* design reason, it is not desireable to wait for the PCB lock to be        */
/* granted.  A path falling in this category is the font path.  Since, the   */
/* font code (primarily) runs under a device independent kernel process, we  */
/* do not want to make all devices wait for the I/O path of a single Ped to  */
/* become available.                                                         */
/*---------------------------------------------------------------------------*/

#ifdef MID_CDD

#define PROTECT_INDIRECT_ACCESS       /* CDD doesn't need to protect access */
#define UNPROTECT_INDIRECT_ACCESS     /* CDD doesn't need to protect access */
#define PROTECT_PCB_ACCESS            /* CDD doesn't need to protect access */
#define UNPROTECT_PCB_ACCESS          /* CDD doesn't need to protect access */

#elif MID_INTR

#ifdef MID_DD
#define PROTECT_INDIRECT_ACCESS       SAVE_INDIRECT_REGISTERS
#define UNPROTECT_INDIRECT_ACCESS     RESTORE_INDIRECT_REGISTERS
#define PROTECT_PCB_ACCESS            /* Not needed if MID_INTR defined */
#define UNPROTECT_PCB_ACCESS          /* Not needed if MID_INTR defined */
#endif MID_DD

#elif MID_DD

#define PROTECT_INDIRECT_ACCESS       SAVE_INDIRECT_REGISTERS
#define UNPROTECT_INDIRECT_ACCESS     RESTORE_INDIRECT_REGISTERS

#if 0 /* Set to 0 for debug until MID_DD locks are working, Set to 1 to ship */
#define PROTECT_PCB_ACCESS                                                    \
	LOCK_KERNEL_ACCESS( MID_PCB_LOCKWORD, MID_PCB_DOMAIN )
#define UNPROTECT_PCB_ACCESS                                                  \
	UNLOCK_KERNEL_ACCESS( MID_PCB_LOCKWORD, MID_PCB_DOMAIN )
#else /* For debug purposes until MID_DD locks are working */
#define PROTECT_PCB_ACCESS                                                    \
	SAVE_INDIRECT_REGISTERS
#define UNPROTECT_PCB_ACCESS                                                  \
	_MID_WAIT_PCB_SPACE( 8 )                                              \
	RESTORE_INDIRECT_REGISTERS
#endif

#elif MID_TED

#define PROTECT_INDIRECT_ACCESS       /* TED doesn't need to protect access */
#define UNPROTECT_INDIRECT_ACCESS     /* TED doesn't need to protect access */
#define PROTECT_PCB_ACCESS            /* TED doesn't need to protect access */
#define UNPROTECT_PCB_ACCESS          /* TED doesn't need to protect access */

#else /* not MID_CDD and not MID_INTR and not MID_DD and not MID_TED */

#define PROTECT_INDIRECT_ACCESS   LOCK_USER_ACCESS(MID_FDL_PTR)
#define UNPROTECT_INDIRECT_ACCESS UNLOCK_USER_ACCESS(MID_FDL_PTR,MID_IND_DOMAIN)
#define PROTECT_PCB_ACCESS        LOCK_USER_ACCESS(MID_FDL_PTR)
#define UNPROTECT_PCB_ACCESS      UNLOCK_USER_ACCESS(MID_FDL_PTR,MID_PCB_DOMAIN)

#endif /* not MID_CDD and not MID_INTR and not MID_DD and not MID_TED */

#endif /* _H_MID_HW_LOCKS */
