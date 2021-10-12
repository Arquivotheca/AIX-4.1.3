static char sccsid[] = "@(#)75  1.2  src/bos/usr/lib/pios/pioevattr.c, cmdpios, bos411, 9428A410j 11/9/93 11:04:30";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  main(), ea_msg(), ea_getav()
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <nl_types.h>
#include <dirent.h>
#include <errno.h>
#include "pioformat.h"

/* External declarations */
extern int		optopt;		/* should've been in <stdlib.h> */
extern int              attrtab_init(char *);
extern int              piogetstr(char *, char *, int, error_info_t *);

/* Misc. macros */
#define STRGZARG(a)		#a
#define STRGZS(a)		STRGZARG(a)
#define WKBUFLEN		(1023)
#define RDBUFSIZE		(WKBUFLEN+1)
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc/"
#define MF_PIOBESETNO		(8)
#define ENVBASEDIR		"PIOBASEDIR"
#define ENVVARDIR		"PIOVARDIR"
#define DDIDIR			"/ddi"
#define QQDEVSEP		":"
#define SETBEGINCHR		'#'
#define SETSEPCHR		':'
#define SMITCCMT		"#!:"
#define MAXQNMLEN		20
#define MAXDNMLEN		20
#define ATTRNMLEN		2
#define BURSTDEFAULT		"nn"
#define HEADER			"header"
#define TRAILER			"trailer"
#define HT_NEVER		"never"
#define HT_FALSE		"false"
#define HT_GROUP		"group"
#define HT_ALWAYS		"always"
#define WHSPCHRS		" \t"
#define WHSPCHRS1		" \t="
#define ASSGCHR			'='
#define CMNTCHR			'#'
#define LSQUEDEVFMT		"/bin/lsquedev -q%." STRGZS(MAXQNMLEN) \
				"s -d%." STRGZS(MAXDNMLEN) "s"
#if defined(MIN)
#undef MIN
#endif		/* MIN */
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#undef  MALLOC
#define MALLOC(p, sz)		do \
				{ \
				   if (!((p) = malloc((size_t)(sz)))) \
				   { \
         			      ea_msg(stderr, MSG_EA_MALLOCERR, \
					     DEFMSG_EA_MALLOCERR, pgnm, \
					     strerror(errno)); \
				      exit(EXIT_FAILURE); \
				   } \
				} while (0)

/* Macros for default messages */
#define DEFMSG_EA_ERRMSG	"Cannot access the message catalog " \
				MF_PIOBE ".\n"
#define DEFMSG_EA_USAGE		"Usage:\t%s -q PrintQueue -d QueueDevice " \
				"[ -u ] AttributeName...\n"
#define MSG_EA_MISSARG		MSG_LVP_MISSARG
#define DEFMSG_EA_MISSARG	"%s: Missing argument for the flag -%c\n"
#define MSG_EA_ILLOPT		MSG_LVP_ILLOPT
#define DEFMSG_EA_ILLOPT	"%s: Illegal flag -%c\n"
#define MSG_EA_ILLUSE		MSG_LVP_ILLUSE
#define DEFMSG_EA_ILLUSE	"%s: Illegal usage\n"
#define MSG_EA_MALLOCERR	MSG_LVP_MALLOCERR
#define DEFMSG_EA_MALLOCERR	"%s: Error '%s' in malloc()\n"
#define MSG_EA_DOPENERR		MSG_LVP_DOPENERR
#define DEFMSG_EA_DOPENERR	"%s: Error '%s' in opening the directory %s\n"
#define DEFMSG_EA_NOVP		"%s: No virtual printer exists for the " \
				"specified queue %s and queue device %s\n"
#define DEFMSG_EA_NOATTR	"%s: The attribute %" EXP_STRGZ(ATTRNMLEN) "." \
				EXP_STRGZ(ATTRNMLEN) "s doesnt exist in the " \
				"virtual printer definition\n"
#define MSG_EA_POPENERR		MSG_LVP_POPENERR
#define DEFMSG_EA_POPENERR	"%s: Error '%s' in opening a pipe to command "\
				"%s\n"
#define MSG_EA_PRDERR		MSG_LVP_PRDERR
#define DEFMSG_EA_PRDERR	"%s: Error '%s' in reading a pipe to command "\
				"%s\n"

/* Local function declarations */
static void 		ea_msg(FILE *, int, const char *, ...);
static const char	*ea_getav(const char *, int, const char *,
				  const char *);

