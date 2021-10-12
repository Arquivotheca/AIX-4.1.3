static char sccsid[] = "@(#)76  1.5  src/bos/usr/lib/pios/piolsvp.c, cmdpios, bos411, 9428A410j 4/25/94 18:53:27";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  main(), lvp_getmsg(), lvp_msg(), lvp_bld_atlist(),
 *             lvp_bld_qlist(), lvp_at_select(), lvp_q_select(),
 *             lvp_parsemsg(), lvp_parsemsg1(), lvp_at_select1()
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <nl_types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <piobe_msg.h>
#include "common.h"
#define	MAXLINE		LINE_MAX
#include "qcadm.h"
#undef	MAXLINE

/* External declarations */
extern int		scandir(char *, struct dirent *(*[]),
				int (*)(struct dirent *),
				int (*)(struct dirent **, struct dirent **));
					/* should've been in <sys/dir.h> */
extern int		alphasort(struct dirent **, struct dirent **);
					/* should've been in <sys/dir.h> */
extern int		optopt;		/* should've been in <stdlib.h> */
extern void		read_qconfig(struct quelist **);

/* Misc. macros */
#define STRGZARG(s)		#s
#define STRGZ(s)		STRGZARG(s)
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc/"
#define MF_PIOBESETNO		(8)
#define MF_PIOATTR1SETNO	(1)
#define qFLAG			0
#define vFLAG			1
#define AFLAG			2
#define nFLAG			3
#define PFLAG			4
#define QFLAG			5
#define NFLAG			6
#define pFLAG			7
#define dFLAG			8
#define DEFBASEDIR		"/usr/lib/lpd/pio"
#define DEFVARDIR		"/var/spool/lpd/pio/@local"
#define ENVBASEDIR		"PIOBASEDIR"
#define ENVVARDIR		"PIOVARDIR"
#define ATFMT			"  %-20.20s %-56.56s"
#define QQDEVMAXLEN		26
#define QFMT			"  %-" STRGZ(QQDEVMAXLEN) "." \
				STRGZ(QQDEVMAXLEN) "s %-50.50s"
#define QDFMT			"  %-20.20s %-20.20s %-35.35s"
#define SQFMT			"%-20.20s %-20.20s %-37.37s"
#define SATFMT			"%-42.42s"
#define dFMT			"  %-20.20s %-56.56s"
#define ATFDIR			"/etc"
#define PREDEFDIR		"/predef"
#define CUSTOMDIR		"/custom"
#define ATFSUFFIX		".attach"
#define CFGSUFFIX		".config"
#define GENATNAME		"other"
#define WHSPCHRS		" \t"
#define WHSPCHRS1		" \t="
#define ASSGCHR			'='
#define CMNTCHR			'#'
#define ATFDESCKW		"description"
#define ATFSNOKW		"seq_num"
#define SETBEGINCHR		'['
#define SETENDCHR		']'
#define MSGSEPCHR		';'
#define FLDSEPCHR		','
#define CUSFSEPCHR		':'
#define SUBATSEP		'.'
#define CFGMCHR			'm'
#define CFGDSCFLDNO		(8)
#define ATTRNMLEN		2
#define ATAT			"zA"
#define ATmn			"mn"
#define ATmL			"mL"
#define ATmt			"mt"
#define ATmA			"mA"
#define LSALLQDEV		"/bin/lsallqdev"
#if defined(MIN)
#undef MIN
#endif		/* MIN */
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#define ARRAYSIZ(a)		(sizeof(a)/sizeof(*a))
#define SETAFLAG(i, f)		(i) |= 0x1U<<(f)
#define ISAFLAGSET(i, f)	((i)&0x1U<<(f))
#define BOOL(a)			((a) ? TRUE : FALSE)
#define MALLOC(p, sz)		do \
				{ \
				   if (!((p) = malloc((size_t)(sz)))) \
				   { \
         			      lvp_msg(stderr, MSG_LVP_MALLOCERR, \
					      DEFMSG_LVP_MALLOCERR, progname, \
					      strerror(errno)); \
				      exit(EXIT_FAILURE); \
				   } \
				} while (0)

/* Macros for default messages */
#define DEFMSG_LVP_ERRMSG	"Cannot access the message catalog " \
				MF_PIOBE ".\n"
#define DEFMSG_LVP_USAGE	"Usage:\t%s { -q | -v | -Q | -p | -A } " \
				"[ -n SmitSelectorName ]\n" \
				"\t\t-P Queue[:QueueDevice] -n " \
				"SmitSelectorName\n" \
				"\t\t-P Queue -d\n" \
				"\t\t-N AttachmentType -n SmitSelectorName\n"
#define DEFMSG_LVP_MISSARG	"%s: Missing argument for the flag -%c\n"
#define DEFMSG_LVP_ILLOPT	"%s: Illegal flag -%c\n"
#define DEFMSG_LVP_ILLUSE	"%s: Illegal usage\n"
#define DEFMSG_LVP_MALLOCERR	"%s: Error '%s' in malloc()\n"
#define DEFMSG_LVP_ATHDR	"# ATTACHMENT TYPE      DESCRIPTION\n"
#define DEFMSG_LVP_QHDR		"# PRINT QUEUE                " \
				"DESCRIPTION\n"
#define DEFMSG_LVP_qHDR		"# PRINT QUEUE[:PRINTER]      " \
				"DESCRIPTION\n"
#define DEFMSG_LVP_vHDR		"# PRINT QUEUE          PRINTER     " \
				"         DESCRIPTION\n"
#define DEFMSG_LVP_dHDR		"# PRINTER              DESCRIPTION\n"
#define DEFMSG_LVP_DOPENERR	"%s: Error '%s' in opening the directory %s\n"
#define DEFMSG_LVP_DRDERR	"%s: Error '%s' in reading the directory %s\n"
#define DEFMSG_LVP_FOPENERR	"%s: Error '%s' in opening the file %s\n"
#define DEFMSG_LVP_FRDERR	"%s: Error '%s' in reading the file %s\n"
#define DEFMSG_LVP_POPENERR	"%s: Error '%s' in opening a pipe to command "\
				"%s\n"
#define DEFMSG_LVP_PRDERR	"%s: Error '%s' in reading a pipe to command "\
				"%s\n"
#define DEFMSG_LVP_DNMNOTFND	"%s: Device name (mn) not found in %s\n"
#define DEFMSG_LVP_PTYPENOTFND	"%s: Printer type (mt) not found in %s\n"
#define DEFMSG_LVP_NOVPSFND	"%s: No virtual printers found for the queue "\
				"%s\n"
#define DEFMSG_LVP_GENATDESC	"User Defined Backend"

/* Typedefs for queue and attatchment lists */
typedef struct dele {
   struct dele		*nextp;		/* next element */
   const char		*dvnm;		/* device name */
   const char		*desc;		/* queue-device description */
   uchar_t		disp;		/* display flag */
} dele_t;
typedef struct qele {
   struct qele		*nextp;		/* next element */
   struct qele		*prevp;		/* prev element */
   const char		*qnm;		/* queue name */
   dele_t		*dp;		/* device list */
   ushort_t		nodvs;		/* no. of devices */
} qele_t;
typedef struct atele {
   struct atele		*nextp;		/* next element */
   const char		*atnm;		/* attachment type */
   const char		*desc;		/* attachment description */
   uchar_t		disp;		/* display flag */
   short		sno;		/* sequence no. */
} atele_t;
typedef struct {
   const char		*qnm;		/* queue name */
   const char		*dvnm;		/* device name */
   const char		*atnm;		/* attachment type */
   const char		*slval;		/* selector value name */
} sqele_t;
typedef struct {
   const char		*atnm;		/* attachment type */
   const char		*slval;		/* selector value name */
} satele_t;

/* Local function declarations */
static const char	*lvp_getmsg(const char *, int, int);
static void 		lvp_msg(FILE *, int, const char *, ...);
static int		lvp_bld_atlist(atele_t **, int, const char *);
static int		lvp_bld_qlist(qele_t **, const atele_t *, const char *,
				      int, int);
static int		lvp_at_select(struct dirent *);
static int		lvp_at_select1(struct dirent *);
static int		lvp_q_select(struct dirent *);
static const char	*lvp_parsemsg(const char *);
static char		*lvp_parsemsg1(const char *);
static sqele_t		*lvp_get_sqinfo(const char *, const char *);
static satele_t		*lvp_get_satinfo(const char *, const char *);

/* Local variable definitions */
const char		*progname;
static char		bdir[PATH_MAX+1];
static char		vdir[PATH_MAX+1];




