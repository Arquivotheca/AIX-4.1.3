static char sccsid[] = "@(#)31	1.11.1.6  src/bos/usr/lib/pios/pioformat.c, cmdpios, bos411, 9428A410j 4/22/94 15:21:15";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, printfile, vspace, get_local_arg
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

/*** pioformat.c - formatter driver ***/

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <IN/standard.h>
#include <locale.h>
#include <sys/ldr.h>
#include "pioformat.h"

#define SPECPL 0x7b000000
#define FormFeed 12
#define ALLFLAGS "!:#:$:@:\
a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:\
A:B:C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z:\
0:1:2:3:4:5:6:7:8:9:"

/* pointers to be initialized for pioparm.c */
extern struct attr_info *attr_pthru_p;
extern struct int_info *int_pthru_p;
extern char *optarg;
extern int optind;
extern int opterr;
extern char *getmsg();

/* Declarations for message ipc (for Palladium) */
extern uchar_t		piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */
extern int	pmi_initmsgipc(void);
extern int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
extern int	pmi_bldintfrm(int);
extern int	pmi_bldstrfrm(const char *);
extern int	pmi_dsptchmsg(const char *);

/* pointer to structure of vvariables shared by the formatter and pioformat */
struct shar_vars *sh_vars;

char formatter[BUFSIZ];		/* name of the formatter */
char *hash_table;		/* address of the primary hash table */
char *mem_start;		/* address of the start of the odm data */
char odm_filepath[BUFSIZ];	/* file path of the odm data */
char *odm_table;		/* address of the odm table */
static int *dptr;               /* dummy pointer for piogetvals to store into */
char *basedir;               /* var (i.e., "pio") directory path name */
char defbasedir[] = DEFVARDIR;  /* default base directory */

/* attrparm parameters to pass to piogetvals() */
static struct attrparms attrval[] = {
PASSTHRUBYTES, VAR_INT, NULL, (union dtypes *) &dptr,
NULL,       0, NULL,  NULL
};
static struct attrparms attrval2[] = {
INTERFACETYPE, VAR_INT, NULL, (union dtypes *) &dptr,
NULL,       0, NULL,  NULL
};

int rc;                 /* return code from subroutines */
int objtot	= 0;	/* number of objects in odm table */
int pagecount	= 0;	/* number of pages processed */
int piomode	= 0;	/* indicates the set of attr. values in use */
int piopgskip	= 0;	/* number of pages to skip (suppress printing of)  */ 
int piodatasent = FALSE;/* data bytes sent to printer yet? */
int security    = 0;    /* indicates the print status of security labels */
int statusfile	= 0;	/* indicates whether a statusfile exists */
int  retcode    = 0;	/* indicates return code of functions */
int pthruval    = 0;    /* 0 = formatting; 1 = passthru */
int gotfmtrname = FALSE;/* FALSE: don't have formatter name yet               */
			/* TRUE: got formatter name from data base or cmd line*/
struct flock lockdat; /* lockdata for fcntl() D61224 */

/* AIX security enhancement */
extern int getlabel();
extern int print_label();
extern char *profile;
/* TCSEC Division C Class C2 */

/*******************************************************************************
*									       *
* NAME: 	  main							       *
*									       *
* DESCRIPTION:	  Sets up pioformat in order for the formatter to use it,      *
*		  resolves all local command line arguments,  reads the        *
*                 database file into memory for later use by piogetarg,        *
*		  calls the formatter routine setup() (which calls piogetarg   *
*		  and piogetopt).					       *
*									       *
* PARAMETERS:	  argc	  argument count				       *
*		  argv	  argument vector				       *
*									       *
* RETURN VALUES:  EXITGOOD success					       *
*		  EXITBAD  failure					       *
*									       *
*******************************************************************************/