/* Local variable definitions */
static const char	*pgnm;
static char		bdir[PATH_MAX+1];
static char		vdir[PATH_MAX+1];
char *hash_table;       /* address of the primary hash table */
char *mem_start;        /* address of the start of the odm data */
char *odm_table;        /* address of the odm table */
int objtot      = 0;    /* number of objects in odm table */
int piomode     = 0;    /* indicates the set of attr. values in use */
int piopgskip   = 0;    /* number of pages to skip (suppress printing of)  */
int piodatasent = FALSE;/* data bytes sent to printer yet? */
int statusfile  = 0;    /* indicates whether a statusfile exists */



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
   int				qflag 		= 0;	/* flag for queue */
   int				dflag 		= 0;	/* flag for qdev */
   int				uflag 		= 0;	/* flag for unresolv */
   const char			*qnm; 			/* queue name */
   const char			*dnm; 			/* qdev name */
   const char			*ccp;
   char				*cp;
   char				ddifl[PATH_MAX+1];	/* ddi file (dir) */
   char				fqnm[MAXQNMLEN+MAXDNMLEN+1+1];	/* 1 for ':' */
   DIR				*Dp;
   register struct dirent	*dp;
   char				const**anmspp;
   register char		const**cpp;
   int				es 		= TRUE;
   register int			i;
   
   (void) setlocale(LC_ALL, "");

   /* Process the arguments and flags. */
   pgnm = *av;
   for (opterr = 0, optind = 1;
	(i = getopt((int)ac, av, ":q:d:u")) != EOF; )
      switch (i)
      {
	 case 'q':
	    qnm = optarg;
	    qflag++;
	    break;

	 case 'd':
	    dnm = optarg;
	    dflag++;
	    break;

	 case 'u':
	    uflag++;
	    break;

	 case ':':
	    ea_msg(stderr, MSG_EA_MISSARG, DEFMSG_EA_MISSARG, pgnm,
		   (char)optopt),
	    ea_msg(stdout, MSG_EA_USAGE, DEFMSG_EA_USAGE, pgnm);
	    exit(EXIT_FAILURE);

	 case '?':
	    ea_msg(stderr, MSG_EA_ILLOPT, DEFMSG_EA_ILLOPT, pgnm,
		   (char)optopt),
	    ea_msg(stdout, MSG_EA_USAGE, DEFMSG_EA_USAGE, pgnm);
	    exit(EXIT_FAILURE);
      }
   if (!qflag  ||  !dflag  ||  optind == ac)
   {
      ea_msg(stderr, MSG_EA_ILLUSE, DEFMSG_EA_ILLUSE, pgnm),
      ea_msg(stdout, MSG_EA_USAGE, DEFMSG_EA_USAGE, pgnm);
      exit(EXIT_FAILURE);
   }
   (void)strncpy(bdir,(cp = getenv(ENVBASEDIR))?cp:DEFBASEDIR,sizeof(bdir)-1),
   *(bdir+sizeof(bdir)-1) = 0;
   (void)strncpy(vdir,(ccp = getenv(ENVVARDIR))?ccp:cp?cp:DEFVARDIR,
		 sizeof(vdir)-1),
   *(vdir+sizeof(vdir)-1) = 0;

   /* Determine the ddi file and load it into memory. */
   (void) strncpy(fqnm, qnm, MAXQNMLEN),
   *(fqnm+MAXQNMLEN) = 0;
   (void) strcat(fqnm, QQDEVSEP);
   cp = fqnm+strlen(fqnm);
   (void) strncpy(cp, dnm, MAXDNMLEN),
   *(cp+MAXDNMLEN) = 0;
   (void) strncpy(ddifl, vdir, sizeof(ddifl)-strlen(DDIDIR)-1),
   *(ddifl+sizeof(ddifl)-strlen(DDIDIR)-1) = 0;
   (void) strcat(ddifl, DDIDIR);
   if (!(Dp = opendir(ddifl)))
   {
      ea_msg(stderr, MSG_EA_DOPENERR, DEFMSG_EA_DOPENERR, pgnm,
	     strerror(errno), ddifl);
      exit(EXIT_FAILURE);
   }
   while (dp = readdir(Dp))
      if (strlen(dp->d_name) > strlen(fqnm)  &&
	  !strcmp(fqnm, dp->d_name+strlen(dp->d_name)-strlen(fqnm)))
	 break;
   if (!dp)
   {
      ea_msg(stderr, MSG_EA_NOVP, DEFMSG_EA_NOVP, pgnm, qnm, dnm);
      exit(EXIT_FAILURE);
   }
   (void) closedir(Dp);
   (void) strncat(ddifl, "/", sizeof(ddifl)-strlen(ddifl)-1),
   *(ddifl+sizeof(ddifl)-1) = 0;
   (void) strncat(ddifl, dp->d_name, sizeof(ddifl)-strlen(ddifl)-1);
   (void) attrtab_init(ddifl);

   /* Evaluate specified attributes. */
   MALLOC(anmspp, (ac-optind)*sizeof(char *));
   for (cpp = anmspp; optind < ac; optind++)
      if (piogetstr(av[optind], NULL, 0, NULL))
	 *cpp++ = av[optind];
      else
         ea_msg(stderr, MSG_EA_NOATTR, DEFMSG_EA_NOATTR, pgnm, av[optind]),
         es = FALSE;
   if (cpp-anmspp)
   {
      int				ano;

      i = ano = cpp-anmspp-1;
      cpp = anmspp;
      (void) putchar(SETBEGINCHR);
      (void) putchar((int)**cpp),
      (void) fputc((int)*(*cpp+++1), stdout);
      for ( ; i; i--, cpp++)
      {
	 (void) putchar(SETSEPCHR);
	 (void) putchar((int)**cpp),
	 (void) putchar((int)*(*cpp+1));
      }
      (void) putchar('\n');
      cpp = anmspp;
      for (ccp = ea_getav(*cpp++, !uflag, qnm, dnm); *ccp; ccp++)
	 if (*ccp == SETSEPCHR)
	    (void) fputs(SMITCCMT, stdout);
	 else
	    (void) putchar((int)*ccp);
      for (i = ano; i; i--)
      {
	 (void) putchar(SETSEPCHR);
	 for (ccp = ea_getav(*cpp++, !uflag, qnm, dnm); *ccp; ccp++)
	    if (*ccp == SETSEPCHR)
	       (void) fputs(SMITCCMT, stdout);
	    else
	       (void) putchar((int)*ccp);
      }
      (void) putchar('\n');
   }
   free((void *)anmspp);

   return es ? EXIT_SUCCESS : EXIT_FAILURE;
}	/* end - main() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           ea_msg                                                       *
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
ea_msg(FILE *outfp, int msgno, const char *defmsg, ...)
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
}	/* end - ea_msg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           ea_getav                                                     *
*                                                                              *
* DESCRIPTION:    Fetch attribute value for a given attribute.                 *
*                                                                              *
* PARAMETERS:     anm		attribute name                                 *
*                 res		flag to resolve or not                         *
*                 qnm		queue name                                     *
*                 dnm		device name                                    *
*                                                                              *
* RETURN VALUES:  ccp		evaluated attribute value                      *
*                                                                              *
*******************************************************************************/
static const char *
ea_getav(const char *anm, int res, const char *qnm, char const *dnm)
{
   static char			wkbuf[WKBUFLEN+1];
   const char			*ccp;
   struct odm			*op;
   char				cmd[PATH_MAX+1];
   FILE				*pfp;
   char				rdbuf[LINE_MAX+1];
   register char		*cp;
   register char		*cp1;
   register char		*cp2;
   const char			*endp;
   size_t			maxlen;

   if (memcmp((void *)anm, (void *)BURSTPAGE, ATTRNMLEN))
         ccp = res ?
	       (void) piogetstr((char *)anm, wkbuf, sizeof wkbuf, NULL),
	       wkbuf :
	       (op = get_odmtab_ent((char *)anm),
	        (void)memcpy((void *)wkbuf, (void *)GET_OFFSET(op->str),
			     maxlen = MIN((size_t)op->length,sizeof(wkbuf)-1)),
		*(wkbuf+maxlen) = 0, wkbuf);
   else
   {
      (void) strncpy(wkbuf, BURSTDEFAULT, sizeof(wkbuf)-1),
      *(wkbuf+sizeof(wkbuf)-1) = 0;
      (void)sprintf(cmd, LSQUEDEVFMT, qnm, dnm);
      if (!(pfp = popen(cmd,"r")))
      {
         ea_msg(stderr,MSG_EA_POPENERR,DEFMSG_EA_POPENERR,pgnm,
		 strerror(errno),cmd);
	 return FALSE;
      }
      while (fgets(rdbuf,sizeof rdbuf,pfp))
      {
         (void)strtok(rdbuf,"\n");
         if (!*rdbuf || (cp = rdbuf+strspn(rdbuf,WHSPCHRS), !*cp) ||
             *cp == CMNTCHR || *cp == ASSGCHR || !strchr(cp,ASSGCHR))
            continue;
	 endp = cp+strlen(cp);
	 if (!(cp = strtok(cp, WHSPCHRS1))  ||
	     (cp1 = cp+strlen(cp)+1) > endp  ||
	     (cp1 += strspn(cp1, WHSPCHRS1), !*cp1))
	    continue;
	 if (!strcmp(cp, HEADER))
	    cp2 = wkbuf;
	 else if (!strcmp(cp, TRAILER))
	    cp2 = wkbuf+1;
	 else
	    continue;
	 *cp2 = !strcmp(cp1, HT_NEVER)  ?	'n' :
		!strcmp(cp1, HT_FALSE)  ?	'n' :
		!strcmp(cp1, HT_ALWAYS) ?	'a' :
		!strcmp(cp1, HT_GROUP)  ?	'g' :
		'n';
      }
      if (ferror(pfp))
      {
         ea_msg(stderr,MSG_EA_PRDERR,DEFMSG_EA_PRDERR,pgnm,
	         strerror(errno),cmd);
         return FALSE;
      }
      (void)pclose(pfp);
      ccp = wkbuf;
   }

   return ccp;
}	/* end - ea_getav() */
