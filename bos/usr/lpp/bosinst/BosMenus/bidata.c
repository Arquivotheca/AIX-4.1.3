static char sccsid[] = "@(#) 84 1.1 src/bos/usr/lpp/bosinst/BosMenus/bidata.c, bosinst, bos411, 9428A410j 93/09/16 08:46:20";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: bidata
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME: bidata.c
 *
 * FUNCTION: retrieve a value from the bosinst.data or image.data file
 *
 * ENVIRONMENT:
 *	2 pipes are set up to communicate with the datadaemon, one
 *	for reading commands, and one for sending back data.
 *	This program sends a request to the datadaemon and prints 
 *	appropriate results on stdout
 *
 * NOTES:
 *   Usage: bidata -b | -i -g stanza -f field [ -c field -v value] 
 *	    bidata -d field -v value
 *	    bidata -a (-l lcoation -s size -n name)
 *	    bidata -b | -i -m stanza -f field -e value [ -c field -v value]
 *	    bidata -w -x
 *	    bidata -S | -D | -G
 *           
 *	-a:  add a target disk to bosinst.dat. One or more of location, size,
 *		or name must be specified
 * 	-b:  manipulate bosinst.data file
 *	-d:  delete a target disk stanza from bosinst.data
 *	-i:  manipulate image.data file
 *	-w:  write both files out
 *      -x:  exit
 *	-g:  get a stanza (requires a field)
 *	-c:  optional criteria (requires a vlaue if used)
 *	-m:  modify a stanza (requires a field and value)
 *	-f:  field within a stanza
 *	-v:  value of a field (get criteria or modify)
 *      -S:  get the status of data check
 *      -D:  delete all target disk data stanzas (bosinst.data)
 *	-G:  get all target disk data stanza (bosinst.data)
 *
 *	Default values are read from bosinst.data and image.data.  
 *
 * RETURNS:
 *	requested data is printed on stdout
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include "bidata.h"