main(argc, argv)

    int  argc;
    char *argv[];
{

    char *odmpathname;		/* directory pathname of odm database*/

    int  argcnt    = 0;		/* number of args removed from argv list */
    int str_len, val, cnt;      /* work integers */
    char *cp;                   /* work pointer */
    struct attr_info *attr_tab_p; /* work pointer */
    int (*loadrc)();
    struct odm *odm_tab_p;	/* pointer to a record in the odm table */

    setlocale(LC_ALL, "");

    piomode = PIO_CURR_MODE;
    /* Set up the lockdat for fcntl() replaces lockf() D61224 */
    lockdat.l_whence = 0;  /* measure from the beginning of file */
    lockdat.l_start = 0;   /* start lock from the beginning */
    lockdat.l_len  = 0;    /* Go till end of file */


    /* Does a status file exist out there ?? */
    if (getenv("PIOSTATUSFILE") != NULL) {
	statusfile = 1;
        lockdat.l_type = F_WRLCK;
        (void) fcntl(3, F_SETLKW, &lockdat);
	(void) log_init();
        lockdat.l_type = F_UNLCK;
        (void) fcntl(3, F_SETLKW, &lockdat);
    }

    /* set up info. in case we have to generate an error message */
    if (statusfile)
	strncpy(errinfo.title, get_title(), sizeof(errinfo.title) - 1);
    else
	strncpy(errinfo.title, getmsg(MF_PIOBE, 1, MSG_NONESTR),
	      sizeof(errinfo.title) - 1);
    strncpy(errinfo.progname, "pioformat", sizeof(errinfo.progname) - 1);

    /* Message ipc initialization */
    pmi_initmsgipc();

    /* get name of odm file */
    if ((retcode = get_local_args(argc, argv, "@")) == 0)
	ERREXIT(0, MSG_DBASEFLAG, NULL, NULL, 0)  /*(don't use semicolon)*/
    else if (retcode < 0)
	ERREXIT(0, MSG_DBASENAME, odm_filepath, NULL, 0)
    argcnt += retcode;

    /* set up the odm table and the attribute table */
    attrtab_init(odm_filepath);

    /* set up more info. in case we have to generate an error message */
    (void) piogetstr(QUEUENAME, errinfo.qname, sizeof(errinfo.qname) - 1, NULL);
    (void) piogetstr(VIRTUALPTRNAME, errinfo.qdname, sizeof(errinfo.qdname) - 1,
      NULL);

    /* get the interface type */
    if ((cp = getenv("PIOINTERFACE")) != NULL)
        val = atoi(cp);
    else
        val = 0;
    attr_tab_p = get_attrtab_ent(INTERFACETYPE);
    if (attr_tab_p != NULL) {         /* safety valve */
        (void) piogetvals(attrval2, NULL); /* set up variable int. attribute*/
        for (cnt = 0; cnt < PIO_NUM_MODES; cnt++) /* initialize int. values */
            attr_tab_p->ptypes.ivals->value[cnt] = val;
    }

    /* Set Up @4 Attribute Value (path for base (i.e., "pio") directory) */
    cp = getenv("PIOBASEDIR");
    if (cp == NULL) {
       cp = (char *)malloc(strlen(DEFBASEDIR));
       strcpy(cp, DEFBASEDIR);
    }
    attr_tab_p = get_attrtab_ent(BASE_DIR);
    attr_tab_p->ptypes.sinfo->ptr = cp;
    attr_tab_p->ptypes.sinfo->len = strlen(cp);

    
    /* Set Up @5 Attribute Value (path for var (i.e., "pio") directory) */
    cp = getenv("PIOVARDIR");
    if (cp == NULL) {
       cp = (char *)malloc(strlen(DEFVARDIR));
       strcpy(cp, DEFVARDIR);
    }
    attr_tab_p = get_attrtab_ent(BASE_VARDIR);
    attr_tab_p->ptypes.sinfo->ptr = cp;
    attr_tab_p->ptypes.sinfo->len = strlen(cp);
    
    /* get the name of the default formatter (if any) */
    str_len = piogetstr(DEFAULTFMTR, formatter, sizeof(formatter), NULL);
    if (str_len > 0)
	gotfmtrname = TRUE;        /* remember that we got a name */

    /* get rest of flag values */
    retcode = get_local_args(argc - argcnt, argv, "!#$");
    argcnt += retcode;

    /* check passthru value */
    if (pthruval < 0 || pthruval > 1)
	ERREXIT(0, MSG_BADARG, "#", NULL, 0);

    /* check security label override value */
    if (security < 0 || security > 1)
	ERREXIT(0, MSG_BADARG, "$", NULL, 0);

    if (gotfmtrname == FALSE)
	ERREXIT(0, MSG_FMTRNAME, NULL, NULL, 0);

    /* load the formatter */
    if (*formatter != '\0') { /* if name is null string, it is already loaded */
	if ((loadrc = (int (*)())load(formatter, 0, 0)) == NULL)
	    ERREXIT(0, MSG_LOADFAIL, formatter, NULL, errno);
	if ((rc = loadbind(0, main, loadrc)) <0)
	    ERREXIT(0, MSG_BINDFAIL, formatter, NULL, errno);
    }

    /* save formatter name in case we have to generate an error message */
    strncpy(errinfo.progname, formatter, sizeof(errinfo.progname) - 1);

    /* setup returns pointers to all values shared by the formatter and the
	preformatter routines.  */
    sh_vars = (struct shar_vars *) setup(argc - argcnt, argv, pthruval);

    /* program name (for error message) back to pioformat */
    strncpy(errinfo.progname, "pioformat", sizeof(errinfo.progname) - 1);

    /* set up pseudo data base attribute name for number of passthrough */
    /* bytes.  pioparm.c will set the value for the variable            */
    attr_pthru_p = get_attrtab_ent(PASSTHRUBYTES);
    if (attr_pthru_p != NULL)   /* safety valve */
    {
	(void) piogetvals(attrval, NULL);
	int_pthru_p = attr_pthru_p->ptypes.ivals;
    }

    /* program name (for error message) back to formatter name */
    strncpy(errinfo.progname, formatter, sizeof(errinfo.progname) - 1);

    if (sh_vars == NULL)
    {
	initialize();
	passthru();
	restore();
	pioexit(PIOEXITGOOD);
    }

    /* print the file */
    printfile();

    pioexit(PIOEXITGOOD);
}


