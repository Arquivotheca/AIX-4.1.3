static char sccsid[] = "@(#)67	1.2.1.1  src/bos/usr/ccs/bin/as/POWER/makeops.c, cmdas, bos411, 9428A410j 5/10/93 15:17:41";
/*
 * COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
 *
 * FUNCTIONS: 
 *
 * ORIGINS:  3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*************************************************************************
*	makeops routine will take input from stdin and output to stdout	 *
*	a list of all opcodes within the input file(of standard format)  *
*	which is sorted by mnemonics.					 * 
*									 *
*       1- If a length bit of a field within the unsigend 32 bit         *
*	   integer can not fit the number allocated in the input file a	 *
*	   warning message will be send to the stderr indicating the     *
*	   the number, the bit field and the mnemonic.			 *
*	2- makeops always creates a unique(taged with the process id)    *
*	   unsorted version of the output in /tmp directory. if the      *
*	   system call to sort performs OK the file created in /tmp will *
*	   be deleted automatically, else an error message indicating    *
*	   the error will appear and the unsorted version of the output  *
*	   will not be deleted.						 *
*************************************************************************/
#include	"stdio.h"
#include	<string.h>
#include 	<sys/errno.h>

char	template[]= "MKOPSXXXXXX";	/* the X will be repalced to allow */
					/* for the uniqueness of file name */
char	fname [80] = "/tmp/";		/* system directory were file stored */
char 	cmd[80] = "sort ";
char 	*addr;				/* file pointer form mktemp  */
FILE	*temp;				/* file descriptor */
extern int	errno;			/* system check if error number */
extern char   	*mktemp();
extern FILE	*fopen();
unsigned int	fmt;			/* format descriptor */

