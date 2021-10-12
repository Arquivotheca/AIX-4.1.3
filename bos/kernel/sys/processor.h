/* @(#)48	1.2  src/bos/kernel/sys/processor.h, sysml, bos41J, 9507A 1/31/95 12:26:53 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PROCESSOR
#define _H_PROCESSOR

typedef short cpu_t;            /* logical processor ID               */
typedef short processor_t;      /* physical processor ID              */
/*
 * Bind a process/thread to a processor 
 */
extern int bindprocessor(int What, int Who, cpu_t Where); 
/*
 * Values for What
 */
#define BINDPROCESS  1    /* Bind all threads in process Who        */
#define BINDTHREAD   2    /* Only bind thread Who                   */
/*
 * Values for Where
 */
#define PROCESSOR_CLASS_ANY ((cpu_t)(-1))  /* Unbound               */
/*
 * The next one is  T E M P O R A R I L Y !
 */
extern cpu_t mycpu(void);
/*
 * Kernel internal
 */
#ifdef _KERNSYS
extern int switch_cpu(cpu_t Where, int Options);
#define SET_PROCESSOR_ID   1
#define RESET_PROCESSOR_ID 2
#define MP_MASTER          0
#endif /* _KERNSYS */
#endif /* _H_PROCESSOR */