/*******************************************************************************
*									       *
* NAME: 	  printfile						       *
*									       *
* DESCRIPTION:	  sets up top and bottom security labels and prints the        *
*		  document.						       *
*									       *
* PARAMETERS:	  none							       *
*									       *
* GLOBALS:								       *
*     MODIFIED:   sh_pl     - page length                                      *
*		  piopgskip - number of pages to skip			       *
*									       *
*     REFERENCED: sh_vincr - vertical increment value			       *
*		  sh_bmarg - bottom margin value			       *
*		  sh_tmarg - top margin value				       *
*									       *
* RETURN VALUES:  0	   if file printed sucessfully			       *
*		  EXITBAD  if fatal error encountered			       *
*									       *
*******************************************************************************/

printfile()
{

  int  count     = 0;		/* number of bytes returned by lineout */
  int vpos_chg   = 0;           /* change in vpos by formatter's lineout() */
  int  ff_exit_loop   = 0;      /* flag for exiting read loop with FormFeed error */
  int exit_complete   = 0;      /* flag for exiting read loop without error */
  int cnt;                      /* work variable */
  int pipetype = 0;             /* 0=print file, 1=hdr page, 2=trailer page */
  int shr_fd;                   /* file desc for shared file */
  long chargecnt;                /* count of pages for billing */

/* AIX security enhancement */
  int bot_vsize  = 0;		/* size of bottom label */
  int islabel   = 0;		/* label flag, if > 0 then label exists */
  int logical_fl = 0;		/* true form length */
  int prev_vpos  = 0;		/* previous vert. position value */
  int top_vsize  = 0;		/* size of top label */
  int savpgskip  = 0;		/* temporary storage for piopgskip */
  int c;                        /* temporary storage for fgetc */
  int chk_zero_ln = FALSE;
  int no_cr_lf = FALSE;		/* flag set if last char in file is FF */

  struct label_attr bot_lattr;	/* struct describing bottom label */
  struct label_attr top_lattr;	/* struct describing top label */
/* TCSEC Division C Class C2 */

  if (statusfile) {
      pipetype = atoi(getenv("PIOPIPETYPE"));
      shr_fd = atoi(getenv("PIOSHRFILDES"));
      (void) read(shr_fd, &chargecnt, sizeof(long));
      (void) lseek(shr_fd, (off_t) 0, SEEK_SET);

      lockdat.l_type = F_WRLCK;
      (void) fcntl(3, F_SETLKW, &lockdat);
      if (log_init() >= 0)
	  (void) log_charge(++chargecnt);     /* assume at least one page */
      lockdat.l_type = F_UNLCK;
      (void) fcntl(3, F_SETLKW, &lockdat);

      (void) write(shr_fd, &chargecnt, sizeof(long)); /*save cumulative count*/
      (void) lseek(shr_fd, (off_t) 0, SEEK_SET);
  }
  
piomode = PIO_SECUR_MODE;
  if (sh_pl < 0)
      ERREXIT(0, MSG_PAGELEN, NULL, NULL, 0)  /*(don't use semicolon)*/
  else
  {
    /* If top margin is < 0, subtrack its absolute value from the page length */
    if (sh_tmarg < 0 && (sh_tmarg += sh_pl, sh_tmarg < 0))
	sh_tmarg = 0;

    if (sh_pl == 0) {            /* page length of zero means don't paginate */
	sh_pl = SPECPL;           /* leave room for reverse line feeds */
	chk_zero_ln = TRUE;
    }

    /* make sure top and bottom margins and page length are compatible */
    if (sh_tmarg < 0 || sh_tmarg >= sh_pl)
	ERREXIT(0, MSG_TOPMARG, NULL, NULL, 0);
    if (sh_bmarg < 0 || sh_bmarg >= sh_pl)
	ERREXIT(0, MSG_BOTMARG, NULL, NULL, 0);
    if (sh_tmarg + sh_bmarg >= sh_pl)
	ERREXIT(0, MSG_TOPBOT, NULL, NULL, 0);
  }
piomode = PIO_CURR_MODE;

/* AIX security enhancement */

  savpgskip = piopgskip;
  piopgskip = -1;

  /*  BEGIN C2 TOP & BOT INITILIZATION  */
  top_lattr.label_type = LAB_TOP;
  bot_lattr.label_type = LAB_BOTTOM;

  if (security)
  {
    if (getlabel(&top_lattr) > 0)
    {
      top_vsize = print_label(&top_lattr, 0);
      islabel++;
    }
 
    if (getlabel(&bot_lattr) > 0)
    {
      bot_vsize = print_label(&bot_lattr, 0);
      islabel++;
    }

    if (bot_vsize + top_vsize + sh_tmarg + sh_bmarg >= sh_pl)
	ERREXIT(0, MSG_SECUR, NULL, NULL, 0);
  }

  sh_pl -= sh_bmarg;
  logical_fl = sh_pl - bot_vsize;
  piopgskip = savpgskip;

  /*  END C2 TOP & BOT INITILIZATION  */
  sh_vpos = sh_tmarg;

  /* call formatter's routine that sends initialization commands */
  piomode = PIO_CURR_MODE;
  if (piopgskip == 0)
      (void) initialize();

  piomode = PIO_SECUR_MODE;
  /* Take care of top margin */
  if ((retcode = vspace(0, sh_tmarg)) < 0)
  {
    piomode = PIO_CURR_MODE;
    return(retcode);
  } 
  piomode = PIO_CURR_MODE;

  if (top_vsize > 0)
    (void) print_label(&top_lattr, 1);
  prev_vpos = sh_vtab_base = sh_vpos;

/* TCSEC Division C Class C2 */

  do
  {
    /* if true we are somewhere in the middle of the page */
    if (sh_vpos < logical_fl)
    {
      count = lineout(stdin);	  	/* give line to output */
      vpos_chg = sh_vpos - prev_vpos;
      if (logical_fl - sh_vpos < (logical_fl/1000)*2)  /* for roundoff error */
	  sh_vpos = logical_fl;
      if (!(count || vpos_chg) || (c=fgetc(stdin)) == EOF)
      { 
         if (sh_vpos!=SPECPL)
               exit_complete=1;
	 else if (sh_vpos == SPECPL && chk_zero_ln == TRUE)	/* Check if */
	       ff_exit_loop=1;		/* last char is form feed and -l=0 */
					/* if so, then set flag for later */
      } else
         ungetc(c,stdin);

      if (!exit_complete)
      {
	if (islabel || (sh_vpos < logical_fl))
	{
	  if (sh_vpos < sh_vtab_base)
	    sh_vpos = sh_vtab_base;    /* don't back up if already at top */
	  if (sh_vpos < prev_vpos)
	  {
	    /* reverse line feed */
	    if (sh_vdecr_cmd != NULL && *sh_vdecr_cmd != '\0')
	      for (cnt = prev_vpos; cnt > sh_vpos; cnt -= sh_vdecr)
		(void) piocmdout(sh_vdecr_cmd, NULL, 0, NULL);
	  }
	  else
	  {
	    (void) vspace(prev_vpos, sh_vpos);
	  }
	  prev_vpos = sh_vpos;
	}
      }
    }
    /* we are at the end of the page */
    else
    {
      if (!(count || vpos_chg) || (c=fgetc(stdin)) == EOF)
      { 
         if (sh_vpos==SPECPL)
              ff_exit_loop=1;
         else 
              exit_complete=1;
      } else
         ungetc(c,stdin);

      if (!exit_complete)
      {	
	if (bot_vsize > 0)
	  (void) print_label(&bot_lattr, 1);

	(void) piocmdout(sh_ff_cmd, NULL, 0, 0);

	if (piopgskip)
	  if (--piopgskip == 0)
	    (void) initialize(); /* formatter's routine to initialize printer*/

	/* Determine if we need to do the spacing on the next page */
	if ((sh_vpos - logical_fl) > 0 &&  /* if would have gone off page */
	(sh_vpos - prev_vpos) < (logical_fl - sh_tmarg)) /* and it will fit */
	{
	  /* do the spacing on the next page */
	  sh_vpos = sh_tmarg + (sh_vpos - prev_vpos);
	  (void) vspace(sh_tmarg, sh_vpos);
	  prev_vpos = sh_vpos;
	  continue;
	}

	sh_vpos = prev_vpos = sh_tmarg;

	piomode = PIO_SECUR_MODE;
        /* Take care of top margin */
	(void) vspace(0, sh_tmarg);
	piomode = PIO_CURR_MODE;

	if (top_vsize > 0)
	  (void) print_label(&top_lattr, 1);
      }

      if (statusfile) {
	if (pipetype == 1)
	  cnt = 0;            /* header page */
	else if (pipetype == 0)
	  cnt = ++pagecount;  /* print file */
	else
	   cnt = pagecount;   /* trailer page */

    	lockdat.l_type = F_WRLCK;
    	(void) fcntl(3, F_SETLKW, &lockdat);
	if (log_init() >= 0) {
	    (void) log_pages(cnt);    /* for status */
	    (void) log_charge(++chargecnt);
	}	
    	lockdat.l_type = F_UNLCK;
    	(void) fcntl(3, F_SETLKW, &lockdat);

	(void) write(shr_fd, &chargecnt, sizeof(long));  /* cumulative count */
	(void) lseek(shr_fd, (off_t) 0, SEEK_SET);
      }
      if (ff_exit_loop)			/* if form feed error reset variables */
      {
	if (chk_zero_ln == TRUE)	/* If length is zero and last char is */
		no_cr_lf = TRUE;	/* FF then we do not want an extra */
					/* CR LF */
	sh_vpos = sh_vincr;
        vpos_chg = sh_vpos - prev_vpos;
	exit_complete = 1;
      }
	
    }
  } while ((count || vpos_chg) && !exit_complete);

  /* print last bottom label */
  if (sh_vpos > sh_tmarg || count > 0)
  {
    /* there is something on the page */
/* AIX security enhancement */
    if (bot_vsize > 0)
    {
      vspace(prev_vpos, logical_fl);
      sh_vpos = logical_fl;
      (void) print_label(&bot_lattr, 1);
    }
/* TCSEC Division C Class C2 */
    if (piodatasent)
      if (sh_ff_at_eof)
	(void) piocmdout(sh_ff_cmd, NULL, 0, 0);
      else if (bot_vsize == 0 && no_cr_lf == FALSE)
	(void) vspace(prev_vpos, sh_vpos);
    if (statusfile) {
      if (pipetype == 1)
	cnt = 0;            /* header page */
      else if (pipetype == 0)
	cnt = ++pagecount;  /* print file */
      else
	 cnt = pagecount;   /* trailer page */

      lockdat.l_type = F_WRLCK;
      (void) fcntl(3, F_SETLKW, &lockdat);
      if (log_init() >= 0)
	  (void) log_pages(cnt);    /* for status */
      lockdat.l_type = F_UNLCK;
      (void) fcntl(3, F_SETLKW, &lockdat);
    }
  }

  /* call formatter's routine that sends restore commands */
  piomode = PIO_CURR_MODE;
  (void) restore();

}


