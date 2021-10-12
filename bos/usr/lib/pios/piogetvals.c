static char sccsid[] = "@(#)34	1.5  src/bos/usr/lib/pios/piogetvals.c, cmdpios, bos411, 9428A410j 12/8/93 18:32:13";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: piogetvals, attrtab_init
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/lockf.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>

#include "pioformat.h"


extern int errno;               /* global errno - set by system */
extern int statusfile;          /* indicates the existence of a statusfile */

extern char *mem_start;         /* address of the start of odm data memory */
extern char *odm_table;         /* address of start of odm table */
extern int  objtot;             /* number of attributes in odm table */
extern int	piocfapi;	/* kludge variable for silencing the
				   error messages for colon file API */

extern struct shar_vars *sh_vars;
int  Maxchar;                   /* indicates max size of variable length strg */

struct odm *get_odmtab_ent();
struct lktable *get_lktab_ent();

/*******************************************************************************
*                                                                              *
* NAME:           piogetvals                                                   *
*                                                                              *
* DESCRIPTION:    Create the attribute table from the odm table and from       *
*                 the data passed to us from the formatter.  The formatter     *
*                 gives us a pointer to an array of structures containing      *
*                 information about the data types of some of the odm          *
*                 attributes, for example all attributes with integer          *
*                 data types should have an entry in the formatter table       *
*                 as well as all attributes that are variable strings.         *
*                 Command strings need not appear in the formatter table       *
*                 as they cannot be dynamically changed.  Piogetvals           *
*                 combines the data from the two tables and creates a third    *
*                 table which consists of records containing pointers to       *
*                 converted attribute values and their associated datatypes.   *
*                 There should be a separate value entry for each separate     *
*                 mode type per attribute.  Command strings are the only       *
*                 exception being that they are immutable.  Piogetvals         *
*                 also fills an entry in each record of the table passed to    *
*                 it by the formatter.  The entry is filled with the address   *
*                 of the converted attribute value which exists as a part of   *
*                 the newly created attribute table.  If more than one mode    *
*                 is define then the entry really points to the beginning of   *
*                 an array of values.                                          *
*                                                                              *
* PARAMETERS:     attr_parm - address of formatter table which contains        *
*                             odm attribute information.                       *
*                 buflen    - Maximum length of a variable length string.      *
*                                                                              *
* INPUTS:         odm database file created by piodigest.                      *
*                 formatter attribute table (the formatter passes us a         *
*                 handle to it throught the parameter list).                   *
* GLOBALS:                                                                     *
*     MODIFIED:   Maxchar      - Maximum length of a variable length string.   *
*                 piomode      - determine which set of attributes to use.     *
*                                                                              *
*                                                                              *
* RETURN VALUES:  0       - pioformat attribute table creation and             *
*                           population sucessful.                              *
*                -1       - error encountered (message sent to submitter)      *
*
*                                                                              *
*******************************************************************************/

