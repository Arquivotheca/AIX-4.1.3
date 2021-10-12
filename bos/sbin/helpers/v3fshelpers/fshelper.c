static char sccsid[] = "@(#)75 1.13.2.3  src/bos/sbin/helpers/v3fshelpers/fshelper.c, cmdfs, bos411, 9428A410j 12/15/93 16:11:09";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fshelper
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


#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/sysmacros.h>
#include <stdio.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <fshelp.h>
#include "fsop.h"
#include <libfs/libfs.h>

#define BAD_OP   ((int (*)()) -1)    
#define NULL_OP  ((int (*)())  0)

extern int op_check();
extern int op_make();
extern int op_statfs();
extern int op_namei();
extern int op_debug();
extern int op_extend();
static int getmode();
static int found_any();
static int get_arg_val();

static int (*opsw[FSHOP_NUM_OPS])() =
{
  BAD_OP,              /* null    */
  op_check,            /* check   */
  op_extend,           /* extend  */
  NULL_OP,             /* findata */
  NULL_OP,             /* free    */
  op_make,             /* make    */
  NULL_OP,             /* rebuild */
  op_statfs,           /* statfs  */
  NULL_OP,             /* stat    */
  NULL_OP,             /* usage   */
  op_namei,            /* find    */
  op_debug	       /* debug   */
};

int   DebugLevel;
int   Mode;
int   PipeFd;
char *ProgName;


static struct old_fs    *OldFsList;

/*
** used for the function eval_bool. this structure contains a list 
** of valid values for strings to be turned into bools.
*/
struct bool_cmp {
    char *str; 
    bool_t val;
};
static struct bool_cmp bools[] = {	 
  {"t", True}, { "f", False}, { "T", True}, { "F", False},
  {"true", True}, {"false", False}, {"TRUE", True}, {"FALSE", False},
  {"True", True}, {"False", False}, 
  {"y", True}, { "n", False}, { "Y", True}, { "N", False},
  {"yes", True}, {"no", False}, {"YES", True}, {"NO", False},
  {"Yes", True}, {"No", False},  
  {"1", True}, {"0", False},
  { NULL, 0 } 
};

/*
** since error codes get passed back as our exit code
** any routine that detects an error simply calls exit
** (luckily there aren't many)
*/

main (ac, av)
     int    ac;
     char  **av;
{
  int      op;
  int      fsfd;
  char    *opflags;
  int      rc;
  
  if (ac > N_ARGS)
     exit (FSHERR_SYNTAX);
  else if (ac < N_ARGS)
     exit (FSHERR_ARGMISSING);

  ProgName   = av[A_NAME];
  
  opflags    = av[A_FLGS];
  op         = atoi (av[A_OP]);
  fsfd       = atoi (av[A_FSFD]);
  PipeFd     = atoi (av[A_COMFD]);
  DebugLevel = atoi (av[A_DEBG]);
  Mode       = getmode (av[A_MODE]);

  rc = (op <= FSHOP_NULL || op > LAST_FSHOP ||
	opsw[op] == BAD_OP?  FSHERR_UNKOP:
	(opsw[op] == NULL_OP?  FSHERR_NOTSUP:
	 (*opsw[op]) (fsfd, opflags)));

  exit (rc);
}    
    
static int
getmode (flags)
     char *flags;
{
  register char  f;
  register int   mode;

  /*
  ** something funny be going on
  */
  if (!flags || *flags != '-')
      exit (FSHERR_INVALMODE);

  /*
  ** resonable, correct?
  */
  mode = 0;
  while (f = *++flags)
  {
    switch (f)
    {
    case FSHMOD_INTERACT_FLAG:
      mode |= FSHMOD_INTERACT;
      break;

    case FSHMOD_FORCE_FLAG:
      mode |= FSHMOD_FORCE;
      break;

    case FSHMOD_NONBLOCK_FLAG:
      mode |= FSHMOD_NONBLOCK;
      break;

    case FSHMOD_PERROR_FLAG:
      mode |= FSHMOD_PERROR;
      break;

    case FSHMOD_ERRDUMP_FLAG:
      mode |= FSHMOD_ERRDUMP;
      break;

    case FSHMOD_STANDALONE_FLAG:
      mode |= FSHMOD_STANDALONE;
      break;

    case FSHMOD_IGNDEVTYPE_FLAG:
      mode |= FSHMOD_IGNDEVTYPE;
      break;


    default:
      exit (FSHERR_INVALMODE);
    }
  }    
  return mode;
}

