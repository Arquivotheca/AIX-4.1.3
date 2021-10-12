static  char sccsid[] = "@(#)96  1.8  src/bldenv/pkgtools/ccss_unpack.c, pkgtools, bos41J, 9513A_all 3/15/95 12:28:51";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: help
 *		main
 *		unpack_file
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
/*------------------------------------------------------------------------*/
/*                                                                        */
/*  Purpose:                                                              */
/*      This program extracts the ptf, infofile, or toc_entry             */
/*      from a ccss format envelop.  'bosinst' and                        */
/*      'bosboot' format envelops are also handled.                       */
/*                                                                        */
/*  Syntax:                                                               */
/*   ptfs/infofiles/toc_entrys:                                           */
/*    ccss_unpack  <-c fn1> <-p fn2> <-i fn3> <-t fn4> [-?|-h]            */
/*   bosboot:                                                             */
/*    ccss_unpack  <-c fn1> <-1 fn2>                                      */
/*   bosinst:                                                             */
/*    ccss_unpack  <-c fn1> <-2 fn2>                                      */
/*                                                                        */
/*  Flags:  Ref. help message                                             */
/*                                                                        */
/*------------------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>

/* argument file names */
char   *ptfname;
char   *tocname;
char   *ptfsize;
char   *infname;
char   *botname;
char   *bosname;
char   *ccsname;

/* flags for parm selected  */
int    ptf,toc,pts,inf,bot,bos,ccs,head;

/*  input file handler */
int inhand;

/* header input area       */
char    hdr[81];
char 	ascii_hdr[81];
char    hdr_name[81];
char    hdr_type[4];
int     hdr_size;

/* misc. vars */
int num_outputted = 0;
int num_inputted  = 0;
int num_chars;
char *comma_ptr;


