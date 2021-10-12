static char sccsid[] = "@(#)15  1.1  src/bos/usr/lib/pios/piocfapi.c, cmdpios, bos411, 9428A410j 12/8/93 18:35:15";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  piogetattrs(), pioputattrs()
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
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <setjmp.h>
#include <dirent.h>
#include <grp.h>
#include <nl_types.h>
#include "pioformat.h"

/* Default messages */
#define DEFMSG_LIM_BADATTR	"The attribute name %.2s is not valid\n"

/* Misc. macros */
#define WKBUFLEN		(2047)
#define MSGBUFLEN		(WKBUFLEN+1)
#define RDBUFSIZE		(WKBUFLEN+1)
#define STRGZARG(a)		#a
#define STRGZS(a)		STRGZARG(a)
#define MF_PIOBE		"piobe.cat"
#define TMPFL_TMPLT		"/tmp/pioputattrsXXXXXX"
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc/"
#define MF_PIOBESETNO		(8)
#define ENVBASEDIR		"PIOBASEDIR"
#define ENVVARDIR		"PIOVARDIR"
#define DDIDIR			"/ddi/"
#define CUSDIR			"/custom/"
#define ETCDIR			"/etc/"
#define QQDEVSEP		":"
#define MAXQNMLEN		20
#define MAXDNMLEN		20
#define WHSPCHRS		" \t"
#define ATTR_DEF_STATE		"zD"
#define ATTR_CUR_STATE		"zS"
#define ATTR_DEF_MSGCAT		"mD"
#define PIOCNVT			"/usr/sbin/piocnvt"
#define PIODIGEST		"piodigest"
#define MAXCFLINELEN		(1000)
#define PRINTQ_GID		(9)
#define PRINTQ_GNM		"printq"
#define CUSFSEPCHR		':'
#define CMNTCHR			'#'
#define WHSPCHRS		" \t"
#define ATVALFLDNO		5
#define PIOATTRCATFMT		"pioattr%c.cat"
#define PIOATTRCATSETNO		(1)
#if defined(MIN)
#undef MIN
#endif		/* MIN */
#define MIN(a,b)		((a)<(b)?(a):(b))
#define RETURNERR		return (piocfapi = FALSE, -1)

/* External declarations */
extern char		*mktemp(char *);	/* should be in sys hdr files */
extern int		mkstemp(char *);	/* should be in sys hdr files */
extern int              attrtab_init(char *);
extern int		piocfapi;	/* flag to include code for cf API */
extern unsigned char	shellchar[];
extern int		piogetstr(char *,char *,int,error_info_t *);

/* Global variable definitions */
char *hash_table;       /* address of the primary hash table */
char *mem_start;        /* address of the start of the odm data */
char *odm_table;        /* address of the odm table */
int objtot      = 0;    /* number of objects in odm table */
int piomode     = 0;    /* indicates the set of attr. values in use */
int piopgskip   = 0;    /* number of pages to skip (suppress printing of)  */
int piodatasent = FALSE;/* data bytes sent to printer yet? */
int statusfile  = 0;    /* indicates whether a statusfile exists */

/* Local function declarations */
static const char	*cfa_getmsg(const char *,int,int);
static void		cfa_error(int,const char *,...);
static void		cfa_catch_signals(int);
static const char	*cfa_btoa(const char *,int);
static const char	*cfa_getatrfld(FILE *,const char *,uchar_t);