/*******************************************************************************
*                                                                              *
* NAME:           vspace                                                       *
*                                                                              *
* DESCRIPTION:    Send commands to printer to vertically advance the paper.    *
*                                                                              *
* PARAMETERS:   vpos - current print position on page.                         *
*               dest - target print position on page.                          *
*                                                                              *
* GLOBALS:                                                                     *
*     REFERENCED: sh_vincr - vertical increment value.                         *
*                 sh_vincr_cmd - vertical increment command.                   *
*                                                                              *
* RETURN VALUES:  >= 0    number of bytes output by cmdout.                    *
*                 <  0    piocmdout failure.                                   *
*                                                                              *
*******************************************************************************/

typedef long DIST_T;          /* size for distance variables in 1440 units */

vspace(vpos, dest)

    DIST_T vpos;                /* intermediate vertical position */
    DIST_T dest;                        /* destination vertical move */
{
    int i;
    int retval  = 0;            /* return value */

    if (sh_vincr_cmd != NULL && *sh_vincr_cmd != '\0')
	for (i = vpos; i < dest; i += sh_vincr)
	{
	    if ((dest - i) < (sh_vincr / 2))  /* round off to nearest one */
		break;
	    (void) piocmdout(sh_vincr_cmd, NULL, 0, NULL);
	}
    return(0);
}



