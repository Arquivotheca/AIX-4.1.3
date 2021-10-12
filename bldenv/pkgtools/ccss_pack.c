static  char sccsid[] = "@(#)97  1.10  src/bldenv/pkgtools/ccss_pack.c, pkgtools, bos41J, 9513A_all 3/15/95 12:28:41";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: help
 *		main
 *		pack_file
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/* argument file names */
char   *ptfname;
char   *tocname;
char   *infname;
char   *ptnname;
char   *botname;
char   *bosname;
char   *ccsname;
char   *classvalue;

/* flags for parm selected  */
int    ptf,toc,inf,ptn,bot,bos,ccs,hdr;

/* output file handle      */
int     ccshand;

/* header record        */
char    header1[] =";; NAME=";
char    header1a[] =",TYPE=RS6,SIZE=";
char    header1b[] =",FID=PTFDOC,CLASS";
char    header2[] =";; NAME=";
char    header2a[] =",TYPE=R6T,SIZE=";
char    header3[] =";; NAME=";
char    header3a[] =",TYPE=INF,SIZE=";
char    header3b[] =",FID=PTFDOC,CLASS";
char    header4[] =";; NAME=RS6BOOT,TYPE=PGM,SIZE=";
char    header5[] =";; NAME=RS6INST,TYPE=PGM,SIZE=";

/* output area          */
char    out80[81];
char    spaces[81] = "                                                                               ";

main(argcnt,argptrs)
int argcnt;
char *argptrs[];
{
    extern  char  *optarg;
    int    c;

    ptf=toc=inf=ptn=bos=ccs=hdr=0;  /*  set parm flags to zero */
    /* parse the command line */
    while ((c = getopt(argcnt , argptrs, "h?H:p:t:i:n:1:2:c:")) != EOF)
    {
	switch(c)
	{
	  case 'h':
	  case '?':help();exit(1);
	  case 'H':hdr=1;classvalue = optarg;break;
	  case 'p':ptf=1;ptfname = optarg;break;
	  case 't':toc=1;tocname = optarg;break;
	  case 'i':inf=1;infname = optarg;break;
	  case 'n':ptn=1;ptnname = optarg;break;
	  case '1':bot=1;botname = optarg;break;
	  case '2':bos=1;bosname = optarg;break;
	  case 'c':ccs=1;ccsname = optarg;break;
	  default:help();exit(1);
	}
    }

    /* validate the parameters   */
    /*  check if ccssfile was given */
    if (ccs==0)
    {
	fprintf(stderr, "ccss_pack: -c parameter is required\n");
	help();
	exit(2);
    }

    /* check that either none or all of the 
     *  Normal fix parameters were entered */
    if (!((ptf+toc+inf+ptn == 0)||(ptf+toc+inf+ptn == 4)))
    {	
	fprintf(stderr, 
	       "ccss_pack: -p,-i,-t,-n parameters must occur together\n");
        help();
	exit(2);
    }

    /* check that exactly 1 of Normal Fix, Bosboot, and BOS Install parameters
     *  were entered           */
    if (ptf+bot+bos != 1)
    {
	fprintf(stderr,
	   "ccss_pack: exactly one of (-p,-i,-t,-n),-1,-2 must be entered\n");
	help();
	exit(2);
    }


    /*  open ccss_file  */
    /*  0600 is user r/w permission */
    ccshand=open(ccsname,O_RDWR|O_CREAT|O_TRUNC,0600);
    if (ccshand == -1)
    {
	fprintf(stderr, "ccss_pack: can't open ccss_file: %s; errno = '%d'\n",
	       ccsname, errno);
	exit(3);
    }


    /*  Output Normal fix Package */
    if (ptf==1)
    {
	pack_file(header2,header2a,tocname);
	pack_file(header3,header3a,infname);
	pack_file(header1,header1a,ptfname);
    }

    /*  Output Bosboot Package */
    if (bot==1)
	pack_file(header4,header4,botname);

    /*  Output Bos Install Package */
    if (bos==1)
	pack_file(header5,header5,bosname);

    /*  Close ccss_file and return 0 */
    close(ccshand);
    return(0);
}  /*  end of main */


