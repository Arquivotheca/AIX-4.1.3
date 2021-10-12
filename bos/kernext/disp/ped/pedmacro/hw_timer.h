/* @(#)82       1.2  src/bos/kernext/disp/ped/pedmacro/hw_timer.h, pedmacro, bos411, 9428A410j 3/17/93 19:27:18 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************/
/*                                                              */
/*      PEDERNALES HW MACRO PROGRAMMING INTERFACE               */
/*                                                              */
/****************************************************************/


#ifndef _H_MID_HW_TIMER
#define _H_MID_HW_TIMER

/*-------------------------------------------------------------*/
/*                                                             */
/*  These routines uses the system clock.  To use them, you    */
/*  must compile and link in the assembly language routine     */
/*  "hw_timer.s" (in the library) with the cc command.         */
/*                                                             */
/*-------------------------------------------------------------*/
#ifdef MID_TED

#define MID_NAP_MSEC( delay_time )                              \
{                                                               \
        double  t1;                                             \
        double  t2;                                             \
                                                                \
        MID_TIME_MSEC( t1 )                                     \
        t1 += delay_time;                                       \
        do                                                      \
        {                                                       \
                MID_TIME_MSEC( t2 )                             \
        }                                                       \
        while(t2 < t1);                                         \
}

#define MID_NAP_NSEC( delay_time )                              \
{                                                               \
        double  t1;                                             \
        double  t2;                                             \
                                                                \
        MID_TIME_NSEC( t1 )                                     \
        t1 += delay_time;                                       \
        do                                                      \
        {                                                       \
                MID_TIME_NSEC( t2 )                             \
        }                                                       \
        while(t2 < t1);                                         \
}

#define MID_TIMES_SEC( value, tu, tl )                          \
{                                                               \
                                                                \
        tu = rtc_upper();                                       \
        tl = rtc_lower();                                       \
                                                                \
        value = ((double)tu + (double)tl / 1000000000.0);       \
}

#define MID_TIME_MSEC( value )                                  \
{                                                               \
        double  tl;                                             \
        double  tu;                                             \
                                                                \
        tu = rtc_upper();                                       \
        tl = rtc_lower();                                       \
                                                                \
        value = (tu * 1000) + (tl / 1000000);                   \
}

#define MID_TIME_NSEC( value )                                  \
{                                                               \
        double  tl;                                             \
        double  tu;                                             \
                                                                \
        tu = rtc_upper();                                       \
        tl = rtc_lower();                                       \
                                                                \
        value = (tu * 1000000000) + tl;                         \
}

#else

#define MID_NAP_MSEC( delay_time )

#define MID_NAP_NSEC( delay_time )

#define MID_TIMES_SEC( value, tu, tl )

#define MID_TIME_MSEC( value )

#define MID_TIME_NSEC( value )

#endif  /* MID_TED */

#endif  /* _H_MID_HW_TIMER */