/*******************************************************************************
*									       *
* NAME: 	  get_local_args					       *
*									       *
* DESCRIPTION:	  Processes all pioformat specific flags and removes their     *
*		  entries from the the argument vector so that the formatter   *
*		  will not stumble over them.				       *
*									       *
* PARAMETERS:	  ac - argument count					       *
*		  av - argument vector					       *
*		  options - recognized argument flags			       *
*									       *
* RETURN VALUES:  >0 success      - number of tokens processed                 *
*                  0 failure      - flag not found (important for -@)          *
*                 <0 failure      - Logical printer name bad (-@)              *
*									       *
*******************************************************************************/

get_local_args(ac, av, options)
    int ac;
    char *av[], *options;
{
    char tmpstr[4];
    char *cp;
    int numtokens;      /* number of tokens for the flag & its arg */

    int ch	 = 0;	/* flag name */
    int argfound = 0;	/* indicates that an flag was found */
    int badarg   = 0;	/* indicates that the argument of a flag was bad */
    int index	 = 0;	/* argument vetcor index */
    int count	 = 0;	/* number of arg vector items deleted */
    int prev_optind;    /* previous value of optind */

    opterr = 0;  /* don't want error message to stderr */
    optind = prev_optind = 1;  /* start with first flag */

    while ((ch = (int) getopt(ac, av, ALLFLAGS)) != EOF)
    {
	if ((cp = strchr(options, (char)ch)) == NULL)
	    goto NEXTFLAG;

	switch(ch)
	{
		/* Name (or partial name) of data base file */
	    case '@':
		if (*optarg == '/')
		{
		    /* full path name */
		    (void) strcpy(odm_filepath, optarg);
		    argfound++;
		}
		else
		{
		    /* partial name of data base file, we must find the rest */
		    /* i.e., we have queue name and maybe virtual printer    */
		    /* (queue device) name                                   */
		    char *fragname;	/* fragment of odm data file name */
		    int  fraglen;	/* length of fragment */
		    int  namelen;	/* length of file name in our dir */ 

		    DIR *dirp;		/* directory pointer */

		    struct dirent *dp;	/* points to a particular dir entry */

		    fragname = optarg;
		    fraglen = strlen(fragname); 

                    /* Determine the base directory */
                    if ((basedir = getenv("PIOVARDIR")) == NULL)
                        basedir = defbasedir;    /* default base directory */

		    strcpy(odm_filepath, basedir);
		    strcat(odm_filepath, "/ddi/");
		    dirp = opendir(odm_filepath);
		    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		    {
			namelen = strlen(dp->d_name); 
			if (!strcmp(dp->d_name + (namelen - fraglen),fragname))
			    break;
		    }

		    if (dp == NULL)
		    {
			strcat(odm_filepath, "*");   /* for error message */
			strcat(odm_filepath, fragname);
			badarg++;
		    }
		    else
		    {
			strcat(odm_filepath, dp->d_name);
			argfound++;
		    }
		    closedir(dirp);
		}
		break;

		/* formatter name */
	    case '!':
		(void) strcpy(formatter, optarg);
		gotfmtrname = TRUE;  /* remember that we got a formatter name */
		argfound++;
		break;

		/* passthru indicator */
	    case '#':
		if (!strcmp(optarg, YES_STRING))
		    pthruval = TRUE;
		else if (!strcmp(optarg, NO_STRING))
		    pthruval = FALSE;
		else
		    pthruval = atoi(optarg);
		argfound++;
		break;

		/* indicates whether to override security labels or not */
	    case '$':
		if (!strcmp(optarg, YES_STRING))
		    security = TRUE;
		else if (!strcmp(optarg, NO_STRING))
		    security = FALSE;
		else
		    security = atoi(optarg);
		argfound++;
		break;

		/* entry not found */
	    case '?':
		break;

	}
	if (argfound)
	{
	    /*  here we delete the flag and argument entry from the argument
		vector because the formatter will not recognize these flags. */
	    argfound--;
	    numtokens = optind - prev_optind;
	    for (index = optind; index <= ac; index++)
		av[index - numtokens] = av[index];
	    count += numtokens;
	    ac -= numtokens;
	    optind -= numtokens;
	}
	else if (badarg || (optind > ac))
	{
	    count = -1;
	    break;
	}
NEXTFLAG:
	prev_optind = optind;
    }
    return(count);
}

