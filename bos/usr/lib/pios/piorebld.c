static char sccsid[] = "@(#)78  1.6  src/bos/usr/lib/pios/piorebld.c, cmdpios, bos411, 9428A410j 5/10/94 16:17:24";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  piorebuild()
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
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <odmi.h>
#include <sys/utsname.h>
#include <errno.h>
#include "pioformat.h"
#include "piolimits.h"
#include "smit_class.h"


/* External function declarations */
extern int			gethostname(char *,int);

/* Local function declarations */
static struct sm_cmd_opt	*rbld_parse_populate(const attrothinfo_t *,int,
						     int *,char **);
static void 			rbld_catch_sigs(int);
static const char		*rbld_getmsg(const char *,int,int);

/* Local variable definitions */
static const char		*gqnm;		/* queue name */
static const char		*gvnm;		/* queue device name */
static char			defmc[PATH_MAX+1]; /* default message catalog */
static CLASS_SYMBOL		odmschd;	/* ODM sm_cmd_hdr id */
static CLASS_SYMBOL		odmscod;	/* ODM sm_cmd_opt id */
static char			wbuf[WKBUFLEN+1]; /* work buf */
static jmp_buf			fjmpenv;	/* for jumping */
static sigjmp_buf		jmpenv;		/* for jumping */
static int			gsigno;		/* signal no */

#define RBLD_ODM_CLEANUP	\
   do \
   { \
      (void)sprintf(wbuf,ODM_QPRT_SCHALL_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmschd,wbuf); \
      (void)sprintf(wbuf,ODM_SETUP_SCH_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmschd,wbuf); \
      (void)sprintf(wbuf,ODM_JOBATTRS_SCH_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmschd,wbuf); \
      (void)sprintf(wbuf,ODM_FILTERS_SCH_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmschd,wbuf); \
      (void)sprintf(wbuf,ODM_FLG_SCO_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmscod,wbuf); \
      (void)sprintf(wbuf,ODM_SYS_SCO_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmscod,wbuf); \
      (void)sprintf(wbuf,ODM_PFL_SCO_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmscod,wbuf); \
      (void)sprintf(wbuf,ODM_FIL_SCO_CRIT,gqnm,gvnm); \
      (void)odm_rm_obj(odmscod,wbuf); \
   } while (0)
#define RBLD_ODM_CLOSE		\
   do \
   { \
      (void)odm_close_class(odmscod); \
      (void)odm_close_class(odmschd); \
      (void)odm_unlock(odmld); \
      free((void *)odmsp); \
      (void)odm_terminate(); \
   } while (0)
/* Instead of exiting, just return from the piorebuild()
   routine so that 'piodigest' can proceed to build a ddi file. */
#define RBLD_ERRRETURN(ocu,et,mno,sv1,sv2,iv)	\
   do \
   { \
      if (ocu)			/* if ODM clean up to be done */ \
      { \
	 RBLD_ODM_CLEANUP; \
	 RBLD_ODM_CLOSE; \
      } \
      ERRDISP(et,mno,sv1,sv2,iv); \
      ERRDISP(0,MSG_RBLD_CONTMSG,NULL,NULL,0); \
      longjmp(fjmpenv,1); \
   } while (0)
