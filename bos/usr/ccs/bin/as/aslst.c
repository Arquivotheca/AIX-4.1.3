static char sccsid[] = "@(#)78	1.13.1.8  src/bos/usr/ccs/bin/as/aslst.c, cmdas, bos411, 9428A410j 12/3/93 12:27:25";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: check_zero, cleanlst, collectlst, displayval, dmp_ftable,
 *  	      initlst, initlst2, initxcross, linecount, newlinelst,
 *	      print_line, print_listheader, printobjcode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "as.h"
#include <string.h>
#include <time.h>		/* to pick current date up */
#include "as0.h"

#define BUFSIZE	 85
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )
#define SPEC_SET -1		/* indicates set pseduo-up */
#define SPEC_DOT -2		/* indicates dots */
#define MAXSIZE  100
#define MAXSIZELIMIT  MAXSIZE-1
#define MAXLEN	 85		/* maximum file name lenght with path */

extern char *outfile;		/* output file name */

static char listfile[MAXLEN];	/* listfile name full path */
static char hdr_srcfile[50];	/* name to print in header */
static int printline;		/* flag for printing line */
static int locounter[MAXSIZE];	/* location counter array */
static unsigned int objcode[MAXSIZE];	/* object code array */
static char sname[MAXSIZE][10];/* csect name to be tagged to location counter*/

static int collectptr;		/* collection pointer */
static int lenarray[MAXSIZE];	/* charlen byte sizes in collect */
static int zero_flag = 0;

static int shift[4] = { 0, 8, 16, 24 };	/* # bits per byte to shift */
static int keep[4] = { 0, 0xff, 0xffff, 0xffffff }; /* # of bytes to keep */
struct tm *date;		   /* structure in time.h for date */
char nls_buf[NLTBMAX];		/* NLDATE (environment format) time buffer */
unsigned char _outbuf[BUFSIZ+8];	/* for list file */
char linebuf[MAXLINE+1];		/* array buffer that keeps source */
				/* lines on 1st pass */
char *linebuf_ptr=linebuf;	/* ptr into linebuf */

/***************************************************************
  Function: initxcross
  Purpose:	this routine will open xcrossfile and print a header
  Input:      xcrosslist  -     user specified xcross file name 
              hdr_file    -     user specified file for header 
******************************************************************/
initxcross(xcrosslist, hdr_file)
	char **xcrosslist;		/* user specified xcross file name */
	char *hdr_file;			/* user specified file for header */
{
	char pathbuff[80];		/* current dir path  */
	int i;
        char xcrossfile[MAXLEN];	/* xcrossfile name full path */
	char *file;
        char *malloc();
        char *p;                   

		/* setup hdr_srcfile to reflect filename/userdef name */
	if (hdr_file == NULL)
		strcpy(hdr_srcfile, infile);
	else
		strcpy(hdr_srcfile, hdr_file);
	if ( !*xcrosslist ) {
		/* construct xcrosslist file name from infile */

					    /* generate filename */
		if (!infile)
			file=outfile;
		else {
			i=strlen(infile);
			for(file= infile + i; *file != '/' && file != infile;)
				file--;	
		}
		 		 	    /* generate path */
		getcwd(pathbuff, BUFSIZE);	/* get current dir. path */
		strcpy(xcrossfile,pathbuff);
		strcat(xcrossfile,"/");
					  /*add filename to path for listname*/
		if(p=strrchr(file,'.'))   /* strip . extension from filename */
		     strncat(xcrossfile, file, (p-file));
		else strcat(xcrossfile, file);
		strcat(xcrossfile,MESSAGE( 119 ));

                if (!(*xcrosslist = malloc (sizeof(xcrossfile)))) {
                                                /* modified message 073 */
           	yyerror( 73 );
	   	delexit();
           	}
		strcpy(*xcrosslist,xcrossfile);

		if ( (xoutfil=fopen(xcrossfile, "w+")) == NULL ) {
                                                /* message 038 */
			yyerror( 38 , xcrossfile);
			exit(2); }
	}
	else  {				/* if xcrosslist file existed */
		if( (xoutfil=fopen(*xcrosslist, "w+")) == NULL ) {
                                                /* message 038 */
			yyerror( 38 , xcrosslist);
			exit(2); }
	}
		fprintf(xoutfil, 
" SYMBOL          FILE            CSECT                   LINENO        \n");
}

