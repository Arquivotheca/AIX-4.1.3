/* static char sccsid[] = src/bos/kernext/disp/ccm/inc/ccmdd_tools.h, dispccm, bos411, 9428A410j 4/22/94 12:08:57 */

/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************
 COMPONENT_NAME: 	(sysx/disp/ccm)       

 MODULE NAME:		ccmdd_tools.h
 
 TITLE:  		Common Character Mode Video Device Driver
 	 		MACROS and TRACE aids

 FUNCTIONS:     	none

 ORIGINS:        	27

 ------------------------------------------------------------------------	

 IBM CONFIDENTIAL -- (IBM Confidential Restricted when combined with
                      the aggregate modules for this product)

                  SOURCE MATERIALS

 (C) COPYRIGHT International Business Machines Corp. 1992, 1994

 Unpublished Work
 All Rights Reserved
 Licensed Material - Property of IBM

 US Government Users Restricted Rights -- Use, duplication, or 
 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


 ---------------------------------------------------------------------------

 PURPOSE:	Defines some macros and trace aids useful for the CCM
		VDD.

 
 INPUTS:  	n/a

 OUTPUTS: 	n/a

 FUNCTIONS/SERVICES CALLED:	n/a 

 DATA:		

 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/





#ifndef __H_CCMDD_TOOLS
#define __H_CCMDD_TOOLS

#include	<sys/syspest.h>
#include 	<sys/trchkid.h>
#include	"ccm_trchkid.h"
#include	<sys/disptrc.h>
#include	<sys/except.h>

	/*=============================================
	| CLEAN UP SOME UGLY DEFINES THAT RESIDE IN
	| <sys/disptrc.h>
	|==============================================*/

#undef		OPEN
#undef		CLOSE
#undef		INTERRUPT
#undef		READ
#undef		WRITE
#undef		IOCTL
#undef		MPX
#undef		SELECT
#undef		INIT
#undef		CHANGE
#undef		START
#undef		STOP
#undef		PAUSE
#undef		RESUME
#undef		STATUS
#undef		WAIT
#undef		BUFFER
#undef		LOAD
#undef		TIMEOUT


/*===================================================================
|
|	MACROS used to simplify <sys/syspest.h>
|
|====================================================================*/

	/*------------------------------------------
	| having a __SYSPEST_LVL gives us a chance to
	| compile in different behaviors by setting the
	| level with a compile time -D__SYSPEST_LVL
	|------------------------------------------*/

#ifndef	__SYSPEST_LVL
#define	__SYSPEST_LVL	0
#endif

#ifdef	DEBUG
	/*----------------------------------------
	| PUBLIC INTERFACE
	|
	| redefine BUGXDEF and BUGVDEF in order to
	| use __SYSPEST_LVL and in order to use the
	| correct variable name in the simplified
	| printing tools
	|-----------------------------------------*/


#define	BUG_INIT					\
		int *	__BUG_NAME;

#define	BUG_USE( name )					\
		__BUG_NAME = &(name);

#define	BUG_XDEF( name )				\
		BUGXDEF( name )				\
		extern int *	__BUG_NAME = &(name);

#define	BUG_VDEF( name )				\
		BUGVDEF( name , __SYSPEST_LVL )		\
		extern int *	__BUG_NAME = &(name);

#define	BUG_V( name )					\
		BUGVDEF( name , __SYSPEST_LVL )

#define	BUG_X( name )					\
		BUGXDEF( name )

#define	BUG_CMNT( s1 )	BUGLPR( (* __BUG_NAME ), BUGACT, ("%s\n", s1 ))
#define	BUG_ENTRY	BUG_CMNT("=====ENTRY=====")
#define	BUG_EXIT	BUG_CMNT("=====EXIT=====")

#define	BUG_SUBR_ENTRY( s1 )				\
	BUGLPR( (* __BUG_NAME ), BUGACT, ("=%s=%s\n",\
		s1 , "=====ENTRY=====" ))

#define	BUG_SUBR_EXIT( s1 )				\
	BUGLPR( (* __BUG_NAME ), BUGACT, ("=%s=%s\n",\
		s1 , "=====EXIT=====" ))

#define	BUG_VAR( var, ptype )				\
			BUGLVT( (* __BUG_NAME ), BUGNTX, var, ptype )

#define	BUG_CVAR( s1, var, ptype )			\
			BUGLRT( (* __BUG_NAME ), BUGNTX, s1, var, ptype )

