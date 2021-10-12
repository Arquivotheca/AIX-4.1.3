static char sccsid[] = "@(#)18  1.1  src/bos/usr/lib/pios/piomsgipc.c, cmdpios, bos411, 9428A410j 12/8/93 18:35:22";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  pmi_initmsgipc(), pmi_bldhdrfrm(), pmi_bldintfrm(),
 *             pmi_bldstrfrm(), pmi_dsptchmsg()
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
#include <sys/access.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include "piostruct.h"

#if defined(MIN)
#undef MIN
#endif		/* MIN */
#define MIN(a,b)		((a)<(b)?(a):(b))
#define ARRAYSIZ(a)		(sizeof(a)/sizeof(*a))
#define MAXINTSTRLEN		15

/* External declarations */
extern int		faccessx(int,int,int);
				/* should have been in a system hdr file */

/* Global variable definitions */
uchar_t			piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */

/* Local variable definitions */
static int		piomsgipcfd;	/* msg ipc channel */
static piomsghdr_t	piomh;		/* msg header */
static piomarghdr_t	piomah[PM_MAXMSGARGNO];		/* msg arg headers */
static char		*piomat[ARRAYSIZ(piomah)];	/* msg arg hdr texts */

/* Function declarations */
int	pmi_initmsgipc(void);
int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
int	pmi_bldintfrm(int);
int	pmi_bldstrfrm(const char *);
int	pmi_dsptchmsg(const char *);



/*******************************************************************************
*
*
* NAME:           pmi_initmsgipc
*
* DESCRIPTION:    Initializes IPC data structures for messages.
*
* PARAMETERS:
*
* RETURN VALUES:
*                 TRUE		success
*		  FALSE		failure
*
*******************************************************************************/
int
pmi_initmsgipc(void)
{
   char const			*cp;

   if (piomsgipc = FALSE, !(cp = getenv(PIOMSGIPCENV)))
      return FALSE;
   if (errno = 0, piomsgipcfd = strtol(cp,NULL,10), errno)
      return FALSE;
   if (faccessx(piomsgipcfd,W_OK,ACC_SELF) == -1)
      return FALSE;
   piomsgipc = TRUE;

   return TRUE;
}	/* end - pmi_initmsgipc() */



/*******************************************************************************
*
*
* NAME:           pmi_bldhdrfrm
*
* DESCRIPTION:    Builds the header frame for message IPC.
*
* PARAMETERS:
*
* RETURN VALUES:
*                 TRUE		success
*		  FALSE		failure
*
*******************************************************************************/
int
pmi_bldhdrfrm(uchar_t mt,ushort_t mno,ushort_t msno,const char *mcnm)
{
   piomh.pm_msgtype		=	mt;
   piomh.pm_msgno		=	mno;
   piomh.pm_setno		=	msno;
   (void)strncpy(piomh.pm_catnm,mcnm,sizeof(piomh.pm_catnm)-1);

   return TRUE;
}	/* end - pmi_bldhdrfrm() */



/*******************************************************************************
*
*
* NAME:           pmi_bldintfrm
*
* DESCRIPTION:    Builds an argument frame for an argument of type 'int'.
*
* PARAMETERS:
*
* RETURN VALUES:
*                 TRUE		success
*		  FALSE		failure
*
*******************************************************************************/
int
pmi_bldintfrm(int i)
{
   char 			s[MAXINTSTRLEN+1];
   size_t			len;
   char				*cp;

   return ((void)sprintf(s,"%d",i),
  	   (!(cp = malloc(len = (strlen(s)+1))) ? FALSE :
	    (piomh.pm_msgargno < ARRAYSIZ(piomah) ?
	     (void)strcpy(cp,s),
	     piomah[piomh.pm_msgargno].pa_argtype = PA_ATINT,
	     piomah[piomh.pm_msgargno].pa_arglen = len,
	     piomat[piomh.pm_msgargno++] = cp,
	     TRUE : ((void)free((void *)cp), FALSE))));
}	/* end - pmi_bldintfrm() */



/*******************************************************************************
*
*
* NAME:           pmi_bldstrfrm
*
* DESCRIPTION:    Builds an argument frame for an argument of type 'string'.
*
* PARAMETERS:
*
* RETURN VALUES:
*                 TRUE		success
*		  FALSE		failure
*
*******************************************************************************/
int
pmi_bldstrfrm(const char *s)
{
   char				*cp;
   size_t			len;

   return (piomh.pm_msgargno < ARRAYSIZ(piomah) ?
	   (cp = malloc(len = strlen(s)+1)) ?
	   (void)strcpy(cp,s),
	   piomah[piomh.pm_msgargno].pa_argtype = PA_ATSTRING,
	   piomah[piomh.pm_msgargno].pa_arglen = len,
	   piomat[piomh.pm_msgargno++] = cp,
	   TRUE : FALSE : FALSE);
}	/* end - pmi_bldstrfrm() */



/*******************************************************************************
*
*
* NAME:           pmi_dsptchmsg
*
* DESCRIPTION:    Dispatches the message header frame and message argument
*		  header frames (if any).  Then this function dispatches the
*		  default message text and the message arguments (if any).
*
* PARAMETERS:
*
* RETURN VALUES:
*                 TRUE		success
*		  FALSE		failure
*
*******************************************************************************/
int
pmi_dsptchmsg(const char *dmsg)
{
#  define WRITE_DATA_LOOP(siz,buf)	\
   for (wc = (siz), vp = (buf); \
	wc > 0 && (rc = write(piomsgipcfd,vp,wc)) >= 0; \
	wc -= rc, vp = (const char *)vp+rc) \
	/* empty statement */
#  define RETURN_RC_ERR	\
   do { \
      if (rc < 0) { \
   	 lck.l_type = F_UNLCK; \
   	 (void)fcntl(piomsgipcfd, F_SETLKW, &lck); \
	 return FALSE; \
      } \
   } while(0)

   ssize_t			rc;
   register size_t		wc;
   struct flock			lck;
   register const piomarghdr_t	*pahp;
   register const void		*vp;
   char				**patp;

   piomh.pm_msglen	=	strlen(dmsg)+1;
   lck.l_whence = lck.l_start = lck.l_len = 0;
   lck.l_type = F_WRLCK;
   (void)fcntl(piomsgipcfd, F_SETLKW, &lck);

   WRITE_DATA_LOOP(sizeof piomh,&piomh);
   RETURN_RC_ERR;
   for (pahp = piomah; pahp < piomah+piomh.pm_msgargno; pahp++) {
      WRITE_DATA_LOOP(sizeof(*pahp),pahp);
      RETURN_RC_ERR;
   }
   WRITE_DATA_LOOP(strlen(dmsg)+1,dmsg);
   RETURN_RC_ERR;
   for (patp = piomat; patp < piomat+piomh.pm_msgargno; patp++) {
      WRITE_DATA_LOOP(strlen(*patp)+1,*patp);
      RETURN_RC_ERR;
   }

   lck.l_type = F_UNLCK;
   (void)fcntl(piomsgipcfd, F_SETLKW, &lck);
   for (patp = piomat; patp < piomat+piomh.pm_msgargno; patp++)
      (void)free(*patp);
   (void)memset((void *)&piomh,0,sizeof piomh);
   (void)memset((void *)piomah,0,sizeof piomah);
   (void)memset((void *)piomat,0,sizeof piomat);

   return TRUE;

#  undef RETURN_RC_ERR
#  undef WRITE_DATA_LOOP
}	/* end - pmi_dsptchmsg() */