/***************************************************************
  Function:  initlst
  Purpose:   this routine will set stdin to source file, open listfile and
	print the header 
  input:      listing  -   user specified listing file name
              hdr_file -   user specified file for header 
******************************************************************/
initlst(listing, hdr_file)
	char *listing;			/* user specified listing file name */
	char *hdr_file;			/* user specified file for header */
{
	char pathbuff[80];		/* current dir path  */
	int i;
	char *file;
        char *p;                   

	setbuf(stdin, _outbuf); 
		/* setup hdr_srcfile to reflect filename/userdef name */
	if (hdr_file == NULL) 
		strcpy(hdr_srcfile, infile);
	else
		strcpy(hdr_srcfile, hdr_file);
	if ( !listing ) {
		/* construct list file name from infile */

					    /* generate filename */
		if (!infile)
			file=outfile;
		else {
			i=strlen(infile);
			for(file= infile + i; *file != '/' && file != infile;)
				file--;	
		}
		 		 	  /* generate path */
		getcwd(pathbuff, BUFSIZE);	/* get current dir. path */
		strcpy(listfile,pathbuff);
		strcat(listfile,"/");
					  /*add filename to path for listname*/
		if(p=strrchr(file,'.'))   /* strip . extension from filename */
		     strncat(listfile, file, (p-file));
		else strcat(listfile, file);
		strcat(listfile,MESSAGE( 120 ));

		if ( (listfil=fopen(listfile, "w+")) == NULL ) {
                                                /* message 038 */
			yyerror( 38 , listfile);
			exit(2); }
	}
	else  {				/* if listfile was not NULL */
		strcpy(listfile, listing);
		if( (listfil=fopen(listfile, "w+")) == NULL ) {
                                                /* message 038 */
			yyerror( 38 , listfile);
			exit(2); }
	}
	print_listheader();
	collectptr=0;
}

/******************************************************************
    Function:  initlst2
    Purpose:	 This routine will do the initilization before pass 2
                 It attaches a pre-opened stream  associated with stdin
                 to the temp. file which was written in Pass 1 with
                 the source text. It also opens the listing file.
    input:      tmpin -  pointer to the temp. file name.
******************************************************************/
initlst2(tmpin)
char *tmpin;
{
	if( (freopen(tmpin, "r", stdin)) == NULL ) {
                                                /* message 045 */
		yyerror( 45 , tmpin);
		exit(2);
	}  
	setbuf(stdin, _outbuf); 
	if (!(listfil=fopen(listfile, "w+"))) {
                                                /* message 038 */
		yyerror( 38 , listfile);
		delexit();
	}
	print_listheader();	/* prints the listing header */
	collectptr=0;		/* initilize the counter into the array */
}	

/*****************************************************************
   Function:  print_listheader
   Purpose:   This routine will print the header for a listing file for
	      pass 1 or pass 2
   input:     None
*********************************************************************/
print_listheader()
{
	int k,c;
	unsigned long hey;		/* tm return on time */
	char *nlsdate_form;

	/* start printing header but first print last 
		20 char. of source name including path */
	if ( (k=strlen(hdr_srcfile)) > 20 )	 /* 20 characters only */
	{	k = k-20;
		fprintf(listfil, MESSAGE( 122 ),hdr_srcfile);      
	}	/* print full file name */

	else 
		fprintf(listfil, MESSAGE( 123 ), hdr_srcfile);
	time((long *)&hey);
	date=localtime((long *)&hey);	        
	if ((nlsdate_form = nl_langinfo(D_FMT)) == (char *)0)
		nlsdate_form = "%D";
	if(strftime(nls_buf,NLTBMAX, nlsdate_form, date))
		fprintf(listfil, "%s                %s\n",
			verid, nls_buf);
	else
		fprintf(listfil, "%s                %.2d/%.2d/%.2d\n",
			verid, date->tm_mon+1,date->tm_mday,date->tm_year);
        if ( mn_xref ) {
           if ( asm_mode_type == POWER) 
              fprintf(listfil, MESSAGE ( 181) );   
           else 
              fprintf(listfil, MESSAGE (180));
        } else
	   fprintf(listfil, MESSAGE( 179 ));
}

