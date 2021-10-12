/* @(#)04	1.4  src/bos/kernext/audio/acpa/trace.h, sysxacpa, bos411, 9428A410j 11/9/93 11:26:12 */
/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _TRACE_H
#define _TRACE_H 1


#ifdef _DISPLAYTRACEFLAG

	/* _DISPLAYTRACEFLAG is a compile time flag used to select
		whether the trace calls will expand to NULL strings
		or to system trace calls.  Display subsystems trace
		calls should be included between the #ifdef and the
		#else.  Each item defined should have a corresponding
		null definition as well.
	*/
 
/************************************************************************/
/************************************************************************/
/*      M-ACPA/A TRACE MACROS AND DEFINITIONS                           */
/************************************************************************/
/************************************************************************/

/* The following defines list each of the possible subfunctions. */

#define ACPA_MAIN               0
#define ACPA_SUBR_ONE           1
#define ACPA_SUBR_TWO           2
#define ACPA_SUBR_THREE         3
#define ACPA_SUBR_FOUR          4
#define ACPA_SUBR_FIVE          5
#define ACPA_SUBR_SIX           6
#define ACPA_SUBR_SEVEN         7

/* The following defines are for the convenience of programmers.  They  */
/* make for easier typing of macros.                                    */

#define CFGINIT                 hkwd_ACPA_CFGINIT
#define CFGTERM                 hkwd_ACPA_CFGTERM
#define OPEN                    hkwd_ACPA_OPEN
#define CLOSE                   hkwd_ACPA_CLOSE
#define INTERRUPT               hkwd_ACPA_INTERRUPT
#define READ                    hkwd_ACPA_READ
#define WRITE                   hkwd_ACPA_WRITE
#define IOCTL                   hkwd_ACPA_IOCTL
#define MPX                     hkwd_ACPA_MPX
#define SELECT                  hkwd_ACPA_SELECT
#define INIT                    hkwd_ACPA_INIT
#define CHANGE                  hkwd_ACPA_CHANGE
#define START                   hkwd_ACPA_START
#define STOP                    hkwd_ACPA_STOP
#define PAUSE                   hkwd_ACPA_PAUSE
#define RESUME                  hkwd_ACPA_RESUME
#define STATUS                  hkwd_ACPA_STATUS
#define WAIT                    hkwd_ACPA_WAIT
#define BUFFER                  hkwd_ACPA_BUFFER
#define LOAD                    hkwd_ACPA_LOAD
#define DEVSTART                hkwd_ACPA_DEVSTART
#define DEVSTOP                 hkwd_ACPA_DEVSTOP
#define INITZ                   hkwd_ACPA_INITZ
#define TIMEDOUT                hkwd_ACPA_TIMEDOUT

#define ENTER_ACPA_PTRACE( function, subfunction, min ) \
	TRCHKL2T( HKWD_ACPA_P | function | TRACE_ENTRY, subfunction, min )

#define EXIT_ACPA_PTRACE( function, subfunction, min ) \
	TRCHKL2T( HKWD_ACPA_P | function | TRACE_EXIT, subfunction, min )

#define DEBUG_ACPA_PTRACE( function, subfunction, var ) \
	TRCHKL1T( HKWD_ACPA_P | function | TRACE_EXIT, subfunction, var )

/************************************************************************/
/*************************************************************************/
/*************************************************************************/
/**************  END OF DISPLAYTRACEFLAG DEFINITIONS *********************/
/*************************************************************************/ 
/*************************************************************************/
#else
	/* The else statement is used as a compile option to force
	    all of the Display Subsystems trace calls to null strings.
            In this manner, the code source will always hold the trace
	    hooks, but the compiled code may have trace omitted.
	*/

/************************************************************************/
/************************************************************************/
/*      M-ACPA/A TRACE MACROS AND DEFINITIONS                           */
/************************************************************************/
/************************************************************************/


/* The following defines list each of the possible subfunctions. */

#define ACPA_MAIN
#define ACPA_SUBR_ONE
#define ACPA_SUBR_TWO
#define ACPA_SUBR_THREE
#define ACPA_SUBR_FOUR
#define ACPA_SUBR_FIVE
#define ACPA_SUBR_SIX
#define ACPA_SUBR_SEVEN

/* The following defines are for the convenience of programmers.  They  */
/* make for easier typing of macros.                                    */

#define CFGINIT
#define CFGTERM
#define OPEN
#define CLOSE
#define INTERRUPT
#define READ
#define WRITE
#define IOCTL
#define MPX
#define SELECT
#define INIT
#define CHANGE
#define START
#define STOP
#define PAUSE
#define RESUME
#define STATUS
#define WAIT
#define BUFFER
#define LOAD
#define DEVSTART
#define DEVSTOP
#define INITZ
#define TIMEDOUT

#define ENTER_ACPA_PTRACE( function, subfunction, min )

#define EXIT_ACPA_PTRACE( function, subfunction, min )

#endif /* _DISPLAYTRACEFLAG */
 




extern int msgdebug;

#ifdef ACPADEBUG

#define TRACE(f) { if ( msgdebug )   \
    {                                \
      debug_setlocation( __LINE__ ); \
      debug_print f ;                \
    }                                \
}

#define TRACE1(f) { if ( msgdebug > 1 ) \
    {                                   \
      debug_setlocation( __LINE__ );    \
      debug_print f ;                   \
    }                                   \
}

/* This is a short version of trace that doesn't output the line number */
#define TRACES(f) { if ( msgdebug )  \
    {                                \
      debug_prints f ;               \
    }                                \
}

#else ACPADEBUG

#define TRACE(f)
#define TRACE1(f)
#define TRACES(f)

#endif 

#endif 



#ifdef CFGDEBUG
#define DEBUG_0(A)                      {printf(A);}
#define DEBUG_1(A,B)                    {printf(A,B);}
#define DEBUG_2(A,B,C)                  {printf(A,B,C);}
#define DEBUG_3(A,B,C,D)                {printf(A,B,C,D);}
#define DEBUG_4(A,B,C,D,E)              {printf(A,B,C,D,E);}
#define DEBUG_5(A,B,C,D,E,F)            {printf(A,B,C,D,E,F);}
#define DEBUG_6(A,B,C,D,E,F,G)          {printf(A,B,C,D,E,F,G);}
#define DEBUGELSE                       else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUGELSE
#endif

extern int acpabug;

#ifdef ACPABUG

#define BUGPTR(f) { if ( acpabug  )     \
    {                                   \
      debug_setlocation( __LINE__ );    \
      debug_print f ;                   \
    }                                   \
}

#else
#define BUGPTR(f)
#endif
