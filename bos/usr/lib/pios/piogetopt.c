static char sccsid[] = "@(#)33	1.8  src/bos/usr/lib/pios/piogetopt.c, cmdpios, bos411, 9428A410j 5/14/91 15:52:19";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: piogetopt
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
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
#include <string.h>
#include <memory.h>

#include "pioformat.h"

#define OPTLIST "a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:\
A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z:\
0:1:2:3:4:5:6:7:8:9:"


extern int errno;               /* global errno - set by system */
extern int statusfile;          /* indicates the existence of a statusfile */

extern char *hash_table;        /* address of start of primary hash table */
extern char *mem_start;         /* address of the start of odm data memory */
extern char *odm_table;         /* address of start of odm table */
extern int  objtot;             /* number of attributes in odm table */
extern int force_recalc;        /* switch to force recalculation of VAR_INTs */
extern struct attr_info *attr_table; /* array of attr_info structures */

extern struct shar_vars *sh_vars;

struct odm *get_odmtab_ent();
struct lktable *get_lktab_ent();

static struct str_info *dptr;   /* dummy pointer for piogetvals to store into */
char attrstr[3];
static struct attrparms attrval[] = {
attrstr, VAR_STR, NULL, (union dtypes *) &dptr,
NULL,       0, NULL,  NULL
};

/*******************************************************************************
*                                                                              *
* NAME:           piogetopt                                                    *
*                                                                              *
* DESCRIPTION:    Process all command line arguements handed to the formatter  *
*                 by pio_main.  Piogetopt will store the argument value        *
*                 associated with the flag name in the attribute table         *
*                 created by piogetarg.                                        *
*                                                                              *
* PARAMETERS:     ac - argument count;                                         *
*                 av - argument vector;                                        *
*                 optstring - list of recognized flag letters                  *
*                                                                              *
* GLOBALS:                                                                     *
*     REFERENCED: optarg - pointer to argument associated with flag.           *
*                                                                              *
* RETURN VALUES:  EOF     all flags sucessfully processed.                     *
*                 '?'     flag letter not found in optstring.                  *
*                 comline_arg  name of argument piogetopt failed on.           *
*                                                                              *
*******************************************************************************/

