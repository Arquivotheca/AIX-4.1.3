static char sccsid[] = "@(#)401.1 src/bos/usr/lpp/bosinst/rda/pindex.c, bosinst, bos411, 9428A410j 91/07/30 21:14:48";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System installation
 *
 * FUNCTIONS: make_dev_index, make_attr_index, line_type, get_stanza_name,
 *            get_attr
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
 */
/*********************************************************************
 * NAME:	pindex.c                                             *
 *                                                                   *
 * FUNCTION: 	Parse stanza files containing old configuration      *
 *              information and fill device and attribute structures *
 *                                                                   *
 * EXECUTION ENVIRONMENT:                                            *
 *                                                                   *
 *              The structures created here are used by dback to     *
 *              determine device configuration and attributes.       *
 *                                                                   *
 *********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <cf.h>
#include   <sys/cfgdb.h>
#include   <sys/cfgodm.h>
#include <sys/sysmacros.h>
#include <sys/device.h>


#include	"dback.h"

/*----------------------------------------------------------------------------
   make_dev_index()  - This routine scans the stanza file and constructs
		   an array of pointers to device stanza structures.

                     RETURNS: Number of pointers to device structure

  ---------------------------------------------------------------------------*/

int make_dev_index(FILE *fd, struct dev_struct *devices[])
{
int			lt, num_of_devs;/* number of devices in array index */
struct dev_struct	*curtop;	/* Pointer to current top rec	*/
struct dev_struct	*newone;	/* Pointer to new record	*/
char			buf[2048];	/* Working buffer		*/
char			aval[256];	/* Temp buffer for attr value	*/
char			anam[256];	/* Temp buffer for attr name	*/

rewind(fd);				/* Go to top of file		*/
num_of_devs = -1;

while(!feof(fd))
   {
   if(fgets(buf,256,fd) == NULL)
	   continue;
					/* Check for a stanza name	*/
	lt = line_type(buf);
    switch(lt)
	{
	case L_STANZA:
	    DEBUG_2("line = %s\tline_type = %d\n", buf, L_STANZA);
	    /* Take off the colon from the end of the name */
            get_stanza_name(buf);
					/* Get a new index record	 */
	    newone = (struct dev_struct *) malloc(sizeof(struct dev_struct));
					/* Fill in the name		*/
	    strcpy(newone->name,buf);
	    strcpy(newone->real_name,buf);
					/* Put it in the index		*/
	    devices[++num_of_devs] = newone;
	    break;
	case    L_ATTR:
	    DEBUG_2("line = %s\tline_type = %d\n", buf, L_ATTR);
	    if(get_attr(buf,anam,aval))
		{
		DEBUG_3("buf = %s anam = %s aval = %d\n", buf, anam, aval[0]);
		continue;
		}
	    if (strcmp(anam,"location") == 0)
			strcpy(devices[num_of_devs]->location,aval);
	    else if (strcmp(anam,"parent") == 0)
			strcpy(devices[num_of_devs]->parent,aval);
	    else if (strcmp(anam,"connwhere") == 0)
			strcpy(devices[num_of_devs]->connwhere,aval);
	    else if (strcmp(anam,"PdDvLn") == 0)
			strcpy(devices[num_of_devs]->PdDvLn,aval);
	    else
			continue;	/* Invalid field  */
	    break;

	default:
	    DEBUG_2("line = %s\tline_type = %d\n", buf, lt);
            break;
        }
   }	
return(num_of_devs);
}


/*----------------------------------------------------------------------------
   make_attr_index()  - This routine scans the stanza file and constructs
		   an array of pointers to attribute stanza structures.

                     RETURNS: Number of pointers to attribute structure

  ---------------------------------------------------------------------------*/