/*
** makeFStab
**
**   tables information in FSfile into handy list of old_fs's
**
**
** Warnings:
**  - uses libIN/AF... routines
**  - will eventually use the ODM
*/

static int
makeFStab (filesystems)
    char *filesystems;
{
  AFILE_t          attrfile;
  ATTR_t           stanza;
  struct old_fs   *ofsp     = NILPTR (struct old_fs);
  struct old_fs   *lastofs  = NILPTR (struct old_fs);
    
  /*
  ** no use redoing
  */
  if (OldFsList != NILPTR (struct old_fs))
      return 0;

  if (!(attrfile = AFopen (filesystems, MAXREC, MAXATR)))
      return -1;

  while (stanza = AFnxtrec (attrfile))
  {
    if (!(ofsp = (struct old_fs *) malloc (sizeof (struct old_fs))))
	return -1;

    memset ((void *)ofsp, 0, (size_t)sizeof(struct old_fs));
    
    /*
    ** add to the old_fs list
    */
    if (!found_any (stanza, ofsp))
	free (ofsp);
    else
    {
      if ( lastofs == NILPTR (struct old_fs) )
	OldFsList = ofsp;
      else
	lastofs->next = ofsp;
      lastofs = ofsp;
    }
  }
  AFclose (attrfile);
  return 0;
}
    
/*
** found_any()
**  
** lots of hard coding of attributes here
** at least its isolated to this routine
** and makeFStab ().
**   "dev"     - raw device       (ie. "/dev/vgXX/fs3"),
**   "name"    - filesystem name  (ie. "/tmp")
**   "log"     - log device       (ie. "/dev/vgXX/log...")
*/
static int
found_any (stanza, ofsp)
     ATTR_t  stanza;
     struct old_fs *ofsp;
{
  char    *attrval;
  int      found = 0;

  if (attrval = AFgetatr (stanza, "dev"))
    (void)strncpy(ofsp->dev, attrval, (size_t)strlen(attrval));

  if (attrval = AFgetatr (stanza, "log"))
  {
    (void) strncpy (ofsp->log, attrval, strlen (attrval));
    found++;
  }

  /*
  ** only worth tabling if it has relevant keywords in it
  */
  if (found)
      (void) strncpy (ofsp->name, stanza->AT_value, strlen (stanza->AT_value));
  
  return found;
}      

/*
** get_oldfs
**
** gets old_fs entry from table
**  This routine is used by the various op's
**  It should not be static.
*/

struct old_fs *
get_oldfs (fsname)
     char *fsname;
{
  struct old_fs  *fsp;
  int             makeFStab();
  
  if (makeFStab (FSYSname))         /* in <IN/FSdefs.h> */
      return NILPTR (struct old_fs);

  for (fsp = OldFsList; fsp; fsp = fsp->next)
      if (strcmp (fsp->name, fsname) == 0)
	  return fsp;

  return NILPTR (struct old_fs);
}

/* 
 * eval_bool(str,ret) -- returns GOOD or BAD.  side effects: fills in 'ret'
 * with the value of the bool-string 'str'
 */
int
eval_bool(str,ret,bools)
char *str;
bool_t *ret;
struct bool_cmp *bools;
{
    int i;

    for (i = 0; bools[i].str != NULL; i++ ) {
	if (!strcmp(bools[i].str,str)) { 
	    *ret = bools[i].val; 
	    return(GOOD); 
	}
    }
    return(BAD);
}