piogetopt(ac, av, optstring, return_on_err)

  int    ac;            /* argument count */
  char *av[];           /* argument vector */
  char *optstring;      /* list of recognized flags (no longer used) */
  int return_on_err;    /* if TRUE, when an error is detected, return with
			   -1 instead of issuing an error msg & exiting */
			/* NOTE: TRUE not currently supported */
{
  extern char *optarg;          /* ptr to arg associated with flag */
  extern int opterr;            /* if zero, no error msgs to stderr */
  extern int optind;            /* index of next getopt() arg to be processed*/

  char c[3];                    /* odm table attribute name (contructed from
				   the argument flag */

  int save;                     /* variable to save piomode temporarly  */
  int comline_arg;              /* flag name */
  int slen;                     /* length of a variable string */
  int i, cnt;                   /* generic counters */
  int intval;                   /* work integer */
  int rc;                       /* return code from a subroutine */

  char save_attr_info[sizeof(struct attr_info)]; /* save area */
  char wkbuf[BUFSIZ];           /* work buffer */

  struct odm *odm_tab_p;        /* entry in odm table */
  struct attr_info *attr_tab_p; /* attribute table record pointer */
  struct lktable  *lktab_p;     /* lookup table record pointer */
  struct str_info *str_info_p;  /* pointer to string info. (pointer & length) */
  struct int_info *int_info_p;  /* pointer to integer info. */
  struct str_info strinfo;      /* work str_info structure */

  /* in case we have to put out an error message */
  strcpy(errinfo.subrname, "piogetopt");

  /* process all flags */
  opterr = 0;
  optind = 1;
  while ((comline_arg = getopt(ac, av, OPTLIST)) != EOF)
  {
    if ((char) comline_arg == '?')
    {
      *wkbuf = (char) optopt;
      *(wkbuf+1) = '\0';
      ERREXIT(0,(comline_arg == '?') ? MSG_BADFLAG : MSG_NOARG2,wkbuf,NULL,0);
    }

    c[0] = '_'; c[1] = (char) comline_arg; c[2] = '\0';
    if ((attr_tab_p = get_attrtab_ent(c)) == (struct attr_info *) NULL)
      ERREXIT(0, MSG_BADFLAG, &c[1], NULL, 0); /* data base out of sync */

    /* indicate flag was specified on command line */
    attr_tab_p->flags |= ONCMDLINE;

    /* go see piogetval for further information */
    switch(attr_tab_p->datatype)
    {

      case VAR_INT:
	for (i = 1; i < PIO_NUM_MODES; i++)
	{
	  if (attr_tab_p->ptypes.ivals->lkuptabp == NULL)
	  {
	    if (!strcmp(optarg,YES_STRING))
	      intval = 1;
	    else if (!strcmp(optarg,NO_STRING))
	      intval = 0;
	    else
	      intval = atoi(optarg);
	  }
	  else
	  {
	    lktab_p = get_lktab_ent((char *) optarg,
		(struct lktable *) attr_tab_p->ptypes.ivals->lkuptabp);
	    if (lktab_p != NULL)
		intval = lktab_p->value;
	    else
		intval = -1;
	  }
	  attr_tab_p->ptypes.ivals->value[i] = intval;
	}
	break;

      case CONST_STR:
	/* turn constant string into variable string */
	strncpy(attrval[0].name, c, 2);
        save = piomode;
        piomode = PIO_DBASE_MODE;   /* use database values */
	if ((rc = piogetvals(attrval, NULL)) < 0)
	  pioexit(PIOEXITBAD);
        piomode = save;   /* restore piomode */
	/* is now a variable string, so fall through to VAR_STR */
      case VAR_STR:
	slen = strlen(optarg);

	/* update the value array associated with the attribute */
	str_info_p = attr_tab_p->ptypes.sinfo;
	for (i = 1; i < PIO_NUM_MODES; i++)
	{
	  str_info_p[i].len = slen;
	  REALLOC(str_info_p[i].ptr, str_info_p[i].ptr, slen + 1);
	  (void) memcpy(str_info_p[i].ptr, optarg, slen + 1);
	}

	break;

      default:
	break;
    }
  }

  /* now that command line flags have been processed, refresh the variable */
  /* integers and strings whose values depend on flag arguments, which may */
  /* have changed                                                          */
  force_recalc = TRUE;   /* force all referenced VAR_INTs to be recalculated */
  attr_tab_p = (struct attr_info *) attr_table;
  for (cnt = 0; cnt < objtot; cnt++, attr_tab_p++) {
    if (!(attr_tab_p->flags & ONCMDLINE)
	&& (attr_tab_p->flags & REFSFLAGARG)) {
      odm_tab_p = (struct odm *) (odm_table + (sizeof(struct odm) * cnt));
      if (attr_tab_p->datatype != VAR_INT && attr_tab_p->datatype != VAR_STR)
	continue;  /* only interested in variable integers & strings */

      /* save the contents of the attribute table entry */
      (void) memcpy(
	  (char *)save_attr_info, (char *)attr_tab_p, sizeof(struct attr_info));

      /* set up attr table entry so piogetstr() will think it is a CONST_STR */
      strinfo.ptr = (char *) GET_OFFSET(odm_tab_p->str);
      strinfo.len = odm_tab_p->length;
      attr_tab_p->ptypes.sinfo = &strinfo;
      attr_tab_p->datatype = CONST_STR;
      attr_tab_p->flags = 0;

      /* get the updated string value that reflects the command line flags */
      slen = piogetstr(odm_tab_p->name, wkbuf, sizeof(wkbuf), NULL);
      ERRETURN();

      /* put the attr table entry back like it was */
      (void) memcpy(       /* save contents of existing table entry */
	  (char *)attr_tab_p, (char *)save_attr_info, sizeof(struct attr_info));

      switch(attr_tab_p->datatype) {

      case VAR_INT:

	/* if plain old integer merely do an atoi on the attr val */
	int_info_p = attr_tab_p->ptypes.ivals;
	if (int_info_p->lkuptabp == NULL)
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
	 * table value associated with the flag argument
	 * value.  In other words, the argument value may say "TRUE", we
	 * really want to find its integer representation which is stored
	 * in one of the formatters lookup table.  So we convert "TRUE" to
	 * an integer 1.
	 */
	{
	  lktab_p = get_lktab_ent
	    (wkbuf, (struct lktable *) int_info_p->lkuptabp);
	  intval = lktab_p->value;
	}

	/* fill the value array associated with the attribute */
	for (i = 1; i < PIO_NUM_MODES; i++)
	  int_info_p->value[i] = intval;

	break;

      case VAR_STR:

	/* update the value array associated with the attribute */
	str_info_p = attr_tab_p->ptypes.sinfo;
	for (i = 1; i < PIO_NUM_MODES; i++)
	{
	  str_info_p[i].len = slen;
	  REALLOC(str_info_p[i].ptr, str_info_p[i].ptr, slen + 1);
	  (void) memcpy(str_info_p[i].ptr, wkbuf, slen + 1);
	}

	break;
      }/*switch*/
    }/*if*/
  }/*for*/
  force_recalc = FALSE;  /* back to normal */

  return(0);
}

