static char sccsid[] = "@(#)50	1.21  src/bos/usr/bin/sdiff/sdiff.c, cmdfiles, bos411, 9428A410j 4/13/94 11:22:10";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: sdiff
 *
 * ORIGINS: 3, 27, 71
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
 * "@(#)$RCSfile: sdiff.c,v $ $Revision: 1.6.1.6 $ (OSF) $Date: 92/04/01 11:54:58 $";
 */
#define _ILS_MACROS
#include <sys/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "sdiff_msg.h"
nl_catd catd;
#define MSGSTR(N,S) catgets(catd,MS_SDIFF,N,S)

#define STDOUT 1
#define WGUTTER 6        /* width of gutter */
#define WLEN	(WGUTTER * 2 + WGUTTER + 2)
#define PROMPT '%'
#define TAB 8

char	GUTTER[WGUTTER] = "     ";
char	*DIFF	= "/usr/bin/diff -b ";
char	diffcmd[MAX_INPUT];
char	inbuf[10];

int	llen	= 130;		/* Default maximum line length written out */
int	hlen;		/* Half line length with space for gutter */
int	len1;		/* Calculated length of left side */
int	nchars;		/* Number of characters in left side - used for tab expansion */
char	change = ' ';
int	leftonly = 0;	/* if set print left side only for identical lines */
int	silent = 0;	/* if set do not print identical lines */
int	midflg = 0;	/* set after middle was output */
int	rcode = 0;	/* return code */

char *pgmname;

char	*file1;
FILE	*fdes1;
char	buf1[LINE_MAX+1];

char	*file2;
FILE	*fdes2;
char	buf2[LINE_MAX+1];

FILE	*diffdes;          /* difference file */
char	diffbuf[LINE_MAX+1];

int oflag;                /* output file */
char	*ofile;
FILE	*odes;

char	*ltemp;          /* temp left file */
FILE	*left;

char	*rtemp;          /* temp right file */
FILE	*right;

FILE *tempdes;           /* temp file */
char *temp;

            /* decoded diff cmd- left side from to; right side from, to */
int from1, to1, from2, to2;
int num1, num2;		/*line count for left side file and right */

int mb_cur_max, mbflag;

static char *filename(char *, char *);

/*
 * NAME:  sdiff [-l] [-s] [-w #] [-o output] file1 file2
 *
 * FUNCTION:  does side by side diff listing
 *	-l leftside only for identical lines
 *	-s silent; only print differences
 *	-w # width of output
 *	-o output  interactive creation of new output
 *
 *	commands to sdiff:
 *		s	silent; do not print identical lines
 *		v	turn off silent
 *		l	copy left side to output
 * 		r	copy right side to output
 *		e l	call ed with left side
 *		e r	call ed with right side
 *		e b	call ed with cat of left and right
 *		e	call ed with empty file
 *		q	exit from program
 *
 *	functions:
 *	cmd	decode diff commands
 *	put1	output left side
 *	put2	output right side
 *	putmid	output gutter
 *	putline	output n chars to indicated file
 *	getlen	calculate length of strings with tabs
 *	mdin	read and process interactive cmds
 *	pp	copy from file to file
 *	edit	call ed with file
 */
