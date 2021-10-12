static char sccsid[] = "@(#)44	1.3  src/bos/usr/lib/pios/piosubr2.c, cmdpios, bos411, 9428A410j 3/31/94 11:32:12";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: get_lktab_ent, get_attrtab_ent, uppercase
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <sys/lockf.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "pioformat.h"


/*******************************************************************************
*									       *
* NAME: 	  get_lktab_ent 					       *
*									       *
* DESCRIPTION:	  Return a lookup table entry for a corresponding odm (or      *
*		  command line) specified value.  The lookup table was set up  *
*		  and populated by the formatter.  The lookup table consists   *
*		  of pairs of values, one is a string, the other is its        *
*		  corresponding integer value.	For example the string "TRUE"  *
*		  may have a corresponding value of 1.	This integer is the    *
*		  actual value the formatter will use, the string is used to   *
*		  make the odm user interface (devices) user friendly.	The    *
*		  integer value is the one actually stored in the attribute    *
*		  table (however that is not what this function does, it       *
*		  merely returns the correct lookup record entry).	       *
*									       *
* PARAMETERS:	  strval     - attribute value from odm table.		       *
*		  record_ptr - pointer to a lookup table.		       *
*									       *
* RETURN VALUES:  pointer to a lookup table entry.			       *
*		  NULL	   no lookup table value associated with strval.       *
*									       *
*******************************************************************************/

struct lktable *
get_lktab_ent(strval, record_ptr)

    char *strval;		/* odm table attribute value (of type int) */
    struct lktable *record_ptr; /* pointer to a lookup table */
{
    char lkupstr[BUFSIZ];	/* lookup table string entry */
    int  strlength;		/* length of odm string value */

    strlength = uppercase(strval);
    for (; record_ptr->str != NULL; record_ptr++)
    {
	strcpy(lkupstr, record_ptr->str);
	(void) uppercase(lkupstr);
	if (strcmp(lkupstr, strval) == 0)
	    return(record_ptr);
    }

    return((struct lktable *) NULL);
}


/*******************************************************************************
*									       *
* NAME: 	  get_attrtab_ent					       *
*									       *
* DESCRIPTION:	  Find an attribute from the attribute table (not to be        *
*		  confused with the odm table).  The attribute table contains  *
*		  all the converted odm table values.  The odm table attribute *
*		  values were all type-less and stored as strings of chars     *
*		  whereas the attribute values in the attribute table are all  *
*		  typed and stored in a way that is suitable to their types.   *
*		  The formatter determines the type of all attributes either   *
*		  directly or by default.				       *
*									       *
* PARAMETERS:	  name - attribute name associated with an attribute table     *
*			 entry.						       *
*									       *
* RETURN VALUES:  pointer to an attribute table entry.			       *
*		  NULL	  no attribute table entry associated with name        *
*			  argument.					       *
*									       *
*******************************************************************************/

struct attr_info *
get_attrtab_ent(name)
    char *name;				 /* odm table attribute name */
{
    extern struct attr_info *attr_table; /* start addr of the attrubute table */
    extern char *odm_table;		 /* start addr of the odm table */

    struct attr_info *attr_tab_p = attr_table;	/* attribute table ptr */
    struct odm *odm_tab_p;			/* odm table ptr */

    if (attr_tab_p == NULL)
        return(attr_tab_p);
    else if ((odm_tab_p = get_odmtab_ent(name)) == (struct odm *) NULL)
	attr_tab_p = NULL;
    else
	attr_tab_p += (((int)odm_tab_p - (int)odm_table)/sizeof(struct odm));
    return(attr_tab_p);
}  


/* makes strings uppercase */
uppercase(name)
    char *name;
{
    char *c;
    int i;

    for(c = name, i = 0; *c != '\0'; c++, i++)
	if (islower(*c))
	    *c = toupper(*c);
    return(i);
}