int make_attr_index(FILE *fa, struct attr_stanza *attributes[])
{
int			lt, num_of_attrs;  /* number of attrs in array index */
struct attr_stanza	*newstanza;	/* used to malloc a new attr record. */
struct attr_struct	*newstruct;	/* Points to attr name/value struct. */
struct attr_struct	*curstruct;	/* Points to attr name/value struct. */
char			buf[2048];	/* Working buffer		*/
char			aval[256];	/* Temp buffer for attr value	*/
char			anam[256];	/* Temp buffer for attr name	*/

rewind(fa);				/* Go to top of file		*/
num_of_attrs = -1;

while(!feof(fa))
   {
   if(fgets(buf,256,fa) == NULL)
	   continue;
					/* Check for a stanza name	*/
    lt = line_type(buf);
    switch(lt)
	{
	case L_STANZA:
	    DEBUG_2("line = %s\tline_type = %d\n", buf, L_STANZA);
	    /* Take off the colon from the end of the name */
            get_stanza_name(buf);
					/* Get a new index record	 */
	    newstanza =(struct attr_stanza *)malloc(sizeof(struct attr_stanza));
					/* Fill in the name		*/
	    strcpy(newstanza->name,buf);
	    newstanza->fields = NULL;
					/* Put it in the index		*/
	    attributes[++num_of_attrs] = newstanza;
	    break;
	case    L_ATTR:
	    DEBUG_2("line = %s\tline_type = %d\n", buf, L_ATTR);
	    if(get_attr(buf,anam,aval))
		{
		DEBUG_3("buf = %s anam = %s aval = %d\n", buf, anam, aval[0]);
		continue;
		}
					/* Get a new fields record */
	    newstruct = (struct attr_struct *) malloc(sizeof(struct attr_struct));
	    strcpy(newstruct->attribute, anam);
	    strcpy(newstruct->value, aval);
	    newstruct->next = NULL;

	    if ( attributes[num_of_attrs]->fields == NULL )
		{
		attributes[num_of_attrs]->fields = newstruct;
		}
	    else
		{
		curstruct->next = newstruct;
		}
	    curstruct = newstruct;
	    break;

	default:
	    DEBUG_2("line = %s\tline_type = %d\n", buf, lt);
            break;
        }
   }	
return(num_of_attrs);
}


/**
    Assumptions for this routine:

       1)  Each stanza header ends in a ':'
       2)  Each attribute line has a '=' before any colons and each
           attribute line has some text following the equal sigh
       3)  Blank lines are ignored
       4)  The '#' sign is used in column 1 to indicate a comment
       5)  Invalid lines are treated as blank lines

**/

/*----------------------------------------------------------------------------
   line_type(buf)  - The line type examines each line and returns the
                     type of line.

                     RETURNS:
                     0 = blank line or comment line
                     1 = stanza header
                     2 = attribute line

  ---------------------------------------------------------------------------*/

line_type(buf)
char   *buf;
{
int    i,j;
char   isblank;                         /* blankline flag                    */

j = strlen(buf)-1;                      /* Calculate length of line          */

if(j < 2)                               /* Line doesn't count if < 2 chars   */
   return(L_BLANK);

if(buf[0] == '#')                       /* Line is a comment                 */
   return(L_COMMENT);

isblank = TRUE;
for(i=0;i<j;i++)
   {
   if(isblank)                          /* Let's check for a non-blank char  */
           {
           if(isgraph(buf[i]))          /* We don't care about the 1st char  */
                   isblank = FALSE;     /* so this throws it away            */
           }
   else
       {
       if(buf[i] == ':')                /* Is it a stanza header             */
           return(L_STANZA);
       if(buf[i] == '=')                /* Is it an attribute line           */
           return(L_ATTR);
       }
   }
return(L_BLANK);                        /* If we get here it is not a valid  */
                                        /* line or it is blank               */
}

/*----------------------------------------------------------------------------
   get_stanza_name(buf)    - Routine will modify the buffer to contain only
                             the stanza header name

                             RETURNS:
                             0 - Success
                             1 - Fail
  ---------------------------------------------------------------------------*/

get_stanza_name(buf)
char   *buf;
{
int    i,j;

j = strlen(buf)-1;                      /* Get length of buffer              */

for(i=0;i<j;i++)
   if(buf[i] == ':')
       {
       buf[i] = 0;
       return(0);
       }
return(1);
}


/*----------------------------------------------------------------------------
   get_attr(buf,anam,aval) - Gets one attribute name and value and returns
                             them in anam,aval.

                             RETURNS:
                             0 - Success
                             1 - Fail
  ---------------------------------------------------------------------------*/

get_attr(buf,anam,aval)
char   *buf,*anam,*aval;
{
int	rc, returnval;
char	equal[10];

returnval = 0;
rc = sscanf(buf,"%s%s%s",anam,equal,aval);
/* fprintf(stderr, "get_attr: rc = %d buf = %s\n", rc, buf); */
if(rc != 3)
	{
	if (rc == 2)
		{
		aval[0] = '\0';
		}
	else
		{
		returnval = 1;
		}
	}
return(returnval);
}