main(argc,argv)
int argc;
char	**argv;
{
	extern onintr(void);
	int com;
	int n1, n2, n;
	int c;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SDIFF, NL_CAT_LOCALE);

	mb_cur_max = MB_CUR_MAX;
	mbflag = mb_cur_max > 1;

	if (signal(SIGHUP,SIG_IGN)!=SIG_IGN)
		signal(SIGHUP,(void (*)(int))onintr);
	if (signal(SIGINT,SIG_IGN)!=SIG_IGN)
		signal(SIGINT,(void (*)(int))onintr);
	if (signal(SIGPIPE,SIG_IGN)!=SIG_IGN)
		signal(SIGPIPE,(void (*)(int))onintr);
	if (signal(SIGTERM,SIG_IGN)!=SIG_IGN)
		signal(SIGTERM,(void (*)(int))onintr);
	pgmname = argv[0];

	while( (c=getopt(argc,argv,"w:lso:")) != -1)
	  switch (c) {
	    case 'w':    /* width of output line */
			 /* -w# instead of -w # */
	      llen = atoi(optarg);
	      if(llen <= WLEN)
		error(MSGSTR(LLERR, "Wrong line length %s"),optarg);
	      if(llen > LINE_MAX)
		llen = LINE_MAX;
	      break;

	    case 'l': /* display only the left side when lines are same */
	      leftonly++;
	      break;

	    case 's': /* do not display identical lines */
	      silent++;
	      break;

	    case 'o':  /* get name of output file */
	      oflag++;
	      ofile = optarg;
	      break;
	    default:
	      fprintf(stderr,MSGSTR(USAGE,
		      "Usage: sdiff [-l | -s] [-o output] [-w number] file1 file2\n"));
	      exit(2);
	  }	

	if (((argc-optind) != 2) || (leftonly && silent)) {
	  fprintf(stderr,MSGSTR(USAGE,				/*MSG*/
		  "Usage: sdiff [-l | -s] [-o output] [-w number] file1 file2\n"));
	  exit(2);
        }

	file1 = argv[optind++];
	file2 = argv[optind++];

	file1=filename(file1,file2);   /* check files */
	file2=filename(file2,file1);
	hlen = (llen - WGUTTER +1)/2;   /* half line length */

	if((fdes1 = fopen(file1,"r")) == NULL)
		error(MSGSTR(OPERR,"Cannot open: %s"),file1);		/*MSG*/

	if((fdes2 = fopen(file2,"r")) == NULL)
		error(MSGSTR(OPERR,"Cannot open: %s"),file2);		/*MSG*/

	if(oflag){
		if(!temp)
			temp = tempnam("/tmp", "sdiff");
		ltemp = tempnam("/tmp", "sdiffl");
		if((left = fopen(ltemp,"w")) == NULL)
			error(MSGSTR(OPERRT,"Cannot open temp file %s"),ltemp);
		rtemp = tempnam("/tmp", "sdiffr");
		if((right = fopen(rtemp,"w")) == NULL)
			error(MSGSTR(OPERRT,"Cannot open temp file %s"),rtemp); /*MSG*/
		if((odes = fopen(ofile,"w")) == NULL)
			error(MSGSTR(OPERRO,"Cannot open output %s"),ofile); /*MSG*/
	}
	/* Call DIFF command */
	strcpy(diffcmd,DIFF);
	strcat(diffcmd,file1);
	strcat(diffcmd," ");
	strcat(diffcmd,file2);
	diffdes = popen(diffcmd,"r");

	num1 = num2 = 0;

	/* Read in diff output and decode commands
	*  "change" is used to determine character to put in gutter
	*  num1 and num2 counts the number of lines in file1 and 2
	*/

	n = 0;
	while(fgets(diffbuf,LINE_MAX+1,diffdes) != NULL){
		if (strlen(diffbuf) >= LINE_MAX)
			error(MSGSTR(LONGLINE, "Line in input file is too long."),NULL); /*MSG*/
		change = ' ';
		com = cmd(diffbuf);

	/* handles all diff output that is not cmd
	   lines starting with <, >, ., --- */
		if(com == 0)
			continue;

	/* Catch up to from1 and from2 */
		rcode = 1;
		n1=from1-num1;
		n2=from2-num2;
		n= n1>n2?n2:n1;
		if(com =='c' && n!=0)
			n--;
		while(n--){
			put1();      /* output left side */
			put2();      /* output right side */
			if(!silent)
				putc('\n',stdout);
			midflg = 0;
		}

	/* Process diff cmd */
		switch(com){

		case 'a':      /* add line(s) from file2 */
			change = '>';
			while(num2<to2){
				put2();      /* output right side */
				putc('\n',stdout);
				midflg = 0;
			}
			break;

		case 'd':                 /* add lines from file1 */
			change = '<';
			while(num1<to1){
				put1();      /* output left side */
				putc('\n',stdout);
				midflg = 0;
			}
			break;

		case 'c':        /* similiar lines in file1 and file2 */
			n1 = to1-from1;
			n2 = to2-from2;
			n = n1>n2?n2:n1;
			change = '|';
			do {
				put1();      /* output left side */
				put2();      /* output right side */
				putc('\n',stdout);
				midflg = 0;
			} while(n--);

			change = '<';
			while(num1<to1){
				put1();      /* output left side */
				putc('\n',stdout);
				midflg = 0;
			}

			change = '>';
			while(num2<to2){
				put2();      /* output right side */
				putc('\n',stdout);
				midflg = 0;
			}
			break;

		default:
			fprintf(stderr,MSGSTR(NOFNDC,"cmd not found %c\n"),com); /*MSG*/
			break;
		}

		if(oflag && com!=0){
			cmdin();    /* get and decode interactive command */
			if((left = fopen(ltemp,"w")) == NULL)
				error(MSGSTR(OPERRT,"Cannot open temp %s"),ltemp); /*MSG*/
			if((right = fopen(rtemp,"w")) == NULL)
				error(MSGSTR(OPERRT,"Cannot open temp %s"),rtemp); /*MSG*/
		}
	}
	/* put out remainder of input files */
	while(put1()){           /* output left side */
		put2();      /* output right side */
		if(!silent)
			putc('\n',stdout);
		midflg = 0;
	}
	if(odes)
		fclose(odes);
	removetmp();       /* remove temp files */
	exit(rcode);
}

