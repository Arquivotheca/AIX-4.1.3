static char sccsid[] = "@(#)48  1.3  src/bos/diag/tu/ethstw/st_eth.c, tu_ethi_stw, bos411, 9428A410j 10/7/92 07:29:42";
/*****************************************************************************
 * COMPONENT_NAME: tu_ethi_stw 
 *
 * FUNCTIONS: 	int start_eth();
 *		int halt_eth();
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
Function(s) Start Ethernet Adapter

Module Name :  st_eth.c

STATUS:

DEPENDENCIES: none

RESTRICTIONS: none

EXTERNAL REFERENCES

   OTHER ROUTINES:

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
#include <memory.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#include <errno.h>
#include "tu_type.h"	/* note that this also includes hxihtx.h */

/*** constants use in this file ***/
#define MAX_START_ATTEMPTS      16
#define MAX_HALT_ATTEMPTS      16
#define SLEEP_TIME               1

/*****************************************************************************
Function: int start_eth()

Function starts the Ethernet Adapter
IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to the situation
*****************************************************************************/
int start_eth (int fdes, TUTYPE *tucb_ptr)
   {
	int i, rc = 0;

	struct session_blk session;
	struct status_block stat_s;

	session.netid = tucb_ptr->ethi.netid;
	session.length= 2;

	/*************************************************
	 * start up the adapter with the session block
	 * PREVIOUSLY initialized (pointed to by session).
	 *************************************************/

	if (rc = ioctl(fdes, CIO_START, &session) < 0)
	   {
	   PRINT((tucb_ptr->msg_file,"start_eth: rc = %d\n",rc));
	   PRINT((tucb_ptr->msg_file,"start_eth: errno = %d\n",errno));
	   PRINT((tucb_ptr->msg_file,"start_eth: CIO_START failed.\n"));
	   PRINT((tucb_ptr->msg_file,"start_eth: status = %ld\n",session.status));
	   return (START_ERR);
	   }

	for (i = 0; i < MAX_START_ATTEMPTS; i++)
	   {
	   /*************************************************
	    * insure that adapter started properly AND
	    * grab the network address from the status block
	    *************************************************/

	   if (rc = ioctl(fdes, CIO_GET_STAT, &stat_s) < 0)
	      {
              PRINT((tucb_ptr->msg_file, "start_eth: CIO_GET_STAT failed.\n"));
	      PRINT((tucb_ptr->msg_file,"start_eth: errno = %d\n",errno));
	      PRINT((tucb_ptr->msg_file,"start_eth: rc = %d\n",rc));
	      return (GET_STAT_ERR);
	      }

	   if ((stat_s.code & 0x0000ffff) == CIO_START_DONE)
	      break;
	   } /* end for loop */
		
   	if (i == MAX_START_ATTEMPTS)
 	   return (START_TIME_ERR);

   	if (stat_s.option[0] != CIO_OK)
	  {
	  PRINT((tucb_ptr->msg_file,"start_eth: status = %ld\n",session.status));
	  PRINT((tucb_ptr->msg_file,"start_eth: option[0] = %ld\n",stat_s.option[0]));
	  return (START_BSTAT_ERR);
	  }

	return (0);
   } /* end start_eth () */

/*****************************************************************************
Function(s) Halt Ethernet Adapter

Function halts the Ethernet Adapter
IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to the situation
*****************************************************************************/
int halt_eth (int fdes, TUTYPE *tucb_ptr)
   {
	int i, rc = 0;

	struct session_blk session;
	struct status_block stat_s;

	session.netid = tucb_ptr->ethi.netid;
        session.length= 2;

	/*************************************************
	 * halt up the adapter with the session block
	 * PREVIOUSLY initialized (pointed to by session).
	 *************************************************/

	if (rc = ioctl(fdes, CIO_HALT, &session) < 0)
	  {
	  PRINT((tucb_ptr->msg_file,"halt_eth: rc = %d\n",rc));
	  PRINT((tucb_ptr->msg_file,"halt_eth: errno = %d\n",errno));
	  PRINT((tucb_ptr->msg_file,"halt_eth: CIO_HALT failed\n"));
	  PRINT((tucb_ptr->msg_file,"halt_eth: status = %ld\n",session.status));
	  return (HALT_ERR);
	  }

	for (i = 0; i < MAX_START_ATTEMPTS; i++)
	   {
	   /*************************************************
	    * insure that adapter halted properly AND
	    * grab the network address from the status block
	    *************************************************/

	   if (rc = ioctl(fdes, CIO_GET_STAT, &stat_s) < 0)
	      {
              PRINT((tucb_ptr->msg_file, "halt_eth: CIO_GET_STAT failed.\n"));
	      PRINT((tucb_ptr->msg_file,"halt_eth: rc = %d\n",rc));
	      PRINT((tucb_ptr->msg_file,"halt_eth: errno = %d\n",errno));
	      return (GET_STAT_ERR);
	      }

	   if ((stat_s.code & 0x0000ffff) == CIO_HALT_DONE)
	      break;
	   } /* end for loop */
		
	if (i == MAX_START_ATTEMPTS)
	   return (HALT_TIME_ERR);

	if (stat_s.option[0] != CIO_OK)
	  {
	  PRINT((tucb_ptr->msg_file,"halt_eth: status = %ld\n",session.status));
	  PRINT((tucb_ptr->msg_file,"start_eth: option[0] = %ld\n",stat_s.option[0]));
	  return (HALT_BSTAT_ERR);
	  }

	return (0);
   } /* end halt_eth () */

