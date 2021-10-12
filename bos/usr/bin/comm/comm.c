static char sccsid[] = "@(#)81	1.13  src/bos/usr/bin/comm/comm.c, cmdfiles, bos412, 9446C 11/14/94 16:47:38";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: comm
 *
 * ORIGINS: 3, 18, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 *
 *
 *	process common lines of two files
 */


#include <stdio.h>
#include <locale.h>
#include <sys/limits.h>
#include <ctype.h>
#include "comm_msg.h"
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_COMM,Num,Str)

static int	one;       /* display flags */
static int	two;
static int	three;

static char	*ldr[3];    /* display spacing holder */

static FILE	*ib1;         /* pointer to file1 */
static FILE	*ib2;         /* pointer to file2 */
static FILE	*openfil();

static int	stdin_flag = FALSE;	/* flag to show that stdin is being used as
			   	a parameter */

/*
 * NAME: comm [-123] file1 file2
 *                                                                    
 * FUNCTION: Selects or rejects lines common to two sorted files.
 *           If you specify - for one of the file names, comm
 *           reads standard in.
 *           comm reads file1 and file2 and writes by default, a three-
 *           column ouput to standard output.
 *           column              output
 *             1           lines that are in only file1
 *             2           lines that are in only file2
 *             3           lines common to both files
 *           flags:
 *            -1           suppresses display of first column
 *            -2           suppresses display of second column
 *            -3           suppresses display of third column
 *
 * NOTE:  Both file1 and file2 should be sorted according to the collating
 *         sequence specified by the environment variable LC_COLLATE
 */  
main(argc,argv)
char **argv;
{
	int	l;             /* index for display spacing */
	char	lb1[MAX_INPUT],lb2[MAX_INPUT];   /* input buffers */
	int	oc,badopt;
	int	strcollresult;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_COMM, NL_CAT_LOCALE);

	ldr[0] = "";            /* display spacing: column 1 */
	ldr[1] = "\t";                           /* column 2 */
	ldr[2] = "\t\t";                         /* column 3 */
	badopt = 0;
	l = 1;
	while ((oc = getopt(argc,argv,"123")) != -1 ) {
		switch((unsigned char)oc) {
		case '1':
			if(!one) {
				one = 1;
				ldr[2] = ldr[l--];
				ldr[1] = ldr[0];
			}
			break;
		case '2':
			if(!two) {
				two = 1;
				ldr[2] = ldr[l--];
			}
			break;
		case '3':
			three = 1;
			break;
		default:
			badopt++;	/* Bad option or option syntax */
		} /* switch(oc) */
	} /* while((oc=getopt...)!=-1) */
	if (badopt || argc-optind != 2)
		usage();
	ib1 = openfil(argv[optind]);         /* open file1 an file2 */
	ib2 = openfil(argv[optind+1]);

	if(rd(ib1,lb1) < 0) {         /* if read line from file1 is bad */
		if(rd(ib2,lb2) < 0)   /* if bad read line from file2 */
			exit(1);      /* exit program */
		copy(ib2,lb2,2);      /* else copy rest file2 to screen */
	}                         /* else  */
	if(rd(ib2,lb2) < 0)        /* if read from file 2 bad */
		copy(ib1, lb1, 1);    /* copy rest of file1 to screen */
	while(1) {              /* else */
		strcollresult = ( (strcollresult = strcoll(lb1,lb2)) == 0 ? strcollresult :
					(strcollresult < 0 ? 1 : 2));
		switch(strcollresult) {
			case 0:        /* lines equal */
				wr(lb1,3);    /* write in column 3 */
				if(rd(ib1,lb1) < 0) {  /* get next line file1 */
					if(rd(ib2,lb2) < 0)
						exit(0);
					copy(ib2,lb2,2);
				}
				if(rd(ib2,lb2) < 0)   /* get next line file2 */
					copy(ib1, lb1, 1);
				continue;

			case 1:    /* line1 less than line2 */
				wr(lb1,1);   /* write in column 1 */
				if(rd(ib1,lb1) < 0)  /* get next line file1 */
					copy(ib2, lb2, 2);
				continue;

			case 2:     /* line1 greater that line 2 */
				wr(lb2,2);       /* write in column 2 */
				if(rd(ib2,lb2) < 0) /* get next line file2 */
					copy(ib1, lb1, 1);
				continue;
		}
	}
}

/*
 * NAME: rd
 *                                                                    
 * FUNCTION:  read a line from a file.
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *          0 - successfully reading a line.
 *         -1 - error.
 */  
static
rd(file,buf)
FILE *file;
char *buf;
{

	int i, j;
	i = j = 0;

	while((j = getc(file)) != EOF) {  /* read character by character */
		*buf = j;
		if(*buf == '\n' || i > MAX_INPUT-2) {
			*buf = '\0';
			return(0);
		}
		i++;
		buf++;
	}
	return(-1);
}

/*
 * NAME: wr
 *                                                                    
 * FUNCTION: write string to standarded out in the proper column.
 */  
static
wr(str,n)
char *str;
{
	switch(n) {
		case 1:          /* check display flags */
			if(one)
				return;
			break;

		case 2:
			if(two)
				return;
			break;

		case 3:
			if(three)
				return;
	}                           /* if display flag not set then print */
	printf("%s%s\n",ldr[n-1],str); /* using display spacing characters */
}

/*
 * NAME: copy
 *                                                                    
 * FUNCTION: copy the rest of file to standard out in the proper column
 *           then exit program.
 */  
static
copy(ibuf,lbuf,n)
FILE *ibuf;
char *lbuf;
{
	do {
		wr(lbuf,n);
	} while(rd(ibuf,lbuf) >= 0);

	exit(0);
}

/*
 * NAME: openfil
 *                                                                    
 * FUNCTION:  open a file or use stdin
 *
 * RETURN VALUE DESCRIPTION:  returns a file pointer.
 */  
static
FILE *openfil(s)
char *s;
{
	FILE *b;

	/* If the file name pointed to by s is the dash */
	if(s[0]=='-' && s[1]==0) {
		if (stdin_flag) {
			fprintf(stderr,MSGSTR(BADFILES,"comm: file1 and file2 can not both refer to stdin\n"));
			usage();
		}
		b = stdin;
		stdin_flag = TRUE;
	}
	else if((b=fopen(s,"r")) == NULL) {
		fprintf(stderr,MSGSTR(CANTOPEN,"comm: cannot open %s\n"),s);
		exit(1);
	}
	return(b);
}

/*
 * NAME: usage
 *                                                                    
 * FUNCTION:  displays the correct usage for comm.
 */  
static
usage()
{
	fprintf(stderr,MSGSTR(USAGE,"usage: comm [-123] file1 file2\n")); 
	exit(2);
}
