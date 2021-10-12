/* @(#)45   1.7  src/bos/kernext/cie/devstats.h, sysxcie, bos411, 9428A410j 4/18/94 16:21:32 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   setHWM
 * 
 * DESCRIPTION:
 * 
 *    Device Statistics
 * 
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

#if !defined(DEVSTATS_H)
#define  DEVSTATS_H

/*---------------------------------------------------------------------------*/
/*                             Device Statistics                             */
/*---------------------------------------------------------------------------*/

typedef struct DEVSTATS      DEVSTATS;

struct DEVSTATS
{
   int                       sesMax;      // Maximum number of sessions
   int                       rdqMax;      // Max Entries on Read Queue
   int                       rdqOvfl;     // Read Queue Overflows
   int                       stqMax;      // Max Entries on Status Queue
   int                       stqOvfl;     // StatusQueue Overflows
};

/*---------------------------------------------------------------------------*/
/*                  Set a high-watermark counter atomically                  */
/*---------------------------------------------------------------------------*/

#define  setHWM(stat,var)                                             \
{                                                                     \
   int   hwmold = stat;                                               \
   while((var) > hwmold && !compare_and_swap(&(stat),&hwmold,(var))); \
}

#endif