/* Local variable definitions */
static char		*mbufp;		/* msg buffer pointer */
static sigjmp_buf	jenv;



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           piogetattrs                                                  *
*                                                                              *
* DESCRIPTION:    The piogetattrs() subroutine retrieves attribute information *
*		  like attribute values, limits and descriptions from a	       *
*		  virtual printer attribute database (custom colon file and    *
*		  digested ddi file).  The subroutine resolves the attribute   *
*		  values and limits after fetching them from the ddi file.     *
*		  For attribute descriptions, it extracts the message catalog  *
*		  information from the custom colon file and uses it for       *
*		  building the descriptions.
*                                                                              *
* PARAMETERS:     qnm		queue name				       *
*		  qdnm		queue device name			       *
*		  aeno		no of attribute elements		       *
*		  aet		attribute element table			       *
*                                                                              *
* RETURN VALUES:  < 0		error                                          *
*                 >= 0		number of extracted attributes                 *
*                                                                              *
*******************************************************************************/
int
piogetattrs(const char *qnm,const char *qdnm,ushort_t aeno,pioattr_t *aet)
{
#define CLEAN_UP		do { \
				   for (ap = attr_table; \
					ap < attr_table+objtot; ap++) \
      				      free((void *)ap->ptypes.sinfo); \
   				   free((void *)attr_table); \
   				   free((void *)mem_start); \
				} while (0)
   register int			i;
   register pioattr_t		*aep;
   register int			srano		=	0;	
   char				ddiflg		=	FALSE;
   char				customflg	=	FALSE;
   char				bdir[PATH_MAX+1];
   char				vdir[PATH_MAX+1];
   char				fl[PATH_MAX+1];
   char				fqnm[MAXQNMLEN+MAXDNMLEN+1+1];	/* 1 for ':' */
   DIR				*Dp;
   register struct dirent	*dp;
   register const char		*ccp;
   register char		*cp;
   register struct attr_info	*ap;
   FILE				*fp;

   /* Initialize the values. */
   for (piocfapi = TRUE, aep = aet; aep < aet+aeno; aep++)
      aep->pa_value = NULL, aep->pa_retcode = FALSE;

   /* Determine if ddi or custom files need be loaded. */
   for (aep = aet; aep < aet+aeno && !(ddiflg && customflg); aep++)
      switch (aep->pa_type)
      {
	 case PA_AVALT:
	 case PA_ALIMT:
	    ddiflg++; break;
	 case PA_ADSCT:
	    customflg++; break;
      }
   
   /* Load ddi file and extract resolved values. */
   if (ddiflg)
   {
      (void)strncpy(bdir,
		    (cp = getenv(ENVBASEDIR))?cp:DEFBASEDIR,sizeof(bdir)-1),
      *(bdir+sizeof(bdir)-1) = 0;
      (void)strncpy(vdir,(ccp = getenv(ENVVARDIR))?ccp:cp?cp:DEFVARDIR,
		    sizeof(vdir)-1),
      *(vdir+sizeof(vdir)-1) = 0;
      (void)strncpy(fqnm,qnm,MAXQNMLEN), *(fqnm+MAXQNMLEN) = 0;
      (void)strcat(fqnm,QQDEVSEP);
      cp = fqnm+strlen(fqnm);
      (void)strncpy(cp,qdnm,MAXDNMLEN), *(cp+MAXDNMLEN) = 0;
      (void)strncpy(fl,vdir,sizeof(fl)-strlen(DDIDIR)-1),
      *(fl+sizeof(fl)-strlen(DDIDIR)-1) = 0;
      (void)strcat(fl,DDIDIR);
      if (!(Dp = opendir(fl)))
      {
	 /* For now, no error message. */
	 RETURNERR;
      }
      while (dp = readdir(Dp))
         if (strlen(dp->d_name) > strlen(fqnm)  &&
	     !strcmp(fqnm,dp->d_name+strlen(dp->d_name)-strlen(fqnm)))
	    break;
      if ((void)closedir(Dp), !dp)
      {
	 /* For now, no error message. */
	 RETURNERR;
      }
      *(fl+sizeof(fl)-1) = 0;
      (void)strncat(fl,dp->d_name,sizeof(fl)-strlen(fl)-1);
      if (attrtab_init(fl) < 0)
      {
	 /* For now, no error message. */
	 RETURNERR;
      }
      for (aep = aet; aep < aet+aeno; aep++)
      {
	 char				val[5*(WKBUFLEN+1)];

	 if (aep->pa_type != PA_AVALT && aep->pa_type != PA_ALIMT ||
	     !get_odmtab_ent(aep->pa_name))
	    continue;
	 if (aep->pa_type == PA_AVALT)		/* attribute value */
	 {
	    if ((i = piogetstr(aep->pa_name,val,(int)(sizeof val),NULL)) < 0)
	       continue;
	 }
	 else					/* attribute limits */
	 {
	    char			tanm[sizeof(aep->pa_name)+1] = "#";

	    (void)memcpy((void *)(tanm+1),(void *)aep->pa_name,sizeof(tanm)-1);
	    if ((i = piogetstr(tanm,val,(int)(sizeof val),NULL)) < 0)
	       continue;
	 }
	 if (!(aep->pa_value = malloc((size_t)(i+1))))
	 {
	    /* For now, no error message. */
	    continue;
	 }
	 (void)memcpy(aep->pa_value,(void *)val,(size_t)i),
	 *((char *)aep->pa_value+i) = 0;
	 aep->pa_vallen = i, aep->pa_retcode = TRUE, srano++;
      }
      CLEAN_UP;
   }

   /* Load custom colon file and extract attribute messages. */
   if (customflg)
   {
      const char		*defmc;
      char			s[MAXCFLINELEN+1];

      /* Open the custom colon file. */
      (void)strncpy(fl,vdir,sizeof(fl)-strlen(CUSDIR)-1),
      *(fl+sizeof(fl)-strlen(CUSDIR)-1) = 0;
      (void)strcat(fl,CUSDIR);
      (void)strncat(fl,qnm,MIN((size_t)MAXQNMLEN,
		    sizeof(fl)-strlen(fl)));
      (void)strncat(fl,QQDEVSEP,sizeof(fl)-strlen(fl));
      (void)strncat(fl,qdnm,MIN((size_t)MAXDNMLEN,
		    sizeof(fl)-strlen(fl)));
      if (!(fp = fopen(fl,"r")))
      {
	 /* For now, no error message. */
	 RETURNERR;
      }
      /* Determine the default message catalog. */
      if (!(defmc = cfa_getatrfld(fp,ATTR_DEF_MSGCAT,(uchar_t)ATVALFLDNO)))
      {
	 /* For now, no error message. */
	 (void)fclose(fp);
	 RETURNERR;
      }
      /* Fetch descriptions for each attribute specified. */
      while (fgets(s,sizeof s,fp))
      {
	 register const char		*ccp1;

	 if (cp = strchr(s,'\n'))
	    *cp = 0;
	 if (!*s || (cp = s+strspn(s,WHSPCHRS), !*cp) ||
	     *cp == CMNTCHR || !(ccp = strchr(cp,CUSFSEPCHR)) ||
	     !(++ccp, ccp1 = strchr(ccp,CUSFSEPCHR)))
	    continue;
	 for (++ccp1, i = FALSE, aep = aet; aep < aet+aeno; aep++)
	 {
	    if (aep->pa_type != PA_ADSCT)
	       continue;
	    if (!strncmp(aep->pa_name,ccp1,sizeof(aep->pa_name)))
	    {
	       if (!i)		/* if info has not already been obtained */
	       {
		  int			msgno;
		  char			mcnm[FILENAME_MAX+1];
		  const char		*mcp;
		  size_t		tlen;

		  mcp = *cp == CUSFSEPCHR ? defmc : cp+1 == ccp-1 ?
			((void)sprintf(mcnm,PIOATTRCATFMT,*cp), mcnm) :
			((void)strncpy(mcnm,cp,
				       tlen = MIN(sizeof(mcnm)-1,(ccp-cp)-1)),
			 *(mcnm+tlen) = 0, mcnm);
		  msgno = strtol(ccp,NULL,10);
		  ccp = cfa_getmsg(mcp,PIOATTRCATSETNO,msgno);
		  i++;
	       }
	       if (ccp && (aep->pa_value = malloc(strlen(ccp)+1)))
		  (void)strcpy(aep->pa_value,ccp),
		  aep->pa_vallen = strlen(ccp),
		  aep->pa_retcode = TRUE,
		  srano++;
	    }
	 }
      }
      if (ferror(fp))
      {
	 /* For now, no error message. */
	 (void)fclose(fp);
	 RETURNERR;
      }
      (void)fclose(fp);
   }

   return (piocfapi = FALSE, srano);