piogetvals(attr_parm, return_on_err)

  struct attrparms *attr_parm;  /* pointer to formatter attribute table */
  int return_on_err;    /* if TRUE, when an error is detected, return with
			   -1 instead of issuing an error msg & exiting */
			/* NOTE: TRUE not currently supported */
{
  extern int Maxchar;           /* max length of variable strings */
  extern int piomode;         /* indicates the set of attribute values in use */

  extern struct attr_info *attr_table;  /* addr of start of attribute table */

  struct int_info *int_info_p;  /* pointer to attribute integer information */
  unsigned int objcnt;          /* index for formatter attribute table */

  struct str_info  *str_info_p; /* pntr to attribute string information */
  struct attr_info *attr_tab_p; /* pntr to attribute table attribute record */
  struct lktable   *lktabptr;   /* pntr to a formatter lookup table */
  struct odm     *odm_tab_p;    /* pntr to an odm table attribute record */
  char wkbuf[BUFSIZ];           /* work buffer */
  int slen, intval, i;          /* work integers */
  char *wkptr;                  /* work pointer */

  /* number of bytes to be malloc'd for each VAR_STR */
  if (Maxchar == 0)
    Maxchar = 1024;

  /* in case we have to put out an error message */
  strcpy(errinfo.subrname, "piogetvals");

  /* Proces each entry (attribute name) in the table passed by the caller */
  for (objcnt = 0; *attr_parm->name != '\0'; attr_parm++, objcnt++)
  {
    if ((odm_tab_p = get_odmtab_ent(attr_parm->name)) == NULL ||
	(attr_tab_p = get_attrtab_ent(attr_parm->name)) == NULL)
      ERREXIT(0, MSG_ATTRNAME, attr_parm->name, NULL, objcnt);

    /* If this one has been processed before, start over as CONST_STR */
    if (attr_tab_p->datatype == VAR_INT || attr_tab_p->datatype == VAR_STR)
    {
      if (attr_tab_p->datatype == VAR_INT)
      {
	REALLOC(wkptr,(char *)attr_tab_p->ptypes.ivals,sizeof(struct str_info));
	str_info_p = (struct str_info *) wkptr;
      }
      else    /* VAR_STR */
      {
	str_info_p = attr_tab_p->ptypes.sinfo;
	for (i = 0; i < PIO_NUM_MODES; i++)
	  (void) free(str_info_p[i].ptr);
	REALLOC(wkptr,(char *)attr_tab_p->ptypes.sinfo,sizeof(struct str_info));
	str_info_p = (struct str_info *) wkptr;
      }
      str_info_p->ptr = (char *) GET_OFFSET(odm_tab_p->str);
      str_info_p->len = odm_tab_p->length;
      attr_tab_p->ptypes.sinfo = str_info_p;
      attr_tab_p->datatype = CONST_STR;
    }

    /* Process according to data type specified by caller                */
    /* NOTE: Any change to the logic below may also need to be made in   */
    /*       piogetopt() where refresh is done after flags are processed */

    switch(attr_parm->datatype)
    {
      case VAR_INT:
	/* get the string to be converted to an integer */
	/* (with logic & attribute references resolved) */
	slen = pioparm(attr_parm->name, NULL, wkbuf, sizeof(wkbuf), 0, 0, NULL);
	if (slen < 0)
	    ERRETURN();

	/* adjust the memory to hold integer information */
	REALLOC(wkptr,(char *)attr_tab_p->ptypes.ivals,sizeof(struct int_info));
	int_info_p = (struct int_info *) wkptr;

	/* if plain old integer merely do an atoi on the odm table attr val */
	if (attr_parm->lktab == NULL)
	{
	    if (!strcmp(wkbuf, YES_STRING))
	      intval = 1;
	    else if (!strcmp(wkbuf, NO_STRING))
	      intval = 0;
	    else
	      intval = atoi(wkbuf);
	}
	else
	/* if not a plain old integer we need to get the formatter lookup
	 * table value associated with the odm table's representational
	 * value.  In other words, the odm table value may say "TRUE", we
	 * really want to find its integer representation which is stored
	 * in one of the formatters lookup table.  So we convert "TRUE" to
	 * an integer 1.
	 */
	{
	  lktabptr = get_lktab_ent(wkbuf, (struct lktable *) attr_parm->lktab);
	  if (lktabptr != NULL)
	      intval = lktabptr->value;
	  else
	      intval = -1;
	}
	int_info_p->lkuptabp =  attr_parm->lktab;

	/* fill the value array associated with the attribute */
	for (i = 0; i < PIO_NUM_MODES; i++)
	  int_info_p->value[i] = intval;
	attr_tab_p->ptypes.ivals = int_info_p;

	/* give the formatter back the addr of where the int vals are stored. */
	attr_parm->types->addr = (int *) int_info_p;
	break;

      case VAR_STR:
	/* get the string (with logic & attribute references resolved) */
	slen = pioparm(attr_parm->name, NULL, wkbuf, sizeof(wkbuf), 0, 0, NULL);
	if (slen < 0)
	    ERRETURN();

	/* adjust the memory to hold variable string information */
	REALLOC(wkptr,attr_tab_p->ptypes.sinfo,
	  PIO_NUM_MODES * sizeof(struct str_info));
	str_info_p = (struct str_info *) wkptr;

	/* fill the value array associated with the attribute */
	for (i = 0; i < PIO_NUM_MODES; i++)
	{
	  str_info_p[i].len = slen;
	  MALLOC(str_info_p[i].ptr, slen + 1);
	  (void) memcpy(str_info_p[i].ptr, wkbuf, slen + 1);
	}
	attr_tab_p->ptypes.sinfo = str_info_p;

	/* give formatter back the addr of where the var str vals are stored. */
	attr_parm->types->sinfo = (struct str_info *) str_info_p;
	break;

      default:
	ERREXIT(0, MSG_DATATYPE, attr_parm->name, NULL, 0);
    } /*switch*/

    /* no more calls to pioparm(), so now OK to set datatype */
    attr_tab_p->datatype = attr_parm->datatype;

  } /*for*/

  return(0);
}


/************  Redefine ERREXIT and MALLOC as a kludge for colon file API
	       so that the control just returns back to the calling
	       program.  *************/
#undef ERREXIT
#define ERREXIT(err_type, msgnum, stringval1, stringval2, integerval) \
      { if (piocfapi) \
	   return -1; \
	errinfo.errtype = err_type; \
	strncpy(errinfo.string1, stringval1, sizeof(errinfo.string1) - 1); \
	strncpy(errinfo.string2, stringval2, sizeof(errinfo.string2) - 1); \
	errinfo.integer = integerval; \
	(void) piogenmsg(msgnum, TRUE); \
	(void) pioexit(PIOEXITBAD); }