/*******************************************************************
   Function: collectlst
   Purpose:  This routine will print the instruction listings one line 
	at the time.
   input:
	 csectname -    the name of the csect for a source seg. 
	 loctr -        location counter 
	 hexcode -      object code generated by each instruction 
	 charlen -      specifies 1 byte, half word or full word 
*********************************************************************/
collectlst(csectname, loctr, hexcode, charlen)
char* csectname;		/* the name of the csect for a source seg. */
int	loctr;			/* location counter */ 
unsigned int hexcode;		/* object code generated by each instruction */
int 	charlen;		/* specifies 1 byte, half word or full word */
{
	int prev;		/* previous count */
	int need,bytes_used;	/* number of bytes to be packed, bytes needed*/
	int bytes_kept, bytes_shift;
	prev=collectptr - 1;	/* the previous array index */

			/* if too large to print, print dots */
     if (collectptr > MAXSIZELIMIT) {
	locounter[MAXSIZELIMIT-1] = SPEC_DOT;
	locounter[MAXSIZELIMIT] = loctr;
	return;
	}

	/* merge hex codes into single 4 byte hexcode if possible */

     if(collectptr > 0 && !strcmp(csectname,sname[prev]) && 
	        locounter[prev] != SPEC_SET && lenarray[prev] < 4 &&
		                  (loctr - locounter[prev]) < 4 ) {
	   need = 4 - lenarray[prev];	/* number of bytes needed to make a
					     full word in the last collect */
	   bytes_shift = need - charlen;	/* # of bytes to be shifted */
	   bytes_used = min(need, charlen);	/* # bytes used to packinsert*/
	   if ( bytes_shift < 0 )	  /* no bytes to be inserted */
		objcode[prev] |= (hexcode >> shift[-bytes_shift]);
	   else				/* bytes to be inserted */
		objcode[prev] |= (hexcode << shift[bytes_shift]);

	   lenarray[prev] += bytes_used;
	   if ( charlen >= need )	/* check if hexcode recieved is zero */
		check_zero(objcode[prev]);
	   bytes_kept = charlen - bytes_used;	/* bytes left for other lines*/
	   hexcode &= keep[bytes_kept];
	   charlen -= bytes_used;
	   loctr += bytes_used;	
	}
				/* create a new line in listing */
     if ( charlen > 0 ) {
	 /*  strncpy(sname[collectptr], csectname, 6);	 section name */
	   strcpy(sname[collectptr], csectname);
	   locounter[collectptr]=loctr;		/* location counter */
	   switch(charlen) {
		case 4: objcode[collectptr] = hexcode; break;
		case 3: objcode[collectptr] = hexcode << 8; break;
		case 2: objcode[collectptr] = hexcode << 16; break;
		case 1: objcode[collectptr] = hexcode << 24; break;  }
	   /* objcode[collectptr] = hexcode << shift[4 - charlen]; */
	   lenarray[collectptr]=charlen; 	/* number of bytes to print */
	   collectptr++;			/* collection ptr index */
	   if ( charlen == 4 && passno == 2)
		check_zero(hexcode);
	}
}

/************************************************
   Function:   displayval
   Purpose:    display the value defined by the .set pseudo-op 
               in the listing file.
	       this routine is called only on .set at this time
   input:     hexcode  -  the value defined by .set   
********************************************************/
displayval(hexcode) int hexcode;
{
	objcode[collectptr]=hexcode;
	locounter[collectptr]= SPEC_SET; /* set display flag for newline call*/
	lenarray[collectptr]= 4;	/* will produce a line by itself */
	collectptr++;

}
/************************************************
   Function:   newlinelst
   Purpose:    this routine is responsible for all printing and 
	       formatting of the listing 
   input:      None
********************************************************/	
newlinelst() {
	int j,i;		/* counter */

	zero_flag=0;
	linecount();
	if (collectptr > 0)	/* collect was called with code generated */
	 { print_line(0);	/* print special cases */
	   printobjcode(0); }	/* prints the hexcode upon number of bytes */
	else
				/* skips the first four columns */
           fprintf(listfil, SPACE_30);  

/*CHANGE FOR LISTING*/
			/* ---- put out the source line ----- */
        fprintf(listfil, SPACE_7);   /* space before source line  */
	if (passno == 1) {
           if (mn_xref ) {
              fputs(mn_buf, listfil);   
              fputs(mn_buf, tmpinfil); 
              strcpy(mn_buf, "          ");
           }
	   fputs(linebuf, listfil);
	   fputs(linebuf, tmpinfil);   
	} else {
	   while ((i=getchar()) != EOF && ( i != '\n')) 
		fprintf(listfil,"%c",i);  
	   fprintf(listfil,"\n");    
        }  
				/* output loccounter and hexcode
			associated with more than one line output */
        for(j=1; j< collectptr; j++) {
	    fprintf(listfil, MESSAGE( 127 ));
	    print_line(j);		/* print special cases */
	    printobjcode(j);
	    fprintf(listfil,"\n");
	}	/* end for loop */
	collectptr=0;
        if (line_error) {
       	   line_error=0;
	   fputs(errbuf, listfil);
	   errbuf_ptr=errbuf;
        }
}