/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           main                                                         *
*                                                                              *
* DESCRIPTION:    Perform main logic.                                          *
*                                                                              *
* PARAMETERS:     ac		arg count                                      *
*                 av		arg vector                                     *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
int
main(int ac, char **av)
{
   register int			i;
   uint_t			aflags 		= 0U;	/* arg flags */
   const char			*smitslnm 	= NULL;	/* smit selector name */
   atele_t			*atheadp	= NULL;	/* at element head */
   qele_t			*qheadp; 		/* q element head */
   const char			*cp;
   register const atele_t	*ap;
   /* Commented out, since we're not doing memory free later
   register const atele_t	*ap1;
    */
   register const qele_t	*qp;
   /* Commented out, since we're not doing memory free later
   register const qele_t	*qp1;
    */
   register const dele_t	*dp;
   /* Commented out, since we're not doing memory free later
   register const dele_t	*dp1;
    */
   const char			*sqnm;			/* single queue name */
   sqele_t			*sqp = NULL;		/* single queue info */
   const char			*satnm;			/* single att. type */
   satele_t			*satp = NULL;		/* single AT info */
   
   (void) setlocale(LC_ALL, "");

   /* Process the arguments and flags. */
   progname = *av;
   for (opterr = 0, optind = 1;
	(i = getopt((int)ac, av, ":qvQAn:P:N:pd")) != EOF; )
      switch (i)
      {
	 case 'q':
	    SETAFLAG(aflags, qFLAG);
	    break;

	 case 'v':
	    SETAFLAG(aflags, vFLAG);
	    break;

	 case 'Q':
	    SETAFLAG(aflags, QFLAG);
	    break;

	 case 'A':
	    SETAFLAG(aflags, AFLAG);
	    break;

	 case 'n':
	    SETAFLAG(aflags, nFLAG);
	    smitslnm = optarg;
	    break;

	 case 'P':
	    SETAFLAG(aflags, PFLAG);
	    sqnm = optarg;
	    break;

	 case 'N':
	    SETAFLAG(aflags, NFLAG);
	    satnm = optarg;
	    break;

	 case 'p':
	    SETAFLAG(aflags, pFLAG);
	    break;

	 case 'd':
	    SETAFLAG(aflags, dFLAG);
	    break;

	 case ':':
	    lvp_msg(stderr, MSG_LVP_MISSARG, DEFMSG_LVP_MISSARG, progname,
		    (char)optopt),
	    lvp_msg(stdout, MSG_LVP_USAGE, DEFMSG_LVP_USAGE, progname);
	    exit(EXIT_FAILURE);

	 case '?':
	    lvp_msg(stderr, MSG_LVP_ILLOPT, DEFMSG_LVP_ILLOPT, progname,
		    (char)optopt),
	    lvp_msg(stdout, MSG_LVP_USAGE, DEFMSG_LVP_USAGE, progname);
	    exit(EXIT_FAILURE);
      }
   if ((BOOL(ISAFLAGSET(aflags, qFLAG))+BOOL(ISAFLAGSET(aflags, vFLAG))+
	BOOL(ISAFLAGSET(aflags, QFLAG))+BOOL(ISAFLAGSET(aflags, pFLAG))+
        BOOL(ISAFLAGSET(aflags, AFLAG))+BOOL(ISAFLAGSET(aflags, PFLAG))+
	BOOL(ISAFLAGSET(aflags, NFLAG)) != 1)
       || (ISAFLAGSET(aflags, PFLAG) && !(ISAFLAGSET(aflags, nFLAG) ||
					  ISAFLAGSET(aflags, dFLAG)))
       || (ISAFLAGSET(aflags, NFLAG) && !ISAFLAGSET(aflags, nFLAG)))
   {
      lvp_msg(stderr, MSG_LVP_ILLUSE, DEFMSG_LVP_ILLUSE, progname),
      lvp_msg(stdout, MSG_LVP_USAGE, DEFMSG_LVP_USAGE, progname);
      exit(EXIT_FAILURE);
   }
   if ((cp = getenv(ENVBASEDIR))  ||  (cp = getenv(ENVVARDIR)))
   {
      (void) strncpy(bdir, cp, sizeof(bdir)-1),
      *(bdir+sizeof(bdir)-1) = 0;
      (void) strncpy(vdir, cp, sizeof(vdir)-1),
      *(vdir+sizeof(vdir)-1) = 0;
   }
   else
   {
      (void) strncpy(bdir, DEFBASEDIR, sizeof(bdir)-1),
      *(bdir+sizeof(bdir)-1) = 0;
      (void) strncpy(vdir, DEFVARDIR, sizeof(vdir)-1),
      *(vdir+sizeof(vdir)-1) = 0;
   }

   /* Build a list of attachment types, if needed. */
   if ((ISAFLAGSET(aflags, nFLAG) && !(ISAFLAGSET(aflags, PFLAG) ||
				       ISAFLAGSET(aflags, NFLAG))  ||
	ISAFLAGSET(aflags, AFLAG))  &&
       !lvp_bld_atlist(&atheadp, ISAFLAGSET(aflags, nFLAG), smitslnm))
      exit(EXIT_FAILURE);

   /* Display the list of attachment types, if specified. */
   if (ISAFLAGSET(aflags, AFLAG))
   {
      lvp_msg(stdout, MSG_LVP_ATHDR, DEFMSG_LVP_ATHDR, progname);
      for (ap = atheadp; ap; ap = ap->nextp)
	 if (ap->disp)
	    (void) printf(ATFMT "\n", ap->atnm, ap->desc);
      (void) fflush(stdout);
   }

   /* Build and display a list of queues and devices, if specified. */
   if (ISAFLAGSET(aflags, qFLAG)  ||  ISAFLAGSET(aflags, vFLAG)  ||
       ISAFLAGSET(aflags, QFLAG)  ||  ISAFLAGSET(aflags, pFLAG))
   {
      if (!lvp_bld_qlist(&qheadp, atheadp, (const char *)NULL,
			 ISAFLAGSET(aflags, nFLAG), ISAFLAGSET(aflags, vFLAG)))
         exit(EXIT_FAILURE);
      if (ISAFLAGSET(aflags, qFLAG))
      {
         lvp_msg(stdout, MSG_LVP_qHDR, DEFMSG_LVP_qHDR, progname);
         for (qp = qheadp; qp; qp = qp->nextp)
	    for (dp = qp->dp; dp; dp = dp->nextp)
	       if (dp->disp)
	       {
	          (void) printf(QFMT "\n", qp->qnm, dp->desc);
	          break;
	       }
         for (qp = qheadp; qp; qp = qp->nextp)
	 {
	    uint_t		nodisps = 0;

	    if (qp->nodvs > 1)
	       for (dp = qp->dp; dp; dp = dp->nextp)
	          if (dp->disp)
	             nodisps++;
	    if (nodisps > 1)
	       for (dp = qp->dp; dp; dp = dp->nextp)
	          if (dp->disp)
		  {
		     char	s[QQDEVMAXLEN+1];

		     (void) strncpy(s, qp->qnm, sizeof(s)-2),
		     *(s+sizeof(s)-2) = 0;
		     (void) strcat(s, ":");
		     (void) strncat(s, dp->dvnm, sizeof(s)-strlen(s)-1),
		     *(s+sizeof(s)-1) = 0;
	       	     (void) printf(QFMT "\n", s, dp->desc);
		  }
	 }
      }
      else if (ISAFLAGSET(aflags, vFLAG))
      {
         lvp_msg(stdout, MSG_LVP_vHDR, DEFMSG_LVP_vHDR, progname);
         for (qp = qheadp; qp; qp = qp->nextp)
	    for (dp = qp->dp; dp; dp = dp->nextp)
	       if (dp->disp)
	       	  (void) printf(QDFMT "\n", qp->qnm, dp->dvnm, dp->desc);
      }
      else if (ISAFLAGSET(aflags, QFLAG))
      {
         lvp_msg(stdout, MSG_LVP_QHDR, DEFMSG_LVP_QHDR, progname);
         for (qp = qheadp; qp; qp = qp->nextp)
	    for (dp = qp->dp; dp; dp = dp->nextp)
	       if (dp->disp)
	       {
	          (void) printf(QFMT "\n", qp->qnm, dp->desc);
	          break;
	       }
      }
      else				/* pFLAG */
      {
         lvp_msg(stdout, MSG_LVP_vHDR, DEFMSG_LVP_vHDR, progname);
         for (qp = qheadp; qp; qp = qp->nextp)
	    for (i = 0, dp = qp->dp; dp; dp = dp->nextp)
	       if (dp->disp)
	       	  (void) printf(QDFMT "\n", i ? "" : (++i, qp->qnm),
				dp->dvnm, dp->desc);
      }
      (void) fflush(stdout);
   }

   /* Fetch and display a single queue information, if requested. */
   if (ISAFLAGSET(aflags, PFLAG))
      if (ISAFLAGSET(aflags, nFLAG))
      {
         if (!(sqp = lvp_get_sqinfo(sqnm, smitslnm)))
	    exit(EXIT_FAILURE);
         (void) printf(SQFMT "\n", sqp->dvnm, sqp->atnm, sqp->slval);
         (void) fflush(stdout);
      }
      else				/* dFLAG */
      {
         if (!lvp_bld_qlist(&qheadp, (atele_t *)NULL, sqnm, FALSE, FALSE))
            exit(EXIT_FAILURE);
	 if (qheadp)
	 {
            lvp_msg(stdout, MSG_LVP_dHDR, DEFMSG_LVP_dHDR, progname);
	    for (dp = qheadp->dp; dp; dp = dp->nextp)
	       if (dp->disp)
	          (void) printf(dFMT "\n", dp->dvnm, dp->desc);
	 }
      }

   /* Fetch and display a single attachment type information, if requested. */
   if (ISAFLAGSET(aflags, NFLAG))
   {
      if (!(satp = lvp_get_satinfo(satnm, smitslnm)))
	 exit(EXIT_FAILURE);
      (void) printf(SATFMT "\n", satp->slval);
      (void) fflush(stdout);
   }

   /* Emancipate all the memory usurped.  Really not necessary, since we're
      exiting anyway.  But, we might as well do it as a good practice. */
   /* Perhaps not.  Let's skip this for the sake of speed. */
   /****
   for (ap = atheadp; ap; ap = ap1)
   {
      ap1 = ap->nextp;
      free((void *)ap->atnm);
      if (ap->desc)
	 free((void *)ap->desc);
      free((void *)ap);
   }
   for (qp = qheadp; qp; qp = qp1)
   {
      qp1 = qp->nextp;
      free((void *)qp->qnm);
      for (dp = qp->dp; dp; dp = dp1)
      {
	 dp1 = dp->nextp;
	 free((void *)dp->dvnm);
         if (dp->desc)
	    free((void *)dp->desc);
	 free((void *)dp);
      }
      free((void *)qp);
   }
   if (sqp)
   {
      free((void *)sqp->qnm);
      free((void *)sqp->dvnm);
      if (sqp->atnm)
         free((void *)sqp->atnm);
      if (sqp->slval)
         free((void *)sqp->slval);
   }
   if (satp  &&  satp->slval)
      free((void *)satp->slval);
   ****/

   return EXIT_SUCCESS;
}	/* end - main() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_getmsg                                                   *
*                                                                              *
* DESCRIPTION:    Fetch an error message and put it in a msg buffer.           *
*                                                                              *
* PARAMETERS:     catnm		catalog name                                   *
*                 setno		set no.                                        *
*                 msgno		message no.                                    *
*                                                                              *
* RETURN VALUES:  message	success                                        *
*                 NULL		failure                                        *
*                                                                              *
*******************************************************************************/
static char const *
lvp_getmsg(const char *catnm, int setno, int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

   /* Fetch the specified message. */
   if (strcmp(catnm, prevcatnm))
   {
      (void) strncpy(prevcatnm, catnm, sizeof(prevcatnm)-1),
      *(prevcatnm+sizeof(prevcatnm)-1) = 0;
      if (md != (nl_catd)-1)
         md = ((void) catclose(md), (nl_catd)-1);
   }
   if (md == (nl_catd)-1  &&
       (md = catopen((char *)catnm, NL_CAT_LOCALE)) == (nl_catd)-1)
   {
      (void) strncpy(defmcpath, DEFMC_PREFIXPATH, sizeof(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      (void) strncat(defmcpath, catnm, sizeof(defmcpath)-strlen(defmcpath)-1);
      if ((md = catopen(defmcpath, NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md, setno, msgno, (char *)dmsg),
		 (char const *)dmsg) ? mp : NULL;
}	/* end - lvp_getmsg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_msg                                                      *
*                                                                              *
* DESCRIPTION:    Fetch a message and put it in a given file.                  *
*                                                                              *
* PARAMETERS:     outfp		output file pointer                            *
*                 msgno		message no.                                    *
*                 defmsg	default error message                          *
*                 ...		(stdarg)                                       *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static void
lvp_msg(FILE *outfp, int msgno, const char *defmsg, ...)
{
   va_list		vap;
   const char		*msgp = lvp_getmsg(MF_PIOBE, MF_PIOBESETNO, msgno);

   /* Parse the message with the specified args. */
   if (!msgp)
      msgp = defmsg;
   va_start(vap, defmsg);
   (void) vfprintf(outfp, msgp, vap);
   va_end(vap);

   return;
}	/* end - lvp_msg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_bld_atlist                                               *
*                                                                              *
* DESCRIPTION:    Build a list of attachment types.  If a smit selector name   *
*                 is passed, exclude those that do not specify this selector   *
*                 in their *.attach files.                                     *
*                                                                              *
* PARAMETERS:     atpp		pointer to atlist                              *
*                 srch		flag to search for a smit selector             *
*                 slnm		smit selector name                             *
*                                                                              *
* RETURN VALUES:  0		success                                        *
*                 !0		failure                                        *
*                                                                              *
*******************************************************************************/
static int
lvp_bld_atlist(atele_t **app, int srch, const char *slnm)
{
#define ADD_ELE_TO_LIST(lp,ep,tp)	do \
					{ \
					   if (!(lp)) \
					      (tp) = (lp) = (ep); \
					   else \
					      (tp)->nextp = (ep), \
					      (tp) = (ep); \
					} while (0)
   atele_t			*ap;
   atele_t			*attailp;
   struct dirent		**dpp;
   char				atdir[PATH_MAX+1];
   char				atfnm[PATH_MAX+1];
   register int			noates;
   FILE				*fp;
   char				s[LINE_MAX];
   register char		*cp;
   register char		*cp1;
   register char const		*ccp;
   char				*desc;
   uchar_t			descfnd;
   uchar_t			slfnd;
   uchar_t			snofnd;
   short			sno;
   ptrdiff_t			len;
   register int			i;
   register struct dirent	**tdpp;

   /* Build a list of attachment types and their descriptions.
      Initially, get a list of attachment ('.attach') files from the .../etc
      dir.  Then, read the relevant information from these files.
      Also if a SMIT selector name field is specified, search for this
      field in the files. */
   *app = NULL;
   (void) strncpy(atdir, bdir, sizeof(atdir)-strlen(ATFDIR)-1),
   *(atdir+sizeof(atdir)-strlen(ATFDIR)-1) = 0;
   (void) strcat(atdir, ATFDIR);
   if ((noates = scandir(atdir, &dpp, lvp_at_select, alphasort)) == -1)
   {
      lvp_msg(stderr, MSG_LVP_DRDERR, DEFMSG_LVP_DRDERR, progname,
	      strerror(errno), atdir);
      return FALSE;
   }
   for (i = noates, tdpp = dpp; i-- > 0; )
   {
      (void) strncpy(atfnm, atdir, sizeof(atfnm)-2),
      *(atfnm+sizeof(atfnm)-2) = 0;
      (void) strcat(atfnm, "/");
      (void) strncat(atfnm, (*tdpp)->d_name, sizeof(atfnm)-strlen(atfnm)-1),
      *(atfnm+sizeof(atfnm)-1) = 0;
      if (!(fp = fopen(atfnm, "r")))
      {
         lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
		 strerror(errno), atfnm);
	 return FALSE;
      }
      for (desc = NULL, descfnd = FALSE, slfnd = !srch, snofnd = FALSE,
	   sno = SHRT_MAX;
	   !(descfnd && slfnd && snofnd)  &&  fgets(s, sizeof s, fp); )
      {
	 const char		*endp;

	 (void) strtok(s, "\n");
	 if (!*s  ||  (cp = s+strspn(s, WHSPCHRS), !*cp)  ||
	     *cp == CMNTCHR  ||  *cp == ASSGCHR  || !strchr(cp, ASSGCHR))
	    continue;
	 endp = cp+strlen(cp);
	 if (!(cp = strtok(cp, WHSPCHRS1))  ||
	     (cp1 = cp+strlen(cp)+1) > endp  ||
	     (cp1 += strspn(cp1, WHSPCHRS1), !*cp1))
	    continue;
	 if (!strcmp(cp, ATFDESCKW))
	    desc = lvp_parsemsg1(cp1),
	    descfnd = TRUE;
	 else if (!strcmp(cp, ATFSNOKW))
	    sno = strtol(cp1, (char **)NULL, 10),
	    snofnd = TRUE;
	 if (srch  &&  !strcmp(cp, slnm))
	    slfnd = TRUE;
      }
      if (ferror(fp))
      {
         lvp_msg(stderr, MSG_LVP_FRDERR, DEFMSG_LVP_FRDERR, progname,
		 strerror(errno), atfnm);
	 return FALSE;
      }
      (void) fclose(fp);
      MALLOC(ap, sizeof(*ap));
      ap->nextp = NULL;
      MALLOC(cp, (len = strlen((*tdpp)->d_name)-strlen(ATFSUFFIX))+1);
      (void) memcpy((void *)cp, (void *)(*tdpp++)->d_name, (size_t)len),
      *(cp+len) = 0;
      ap->atnm = cp;
      ap->desc = desc;
      ap->disp = slfnd;
      ap->sno = sno;
      ADD_ELE_TO_LIST(*app, ap, attailp);
   }

   for (i = noates, tdpp = dpp; i-- > 0; tdpp++)
      free((void *)*tdpp);
   free((void *)dpp);

   /* Rearrange the attachment types in the list so that they are ordered
      as per their sequence numbers. */
   for (ap = NULL; *app; )
   {
      register atele_t			*tap		= *app;
      register atele_t			*tap1		= NULL;
      register atele_t			*minap 		= *app;
      register atele_t			*minap1		= NULL;

      for (; tap; tap1 = tap, tap = tap->nextp)
	 if (tap->sno < minap->sno)
	    minap = tap,
	    minap1 = tap1;
      if (minap == *app)
	 *app = minap->nextp;
      else
	 minap1->nextp = minap->nextp;
      minap->nextp = NULL;
      minap->sno = SHRT_MAX;
      ADD_ELE_TO_LIST(ap, minap, attailp);
   }
   if (ap)
      *app = ap;

   /* Add to the list of attachment types and their descriptions
      obtained from the '.config' files .../etc dir.
      Then, read the relevant information from these files. */
   if ((noates = scandir(atdir, &dpp, lvp_at_select1, alphasort)) == -1)
   {
      lvp_msg(stderr, MSG_LVP_DRDERR, DEFMSG_LVP_DRDERR, progname,
	      strerror(errno), atdir);
      return FALSE;
   }
   for (i = noates, tdpp = dpp; i-- > 0; )
   {
      (void) strncpy(atfnm, atdir, sizeof(atfnm)-2),
      *(atfnm+sizeof(atfnm)-2) = 0;
      (void) strcat(atfnm, "/");
      (void) strncat(atfnm, (*tdpp)->d_name, sizeof(atfnm)-strlen(atfnm)-1),
      *(atfnm+sizeof(atfnm)-1) = 0;
      if (!(fp = fopen(atfnm, "r")))
      {
         lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
		 strerror(errno), atfnm);
	 return FALSE;
      }
      for (desc = NULL, descfnd = FALSE;
	   !descfnd  &&  fgets(s, sizeof s, fp); )
      {
	 register int		j 	= 0;

	 (void) strtok(s, "\n");
	 if (!*s  ||  (cp = s+strspn(s, WHSPCHRS), !*cp)  ||
	     *cp == CMNTCHR  ||  *cp != CFGMCHR)
	    continue;
	 for ( ; j < CFGDSCFLDNO  &&  (cp = strchr(cp, CUSFSEPCHR)); j++)
	    cp++;
	 if (!cp  ||  !(cp1 = strchr(cp, CUSFSEPCHR)))
	    continue;
	 *cp1 = MSGSEPCHR;
	 desc = lvp_parsemsg1(cp),
	 descfnd = TRUE;
      }
      if (ferror(fp))
      {
         lvp_msg(stderr, MSG_LVP_FRDERR, DEFMSG_LVP_FRDERR, progname,
		 strerror(errno), atfnm);
	 return FALSE;
      }
      (void) fclose(fp);
      MALLOC(ap, sizeof(*ap));
      ap->nextp = NULL;
      MALLOC(cp, (len = strlen((*tdpp)->d_name)-strlen(CFGSUFFIX))+1);
      (void) memcpy((void *)cp, (void *)(*tdpp++)->d_name, (size_t)len),
      *(cp+len) = 0;
      ap->atnm = cp;
      ap->desc = desc;
      ap->disp = !srch;
      ap->sno = SHRT_MAX;
      ADD_ELE_TO_LIST(*app, ap, attailp);
   }

   for (i = noates, tdpp = dpp; i-- > 0; tdpp++)
      free((void *)*tdpp);
   free((void *)dpp);

   /* Add Generic Queue (with user-defined backend) attachment to the list. */
   MALLOC(ap, sizeof(*ap));
   ap->nextp = NULL;
   MALLOC(cp, sizeof(GENATNAME));
   (void) strcpy(cp, GENATNAME);
   ap->atnm = cp;
   if (!(ccp = lvp_getmsg(MF_PIOBE, MF_PIOBESETNO, MSG_LVP_GENATDESC)))
      ccp = DEFMSG_LVP_GENATDESC;
   MALLOC(cp, strlen(ccp)+1);
   (void) strcpy(cp, ccp);
   ap->desc = cp;
   ap->disp = !srch;
   ap->sno = SHRT_MAX;
   ADD_ELE_TO_LIST(*app, ap, attailp);

   return TRUE;
#undef ADD_ELE_TO_LIST
}	/* end - lvp_bld_atlist() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_bld_qlist                                                *
*                                                                              *
* DESCRIPTION:    Build a list of queues and devices.  If a smit selector      *
*                 name is passed, exclude those whose attachment files do      *
*                 not specify this selector in their *.attach files.           *
*                 Also if a single queue name is passed, build device info     *
*                 only for this queue.                                         *
*                                                                              *
* PARAMETERS:     qpp		pointer to atlist                              *
*                 ap		atlist                                         *
*                 sqnm		single queue name                              *
*                 srch		flag to search for a smit selector             *
*                 vponly	flag to build a list of virtual printers only  *
*                                                                              *
* RETURN VALUES:  0		success                                        *
*                 !0		failure                                        *
*                                                                              *
*******************************************************************************/
static int
lvp_bld_qlist(qele_t **qpp, const atele_t *ap, const char *sqnm, int srch,
	      int vponly)
{
   register qele_t		*qp;
   register qele_t		*qtailp;
   register dele_t		*dp;
   register dele_t		*dtailp;
   register char		*cp;
   register const char		*ccp;
   struct dirent		**dirpp;
   register struct dirent	**tdpp;
   char				cusdir[PATH_MAX+1];
   char				predir[PATH_MAX+1];
   char				cusfnm[PATH_MAX+1];
   char				s[LINE_MAX];
   int				nocfs;
   register int			i;
   FILE				*fp;
   DIR				*dirp;
   struct quelist		*qlistp		= NULL;
   int				qcfgfd;
   struct flock			lck;
   register struct quelist	*tqlp;


   /* Build a list of queues and their devices by parsing the /etc/qconfig
      file. */
   if (*qpp = NULL, (qcfgfd = open(QCONFIG, O_RDONLY)) == -1)
   {
      lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
	      strerror(errno), QCONFIG);
      return FALSE;
   }
   lck.l_whence = lck.l_start = lck.l_len = 0;
   lck.l_type = F_RDLCK;
   (void) fcntl(qcfgfd, F_SETLKW, &lck);
   read_qconfig(&qlistp);
   /* Not necessary, since close() will unlock the file anyway.
   lck.l_type = F_UNLCK;
   (void) fcntl(qcfgfd, F_SETLKW, &lck);
    */
   (void) close(qcfgfd);
   if (!qlistp)
      return TRUE;
      
   /* If a single queue name was passed, build info. only for this
      queue.  Else, build info. for all the queues. */
   if (sqnm)
   {
      for (tqlp = qlistp; tqlp && strcmp(tqlp->qname, sqnm); tqlp = tqlp->next)
	 ;
      if (!tqlp)
	 return TRUE;
      MALLOC(*qpp, sizeof(**qpp));
      MALLOC(cp, strlen(sqnm)+1);
      (void) strcpy(cp, sqnm);
      (*qpp)->nextp = (*qpp)->prevp = NULL;
      (*qpp)->qnm = cp;
      (*qpp)->dp = NULL;
      (*qpp)->nodvs = 0;
      qtailp = *qpp;
      for (tqlp = tqlp->next; tqlp && !strcmp(tqlp->qname, (*qpp)->qnm);
	   tqlp = tqlp->next)
      {
	 MALLOC(dp, sizeof(*dp));
	 MALLOC(cp, strlen(tqlp->dname)+1);
	 (void) strcpy(cp, tqlp->dname);
	 dp->nextp = NULL;
	 dp->dvnm = cp;
	 dp->desc = NULL;
	 dp->disp = vponly ? FALSE : !srch;
	 if (!(*qpp)->dp)
	    dtailp = (*qpp)->dp = dp;
	 else
	    dtailp->nextp = dp,
	    dtailp = dp;
	 (*qpp)->nodvs++;
      }
      if (!(*qpp)->nodvs)
      {
	 free((void *)(*qpp)->qnm);
	 free((void *)*qpp);
	 *qpp = qtailp = NULL;
      }
   }
   else for (tqlp = qlistp; tqlp; )
   {
      ccp = tqlp->qname;
      MALLOC(qp, sizeof(*qp));
      MALLOC(cp, strlen(ccp)+1);
      (void) strcpy(cp, ccp);
      qp->nextp = NULL;
      qp->qnm = cp;
      qp->dp = NULL;
      qp->nodvs = 0;
      if (!*qpp)
         qp->prevp = NULL,
         *qpp = qtailp = qp;
      else
         qp->prevp = qtailp,
         qtailp->nextp = qp,
         qtailp = qp;
      for (tqlp = tqlp->next; tqlp && !strcmp(tqlp->qname, ccp);
	   tqlp = tqlp->next)
      {
	 MALLOC(dp, sizeof(*dp));
	 MALLOC(cp, strlen(tqlp->dname)+1);
	 (void) strcpy(cp, tqlp->dname);
	 dp->nextp = NULL;
	 dp->dvnm = cp;
	 dp->desc = NULL;
	 dp->disp = vponly ? FALSE : !srch;
	 if (!qp->dp)
	    dtailp = qp->dp = dp;
	 else
	    dtailp->nextp = dp,
	    dtailp = dp;
	 qp->nodvs++;
      }
      if (!qp->nodvs)
      {
	 if (qp->nextp)
	    qp->nextp->prevp = qp->prevp;
	 if (qp->prevp)
	    qp->prevp->nextp = qp->nextp;
	 if (qp == *qpp)
	    *qpp = qp->nextp;
	 qtailp = qp->prevp;
	 free((void *)qp->qnm);
	 free((void *)qp);
      }
   }
   if (!*qpp)
      return TRUE;

   /* If a default printer is specified in the process's environment
      variables LPDEST or PRINTER, make sure that it is listed first
      in the queue list. */
   if (cp = (cp = getenv(STRGZARG(LPDEST))) ? cp : getenv(STRGZARG(PRINTER)))
   {
      size_t			qnmlen;

      qnmlen = (ccp = strchr(cp,CUSFSEPCHR))?(size_t)(ccp++-cp):strlen(cp);
      for (qp = *qpp; qp  &&  strncmp(qp->qnm, cp, qnmlen); qp = qp->nextp)
	 ;
      if (qp  &&  qp != *qpp)
      {
	 if (qp->nextp)
	    qp->nextp->prevp = qp->prevp;
	 if (qp->prevp)
	    qp->prevp->nextp = qp->nextp;
	 qp->nextp = *qpp, (*qpp)->prevp = qp;
	 *qpp = qp;
      }
      if (qp && ccp)
      {
	 register dele_t		*dp1	= NULL;

	 for (dp = (*qpp)->dp; dp && strcmp(dp->dvnm,ccp);
	      dp1 = dp, dp = dp->nextp)
	    ;
	 if (dp && dp1)
	 {
	    dp1->nextp = dp->nextp;
	    dp->nextp = (*qpp)->dp;
	    (*qpp)->dp = dp;
	 }
      }
   }

   /* Select the print queues out of these (those with virtual printer
      definitions).  For each print queue selected, determine the attachment
      type from its custom colon file.  Also, retrieve relevant information
      for each print queue from its colon file.
    */
   (void) strncpy(cusdir, vdir, sizeof(cusdir)-strlen(CUSTOMDIR)-1),
   *(cusdir+sizeof(cusdir)-strlen(CUSTOMDIR)-1) = 0;
   (void) strcat(cusdir, CUSTOMDIR);
   if ((nocfs = scandir(cusdir, &dirpp, lvp_q_select, alphasort)) == -1)
   {
      lvp_msg(stderr, MSG_LVP_DRDERR, DEFMSG_LVP_DRDERR, progname,
	      strerror(errno), cusdir);
      return FALSE;
   }
   (void) strncpy(predir, bdir, sizeof(predir)-strlen(PREDEFDIR)-1),
   *(predir+sizeof(predir)-strlen(PREDEFDIR)-1) = 0;
   (void) strcat(predir, PREDEFDIR);
   if (!(dirp = opendir(predir)))
   {
      lvp_msg(stderr, MSG_LVP_DOPENERR, DEFMSG_LVP_DOPENERR, progname,
	      strerror(errno), predir);
      return FALSE;
   }
   for (qp = *qpp; qp; qp = qp->nextp)
   {
      for (i = nocfs, tdpp = dirpp; i; i--, tdpp++)
      {
         size_t			qlen;
         struct dirent		*drtp;
	 unsigned int		nopfls;
         struct attrs {  	
				/* !!!COMPILER ERROR!!!  in assigning
				   values to fnd and aval fields later???
				const char	anm[ATTRNMLEN];
				 */
				char		anm[ATTRNMLEN];
				uchar_t		fnd;
				char		*aval;
		      }		attrtbl[] = {
/*	 Commented out so as not to use "mn" attribute for device, but
	 to use qdev instead.
					{ ATAT, (uchar_t)FALSE, NULL },
#        define	ATINDEX	0
					{ ATmn, (uchar_t)FALSE, NULL },
#        define	mnINDEX	1
					{ ATmL, (uchar_t)FALSE, NULL },
#        define	mLINDEX	2
					{ ATmt, (uchar_t)FALSE, NULL },
#        define	mtINDEX	3
					{ ATmA, (uchar_t)FALSE, NULL },
#        define	mAINDEX	4
 */
					{ ATAT, (uchar_t)FALSE, NULL },
#        define	ATINDEX	0
					{ ATmL, (uchar_t)FALSE, NULL },
#        define	mLINDEX	1
					{ ATmt, (uchar_t)FALSE, NULL },
#        define	mtINDEX	2
					{ ATmA, (uchar_t)FALSE, NULL },
#        define	mAINDEX	3
					{ "", (uchar_t)FALSE, NULL }
					  };
         struct attrs		*attrp;

	 if (!(ccp = strchr((*tdpp)->d_name, CUSFSEPCHR)))
	    continue;
	 qlen = MIN((size_t)(ccp-(*tdpp)->d_name), sizeof(s)-1);
	 (void) memcpy((void *)s, (void *)(*tdpp)->d_name, qlen),
	 *(s+qlen) = 0;
	 if (strcmp(qp->qnm, s))
	    continue;
	 for (++ccp, dp = qp->dp; dp  &&  strcmp(ccp, dp->dvnm); dp = dp->nextp)
	    ;
	 if (!dp)
	    continue;

         (void) strncpy(cusfnm, cusdir, sizeof(cusfnm)-2),
         *(cusfnm+sizeof(cusfnm)-2) = 0;
	 (void) strcat(cusfnm, "/");
         (void) strncat(cusfnm, (*tdpp)->d_name,
			sizeof(cusfnm)-strlen(cusfnm)-1),
         *(cusfnm+sizeof(cusfnm)-1) = 0;
         if (!(fp = fopen(cusfnm, "r")))
         {
            lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
		    strerror(errno), cusfnm);
	    return FALSE;
         }
         while (fgets(s, sizeof s, fp))
	 {
	    uchar_t		allfnd;

	    (void) strtok(s, "\n");
	    if (!*s  ||  (ccp = s+strspn(s, WHSPCHRS), !*ccp)  ||
		*ccp == CMNTCHR  || !(ccp = strchr(ccp, CUSFSEPCHR))  ||
		!(ccp = strchr(ccp+1, CUSFSEPCHR)))
	       continue;
	    for (attrp = attrtbl, ++ccp; *attrp->anm; attrp++)
	       if (!memcmp((void *)attrp->anm, (void *)ccp,
		   (size_t)ATTRNMLEN))
		  break;
	    if (!*attrp->anm)
	       continue;
	    if (!(ccp = strchr(ccp, CUSFSEPCHR))  ||
		!(ccp = strchr(ccp+1, CUSFSEPCHR)))
	       continue;
	    if (!strcmp(attrp->anm, attrtbl[ATINDEX].anm))
	    {
	       if (cp = strchr(++ccp, SUBATSEP))
	       {
	          MALLOC(attrp->aval, cp-ccp+1);
	          (void) memcpy((void *)attrp->aval, (void *)ccp,
				(size_t)(cp-ccp)),
		  *(attrp->aval+(cp-ccp)) = 0;
	       }
	       else
	       {
	          MALLOC(attrp->aval, strlen(ccp)+1);
	          (void) strcpy(attrp->aval, ccp);
	       }
	    }
	    else
	    {
	       MALLOC(attrp->aval, strlen(++ccp)+1);
	       (void) strcpy(attrp->aval, ccp);
	    }
	    attrp->fnd = TRUE;
	    for (allfnd = TRUE, attrp = attrtbl; *attrp->anm; attrp++)
	       allfnd &= attrp->fnd;
	    if (allfnd)
	       break;
	 }
	 if (ferror(fp))
	 {
            lvp_msg(stderr, MSG_LVP_FRDERR, DEFMSG_LVP_FRDERR, progname,
		    strerror(errno), cusfnm);
	    return FALSE;
	 }
	 (void) fclose(fp);
	 /* Commented out -
	 if (!attrtbl[mnINDEX].fnd)
	  */
	 if (!attrtbl[mtINDEX].fnd)
	 {
            lvp_msg(stderr, MSG_LVP_PTYPENOTFND, DEFMSG_LVP_PTYPENOTFND,
		    progname, cusfnm);
	    continue;
	 }
	 /* Commented out -
	 dp->dvnm = attrtbl[mnINDEX].aval;
	  */
	 for (rewinddir(dirp), nopfls = 0U,
	      (void) strncpy(s, attrtbl[mtINDEX].aval, sizeof(s)-2),
	      *(s+sizeof(s)-2) = 0, (void) strcat(s, ".");
	      nopfls < 2  &&  (drtp = readdir(dirp)); )
	    if (!strncmp(drtp->d_name, s, strlen(s)))
	       nopfls++;
	 if (nopfls < 2)
	 {
	    if (attrtbl[mLINDEX].fnd)
	    {
	       dp->desc = (ccp = lvp_parsemsg(attrtbl[mLINDEX].aval))?ccp:
		          attrtbl[mLINDEX].aval;
	       free((void *)attrtbl[mtINDEX].aval);
	    }
	    else
	       dp->desc = attrtbl[mtINDEX].aval;
	    if (attrtbl[mAINDEX].fnd)
	       free((void *)attrtbl[mAINDEX].aval);
	 }
	 else
	 {
	    if (attrtbl[mAINDEX].fnd)
	    {
	       MALLOC(cp, strlen(attrtbl[mtINDEX].aval)+3+
		      strlen(attrtbl[mAINDEX].aval)+1);
				/* 3 - for " (" prefix and ")" suffix */
	       (void) sprintf(cp, "%s (%s)", attrtbl[mtINDEX].aval,
			      attrtbl[mAINDEX].aval);
	       dp->desc = cp;
	       free((void *)attrtbl[mtINDEX].aval);
	       free((void *)attrtbl[mAINDEX].aval);
	    }
	    else
	       dp->desc = attrtbl[mtINDEX].aval;
	    if (attrtbl[mLINDEX].fnd)
	       free((void *)attrtbl[mLINDEX].aval);
	 }
	 if (!srch  ||  !attrtbl[ATINDEX].fnd)
	    dp->disp = TRUE;
	 else
	 {
   	    register atele_t const	*tap;

	    for (tap = ap; tap; tap = tap->nextp)
	       if (!strcmp(tap->atnm, attrtbl[ATINDEX].aval))
	          break;
	    dp->disp = tap ? tap->disp : TRUE;
	 }
	 if (attrtbl[ATINDEX].fnd)
	    free((void *)attrtbl[ATINDEX].aval);
#        undef ATINDEX
#        undef mnINDEX
#        undef mLINDEX
#        undef mtINDEX
#        undef mAINDEX
      }
   }
   (void) closedir(dirp);

   for (i = nocfs, tdpp = dirpp; i; i--, tdpp++)
      free((void *)tdpp);
   free((void *)dirpp);
   return TRUE;
}	/* end - lvp_bld_qlist() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_get_sqinfo                                               *
*                                                                              *
* DESCRIPTION:    Get info for a given single queue.                           *
*                                                                              *
* PARAMETERS:     sqnm		single queue name                              *
*                 slnm		SMIT selector name                             *
*                                                                              *
* RETURN VALUES:  0		success                                        *
*                 !0		failure                                        *
*                                                                              *
*******************************************************************************/
static sqele_t *
lvp_get_sqinfo(const char *sqnm, const char *slnm)
{
   static sqele_t		sq;
   char				s[LINE_MAX];
   char				dr[PATH_MAX+1];
   char				fl[PATH_MAX+1];
   FILE				*fp;
   register char		*cp;
   register char		*cp1;
   register int			i;

   /* Determine queue name and queue device name for the given queue.
      If queue was specified in [q:qdev] format, use the sub-fields.
      Else, get a list of queue devices by executing 'lsallqdev' command,
      and select the first queue device with a virtual printer definition.
    */
   sq.dvnm = sq.atnm = sq.slval = NULL;
   (void) strncpy(dr, vdir, sizeof(dr)-strlen(CUSTOMDIR)-1),
   *(dr+sizeof(dr)-strlen(CUSTOMDIR)-1) = 0;
   (void) strcat(dr, CUSTOMDIR);
   if (cp = strchr(sqnm, CUSFSEPCHR))
   {
      MALLOC(cp1, cp-sqnm+1);
      (void) memcpy((void *)cp1, (void *)sqnm, (size_t)(cp-sqnm)),
      *(cp1+(cp++-sqnm)) = 0;
      sq.qnm = cp1;
      MALLOC(cp1, strlen(cp)+1);
      (void) strcpy(cp1, cp);
      sq.dvnm = cp1;
   }
   else
   {
      FILE				*pp;
      DIR				*dp;
      struct dirent			*dep;
      char				cmd[PATH_MAX+1] = LSALLQDEV;
      size_t				prefixlen;

      if (!(dp = opendir(dr)))
      {
         lvp_msg(stderr, MSG_LVP_DOPENERR, DEFMSG_LVP_DOPENERR, progname,
	         strerror(errno), dr);
         return (sqele_t *)NULL;
      }
      (void) strncat(cmd, " -q", sizeof(cmd)-strlen(cmd)-1),
      *(cmd+sizeof(cmd)-1) = 0;
      (void) strncat(cmd, sqnm, sizeof(cmd)-strlen(cmd)-1);
      (void) strncpy(fl, sqnm, sizeof(fl)-2),
      *(fl+sizeof(fl)-2) = 0;
      (void) strcat(fl, ":");
      if (cp = fl+(prefixlen = strlen(fl)), !(pp = popen(cmd, "r")))
      {
         lvp_msg(stderr, MSG_LVP_POPENERR, DEFMSG_LVP_POPENERR, progname,
	         strerror(errno), cmd);
         return (sqele_t *)NULL;
      }
      /***
       Only consider the first qdevice, while checking for virtual printer
       definitions for the specified queue.  This is a departure from the
       initially-coded logic which made the assumption of checking for
       all the qdevices for the queue.
       ***/
      while (fgets(s, sizeof s, pp))
      {
         (void) strtok(s, "\n");
	 (void) strncpy(cp, s, sizeof(fl)-prefixlen-1),
	 *(fl+sizeof(fl)-1) = 0;
	 for (rewinddir(dp); (dep = readdir(dp))  &&
	      strcmp(dep->d_name, fl); )
	    ;
	 /***	Always break after looking at the first device.
	 if (dep)
	  ***/
	    break;
      }
      if (ferror(pp))
      {
         lvp_msg(stderr, MSG_LVP_PRDERR, DEFMSG_LVP_PRDERR, progname,
	         strerror(errno), cmd);
         return (sqele_t *)NULL;
      }
      if (!dep)
      {
	 lvp_msg(stderr, MSG_LVP_NOVPSFND, DEFMSG_LVP_NOVPSFND, progname, sqnm);
         return (sqele_t *)NULL;
      }
      MALLOC(cp, strlen(sqnm)+1);
      (void) strcpy(cp, sqnm);
      sq.qnm = cp;
      MALLOC(cp, strlen(s)+1);
      (void) strcpy(cp, s);
      sq.dvnm = cp;

      (void) closedir(dp);
      (void) pclose(pp);
   }

   /* Fetch the attachment type of the queue from its custom colon file. */
   (void) strncpy(fl, dr, sizeof(fl)-2),
   *(fl+sizeof(fl)-2) = 0;
   (void) strcat(fl, "/");
   (void) strncat(fl, sq.qnm, sizeof(fl)-strlen(fl)-1),
   *(fl+sizeof(fl)-1) = 0;
   (void) strncat(fl, ":", sizeof(fl)-strlen(fl)-1);
   (void) strncat(fl, sq.dvnm, sizeof(fl)-strlen(fl)-1);
   if (i = FALSE, !(fp = fopen(fl, "r")))
   {
      lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
	      strerror(errno), fl);
      return (sqele_t *)NULL;
   }
   while (fgets(s, sizeof s, fp))
   {
      (void) strtok(s, "\n");
      if (!*s  ||  (cp = s+strspn(s, WHSPCHRS), !*cp)  ||
	  *cp == CMNTCHR  || !(cp = strchr(cp, CUSFSEPCHR))  ||
	  !(cp = strchr(cp+1, CUSFSEPCHR))  ||
          memcmp((void *)ATAT, (void *)++cp, (size_t)ATTRNMLEN)  ||
          !(cp = strchr(cp, CUSFSEPCHR))  ||  !(cp = strchr(cp+1, CUSFSEPCHR)))
         continue;
      MALLOC(cp1, strlen(++cp)+1);
      (void) strcpy(cp1, cp);
      sq.atnm = cp1, i = TRUE;
      break;
   }
   if (ferror(fp))
   {
      lvp_msg(stderr, MSG_LVP_FRDERR, DEFMSG_LVP_FRDERR, progname,
	      strerror(errno), fl);
      return (sqele_t *)NULL;
   }
   (void) fclose(fp);
   if (!i)
      return &sq;

   /* Get the SMIT selector value for the specified selector field from
      the attachment file for the given queue. */
   (void) strncpy(fl, bdir, sizeof(fl)-strlen(ATFDIR)-1),
   *(fl+sizeof(fl)-strlen(ATFDIR)-1) = 0;
   (void) strcat(fl, ATFDIR);
   (void) strncat(fl, "/", sizeof(fl)-strlen(fl)-1),
   *(fl+sizeof(fl)-1) = 0;
   if (cp = strchr(sq.atnm, SUBATSEP))
   {
      size_t			l = MIN(sizeof(fl)-strlen(fl)-1, cp-sq.atnm);

      (void) memcpy((void *)(cp = fl+strlen(fl)), (void *)sq.atnm, l),
      *(cp+l) = 0;
   }
   else
      (void) strncat(fl, sq.atnm, sizeof(fl)-strlen(fl)-1);
   (void) strncat(fl, ATFSUFFIX, sizeof(fl)-strlen(fl)-1);
   if (i = FALSE, !(fp = fopen(fl, "r")))
   {
      lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
	      strerror(errno), fl);
      return (sqele_t *)NULL;
   }
   while (!i  &&  fgets(s, sizeof s, fp))
   {
      const char		*endp;

      (void) strtok(s, "\n");
      if (!*s  ||  (cp = s+strspn(s, WHSPCHRS), !*cp)  ||
	  *cp == CMNTCHR  ||  *cp == ASSGCHR  || !strchr(cp, ASSGCHR)  ||
          (endp = cp+strlen(cp), !(cp = strtok(cp, WHSPCHRS1)))  ||
	  (cp1 = cp+strlen(cp)+1) > endp  ||
	  (cp1 += strspn(cp1, WHSPCHRS1), !*cp1))
	 continue;
      if (!strcmp(cp, slnm))
      {
	 MALLOC(cp, strlen(cp1)+1);
	 (void) strcpy(cp, cp1);
	 sq.slval = cp, i = TRUE;
      }
   }
   if (ferror(fp))
   {
      lvp_msg(stderr, MSG_LVP_FRDERR, DEFMSG_LVP_FRDERR, progname,
	      strerror(errno), fl);
      return (sqele_t *)NULL;
   }
   (void) fclose(fp);

   return &sq;
}	/* end - lvp_get_sqinfo() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_get_satinfo                                              *
*                                                                              *
* DESCRIPTION:    Get info for a given single attachment type.                 *
*                                                                              *
* PARAMETERS:     satnm		single attachment type                         *
*                 slnm		SMIT selector name                             *
*                                                                              *
* RETURN VALUES:  0		success                                        *
*                 !0		failure                                        *
*                                                                              *
*******************************************************************************/
static satele_t *
lvp_get_satinfo(const char *satnm, const char *slnm)
{
   static satele_t		sat;
   char				s[LINE_MAX];
   char				fl[PATH_MAX+1];
   FILE				*fp;
   register char		*cp;
   register char		*cp1;
   register int			slfnd 		= FALSE;

   /* Fetch the SMIT selector value from the attachment (.attach) file for
      the specified attachment type for the given field. */
   *(fl+sizeof(fl)-1) = 0;
   (void) strncpy(fl, bdir, sizeof(fl)-1);
   (void) strncat(fl, ATFDIR "/", sizeof(fl)-strlen(fl));
   (void) strncat(fl, satnm, sizeof(fl)-strlen(fl));
   (void) strncat(fl, ATFSUFFIX, sizeof(fl)-strlen(fl));
   if (!(fp = fopen(fl, "r")))
   {
      lvp_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
              strerror(errno), fl);
      return (satele_t *)NULL;
   }
   for ( ; !slfnd  &&  fgets(s, sizeof s, fp); )
   {
      const char		*endp;

      (void) strtok(s, "\n");
      if (!*s  ||  (cp = s+strspn(s, WHSPCHRS), !*cp)  ||
          *cp == CMNTCHR  ||  *cp == ASSGCHR  || !strchr(cp, ASSGCHR))
         continue;
      endp = cp+strlen(cp);
      if (!(cp = strtok(cp, WHSPCHRS1))  ||
          (cp1 = cp+strlen(cp)+1) > endp  ||
          (cp1 += strspn(cp1, WHSPCHRS1), !*cp1))
         continue;
      if (!strcmp(cp, slnm))
         slfnd = TRUE;
   }
   if (ferror(fp))
   {
      lvp_msg(stderr, MSG_LVP_FRDERR, DEFMSG_LVP_FRDERR, progname,
              strerror(errno), fl);
      return (satele_t *)NULL;
   }
   (void) fclose(fp);
   if (slfnd)
   {
      MALLOC(cp, strlen(cp1)+1);
      (void) strcpy(cp, cp1);
      sat.slval = cp;
   }

   return &sat;
}	/* end - lvp_get_satinfo() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_at_select                                                *
*                                                                              *
* DESCRIPTION:    Select attachment type (.attach) files.                      *
*                                                                              *
* PARAMETERS:     dp		directory entry                                *
*                                                                              *
* RETURN VALUES:  0 		to be selected                                 *
*                 !0		not to be selected                             *
*                                                                              *
*******************************************************************************/
static int
lvp_at_select(struct dirent *dp)
{
   size_t		len;
   size_t		len1;

   return ((len = strlen(dp->d_name)) > (len1 = strlen(ATFSUFFIX))) ?
	   !strcmp(dp->d_name+len-len1, ATFSUFFIX) : FALSE;
}	/* end - lvp_at_select() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_at_select1                                               *
*                                                                              *
* DESCRIPTION:    Select attachment type (.config) files.                      *
*                                                                              *
* PARAMETERS:     dp		directory entry                                *
*                                                                              *
* RETURN VALUES:  0 		to be selected                                 *
*                 !0		not to be selected                             *
*                                                                              *
*******************************************************************************/
static int
lvp_at_select1(struct dirent *dp)
{
   size_t		len;
   size_t		len1;

   return ((len = strlen(dp->d_name)) > (len1 = strlen(CFGSUFFIX))) ?
	   !strcmp(dp->d_name+len-len1, CFGSUFFIX) : FALSE;
}	/* end - lvp_at_select1() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_q_select                                                 *
*                                                                              *
* DESCRIPTION:    Select queues with virtual printers.                         *
*                                                                              *
* PARAMETERS:     dp		directory entry                                *
*                                                                              *
* RETURN VALUES:  0 		to be selected                                 *
*                 !0		not to be selected                             *
*                                                                              *
*******************************************************************************/
static int
lvp_q_select(struct dirent *dp)
{
   return strchr(dp->d_name, CUSFSEPCHR) ? TRUE : FALSE;
}	/* end - lvp_q_select() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_parsemsg                                                 *
*                                                                              *
* DESCRIPTION:    Parse a given message and fetch it, if necessary.            *
*                                                                              *
* PARAMETERS:     mp		msg                                            *
*                                                                              *
* RETURN VALUES:  fmp 		fetched msg                                    *
*                                                                              *
*******************************************************************************/
static const char *
lvp_parsemsg(const char *mp)
{
   register const char		*cp;
   register const char		*sp;
   const char			*tp;
   char				*fmp		= NULL;
   char				tmp[LINE_MAX+1];
   char 			mcnm[PATH_MAX+1];
   int				setno		= MF_PIOATTR1SETNO;
   int				msgno;
   int				parseerr	= FALSE;
   size_t			maxlen;

   if (!mp || !*mp)
      return (char *)NULL;

   if (*mp != SETBEGINCHR || *(cp = mp+strlen(mp)-1) != SETENDCHR)
   {
      MALLOC(fmp,strlen(mp)+1);
      (void)strcpy(fmp,mp);
      return fmp;
   }

   (void)memcpy(tmp,mp+1,maxlen = MIN(cp-(mp+1),sizeof(tmp)-1)),
   *(tmp+maxlen) = 0;
   if (!(cp = strchr(tmp,MSGSEPCHR)))
   {
      MALLOC(fmp,strlen(tmp)+1);
      (void)strcpy(fmp,tmp);
      return fmp;
   }

   if (cp == tmp)
   {
      cp++;
      MALLOC(fmp,strlen(cp)+1);
      (void)strcpy(fmp,cp);
      return fmp;
   }

   *mcnm = 0;
   do
   {
      /* Get msg no. */
      for (sp = cp-1; *sp != FLDSEPCHR; )
	 if (sp == tmp)
	    break;
	 else
	    sp--;
      if (msgno = (int)strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,
			      (char **)&tp,10), *tp != MSGSEPCHR)
      {
	 parseerr++;
	 break;
      }
      if (*sp != FLDSEPCHR  ||  sp == tmp)
	 break;

      /* Get set no. */
      for (--sp; *sp != FLDSEPCHR; )
	 if (sp == tmp)
	    break;
	 else
	    sp--;
      if (setno = (int)strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,
			      (char **)&tp,10), *tp != FLDSEPCHR)
      {
	 parseerr++;
	 break;
      }
      if (*sp != FLDSEPCHR || sp == tmp)
	 break;

      /* Get message catalog name. */
      (void)memcpy((void *)mcnm,(void *)tmp,
		    /***  COMPILER ERROR! ***/
		    /*
		    maxlen = MIN(sp-tmp,sizeof(mcnm)-1)),
		     */
		    maxlen = MIN((char *)sp-tmp,sizeof(mcnm)-1)),
      *(mcnm+maxlen) = 0;
   } while (0);

   if (parseerr || !*mcnm ||
       !(sp = lvp_getmsg(mcnm,setno?setno:MF_PIOATTR1SETNO,msgno)))
   {
      cp++;
      MALLOC(fmp,strlen(cp)+1);
      (void)strcpy(fmp,cp);
   }
   else
   {
      MALLOC(fmp,strlen(sp)+1);
      (void)strcpy(fmp,sp);
   }

   return fmp;
}	/* end - lvp_parsemsg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lvp_parsemsg1                                                *
*                                                                              *
* DESCRIPTION:    Parse a given message and fetch it, if necessary.            *
*                                                                              *
* PARAMETERS:     mp		msg                                            *
*                                                                              *
* RETURN VALUES:  fmp 		fetched msg                                    *
*                                                                              *
*******************************************************************************/
static char *
lvp_parsemsg1(const char *mp)
{
   register const char		*cp;
   register const char		*sp;
   const char			*tp;
   char				*fmp = NULL;
   char 			mcnm[PATH_MAX+1] = MF_PIOBE;
   int				setno = MF_PIOBESETNO;
   int				msgno;
   int				parseerr = FALSE;
   size_t			maxlen;

   if (!mp)
      return (char *)NULL;

   if (!(cp = strchr(mp, MSGSEPCHR)))
   {
      MALLOC(fmp, strlen(mp)+1);
      (void) strcpy(fmp, mp);
      return fmp;
   }

   if (cp == mp)
   {
      cp++;
      MALLOC(fmp, strlen(cp)+1);
      (void) strcpy(fmp, cp);
      return fmp;
   }

   do
   {
      /* Get msg no. */
      for (sp = cp-1; *sp != FLDSEPCHR; )
	 if (sp == mp)
	    break;
	 else
	    sp--;
      if (msgno = (int)strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,
			      (char **)&tp, 10), *tp != MSGSEPCHR)
      {
	 parseerr++;
	 break;
      }
      if (*sp != FLDSEPCHR  ||  sp == mp)
	 break;

      /* Get set no. */
      for (--sp; *sp != FLDSEPCHR; )
	 if (sp == mp)
	    break;
	 else
	    sp--;
      if (setno = (int)strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,
			      (char **)&tp, 10), *tp != FLDSEPCHR)
      {
	 parseerr++;
	 break;
      }
      if (*sp != FLDSEPCHR  ||  sp == mp)
	 break;

      /* Get message catalog name. */
      (void) memcpy((void *)mcnm, (void *)mp,
		    /***  COMPILER ERROR! ***/
		    /*
		    maxlen = MIN(sp-mp, sizeof(mcnm)-1)),
		     */
		    maxlen = MIN((char *)sp-mp, sizeof(mcnm)-1)),
      *(mcnm+maxlen) = 0;
   } while (0);

   if (parseerr  ||  !(sp = lvp_getmsg(mcnm, setno, msgno)))
   {
      cp++;
      MALLOC(fmp, strlen(cp)+1);
      (void) strcpy(fmp, cp);
   }
   else
   {
      MALLOC(fmp, strlen(sp)+1);
      (void) strcpy(fmp, sp);
   }

   return fmp;
}	/* end - lvp_parsemsg1() */