main(argc, argv)
int argc;
char *argv[];
{
    int request;		/* reuest pipe			*/
    int reply;			/* respones pipe		*/
    int c;			/* getopt return		*/
    int dobos = 0;		/* bosinstdata flag		*/
    int doimage = 0;		/* imagedata flag		*/
    int dowrite = 0;		/* write files flag		*/
    int doexit = 0;		/* exit flag			*/
    int doget = 0;		/* get a value flag		*/
    int domod = 0;		/* modify a field flag		*/
    int dodel = 0;		/* delete stanza flag		*/
    int doadd = 0;		/* add a disk flag		*/
    int doGet = 0;		/* get all target disk stanzas  */
    int doDel = 0;		/* Del all target disk stanzas  */
    int error = 0;		/* error flag			*/
    int docheck=0;		/* do status check		*/
    char *stanza = 0;		/* stanza  to work with		*/
    char *field = 0;		/* field to work with/overload disk name  */
    char *criteria = 0;		/* criteria field/overload disk size	  */
    char *value = 0;		/* criteria value/overload disk location  */
    char *newvalue=0;		/* modify value			*/

    struct bidata birequest;	/* data reuest strucutre	*/
    struct ddresponse response;	/* datadaemon response		*/


    /* clear the request buffer */
    bzero(&birequest, sizeof(struct bidata));

    /* parse the command arguments */
    while ((c = getopt(argc, argv, "biwxaSDGg:d:m:f:c:v:l:s:n:e:")) != EOF)
    {
        switch (c)
        {
	case 'b':		/* deal with bisinst.data */
	    if (doimage || dowrite || doexit)
		error++;
	    dobos++;
	    break;

	case 'i' :		/* deal with image.data */
	    if (dobos || dowrite || doexit)
		error++;
	    doimage++;
	    break;

	case 'w':		/* write out files */
	    if (dobos || doimage)
		error++;
	    dowrite++;
	    break;
	
	case 'x':		/* datadaemon exit */
	    if (dobos || doimage)
		error++;
	    doexit++;
	    break;

	case 'a':		/* add a target disk to bosinst.data */
	    if (dobos || doimage || dodel)
		error++;

	    doadd++;
	    break;

	case 'g':		/* get a value	*/
	    if (!(dobos || doimage || domod || dodel))
		error++;
	    doget++;
	    stanza = optarg;
	    break;
	
	case 'd':		/* delete a disk from bosinst.data */
	     if (dobos || doimage || domod || doget)
		 error++;
	    dodel++;
	    field = optarg;
	    break;

	case 'm':		/* modify (a) value(s) */
	     if (!(dobos || doimage) || dodel || doget)
		 error++;
	    domod++;
	    stanza = optarg;
	    break;
	
	case 'f':		/* field to deal with */
	    if (!(dobos || doimage || doget || domod))
		error++;
	    
	    field = optarg;
	    break;

	case 'c':		/* get/modify criteria field */
	    if (!(dobos || doimage || doget || domod))
		error++;
	    
	    criteria = optarg;
	    break;

	case 'v':		/* get/modify criteria value */
	    value = optarg;
	    break;

	case 'n':		/* add disk hdiskname	*/
	    if (!doadd)
		error++;

	    field = optarg;
	    break;

	case 's':		/* add disk size	*/
	    if (!doadd)
		error++;

	    criteria = optarg;
	    break;

	case 'l':		/* add disk location */
	    if (!doadd)
		error++;

	    value = optarg;
	    break;

	case 'e':		/* modify field new value */
	    if (!domod)
		error++;
	    
	    newvalue = optarg;
	    break;

	case 'S':		/* get status		*/
	    if (dobos || doimage || dowrite || doexit)
		error++;

	    docheck++;
	    break;

	case 'G':		/* Get all target disk stanzas  */
	    if (dobos || doimage || dowrite || doexit || docheck || doDel)
		error++;

	    doGet++;
	    break;

	case 'D':		/* Delete all target disk stanzas  */
	    if (dobos || doimage || dowrite || doexit || docheck || doGet)
		error++;

	    doDel++;
	    break;

	default:		/* oh, well... */
	    error++;
	}
    }
    if (error)
    {
	do_error();
	exit(1);
    }

    /* initialize type */
    birequest.type = 0;

    if (!dowrite && !doexit)
    {
	/* load the rest of the request */
	strcpy(birequest.stanza, stanza);
	strcpy(birequest.field, field);
    }

    /* figure out what needs to be done */
    if (dobos )
    {
	/* copy the criteria */
	if (criteria)
	{
	    strcpy(birequest.auxfield, criteria);
	    strcpy(birequest.auxvalue, value);
	}
        if (doget)
	{
	    birequest.type = GET_BOSINST_FIELD;

	}
	else if (domod)
	{
	    birequest.type = CHANGE_BOSINST_FIELD;

	    /* load the reuest buffer */
	    strcpy(birequest.value, newvalue);
	}
    }
    else if (doimage )
    {
	/* copy the criteria */
	if (criteria)
	{
	    strcpy(birequest.auxfield, criteria);
	    strcpy(birequest.auxvalue, value);
	}
	if (doget)
	{
	    birequest.type = GET_IMAGEDATA_FIELD;
	}
	else if (domod)
	{
	    birequest.type = CHANGE_IMAGEDATA_FIELD;

	    /* load the reuest buffer */
	    strcpy(birequest.value, newvalue);
	}
    }
    else if (dodel)
    {
	birequest.type = DELETE_BOSINST_DISK;
	strcpy(birequest.auxfield, field);
	strcpy(birequest.auxvalue, value);
	
    }
    else if (doadd)
    {
	birequest.type = ADD_BOSINST_DISK;
	if (field)
	    strcpy(birequest.field, field);
	if (criteria)
	    strcpy(birequest.auxfield, criteria);
	if (value)
	    strcpy(birequest.value, value);
    }
    else if (dowrite || doexit)
    {
	/* There's nothing left but exit and write.  OR the values together */
	if (dowrite)
	    birequest.type = WRITE_FILES;
	if (doexit)
	    birequest.type |= EXIT;
    }
    else if (docheck)
    {
	birequest.type = GET_STATUS;
    }
    else if (doGet)
    {
	birequest.type = GET_ALL_DISKS;
    }
    else if (doDel)
    {
	birequest.type = DEL_ALL_DISKS;
    }
    else
    {
	/* error */
	do_error(argc, argv);
	exit(1);
    }


    /* open the pipes */
    request = open(BIDATA_COMMAND, O_WRONLY);
    if (request < 0)
	exit(1);	/* baaaad! */

    reply = open(BIDATA_RESPONSE, O_RDONLY);
    if (reply < 0)
	exit(1);		/* baaad!  */


    /* send the request */
    write(request, &birequest, sizeof(struct bidata));

    /* get the reponse */
    read (reply,&response,  sizeof(struct ddresponse));

    /* if the return code is valid, print the response */
    if (response.rc == 0)
    {
	if (response.data[0] != '\0')
	    printf("%s\n", response.data);

	exit(0);
    }
    if (response.rc == 2)
	exit(0);

    /* exit bad status */
    exit(1);
}

do_error(int argc,char *argv[])
{
    int i;
    fprintf(stderr, "Got: ");
    for(i=0; i < argc; i++)
	fprintf(stderr, "%s ", argv[i]);

    fprintf(stderr, "\nUsage: bidata -b | -i -g stanza -f field [ -c field -v value]\n");
    fprintf(stderr,"bidata -b | -i -m stanza -f field -e value [ -c field -v value]\n");
    fprintf(stderr,"bidata -d field -v value\n");
    fprintf(stderr,"bidata -a (-l location -s size -n name)\n");
    fprintf(stderr,"bidata -w -x\n");
    fprintf(stderr,"bidata -S | -G | -D\n");
}

