static char Xsccsid[] = "@(#)43	1.11.2.2  src/bos/usr/lib/sendmail/nlsbody.c, cmdsend, bos411, 9428A410j 3/17/94 14:18:32";
/* 
 * COMPONENT_NAME: CMDSEND nlsbody.c
 * 
 * FUNCTIONS: MSGSTR, esc_flat, esc_iso, iso_nls, jis_sj, 
 *            make_newbody, newtemp, nls_iso, sj_jis 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**
**  Created: 03/17/89, INTERACTIVE Systems Corporation
**
**  These routines were added to the sendmail program to
**  encode the body of a mail item for either NLS (AIX) or
**  ISO 8859 compatible systems.
**
*/

# include <stdio.h>
# include <fcntl.h>
# include <errno.h>
# include <memory.h>
# include <string.h>
# include <sys/lockf.h>
# include <sys/types.h>
# include <sys/stat.h>
# include "sysexits.h"
# include <netinet/in.h>
# include "conf.h"
# include "sendmail.h"
# include <sys/access.h>	  	/* security */

char * xalloc();
char * queuename();
    
#define ESC_OUTMAX MAXLINE*5	/* up to 5 chars per escape sequence */

#include <iconv.h>
#include <langinfo.h>
#include <stdlib.h>
#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 


static 	char *tif;		/* temp input file name */
static	FILE *tifp;		/* temp input file pointer */
static 	char *tof;		/* temp output file name */
static	FILE *tofp;		/* temp output file pointer */

iconv_t	convd;			/* conversion desciptor for iconv */

FILE *dfopen();

/*
**  MAKE_NEWBODY -- creat a new temp file of a different body type
**
**	Parameters:
**		e -- envelope
**		btype -- body type to be created
**
**	Returns:
**		none - file errors will be caught back in putbody
**
**	Side Effects:
**		may create a new file of the given body type.
*/

make_newbody(btype, e)
    int btype;
    ENVELOPE *e;
{
	char  *current_codeset;

#ifdef DEBUG
	if (tTd(66, 5))
		(void) printf("make_newbody: creating bodytype %d\n", btype);
#endif DEBUG
	/*
	**  We will use the original temp file if no conversion can (or should)
	**  be made.  Also, if there are are any file I/O errors while doing
	**  the conversion, it is probably most reasonable to use the original
	**  unconverted body.
	*/
	tof = e->e_df;
	tofp = e->e_dfp;

	/*
	**  Determine the body type, create a new body type if necessary
	**  and set the input file pointer (ifp).
	*/
	switch(btype)  {
	/* common body types */
	case BT_ESC: 	/* encode using NCesc() */
	    if (e->e_dfn != NULL)  {  	/* NLS extended characters? */
		tif = e->e_dfn;
		tifp = e->e_dfnp;
		convd = iconv_open("fold7", nl_langinfo(CODESET));
		newtemp(convd, 'e');
		e->e_dfnp = tifp;
	    }
	    e->e_dfe = tof;
	    e->e_dfep = tofp;
	    /* 
	    **  Note: We do not code ISO characters since this is not for
	    **  a local user and a proper NLS system on the receiving end
	    **  should do the encoding.
	    */
	    break;
	case BT_ISO:	/* encode with ISO 8859/1 characters */
	    if (e->e_dfn != NULL)  {	/* NLS extended characters? */
		tif = e->e_dfn;
		tifp = e->e_dfnp;
		current_codeset = nl_langinfo(CODESET);
		if (strcmp("ISO8859-1", current_codeset)) {
		   convd = iconv_open("ISO8859-1", current_codeset);
		   newtemp(convd, 'i');
		   e->e_dfnp = tifp;
		}
	    }
	    else if (e->e_dfe != NULL)  {	/* body from NCesc() ? */
		tif = e->e_dfe;
		tifp = e->e_dfep;
		convd = iconv_open("ISO8859-1", "fold7");
		newtemp(convd, 'i');
		e->e_dfep = tifp;
	    }
	    e->e_dfi = tof;
	    e->e_dfip = tofp;
	    break;
	case BT_NLS:	/* encode with nls extended characters */
	    if (e->e_dfe != NULL)  {	/* body from NCesc() ? */
		tif = e->e_dfe;
		tifp = e->e_dfep;
		convd = iconv_open(nl_langinfo(CODESET), "fold7");
		newtemp(convd, 'n');
		e->e_dfep = tifp;
	    } else { 			/* Unknown. Possibly ISO? */
		if (IsoIn)  {	/* assume incoming mail could be ISO? */
		    tif = e->e_df;
		    tifp = e->e_dfp;
		    convd = iconv_open(nl_langinfo(CODESET), "ISO8859-1");
		    newtemp(convd, 'n');
		    e->e_dfep = tifp;
		}
	    }
	    e->e_dfn = tof;
	    e->e_dfnp = tofp;
	    break;
	case BT_FLAT:	/* flatten to ASCII text */
	    if (e->e_dfn != NULL)  {	/* NLS extended characters? */
		tif = e->e_dfn;
		tifp = e->e_dfnp;
		convd = iconv_open("fold7", nl_langinfo(CODESET));
		newtemp(convd, 'f');
		e->e_dfnp = tifp;
	    }
	    e->e_dff = tof;
	    e->e_dffp = tofp;
	    /* 
	    **  Note: We do not code ISO characters since this is none
	    **  of our business.  In this case it would be mail between
	    **  non-NLS systems.
	    */
	    break;
  /* Kanji body types */
	case BT_NC:	/* encode as NetCode */
	    tif = e->e_dfs;
	    tifp = e->e_dfsp;
	    convd = iconv_open(NetCode, MailCode);
	    newtemp(convd, 'j');
	    e->e_dfsp = tifp;
	    e->e_dfj = tof;
	    e->e_dfjp = tofp;
	    break;
	case BT_MC: 	/* encode as MailCode */
	    tif = e->e_dfj;
	    tifp = e->e_dfjp;
	    convd = iconv_open(MailCode, NetCode);
	    newtemp(convd, 's');
	    e->e_dfjp = tifp;
	    e->e_dfs = tof;
	    e->e_dfsp = tofp;
	    break;
	}
}



