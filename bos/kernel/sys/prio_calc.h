/* @(#)58	1.11  src/bos/kernel/sys/prio_calc.h, sysproc, bos41J, 9512A_all 3/20/95 14:58:17 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: prio_calc
 *
 *
 *   ORIGINS: 27,83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_PRIO_CALC
#define _H_PRIO_CALC

#include  <sys/proc.h>
#include  <sys/param.h>
#include  <sys/pri.h>
#include  <sys/sched.h>   

extern int sched_R;

/*
 * MACRO prio_calc - re-calculates the the priority of a thread and returns
 *                   the new priority to invoker.
 *                   The priority of a thread is calculated as follows:
 *                     1. if the priority is boosted, then you return the
 *                        current value.
 *                     2. for a thread with a fixed priority:
 *                        new priority = minimum(t_sav_pri,
 *                                               t_wakepri)
 *                     3. for a thread without a fixed priority:
 *                        new priority = minimum((t_cpu*R + p_nice),
 *                                                t_wakepri,
 *                                                PRI_LOW)
 *
 *
 *        Where R is a parameter tunable via the schedtune interface.
 *
 *        INPUT:  (t)  - pointer to thread block whose priority is to be
 *                       re-calculated.
 *
 *        RETURNS: thread's new priority.
 *
 *
 */

#define  prio_calc(t)                                                          \
 (t->t_boosted ? t->t_lockpri :					               \	
 (MIN( (((t)->t_policy == SCHED_OTHER) ?                                       \
            MIN(((((t)->t_cpu*sched_R)>>5) + (t)->t_procp->p_nice), PRI_LOW) : \
            ((t)->t_sav_pri)),                                                 \
       (t)->t_wakepri)))

#endif /* _H_PRIO_CALC */
