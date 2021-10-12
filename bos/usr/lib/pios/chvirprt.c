static char sccsid[] = "@(#)11  1.12.1.2  src/bos/usr/lib/pios/chvirprt.c, cmdpios, bos411, 9428A410j 7/23/93 16:23:59";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*** chvirprt.c ***/

#include <locale.h>
#include "virprt.h"
#include <sys/lockf.h>
#include <fcntl.h>
#include <sys/mode.h>
#include <string.h>

/*===========================================================================*/
/*
**  A bunch of interesting stuff is done here.
**
**  First, we make sure that attributes for default state and current state
**  ("zD" and "zS") are not passed in argument list.  If so, an error msg
**  is displayed and the program exits.  Then a check is made to see if
**  the custom colon file exists.  If it does, we
**  use piocnvt to make certain that it's fully expanded, but the expanded
**  version is put out in a temporary file (even if the file existing
**  in the custom directory is already expanded).
**
**  Then the temp file is opened for reading, and the custom file is
**  opened for writing.  On line at a time, the temp file is read.  The
**  attribute name is compared with the attributes found on the command line.
**  When a match is found, the changed attribute value is copied over the
**  original value.  Finally, the line is written out to the custom file.
**
**  When the whole file has been thru the translation process, the new
**  custom file is digested.  Then, piocnvt is called again, without
**  a -s flag, to make sure that the file is left in the state the user
**  desires it to be left in (as indicated by the 'zD' attribute.
**
**  After all that, we clean up and go home.
*/
/*===========================================================================*/
void main(argc,argv)
int argc;
char *argv[];
{
    int i;
    int fd;   /* file descriptor used to check if the file is locked */
    FILE *tmp_file, *cln_file;
    char attr[3], line[LINE_MAX], *ptr;
    register struct attr	*tmp_attp;	/* temp ptr to attr list */
    PROHIBIT_ATTRLIST_DEFN;			/* list of prohibited attrs */
    register char		**tmp_prohibit_attp;
					/* temp ptr to prohibited attr list */

    (void) setlocale(LC_ALL, "");

   /* Convert New Flag Letters (if present) To Old Flag Letters (s->d, d->v) */
    { CNVTFLAGS }

    putenv("IFS=' \t\n'");
    make_files();           /* create full path names from PIOBASEDIR */


    INIT(custom_name);        /* initialize custom file name */

    if ( parse(argc,argv,"q:v:","qv",0,&att,2,&pqname,&vpname) )
        err_sub(ABORT,USAGE_CH);

    /* If "zD" or "zS" are passed in the argument list, display an error
       and exit. */
    for (tmp_attp = att; *tmp_attp->attribute; tmp_attp++)
       for (tmp_prohibit_attp = prohibit_attrs; *tmp_prohibit_attp;
	    tmp_prohibit_attp++)
	  if (!strcmp (*tmp_prohibit_attp, tmp_attp->attribute))
             (void) strcpy (cmd, tmp_attp->attribute),	/* err_sub uses 'cmd' */
	     (void) err_sub (ABORT, MSG_ATTR_NONMODFBLE); 

    sprintf(cusfile,"%s%s:%s", cuspath, pqname, vpname);
    switch( file_stat(cusfile) )
        {
        case DZNTXST: err_sub(ABORT,CUS_NOXST);             /* doesn't exist */
        case PERM_OK: break;                                /* exists */
        case PERM_BAD: err_sub(ABORT,CUS_NOPEN);            /* can't open */
        }
    fd = open(cusfile,O_RDONLY);
    if (lockf(fd,F_TEST,0) == -1)
        {  /* file is locked by somebody else using lsvirprt */
        sprintf(custom_name,"%s:%s",pqname, vpname);
        err_sub(ABORT,MSG_FILELOCK); 
        }
    close(fd);

    umask(75);
    sprintf(cmd,"%s -s+ -i%s; %s %s %s",
            PIOCNVT,cusfile, CP, cusfile,tempfile);
    system(cmd);

    tmp_file = fopen(tempfile,"r");
    cln_file = fopen(cusfile,"w");
    while ( !fgetln(tmp_file,line) )
        {
        ptr = line;
        while ( *ptr++ != ':' );
        while ( *ptr++ != ':' );

        /*
        **  Get attribute name
        */
        attr[0] = *ptr++;
        attr[1] = *ptr++;
        attr[2] = NULL;

        while ( *ptr++ != ':' );
        while ( *ptr++ != ':' );

        i = 0;
        while ( *att[i].attribute )
            {
            if ( !strcmp(att[i].attribute,attr) ) strcpy(ptr,att[i].value);
            i++;
            }
        fprintf(cln_file,"%s\n",line);
        }

    fclose(tmp_file);
    fclose(cln_file);
    unlink(tempfile);

	/* digest the custom file and exit if there is an error */
    if (call_piodigest() != 0)
		exit(1);

    sprintf(cmd,"%s -i%s",PIOCNVT, cusfile);
    system(cmd);

    exit(0);
}