/*  translate to headers to ebcdic, write out headers, write out input file */
pack_file(outh1,outh2,inname)
char   *outh1;
char   *outh2;
char   *inname;
{
    int    inhand,inlen,j,num_spaces;
    int	   outCnt;
    char   inlenc[14];
    char   buf[512];

    inhand = open(inname,O_RDONLY);          /* open input file     */
    if (inhand == -1)
    {
	fprintf(stderr, "ccss_pack:can't open input file: %s; errno = '%d'\n",
		inname, errno);
	exit(4);
    }
    inlen = lseek(inhand,0,2);            /* find length of file      */
    close(inhand);                       /* close input file         */

    if (ptf==1)                         /*  build ptf header record */
    {
        if (hdr==1 && !strcmp(inname, infname)) 
	    sprintf(out80,"%s%s%s%d%s%s",
		    outh1,ptnname,outh2,inlen,header3b,classvalue);
        else if (hdr==1 && !strcmp(inname, ptfname)) 
	    sprintf(out80,"%s%s%s%d%s%s",
		    outh1,ptnname,outh2,inlen,header1b,classvalue);
        else
	    sprintf(out80,"%s%s%s%d",outh1,ptnname,outh2,inlen);

        num_spaces= 80 - strlen(out80);
        strncat(out80,spaces,num_spaces);
    }

    if ((bot==1)||(bos==1))           /*  build bos/bot header record */
    {
	sprintf(out80,"%s%d",outh1,inlen);
        num_spaces= 80 - strlen(out80);
        strncat(out80,spaces,num_spaces);
    }

    NLxout(out80,out80,80);              	/* convert header to ebcdic */
    outCnt = write(ccshand,out80,80);	 	/* output header record     */
    if ((outCnt == -1) && (errno == ENOSPC))
    {
	fprintf(stderr,
	       "\nccss_pack: No free space is left on the filesystem for %s\n",
		   ccsname);
	exit(6);
    }
    else if (outCnt != 80)
    {
	fprintf(stderr,
		"\nccss_pack: Could not complete write to '%s';errno = '%d'\n",
		ccsname, errno);
	fprintf(stderr, "    Check for space on the filesystem\n");
	exit(6);
    }

    if ((inhand = open(inname,O_RDONLY)) == -1)
    {
	fprintf(stderr, "\nccss_pack: Could not open 'inname'; errno = '%d'\n",
		errno);
	exit(4);
    }
    j = 512;
    while (j > 0)
    {
	j = read(inhand,buf,512);        /* read input file          */
	if (j < 0)
	{
	    fprintf(stderr, 
		    "ccss_pack: Error reading input file '%s'; errno = '%d'\n",
		    inname, errno);
	    exit(6);
	}
	outCnt = write(ccshand,buf,j);
	if ((outCnt == -1) && (errno == ENOSPC))
	{
	    fprintf(stderr,
	       "\nccss_pack: No free space is left on the filesystem for %s\n",
		    ccsname);
	    exit(6);
	}
	else if (outCnt != j)
	{
	    fprintf(stderr,
		"\nccss_pack: Could not complete write to '%s';errno = '%d'\n",
		    ccsname, errno);
	    fprintf(stderr, "    Check for space on the filesystem\n");
	    exit(6);
	}
    }
    close(inhand);                    	 /* close input file         */
    return(0);
}


help()
{
    fprintf(stderr, "%s\n",sccsid);
    fprintf(stderr, "Usage:\n");
    fprintf(stderr,
        "ccss_pack [ -p ptf_filename -t toc_fragment -i infofile -n ptf# ]\n");
    fprintf(stderr,
	    "          [ -1 boot_filename ]     [ -2 bosinst_filename ]\n");
    fprintf(stderr, "          -c ccss_filename   [-h]  [-?]\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "ccss_pack -p ptf -t toc -i inf -n ptf# -c ccssfile\n");
    fprintf(stderr, "ccss_pack -1 bot -c ccssfile\n");
    fprintf(stderr, "ccss_pack -2 bos -c ccssfile\n\n");
}
/* END OF PROGRAM */
