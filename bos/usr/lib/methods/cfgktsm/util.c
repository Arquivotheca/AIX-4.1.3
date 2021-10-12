static char sccsid[] = "@(#)54	1.1  src/bos/usr/lib/methods/cfgktsm/util.c, inputdd, bos41J, 9509A_all 2/14/95 12:56:22";
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS:
 *      defchild
 *		gettok
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cfgktsm.h"

/*
 * NAME     : defchild
 *
 * FUNCTION : This function defines child attached to the adapter
 *
 * RETURNS : 0
 * 	
 */

int defchild(char *lname, char *uniquetype, char *connection)
{
	struct PdDv	pdobj;	         /* Predefined object for device */
	struct CuDv	cdobj;	         /* Customized object for device */
	char   sstring[100];         /* search criteria string       */
	int    rrc = 0, rc;

	DEBUG_0( "\nenter defchild\n" );

	/* continue only if both specified */
    if (*uniquetype && *connection)  {

	/* get customized object for device */
	  sprintf( sstring,
	    "PdDvLn = %s AND parent = %s AND connwhere = %s",
	    uniquetype, lname, connection );

	  DEBUG_1("Reading from CuDv where %s\n", sstring )

	  rc = (int) odm_get_obj( CuDv_CLASS, sstring, &cdobj, ODM_FIRST);
	  if( rc == -1 ) {
	    DEBUG_0("odm_get_obj failed\n")
	    rrc = E_ODMGET;
	  }
	  else {

	  /* device not defined, run method */
	    if( rc == 0 ) { 
	      sprintf( sstring, "uniquetype = %s", uniquetype);
	      DEBUG_1( "Device not defined, search PdDv with %s\n",sstring )
	      rc=(int)odm_get_obj(PdDv_CLASS, sstring, &pdobj , ODM_FIRST);
	      if( rc == -1 ) {			
	        DEBUG_0("odm_get_obj failed\n")
	        rrc = E_ODMGET;
	      }
	      else {
	        if( rc == 0 ) {
	          DEBUG_0("odm_get_obj: no PdDv obj found\n")
	        }
            else {
	          sprintf( sstring, "-c %s -s %s -t %s -p %s -w %s",
	          pdobj.class, pdobj.subclass,
	          pdobj.type, lname, connection );

	          DEBUG_2( "Running %s %s\n", pdobj.Define, sstring )
	          if( odm_run_method( pdobj.Define, sstring, NULL, NULL )) {
	            DEBUG_2( "Can't run %s %s\n", pdobj.Define, sstring )
	          }
	        }
	      }
	    } 

	    /* Device has already been defined */
	    else {
	      /* update change status              */
	      if (cdobj.status == DEFINED) {
	        if (cdobj.chgstatus == MISSING) {
	          cdobj.chgstatus = SAME;
	          if (odm_change_obj(CuDv_CLASS,&cdobj) == -1) {
	            DEBUG_0("Error updating change status.\n");
	          }
	        }
	      }
	      /* output logical name of child so cfgmgr will config the device */
	      printf("%s ", cdobj.name );
	    }
  	  }
	}
	return(rrc);
}

/*
 * NAME     : gettok
 *
 * FUNCTION : parse space separated tokens
 *
 * RETURNS : pointer to start of token
 * 
 */

char *gettok(char **p)
{
	char *rc;

	while(**p == ' ') (*p)++;            /* strip leading blanks  */
	rc = *p;                             /* start of token        */
	while(**p && **p != ' ') (*p)++;     /* skip to end of token  */
	if (**p) {                           /* null terminate token  */
	  **p = 0;
	  (*p)++;
	}

	DEBUG_1("gettok: %s\n", rc);
	return(rc);
}
