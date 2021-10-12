static char sccsid[] = "@(#)79  1.2  src/bos/usr/lib/pios/piomgpdev.c, cmdpios, bos411, 9428A410j 11/9/93 11:04:47";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  main(), mpd_msg(), mpd_add_fldele(), mpd_read_pdev(),
 *	       mpd_write_pdev(), mpd_getmsg(), mpd_parsemsg()
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/chownx.h>
#include <sys/mode.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <grp.h>
#include <locale.h>
#include <nl_types.h>
#include <errno.h>
#include <piobe_msg.h>

/* External declarations */
extern int		fchownx(int,uid_t,gid_t,int);
					/* should've been in <sys/chownx.h> */

/* Misc. macros */
#define PRINTQ_GID		(9)
#define PRINTQ_GNM		"printq"
#define WKBUFLEN		(1023)
#define RDBUFSIZE		(WKBUFLEN+1)
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc/"
#define MF_PIOBE		"piobe.cat"
#define MF_PIOBESETNO		(8)
#define MF_PIOATTR1SETNO	(1)
#define DEFVARDIR		"/var/spool/lpd/pio/@local"
#define ENVBASEDIR		"PIOBASEDIR"
#define ENVVARDIR		"PIOVARDIR"
#define DEVDIR			"/dev/"
#define WHSPCHRS		" \t"
#define WHSPCHRS1		" \t="
#define SETSTARTCHR		'#'
#define SETSEPCHR		':'
#define PDATSEPCHRSTR		"#"
#define SMITCCMT		"#!:"
#define ASSGCHR			'='
#define CMNTCHR			'#'
#define PDFLRECFMT		"%s\t=\t%s\n"
#define SETBEGINCHR		'['
#define SETENDCHR		']'
#define MSGSEPCHR		';'
#define FLDSEPCHR		','
#if defined(MIN)
#undef MIN
#endif		/* MIN */
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#undef  MALLOC
#define MALLOC(p,sz)		do \
				{ \
				   if (!((p) = malloc((size_t)(sz)))) \
				   { \
         			      mpd_msg(stderr,MSG_MPD_MALLOCERR, \
					     DEFMSG_MPD_MALLOCERR,pgnm, \
					     strerror(errno)); \
				      exit(EXIT_FAILURE); \
				   } \
				} while (0)
      


/* Macros for default messages */
#define DEFMSG_MPD_ERRMSG	"Cannot access the message catalog " \
				MF_PIOBE ".\n"
#define DEFMSG_MPD_USAGE	"Usage:\t%s -p PseudoDevice -t AttachmentType" \
				"\n\t\t{ -A | -C | -R | -D } [ -a Clause... ]\n"
#define MSG_MPD_MISSARG		MSG_LVP_MISSARG
#define DEFMSG_MPD_MISSARG	"%s: Missing argument for the flag -%c\n"
#define MSG_MPD_ILLOPT		MSG_LVP_ILLOPT
#define DEFMSG_MPD_ILLOPT	"%s: Illegal flag -%c\n"
#define MSG_MPD_ILLUSE		MSG_LVP_ILLUSE
#define DEFMSG_MPD_ILLUSE	"%s: Illegal usage\n"
#define MSG_MPD_MALLOCERR	MSG_LVP_MALLOCERR
#define DEFMSG_MPD_MALLOCERR	"%s: Error '%s' in malloc()\n"
#define MSG_MPD_FOPENERR	MSG_LVP_FOPENERR
#define DEFMSG_MPD_FOPENERR	"%s: Error '%s' in opening the file %s\n"
#define DEFMSG_MPD_UNLINKERR	"%s: Error '%s' in removing the file %s\n"
#define DEFMSG_MPD_FLDNOTFND	"%s: The specified field '%s' is not found" \
				" in the pseudo-device file %s\n"
#define MSG_MPD_FRDERR		MSG_LVP_FRDERR
#define DEFMSG_MPD_FRDERR	"%s: Error '%s' in reading the file %s\n"
#define DEFMSG_MPD_FWRERR	"%s: Error '%s' in writing to the file %s\n"
#define DEFMSG_MPD_FCHERR	"%s: Error '%s' in changing access rights of" \
				" the file %s\n"
#define DEFMSG_MPD_PDEXISTS	"%1$s: The device '%2$s' already exists; can " \
				"not create\n"

/* Local variable definitions */
static const char	*pgnm;

/* Structures for field clauses */
typedef struct fldinfo {
	struct fldinfo		*nextp;
	caddr_t			fnm;
	caddr_t			fval;
	uchar_t			fnd;
} fldinfo_t;