#define RBLD_ERREXIT(ocu,et,mno,sv1,sv2,iv)	\
   do \
   { \
      if (ocu)			/* if ODM clean up to be done */ \
      { \
	 RBLD_ODM_CLEANUP; \
	 RBLD_ODM_CLOSE; \
      } \
      ERREXIT(et,mno,sv1,sv2,iv); \
   } while (0)


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           piorebuild()                                                 *
*                                                                              *
* DESCRIPTION:    Rebuilds ODM objects for SMIT dialogs 'Print a File Dialog', *
*                 'Printer Setup Dialog', 'Default Job Attributes Dialog',     *
*                 'Pre-processing Filters Dialog'                              *
*                                                                              *
* PARAMETERS:     qnm		queue name                                     *
*                 vnm		queue device name                              *
*                 cfnm		colon file name                                *
*                 ono		no of objects                                  *
*                 aoip		pointer to non-ddi attribute info records      *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
void
piorebuild(const char *qnm,const char *vnm,char const *cfnm,unsigned int ono,
	   const attrothinfo_t *aoip)
{
   register char		*cp;
   register const char		*ccp;
   char				sdir[PATH_MAX+1];	/* SMIT ODM dir */
   char				s1dir[PATH_MAX+1];	/* SMIT ODM dir */
   char				vdir[PATH_MAX+1];	/* var dir */
   char				*odmsp	= NULL;		/* ODM set path id */
   int				odmld	= -1;		/* ODM lock desc. */
   dsinfo_t			*dip;			/* datastream info */
   register dsinfo_t		*tdip;			/* tmp ds info */
   uint_t			nodi;			/* no of datastreams */
   register const attrothinfo_t	*taoip;			/* tmp atr other info */
   register int			i;			/* scrach counter */
   register int			j;			/* scrach counter */
   struct sm_cmd_hdr		odmsch;			/* dialog hdr object */
   struct sm_cmd_opt		*odmscop;		/* dialog opt object */
   int				dspflg;			/* display or not */
   register char 		(*trasp)[ATTRNAMELEN];	/* tmp ref attrs ptr */
   char				allats[MAXNOATSINASCH][ATTRNAMELEN]; /*atlist */
   uint_t			noallats;		/* no of ats in list */
   char				*cmpflgstr;		/* str of cmp flags */
   char				soi[WKBUFLEN+1];	/* option_id (65 ???) */
   char				sctd[WKBUFLEN+1];	/* cmd_to_discover */
   struct sigaction		newsigh;		/* new sig handler */
   struct sigaction		osighup;		/* old sig handler */
   struct sigaction		osigint;		/* old sig handler */
   struct sigaction		osigterm;		/* old sig handler */
   size_t			len;			/* scrach var */
   uchar_t			sigsset	= 0;		/* signals set? */

   /* Set up an escape hatch for error handling. */
   if (setjmp(fjmpenv))
   {
      if (sigsset)
         (void)sigaction(SIGHUP,&osighup,(struct sigaction *)NULL),
         (void)sigaction(SIGINT,&osigint,(struct sigaction *)NULL),
         (void)sigaction(SIGTERM,&osigterm,(struct sigaction *)NULL);
      return;
   }

   /* Perform ODM initialization. */
   gqnm = qnm, gvnm = vnm;
   (void)strncpy(errinfo.subrname,"piorebuild",sizeof(errinfo.subrname)-1),
   *(errinfo.subrname+sizeof(errinfo.subrname)-1) = 0;
   (void)strncpy(errinfo.qname,qnm,sizeof(errinfo.qname)-1),
   *(errinfo.qname+sizeof(errinfo.qname)-1) = 0;
   (void)strncpy(errinfo.qdname,vnm,sizeof(errinfo.qdname)-1),
   *(errinfo.qdname+sizeof(errinfo.qdname)-1) = 0;
   if (odm_initialize() == -1)
      RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRODMINIT,NULL,NULL,odmerrno);
   if ((ccp = getenv("PIOVARDIR")) || (ccp = getenv("PIOBASEDIR")))
      (void)strncpy(sdir,ccp,sizeof(sdir)-1),
      *(sdir+sizeof(sdir)-1) = 0;
   else
      (void)strncpy(sdir,DEFVARDIR,sizeof(sdir)-1),
      *(sdir+sizeof(sdir)-1) = 0;
   (void)strncpy(s1dir,sdir,sizeof(s1dir)-1),
   *(s1dir+sizeof(s1dir)-1) = 0;
   if (s1dir[len = strlen(s1dir)-1] == '/')
      s1dir[len] = 0;
   if (cp = strrchr(s1dir,'/'))
   {
      char		hname[256+1];
      struct hostent	*hp;
      struct utsname	ust;

      ccp = gethostname(hname,(int)sizeof hname) != -1 ?
	    ((hp = gethostbyname(hname)) ? hp->h_name : hname) :
	    uname(&ust) != -1 ? ust.nodename : NULL;
      if (ccp)
	 ++cp, (void)strncpy(cp,ccp,s1dir+sizeof(s1dir)-cp-1);
   }
   (void)strncpy(vdir,s1dir,sizeof(vdir)-1),
   *(vdir+sizeof(vdir)-1) = 0;
   (void)strncat(sdir,SMITDIR,sizeof(sdir)-strlen(sdir));
   (void)strncat(s1dir,SMITDIR,sizeof(s1dir)-strlen(s1dir));
   if ((odmsp = odm_set_path(sdir)) == (char *)-1)
      RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRODMSP,sdir,NULL,odmerrno);
   if ((odmld = odm_lock(sdir,ODM_LOCKTIME)) == -1)
      RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRODMLOCK,sdir,NULL,odmerrno);
   if ((int)(odmschd = odm_open_class(sm_cmd_hdr_CLASS)) == -1)
      RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRODMOPEN,ODM_SCHSTR,NULL,odmerrno);
   if ((int)(odmscod = odm_open_class(sm_cmd_opt_CLASS)) == -1)
      RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRODMOPEN,ODM_SCOSTR,NULL,odmerrno);

   /* Remove existing ODM information (if any) for the given queue and
      queue device. */
   RBLD_ODM_CLEANUP;

   /* Determine lists of referenced flags for all the datastreams supported
      for this queue. Also find out the default message catalog. */
   if (piogetstr(DEFMCATTR,defmc,(int)(sizeof defmc),NULL) < 0)
      RBLD_ERRRETURN(FALSE,0,MSG_ATTRNAME,DEFMCATTR,NULL,0);
   for (nodi = 0, taoip = aoip; taoip < aoip+ono; taoip++)
      if (*taoip->anm == INPUTPIPEPREFIX)
	 nodi++;
   if (!nodi)
      RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRNOIP,cfnm,NULL,0);
   MALLOC(dip,nodi*sizeof(*dip));
   for (taoip = aoip, tdip = dip; taoip < aoip+ono; taoip++)
      if (*taoip->anm == INPUTPIPEPREFIX)
      {
	 tdip->dsop = *(taoip->anm+1);
	 if ((i = piogetrefs(taoip->anm,TRUE,&tdip->refats)) < 0)
            RBLD_ERRRETURN(FALSE,0,MSG_RBLD_ERRGRFS,cfnm,taoip->anm,0);
	 tdip->norefats = i;
	 tdip->noregflgs = tdip->nocmpflgs = 0;
	 tdip++;
      }

   /* Set up a signal handler for a few signals so that the partiallly
      built ODM information can be cleaned up (and will not be left dangling
      in the system). */
   if (sigsetjmp(jmpenv,1))
   {
      RBLD_ERREXIT(TRUE,0,MSG_RBLD_SIGRECVD,NULL,NULL,gsigno);
   }
   newsigh.sa_handler = rbld_catch_sigs;
   (void)sigemptyset(&newsigh.sa_mask);
   newsigh.sa_flags = 0;
   (void)sigaction(SIGHUP,&newsigh,&osighup);
   (void)sigaction(SIGINT,&newsigh,&osigint);
   (void)sigaction(SIGTERM,&newsigh,&osigterm);
   sigsset++;

   /* For each selected attribute in the section __FLG, add an object to
      sm_cmd_opt ODM.  If referenced, also add it to the sm_cmd_hdr option
      ID list for each datastream's 'Print A File Dialog' header. */
   for (noallats = 0, taoip = aoip; taoip < aoip+ono; taoip++)
      if (!strncmp(taoip->sctnm,FLGSCTNM,SECTNMLEN) &&
	  (*taoip->anm == REGFLGPREFIX || *taoip->anm == CMPFLGPREFIX))
      {
	 if (!(odmscop = rbld_parse_populate(taoip,TRUE,&dspflg,&cmpflgstr)))
            RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRPARSE,taoip->anm,cfnm,0);
	 if (dspflg)
	 {
	    if (odm_add_obj(odmscod,odmscop) == -1)
               RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCOSTR,NULL,odmerrno);
	    if (noallats < ARRAYSIZ(allats))
	       (void)memcpy((void *)allats[noallats++],(void *)taoip->anm,
			    (size_t)ATTRNAMELEN);
	    if (!memcmp((void *)taoip->anm,(void *)IN_DATASTREAM,
			(size_t)ATTRNAMELEN))
	       continue;
	    if (dspflg == ALWAYS)
	    {
	       if (*taoip->anm == REGFLGPREFIX)
	       {
	          for (tdip = dip; tdip < dip+nodi; tdip++)
		     if (tdip->noregflgs < ARRAYSIZ(tdip->regflgs))
			(void)memcpy((void *)tdip->regflgs[tdip->noregflgs++],
				     (void *)taoip->anm,(size_t)ATTRNAMELEN);
	       }
	       else
	       {
	          for (tdip = dip; tdip < dip+nodi; tdip++)
		     if (tdip->nocmpflgs < ARRAYSIZ(tdip->cmpflgs))
			(void)memcpy((void *)tdip->cmpflgs[tdip->nocmpflgs++],
				     (void *)taoip->anm,(size_t)ATTRNAMELEN);
	       }
	    }
	    else
	    {
	       if (*taoip->anm == REGFLGPREFIX)
	       {
	          for (tdip = dip; tdip < dip+nodi; tdip++)
	             for (trasp = tdip->refats;
			  trasp < tdip->refats+tdip->norefats; trasp++)
		        if (!memcmp((void *)taoip->anm,(void *)*trasp,
			            (size_t)ATTRNAMELEN))
			{
			   if (tdip->noregflgs < ARRAYSIZ(tdip->regflgs))
			      (void)memcpy(
				 (void *)tdip->regflgs[tdip->noregflgs++],
				 (void *)taoip->anm,(size_t)ATTRNAMELEN);
		           break;
		        }
	       }
	       else
	       {
		  if (cmpflgstr && *cmpflgstr)
		  {
		     char	cfas[MAXNOCFSPERATTR][ATTRNAMELEN];

		     for (i = 0, cp = strtok(cmpflgstr,",");
			  cp && i < ARRAYSIZ(cfas);
			  cp = strtok((char *)NULL,","))
			(void)strncpy(cfas[i++],cp,(size_t)ATTRNAMELEN);
	             for (tdip = dip; tdip < dip+nodi; tdip++)
		     {
		        uchar_t		cfnd 	= TRUE;

			for (j = 0; j < i; j++)
			{
			   uchar_t	efnd	= FALSE;

	                   for (trasp = tdip->refats;
			        trasp < tdip->refats+tdip->norefats; trasp++)
			      if (!memcmp((void *)cfas[j],(void *)*trasp,
					  (size_t)ATTRNAMELEN))
			      {
				 efnd = TRUE;
				 break;
			      }
			   if (!efnd)
			   {
			      cfnd = FALSE;
			      break;
			   }
			}
			if (cfnd)
			   (void)memcpy(
			      (void *)tdip->cmpflgs[tdip->nocmpflgs++],
			      (void *)taoip->anm,(size_t)ATTRNAMELEN);
		     }
		  }
		  if (cmpflgstr)
		     free((void *)cmpflgstr);
	       }
	    }
	 }
      }

   /* Build an ODM sm_cmd_hdr object for each datastream's 'Print A File
      Dialog'. */
   for (tdip = dip; tdip < dip+nodi; tdip++)
   {
      char			regflgs[MAXNOOFREGFLGS+1];
      char			cmpflgs[MAXNOOFCMPFLGS+1];
      char			scte[WKBUFLEN+1];	/* cmd_to_exec */

      (void)memset((void *)&odmsch,0,sizeof odmsch);
      (void)sprintf(odmsch.id,ODM_SCH_QPRT_IDFMT,qnm,vnm,tdip->dsop);
      for (i = 0; i < tdip->noregflgs && i < sizeof(regflgs)-1; i++)
	 regflgs[i] = *(tdip->regflgs[i]+1);
      regflgs[i] = 0;
      for (i = 0; i < tdip->nocmpflgs && i < sizeof(cmpflgs)-1; i++)
	 cmpflgs[i] = *(tdip->cmpflgs[i]+1);
      cmpflgs[i] = 0;
      (void)sprintf(soi,ODM_SCH_QPRT_OPIDFMT,s1dir,qnm,vnm,regflgs,s1dir,qnm,
		    vnm,cmpflgs);
      odmsch.option_id 		= soi;
      *odmsch.has_name_select 	= 'y';
      odmsch.name 		= ODM_SCH_QPRT_NAME;
      odmsch.name_msg_file 	= SMITMSGCAT;
      odmsch.name_msg_set 	= ODM_SCH_QPRT_MSGSET;
      odmsch.name_msg_id 	= ODM_SCH_QPRT_MSGID;
      (void)sprintf(scte,ODM_SCH_QPRT_CTEFMT,vdir,QPRT,tdip->dsop);
      odmsch.cmd_to_exec 	= scte;
      *odmsch.ask		= 'n';
      *odmsch.exec_mode		= 'p';
      *odmsch.ghost		= 'n';
      (void)sprintf(sctd,ODM_SCH_QPRT_CTDFMT,vdir,PIOEVATTR,qnm,vnm);
      ccp = sctd+sizeof(sctd), cp = sctd+strlen(sctd);
      for (i = 0; i < tdip->noregflgs && cp < ccp-(ATTRNAMELEN+1)-1;
	   i++, cp += ATTRNAMELEN+1)
	 (void)memcpy((void *)cp,(void *)tdip->regflgs[i],(size_t)ATTRNAMELEN),
	 *(cp+ATTRNAMELEN) = ' ';
      for (i = 0; i < tdip->nocmpflgs && cp < ccp-(ATTRNAMELEN+1)-1;
	   i++, cp += ATTRNAMELEN+1)
	 (void)memcpy((void *)cp,(void *)tdip->cmpflgs[i],(size_t)ATTRNAMELEN),
	 *(cp+ATTRNAMELEN) = ' ';
      *cp = 0;
      odmsch.cmd_to_discover	= sctd;
      odmsch.cmd_to_discover_postfix = NULLSTR;
      (void)strncpy(odmsch.help_msg_id,ODM_SCH_QPRT_HMI,
		    sizeof(odmsch.help_msg_id)-1),
      *(odmsch.help_msg_id+sizeof(odmsch.help_msg_id)-1) = 0;
      odmsch.help_msg_loc	=
      odmsch.help_msg_base	= odmsch.help_msg_book		= NULLSTR;
      if (odm_add_obj(odmschd,&odmsch) == -1)
         RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCHSTR,NULL,odmerrno);
   }

   /* Build an sm_cmd_hdr ODM object for 'Change / Show Default Print Job
      Attributes' dialog. */
   {
      (void)memset((void *)&odmsch,0,sizeof odmsch);
      (void)sprintf(odmsch.id,ODM_SCH_JOBATTRS_IDFMT,qnm,vnm);
      (void)sprintf(soi,ODM_SCH_JOBATTRS_OPIDFMT,s1dir,qnm,vnm);
      odmsch.option_id 		= soi;
      *odmsch.has_name_select 	= 'y';
      odmsch.name 		= ODM_SCH_JOBATTRS_NAME;
      odmsch.name_msg_file 	= PIOSMITMSGCAT;
      odmsch.name_msg_set 	= ODM_SCH_JOBATTRS_MSGSET;
      odmsch.name_msg_id 	= ODM_SCH_JOBATTRS_MSGID;
      odmsch.cmd_to_exec 	= PIOCHPQ;
      *odmsch.ask		= 'n';
      *odmsch.exec_mode		= 'p';
      *odmsch.ghost		= 'n';
      (void)sprintf(sctd,ODM_SCH_JOBATTRS_CTDFMT,PIOEVATTR,qnm,vnm);
      ccp = sctd+sizeof(sctd), cp = sctd+strlen(sctd);
      for (i = 0; i < noallats && cp < ccp-(ATTRNAMELEN+1)-1;
	   i++, cp += ATTRNAMELEN+1)
	 (void)memcpy((void *)cp,(void *)allats[i],(size_t)ATTRNAMELEN),
	 *(cp+ATTRNAMELEN) = ' ';
      *cp = 0;
      odmsch.cmd_to_discover	= sctd;
      odmsch.cmd_to_discover_postfix = NULLSTR;
      (void)strncpy(odmsch.help_msg_id,ODM_SCH_JOBATTRS_HMI,
		    sizeof(odmsch.help_msg_id)-1),
      *(odmsch.help_msg_id+sizeof(odmsch.help_msg_id)-1) = 0;
      odmsch.help_msg_loc	=
      odmsch.help_msg_base	= odmsch.help_msg_book		= NULLSTR;
      if (odm_add_obj(odmschd,&odmsch) == -1)
         RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCHSTR,NULL,odmerrno);
   }

   /* For each selected attribute in the sections __SYS and __PFL, add an
      object to sm_cmd_opt ODM.  Also add it to the sm_cmd_hdr option
      ID list for 'Change / Show Printer Setup' header. */
   for (noallats = 0, taoip = aoip; taoip < aoip+ono; taoip++)
      if (!strncmp(taoip->sctnm,SYSSCTNM,SECTNMLEN) ||
	  !strncmp(taoip->sctnm,PFLSCTNM,SECTNMLEN))
      {
	 if (!(odmscop = rbld_parse_populate(taoip,FALSE,&dspflg,&cmpflgstr)))
            RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRPARSE,taoip->anm,cfnm,0);
	 if (dspflg)
	 {
	    if (odm_add_obj(odmscod,odmscop) == -1)
               RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCOSTR,NULL,odmerrno);
	    if (noallats < ARRAYSIZ(allats))
	       (void)memcpy((void *)allats[noallats++],(void *)taoip->anm,
			    (size_t)ATTRNAMELEN);
	 }
	 if (cmpflgstr)
	    free((void *)cmpflgstr);
      }

   /* Build an ODM sm_cmd_hdr object for 'Change / Show Printer Setup'
      dialog. */
   {
      (void)memset((void *)&odmsch,0,sizeof odmsch);
      (void)sprintf(odmsch.id,ODM_SCH_SETUP_IDFMT,qnm,vnm);
      (void)sprintf(soi,ODM_SCH_SETUP_OPIDFMT,s1dir,qnm,vnm,s1dir,qnm,vnm);
      odmsch.option_id 		= soi;
      *odmsch.has_name_select 	= 'y';
      odmsch.name 		= ODM_SCH_SETUP_NAME;
      odmsch.name_msg_file 	= PIOSMITMSGCAT;
      odmsch.name_msg_set 	= ODM_SCH_SETUP_MSGSET;
      odmsch.name_msg_id 	= ODM_SCH_SETUP_MSGID;
      odmsch.cmd_to_exec 	= PIOCHPQ;
      *odmsch.ask		= 'n';
      *odmsch.exec_mode		= 'p';
      *odmsch.ghost		= 'n';
      (void)sprintf(sctd,ODM_SCH_SETUP_CTDFMT,PIOEVATTR,qnm,vnm);
      ccp = sctd+sizeof(sctd), cp = sctd+strlen(sctd);
      for (i = 0; i < noallats && cp < ccp-(ATTRNAMELEN+1)-1;
	   i++, cp += ATTRNAMELEN+1)
	 (void)memcpy((void *)cp,(void *)allats[i],(size_t)ATTRNAMELEN),
	 *(cp+ATTRNAMELEN) = ' ';
      *cp = 0;
      odmsch.cmd_to_discover	= sctd;
      odmsch.cmd_to_discover_postfix = NULLSTR;
      (void)strncpy(odmsch.help_msg_id,ODM_SCH_SETUP_HMI,
		    sizeof(odmsch.help_msg_id)-1),
      *(odmsch.help_msg_id+sizeof(odmsch.help_msg_id)-1) = 0;
      odmsch.help_msg_loc	=
      odmsch.help_msg_base	= odmsch.help_msg_book		= NULLSTR;
      if (odm_add_obj(odmschd,&odmsch) == -1)
         RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCHSTR,NULL,odmerrno);
   }

   /* For each selected attribute in the section __FIL, add an
      object to sm_cmd_opt ODM.  Also add it to the sm_cmd_hdr option
      ID list for 'Change / Show Pre-processing Filters' header. */
   for (noallats = 0, taoip = aoip; taoip < aoip+ono; taoip++)
      if (!strncmp(taoip->sctnm,FILSCTNM,SECTNMLEN))
      {
	 if (!(odmscop = rbld_parse_populate(taoip,FALSE,&dspflg,&cmpflgstr)))
            RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRPARSE,taoip->anm,cfnm,0);
	 if (dspflg)
	 {
	    if (odm_add_obj(odmscod,odmscop) == -1)
               RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCOSTR,NULL,odmerrno);
	    if (noallats < ARRAYSIZ(allats))
	       (void)memcpy((void *)allats[noallats++],(void *)taoip->anm,
			    (size_t)ATTRNAMELEN);
	 }
	 if (cmpflgstr)
	    free((void *)cmpflgstr);
      }

   /* Build an ODM sm_cmd_hdr object for 'Change / Show Pre-processing Filters'
      dialog. */
   {
      (void)memset((void *)&odmsch,0,sizeof odmsch);
      (void)sprintf(odmsch.id,ODM_SCH_FILTERS_IDFMT,qnm,vnm);
      (void)sprintf(soi,ODM_SCH_FILTERS_OPIDFMT,s1dir,qnm,vnm);
      odmsch.option_id 		= soi;
      *odmsch.has_name_select 	= 'y';
      odmsch.name 		= ODM_SCH_FILTERS_NAME;
      odmsch.name_msg_file 	= PIOSMITMSGCAT;
      odmsch.name_msg_set 	= ODM_SCH_FILTERS_MSGSET;
      odmsch.name_msg_id 	= ODM_SCH_FILTERS_MSGID;
      /* Changed to 'piochpq' as 'chvirprt' does not support multiple
	 '-a' flags yet.
      odmsch.cmd_to_exec 	= CHVIRPRT;
       */
      odmsch.cmd_to_exec 	= PIOCHPQ;
      *odmsch.ask		= 'n';
      *odmsch.exec_mode		= 'p';
      *odmsch.ghost		= 'n';
      (void)sprintf(sctd,ODM_SCH_FILTERS_CTDFMT,PIOEVATTR,qnm,vnm);
      ccp = sctd+sizeof(sctd), cp = sctd+strlen(sctd);
      for (i = 0; i < noallats && cp < ccp-(ATTRNAMELEN+1)-1;
	   i++, cp += ATTRNAMELEN+1)
	 (void)memcpy((void *)cp,(void *)allats[i],(size_t)ATTRNAMELEN),
	 *(cp+ATTRNAMELEN) = ' ';
      *cp = 0;
      odmsch.cmd_to_discover	= sctd;
      odmsch.cmd_to_discover_postfix = NULLSTR;
      (void)strncpy(odmsch.help_msg_id,ODM_SCH_FILTERS_HMI,
		    sizeof(odmsch.help_msg_id)-1),
      *(odmsch.help_msg_id+sizeof(odmsch.help_msg_id)-1) = 0;
      odmsch.help_msg_loc	=
      odmsch.help_msg_base	= odmsch.help_msg_book		= NULLSTR;
      if (odm_add_obj(odmschd,&odmsch) == -1)
         RBLD_ERRRETURN(TRUE,0,MSG_RBLD_ERRODMADD,ODM_SCHSTR,NULL,odmerrno);
   }

   /* Perform ODM cleanup.  Also restore the signal handlers now that ODM has
      been built completely. */
   (void)sigaction(SIGHUP,&osighup,(struct sigaction *)NULL);
   (void)sigaction(SIGINT,&osigint,(struct sigaction *)NULL);
   (void)sigaction(SIGTERM,&osigterm,(struct sigaction *)NULL);
   (void)odm_close_class(odmscod);
   (void)odm_close_class(odmschd);
   (void)odm_unlock(odmld);
   free((void *)odmsp);
   (void)odm_terminate();
}	/* end - piorebuild() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           rbld_parse_populate()                                        *
*                                                                              *
* DESCRIPTION:    Parses the limits field of a given attribute, and populates  *
*                 its ODM sm_cmd_opt data.                                     *
*                                                                              *
* PARAMETERS:     aoip		attribute other info                           *
*                 fflg		FLG section flag (TRUE/FALSE)                  *
*                 dflgp		whether to display or not (returned)           *
*                 cfsp		ptr to a string of composite flags             *
*                                                                              *
* RETURN VALUES:  scop		successful                                     *
*                 NULL		unsuccessful                                   *
*                                                                              *
*******************************************************************************/
static struct sm_cmd_opt *
rbld_parse_populate(const attrothinfo_t *aoip,int fflg,int *dflgp, char **cfsp)
{
#define FIND_SETENDCHAR		\
	    do \
	    { \
	       if (++ccp, !(ccp1 = strchr(++ccp,SETENDCHAR))) \
	       { \
	          ERRDISP(0,MSG_RBLD_INVLIMSYNTAX,ccp-2,aoip->anm,0); \
	          return (struct sm_cmd_opt *)NULL; \
	       } \
	    } \
	    while (0)
   static struct sm_cmd_opt	sco;
   struct odm 			*op;
   static char			sanm[ATTRNAMELEN+1];
   static char			snm[WKBUFLEN+1];
   static char			smc[PATH_MAX+1];
   static char			spfx[sizeof(PREFIXPREFIX)+ATTRNAMELEN+1];
						/* 1 for '=' */
   static char			sctl[WKBUFLEN+1];
   static char			sdv[2*(WKBUFLEN+1)];
   static char			svmf[WKBUFLEN+1];
   static char			sav[2*(WKBUFLEN+1)];
   static char			shml[WKBUFLEN+1];
   static char			shmb[WKBUFLEN+1];
   static char			shmB[WKBUFLEN+1];
   static ulong_t		sqno		= DEFMINSEQNO;
   uchar_t			Cflg 		= FALSE; /* comp. flag */
   char				wkbuf[10*(WKBUFLEN+1)];
   char				*limp;
   register char		*cp;
   register const char		*ccp;
   register const char		*ccp1;
   const char			*endp;
   size_t			len;


   /* Initialization. */
   *dflgp = TRUE; *cfsp = NULL;
   if (!(op = get_odmtab_ent(aoip->anm)))
   {
      ERRDISP(0,MSG_ATTRNAME,aoip->anm,NULL,0);
      return (struct sm_cmd_opt *)NULL;
   }
   (void)memset((void *)&sco,0,sizeof sco);
   (void)sprintf(sco.id,ODM_SCO_IDFMT,gqnm,gvnm,aoip->sctnm,aoip->anm);
   (void)sprintf(sco.id_seq_num,DEFSETSEQNO "%4lu",sqno++);
   (void)memcpy((void *)sanm,(void *)aoip->anm,(size_t)ATTRNAMELEN);
   sco.disc_field_name 		= sanm;
   if (*aoip->mcnm)
   {
      if (*(aoip->mcnm+1))
	 (void)strncpy(smc,aoip->mcnm,sizeof(smc)-1);
      else
	 (void)sprintf(smc,MCNMFMT,*aoip->mcnm);
   }
   else
      (void)strncpy(smc,defmc,sizeof(smc)-1);
   if (ccp = rbld_getmsg(
			 sco.name_msg_file            = smc,
			 sco.name_msg_set             = DEFMCSETNO,
			 sco.name_msg_id              = aoip->msgno
			))
      (void)strncpy(snm,ccp,sizeof(snm)-1);
   else
      (void)strncpy(snm,ODM_SCO_NMPREFIX,sizeof(snm)-1),
      (void)memcpy((void *)(cp = snm+strlen(snm)),(void *)aoip->anm,
		   len = MIN((size_t)ATTRNAMELEN,sizeof(snm)-strlen(snm)-1)),
      *(cp+len) = 0;
   sco.name 			= snm;
   *sco.op_type 		= 'n';
   *sco.entry_type 		= 't';
   *sco.required 		= 'n';
   if (fflg)		/* for __FLG section */
   {
      if (*aoip->anm == REGFLGPREFIX)
	 *spfx = '-',
	 *(spfx+1) = *(aoip->anm+1),
	 *(spfx+2) = ' ',
	 *(spfx+3) = 0;
      else
	 *spfx = 0;
   }
   else			/* other sections */
      (void)strcpy(spfx,PREFIXPREFIX),
      (void)strncat(spfx,aoip->anm,(size_t)ATTRNAMELEN),
      *(cp = spfx+strlen(PREFIXPREFIX)+ATTRNAMELEN) = '=',
      *(cp+1) = 0;
   sco.prefix 			= spfx;
   *sco.cmd_to_list_mode 	= 'a';
   sco.cmd_to_list = sco.cmd_to_list_postfix = sco.disp_values =
      sco.values_msg_file = sco.aix_values = sco.help_msg_loc = 
      sco.help_msg_base = sco.help_msg_book = NULLSTR;

   /* If limits field is null, return now. */
   if (!op->limlength)
   {
      if (fflg && *aoip->anm == CMPFLGPREFIX)
         *dflgp = (--sqno, FALSE);
      return &sco;
   }

   /* Parse the limits field and populate the ODM sm_cmd_opt. */
   if (memchr((void *)(limp = GET_ATTR_LIMSTR(op)),ESCCHAR,
              (size_t)op->limlength))
   {
      char			panm[ATTRNAMELEN+1] = "#";

      *(panm+1) = *aoip->anm,
      *(panm+2) = *(aoip->anm+1);
      if (piogetstr(panm,wkbuf,(int)(sizeof wkbuf),NULL) < 0)
      {
         ERRDISP(0,MSG_ATTRNAME,panm,NULL,0);
         return (struct sm_cmd_opt *)NULL;
      }
   }
   else
   {
      (void)memcpy((void *)wkbuf,(void *)limp,
		   len = MIN((size_t)op->limlength,sizeof(wkbuf)-1)),
      *(wkbuf+len) = 0;
   }
   limp = wkbuf;
   for (ccp = limp, endp = limp+strlen(limp); ccp < endp; )
   {
      switch (*ccp)
      {
	 case ENTRYOP:			/* E?	- entry type */
	    *sco.entry_type 		= *++ccp, ccp++;
	    break;

	 case MLISTOP:			/* M[.]	- multi-select list */
	    *sco.multi_select 		= 'm';		/* fall-through */

	 case LISTOP:			/* L[.]	- list */
	    *sco.op_type 		= 'l';
	    FIND_SETENDCHAR;
	    (void)memcpy((void *)sctl,(void *)ccp,
			 len = MIN((size_t)(ccp1-ccp),sizeof(sctl)-1)),
	    *(sctl+len) = 0;
	    sco.cmd_to_list 		= sctl;
	    ccp = ccp1+1;
	    break;

	 case MRINGOP:			/* T[.]	- multi-select ring */
	    *sco.multi_select 		= 'm';		/* fall-through */
	    break;

	 case RINGOP:			/* R[.]	- ring */
	    *sco.op_type 		= 'r';
	    {
	       char			*wbufp;
	       const char		*avls		= NULL;
	       const char		*dvls		= NULL;

	       FIND_SETENDCHAR;
	       MALLOC(wbufp,(size_t)((ccp1-ccp)+1));
	       (void)memcpy((void *)wbufp,(void *)ccp,(size_t)(ccp1-ccp)),
	       *(wbufp+(ccp1-ccp)) = 0;
   	       if (cp = strchr((void *)wbufp,RINGAIXVSEP))
		  *cp = 0,
		  avls = cp+1;
      	       if (cp = strchr((void *)wbufp,RINGMSGSEP))
      	       {
		  dvls = cp+1;
	 	  if (cp != wbufp)
	 	  {
	    	     register const char	*sp;
	    	     const char			*tp;

		     *(svmf+sizeof(svmf)-1) = 0;
		     (void)strncpy(svmf,RING_MC,sizeof(svmf)-1);
		     sco.values_msg_file 	= svmf;
		     sco.values_msg_set 	= RING_MCSETNO;
	    	     do
	    	     {
	       		/* Get msg no. */
	       		for (sp = cp-1; *sp != RINGLISTSEP; )
		  	   if (sp == wbufp)
		     	      break;
		  	   else
		     	      sp--;
	       		if (sco.values_msg_id = 
			    (int)strtol(tp = *sp == RINGLISTSEP ? sp+1 : sp,
					(char **)&tp,10), *tp != RINGMSGSEP)
	       		{
	          	   ERRDISP(0,MSG_RBLD_INVLIMSYNTAX,ccp,aoip->anm,0);
	          	   return (struct sm_cmd_opt *)NULL;
	       		}
	       		if (*sp != RINGLISTSEP  ||  sp == limp)
		  	   break;

	       		/* Get set no. */
	       		for (--sp; *sp != RINGLISTSEP; )
		  	   if (sp == limp)
		     	      break;
		  	   else
		     	      sp--;
	       		if (sco.values_msg_set = 
			    (int)strtol(tp = *sp == RINGLISTSEP ? sp+1 : sp,
				        (char **)&tp,10), *tp != RINGLISTSEP)
	       		{
	          	   ERRDISP(0,MSG_RBLD_INVLIMSYNTAX,ccp,aoip->anm,0);
	          	   return (struct sm_cmd_opt *)NULL;
	       		}
	       		if (*sp != RINGLISTSEP  ||  sp == wbufp)
		  	   break;

	       		/* Get message catalog name. */
	       		(void)memcpy((void *)svmf,(void *)wbufp,
			     	     /***  COMPILER ERROR! ***/
			     	     /*
			     	     len = MIN(sp-wbufp,sizeof(svmf)-1)),
			      	     */
			     	     len = MIN((char *)sp-wbufp,
					       sizeof(svmf)-1)),
	       		*(svmf+len) = 0;
	    	     } while (0);
	 	  }
               }
	       else
		  dvls = wbufp;
	       if (dvls)
		  (void)strncpy(sdv,dvls,sizeof(sdv)-1),
		  *(sdv+sizeof(sdv)-1) = 0,
		  sco.disp_values 		= sdv;
	       if (avls)
		  (void)strncpy(sav,avls,sizeof(sav)-1),
		  *(sav+sizeof(sav)-1) = 0,
		  sco.aix_values 		= sav;
	       free((void *)wbufp);
	    }
	    ccp = ccp1+1;
	    break;

	 case RANGEOP:			/* G[.]	- range list */
	    *sco.op_type 			= 'l';
	    *sco.cmd_to_list_mode 		= 'r';
	    FIND_SETENDCHAR;
	    *(sctl+sizeof(sctl)-1) = 0;
	    (void)strncpy(sctl,ODM_SCO_RINGCMD,sizeof(sctl)-1);
	    (void)memcpy((void *)(cp = sctl+strlen(sctl)),(void *)ccp,
			 len = MIN((size_t)(ccp1-ccp),
				   sizeof(sctl)-strlen(sctl)-1)),
	    *(cp+len) = 0;
	    sco.cmd_to_list 			= sctl;
	    ccp = ccp1+1;
	    break;

	 case VALIDATEOP:		/* V[.]	- validate expression */
	    FIND_SETENDCHAR;
	    ccp = ccp1+1;
	    break;

	 case SEQNOOP:			/* S[.]	- sequence no. */
	    FIND_SETENDCHAR;
	    if (ccp1 != ccp)
	    {
	       (void)memcpy((void *)sco.id_seq_num,(void *)ccp,
			    len = MIN((size_t)(ccp1-ccp),
				      sizeof(sco.id_seq_num)-1)),
	       *(sco.id_seq_num+len) = 0;
	       sqno--;
	    }
	    ccp = ccp1+1;
	    break;

	 case DISPLAYOP:		/* D?	- display mode */
	    *dflgp = *++ccp == 'n' ? FALSE : ALWAYS, ccp++;
	    break;

	 case CTLISTOP:			/* F?	- command to list mode */
	    *sco.cmd_to_list_mode 		= *++ccp, ccp++;
	    break;

	 case REQDOP:			/* Q?	- required */
	    *sco.required 			= *++ccp, ccp++;
	    break;

	 case COMPOP:			/* C[.]	- composite flag */
	    FIND_SETENDCHAR;
	    MALLOC(*cfsp,(size_t)((ccp1-ccp)+1));
	    (void)memcpy((void *)*cfsp,(void *)ccp,(size_t)(ccp1-ccp)),
	    *(*cfsp+(ccp1-ccp)) = 0;
	    ccp = ccp1+1;
	    Cflg = TRUE;
	    break;

	 case HELPMSGOP:		/* H[.]	- help message specs */
	    FIND_SETENDCHAR;
	    if (limp = memchr((void *)ccp,RINGLISTSEP,(size_t)(ccp1-ccp)))
	    {
	       (void)memcpy((void *)shml,(void *)ccp,
			    len = MIN((size_t)(limp-ccp),
				      sizeof(shml)-1)),
	       *(shml+len) = 0;
	       ++limp;
	       (void)memcpy((void *)sco.help_msg_id,(void *)limp,
			    len = MIN((size_t)(ccp1-limp),
				      sizeof(sco.help_msg_id)-1)),
	       *(sco.help_msg_id+len) = 0;
	    }
	    else
	       (void)memcpy((void *)shml,(void *)ccp,
			    len = MIN((size_t)(ccp1-ccp),
				      sizeof(shml)-1)),
	       *(shml+len) = 0;
	    sco.help_msg_loc 			= shml;
	    ccp = ccp1+1;
	    break;

	 case HELP1MSGOP:		/* I[.]	- help info specs */
	    FIND_SETENDCHAR;
	    do
	    {
	       char		tbuf[WKBUFLEN+1];

	       (void)memcpy((void *)tbuf,(void *)ccp,
			    len = MIN((size_t)(ccp1-ccp),sizeof(tbuf)-1)),
	       *(tbuf+len) = 0;
	       if (!(cp = strtok(tbuf,LISTSEPCHRSTR)))
	          break;
	       (void)strncpy(sco.help_msg_id,cp,sizeof(sco.help_msg_id)-1),
	       *(sco.help_msg_id+sizeof(sco.help_msg_id)-1) = 0;
	       if (!(cp = strtok(NULL,LISTSEPCHRSTR)))
	          break;
	       (void)strncpy(shmb,cp,sizeof(shmb)-1),
	       *(shmb+sizeof(shmb)-1) = 0;
	       sco.help_msg_base		= shmb;
	       if (!(cp = strtok(NULL,LISTSEPCHRSTR)))
	          break;
	       (void)strncpy(shmB,cp,sizeof(shmB)-1),
	       *(shmB+sizeof(shmB)-1) = 0;
	       sco.help_msg_book		= shmB;
	    } while (0);
	    ccp = ccp1+1;
	    break;

	 default:
            ERRDISP(0,MSG_RBLD_INVLIMOP,ccp,aoip->anm,0);
            return (struct sm_cmd_opt *)NULL;
      }
   }

   if (*dflgp != ALWAYS && fflg && *aoip->anm == CMPFLGPREFIX)
      *dflgp &= Cflg;
   if (!*dflgp)
      --sqno;
   return &sco;
}	/* end - rbld_parse_populate() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           rbld_catch_sigs()                                            *
*                                                                              *
* DESCRIPTION:    Catches signals so that clean up of ODM data can be          *
*                 performed (so as not to leave partial ODM info).             *
*                                                                              *
* PARAMETERS:     signo		signal no                                      *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static void
rbld_catch_sigs(int signo)
{
   gsigno = signo;
   siglongjmp(jmpenv,1);
}	/* end - rbld_catch_sigs() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           rbld_getmsg                                                  *
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
rbld_getmsg(const char *catnm,int setno,int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

   if (strcmp(catnm,prevcatnm))
   {
      (void)strncpy(prevcatnm,catnm,sizeof(prevcatnm)-1);
      if (md != (nl_catd)-1)
         md = ((void)catclose(md), (nl_catd)-1);
   }

   if (md == (nl_catd)-1)
   {
      if (*catnm != '/')
      {
         (void)strncpy(defmcpath,DEFMC_PREFIXPATH,sizeof(defmcpath)-1),
         *(defmcpath+sizeof(defmcpath)-1) = 0;
      }
      else
	 *defmcpath = 0;
      (void)strncat(defmcpath,catnm,sizeof(defmcpath)-strlen(defmcpath));
      if ((md = catopen(defmcpath, NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md, setno, msgno, (char *)dmsg),
		 dmsg) ? mp : NULL;
}	/* end - rbld_getmsg() */