main()
{
	char vers[10];		/* version id */
	char s1[10];		/* mnemonics, for PowerPC */
        char s4[10];            /* mnemonics, for POWER   */
	char s2[10];		/* if */
	char s3[10];		/* of */
	char str[4];		/* will store the - (dash) between numbers */
	char f1[10];		/* split between if */
	char f2[10];		/* splt between if */
        unsigned char type_id;  /*  '1' = POWER    */ 
                                /*  '2' = PowerPC  */
                                /*  '0' = POWER and PowerPC has the same MN */
 	char *s;
	char *ptr;		/* pointer to ? in a string */
	int ch;			/* will pick one character */
	int num;		/* picks up the numbers */
	int n; 			/* number of items picked up by scan stream */
	int more;		/* flag - read more numbers */
	int l;			/* string length of the mnemonic */
	int j, i;		/* counter */
        int flg1, flg2;         /* indicate instruction output format  */
        unsigned long inst_bmap;  /* instruction bit map value        */
        char  type_ind[6];         /* instruction type indicator       */ 
         

	addr= mktemp(template);		/* this routine will create a
					unique file by replacing the X */
	strcat(fname, addr);		/* the file is created in /tmp */
	temp= fopen(fname, "w+");	/* open the file  to write */

	/*
	 * read lines - ignore comments, blanks, tabs 
    	 * 	comment - first character is '*'
	 *	everything else - nothing in first char
	 */

	while ( EOF != (ch = getchar()) ) {  /* read first character in line */
				             /* first char may be * or # or % */
           if ( (char)ch == '*' || (char)ch == '#' || (char)ch == '%') {
	      if( (char)ch == '#')  {
	         scanf("%s", vers);
		 fprintf(temp,"#define versionID \"%s\"\n",vers);
              }
              if ( (char)ch == '%' ) {   /* get type indicator  */
                 scanf("%s", type_ind);
                 inst_bmap = (unsigned long) atol(type_ind); 
              }
	      while (getchar() != '\n');     /* keep reading until EOLN */
	   }
           else {				/* not a comment line */
		n =scanf("%s%s%s", s1, s2, s3);
		l=strlen(s1);		/* get mnemonic string len */
		if ( s1[l-1] == '\047')	/* compare ascii value of ' */
	            s1[l-1] = '\0';	/* chop off the extra ' */
                for (s=s3; (*s); s++) *s = toupper(*s);
		if ( (ptr=strchr(s2,'?') ) == NULL ) {
                     flg1 = 1;
	             fprintf(temp,"OP(\"%s\",%u, \"%s\", \"\",%s,", 
                                   s1,inst_bmap, s2, s3);
		} else { /* it may be "y?XZZ" or "?!XZ" or "?&XZZ"  */
                     flg2 = 1;
                     if ( !strchr(s2,'!') && !strchr(s2, '&') ) {
                                  /* it must be "y?XZZ" */
		         for (j=0; j<strlen(s2); j++) {
				if (s2[j] == 'y')
					break;
				f1[j]=s2[j];   
				f2[j]=s2[j];
                         }
		         f2[j]=s2[strlen(s2)-3];
		         f2[++j] = '\0';
		         f1[--j]=s2[strlen(s2)-2];
		         f1[++j]=s2[strlen(s2)-1];
		         f1[++j]='\0';
	       	     }		/* end if no ! */
		     else {
		         for(j=0; j<strlen(s2); j++) {
				if (s2[j] == '?')
					break;	
				f1[j]=s2[j];
				f2[j]=s2[j];	
                         }
                         if ( !strchr(s2,'&') ) {
		                        /*  "?!XZ"  included */
			   f1[j]=s2[strlen(s2)-2];
			   f1[++j]= '\0';
		           f2[--j]=s2[strlen(s2)-1];
		           f2[++j]= '\0';
	                 } else {  /*   /* "?&XZZ" format  */
                           for ( i = j+2; i < (strlen(s2)-1 ); i++ ){
                             f1[j] = s2[i];
                             f2[j++] = s2[i+1];
                           }
                           f2[j] = '\0';
                           f1[j++] = s2[i];
                           f1[j] = '\0';
                         }
                     }
		     fprintf(temp,"OP(\"%s\",%u, \"%s\", \"%s\", %s,", s1,
                                    inst_bmap, f1, f2, s3);
	        }	
			/* -- read in format descriptor fields -- */
			for (fmt=0, more=1, n=0; more && (n < 8); n++) {
			    more = (2 == scanf("%d%[-]", &num, str));
			    switch (n) {
				case 3:		/* ov bit */
				case 6:		/* ab bit */
					if(num != 0)
					  set(n, num, s1);  break;
				default:  set(n, num, s1); }
			}			/* end for loop */
			/* -- print format descriptor ---- */
/*******		fprintf(temp, "0x%x),\n", fmt);    ******/
			fprintf(temp, "0x%x", fmt);
				/* ---- read end of line ---- */
                        if ( (ch = getchar()) != '\n' ) {
                           ungetc(ch, stdin);
                           scanf("%s", s4);
                           type_id = '2';       /* s1 holds PowerPC MN now */
                           fprintf(temp, ", \"%s\",%c),\n", s4, type_id);
                           type_id = '1';  /* prepare for POWER MN record */
                           if ( flg1 ) {
         fprintf(temp, "OP(\"%s\",%u, \"%s\", \"\",%s,0x%x, \"%s\",%c),\n",
                                    s4,inst_bmap, s2, s3, fmt, s1, type_id);
                              flg1 = 0;
                           }
                           else if ( flg2 ) {
        fprintf(temp,"OP(\"%s\",%u, \"%s\", \"%s\", %s, 0x%x, \"%s\",%c),\n", 
                              s4, inst_bmap, f1, f2, s3, fmt, s1, type_id);
                              flg2 = 0;
                           }  
                           while ( ch = getchar() != '\n');
                        }
                        else {
                           fprintf(temp, ",\"\",0),\n");
                           flg1 = 0;
                           flg2 = 0;
                        }
		      }					/* end else */
		}						/* end while */
	fflush(temp);			/* remove fname flush output buff */

	strcat(cmd, fname);
	strcat(cmd, "\n");
	if (system (cmd) < 0 )			/* sort file fname */
		perror("sort:");		/* if error printf error msg */
	else {				/* if error accurs at sort time */
		strncpy(cmd, "rm ", 4);	/* unsorted file in /tmp is not */
		strcat(cmd, fname);	/* removed for checking */
		system(cmd);   }
}	/* end main */

char*	names[] ={ "opc","exopc","rc/lk","ov","frt","fra","ab","frb"};
	/* the highest bit position of exopc was 29 and length was 8 */
	/* but considering ab bit is an overlay on exopc if bit is 1 */
				/* highest bit position in argument n */
		/* opc-exopc-rc/lk-ov-frt-fra-ab-frb */
int offset[8] = {  5,   30,   31,  21,10, 15, 30, 20 }; 
				/* length in bits of field */
		/* opc-exopc-rc/lk-ov-frt-fra-ab-frb */
int  length[8] = {  6,   10,   1,   1, 5,  5,  1, 5 }; 


set(n, num, numonic)
	int 	n;				/* argument number */
	int 	num;				/* value */
	char*   numonic;			/* numonic */
{				    /* if num fits in bits length specified */
	if (num < (power(2, length[n])) ) /* 2 to the power of length[n] */
	    fmt |= (num << (31 - offset[n]));
	else				/* error since bits do not fit */
    fprintf(stderr,"Value %d larger than bit lenght of %s = %d numonic %s\n",
			num, names[n], length[n], numonic); 
}

power(x,m)			/* raise x to the m-th power */
int x, m;
{
	int i,p;
	
	p =1;
	for(i=1; i<= m; ++i)
		p = p * x;
	return(p);
}

