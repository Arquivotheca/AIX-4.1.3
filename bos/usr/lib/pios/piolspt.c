static char sccsid[] = "@(#)80  1.3  src/bos/usr/lib/pios/piolspt.c, cmdpios, bos411, 9428A410j 3/31/94 14:28:47";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  main(), lpt_getmsg(), lpt_msg()
 *             lpt_parsemsg(), lpt_bld_mlist(), lpt_bld_plist()
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/mode.h>
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
#include <IN/AFdefs.h>
#include <piobe_msg.h>

/* External declarations */
extern int		strcasecmp(const char *,const char *);
					/* should've been in <string.h> */
extern int		optopt;		/* should've been in <stdlib.h> */

/* Misc. macros */
#define STRGZARG(s)		#s
#define STRGZ(s)		STRGZARG(s)
#define PIOLSVP			"/usr/lib/lpd/pio/etc/piolsvp"
#define INVFLNM			"/usr/lib/lpd/pio/etc/printers.inv"
#define WKBUFLEN		(1023)
#define MAXIFRECSZ		(LINE_MAX)
#define MAXIFATRNO		(10)
#define MAXPTRNMLEN		(PATH_MAX)
#define MAXPTRDESCLEN		(WKBUFLEN)
#define MAXMFRNMLEN		(LINE_MAX)
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc/"
#define MF_PIOBE		"piobe.cat"
#define MF_PIOBESETNO		(8)
#define MF_PIOATTR1SETNO	(1)
#define DEFBASEDIR		"/usr/lib/lpd/pio"
#define ENVBASEDIR		"PIOBASEDIR"
#define ENVVARDIR		"PIOVARDIR"
#define PREDEFDIR		"/predef/"
#define ATTRNMLEN		2
#define SETBEGINCHR		'['
#define SETENDCHR		']'
#define MSGSEPCHR		';'
#define FLDSEPCHR		','
#define CUSFSEPCHR		':'
#define PTRDSSEP		'.'
#define CMNTCHR			'#'
#define WHSPCHRS		" \t"
#define PRTLISTSEPSTR		", \t"
#define ATPRTDESC		"mL"
#define ATMFRNAME		"zM"
#define MFRDISPFMT		"%-s"
#define PTRDISPFMT		"%-*.*s  %-s"
#define IFKW_PTYPE		"type"
#define IFKW_PDESC		"description"
#define IFKW_PDMC		"desc_cat_file"
#define IFKW_PDSNO		"desc_cat_setno"
#define IFKW_PDMNO		"desc_cat_msgno"
#define IFKW_MFR		"manufacturer"
#define GENERICMFR		"generic"
#define OTHERMFR		"+++OTHER+++"
#define OSPPTR			"osp"
#define OPPPTR			"opp"
#define GENERICPTR		"generic"

#if defined(MIN)
#undef MIN
#endif		/* MIN */
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#define ARRAYSIZ(a)		(sizeof(a)/sizeof(*a))
#define MALLOC(p, sz)		do \
				{ \
				   if (!((p) = malloc((size_t)(sz)))) \
				   { \
         			      lpt_msg(stderr, MSG_LPT_MALLOCERR, \
					      DEFMSG_LPT_MALLOCERR, pgnm, \
					      strerror(errno)); \
				      exit(EXIT_FAILURE); \
				   } \
				} while (0)

/* Macros for default messages */
#define DEFMSG_LPT_ERRMSG	"Cannot access the message catalog " \
				MF_PIOBE ".\n"
#define DEFMSG_LPT_USAGE	"Usage:\t%s -t [ -A AttachmentType ] Maker\n" \
				"\t\t-T -A AttachmentType\n" \
				"\t\t-m\n"