/*
** routines to pull args out of opflags
** individual op's must allocate space for the value to be stored
** (Args[index]->iptr)
** returns:
**          OK:         FSHERR_GOOD
**          otherwise:  FSHERR_SYNTAX    (unrecognized, not in Args list)
**                      FSHERR_INVALARG  (value was meaningless)
*/
int
get_args(args, opflags)
     struct arg  *args;
     char        *opflags;
{
  struct arg      *ap;
  char            *op;
  int              arg_ok;
  int              oplen;

  /*
  ** pull out options, assumed to be a comma
  ** separated list of keyword=numbervalue's or keywords (if boolean)
  */
  for (op = strtok (opflags, ","); op; op = strtok (NILPTR (char), ","))
  {
    for (arg_ok = 0, ap = args; ap->name && !arg_ok; ap++)
    {
        oplen = strcspn (op, "=");
        if (strncmp (ap->name, op, oplen? oplen: strlen (op)) == 0)
	  arg_ok++;
    }
    if (!arg_ok)
	return FSHERR_INVALARG;
    --ap;
    if (get_arg_val (op, ap->type, ap->iptr) == BAD)
	return FSHERR_SYNTAX;
  }
  return FSHERR_GOOD;
}
/*
** get_arg_val
**
** return values: BAD:   problem
**                TRUE:  ok
**                FALSE: "never"
*/
static int
get_arg_val (str, type, rtrnptr)
      char     *str;
      arg_t     type;
      caddr_t   rtrnptr;
{
  char          *cp, *sp, *p;
  register char *dp;
  char           buf[BUFSIZ];
  int            rc = FALSE;
  int 		 n;
  char   	 *rv;
  struct fsh_argvector *v;
  int stride;
  
  if (strncpy (buf, str, (size_t)(BUFSIZ-2)) != buf)
    return BAD;
  buf[BUFSIZ-1] = '\0';

  if ((cp = strrchr (buf, '=')) != NILPTR (char))
  {
    cp++;				/* step over the '=' */
    switch (type)
    {
	case STR_T:
	  rc = ((char *)strcpy((char *) rtrnptr, cp) != rtrnptr? BAD: TRUE);
	  break;
	case INT_T:
	  /* figger out base of number with strtol */
	  n = strtol(cp,&rv,0);
	  /* check validity.. */
	  if (n == 0 && rv == cp) rc = BAD;
          *((int *) rtrnptr) = n;
          rc = TRUE;
	  break;
	case BOOL_T:
          rc = eval_bool(cp,(bool_t *)rtrnptr,bools);
	  break;
	case INTV_T:
	  /* malloc space for vector */
	  v = (struct fsh_argvector *)malloc(sizeof(struct fsh_argvector));
	  if (v == NULL) { rc = BAD; break; }

	  stride = strlen(buf);
	  if (buf[stride-1] != FSH_VECTORDELIM) {
	      buf[stride++] = FSH_VECTORDELIM; buf[stride++] = '\0';
	  }

	  /* count elements of vector */
	  for (p = cp, stride = 0; *p; stride+=(*p==FSH_VECTORDELIM),p++);

	  /* fill in stride of vector and malloc space for elements of vector */
	  v->iv_stride = stride;
	  v->iv_int = (int *) malloc(sizeof(int)*(stride));

	  /* reset stride and fill in elements of vector */
	  stride = 0; 
	  for (sp=p=cp;*p;p++) {
	    /* validate number */
	    if (*p == FSH_VECTORDELIM) {
	      *p = 0;
	      /* figger out base of number! */
	      v->iv_int[stride++] = strtol(sp,&rv,0);
	      if (v->iv_int[stride-1] == 0 && rv == sp) rc = BAD;
	      if (rc == BAD) break;
	      sp = p+1;
	      *p = FSH_VECTORDELIM;
	    }
	  }
	  if (rc == BAD) break;
	  *(struct fsh_argvector **)rtrnptr = v;
	  rc = GOOD;
	  break;

	case BOOLV_T:		/* boolean vector */
	  /* malloc space for vector */
	  v = (struct fsh_argvector *)malloc(sizeof(struct fsh_argvector));
	  if (v == NULL) { rc = BAD; break; }

	  stride = strlen(buf);
	  if (buf[stride-1] != FSH_VECTORDELIM) {
	      buf[stride++] = FSH_VECTORDELIM; buf[stride++] = '\0';
	  }

	  /* count elements of vector */
	  for (p = cp, stride = 0; *p; stride+=(*p==FSH_VECTORDELIM),p++);

	  /* fill in stride of vector and malloc space for elements of vector */
	  v->iv_stride = stride;
	  v->iv_bool = (bool_t *) malloc(sizeof(bool_t)*(stride));

	  /* reset stride and fill in elements of vector */
	  stride = 0;
	  for (sp=p=cp;*p;p++) {
	    /* validate number */
	    if (*p == FSH_VECTORDELIM) {
	      *p = 0;
              rc = TRUE;
	      if (eval_bool(sp,&(v->iv_bool[stride]),bools) == BAD) rc = BAD;
	      if (rc == BAD) break;
	      sp = p+1;
	      *p = FSH_VECTORDELIM;
	      stride++;
	    }
	  }
	  if (rc == BAD) break;
	  *(struct fsh_argvector **)rtrnptr = v;
	  rc = GOOD;
	  break;

	case STRV_T:
	  /* malloc space for vector */
	  v = (struct fsh_argvector *)malloc(sizeof(struct fsh_argvector));
	  if (v == NULL) { rc = BAD; break; }

	  stride = strlen(buf);
	  if (buf[stride-1] != FSH_VECTORDELIM) {
	      buf[stride++] = FSH_VECTORDELIM; buf[stride++] = '\0';
	  }

	  /* count elements of vector */
	  for (p = cp, stride = 0; *p; stride+=(*p==FSH_VECTORDELIM),p++);

	  /* fill in stride of vector and malloc space for elements of vector */
	  v->iv_stride = stride;
	  v->iv_str = (char **) malloc(sizeof(char *)*(stride));

	  /* reset stride and fill in elements of vector */
	  stride = 0;
	  for (sp=p=cp;*p;p++) {
	    /* split string */
	    if (*p == FSH_VECTORDELIM) {
	      *p = 0;
	      v->iv_str[stride] = (char *) malloc(strlen(sp)+1);
	      if (v->iv_str[stride] == NULL) { rc = BAD; break; }
	      rc = (((char *)strcpy(v->iv_str[stride],sp)!=v->iv_str[stride])?BAD:TRUE);
	      if (rc == BAD) break;
	      sp = p+1;
	      stride++;
	      *p = FSH_VECTORDELIM;
	    }
	  }
	  if (rc == BAD) break;
	  *(struct fsh_argvector **)rtrnptr = v;
	  rc = GOOD;
	  break;

        default:
	  rc = BAD;
	  break;
    }
  }
  else
  {
     rc = TRUE;
     switch (type)
     {
       case BOOL_T:
          *((bool_t *) rtrnptr) = True;
          break;

       case STR_T:
          *((char *) rtrnptr) = '\0';
	  break;

       case INT_T:
          *((int *) rtrnptr) = 0;
          break;
       case BOOLV_T:
       case INTV_T:
       case STRV_T:
       default:
          rc = BAD;
          break;
     }
  }
  return rc;
}

