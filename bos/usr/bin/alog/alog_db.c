static char sccsid[] = "@(#)42        1.8  src/bos/usr/bin/alog/alog_db.c, cmdalog, bos411, 9428A410j 3/17/94 15:28:25";

/*
 *   COMPONENT_NAME: CMDALOG
 *
 *   FUNCTIONS: validate_type
 *		output_attr
 *		get_dbitem
 *		chg_dbitem
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Routines for alog database access				*/

#include	"alog.h"
#include	<errlg/SWservAt.h>
#include	<sys/access.h>

extern int result;
extern char c_flag;

/* 
 *--------------------------------------------------------------------
 *	validate_type() - Routine to validate log type
 *		This routine will do a lookup in the database
 *		for all attributes of alog_type and return
 *		TRUE or FALSE depending on the validity of the
 *		type passed to the routine.
 *--------------------------------------------------------------------
*/

validate_type(type)
char	*type;
{
struct  SWservAt *swservp;		/* pointer to list of SWservAt objects  */
struct  SWservAt *origp;		/* pointer to hold the original value of swservp */
int	valid_type=FALSE;		/* Valid type flag, set to FALSE        */
int	found;				/* Number of items found	        */
int	i;

if (odm_initialize() != -1) 
   {
   odm_set_path("/etc/objrepos");	
   swservp = ras_getattr("alog_type",TRUE,&found);
   if (swservp == (struct SWservAt *)NULL)
      return(valid_type);
	
   origp = swservp;	
   
   /* Check if valid type entered  */
   for(i=0;i<found && !valid_type;i++)
      if(strcmp(type,swservp->value) == 0)
         valid_type = TRUE;
      else
	 swservp++;

   free(origp);		/* Clean up space malloc by ras_getattr */

   odm_terminate();
   }  /* end if */

return(valid_type);
}  /* end validate_type */
				
/* 
 *--------------------------------------------------------------------
   output_attr() - Routine to list attributes.  This routine takes
	a type and ...
        if type is NULL
           list the alog types that are supported
        else
           list the attributes for that alog type
  
 *--------------------------------------------------------------------
*/
int output_attr(type,valid_type)
char	*type;
int	valid_type;
{
struct  SWservAt	*swservp1;	/* pointer to SWservAt objects  */
struct  SWservAt	*swservp2;	/* pointer to SWservAt objects  */
struct  SWservAt	*swservp3;	/* pointer to SWservAt objects  */
struct  SWservAt	*origp;		/* pointer to hold original value of swservp1 */
struct  stat		*statbuf;	/* used to see if SWservAt is there */
int	attr_buf1[256];			/* Buffer for log_name argument */
int	attr_buf2[256];			/* Buffer for log_size argument */
int	attr_buf3[256];			/* Buffer for log_vebosity argument */
int	found;				/* Number of items found	    */
int	i;

/* we know that we're going to access ODM, so try it here */
if ((result=odm_initialize()) != -1) 
   {
   odm_set_path("/etc/objrepos");
   if(type == NULL)  /* used -L without specifying a type */
      {
      swservp1 = ras_getattr("alog_type",TRUE,&found);
      origp = swservp1;
      if (swservp1 == (struct SWservAt *)NULL)
     	 if (access("/etc/objrepos/SWservAt", R_ACC) != 0) 
		{
		fprintf(stderr,MSGSTR(SWSERV_NO_ACCESS,"alog: Unable to access \
the ODM object class SWservAt.\n\
Possible causes:\n\
1.  The ODM object class SWservAt does not exist.\n\
2.  The ODM object class SWservAt does not have read permissions.\n"));
	 	set_result(3);   /* can't access SWservAt */ 
		}
         else
		set_result(0);   /* no alog_types, so return 0 */ 
      else
         for(i=0; i<found; i++)  /* output the types found */
            {
            printf("%s\n",swservp1->value);
	    swservp1++;
            }  /* end for */
      free(origp);	/* Clean up space malloc by ras_getattr */
      }  /* end if */
   else  /* used -L and specified a type */
      {
      /* Build the search attribute strings */
      sprintf(attr_buf1,"%s%s",type,"_logname"); /* Construct value */
      sprintf(attr_buf2,"%s%s",type,"_logsize"); /* Construct value */
      sprintf(attr_buf3,"%s%s",type,"_logverb"); /* Construct value */

      /* Get the attributes for the alog_type */
      swservp1 = ras_getattr(attr_buf1,FALSE,&found);
      swservp2 = ras_getattr(attr_buf2,FALSE,&found);
      swservp3 = ras_getattr(attr_buf3,FALSE,&found);

      /* print out the attributes */
      /* We don't care if a value does not exist, SMIT can handle it */
      if (c_flag)
	 {
	 fprintf(stdout,MSGSTR(LIST_ALL,"#type:file:size:verbosity\n"));
	 printf("%s:%s:%s:%s\n",type,swservp1->value,swservp2->value,swservp3->value);
	 }
      else
	 {
         fprintf(stdout,MSGSTR(LIST_HEADERS,"#file:size:verbosity\n"));
         printf("%s:%s:%s\n",swservp1->value,swservp2->value,swservp3->value);
	 }

      free(swservp1);		/* Clean up space malloc by ras_getattr */
      free(swservp2);		/* Clean up space malloc by ras_getattr */
      free(swservp3);		/* Clean up space malloc by ras_getattr */
      set_result(0);         	/* save result for later */ 
      }  /* end else */

   odm_terminate();
   }  /* end if */

if (result != 0)
   set_result(3);

return(result);
}  /* end output_attr */
/* 
 *--------------------------------------------------------------------
   get_dbitem() - Routine to get a database item. (As a character buf)
	This routine takes a type and a suffix and will use this
	information to construct an argument which is an attribute
	name of SWservAt and call ras_getattr to fetch the current value
	of the attribute. This routine allows the encapsulation
	of the lookup mechanism to make the alog main routine
	independent of the lookup method. 

	On succesful db access this routine returns a malloced
	area with the attribute value copied in. On failure this
	routine will return NULL.
 *--------------------------------------------------------------------
*/
char *get_dbitem(type,suffix)
char	*type;
char	*suffix;
{
struct  SWservAt *swservp;	/* pointer to list of SWservAt objects  */
int	found;			/* Number of items found	    */
int	attr_buf[256];		/* Buffer for constructing argument */
char	*answer=NULL;		/* Initialize pointer to the answer */

sprintf(attr_buf,"%s%s",type,suffix);  /* Construct attribute value */

if (odm_initialize() != -1) 		/* Now let's go get it	    */
   {
   odm_set_path("/etc/objrepos");
   swservp = ras_getattr(attr_buf,FALSE,&found);
   /* the attribute is not found in the database */
   if (swservp == (struct SWservAt *)NULL)
      return(answer);

   /* the attribute is there but has no value */
   if (strlen(swservp->value) == 0)
      return(answer);

   answer = malloc(strlen(swservp->value));
   strcpy(answer,swservp->value);

   free(swservp);			/* Clean up space malloc by ras_getattr */

   odm_terminate();
   }  /* end if */

return(answer);
}  /* end get_dbitem */