#undef CLEAN_UP
}	/* end - piogetattrs() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           pioputattrs                                                  *
*                                                                              *
* DESCRIPTION:    The pioputattrs() subroutine updates attribute values        *
*		  in the custom colon file for a specified queue and device.   *
*		  Initially, the subroutine finds the custom colon file.       *
*		  Then, it expands it into a temp file using the command       *
*		  'piocnvt'.  Then, it opens another temp file, and starts     *
*		  writing attribute information to it, while reading each      *
*		  attribute line from the first temp file, and changing the    *
*		  value, if specified in the attribute element list.           *
*		  After it is through writing updated information to the       *
*		  second temp file, it digests it using the command 'piodigest'*
*		  and replaces the original custom file with this temp file,   *
*		  after contracting it back with the command 'piocnvt'.	       *
*		  Note: While changing the attribute info, if a given attribute*
*		  is either "zD" or "zS", it will skip the change.             *
*                                                                              *
* PARAMETERS:     qnm		queue name				       *
*		  qdnm		queue device name			       *
*		  aeno		no of attribute elements		       *
*		  aet		attribute element table			       *
*                                                                              *
* RETURN VALUES:  < 0		error                                          *
*                 >= 0		number of updated attributes                   *
*                                                                              *
*******************************************************************************/
int
pioputattrs(const char *qnm,const char *qdnm,ushort_t aeno,pioattr_t *aet)
{
#define SIG_TMP_CLEANUP		do { \
				   (void)sigaction(SIGHUP,&oldshup,NULL); \
				   (void)sigaction(SIGINT,&oldsint,NULL); \
				   (void)sigaction(SIGQUIT,&oldsquit,NULL); \
				   (void)sigaction(SIGTERM,&oldsterm,NULL); \
				   if (tmpifp) \
				      (void)fclose(tmpifp); \
				   if (tmpofp) \
				      (void)fclose(tmpofp); \
				   else if (tmpofd != -1) \
				      (void)close(tmpofd); \
				   (void)unlink(tmpifl); \
				   (void)unlink(tmpofl); \
				} while (0)
   register int			i;
   register pioattr_t		*aep;
   register int			srano		=	0;	
   char				bdir[PATH_MAX+1];
   char				vdir[PATH_MAX+1];
   char				cusfl[PATH_MAX+1];
   struct group			*grpp		=	getgrnam(PRINTQ_GNM);
   gid_t			pqgid		=	grpp?
							grpp->gr_gid:PRINTQ_GID;
   char				tmpifl[]	= 	TMPFL_TMPLT;
   char				tmpofl[]	= 	TMPFL_TMPLT;
   int				tmpofd		=	-1;
   FILE				*tmpifp		=	NULL;
   FILE				*tmpofp		=	NULL;
   register const char		*ccp;
   register char		*cp;
   struct sigaction		newsigs;
   struct sigaction		oldshup;
   struct sigaction		oldsint;
   struct sigaction		oldsquit;
   struct sigaction		oldsterm;
   struct flock			lck;
   char				cmd[WKBUFLEN+1];
   char				s[MAXCFLINELEN+1];
   char const 			*prohibit_attrs[] = {
						       ATTR_DEF_STATE,
						       ATTR_CUR_STATE,
						       NULL
						    };
   register const char		**tmp_prohibit_attp;

   /* To invoke this API function, the user needs to be super-user or should
      belong to 'printq' group. */
   if (geteuid() && getegid() != pqgid)
   {
      return -1;
   }

   /* Initialize the values. */
   if (!aeno)
      return 0;
   for (piocfapi = TRUE, aep = aet; aep < aet+aeno; aep++)
      aep->pa_retcode = FALSE;

   /* Check for the custom colon file. */
   (void)strncpy(bdir,(cp = getenv(ENVBASEDIR))?cp:DEFBASEDIR,sizeof(bdir)-1),
   *(bdir+sizeof(bdir)-1) = 0;
   (void)strncpy(vdir,(ccp = getenv(ENVVARDIR))?ccp:cp?cp:DEFVARDIR,
		 sizeof(vdir)-1),
   *(vdir+sizeof(vdir)-1) = 0;
   (void)strncpy(cusfl,vdir,sizeof(cusfl)-strlen(CUSDIR)-1),
   *(cusfl+sizeof(cusfl)-strlen(CUSDIR)-1) = 0;
   (void)strcat(cusfl,CUSDIR);
   (void)strncat(cusfl,qnm,MIN((size_t)MAXQNMLEN,sizeof(cusfl)-strlen(cusfl)));
   (void)strncat(cusfl,QQDEVSEP,sizeof(cusfl)-strlen(cusfl));
   (void)strncat(cusfl,qdnm,MIN((size_t)MAXDNMLEN,sizeof(cusfl)-strlen(cusfl)));
   if ((i = open(cusfl,O_RDONLY)) == -1)
   {
      RETURNERR;
   }
   lck.l_whence = lck.l_start = lck.l_len = 0;
   lck.l_type = F_RDLCK;
   if (fcntl(i,F_GETLK,&lck) == -1 || ((void)close(i), lck.l_type != F_UNLCK))
   {
      RETURNERR;
   }

   /* Set up temp. files and signal handlers to clean up the temp files upon
      receipt of a signal. */
   if (sigsetjmp(jenv,1))
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }
   newsigs.sa_handler = cfa_catch_signals;
   (void)sigemptyset(&newsigs.sa_mask);
   newsigs.sa_flags = 0;
   (void)sigaction(SIGHUP,&newsigs,&oldshup);
   (void)sigaction(SIGINT,&newsigs,&oldsint);
   (void)sigaction(SIGQUIT,&newsigs,&oldsquit);
   (void)sigaction(SIGTERM,&newsigs,&oldsterm);
   if (!mktemp(tmpifl) || (tmpofd = mkstemp(tmpofl)) == -1)
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }

   /* Expand the custom colon file into a temp file, and read this file
      to update attributes into another temp file. */
   if ((void)sprintf(cmd,"%s -i %s -s + -o %s >/dev/null 2>&1",PIOCNVT,cusfl,
		     tmpifl), system(cmd))
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }
   if (!(tmpifp = fopen(tmpifl,"r")) || !(tmpofp = fdopen(tmpofd,"w")))
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }
   while (fgets(s,sizeof s,tmpifp))
   {
      char				s1[sizeof s];
      char const			*ccp1;
      char				*cp1;
      size_t				tlen;
      size_t				tlen1;

      if (cp = strchr(s,'\n'))
	 *cp = 0;
      if (!*s || (ccp = s+strspn(s,WHSPCHRS), !*ccp) ||
	  *ccp == CMNTCHR || !(ccp = strchr(ccp,CUSFSEPCHR)) ||
	  !(ccp = strchr(ccp+1,CUSFSEPCHR)) ||
	  !(++ccp, ccp1 = strchr(ccp,CUSFSEPCHR)) ||
	  !(++ccp1, cp = strchr(ccp1,CUSFSEPCHR)))
      {
	 (void)fputs(s,tmpofp), (void)putc('\n',tmpofp);
	 continue;
      }
      for (i = FALSE, tmp_prohibit_attp = prohibit_attrs;
	   *tmp_prohibit_attp; tmp_prohibit_attp++)
	 if (!strncmp(*tmp_prohibit_attp,ccp,(size_t)((ccp1-1)-ccp)))
	 {
	    i = TRUE;
	    break;
	 }
      if (i)
      {
	 (void)fputs(s,tmpofp), (void)putc('\n',tmpofp);
	 continue;
      }
		/* copy the attribute line upto the value for now */
      ++cp;
      (void)memcpy((void *)s1,(void *)s,tlen = cp-s);
      for (i = FALSE, aep = aet+aeno-1; aep >= aet; aep--)
      {
	 if (!strncmp(aep->pa_name,ccp,
		      MIN((size_t)((ccp1-1)-ccp),sizeof(aep->pa_name))) &&
	     aep->pa_type == PA_AVALT)
	 {
	    i = TRUE;
	    break;
	 }
      }
      if (i)
	 ccp = cfa_btoa(aep->pa_value,(int)aep->pa_vallen),
	 (void)memcpy((void *)(cp1 = s1+tlen),ccp,
			 tlen1 = MIN(strlen(ccp),sizeof(s1)-tlen-1)),
	 srano++, aep->pa_retcode = TRUE;
      else
	 (void)memcpy((void *)(cp1 = s1+tlen),cp,tlen1 = sizeof(s1)-tlen-1);
      *(cp1+tlen1) = 0;
      (void)fputs(s1,tmpofp), (void)putc('\n',tmpofp);
   }
   if (ferror(tmpifp) || ferror(tmpofp))
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }
   (void)fflush(tmpofp);

   /* Digest the updated temp file.  If successful, restore it back to the
      default state and replace the original custom colon file with it. */
   if ((void)sprintf(cmd,"%s%s%s %s >/dev/null 2>&1",bdir,ETCDIR,PIODIGEST,
		     tmpofl), system(cmd))
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }
   if ((void)sprintf(cmd,"%s -i %s -o %s >/dev/null 2>&1",PIOCNVT,tmpofl,cusfl),
       system(cmd))
   {
      SIG_TMP_CLEANUP;
      RETURNERR;
   }

   /* Clean up and return. */
   SIG_TMP_CLEANUP;
   return (piocfapi = FALSE, srano);