/********************************************
   Function:    print_line
   Purpose: this routine writes listing file for special cases   
   input:    k  -  index to locounter array  and sname array
***********************************************/
print_line(k)
int k;
{
	   if (sname[k][0] == '%') 
		fprintf(listfil, MESSAGE( 128 ));
	   else if (locounter[k] == SPEC_SET)	/* if display val called */
		fprintf(listfil, SPACE_23);
	   else if (locounter[k] == SPEC_DOT)	/* if display requires dots */
		fprintf(listfil, SPACE_13_X, (locounter[k-1]+4) );
	   else
/****	  	fprintf(listfil, MESSAGE( 131 ), sname[k],locounter[k]);  */
	  	fprintf(listfil, " %-4s %-6.6s %.8x  ", 
                           asm_mode_str, sname[k], locounter[k]);  
}

/* *********************************************
  Function:  dmp_ftable
  Purpose: print file_table ( at bottom of .lst file )
  Input:  None
*********************************************** */
int
dmp_ftable()
	{
	register int i;

	if(last>=0){  
  		/* table is not empty 	*/
                if(*file_table[0]=='-'){
			/* "-" is first file name - skip it */
			i=1;
			}
                else{
			i=0;
			}
		fprintf(listfil, MESSAGE( 132 ));
		for(;i<=last;i++){ 
			fprintf(listfil,MESSAGE( 133 ),i,file_table[i]);
			}
		}
	}
	 
/*********************************************
   Function:  linecount
   Purpose:   this routine outputs file number, line number and seperator
   input:     None
*********************************************/
linecount() {
	 fprintf(listfil, MESSAGE( 134), file_no, lineno);
}


/***********************************************
   Function: cleanlst
   Purpose: this routine prints file_table, closes and 
            cleans the list file
   input:   None
**************************************************/
cleanlst() { 
	dmp_ftable();
	fclose(listfil);
	}	


/********************************************
   Function:  printobjcode
   Purpose:   Write the objcode into the listing file
	      It prints a byte or half or long number upon
              recieving the length to be printed
   input:     number -  index into the objcode and locounter arrays
************************************************/
printobjcode(number) 
int number;
{
	int h;
	if (passno ==1)
		fprintf(listfil, MESSAGE( 135 ));
	else if (locounter[number] == SPEC_DOT )
		fprintf(listfil, MESSAGE( 136 ));
	 else
	 switch(lenarray[number]) {
		case 1:	fprintf(listfil,MESSAGE(137), (objcode[number] >> 24));
			break; 
		case 2:	fprintf(listfil,MESSAGE(138), (objcode[number] >> 16));
			break;
		case 3: fprintf(listfil,MESSAGE(139), (objcode[number] >> 8));
			break;
		case 4:	fprintf(listfil,MESSAGE(140), objcode[number]);
			break;	} 
}

/******************************************
    Function:  check_zero
    Purpose: this routine checks hexcode recieved in
	collect call to see if more than 2 sets of hexcode
	with a value of zero has been called and processes the 
	output in a special format 
    input:  hex_code -  object code
**************************************************/
check_zero(hex_code)
unsigned int hex_code;
{
	if (hex_code == 0) 
		switch ( zero_flag ) {
			case 0:		/* first zero line found */
			case 1:		/* second zero line found */
				zero_flag++;
				break;
			case 2:		/* third zero line found */
				zero_flag++;
				/* throw away 2nd zero - overwrite with dots */
				locounter[collectptr-2] = SPEC_DOT;
				break;
			case 3:
				collectptr--;	/* dec counter for last loc */
						/* overwrite the last line */
				locounter[collectptr-1]=locounter[collectptr];
				break;
			default:
				printf( MESSAGE( 141 ));
			}
	else  zero_flag =0;
}