/*
 * NAME: put1
 *
 * FUNCTION:  output left side
 */
put1()
{
	/* len1 = length of left side */
	/* nchars = num of chars including tabs */


	if(fgets(buf1,LINE_MAX+1,fdes1) != NULL){
		len1 = getlen(0,buf1);
		if((!silent || change != ' ') && len1 != 0)
			putline(stdout,buf1,nchars);
		
		if(oflag){
		/*put left side either to output file
		  if identical to right
		  or left temp file if not */

			if(change == ' ')
				putline(odes,buf1,strlen(buf1));
			else
				putline(left,buf1,strlen(buf1));
		}
		if(change != ' ')
			putmid(1);       /* put gutter out */
		num1++;
		return(1);
	} else
		return(0);
}

/*
 * NAME: put2
 *
 * FUNCTION:  output right side
 */
put2()
{

	if(fgets(buf2,LINE_MAX+1,fdes2) != NULL){
		getlen(0,buf2); 	

		/* if the left and right are different they are always
		   printed.
		   If the left and right are identical
		   right is only printed if leftonly is not specified
		   or silent mode is not specified
		   or the right contains other than white space (len1 !=0)
		*/
		if(change != ' '){
		
		/* put right side to right temp file only
		   because left side was written to output for
		   identical lines */

			if(oflag)
				putline(right,buf2,strlen(buf2));
			
			if(midflg == 0)
				putmid(1);
			putline(stdout,buf2,nchars);
		} else
			if(!silent && !leftonly && len1!=0) {
				if(midflg == 0)
					putmid(1);
				putline(stdout,buf2,nchars);
			}
		num2++;
		len1 = 0;
		return(1);
	} else {
		len1 = 0;
		return(0);
	}
}

/*
 * NAME: putline
 *
 * FUNCTION:  output num chars to indicated file
 */
putline(file,start,num)
FILE *file;
char *start;
int num;
{

	char *cp, *end;

	cp = start;
	end = cp + num;
	while(cp < end)
			putc(*cp++,file);
}

/*
 * NAME: cmd
 *
 * FUNCTION: decode diff output
 */