/*  Create the new temp file name, create the file, and do the conversion. */

newtemp(conv, type)  
    iconv_t conv;
    char type;
{
	int ret, errors = 0;
	char *outname;
	FILE *outfile;
	char buf[MAXLINE], obuf[ESC_OUTMAX];
	char *inpointer, *outpointer;
	size_t inbytes, outbytes;

	/* just return if no input file name */
	if (tif == NULL)
	    return;

	/* just return if conversion descriptor is equal to a -1 */
	if (conv < 0) {
#ifdef LOG
		syslog(LOG_ERR, MSGSTR(NL_CONVD, "iconv_open() did not return a valid conversion descriptor"));
#endif LOG
		return;
	}

	outname = newstr(queuename(CurEnv, type));
	/* try to open in case it already exists */
	if ((outfile = dfopen(outname, "r")) != NULL)  {
	    tof = outname;
	    tofp = outfile;
	    return;
	}


	/* We will create it, since it doesn't exist */
	if ((outfile = dfopen(outname, "w")) == NULL)  {
	    syserr(MSGSTR(CO_ECREAT, "Cannot create %s"), outname); /*MSG*/
	    return;
	}
	(void) acl_set (outname, R_ACC | W_ACC, R_ACC | W_ACC, NO_ACC);

	/* Open the input file for read if not already open. */
	if (tifp == NULL)  {
	    tifp = fopen(tif, "r");
	    if (tifp == NULL)  {
		syserr(MSGSTR(NL_EOPENNT, "newtemp: Cannot open %s"), tif); /*MSG*/
		tof = NULL;
		return;
	    }
	}
	rewind(tifp);

	/* Empty the input and output buffers before passing to iconv() */
	bzero(buf, sizeof(buf));
	bzero(obuf, sizeof(obuf));

	/* Read in each line, convert it, and write it out. */
	while (!ferror(tifp) && fgets(buf, sizeof buf, tifp) != NULL) {
	    inbytes = MAXLINE;
	    outbytes = ESC_OUTMAX;
	    inpointer = buf;
	    outpointer = obuf;
	    ret = iconv(conv, &inpointer, &inbytes, &outpointer, &outbytes);
	    if (ret < 0) {
		errors++;
		syserr(MSGSTR(NL_ICONV, "newbody: iconv error occurred\n")); /*MSG*/
		break;
	    }
	    fputs(obuf, outfile);
	}
	if (ferror(tifp)) {
	    errors++;
	    syserr(MSGSTR(NL_EREADNB, "newbody: read error")); /*MSG*/
	}

	(void) fflush(outfile);
	if (ferror(outfile) && errno != EPIPE)  {
	    errors++;
	    syserr(MSGSTR(NL_EWRITENB, "newbody: write error")); /*MSG*/
	}
	errno = 0;
	fclose(outfile);		

	/* If we have no errors, we can set the "tof" and "tofp" */
	if (errors == 0)  {
	    if ((outfile = dfopen(outname, "r")) == NULL)
		syserr(MSGSTR(CO_EREOPEN, "Cannot reopen %s"), outname); /*MSG*/
	    tof = outname;
	    tofp = outfile;
	} else {
	    /* otherwise, remove any evidence */
	    unlink(outname);
	}
}