/*  unpack input to appropriate output file */
unpack_file(outname,outsize)
char   *outname;
int     outsize;
{
    int     outhand,outhead,j;
    char    buf[512],out_head_name[81];
    int     outnum;

    outhand=open(outname,O_RDWR|O_CREAT|O_TRUNC);
    if (outhand==-1)
    {
	fprintf(stderr,
		"ccss_unpack:Can't open outfile file: %s; errno = '%d'\n",
		outname, errno);
	exit(3);
    }

    /* read, write permission: user, group and others */
    chmod(outname,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    if (head==1 && !(strcmp(outname, infname)))
    {
	strcpy(out_head_name, outname);
	strcat(out_head_name, ".head");
	outhead=open(out_head_name, O_RDWR|O_CREAT|O_TRUNC);
	if(outhead==-1)
	{
	    fprintf(stderr,
		    "ccss_unpack:Can't open outfile file: %s; errno = '%d'\n", 
		    out_head_name, errno);
	    exit(3);
	}
	chmod(outname,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	outnum = write(outhead,ascii_hdr,80);
	if ((outnum == -1) && (errno == ENOSPC))
	{
	    fprintf(stderr,
	     "\nccss_unpack: No free space is left on the filesystem for %s\n",
		   out_head_name);
	    exit (4);
	}
	else if (outnum != 80)
	{
	    fprintf (stderr,
		  "\nccss_unpack: Not enough space on the filesystem for %s\n",
		    outname);
	    exit (4);
	}
    }

    while (outsize >= 512)
    {
	if ((read(inhand,buf,512)) < 0)          /* read input file      */
	{
	    fprintf(stderr, 
		    "ccss_unpack: read failure from '%s'; errno = '%d'\n",
		    ccsname, errno);
	    exit(4);
	}
	outnum=write(outhand,buf,512);            /* write output file    */
	if ((outnum == -1) && (errno == ENOSPC))
	{
	    fprintf(stderr,
	     "\nccss_unpack: No free space is left on the filesystem for %s\n",
		   outname);
	    exit (4);
	}
	else if (outnum != 512)
	{
	    fprintf (stderr,
		  "\nccss_unpack: Not enough space on the filesystem for %s\n",
		    outname);
	    exit (4);
	}
	else
	{
	    outsize = outsize - 512;
	}
    }
    if (outsize > 0)
    {
	if ((read(inhand,buf,outsize)) < 0)
	{
	    fprintf(stderr, 
		    "ccss_unpack: read failure from '%s'; errno = '%d'\n",
		    ccsname, errno);
	    exit(4);
	}
	outnum=write(outhand,buf,outsize);     /* write output file    */
	if ((outnum == -1) && (errno == ENOSPC))
	{
	    fprintf(stderr,
	     "\nccss_unpack: No free space is left on the filesystem for %s\n",
		    outname);
	    exit (4);
	}
	else if  (outnum != outsize)
	{
	    fprintf(stderr,
		  "\nccss_unpack: Not enough space on the filesystem for %s\n",
		    outname);
	    exit (4);
	}
    }
    close(outhand);
    num_outputted++;
}

main(argcnt,argptrs)
int argcnt;
char *argptrs[];
{
extern  char  *optarg;
int    c;
FILE   *fout;

ptf=toc=pts=inf=bot=bos=ccs=head=0;  /*  set parm flags to zero */

/* parse the command line */
while(( c = getopt(argcnt , argptrs, "h?ap:t:s:i:n:1:2:c:")) != EOF)
{
    switch(c)
    {
      case 'h':
      case '?':help();exit(1);
      case 'p':ptf=1;ptfname = optarg;break;
      case 't':toc=1;tocname = optarg;break;
      case 's':pts=1;ptfsize = optarg;break;
      case 'i':inf=1;infname = optarg;break;
      case '1':bot=1;botname = optarg;break;
      case '2':bos=1;bosname = optarg;break;
      case 'c':ccs=1;ccsname = optarg;break;
      case 'a':head=1;break;
      default :help();exit(1);
    }
}

    /* validate the parameters   */
    /*  check that ccss_file was given */
    if (ccs==0)
    {
	fprintf(stderr, "ccss_unpack: -c parameter is required\n");
	help();
	exit(2);
    }

    /*  check that at least 1 file was given to unpack */
    if ((ptf==0)&&(toc==0)&&(inf==0)&&(pts==0)&&(bot==0)&&(bos==0))
    {
	fprintf(stderr, "ccss_unpack: no output file given to unpack\n");
	help();
	exit(2);
    }

    /*  check for bad combination of ptf and (bot or bos) parameters */
    if (((toc==1)||(inf==1)||(ptf==1)||(pts==1)) && ((bos==1)||(bot==1)) )
    {
	fprintf(stderr, 
		"ccss_unpack: -p,-t,-s, or -i can't be used with -1 or -2\n");
	help();
	exit(2);
    }

    /*  check for bad combination of bos and bot parameters */
    if ((bot==1)&&(bos==1))
    {
	fprintf(stderr, "ccss_unpack: -1 and -2 can't be used together\n");
	help();
	exit(2);
    }


    /* read ccss_file and write to appropriate output files */
    inhand = open(ccsname,O_RDONLY);   /* open input file  */
    if (inhand==-1)
    {
	fprintf(stderr, 
		"ccss_unpack: unable to open input file: %s;errno = '%d'\n",
		ccsname, errno);
	exit(4);
    }

    /* set up number of parms inputted */
    num_inputted = ptf + toc + pts + inf + bos + bot;

    while (num_inputted != num_outputted)
    {
	if(read(inhand,hdr,80) != 80)        /*  read header */
	{  
	    fprintf(stderr,
		    "ccss_unpack:ccss_file doesn't contain specified files\n");
	    exit(4);
	}	
	NLxin(hdr,hdr,80);                /*  convert ebcdic to ascii */
	hdr[80]='\0';
	strcpy(ascii_hdr,hdr);

	/*  change commas to blanks so the subsequent sscanf
	    recognizes white space to delimit items in the header */
	while( (comma_ptr = strrchr(hdr,',')) != NULL )
	{
	    comma_ptr[0] = ' ';
	}

	/*   parse header  */
	if (sscanf(hdr,";; NAME=%s TYPE=%s SIZE=%d",
		   hdr_name,hdr_type,&hdr_size) != 3)   /* must find 3 items */
	{
	    fprintf(stderr,
		    "ccss_unpack:invalid header found in %s:\n  %s\n",
		    ccsname, hdr);
	    exit(4);
	}	

	/*  output ptf_size */
	if ((pts==1)&&(strcmp(hdr_type,"RS6")==0))
	{
	    if ( (fout = fopen (ptfsize, "w+")) == NULL )
	    {
		fprintf(stderr,
		     "ccss_unpack:Can't open outfile file: %s; errno = '%d'\n",
			ptfsize, errno);
                exit(5);
	    }
	    else             /* read, write permission: user, group, others */
	    {
		chmod(ptfsize,S_IRUSR|S_IWUSR|S_IROTH|S_IRGRP);
                fprintf (fout, "%d\n", hdr_size);
                fclose (fout);
                num_outputted++;
	    }
	}

	/*  output to appropriate file */
	if ((toc==1)&&(strcmp(hdr_type,"R6T")==0))
	    unpack_file(tocname,hdr_size);
	else if ((ptf==1)&&(strcmp(hdr_type,"RS6")==0))
	    unpack_file(ptfname,hdr_size);
	else if ((inf==1)&&(strcmp(hdr_type,"INF")==0))
	    unpack_file(infname,hdr_size);
	else if ((bos==1)&&(strcmp(hdr_type,"PGM")==0)
		 &&(strcmp(hdr_name,"RS6INST")==0))
	    unpack_file(bosname,hdr_size);
	else if ((bot==1)&&(strcmp(hdr_type,"PGM")==0)
		 &&(strcmp(hdr_name,"RS6BOOT")==0))
	    unpack_file(botname,hdr_size);
	else if ( num_inputted != num_outputted ) /* only lseek if not done */
	    lseek(inhand,hdr_size,1);  /*  skip over if file not wanted */

    } /* end of while */

    close(inhand);
    return(0);
}  /*  end of main */



help()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "ccss_unpack  -c ccss_filename [-h] [-?] [-a]   \n");
    fprintf(stderr, "     {  [-p ptf_filename -i infofile -t toc_fragment ");
    fprintf(stderr, "-s ptfsize_filename] \n");
    fprintf(stderr, 
	    "        [ -1 boot_filename ]  [ -2 bosinst_filename ]  } \n\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "ccss_unpack -c ccssfile -p ptf -t toc -i inf \n");
    fprintf(stderr, "ccss_unpack -c ccssfile -i info \n");
    fprintf(stderr, "ccss_unpack -c ccssfile -s pts \n");
    fprintf(stderr, "ccss_unpack -c ccssfile -1 boot\n");
    fprintf(stderr, "ccss_unpack -c ccssfile -2 bos\n\n");
}
/* END OF PROGRAM */