#define	BUG_BRKPT( p1 )					\
		BUGLFUNCT( (* __BUG_NAME), BUGNTX, 0, 	\
			   brkpoint( (p1) )		\
			 );

#else

#define	BUG_INIT
#define	BUG_USE(arg1)
#define	BUG_XDEF(arg1)
#define	BUG_VDEF(arg1)
#define	BUG_V(arg1)
#define	BUG_X(arg1)
#define	BUG_CMNT(arg1)
#define	BUG_ENTRY
#define	BUG_EXIT
#define	BUG_SUBR_ENTRY(arg1)
#define	BUG_SUBR_EXIT(arg1)
#define	BUG_VAR(arg1, arg2)
#define	BUG_CVAR(arg1, arg2, arg3)
#define	BUG_BRKPT(arg1)

#endif


#ifdef	_DISPLAYTRACEFLAG
/*========================================================================
|
|	MACROS used to control system trace
|
|========================================================================*/

#define	TRACE_INSIDE	TRACE_ENTRY

	/*==========================================================
	|  VDD MACROS
	|===========================================================*/

		/*---------------------------------
		| for use only inside this module!!
		|----------------------------------*/

#define	CCM_VDD_hkwd( proc )		hkwd_DISPLAY_VTSS_VDD_##proc
#define CCM_VDD_hkwd_( s1 )		s1##_

