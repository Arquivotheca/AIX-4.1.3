static char sccsid[] = "@(#)22  1.7.1.1  src/bos/usr/ccs/lib/libodm/odmerrmsg.c, libodm, bos411, 9430C411a 7/12/94 18:55:05";
/*
 *   COMPONENT_NAME: LIBODM
 *
 *   FUNCTIONS: get_odm_msg
 *		odm_err_msg
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <odmi.h>
#include "odmmsg.h"

 /************************************************************************
 *
 * NAME: odmerrmsg()
 *
 * FUNCTION: Returns message strings for ODM errors.
 *
 * EXECUTION ENVIRONMENT:  This subroutine is part of the ODM library of
 *      of subroutines.  It may be called by ODM applications which have
 *      a need for displaying some sort of message if an ODM subroutine
 *      fails.
 *
 * RETURN VALUE:  returns 0 if successful,
 *                returns -1 if no matching string found for input odmerrno.
 *
 *************************************************************************/

int odm_err_msg ( inputerrno, msg_string )

int inputerrno;                         /* input odmerrno               */
char **msg_string;                      /* address of string pointer in */
					/*   application program.       */
{                                       
   char *get_odm_msg();

   odmerrno = 0;
   /*
      Open the message catalog.
      If the catalog_id returned is -1, then use default messages.
   */
   if (msg_string == NULL)
     {
       odmerrno = ODMI_PARAMS;
       return(-1);
     } /* endif */

   catalog_id = catopen ( "libodm.cat", NL_CAT_LOCALE );  /* Defect 116162 */ 

   /*
      The ODM message catalog is separated into 2 sets:

      1) The first set are the ODM-generated odmerrno's in the
         range 5900 through 5929.

      2) The second set are the ODM-generated odmerrno's in the
         range 5800 through 5819.

      Verify that the input odmerrno is valid before retrieving message.
   */

   if ( ( inputerrno >= FIRST_ODM1_ERRNO ) &&
        ( inputerrno <= LAST_ODM1_ERRNO ) )
   {
      
      *msg_string =
           get_odm_msg ( ODM_SET1, inputerrno - FIRST_ODM1_ERRNO );
   }
   else
   {
      if ( ( inputerrno >= FIRST_ODM2_ERRNO ) &&
           ( inputerrno <= LAST_ODM2_ERRNO ) )
      {
         *msg_string =
              get_odm_msg ( ODM_SET2, inputerrno - FIRST_ODM2_ERRNO );
      }
      else
      {
         *msg_string = "";
      }
   }

   /*
      If no message was found to match the inputerrno, then the
      message string has been set to a null string (""), and
      we will return CATD_ERR.  Otherwise, return ok.
   */

   if ( strcmp ( *msg_string, "" ) == 0 )
    {
      odmerrno = ODMI_PARAMS;
      return ( -1 );
    }
   else
      return ( 0 );
}



 /************************************************************************
 *
 * NAME: get_odm_msg ( set_number, msg_index )
 *
 * FUNCTION: Calls the catgets() to retrieve messages.
 *
 * EXECUTION ENVIRONMENT:  This subroutine is an internal ODM subroutine
 *      called by odmerror() to call the catalog facilities for retrieval.
 *
 * RETURN VALUE:  returns pointer to the message string.
 *
 *************************************************************************/

char *get_odm_msg ( set_number, msg_index )
int set_number;
int msg_index;
{
   /*
      If the catalog_id if valid, then call the catgets()
      to retrieve a message either from the catalog, or use the default.
   */
   int msg_index2;

   if ( catalog_id != CATD_ERR )
   {
      switch ( set_number )
      {
         case ( ODM_SET1 ):
		  /* This hack is used because the libodm.msg 
			used hard-coded message number instead of
			symbolic message number
		  */
                  if ( msg_index == 0 )
		       msg_index2 = 31;
		  else if (msg_index == 31 )
		       msg_index2 = 32;
		  else msg_index2 = msg_index;
                  return ( catgets ( catalog_id,
                                       set_number,
                                       msg_index2,
                                       odm_messages1[msg_index] ) );
         case ( ODM_SET2 ):
		  if ( msg_index == 0 )
		       msg_index2 = 5;
		  else msg_index2 = msg_index;
                  return ( catgets ( catalog_id,
                                       set_number,
                                       msg_index2,
                                       odm_messages2[msg_index] ) );
         default:
                  return ( "" );
      }
   }
   else
   {
      /*
         If the catalog_id is not valid, then just return the
         default message.
      */

      switch ( set_number )
      {
         case ( ODM_SET1 ):
            return ( odm_messages1[msg_index] );
         case ( ODM_SET2 ):
            return ( odm_messages2[msg_index] );
         default:
            return ( "" );
      }
   }

}