/*
 *	getarg
 *
 *	- yet another way to parse flags
 *
 *	- 'flags' is the NULL terminated, comma separated list of flags
 *	  and value
 *	- 'names' is a NULL terminated array of valid flags.  flags need
 *	  not include the '='
 *
 *	return values:
 *		-2:	end of flags has been reached (i.e. all
 *				of the 'flags' string has been looked at
 *		-1:	we found a flag not listed in 'names'
 *		>=0:	index into 'names' of the recognized flag we
 *				just found in 'flags'
 *
 *	if -1 is returned, 'argarg' will point to the invalid flag
 *	if 0 is returned, 'argarg' will point to the value of the valid flag
 *
 *	examples:
 *		static char *names[] = { "file=", "tmp", "preen", NULL };
 *
 *		getarg("file=hello,preen,tmp=/tmp,mom", names)
 *			1st time, returns 0, argarg = "hello"
 *			2nd time, returns 2, argarg = NULL
 *			3rd time, returns 1, argarg = "/tmp"
 *			4th time, returns -1, argarg = "mom"
 *			5th time, returns -2
 *
 *	'argfirst' can be set to 1 to start over at the beginning of
 *	the string (and has to be set if you start on a new string)
 *
 *	beware, this calls strtok(), which as we all know and hate, will
 *	stick NULLs into 'flags' in place of the commas
 *
 */

char	*argarg = NULL;
int	 argfirst = 1;

int
getarg(flags, names)
char *flags;
char *names[];
{
	int return_code;
	static char *theflags;
	register char **name, *equal;

	/*
	 * set up first time in; point back to the beginning of 'flags'
	 */
	if (argfirst) {
		theflags = flags;
		argfirst = 0;
		}

	/*
	 * get the next flag, point 'argarg' at it
	 */
	if ((argarg = strtok(theflags, ",")) == NULL)
		return_code = -2;

	else {
		theflags = NULL;	/* strtok needs NULL next time	*/

		/*
		 * search 'names' for our current flag
		 */
		for (name = names; *name != NULL && **name != '\0' &&
		     strncmp(*name, argarg, strcspn(*name, "=")) != 0;
		     name++)
			;

		/*
		 * went too far, didn't find this flag - invalid
		 */
		if (*name == '\0' || **name == '\0')
			return_code = -1;

		else {
			/*
			 * point argarg past the equal if it exists, else
			 * NULL
			 */
			argarg = (equal = strchr(argarg, '=')) != NULL ?
					equal + 1 : NULL;

			/*
			 * compute index
			 */
			return_code = name - names;
			}
		}

	return (return_code);
}