#define	CCM_VDD_hkwd_func(  proc, func )				\
			CCM_VDD_hkwd_func1( proc, _##func )

#define	CCM_VDD_hkwd_func1( proc, func1 )				\
			CCM_VDD_hkwd_func2( proc##func1 )

#define	CCM_VDD_hkwd_func2( s1 )	hkwd_func_DISPLAY_VTSS_VDD_##s1

#define	CCM_VDD_DW_0	CCM_DDS_device_num( CCMDD_dds )
#define	CCM_VDD_DW_1	CCM_DDS_config_display_id( CCMDD_dds )

#define	CCM_VDD_HKWD( proc, ex, func )					\
	HKWD(	HKWD_DISPLAY_VTSS_P,					\
		CCM_VDD_hkwd(proc) ,					\
		CCM_VDD_hkwd_func(proc,func),				\
		TRACE_##ex						\
	    )

		/*----------------------------------------------
		| PUBLIC INTERFACE
		|
		| macros used by other *.c and *.h modules
		|-----------------------------------------------*/

#define	CCM_VDD_TRACE( proc, ex , func )				\
	{TRCHKL2T(							\
		CCM_VDD_HKWD( proc, ex, func ),				\
		CCM_VDD_DW_0,						\
		CCM_VDD_DW_1						\
		);}

#define	CCM_VDD_INTR_TRACE( ex , pd )					\
	{TRCHKL1T(							\
		CCM_VDD_HKWD( INTERRUPT, ex, MAIN ),			\
		(pd)							\
		);}

	/*==========================================================
	|  CDD MACROS (Video ROM Scan)
	|===========================================================*/

		/*---------------------------------
		| for use only inside this module!!
		|----------------------------------*/

#define	CCM_CDD_DW_0		CCMDD_cdd 

#define	CCM_CDD_INTR_DW_0( pd )	CCM_INTR_head( pd )

#define	CCM_CDD_HKWD( proc, ex )					\
	HKWD(	HKWD_DISPLAY_VTSS_P,					\
		hkwd_DISPLAY_VTSS_CDD_ENTRY_KMOD ,			\
		hkwd_func_DISPLAY_VTSS_CDD_ENTRY_##proc ,		\
		TRACE_##ex						\
	    )

#define	CCM_CDD_INTR_HKWD( ex )						\
	HKWD(	HKWD_DISPLAY_VTSS_P,					\
		hkwd_DISPLAY_VTSS_CDD_INTERRUPT ,			\
		0 ,							\
		TRACE_##ex						\
	    )

		/*----------------------------------------------
		| PUBLIC INTERFACE
		|
		| macros used by other *.c and *.h modules
		|-----------------------------------------------*/

#define	CCM_CDD_TRACE( proc, ex )					\
	{TRCHKL1T(							\
		CCM_CDD_HKWD( proc, ex ),				\
		CCM_CDD_DW_0						\
		);}

#define	CCM_CDD_INTR_TRACE(  ex , pd )					\
	{TRCHKL1T(							\
		CCM_CDD_INTR_HKWD( ex ),				\
		CCM_CDD_INTR_DW_0( pd )					\
		);}

#else

/*===================================================================
|
|	NULL DEFINITIONS OF TRACE 
|
|	NEGATE ONLY THE PUBLIC INTERFACE!!
|
|	(When _DISPLAYTRACEFLAG is not defined)
|
|====================================================================*/

#define	CCM_VDD_TRACE( a , b, c )
#define	CCM_VDD_INTR_TRACE( a , b )
#define CCM_CDD_TRACE( a , b )
#define CCM_CDD_INTR_TRACE( a , b )

#endif
/*=====================================
| END OF TRACE MACROS
|======================================*/


/*=====================================================================
|
|	MACROS USED IN DEVICE DRIVER PROGRAMMING
|
|=====================================================================*/

/*----------------------------------------------------------------------------*
   This following is copied from the MID device driver.
   CCM_BREAK (parm1, parm2, parm3, parm4, parm5) 

   Breakpoint when CCM_BREAK is comiled ON.  Else a NOP.

 *----------------------------------------------------------------------------*/

#ifdef 	CCM_BREAK_SWITCH
#define 	CCM_BREAK(p1, p2, p3, p4, p5)  		\
{								\
       brkpoint (p1,p2,p3,p4,p5); \
	   							\
}

#else  
#define 	CCM_BREAK(p1, p2, p3, p4, p5)  		
#endif



/*----------------------------------------------------------------------------*
   CCM_ASSERT (condition, parm1, parm2, parm3, parm4, parm5) 

   Tests the condition, if FALSE, we break. 
   Breakpoint when CCM_ASSERT_SWITCH is comiled ON.  Else a NOP.
   Right now this is at least when DEBUG is ON. 

 *----------------------------------------------------------------------------*/

#ifdef 	CCM_ASSERT_SWITCH
#define 	CCM_ASSERT(condition, p1, p2, p3, p4, p5) 	\
{						\
	if (!(condition))  			\
	{					\
		CCM_BREAK(p1, p2, p3, p4, p5) 	\
	}			\
}

#else 
#define 	CCM_ASSERT(condition, p1, p2, p3, p4, p5)  		\
		ASSERT( condition);
#endif


		/*----------------------------------------------
		| PUBLIC INTERFACE
		|
		| macros used by other *.c and *.h modules
		|-----------------------------------------------*/
#undef	MIN
#define	MIN( a , b )	( ((a) < (b)) ? (a) : (b) )

#define	CCM_EXCEPTION_TRAP( rc )					\
{									\
	struct except 		_e;					\
	int			_i;					\
									\
	getexcept( &_e );						\
	for ( _i = 0; _i<5; _i++ )					\
	{								\
		BUG_CVAR("except structure" , _e.except[ _i ], x );	\
	}								\
}

#define	CCM_ERROR_CHECK( rc , vp , proc )				\
{									\
if ( (rc) != 0 )							\
{									\
    if ( (rc) == E_CDD_BAD_HW_OP )					\
    {									\
		( (ccm_ksr_data_t *) ((vp)->vttld))->assume_hw_is_bad = TRUE;	\
    }									\
    else								\
    {									\
		CCM_VDD_TRACE( proc, EXIT, ERROR )				\
		BUG_EXIT							\
    }									\
	return ( (rc) );						\
}									\
}


#define	CCM_SUBR_ERROR_CHECK( rc , vp , s1 )				\
{									\
if ( (rc) != 0 )							\
{									\
    if ( (rc) == E_CDD_BAD_HW_OP )					\
    {									\
		( (ccm_ksr_data_t *) ((vp)->vttld))->assume_hw_is_bad = TRUE;	\
		BUGLPR( (* __BUG_NAME ), BUGACT, ("=%s=%s\n", s1 , "==CDD IO ERROR===" ));				\
    }									\
    else								\
    {									\
		BUGLPR( (* __BUG_NAME ), BUGACT, ("=%s=%s%d\n", s1 , "==BAD RC FOUND==  " , (rc) ));			\
		BUG_SUBR_EXIT( s1 )						\
    }									\
	return ( (rc) );						\
}									\
}

#define	CCM_CHECK_CDD_RC( rc , vp  )					\
{									\
    switch (rc)								\
    {									\
    case E_CDD_PASS:							\
									\
	(rc) = 0;							\
	break;								\
									\
    case E_CDD_IO_FAILED:						\
    case E_CDD_IO_EXCEPTION:						\
    case E_CDD_BAD_HW_OP:						\
    									\
	(rc) = E_CDD_BAD_HW_OP;						\
	((ccm_ksr_data_t *)((vp)->vttld)) -> assume_hw_is_bad = TRUE;	\
	break;								\
									\
    default:								\
									\
	CCM_ASSERT( ( (rc) == E_CDD_PASS ), rc, 0,0,0,0 );				\
	break;								\
    }									\
}



#define	CCM_GLYPH_ADDR( charcode , ld )				\
	/*	char *		*/					\
	( ld->glyph_ptr 						\
	            + (((ld->char_ptr)[ (charcode) ]).byteOffset) )

#define	_CCM_PSE_OFFSET( row, col, ld )					\
	( ( ( (row) - 1 ) * ( (ld)->ps_size.wd ) ) + ( (col) -1 ) )

#define	_CCM_PSE_OFFSET_SCROLLED( offset , ld )				\
	( (offset) % ( (ld)->ps_words ) )

#define	CCM_PSE_CHAR( row, col, ld )					\
	/* 	ulong		*/					\
	(*( ( (ulong *) (ld)->pse) + ( _CCM_PSE_OFFSET_SCROLLED( _CCM_PSE_OFFSET( row, col, ld) , ld ))))

#define	CCM_PSE_CHAR_CODE( ps_char )					\
	( ((ps_char) >> 16) & 0x00FF )


	/*=====================================
	| macros used for I/O control with the 
	| CDD services
	| 
	| NOTES;  The macros DO NOT WORK at the
	|	  INTERRUPT LEVEL because they
	| 	  access non-pinned code and data 
	|=====================================*/

#ifndef	CCMDD_IO

#define	CCM_IOCC_DET( b )		IOCC_DET( b )
#define	CCM_BUSMEM_DET( b )		BUSMEM_DET( b )

#define	CCM_IOCC_ATT( s , p )		IOCC_ATT( s , p )
#define	CCM_BUSMEM_ATT( s , p )		io_att(s, p);

#define	CCM_IO_STATE_VAR		/* nothing */
#define	CCM_IO_STATE_SAVE		/* nothing */
#define CCM_IO_STATE_RESTORE( bool )	/* nothing */

#else	/* CCMDD_IO was defined */

#define	CCM_IOCC_DET( b )							\
	{	if ( CCMDD_fast_io == FALSE )					\
		{								\
			IOCC_DET( b );						\
		}								\
	}

#define	CCM_BUSMEM_DET( b )							\
	{	if ( CCMDD_fast_io == FALSE )					\
		{								\
			BUSMEM_DET( b );					\
		}								\
	}

#define	CCM_IOCC_ATT( s , p )							\
	( (CCMDD_fast_io) 							\
	?	( (ulong) ( CCMDD_iocc_att | ( CDD_ADDR_MASK & (p) )))		\
	:	( (ulong) (IOCC_ATT( (s) , (p) )))				\
	)

#define	CCM_BUSMEM_ATT( s , p )							\
	( (CCMDD_fast_io)							\
	?	( (ulong) (CCMDD_busmem_att | ( CDD_ADDR_MASK & (p) ) ))	\
	:	( (ulong) (io_att(s, p)) )					\
	)		


#define CCM_IO_STATE_VAR	ulong	_fast_io; ulong _iocc_att; ulong _busmem_att;

#define	CCM_IO_STATE_SAVE							\
	{	_fast_io	= CCMDD_fast_io;				\
		_iocc_att	= CCMDD_iocc_att;				\
		_busmem_att	= CCMDD_busmem_att;				\
		CCMDD_iocc_att	= IOCC_ATT( (CDD_iocc_seg(CCMDD_cdd)), 0 ); 	\
		CCMDD_busmem_att= io_att( (CDD_busmem_seg(CCMDD_cdd)), 0 );	\
		CCMDD_fast_io	= TRUE;						\
	}

#define	CCM_IO_STATE_RESTORE( bool )					\
	{	if ( (bool) )						\
		{							\
			IOCC_DET( CCMDD_iocc_att );			\
			BUSMEM_DET( CCMDD_busmem_att );			\
			CCMDD_iocc_att	= _iocc_att;			\
			CCMDD_busmem_att = _busmem_att;			\
		 	CCMDD_fast_io	= _fast_io;			\
		}							\
	}

#endif	/* CCMDD_IO */

/*=======================================================================
|
| 	define macros for accessing char_box data
|
|========================================================================*/


#define CCM_DEFAULT_FONT	0

#define CCM_FONT_PTR ((aixFontInfoPtr)(font_table[font_index].font_ptr))

#define CCM_DEF_PTR  ((aixFontInfoPtr)(font_table[CCM_DEFAULT_FONT].font_ptr))

#endif		/* __H_CCMDD_TOOLS */