cmd(start)
char *start;
{

	char *cp, *cps;
	int com;

	if(*start == '>' || *start == '<' || *start == '-' || *start == '.')
		return(0);

	cp = cps = start;
	while(isdigit((int)*cp))
		cp++;
	from1 = atoi(cps);
	to1 = from1;
	if(*cp == ','){
		cp++;
		cps = cp;
		while(isdigit((int)*cp))
			cp++;
		to1 = atoi(cps);
	}

	com = *cp++;
	cps = cp;

	while(isdigit((int)*cp))
		cp++;
	from2 = atoi(cps);
	to2 = from2;
	if(*cp == ','){
		cp++;
		cps = cp;
		while(isdigit((int)*cp))
			cp++;
		to2 = atoi(cps);
	}
	return(com);
}

/*
 * NAME: getlen
 *
 * FUNCTION: calculate length of strings with tabs
 */
getlen(startpos,buffer)
char *buffer;
int startpos;
{
	/* get the length of the string in buffer
	*  expand tabs to next multiple of TAB
	*/

	char *cp;
	int len, slen, tlen, chlen;
	int notspace;
	wchar_t	wc;

	nchars = 0;
	notspace = 0;
	tlen = startpos;
	for(cp=buffer; *cp != '\n'; cp++){
		if(*cp == '\t'){
			slen = tlen;
			tlen += TAB - (tlen%TAB);
			if(tlen>=hlen) {
				tlen= slen;
				break;
			}
			nchars++;
		}else{
			if(tlen>=hlen)break;
			if(!isspace((int)*cp))
				notspace = 1;
                        if(mbflag)
                        {
				if ((chlen=mblen(cp, mb_cur_max)) > 0) {
					if (mbtowc(&wc,cp,mb_cur_max) > 0)
						/* fix D36998 */
						tlen += (len = wcwidth(wc)) == -1 ? 1 : len;
					if (tlen > hlen) { /* MB alignment */
						tlen -= len;
						break;
					}
					nchars += chlen;
					cp += (chlen - 1);
				}
                        } else {
				tlen++;
				nchars++;
			}
		}
	}
	return(notspace?tlen:0);
}

/*
 * NAME: putmid
 *
 * FUNCTION:  output gutter
 */
putmid(bflag)
int bflag;
{
	/* len1 set by getlen to the possibly truncated
	*  length of left side
	*  hlen is length of half line
	*/
	int numspace = (hlen - len1);
	midflg = 1;
	if(bflag) {
		 while(numspace--)
			putc(' ',stdout);
	}
	GUTTER[2] = change;
	putline(stdout,GUTTER,5);
}

/*
 * NAME: error
 *
 * FUNCTION: print error messages and exit program
 */
error(s1,s2)
char *s1, *s2;
{
	fprintf(stderr,"%s: ",pgmname);
	fprintf(stderr,s1,s2);
	putc('\n',stderr);
	removetmp();
	exit(2);
}

/*
 * NAME: onintr
 *
 * FUNCTION: on interrupt remove temp file and exit
 */
onintr(void)
{
	removetmp();
	exit(rcode);
}

/*
 * NAME: removetmp
 *
 * FUNCTION: remove temp files
 */
removetmp()
{
	if(ltemp)
		unlink(ltemp);
	if(rtemp)
		unlink(rtemp);
	if(temp)
		unlink(temp);
}

/*
 * NAME: cmdin
 *
 * FUNCTION: read and decode interactive commands
 */