/**************************************************************************
 	The following code defines functions and variables that are
	used in in read_qconfig() function.  This is a kludge to get
	'qccom.c' file to compile and link properly.
***************************************************************************/
#undef va_start
#undef va_end
#undef va_arg
#include <varargs.h>	/* for functions that are used in read_qconfig() */

int			sysmsg(char *);
int			syserr();
int			syswarn();
int			systell();
int			sysraw();
void			*Qalloc(size_t);

/*==== Send a message to the correct place */
sysmsg(char *message /* the message to print */)
{
	/*----Combine the error message and the perror messages */
	if (errno)
        	fprintf(stderr, "%s%s: errno = %d: %s\n",message,progname,
			errno,strerror(errno));
	else
		fprintf(stderr, "%s\n", message);

	/*----Reset errno and return */
	errno = 0;
	return(0);
}


/*==== Fatal error message, and die                                          */
/*     Call:  syserr(<exitcode>, <message_format>, <thing1>, <thing2>, ...); */
syserr(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
	char	message[256];
        char    *fmt;
	int	exitcode;

        va_start(args);
	exitcode = va_arg(args,int);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n",progname,buf);
        va_end(args);
	sysmsg(message);
	exit(exitcode);
}


/*==== Warning error message                                      */
/*     Call:  syswarn(<message_format>, <thing1>, <thing2>, ...); */
syswarn(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
	char	message[256];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n", progname, buf);
        va_end(args);
	sysmsg(message);
	return(0);
}



/*==== Information message                                        */
/*     Call:  systell(<message_format>, <thing1>, <thing2>, ...); */
systell(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
	char	message[256];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n", progname, buf);
        va_end(args);
	sysmsg(message);
	return(0);
}



/*==== Raw, unformatted information message (for debugging statements) */
/*     Call:  sysraw(<message_format>, <thing1>, <thing2>, ...);       */
/*     NOTE: no line feed added                                        */
sysraw(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
        va_end(args);
	errno = 0;
	sysmsg(buf);
	return(0);
}



void *Qalloc(size_t size)
{       
	register void *ans;

	MALLOC(ans,size);
	bzero(ans,size);
	return(ans);
}