#undef MALLOC
#define MALLOC(memptr, size) \
   {if ((memptr = malloc(size)) == NULL) { \
	if (piocfapi) \
	   return -1; \
	(void) piomsgout(getmsg(MF_PIOBE,1,MSG_MALLOC)); \
	(void) pioexit(PIOEXITBAD); \
    }}

/*******************************************************************************
*                                                                              *
* NAME:           attrtab_init                                                 *
*                                                                              *
* DESCRIPTION:    Read in the odm table and create the attribute table         *
*                 from the odm table                                           *
*                                                                              *
* PARAMETERS:     filename - name of the file containing the digested          *
*                            data base values                                  *
*                                                                              *
* INPUTS:         odm database file created by piodigest.                      *
*                                                                              *
* GLOBALS:        attr_table  - addr of start of attribute table               *
*     MODIFIED:                                                                *
*                                                                              *
*                                                                              *
* RETURN VALUES:  0       - successful                                         *
*
* ERROR EXIT:     PIOEXITBAD - error encountered
*
*                                                                              *
*******************************************************************************/

attrtab_init(odm_filepath)
char * odm_filepath;
{
extern char *mem_start;         /* address of the start of odm data memory */
extern struct attr_info *attr_table;  /* addr of start of attribute table */
extern char *hash_table;        /* address of start of primary hash table */
extern char *odm_table;         /* address of start of odm table */

  unsigned int objcnt;          /* index for formatter attribute table */
  struct odm *odm_tab_p;        /* pntr to an odm table attribute record */
  struct attr_info *attr_tab_p; /* pntr to attribute table attribute record */
  struct stat statbuf;          /* file status buffer */
  int fildes;                   /* file descriptor */
  int header_failed = FALSE;    /* set to TRUE if read for file header fails */
  static struct {               /* area to read file header into */
    char hdrtext[sizeof (ODMFILE_HEADER_TEXT) - 1];
    char nullterm[1];
  } hdr;
  char *wkptr;                  /* work pointer */

  /* does file exist ?? */
  if (stat(odm_filepath, &statbuf) != 0 ||
	(fildes = open(odm_filepath, O_RDONLY)) < 0)
      ERREXIT(0, MSG_DBOPENFAIL, odm_filepath, NULL, errno);

  /* read in the file header to verify that it is a digested data base */
  /* file and that the file format version number is one we understand */
  if (read(fildes, hdr.hdrtext, sizeof(hdr.hdrtext)) < 0
	&& statbuf.st_size >= sizeof(hdr.hdrtext))
      header_failed = TRUE;
  else
  {
      if (statbuf.st_size < sizeof(hdr.hdrtext)
	    || strncmp(hdr.hdrtext, ODMFILE_HEADER, sizeof (ODMFILE_HEADER) - 1))
	  ERREXIT(0, MSG_NOTDBFILE, odm_filepath, NULL, 0);
      if (strncmp(hdr.hdrtext + (sizeof (ODMFILE_HEADER) - 1),
		  EXP_STRGZ (ODMFILE_VERSION),
		  sizeof (EXP_STRGZ (ODMFILE_VERSION)) - 1))
	  ERREXIT(0, MSG_FMTLEVEL, odm_filepath,
		  hdr.hdrtext + (sizeof (ODMFILE_HEADER) - 1), 0);
  }

  /* malloc space so we can read in the odm data file */
  MALLOC(mem_start, statbuf.st_size - sizeof(hdr.hdrtext));

  /* slurp up the odm data file */
  if (header_failed ||
	read(fildes, mem_start, statbuf.st_size - sizeof(hdr.hdrtext)) < 0)
      ERREXIT(0, MSG_DBREADFAIL, odm_filepath, NULL, errno);

  /* get the value for the total number of odm attributes */
  memcpy(&objtot, mem_start, sizeof(int));
  odm_table  = ODM_TABLE;
  hash_table = HASH_TABLE;

  /* allocate space for the attribute table and initialize the table */
  MALLOC(wkptr, objtot * sizeof(struct attr_info));
  attr_table = (struct attr_info *) wkptr;

  attr_tab_p = (struct attr_info *) attr_table;

  /* initialize the attribute table */
  for (objcnt = 0; objcnt < objtot; objcnt++, attr_tab_p++)
  {
    MALLOC(wkptr, sizeof(struct str_info));
    attr_tab_p->ptypes.sinfo = (struct str_info *) wkptr;
    odm_tab_p = (struct odm *) (odm_table + (sizeof(struct odm) * objcnt));
    attr_tab_p->datatype = CONST_STR;
    attr_tab_p->flags = 0;
    attr_tab_p->ptypes.sinfo->ptr = (char *) (mem_start + odm_tab_p->str);
    attr_tab_p->ptypes.sinfo->len = odm_tab_p->length;
  }
  return(0);
}