#define MSG_LPT_MISSARG		MSG_LVP_MISSARG
#define DEFMSG_LPT_MISSARG	"%s: Missing argument for the flag -%c\n"
#define MSG_LPT_ILLOPT		MSG_LVP_ILLOPT
#define DEFMSG_LPT_ILLOPT	"%s: Illegal flag -%c\n"
#define MSG_LPT_ILLUSE		MSG_LVP_ILLUSE
#define DEFMSG_LPT_ILLUSE	"%s: Illegal usage\n"
#define MSG_LPT_MALLOCERR	MSG_LVP_MALLOCERR
#define DEFMSG_LPT_MALLOCERR	"%s: Error '%s' in malloc()\n"
#define MSG_LPT_DOPENERR	MSG_LVP_DOPENERR
#define DEFMSG_LPT_DOPENERR	"%s: Error '%s' in opening the directory %s\n"
#define MSG_LPT_DRDERR		MSG_LVP_DRDERR
#define DEFMSG_LPT_DRDERR	"%s: Error '%s' in reading the directory %s\n"
#define MSG_LPT_FOPENERR	MSG_LVP_FOPENERR
#define DEFMSG_LPT_FOPENERR	"%s: Error '%s' in opening the file %s\n"
#define MSG_LPT_FRDERR		MSG_LVP_FRDERR
#define DEFMSG_LPT_FRDERR	"%s: Error '%s' in reading the file %s\n"
#define MSG_LPT_POPENERR	MSG_LVP_POPENERR
#define DEFMSG_LPT_POPENERR	"%s: Error '%s' in opening a pipe to command "\
				"%s\n"
#define MSG_LPT_PRDERR		MSG_LVP_PRDERR
#define DEFMSG_LPT_PRDERR	"%s: Error '%s' in reading a pipe to command "\
				"%s\n"
#define DEFMSG_LPT_GENMFR	"Other (Select this if your printer type is " \
				"not listed above)\n"
#define DEFMSG_LPT_GENPTR	"Generic Printer"

/* Typedefs for maker and printer type lists */
typedef struct mele {		/* manufacturer structure */
   struct mele		*me_nextp;	/* next element */
   const char		*me_mnm;	/* maker name */
} mele_t;
typedef enum { FROMINV,FROMPCF }	disrc_t;
typedef struct pele {		/* printer type structure */
   struct pele		*pe_nextp;	/* next element */
   struct pele		*pe_prevp;	/* prev element */
   const char		*pe_ptype;	/* printer type */
   disrc_t		pe_disrcf;	/* flag to denote source of desc info */
   union {
      struct {
         const char	*pe_pdesc;	/* printer description */
         const char	*pe_mcnm;	/* message catalog name */
         ushort_t	pe_setno;	/* message set number */
         ushort_t	pe_msgno;	/* message number */
      } 		pe_diinv;	/* description info from inventory */
      struct {
	 const char	*pe_pdesc;	/* printer description */
      }			pe_dipcf;	/* description info from predef cf */
   } 			pe_di;		/* desc info (union) */
} pele_t;

/* Local function declarations */
static const char	*lpt_getmsg(const char *,int,int);
static void 		lpt_msg(FILE *,int,const char *,...);
static char		*lpt_parsemsg(const char *);
static int		lpt_bld_mlist(mele_t **);
static int		lpt_bld_plist(pele_t **,const char *,const char *);

