static char sccsid[] = "@(#)38	1.5  src/bos/usr/lib/pios/piosecure.c, cmdpios, bos411, 9428A410j 7/30/91 11:04:45";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: getlabel, print_label
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <IN/backend.h>
#include <stdio.h>
#include <string.h>
#include "pioformat.h"

extern char *getmsg();

extern struct shar_vars *sh_vars;

/* AIX security enhancement */
int
getlabel(attr)
    struct label_attr *attr;		/* label attribute structure */
{
  /*extern char *get_label();*/

    char *tmp_label;		/* pointer to label returned by get_label() */
    int msgnum;

    attr->label = NULL;
    attr->label_size = 0;
    if (attr->label_type == LAB_TOP)                    /* use canned label */
	msgnum = MSG_TOPLAB;                            /* since labels are */
    else                                                /* not currently    */
	msgnum = MSG_BOTLAB;                            /* supported        */
    tmp_label = getmsg(MF_PIOBE, 1, msgnum); /*                  */
  /*tmp_label = get_label(attr->label_type);*/          /* get the label */
    if (tmp_label == NULL || *tmp_label == '\0')        /* return NULL */
      return(attr->label_size);

    switch (attr->label_type) {
	case LAB_TOP:		/* malloc space for top & bot labels because */
	case LAB_BOTTOM:	/* they will be reused (for each page).      */
	    attr->label_size = strlen(tmp_label) + 1;
	    MALLOC(attr->label, attr->label_size + 1);
	    (void) strcpy(attr->label, tmp_label);
	    attr->label[attr->label_size - 1] = '\n';
	    attr->label[attr->label_size] = '\0';
	    break;

	default:
	    attr->label_size = -1;
	    break;
    }
    return(attr->label_size);
}
/* TCSEC Division C Class C2 */

/* AIX security enhancement */

char sav_cmd[BUFSIZ];

print_label(attr, doprint)
    struct label_attr *attr;
    short  doprint;		/* all purpose flag... kind of ugly */
{
    char *s1;
    char *s2;
    char strbuf[BUFSIZ];

    int  lineout();

    int	count	   = 0;		/* number of bytes formatted by lineout() */
    int	differ	   = 0;		/* number of bytes formatted by lineout() */
    int i	   = 0;		/* number of bytes written to pipe */
    int label_size = 0;		/* absolute size of label in vertical units */
    int pipedes[2];		/* file descriptors for pipe */
    int prtflg     = 0;		/* flag */
    int	sum	   = 0;		/* sum total of bytes sent to pipe */
    int strlen	   = 0;
    int retcode	   = 0;
    int tmp_vpos   = 0;		/* temporary holding pen for sh_vpos */
    int wcnt	   = 0;		/* number of bytes written to pipe */

    FILE  *rdptr;		/* file pointer fo pipe */
    FILE  *wrtptr;		/* file pointer fo pipe */

    /*
     *  We feed the character(+) string into a pipe so that we can have
     *  lineout() process the string for us.  lineout() takes a file pointer
     *  as an argument so we fdopen() the pipe file descriptors.
     */
    piomode = PIO_SECUR_MODE;
    if (doprint)
    {
	strlen = piogetstr(sh_set_cmd, strbuf, sizeof(strbuf), NULL);
	s1 = strbuf, s2 = sav_cmd;
	for (i = 0; i < strlen; *s2++ = *s1++, i++)
	    if (*s1 != *s2)
	       differ++;
	if (differ)
	for (i = 0, s1 = strbuf; i < strlen; i++)
	    pioputchar(*s1++);
    }

    if (attr->label_size <= 0)
    {
	piomode = PIO_CURR_MODE;
	return(0);
    }

    if (pipe(pipedes))					/* open pipe */
	return(-1);

    if ((rdptr = fdopen(pipedes[READ], "r")) == NULL)	/* create read filptr */
	return(-1);

    if ((wrtptr = fdopen(pipedes[WRITE], "w")) == NULL)	/* create wrt filptr  */
	return(-1);

    if (doprint)
	prtflg = PRINT;

    /**
    ***  Loading pipe, if the pipe is full, call lineout()
    ***  to smoke a line, then continue loading the pipe.  Exit when
    ***  all the leaf has been loaded into the pipe.
    **/
    while (sum < attr->label_size)
    {
	if ((wcnt = write(pipedes[WRITE], attr->label, attr->label_size)) < 0)
	    return(-1);
	else if (wcnt == 0)
	{
	    lineout(rdptr);
	    if (doprint)
		piocmdout(sh_vincr_cmd, NULL, 0, NULL);
	}
	else 
	    sum += wcnt;
    }

    /* light pipe */
    (void) fclose(wrtptr);
    (void) close(pipedes[WRITE]);

    /* Smoke pipe until roasted */
    /* format line and increment sh_vpos */

    if (!prtflg)
    {
	tmp_vpos = sh_vpos;
	sh_vpos = 0;
    }

    do
    {
	count += lineout(rdptr);
	if (sh_vpos <= sh_pl)
        {
	    if (doprint && sh_vpos < sh_pl)
		piocmdout(sh_vincr_cmd, NULL, 0, NULL);
        }
	else
	    break;
    } while (count < sum);

    if (!prtflg)
    {
        label_size = sh_vpos;
	sh_vpos = tmp_vpos;
    }

    /* Clean the pipe and put it away. */
    (void) close(pipedes[READ]);
    (void) fclose(rdptr);

    piomode = PIO_CURR_MODE;
    strlen = piogetstr(sh_set_cmd, strbuf, sizeof(strbuf), NULL);
    s1 = strbuf, s2 = sav_cmd;

    for (i = 0; i < strlen; *s2++ = *s1++, i++)
	if (*s1 != *s2)
	    differ++;

    if (differ)
	for (i = 0, s1 = strbuf; i < strlen; i++)
	    pioputchar(*s1++);

    return(label_size);
}

/* TCSEC Division C Class C2 */
