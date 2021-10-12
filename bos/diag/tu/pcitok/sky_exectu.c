static char sccsid[] = "@(#)08	1.2  src/bos/diag/tu/pcitok/sky_exectu.c, tu_pcitok, bos41J, 9516A_all 4/18/95 09:59:09";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: exectu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

/*****************************************************************************
Function(s) Exec TU for HTX Exerciser and Diagnostics

Module Name :  exectu.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:
        extern int tu000();             tu000.c
        extern int tu001();             tu001.c
        extern int tu002();             tu002.c
        extern int tu003();             tu003.c
        extern int tu004();             tu004.c
        extern int tu005();             tu005.c
        extern int tu006();             tu006.c
        extern int tu007();             tu007.c
        extern int tu010();             tu010.c

        error codes and constants are defined in skytu_type.h
                                             and mps_err_codes.h

   DATA AREAS:

   TABLES: none

   MACROS: none

COMPILER / ASSEMBLER

   TYPE, VERSION: AIX C Compiler, version:

   OPTIONS:

NOTES: Nones.
*****************************************************************************/

/*** header files ***/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* local header files */
#include "skytu_type.h"

/*** external files ***/
extern int tu000();
extern int tu001();
extern int tu002();
extern int tu003();
extern int tu004();
extern int tu005();
extern int tu006();
extern int tu007();
extern int tu010();


/*****************************************************************************
Function: int exectu()--Exec TU for HTX Exerciser and Diagnostics.

Function called by both the Hardware Exercise, Manufacturing Application,
and Diagnostic Application to invoke a Test Unit (TU).

If the "mfg" member of the struct within TUTYPE is set to INVOKED_BY_HTX,
then exectu() is being invoked by the HTX Hardware Exerciser so
test units know to look at variables in TUTYPE for values from the
rule file.  Else, the test units use pre-defined values while testing.
*****************************************************************************/
int exectu (char *adapter_name, TUTYPE *tucb_ptr)
{
  long            i, loop, tu;
  int             rc = 0;
  int             counter;
  ADAPTER_STRUCT  *adapter_info;

  tu = tucb_ptr->header.tu;
  loop = tucb_ptr->header.loop;
  tucb_ptr->header.loop_remain = loop;
  if (tucb_ptr->header.adapter_info == NULL)
  {
  	tucb_ptr->header.adapter_info = (ADAPTER_STRUCT *) 
		malloc(sizeof(ADAPTER_STRUCT));
  }

  adapter_info = tucb_ptr->header.adapter_info;
#ifdef BEST
  adapter_info->best_orw = tucb_ptr->header.best_orw;
  adapter_info->buf_addr = tucb_ptr->header.buf_addr;
#endif

  for (i = 1; i <= loop; i++, (tucb_ptr->header.loop_remain)--) {
  	switch(tu) {
		case TU_OPEN:       
			rc = tu000 (adapter_name, tucb_ptr, adapter_info);
			DEBUG_0("TU_OPEN ");
			break;
		case  CFG_TEST:
			rc = tu001 (adapter_info, tucb_ptr);
			DEBUG_0("CFG_TEST ");
			break;
		case  IO_TEST:      
			rc = tu002 (adapter_info, tucb_ptr);
			DEBUG_0("IO_TEST ");
			break;

		case  ONCARD_TEST:  
			rc = tu003 (adapter_info, tucb_ptr);
			DEBUG_0("ONCARD_TEST ");
			break;

		case  CONNECT_TEST: 
			rc = tu004 (adapter_info, tucb_ptr);
			DEBUG_0("CONNECT_TEST ");
			break;
		case  INT_WRAP_TEST: 
			rc = tu005 (adapter_info, tucb_ptr);
			DEBUG_0("INT_WRAP_TEST ");
			break;

		case  EXT_WRAP_TEST: 
			rc = tu006 (adapter_info, tucb_ptr);
			DEBUG_0("EXT_WRAP_TEST ");
			break;

		case  NETWORK_TEST:  
			rc = tu007 (adapter_info, tucb_ptr);
			DEBUG_0("NETWORK_TEST ");
			break;
		case  TU_CLOSE:     
			rc = tu010 (adapter_info, tucb_ptr);
			DEBUG_0("TU_CLOSE ");
			break;

		default:            
			DEBUG_0("Invalid tu number.\n");
			rc = ILLEGAL_TU_ERR;
			break;
  	}; /* end of case */

  	if (rc) {
  		DEBUG_0("TU failed\n");
  		break; /* TU failed.  get out of the loop. */
  	}

  } /* end for */

  return(rc);

} /* end exectu.c */