/* 
 *--------------------------------------------------------------------
   chg_dbitem() - Routine to change attributes.  This routine takes
	a type, an attribute and a new value and changes the
        current value in the database.
 *--------------------------------------------------------------------
*/
int chg_dbitem(type,item,newvalue)
char	*type;
char	*item;
char	*newvalue;
{
int	attr_buf[256];		/* Buffer for constructing argument     */
struct  SWservAt *swservp;	/* pointer to list of SWservAt objects  */
int	found;			/* Number of items found	    	*/
int	lock;			/* Number of items found	    	*/

sprintf(attr_buf,"%s%s",type,item);     /* Construct attribute value */

if (odm_initialize() != -1) 		/* Now let's go get it	    */
   {
   odm_set_path("/etc/objrepos");	
   swservp = ras_getattr(attr_buf,FALSE,&found);
   /* the attribute is not found in the database */
   if (swservp == (struct SWservAt *)NULL)
      {
      fprintf(stderr,MSGSTR(ATTR_NOT_FOUND,"alog: Unable to find \
attribute, %s,\n\
in the ODM object class SWservAt.\n\
Possible cause(s):\n\
1.  The attribute does not exist in the ODM object class SWservAt.\n\
2.  The ODM object class SWservAt is corrupted.\n"),attr_buf);

      set_result(3);                      /* set result for later use */
      }
   else
      {
      swservp->value = newvalue;	 /* put new value in swservp */

      lock=odm_lock("/etc/objrepos/config_lock",ODM_NOWAIT);
      if (lock != -1)
         {
         if(ras_putattr(swservp) !=0)          /* see if ras_putattr worked */
            {
            fprintf(stderr,MSGSTR(UPD8_ATTR_ERR,"alog: Unable to \
update attribute, %s,\n\
in the ODM object class SWservAt.\n\
Possible cause(s):\n\
1.  The user is not a root user.\n"),attr_buf);

            set_result(3);                   /* save result */
            }  /* end if */
         odm_unlock(lock);
         }
      else
         {
         fprintf(stderr,MSGSTR(ERR_LOCK_ODM,"alog: Unable to lock \
ODM.\n"));
         set_result(3);                   /* save result */
        }
      }  /* end else */

   if (result != 3)	
   	savebase();	/* Call savebase to make ODM persistant across boots. */ 
   free(swservp);	/* Clean up space malloc by ras_getattr */
   odm_terminate();
   }  /* end if */
 
return(result);
}  /* end chg_dbitem */