/* Local variable definitions */
const char		*pgnm;
static char		bdir[PATH_MAX+1];




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
main(int ac,char **av)
{
   register int			i;
   mele_t			*mheadp		= NULL;	/* m element head */
   register mele_t		*mp;
   pele_t			*pheadp		= NULL; /* p element head */
   register pele_t		*pp;
   uchar_t			mflag		= FALSE;
   uchar_t			tflag		= FALSE;
   uchar_t			Tflag		= FALSE;
   uchar_t			Aflag		= FALSE;
   char				*cp;
   const char			*ccp;
   const char			*attype		= NULL;	/* attachment type */
   
   (void)setlocale(LC_ALL,"");

   /* Process the arguments and flags. */
   pgnm = *av;
   for (opterr = 0, optind = 1;
	(i = getopt(ac,av,":mtTA:")) != EOF; )
      switch (i)
      {
	 case 'm':
	    mflag = TRUE;
	    break;

	 case 't':
	    tflag = TRUE;
	    break;

	 case 'T':
	    Tflag = TRUE;
	    break;

	 case 'A':
	    attype = optarg;
	    Aflag = TRUE;
	    break;

	 case ':':
	    lpt_msg(stderr,MSG_LPT_MISSARG,DEFMSG_LPT_MISSARG,pgnm,
		    (char)optopt),
	    lpt_msg(stdout,MSG_LPT_USAGE,DEFMSG_LPT_USAGE,pgnm);
	    exit(EXIT_FAILURE);

	 case '?':
	    lpt_msg(stderr,MSG_LPT_ILLOPT,DEFMSG_LPT_ILLOPT,pgnm,
		    (char)optopt),
	    lpt_msg(stdout,MSG_LPT_USAGE,DEFMSG_LPT_USAGE,pgnm);
	    exit(EXIT_FAILURE);
      }
   if (mflag+tflag+Tflag != TRUE || mflag && Aflag || Tflag && !Aflag ||
       ac != optind+(tflag?1:0))
   {
      lpt_msg(stderr,MSG_LPT_ILLUSE,DEFMSG_LPT_ILLUSE,pgnm),
      lpt_msg(stdout,MSG_LPT_USAGE,DEFMSG_LPT_USAGE,pgnm);
      exit(EXIT_FAILURE);
   }
   (void)strncpy(bdir,(cp = getenv(ENVBASEDIR)) || (cp = getenv(ENVVARDIR)) ?
		 cp : DEFBASEDIR,sizeof(bdir)-1),
   *(bdir+sizeof(bdir)-1) = 0;

   /* Generate a list of manufacturers and display them. */
   if (mflag)
   {
      if (!lpt_bld_mlist(&mheadp))
	 exit(EXIT_FAILURE);
      for (mp = mheadp; mp; mp = mp->me_nextp)
	 (void)printf(MFRDISPFMT "\n",mp->me_mnm);
      lpt_msg(stdout,MSG_LPT_GENMFR,DEFMSG_LPT_GENMFR,pgnm),
      (void)putchar('\n');
      (void)fflush(stdout);
   }

   /* Generate a list of printer types and display them. */
   if (tflag|Tflag)
   {
      size_t			len;
      size_t			maxlen;
      uchar_t			omfrflag;

      if (!lpt_bld_plist(&pheadp,tflag?av[optind]:NULL,attype))
	 exit(EXIT_FAILURE);
      for (maxlen = tflag ? (omfrflag = !strcmp(av[optind],OTHERMFR))?
		    strlen(GENERICPTR):0UL : 0UL,
	   pp = pheadp; pp; pp = pp->pe_nextp)
	 if ((len = strlen(pp->pe_ptype)) > maxlen)
	    maxlen = len;
      for (pp = pheadp; pp; pp = pp->pe_nextp)
      {
	 if (pp->pe_disrcf == FROMINV)
	    (void)printf(PTRDISPFMT "\n",maxlen,maxlen,pp->pe_ptype,
			 (ccp = lpt_getmsg(pp->pe_di.pe_diinv.pe_mcnm,
					   pp->pe_di.pe_diinv.pe_setno,
					   pp->pe_di.pe_diinv.pe_msgno)) ?
			 ccp : pp->pe_di.pe_diinv.pe_pdesc);
	 else
	    if (ccp = lpt_parsemsg(pp->pe_di.pe_dipcf.pe_pdesc))
	       (void)printf(PTRDISPFMT "\n",maxlen,maxlen,pp->pe_ptype,ccp),
	       free((void *)ccp);
	    else
	       (void)printf(PTRDISPFMT "\n",maxlen,maxlen,pp->pe_ptype,"");
      }
      if (tflag)
      {
         if (omfrflag)
            (void)printf(PTRDISPFMT,maxlen,maxlen,GENERICPTR,""),
            lpt_msg(stdout,MSG_LPT_GENPTR,DEFMSG_LPT_GENPTR,pgnm);
         else
            lpt_msg(stdout,MSG_LPT_GENMFR,DEFMSG_LPT_GENMFR,pgnm);
         (void)putchar('\n');
      }
      (void)fflush(stdout);
   }

   /* Since we're exiting anyway, there is no need to free the memory
      allocated so far. */

   return EXIT_SUCCESS;
}	/* end - main() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lpt_bld_mlist                                                *
*                                                                              *
* DESCRIPTION:    Build a list of manufacturers.                               *
*                                                                              *
* PARAMETERS:     mpp		ptr to maker list                              *
*                                                                              *
* RETURN VALUES:  TRUE/FALSE	success/failure                                *
*                                                                              *
*******************************************************************************/
static int
lpt_bld_mlist(mele_t **mpp)
{
#define ADD_ELE_MLIST(sp)	\
   do \
   { \
      for (mp = *mpp, mp1 = NULL; \
	   mp && (rc = strcmp(sp,mp->me_mnm)) > 0; \
	   mp1 = mp, mp = mp->me_nextp) \
	 ; \
      if (!mp || rc) \
      { \
	 MALLOC(cp,strlen(sp)+1); \
	 (void)strcpy(cp,sp); \
	 if (mp1) \
	 { \
	    MALLOC(mp1->me_nextp,sizeof(mele_t)); \
	    mp1->me_nextp->me_mnm = cp; \
	    mp1->me_nextp->me_nextp = mp; \
	 } \
	 else \
	 { \
	    MALLOC(*mpp,sizeof(mele_t)); \
	    (*mpp)->me_mnm = cp; \
	    (*mpp)->me_nextp = mp; \
	 } \
      } \
   } while (0)
      
   AFILE_t			invfp;
   ATTR_t			stnz;
   register char		*cp;
   register char const		*ccp;
   register mele_t		*mp;
   register mele_t		*mp1;
   int				rc;
   char				pdir[PATH_MAX+1];
   char				pcf[PATH_MAX+1];
   DIR				*Dp;
   register struct dirent	*dp;
   struct stat			sbuf;
   char				ptype[MAXPTRNMLEN+1];
   char				prevptype[MAXPTRNMLEN+1]	= "";

   /* Build a list of maker names from the inventory file first. */
   if (!(invfp = AFopen(INVFLNM,MAXIFRECSZ,MAXIFATRNO)))
   {
      /* For now, if the inventory file can not be opened for reading,
	 do not consider it as an error.  Instead, skip reading this file,
	 and continue on to reading the 'predef' files. */
   }
   else
   {
   while (stnz = AFnxtrec(invfp))
      if ((ccp = AFgetatr(stnz,IFKW_MFR)) && strcasecmp(ccp,GENERICMFR))
	 ADD_ELE_MLIST(ccp);
   (void)AFclose(invfp);
   }

   /* Build an additional list of manufacturers from the 'predef' colon
      files. */
   *(pdir+sizeof(pdir)-1) = 0;
   (void)strncpy(pdir,bdir,sizeof(pdir)-1);
   (void)strncat(pdir,PREDEFDIR,sizeof(pdir)-strlen(pdir));
   if (!(Dp = opendir(pdir)))
   {
      lpt_msg(stderr,MSG_LPT_DOPENERR,DEFMSG_LPT_DOPENERR,pgnm,strerror(errno),
	      pdir);
      return FALSE;
   }
   while (dp = readdir(Dp))
   {
      FILE			*pfp;
      char			rdbuf[LINE_MAX+1];
      size_t			len;

      if (!(ccp = strchr(dp->d_name,PTRDSSEP)) || ccp == dp->d_name ||
	  (++ccp, !*ccp) || strchr(ccp,PTRDSSEP) ||
	  strchr(dp->d_name,CUSFSEPCHR) ||
	  (*(pcf+sizeof(pcf)-1) = 0,
	   (void)strncpy(pcf,pdir,sizeof(pcf)-1),
	   (void)strncat(pcf,dp->d_name,sizeof(pcf)-strlen(pcf)),
	   stat(pcf,&sbuf) == -1) ||
	  (sbuf.st_mode&S_IFMT) != S_IFREG)
	 continue;
      
      ccp = strchr(dp->d_name,PTRDSSEP);
      (void)memcpy((void *)ptype,(void *)dp->d_name,
		   len = MIN(ccp-dp->d_name,sizeof(ptype)-1)),
      *(ptype+len) = 0;
      if (!strcmp(ptype,prevptype))
	 continue;
      if (!(pfp = fopen(pcf,"r")))
	 continue;
      while (fgets(rdbuf,sizeof rdbuf,pfp))
      {
	 (void)strtok(rdbuf,"\n");
	 if (!*rdbuf || (ccp = rdbuf+strspn(rdbuf,WHSPCHRS), !*ccp) ||
	     *ccp == CMNTCHR || !(ccp = strchr(ccp,CUSFSEPCHR)) ||
	     !(ccp = strchr(ccp+1,CUSFSEPCHR)) ||
	     (++ccp, memcmp((void *)ccp,(void *)ATMFRNAME,(size_t)ATTRNMLEN)))
	    continue;
	 if (!(ccp = strchr(ccp,CUSFSEPCHR)) ||
	     !(ccp = strchr(ccp+1,CUSFSEPCHR)) ||
	     !*++ccp)
	    continue;
	 (void)strcpy(prevptype,ptype);
	 if (strcasecmp(ccp,GENERICMFR))
	    ADD_ELE_MLIST(ccp);
	 break;
      }
      if (ferror(pfp))
      {
         lpt_msg(stderr,MSG_LPT_FRDERR,DEFMSG_LPT_FRDERR,pgnm,strerror(errno),
		 pcf);
	 return FALSE;
      }
      (void)fclose(pfp);
   }
   (void)closedir(Dp);

   return TRUE;
#undef ADD_ELE_MLIST
}	/* end - lpt_bld_mlist() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lpt_bld_plist                                                *
*                                                                              *
* DESCRIPTION:    Build a list of printer types and their descriptions.        *
*                                                                              *
* PARAMETERS:     ppp		ptr to printer list                            *
*                 mfrnm		manufacturer name                              *
*                 atnm		attachment type name                           *
*                                                                              *
* RETURN VALUES:  TRUE/FALSE	success/failure                                *
*                                                                              *
*******************************************************************************/
static int
lpt_bld_plist(pele_t **ppp,const char *mfrnm,const char *atnm)
{
#define ADD_ELE_PLIST(pt,sf,pd,pm,ps,pM)	\
   do \
   { \
      for (pp = *ppp, pp2 = NULL; \
	   pp && (rc = strcmp((pt),pp->pe_ptype)) > 0; \
	   pp2 = pp, pp = pp->pe_nextp) \
	 ; \
      if (!pp || rc) \
      { \
	 MALLOC(pp1,sizeof(pele_t)); \
	 MALLOC(cp,strlen(pt)+1); \
	 (void)strcpy(cp,(pt)); \
	 pp1->pe_ptype = cp; \
	 MALLOC(cp,strlen(pd)+1); \
	 (void)strcpy(cp,(pd)); \
	 if ((pp1->pe_disrcf = (sf)) == FROMINV) \
	 { \
	    pp1->pe_di.pe_diinv.pe_pdesc = cp; \
	    MALLOC(cp,strlen(pm)+1); \
	    (void)strcpy(cp,(pm)); \
	    pp1->pe_di.pe_diinv.pe_mcnm = cp; \
	    pp1->pe_di.pe_diinv.pe_setno = (ps); \
	    pp1->pe_di.pe_diinv.pe_msgno = (pM); \
	 } \
	 else \
	    pp1->pe_di.pe_dipcf.pe_pdesc = cp; \
	 if (pp1->pe_nextp = pp) \
	    pp->pe_prevp = pp1; \
	 if (pp1->pe_prevp = pp2) \
	    pp2->pe_nextp = pp1; \
	 if (pp == *ppp) \
	    *ppp = pp1; \
      } \
   } while (0)
#define DEL_ELE_PLIST(p)	\
   do \
   { \
      if ((p)->pe_nextp) \
	 (p)->pe_nextp->pe_prevp = (p)->pe_prevp; \
      if ((p)->pe_prevp) \
	 (p)->pe_prevp->pe_nextp = (p)->pe_nextp; \
      if ((p) == *ppp) \
	 *ppp = (p)->pe_nextp; \
      if ((p)->pe_disrcf == FROMINV) \
      { \
	 free((void *)(p)->pe_di.pe_diinv.pe_pdesc); \
	 free((void *)(p)->pe_di.pe_diinv.pe_mcnm); \
      } \
      else \
	 free((void *)(p)->pe_di.pe_dipcf.pe_pdesc); \
      free((void *)(p)->pe_ptype); \
      free((void *)(p)); \
   } while (0)
      
   AFILE_t			invfp;
   ATTR_t			stnz;
   register char		*cp;
   register char const		*ccp;
   register pele_t		*pp;
   register pele_t		*pp1;
   register pele_t		*pp2;
   int				rc;
   char				pdir[PATH_MAX+1];
   char				pcf[PATH_MAX+1];
   DIR				*Dp;
   register struct dirent	*dp;
   struct stat			sbuf;
   char				ptype[MAXPTRNMLEN+1];
   char				prevptype[MAXPTRNMLEN+1]	= "";
   char				pdesc[MAXPTRDESCLEN+1];
   char				mfr[MAXMFRNMLEN+1];
   char				pdmc[PATH_MAX+1];
   ulong_t			pdsno;
   ulong_t			pdmno;
   uchar_t			omfrflag;
   uchar_t			tflag				= !!mfrnm;

   /* Build a list of printer types from the inventory file first.
      Do this, only if the specified manufacturer is not 'Other'. */
   if (*(ptype+sizeof(ptype)-1) =
       *(pdesc+sizeof(pdesc)-1) =
       *(pdmc+sizeof(pdmc)-1) =
       *(mfr+sizeof(mfr)-1) = 0,
       !tflag || !(omfrflag = !strcmp(mfrnm,OTHERMFR)))
   {
      if (!(invfp = AFopen(INVFLNM,MAXIFRECSZ,MAXIFATRNO)))
      {
         /* For now, if the inventory file can not be opened for reading,
	    do not consider it as an error.  Instead, skip reading this file,
	    and continue on to reading the 'predef' files. */
      }
      else
      {
      while (stnz = AFnxtrec(invfp))
      {
         if (!(ccp = AFgetatr(stnz,IFKW_MFR)) || tflag && strcmp(ccp,mfrnm))
	    continue;
         if (!(ccp = AFgetatr(stnz,IFKW_PTYPE)) || !strcmp(ccp,OSPPTR) ||
	     !strcmp(ccp,OPPPTR))
	    continue;
	 (void)strncpy(ptype,ccp,sizeof(ptype)-1);
	 *pdesc = *pdmc = 0,
	 pdsno = MF_PIOATTR1SETNO, pdmno = USHRT_MAX;
	 if (ccp = AFgetatr(stnz,IFKW_PDESC))
	    (void)strncpy(pdesc,ccp,sizeof(pdesc)-1);
	 if (ccp = AFgetatr(stnz,IFKW_PDMC))
	    (void)strncpy(pdmc,ccp,sizeof(pdmc)-1);
	 if (ccp = AFgetatr(stnz,IFKW_PDSNO))
	    pdsno = strtoul(ccp,NULL,10);
	 if (ccp = AFgetatr(stnz,IFKW_PDMNO))
	    pdmno = strtoul(ccp,NULL,10);
	 ADD_ELE_PLIST(ptype,FROMINV,pdesc,pdmc,pdsno,pdmno);
      }
      (void)AFclose(invfp);
      }
   }

   /* Build an additional list of manufacturers from the 'predef' colon
      files. If the manufacturer specified is 'Other', consider the colon
      files that do not have manufacturer specified in them.  Else,
      consider those that have the same manufacturer specified. */
   *(pdir+sizeof(pdir)-1) = 0;
   (void)strncpy(pdir,bdir,sizeof(pdir)-1);
   (void)strncat(pdir,PREDEFDIR,sizeof(pdir)-strlen(pdir));
   if (!(Dp = opendir(pdir)))
   {
      lpt_msg(stderr,MSG_LPT_DOPENERR,DEFMSG_LPT_DOPENERR,pgnm,strerror(errno),
	      pdir);
      return FALSE;
   }
   while (dp = readdir(Dp))
   {
      FILE			*pfp;
      char			rdbuf[LINE_MAX+1];
      uchar_t			pdescfnd = FALSE;
      uchar_t			mfrnmfnd = !tflag;
      register const char	*ccp2;
      size_t			len;

      if (!(ccp = strchr(dp->d_name,PTRDSSEP)) || ccp == dp->d_name ||
	  (++ccp, !*ccp) || strchr(ccp,PTRDSSEP) ||
	  strchr(dp->d_name,CUSFSEPCHR) ||
	  (*(pcf+sizeof(pcf)-1) = 0,
	   (void)strncpy(pcf,pdir,sizeof(pcf)-1),
	   (void)strncat(pcf,dp->d_name,sizeof(pcf)-strlen(pcf)),
	   stat(pcf,&sbuf) == -1) ||
	  (sbuf.st_mode&S_IFMT) != S_IFREG)
	 continue;
      
      ccp = strchr(dp->d_name,PTRDSSEP);
      (void)memcpy((void *)ptype,(void *)dp->d_name,
		   len = MIN(ccp-dp->d_name,sizeof(ptype)-1)),
      *(ptype+len) = 0;
      if (!strcmp(ptype,prevptype))
	 continue;
      if (!(pfp = fopen(pcf,"r")))
	 continue;
      for (*pdesc = *mfr = 0;
	   !(pdescfnd && mfrnmfnd) && fgets(rdbuf,sizeof rdbuf,pfp); )
      {
	 (void)strtok(rdbuf,"\n");
	 if (!*rdbuf || (ccp = rdbuf+strspn(rdbuf,WHSPCHRS), !*ccp) ||
	     *ccp == CMNTCHR || !(ccp = strchr(ccp,CUSFSEPCHR)) ||
	     !(ccp = strchr(ccp+1,CUSFSEPCHR)))
	    continue;
	 if (!(++ccp, ccp2 = strchr(ccp,CUSFSEPCHR)) ||
	     !(ccp2 = strchr(ccp2+1,CUSFSEPCHR)) ||
	     !*++ccp2)
	    continue;
	 if (!memcmp((void *)ccp,(void *)ATPRTDESC,(size_t)ATTRNMLEN))
	    (void)strncpy(pdesc,ccp2,sizeof(pdesc)-1), pdescfnd++;
	 else if (tflag &&
		  !memcmp((void *)ccp,(void *)ATMFRNAME,(size_t)ATTRNMLEN))
	    (void)strncpy(mfr,ccp2,sizeof(mfr)-1), mfrnmfnd++,
	    (void)strcpy(prevptype,ptype);
      }
      if (ferror(pfp))
      {
         lpt_msg(stderr,MSG_LPT_FRDERR,DEFMSG_LPT_FRDERR,pgnm,strerror(errno),
		 pcf);
	 return FALSE;
      }
      (void)fclose(pfp);
      if (!tflag || omfrflag && !mfrnmfnd ||
	  !omfrflag && mfrnmfnd && !strcmp(mfrnm,mfr))
      {
	 ADD_ELE_PLIST(ptype,FROMPCF,pdesc,"",0,0);
	 (void)strcpy(prevptype,ptype);
      }
   }
   (void)closedir(Dp);

   /* If OTHER manufacturer is specified, remove the printer types that
      we extracted from the colon files and are specified in the inventory
      file. */
   if (tflag && omfrflag)
   {
      if (!(invfp = AFopen(INVFLNM,MAXIFRECSZ,MAXIFATRNO)))
      {
         /* For now, if the inventory file can not be opened for reading,
	    do not consider it as an error.  Instead, skip reading this file,
	    and continue on to reading the 'predef' files. */
      }
      else
      {
      while (stnz = AFnxtrec(invfp))
      {
         if (!(ccp = AFgetatr(stnz,IFKW_PTYPE)))
	    continue;
	 (void)strncpy(ptype,ccp,sizeof(ptype)-1);
         for (pp = *ppp;
	      pp && (rc = strcmp(ptype,pp->pe_ptype)) > 0;
	      pp = pp->pe_nextp)
	    ;
         if (pp && !rc)
	    DEL_ELE_PLIST(pp);
      }
      (void)AFclose(invfp);
      }
   }

   /* If attachment type is specified, check to see if 'supported' list is
      specified for this.  If so, keep only those that are included in this
      list.  If 'supported' list is not specified, check to see if
      'unsupported' list is specified.  If so, remove these from the
      previously generated list. */
   if (atnm)
   {
      FILE				*pfp;
      char				cmd[PATH_MAX+1];
      char				rdbuf[LINE_MAX+1];
      char				wkbuf[LINE_MAX+1];
      uchar_t				supfnd = FALSE;

      *(cmd+sizeof(cmd)-1) = 0;
      (void)strncpy(cmd,PIOLSVP,sizeof(cmd)-1);
      (void)strncat(cmd," -N",sizeof(cmd)-strlen(cmd));
      (void)strncat(cmd,atnm,sizeof(cmd)-strlen(cmd));
      (void)strncat(cmd," -nsupported",sizeof(cmd)-strlen(cmd));
      if (!(pfp = popen(cmd,"r")))
      {
         lpt_msg(stderr,MSG_LPT_POPENERR,DEFMSG_LPT_POPENERR,pgnm,
		 strerror(errno),cmd);
	 return FALSE;
      }
      if (fgets(rdbuf,sizeof rdbuf,pfp) &&
	  ((void)strtok(rdbuf,"\n"), *(cp = rdbuf+strspn(rdbuf,WHSPCHRS))))
      {
	 for (supfnd = TRUE, *(wkbuf+sizeof(wkbuf)-1) = 0, pp = *ppp; pp;
	      pp = pp1)
	 {
            uchar_t				fnd = FALSE;

	    pp1 = pp->pe_nextp;
	    (void)strncpy(wkbuf,cp,sizeof(wkbuf)-1);
	    for (ccp = strtok(wkbuf,PRTLISTSEPSTR); ccp;
		 ccp = strtok(NULL,PRTLISTSEPSTR))
	       if (!strcmp(ccp,pp->pe_ptype))
	       {
		  fnd++;
		  break;
	       }
	    if (!fnd)
	       DEL_ELE_PLIST(pp);
	 }
      }
      if (ferror(pfp))
      {
         lpt_msg(stderr,MSG_LPT_PRDERR,DEFMSG_LPT_PRDERR,pgnm,
	         strerror(errno),cmd);
         return FALSE;
      }
      (void)pclose(pfp);
      if (!supfnd)
      {
         (void)strncpy(cmd,PIOLSVP,sizeof(cmd)-1);
         (void)strncat(cmd," -N",sizeof(cmd)-strlen(cmd));
         (void)strncat(cmd,atnm,sizeof(cmd)-strlen(cmd));
         (void)strncat(cmd," -nunsupported",sizeof(cmd)-strlen(cmd));
         if (!(pfp = popen(cmd,"r")))
         {
            lpt_msg(stderr,MSG_LPT_POPENERR,DEFMSG_LPT_POPENERR,pgnm,
		    strerror(errno),cmd);
	    return FALSE;
         }
         if (fgets(rdbuf,sizeof rdbuf,pfp) &&
	     ((void)strtok(rdbuf,"\n"), *(cp = rdbuf+strspn(rdbuf,WHSPCHRS))))
         {
	    for (pp = *ppp; pp; pp = pp1)
	    {
	       pp1 = pp->pe_nextp;
	       (void)strncpy(wkbuf,cp,sizeof(wkbuf)-1);
	       for (ccp = strtok(wkbuf,PRTLISTSEPSTR); ccp;
		    ccp = strtok(NULL,PRTLISTSEPSTR))
	          if (!strcmp(ccp,pp->pe_ptype))
	          {
		     DEL_ELE_PLIST(pp);
		     break;
	          }
	    }
         }
         if (ferror(pfp))
         {
            lpt_msg(stderr,MSG_LPT_PRDERR,DEFMSG_LPT_PRDERR,pgnm,
	            strerror(errno),cmd);
            return FALSE;
         }
         (void)pclose(pfp);
      }
   }

   return TRUE;
#undef DEL_ELE_PLIST
#undef ADD_ELE_PLIST
}	/* end - lpt_bld_plist() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lpt_getmsg                                                   *
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
lpt_getmsg(const char *catnm,int setno,int msgno)
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
}	/* end - lpt_getmsg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lpt_msg                                                      *
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
lpt_msg(FILE *outfp,int msgno,const char *defmsg,...)
{
   va_list		vap;
   const char		*msgp = lpt_getmsg(MF_PIOBE,MF_PIOBESETNO,msgno);

   /* Parse the message with the specified args. */
   if (!msgp)
      msgp = defmsg;
   va_start(vap,defmsg);
   (void)vfprintf(outfp,msgp,vap);
   va_end(vap);

   return;
}	/* end - lpt_msg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           lpt_parsemsg                                                 *
*                                                                              *
* DESCRIPTION:    Parse a given message and fetch it, if necessary.            *
*                                                                              *
* PARAMETERS:     mp		msg                                            *
*                                                                              *
* RETURN VALUES:  fmp 		fetched msg                                    *
*                                                                              *
*******************************************************************************/
static char *
lpt_parsemsg(const char *mp)
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
       !(sp = lpt_getmsg(mcnm,setno?setno:MF_PIOATTR1SETNO,msgno)))
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
}	/* end - lpt_parsemsg() */