#undef SIG_TMP_CLEANUP
}	/* end - pioputattrs() */



/*
*******************************************************************************
** NAME:        cfa_btoa
**
** DESCRIPTION: converts binary string to ASCII string.  If input byte is not
**              an ASCII character, or the character is special to the shell,
**              octal notation (i.e., \xxx) is used.
**
** CALLING
**  SEQUENCE:   btoa(const char *bin_str,int str_len);
**              char *bin_str;
**              int str_len;
**
** RETURN:      pointer to constructed ASCII string
**
*******************************************************************************
*/

char const *
cfa_btoa(const char *bin_str,int str_len)
{
   int 			ch, cnt;
   char 		*wkptr;
   static char		obuf[4*MAXCFLINELEN+1];

   for (cnt = 0, wkptr = obuf; cnt < str_len; cnt++)
   {
    ch = (int) *(bin_str + cnt);
    if (ch > ' ' && ch <= '~' && !shellchar[ch])
    {
	if (ch >= '0' && ch <= '7')
	{
	    if (wkptr >= obuf + 2 && *(wkptr - 2) == '\\')
	    {
		*wkptr = *(wkptr - 1);
		*(wkptr - 1) = '0';
		wkptr++;
	    }
	    if (wkptr >= obuf + 3 && *(wkptr - 3) == '\\')
	    {
		*wkptr = *(wkptr - 1);
		*(wkptr - 1) = *(wkptr - 2);
		*(wkptr - 2) = '0';
		wkptr++;
	    }
	}
	*wkptr++ = (char) ch;
    }
    else if (ch == ' ')
	*wkptr++ = ch;
    else
	wkptr += sprintf(wkptr, "\\%o", ch);
   }
   *wkptr = '\0';
   return (obuf);
}	/* end - cfa_btoa() */




