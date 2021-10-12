#ifndef lint
static char sccsid[] = "@(#)75 1.2 src/bos/usr/lib/methods/chgtty/chgttyutil.c, cfgtty, bos41J, 9524I_all 6/16/95 10:36:35";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) Change Method for streams based tty or serial printers
 *
 * FUNCTIONS: check_parms
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/device.h>
#include <string.h>
#include <cf.h>        /* Error codes */
#include <errno.h>
#include <fcntl.h>
#include <sys/termio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/termiox.h>

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "cfgdebug.h"
#include "ttycfg.h"
#include "pparms.h"

/*
 * ==============================================================================
 * Defines and extern variables
 * ==============================================================================
 */

#define LOGIN_ATT   "login"
#define TERM_ATT    "term"
#define AUTO_ATT    "autoconfig"
#define LOGGER_ATT  "logger"
#define TIMEOUT_ATT "timeout"
#define LOGMOD_ATT  "logmodes" 

#define NULL_STRING         ""

extern int Pflag;          /* Flag to change db only */


/*
 * =============================================================================
 *                       CHECK_PARMS
 * =============================================================================
 * 
 * This function checks the attributes to be changed for the
 * aynchronous ports.
 * 
 * This function operates as a device dependent subroutine called 
 * by the generic change method for all devices. It is used to 
 * check the attributes to be changed for the async ports.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */
int check_parms(attrs,ppflag,tflag,cus_obj,par_cus_obj,parent,location,badlist)
struct attr * attrs;
int    ppflag, tflag;
struct CuDv * cus_obj; /*customized CuDv device pointer */
struct CuDv * par_cus_obj; /*parent customized CuDv device pointer */
char * parent; /* did parent change */
char * location; /* did location change */
char * badlist;
{
    extern char allattrs[];    /* attribute string for attrval() */
    extern int mattrval();      /* attribute values are in range ? */
    extern int set_inittab();     

    char    sstring[50];       /* search string */
    char    loginattr[256];    /* login attribute */	
    char *  badattr;           /* ptr to list of invalid attributes */

    int     attrs_present;
    int     other_attrs;
    int     return_code;       /* return code */
    register int j;


    /* ================================================= */
    /* Validate attributes with mattrval() routine       */
    /* ================================================= */
    if (allattrs[0] != NULL) 
    {

        DEBUG_2 ("chgtty: Calling mattrval for %s with %s.\n",
                        cus_obj->PdDvLn_Lvalue, allattrs);

        if (mattrval(cus_obj->PdDvLn_Lvalue,par_cus_obj->PdDvLn_Lvalue,
						 allattrs, &badattr) > 0) 
	{
            DEBUG_0 ("chgtty: Attrval failed.\n");
            strcpy(badlist, badattr);
            return E_ATTRVAL;
        }
        DEBUG_0 ("chgtty: Attrval finished OK.\n");
        
		
    };  /* End if */

    /* ======================================================== */
    /* If the device is being moved, then we want to force the  */
    /* device to go through the unconfigure, update database,   */
    /* configure device sequence.                               */
    /* Otherwise we'll update the AVAILABLE device directly     */
    /* below and set the "change db only" flag.                 */
    /* ======================================================== */
    if ( (parent != NULL) || (location != NULL) ) {

        /* The tty is being moved, so we need to make sure      */
        /* that a correct minor number gets generated for it.   */
        /* If the device is not AVAILABLE, then we need to      */
        /* manually delete the existing devno in the dbase.     */
        /* Devices that are AVAILABLE will have their devnos    */
        /* released when they are unconfigured.                 */

        if (cus_obj->status != AVAILABLE) {
            sprintf(sstring,"resource = devno and value3 = %s",
                    cus_obj->name);
            if (odm_rm_obj(CuDvDr_CLASS,sstring) < 0) {
                DEBUG_2("odm_rm_obj failed, crit=%s, odmerrno=%d\n",
                         sstring, odmerrno);
                return(E_ODMDELETE);
            };
        };
        return (0);
    };
    
    
    /* =========================================================*/
    /* If the following attributes are selected, and no other   */
    /* attributes are selected, then set the Pflag and          */
    /* update database and inittab only:                        */
    /* LOGIN_ATT, TERM_ATT, AUTO_ATT LOGGER_ATT TIMEOUT_ATT     */
    /* LOGMOD_ATT.                                              */
    /* =========================================================*/

    if (!strncmp(cus_obj->PdDvLn_Lvalue, TTY_PDDV_CLASS,strlen(TTY_PDDV_CLASS))
          && (cus_obj->status != DEFINED))


    {
        attrs_present = FALSE;  /*initialize*/
        other_attrs = FALSE; 
        loginattr[0] = NULL;

        while (attrs->attribute != (char *) NULL) {

               if ( !strncmp(attrs->attribute,TERM_ATT,strlen(TERM_ATT))
                    || !strncmp(attrs->attribute,LOGIN_ATT,strlen(LOGIN_ATT))
                    || !strncmp(attrs->attribute,AUTO_ATT,strlen(AUTO_ATT))
                    || !strncmp(attrs->attribute,LOGGER_ATT,strlen(LOGGER_ATT))
                    || !strncmp(attrs->attribute,LOGMOD_ATT,strlen(LOGMOD_ATT))
                    || !strncmp(attrs->attribute,TIMEOUT_ATT,
                                                      strlen(TIMEOUT_ATT))) 
               {

		      if (!strncmp(attrs->attribute,LOGIN_ATT, 
                                                      strlen(LOGIN_ATT)))
                      {	
			    strcpy(loginattr,attrs->value);
		      }
                      attrs_present = TRUE; 
                      DEBUG_1("check_parms: attrs_present =  %d \n",
                                                           attrs_present);
               }
               else {
                     other_attrs = TRUE;
                     DEBUG_1("check_parms: other_attrs =  %d \n", other_attrs);
                     break;
               } 
               attrs++;       
        } /* end while */ 


         /*if no other attributes found, then set Pflag                */    
         /*otherwise let continue and return E_BUSY in the unconfigure */    
         if (attrs_present &&  !other_attrs ) {
		  /* Update the 'inittab' file if needed */
                  if (loginattr[0] != NULL)
		  {
		        if (return_code = set_inittab(cus_obj, &loginattr,
                                                              RUNTIME_CFG)) 
                        {
		            DEBUG_0("change_tty: set_inittab failed\n");
 		            return(return_code);
		        }
                  }
                  Pflag = 1;   /* change ODM only flag */		
         }
    }/* end if (!strncmp(cus_obj.... */

    return(0);
}