/* Local function declarations */
static void 		mpd_msg(FILE *,int,const char *,...);
static void		mpd_add_fldele(fldinfo_t **,fldinfo_t **,char *,int);
static int		mpd_read_pdev(const char *,fldinfo_t **);
static int		mpd_write_pdev(const char *,fldinfo_t *);
static const char	*mpd_getmsg(const char *,int,int);
static const char	*mpd_parsemsg(const char *);



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
   int				pflag 		= 0;	/* flag for pdev */
   int				tflag 		= 0;	/* flag for attype */
   int				Aflag 		= 0;	/* flag for add */
   int				Cflag 		= 0;	/* flag for change */
   int				Rflag 		= 0;	/* flag for remove */
   int				Dflag 		= 0;	/* flag for display */
   int				aflag 		= 0;	/* flag for a clause */
   const char			*pdnm; 			/* pseudo device */
   const char			*atnm; 			/* attachment type */
   const char			*ccp;
   const char			*ccp1;
   caddr_t			cp;
   char				pdfl[PATH_MAX+1];	/* pdev file */
   register int			i;
   register int			j;
   fldinfo_t			*pfheadp	= NULL;	/* passed fld header */
   fldinfo_t			*pftailp	= NULL;	/* passed fld tail */
   fldinfo_t			*rfheadp	= NULL;	/* read fld header */
   register fldinfo_t		*tfp;			/* tmp fld list ptr */
   register fldinfo_t		*tfp1;			/* tmp fld list ptr */
   
   (void)setlocale(LC_ALL,"");

   /* Process the arguments and flags. */
   pgnm = *av;
   for (opterr = 0, optind = 1;
	(i = getopt(ac,av,":p:t:ACRDa:")) != EOF; )
      switch (i)
      {
	 case 'p':
	    pdnm = optarg;
	    pflag++;
	    break;

	 case 't':
	    atnm = optarg;
	    tflag++;
	    break;

	 case 'A':
	    Aflag = 1;
	    break;

	 case 'C':
	    Cflag = 1;
	    break;

	 case 'R':
	    Rflag = 1;
	    break;

	 case 'D':
	    Dflag = 1;
	    break;

	 case 'a':
	    mpd_add_fldele(&pfheadp,&pftailp,optarg,TRUE);
	    aflag++;
	    break;

	 case ':':
	    mpd_msg(stderr,MSG_MPD_MISSARG,DEFMSG_MPD_MISSARG,pgnm,
		    (char)i),
	    mpd_msg(stdout,MSG_MPD_USAGE,DEFMSG_MPD_USAGE,pgnm);
	    exit(EXIT_FAILURE);

	 case '?':
	    mpd_msg(stderr,MSG_MPD_ILLOPT,DEFMSG_MPD_ILLOPT,pgnm,
		   (char)i),
	    mpd_msg(stdout,MSG_MPD_USAGE,DEFMSG_MPD_USAGE,pgnm);
	    exit(EXIT_FAILURE);
      }
   if (!(pflag && tflag) || Aflag+Cflag+Rflag+Dflag != 1
       || Aflag|Cflag|Dflag && !aflag || Rflag && aflag || optind != ac)
   {
      mpd_msg(stderr,MSG_MPD_ILLUSE,DEFMSG_MPD_ILLUSE,pgnm),
      mpd_msg(stdout,MSG_MPD_USAGE,DEFMSG_MPD_USAGE,pgnm);
      exit(EXIT_FAILURE);
   }

   /* Determine the pseudo-device file name. */
   (void)strncpy(pdfl,(cp = getenv(ENVBASEDIR)) || (cp = getenv(ENVVARDIR))
		 ? cp : DEFVARDIR,sizeof(pdfl)-1);
   (void)strncat(pdfl,DEVDIR,sizeof(pdfl)-strlen(pdfl));
   (void)strncat(pdfl,pdnm,sizeof(pdfl)-strlen(pdfl));
   (void)strncat(pdfl,PDATSEPCHRSTR,sizeof(pdfl)-strlen(pdfl));
   (void)strncat(pdfl,atnm,sizeof(pdfl)-strlen(pdfl));

   /* For change or display options, read the file and build a list of
      fields and their values. */
   if (Aflag)			/* add the device */
   {
      struct stat			sbuf;

      if (stat(pdfl,&sbuf) != -1)
      {
         mpd_msg(stderr,MSG_MPD_PDEXISTS,DEFMSG_MPD_PDEXISTS,pgnm,pdnm);
	 exit(EXIT_FAILURE);
      }
      if (mpd_write_pdev(pdfl,pfheadp) == -1)
	 exit(EXIT_FAILURE);
   }
   else if (Cflag)		/* change the device characteristics */
   {
      if (mpd_read_pdev(pdfl,&rfheadp) == -1)
	 exit(EXIT_FAILURE);
      for (tfp = rfheadp; tfp; tfp = tfp->nextp)
      {
	 for (tfp1 = pfheadp; tfp1 && (i = strcmp(tfp1->fnm,tfp->fnm)) < 0;
	      tfp1 = tfp1->nextp)
	    ;
	 if (tfp1 && !i)
	 {
	    tfp1->fnd = TRUE;
	    if (tfp->fval)
	       free((void *)tfp->fval);
	    if (tfp1->fval)
	    {
	       MALLOC(tfp->fval,strlen(tfp1->fval)+1);
	       (void)strcpy(tfp->fval,tfp1->fval);
	    }
	    else
	       tfp->fval = NULL;
	 }
      }
      if (mpd_write_pdev(pdfl,rfheadp) == -1)
	 exit(EXIT_FAILURE);
   }
   else if (Rflag)		/* remove the device */
   {
      if (unlink(pdfl) == -1)
      {
         mpd_msg(stderr,MSG_MPD_UNLINKERR,DEFMSG_MPD_UNLINKERR,pgnm,
		 strerror(errno),pdfl);
         exit(EXIT_FAILURE);
      }
   }
   else if (Dflag)		/* display the device characteristics */
   {
      if (mpd_read_pdev(pdfl,&rfheadp) == -1)
	 exit(EXIT_FAILURE);
      for (tfp = rfheadp; tfp; tfp = tfp->nextp)
      {
	 for (tfp1 = pfheadp; tfp1 && (i = strcmp(tfp1->fnm,tfp->fnm)) < 0;
	      tfp1 = tfp1->nextp)
	    ;
	 if (tfp1 && !i)
	 {
	    tfp1->fnd = TRUE;
	    if (tfp1->fval)
	       free((void *)tfp1->fval);
	    if (tfp->fval)
	    {
	       MALLOC(tfp1->fval,strlen(tfp->fval)+1);
	       (void)strcpy(tfp1->fval,tfp->fval);
	    }
	    else
	       tfp1->fval = NULL;
	 }
      }
      for (i = TRUE, j = FALSE, tfp = pfheadp; tfp; tfp = tfp->nextp)
	 if (tfp->fnd)
	 {
	    (void)fputc(i ? i = FALSE, SETSTARTCHR : SETSEPCHR,stdout);
	    (void)fputs(tfp->fnm,stdout);
	    j++;
	 }
      if (j)
      {
	 (void)putchar('\n');
	 for (i = TRUE, tfp = pfheadp; tfp; tfp = tfp->nextp)
	    if (tfp->fnd)
	    {
	       if (i)
		  i = FALSE;
	       else
		  (void)putchar(SETSEPCHR);
	       for (ccp = *tfp->fval == SETBEGINCHR?
			  (ccp1 = mpd_parsemsg(tfp->fval))?ccp1:tfp->fval:
			  tfp->fval; *ccp; ccp++)
		  if (*ccp == SETSEPCHR)
		     (void)fputs(SMITCCMT,stdout);
		  else
		     (void)putchar((int)*ccp);
	    }
	 (void)putchar('\n');
	 (void)fflush(stdout);
      }
   }

   /* If any specified fields were not found for change or display options,
      flag them as errors. */
   if (Cflag || Dflag)
      for (tfp = pfheadp; tfp; tfp = tfp->nextp)
	 if (!tfp->fnd)
            mpd_msg(stderr,MSG_MPD_FLDNOTFND,DEFMSG_MPD_FLDNOTFND,pgnm,
		    tfp->fnm,pdfl);

   /* All the memory allocated so far (in linked lists) will be freed,
      as we're exiting.  Hence, code to free the memory is obviated. */

   return EXIT_SUCCESS;
}	/* end - main() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mpd_msg                                                      *
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
mpd_msg(FILE *outfp, int msgno, const char *defmsg, ...)
{
   static nl_catd		md = (nl_catd)-1;
   const char			*const dmsg = "d u m m y";
   char				*mp;
   const char			*msgp;
   va_list			vap;

   /* Fetch the specified message. */
   if (md == (nl_catd)-1  &&
       (md = catopen(MF_PIOBE, NL_CAT_LOCALE)) == (nl_catd)-1)
   {
      char				defmcpath[PATH_MAX+1];

      (void) strncpy(defmcpath, DEFMC_PREFIXPATH, sizeof(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      (void) strncat(defmcpath, MF_PIOBE,
		     sizeof(defmcpath)-strlen(defmcpath)-1);
      md = catopen(defmcpath, NL_CAT_LOCALE);
   }
   msgp = md == (nl_catd)-1 ? defmsg :
	  strcmp(mp = catgets(md, MF_PIOBESETNO, msgno, (char *)dmsg),
		 (char const *)dmsg) ? mp : defmsg;

   /* Parse the message with the specified args. */
   va_start(vap, defmsg);
   (void) vfprintf(outfp, msgp, vap);
   va_end(vap);

   return;
}	/* end - mpd_msg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mpd_add_fldele                                               *
*                                                                              *
* DESCRIPTION:    Parses the field info from a given string and adds a field   *
*                 info structure to the specified linked list.                 *
*                                                                              *
* PARAMETERS:     hpp		ptr to head element ptr                        *
*                 tpp		ptr to tail element ptr                        *
*                 sp		field data string                              *
*                 sflg          flag to denote whether to sort or not          *
*				( TRUE = sort; FALSE = nosort )		       *
*                                                                              *
* RETURN VALUES:  (none)	                                               *
*                                                                              *
*******************************************************************************/
static void
mpd_add_fldele(fldinfo_t **hpp,fldinfo_t **tpp,char *sp,int sflg)
{
   register char			*tcp;
   register char			*tcp1;
   register fldinfo_t			*p;

   MALLOC(p,sizeof(*p));
   p->fnd = FALSE;
   tcp1 = sp+strlen(sp);
   tcp = strtok(tcp = sp+strspn(sp,WHSPCHRS),WHSPCHRS1);
   MALLOC(p->fnm,strlen(tcp)+1);
   (void)strcpy(p->fnm,tcp);
   if (tcp += strlen(tcp)+1, tcp < tcp1)
   {
      tcp += strspn(tcp,WHSPCHRS1);
      MALLOC(p->fval,strlen(tcp)+1);
      (void)strcpy(p->fval,tcp);
   }
   else
      p->fval = NULL;
   if (sflg)
   {
      register fldinfo_t	*fip;
      register fldinfo_t	*fip1;
      int			rc;
        
      for (fip = *hpp, fip1 = NULL;
	   fip && (rc = strcmp(p->fnm,fip->fnm)) > 0;
	   fip1 = fip, fip = fip->nextp)
	 ;
      *(fip1 ? &fip1->nextp : hpp) = p;
      if (fip && !rc)
      {
	 p->nextp = fip->nextp;
	 if (fip->fval)
	    free((void *)fip->fval);
	 free((void *)fip->fnm);
	 free((void *)fip);
      }
      else
	 p->nextp = fip;
   }
   else
   {
      if (p->nextp = NULL, *hpp)
	 (*tpp)->nextp = p,
	 *tpp = p;
      else
	 *hpp = *tpp = p;
   }
}	/* end - mpd_add_fldele() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mpd_read_pdev                                                *
*                                                                              *
* DESCRIPTION:    Reads the pseudo-device file and builds a list of field      *
*                 info structures.                                             *
*                                                                              *
* PARAMETERS:     flnm		pseudo-device file name                        *
*                 fipp		ptr to ptr to field info struct                *
*                                                                              *
* RETURN VALUES:  ui		no of successfully built field info structs    *
*				( -1 = Error; 0 = none; >0 = number )	       *
*                                                                              *
*******************************************************************************/
static int
mpd_read_pdev(const char *flnm,fldinfo_t **fipp)
{
   char 			rdbuf[LINE_MAX+1];
   FILE				*fp;
   register uint_t		ui	= 0U;
   register caddr_t		cp;
   fldinfo_t			*tp	= NULL;

   if (!(fp = fopen(flnm,"r")))
   {
      mpd_msg(stderr,MSG_MPD_FOPENERR,DEFMSG_MPD_FOPENERR,pgnm,strerror(errno),
	      flnm);
      return -1;
   }
   for (*fipp = NULL; fgets(rdbuf,sizeof rdbuf,fp); )
   {
      (void)strtok(rdbuf,"\n");
      if (!*rdbuf || (cp = rdbuf+strspn(rdbuf,WHSPCHRS), !*cp) ||
          *cp == CMNTCHR || *cp == ASSGCHR || !strchr(cp,ASSGCHR))
         continue;
      mpd_add_fldele(fipp,&tp,cp,FALSE);
      ui++;
   }
   if (ferror(fp))
   {
      mpd_msg(stderr,MSG_MPD_FRDERR,DEFMSG_MPD_FRDERR,pgnm,strerror(errno),
	      flnm);
      return -1;
   }
   (void)fclose(fp);

   return (int)ui;
}	/* end - mpd_read_pdev() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mpd_write_pdev                                               *
*                                                                              *
* DESCRIPTION:    Writes a list of field info structures to the pseudo-device  *
*                 file.                                                        *
*                                                                              *
* PARAMETERS:     flnm		pseudo-device file name                        *
*                 fipp		ptr to ptr to field info struct                *
*                                                                              *
* RETURN VALUES:  -1		Error                                          *
*                 TRUE		successful                                     *
*                                                                              *
*******************************************************************************/
static int
mpd_write_pdev(const char *flnm,register fldinfo_t *fip)
{
   FILE				*fp;
   struct sigaction		sigign;
   struct sigaction		osighup;
   struct sigaction		osigint;
   struct sigaction		osigterm;
   struct group			*grpp	=	getgrnam(PRINTQ_GNM);
   gid_t			pqgid	=	grpp?grpp->gr_gid:PRINTQ_GID;

   sigign.sa_handler = SIG_IGN;
   (void)sigemptyset(&sigign.sa_mask);
   sigign.sa_flags = 0;
   (void)sigaction(SIGHUP,&sigign,&osighup);
   (void)sigaction(SIGINT,&sigign,&osigint);
   (void)sigaction(SIGTERM,&sigign,&osigterm);
   if (!(fp = fopen(flnm,"w")))
   {
      mpd_msg(stderr,MSG_MPD_FOPENERR,DEFMSG_MPD_FOPENERR,pgnm,strerror(errno),
	      flnm);
      return -1;
   }
   if (chmod(flnm,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH) == -1)
   {
      (void)unlink(flnm);
      mpd_msg(stderr,MSG_MPD_FCHERR,DEFMSG_MPD_FCHERR,pgnm,strerror(errno),
	      flnm);
      return -1;
   }
   if (fchownx(fileno(fp),-1,pqgid,T_OWNER_AS_IS) == -1)
   {
      (void)unlink(flnm);
      mpd_msg(stderr,MSG_MPD_FCHERR,DEFMSG_MPD_FCHERR,pgnm,strerror(errno),
	      flnm);
      return -1;
   }
   for ( ; fip; fip = fip->nextp)
      (void)fprintf(fp,PDFLRECFMT,fip->fnm,fip->fval ? fip->fval : "");
   if (ferror(fp))
   {
      mpd_msg(stderr,MSG_MPD_FWRERR,DEFMSG_MPD_FWRERR,pgnm,strerror(errno),
	      flnm);
      return -1;
   }
   (void)fclose(fp);
   (void)sigaction(SIGHUP,&osighup,(struct sigaction *)NULL);
   (void)sigaction(SIGINT,&osigint,(struct sigaction *)NULL);
   (void)sigaction(SIGTERM,&osigterm,(struct sigaction *)NULL);

   return TRUE;
}	/* end - mpd_write_pdev() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mpd_getmsg                                                   *
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
mpd_getmsg(const char *catnm,int setno,int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

   /* Fetch the specified message. */
   if (strcmp(catnm,prevcatnm))
   {
      (void)strncpy(prevcatnm,catnm,sizeof(prevcatnm)-1),
      *(prevcatnm+sizeof(prevcatnm)-1) = 0;
      if (md != (nl_catd)-1)
         md = ((void)catclose(md), (nl_catd)-1);
   }
   if (md == (nl_catd)-1  &&
       (md = catopen((char *)catnm,NL_CAT_LOCALE)) == (nl_catd)-1)
   {
      (void)strncpy(defmcpath,DEFMC_PREFIXPATH,sizeof(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      (void)strncat(defmcpath,catnm,sizeof(defmcpath)-strlen(defmcpath)-1);
      if ((md = catopen(defmcpath,NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md,setno,msgno,(char *)dmsg),
		 (char const *)dmsg) ? mp : NULL;
}	/* end - mpd_getmsg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mpd_parsemsg                                                 *
*                                                                              *
* DESCRIPTION:    Parse a given message and fetch it, if necessary.            *
*                                                                              *
* PARAMETERS:     mp		msg                                            *
*                                                                              *
* RETURN VALUES:  fmp 		fetched msg                                    *
*                                                                              *
*******************************************************************************/
static const char *
mpd_parsemsg(const char *mp)
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
       !(sp = mpd_getmsg(mcnm,setno?setno:MF_PIOATTR1SETNO,msgno)))
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
}	/* end - mpd_parsemsg() */