cmdin()
{
	char *cp, *ename;
	int notacc;

	fclose(left);
	fclose(right);
	notacc = 1;
	while(notacc){
		putc(PROMPT,stdout);
		cp = fgets(inbuf,10,stdin);
		if (!cp) {
			removetmp();
			exit(rcode);
		}
		switch(*cp){

		case 's':     /* silent, do not print identical lines */
			silent = 1;
			break;

		case 'v':     /* turn off silent */
			silent = 0;
			break;

		case 'q':    /* quit */
			removetmp();
			exit(rcode);
			break;

		case 'l':   /* copy left side to output */
			cpp(ltemp,left,odes);
			notacc = 0;
			break;

		case 'r':   /* copy right side to output */
			cpp(rtemp,right,odes);
			notacc = 0;
			break;

		case 'e':   /* call ed */
			while(*++cp == ' ')
				;
			switch(*cp){
			case 'l':    /* with left file */
			case '<':
				notacc = 0;
				ename = ltemp;
				edit(ename);
				break;

			case 'r':     /* with right file */
			case '>':
				notacc = 0;
				ename = rtemp;
				edit(ename);
				break;

			case 'b':      /* with both files */
			case '|':
				if((tempdes = fopen(temp,"w")) == NULL)
					error(MSGSTR(OPERRT,"Cannot open temp file %s"),temp); /*MSG*/
				cpp(ltemp,left,tempdes); /* build temp file */
				cpp(rtemp,right,tempdes);
				fclose(tempdes);
				notacc = 0;
				ename = temp;
				edit(ename);
				break;

			case '\n':   /* edit empty file */
				if((tempdes=fopen(temp,"w")) == NULL)
					error(MSGSTR(OPERRT,"Cannot open temp %s"),temp); /*MSG*/
				fclose(tempdes);
				notacc = 0;
				ename = temp;
				edit(ename);
				break;
			default:
				fprintf(stderr,
					MSGSTR(ILLCMD,
					       "Unknown option %s to `e', reenter\n"),
                                        cp);

				break;
			}
			if(notacc == 0)
				cpp(ename,tempdes,odes);
			break;

		default:
fprintf(stderr,MSGSTR(ILLCMD,"Illegal command %s reenter\n"), cp); /*MSG*/
			break;
		}
	}
}

/*
 * NAME: cpp
 *
 * FUNCTION: copy from file to another file
 */
cpp(from,fromdes,todes)
char *from;
FILE *fromdes,*todes;
{
	char tempbuf[LINE_MAX+1];

	if((fromdes = fopen(from,"r")) == NULL)
		error(MSGSTR(OPERR,"Cannot open %s"),from);	/*MSG*/
	while((fgets(tempbuf,LINE_MAX+1,fromdes) != NULL))
		fputs(tempbuf,todes);
	fclose(fromdes);
}

/*
 * NAME: edit
 *
 * FUNCTION: edit a file using ed
 */
edit(file)
char *file;
{
	char	edcmd[LINE_MAX];

	sprintf(edcmd, "/usr/bin/ed %s", file);
	if (system(edcmd)) { /* fix D73243 */
		perror("sdiff");
		exit(2);
	}
}

/*
 * NAME: filename
 *
 * FUNCTION:  check input files
 */
static char *
filename(char *pa1, char *pa2)
{
	int c;
	char  *cp;
	struct stat stbuf;
	cp = pa1;
	if(stat(pa1,&stbuf)!=-1 && ((stbuf.st_mode&S_IFMT)==S_IFDIR)) {
		if ((cp = malloc(PATH_MAX+1)) == NULL) {
			perror("sdiff");
			exit(2);
		}
		*(cp+PATH_MAX) = '\0';
		(void) strncpy(cp,pa1,PATH_MAX);
		(void) strncat(cp,"/",PATH_MAX-strlen(cp));
		(void) strncat(cp,basename(pa2),PATH_MAX-strlen(cp));
	}
	else if(pa1[0] == '-' && pa1[1] == 0 && temp ==0) {
		cp = temp = tempnam("/tmp","sdiff");
		if((tempdes = fopen(temp,"w")) == NULL)
			error(MSGSTR(OPERRT,"Cannot open temp %s"),temp); /*MSG*/
		while((c=getc(stdin)) != EOF)
			putc(c,tempdes);
		fclose(tempdes);
	}
	return(cp);
}
