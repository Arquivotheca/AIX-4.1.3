/* @(#)04       1.7  src/bos/kernext/disp/inc/disptrc.h, sysxdisp, bos411, 9428A410j 3/7/94 15:29:08 */
/*
 * COMPONENT_NAME: (sysxdisp) 
 *
 * FUNCTIONS: header file for system trace hookwords
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#ifndef _H_DISPLAY_TRACE
#define _H_DISPLAY_TRACE
 
#ifdef _DISPLAYTRACEFLAG

	/* _DISPLAYTRACEFLAG is a compile time flag used to select
		whether the trace calls will expand to NULL strings
		or to system trace calls.  Display subsystems trace
		calls should be included between the #ifdef and the
		#else.  Each item defined should have a corresponding
		null definition as well.
	*/
 
/***********************************************************************/
/*  First thing to do is to include the standard trace include files,
    in which were previously entered the definitions for the Display Subsystem
    area.  These three files must be included in the order listed and
    must all be included.
*/
/***********************************************************************/
 
#include <sys/trcctl.h>          /* 1/st */
#include <sys/trcmacros.h>       /* 2/nd */
#include <sys/trchkid.h>         /* 3/rd */
 
 
/***********************************************************************/
/*  Next thing to do is to define some symbols that are common to all
    of the display subsystems components' trace needs.
*/
/***********************************************************************/
 
#define  TRACE_ENTRY    0
#define  TRACE_EXIT     1
 
/***********************************************************************/
/*   Now we define macros which are common to all of the display subsystems'
     components for purposes of trace.
*/
/***********************************************************************/
 
#define	HKWD( hkid,procid,funcid,ex ) ( hkid | procid | (funcid << 1) | ex )
#define	DEVID(vp)	(*(int *)&( vp->display->disp_devid[0] ))
 
/************************************************************************/
/*	VDD TRACE MACROS AND DEFINITIONS				*/
/************************************************************************/
/************************************************************************/

#define	VDD_HKWD( proc, ex	)			\
	HKWD(	HKWD_DISPLAY_VTSS_P,			\
		hkwd_DISPLAY_VTSS_VDD_ ## proc,		\
		0, TRACE_ ## ex)	

#define	VDD_TRACE( proc, ex, vp	)				\
	TRCHKL1T(						\
		VDD_HKWD( proc, ex	),			\
		DEVID( vp )					\
		)
		
 
/************************************************************************/
/************************************************************************/
/*	GAI 2DM1 TRACE MACROS AND DEFINITIONS				*/
/************************************************************************/
/************************************************************************/
 
 
/* The following defines list each of the supported display adapters.   */
 
#define SK_COLOR        1
#define SK_MONO         2
#define SK_COMMON       3
 
/* The following defines are for the convenience of programmers.  They  */
/* make for easier typing of macros.                                    */
 
 
#define ENTER_PTRACE( function, display ) \
	TRCHKL1T( HKWD_DISPLAY_GAI2DM1_P | function | TRACE_ENTRY, \
	display )
 
#define EXIT_PTRACE( function, display ) \
	TRCHKL1T( HKWD_DISPLAY_GAI2DM1_P | function | TRACE_EXIT, \
	display )
 
 

 
/************************************************************************/
/************************************************************************/
/*	X11 TRACE MACROS AND DEFINITIONS				*/
/************************************************************************/
/************************************************************************/


/* The following defines and macros are for the use of X programmers.   */
 
#define SERVERWAIT              0x100           /* Wait function code   */
#define SERVERREAD              0x101           /* Read function code   */
 
#define ENTER_XWAIT_PTRACE \
	TRCHKL0T( HKWD_DISPLAY_X11_P | ( SERVERWAIT << 4 ) | TRACE_ENTRY )
 
#define EXIT_XWAIT_PTRACE \
	TRCHKL0T( HKWD_DISPLAY_X11_P | ( SERVERWAIT << 4 ) | TRACE_EXIT )
 
#define ENTER_XREAD_PTRACE( client ) \
	TRCHKL1T( HKWD_DISPLAY_X11_P | ( SERVERREAD << 4 ) | TRACE_ENTRY, client )
 
#define EXIT_XREAD_PTRACE( client ) \
	TRCHKL1T( HKWD_DISPLAY_X11_P | ( SERVERREAD << 4 ) | TRACE_EXIT, client )
 
#define ENTER_XFUNCTION_PTRACE( request ) \
	TRCHKL0T( HKWD_DISPLAY_X11_P | ( request << 4 ) | TRACE_ENTRY )
 
#define EXIT_XFUNCTION_PTRACE( request ) \
	TRCHKL0T( HKWD_DISPLAY_X11_P | ( request << 4 ) | TRACE_EXIT )


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

 
#define TRACE_ENTRY 
#define TRACE_EXIT 
#define	HKWD( hkid,procid,funcid,ex ) 

 
/************************************************************************/
/*	VDD TRACE MACROS AND DEFINITIONS				*/
/************************************************************************/


#define	VDD_HKWD( proc, ex	)

#define	VDD_TRACE( proc, ex, vp	)


		
/************************************************************************/
/*	GAI 2DM1 TRACE MACROS AND DEFINITIONS				*/
/************************************************************************/
 
#define SK_COLOR
#define SK_MONO
#define SK_COMMON
 
#define ENTER_PTRACE( function, display )
#define EXIT_PTRACE( function, display )
 
/************************************************************************/
/************************************************************************/
/*      X11 TRACE MACROS AND DEFINITIONS                                */
/************************************************************************/
/************************************************************************/

#define SERVERWAIT
#define SERVERREAD
 
#define ENTER_XWAIT_PTRACE
#define EXIT_XWAIT_PTRACE
#define ENTER_XREAD_PTRACE( client )
#define EXIT_XREAD_PTRACE( client )
#define ENTER_XFUNCTION_PTRACE( request )
#define EXIT_XFUNCTION_PTRACE( request )
 
#endif /* _DISPLAYTRACEFLAG */
 
#endif /* _H_DISPLAY_TRACE */
 