/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           cfa_getmsg                                                   *
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
cfa_getmsg(const char *catnm, int setno, int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

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
      (void) strncat(defmcpath, catnm, sizeof(defmcpath)-strlen(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      if ((md = catopen(defmcpath, NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md, setno, msgno, (char *)dmsg),
		 (char const *)dmsg) ? mp : NULL;
}	/* end - cfa_getmsg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           cfa_error                                                    *
*                                                                              *
* DESCRIPTION:    Fetch an error message and put it in a msg buffer.           *
*                                                                              *
* PARAMETERS:     msgno		message no.                                    *
*                 defmsg	default error message                          *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static void
cfa_error(int msgno, const char *defmsg, ...)
{
   static FILE		*tmpfp;
   static size_t	memacqd;
   static size_t	memused;
   va_list		vap;
   const char		*msgp;
   ushort_t		parseerr = FALSE;
   int			msgcnt;

   if (!mbufp)
      memacqd = memused = 0;

   if (!(msgp = cfa_getmsg(MF_PIOBE, MF_PIOBESETNO, msgno)))
      msgp = defmsg;

   if (!tmpfp)
      tmpfp = tmpfile();
   else
      rewind(tmpfp);

   if (tmpfp)
   {
      va_start(vap, defmsg);
      if ((msgcnt = vfprintf(tmpfp, msgp, vap)) < 0)
	 parseerr = TRUE;
      va_end(vap);
   }
   else
      parseerr = TRUE;

   if (parseerr)
      msgcnt = strlen(msgp);

   while (memacqd < memused+msgcnt+1)
   {
      /*
      REALLOC(mbufp, mbufp, memacqd+MSGBUFLEN);
       */
      if (!(mbufp = realloc(mbufp,memacqd+MSGBUFLEN)))
	 return;
      memacqd += MSGBUFLEN;
   }

   if (!parseerr)
   {
      register size_t		trdcnt;
      register const char	*cp;
      size_t			nread;

      for ((void) rewind(tmpfp), trdcnt = (size_t)msgcnt, cp = mbufp+memused;
	   trdcnt > 0;
	   cp += nread, trdcnt -= nread)
      {
         nread = trdcnt < RDBUFSIZE ? trdcnt : RDBUFSIZE;
         if (fread((void *)cp, nread, (size_t)1, tmpfp) != (size_t)1  ||
	     ferror(tmpfp))
	 {
	    parseerr = TRUE;
	    break;
	 }
      }
   }
   else
      (void) memcpy((void *)(mbufp+memused), (void *)msgp, (size_t)msgcnt);
   *(mbufp+memused+msgcnt) = 0;

   memused += msgcnt;

   return;
}	/* end - cfa_error() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           cfa_getatrfld                                                *
*                                                                              *
* DESCRIPTION:    Signal handler function.                                     *
*                                                                              *
* PARAMETERS:     signo		signal no.                                     *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static const char *
cfa_getatrfld(FILE *fp,const char *anm,register uchar_t fldno)
{
   static char			s[MAXCFLINELEN+1];
   int				atfnd		=	FALSE;
   register char		*cp;
   register char const		*ccp;
   register char		*cp1;

   for ((void)rewind(fp); fgets(s,sizeof s,fp); )
   {
      if (cp = strchr(s,'\n'))
	 *cp = 0;
      if (*s && (ccp = s+strspn(s,WHSPCHRS), *ccp) &&
	  *ccp != CMNTCHR && (ccp = strchr(ccp,CUSFSEPCHR)) &&
	  (ccp = strchr(ccp+1,CUSFSEPCHR)) &&
	  (++ccp, !strncmp(anm,ccp,strlen(anm))))
      {
	 for (atfnd = TRUE, cp = s;
	      fldno-- > 1 && (cp = strchr(cp,CUSFSEPCHR)); )
	    ++cp;
	 if (cp && (cp1 = strchr(cp,CUSFSEPCHR)))
	    *cp1 = 0;
         break;
      }
   }

   return ferror(fp)?NULL:((void)rewind(fp),atfnd?cp:NULL);
}	/* end - cfa_getatrfld() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           cfa_catch_signals                                            *
*                                                                              *
* DESCRIPTION:    Signal handler function.                                     *
*                                                                              *
* PARAMETERS:     signo		signal no.                                     *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static void
cfa_catch_signals(int signo)		/*NOTUSED*/ /* arg not used - lint */
{
   siglongjmp(jenv,1);
}	/* end - cfa_catch_signals() */
