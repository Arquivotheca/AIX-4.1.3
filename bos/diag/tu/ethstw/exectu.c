static char sccsid[] = "@(#)exectu.c    1.9 6/26/91 12:20:09";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: int exectu();
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Function(s) Exec TU for HTX Exerciser and Diagnostics

Module Name :  exectu.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:	
       	extern int tu001();		tu001.c
       	extern int tu002();             tu002.c
       	extern int tu003();             tu003.c
       	extern int tu004();             tu004.c
       	extern int tu005();             tu005.c
       	extern int tu006();             tu006.c
       	extern int tu007();             tu007.c
       	extern int tu008();             tu008.c
       	extern int tu009();             tu009.c
       	extern int tu00A();             tu00A.c
       	extern int tu00B();             tu00B.c
       	extern int tu00C();             tu00C.c
	extern int tu00D();             tu00D.c
        extern int tu00E();             tu00E.c
        extern int tu00F();             tu00F.c
        extern int tu010();             tu010.c
        extern int tu011();             tu011.c

	error codes and constants are defined in tu_type.h 

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
#include <fcntl.h>
#include <errno.h>
#include "tu_type.h"

/*** external files ***/
        extern int tu001();
        extern int tu002();
        extern int tu003();
        extern int tu004();
        extern int tu005();
        extern int tu006();
        extern int tu007();
        extern int tu008();
        extern int tu009();
        extern int tu00A();
        extern int tu00B();
        extern int tu00C();
        extern int tu00D();
        extern int tu00E();
        extern int tu00F();
        extern int tu010();
        extern int tu011();

/*****************************************************************************
Function: int exectu()--Exec TU for HTX Exerciser and Diagnostics.

Function called by both the Hardware Exercise, Manufacturing Application,
and Diagnostic Application to invoke a Test Unit (TU).

If the "mfg" member of the struct within TUTYPE is set to INVOKED_BY_HTX,
then exectu() is being invoked by the HTX Hardware Exerciser so
test units know to look at variables in TUTYPE for values from the
rule file.  Else, the test units use pre-defined values while testing.
*****************************************************************************/
int exectu (int fdes, TUTYPE *tucb_ptr)
   {
	long i, loop, tu;
	int rc = 0;
	int counter;

	tu = tucb_ptr->header.tu;
	loop = tucb_ptr->header.loop;
	tucb_ptr->ethi.loop_remain = loop;

	/* set up netid */
	tucb_ptr->ethi.netid = 0xbeef;

	for (i = 1; i <= loop; i++, (tucb_ptr->ethi.loop_remain)--)
	   {
	   switch(tu)
		{
	  	case  0x01:
		   rc = tu001 (fdes, tucb_ptr);
		   break;

  	   	case  0x02:  
		   rc = tu002 (fdes, tucb_ptr);
		   break;

	   	case  0x03:
		   rc = tu003 (fdes, tucb_ptr);
		   break;

	   	case  0x04:
		   rc = tu004 (fdes, tucb_ptr);
		   break;

	      	case  0x05:
		   rc = tu005 (fdes, tucb_ptr);
		   break;

	   	case  0x06:
		   rc = tu006 (fdes, tucb_ptr);
		   break;

	   	case  0x07:
		   rc = tu007 (fdes, tucb_ptr);
		   break;

  	  	case  0x08:
		   rc = tu008 (fdes, tucb_ptr);
		   break;

	   	case  0x09:
		   rc = tu009 (fdes, tucb_ptr);
		   break;

   		case 0x0A:
		   rc = tu00A (fdes, tucb_ptr);
		   break;

	   	case 0x0B:
		   rc = tu00B (fdes, tucb_ptr);
		   break;

	   	case 0x0C:
		   rc = tu00C (fdes, tucb_ptr);
		   break;

                case  0x0D:
                   rc = tu00D (fdes, tucb_ptr);
                   break;

                case  0x0E:
                   rc = tu00E (fdes, tucb_ptr);
                   break;

                case  0x0F:
                   rc = tu00F (fdes, tucb_ptr);
                   break;

                case  0x10:
                   rc = tu010 (fdes, tucb_ptr);
                   break;

                case  0x11:
                   rc = tu011 (fdes, tucb_ptr);
                   break;

	   	default:
		   PRINT((tucb_ptr->msg_file,"Invalid tu number.\n")); 
		   rc = (ILLEGAL_TU_ERR);
		   break;
	  	}; /* end of case */
	   if (rc && rc!=THICK && rc!=THIN && rc!=TWISTED_PAIR)
	      {
	      PRINT((tucb_ptr->msg_file,"TU %x failed\n",tu));
	      break; /* TU failed.  get out of the loop. */
	      }
	   } /* end for */

	return(rc);
   } /* end exectu.c */
