static char sccsid[] = "@(#)91  1.1  src/bldenv/brand/brand.c, bldprocess, bos412, GOLDA411a 8/7/92 18:20:44";
/*
 * COMPONENT_NAME: (BOSBUILD) Build Tools and Makefiles
 *
 * FUNCTIONS: brand
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

 /* 
  * brand substitutes strings for keynames via a substitution file.
  * It takes as its input a file consisting of lines containing
  * a keyname, white space, and a double-quoted string.  All characters
  * between the outer-most double-quotes are taken as the substitution 
  * string.  Comment lines begin with a '#' in the first column.
  * Blank lines are allowed.  Each keyname MUST begin with "%%_".
  */

 /*
  * brand takes the name of the substitution file as input, processes
  * standard input, and writes to standard output.
  */

 /*
  * For each instance of a keyname found in stdin, the
  * text between the corresponding outer double quotes is substituted.
  */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>

#define MAXNUMSUBS 1000	    /* Maximum number of substitution pairs */
#define MAXKEYSIZE 80	    /* Maximum length of the substitution keyword */
#define MAXSUBSIZE 1024     /* Maximum length of quoted substitution text */
#define MAXSUBLINE 2048	    /* Maximum length of substitution pair line */
#define MAXSRCLINE 10240    /* Maximum length of processed source line */
#define KEYPATTERN "%%_"    /* Recognized keyword pattern */

char keyname[MAXNUMSUBS][MAXKEYSIZE+1];	 /* Substitution keywords */
char substring[MAXNUMSUBS][MAXSUBSIZE+1]; /* Substitution text */

char *keypattern = KEYPATTERN;

int subst_count = 0;

char *prog;

main(argc,argv)
int argc;
char **argv;
{
   setlocale(LC_ALL,""); 
   prog = argv[0];
   
   if (argc != 2) {
	fprintf(stderr,"Usage:\t%s filename\n", prog);
	fprintf(stderr,"\t\tfilename contains word-phrase substitution text;\n");
	fprintf(stderr,"\t\tOne substitution per line: keyname \"substitution text\"\n");
	return 1;
   }

   if (read_subst_data(argv[1]))
       return 1;

   return substitute();
}

/* 
 *  read_subst_data - Read substitution data
 *
 *  Process the substitution data.  Substitution data is expected to consist
 *  of pairs of substitution keywords and substitution strings.  The keywords
 *  are expected to begin with a designated key pattern, and the substitution
 *  strings are double-quoted.  Anything within the outer double-quotes is
 *  taken as the replacement text.
 *
 *  It returns -1 upon failure.  0 upon success.
 */
   
read_subst_data(filename)
char *filename;
{
   FILE *subfile;
   char fileline[MAXSUBLINE];
   char *begquote;
   char *endquote;

   subfile = fopen(filename, "r");
   if (subfile == NULL) {
       fprintf(stderr, "%s: ERROR: Unable to open substitution file %s\n", prog, filename);
       return -1;
   }

   errno = 0;
   while (fgets(fileline,MAXSUBLINE,subfile) != NULL) {
   /* Allow for comment lines */
       if (fileline[0] == '#')
	   continue;
   /* Allow for blank lines */
       if (sscanf(fileline,"%s",keyname[subst_count]) != 1) {
	  continue;
       }
        
       begquote = strchr(fileline,'"');
       endquote = strrchr(fileline,'"');

       if (!(strstr(keyname[subst_count],KEYPATTERN))) {
              fprintf(stderr, "%s: ERROR: Error in key pattern text on line %d\n", prog, subst_count+1);
              fprintf(stderr, "%s:        Key pattern name must begin with %s\n", prog, KEYPATTERN);
              return -1;
       } 

        /* Check for no quotes  or  finding one quote.  Oooh...smart. */
       if (begquote == endquote)  {
              fprintf(stderr, "%s: ERROR: Error in substitution text on line %d\n", prog, subst_count+1);
              fprintf(stderr, "%s:        Substitution text must be double-quoted at each end\n", prog);
              return -1;
       }
    
       *endquote = '\0';
       if (subst_count >= MAXNUMSUBS) {
              fprintf(stderr, "%s: ERROR: Maximum number of substitutions exceeded processing %d\n", prog, filename);
              fprintf(stderr, "%s:        Rebuild with increased MAXNUMSUBS.\n", prog);
              return -1;
       }
       strcpy(substring[subst_count++],begquote+1);
   }

   if (errno != 0) {
              fprintf(stderr, "%s: ERROR: Error reading substitution text on line %d\n", prog, subst_count+1);
              return -1;
   }

   return 0;
}

/*
 * substitute - Perform the substitutions on stdin and write to stdout
 */

substitute() {
   char buf[2][MAXSRCLINE];
   char *input, *output;
   char *keystr;
   int curbuf = 0;
   int ln = 1;
   int i;
   int ret = 0;

   input = buf[0];
   output = buf[1];
   while (gets(input) != NULL) {
       if (!(strstr(input,KEYPATTERN))) {
	   puts(input);
       } else  {
	  /* At this point, we are probably going to substitute something */
           for (i = 0; i < subst_count; i++) {
		keystr = strstr(input,keyname[i]);
		if (keystr != NULL) {
		    while (keystr != NULL) {
			*keystr = '\0';
			strcpy(output,input);
			output += keystr-input;
			input = keystr+strlen(keyname[i]);
			strcpy(output,substring[i]);
			output += strlen(substring[i]);
			keystr = strstr(input,keyname[i]);
		    }
		    strcpy(output,input);

		    /* Switch the buffers around */
		    if (curbuf == 0) {
			input = buf[1];
			output = buf[0];
			curbuf = 1;
		    } else {
			input = buf[0];
			output = buf[1];
			curbuf = 0;
		    }

		}
		
	   }
	   if (strstr(input,KEYPATTERN)) {
	       fprintf(stderr,"%s: WARNING: possible translation error at line %d\n", prog, ln);	
	       fprintf(stderr,"%s:    Line: %s\n\n",prog, input);	
               ret = 1;
	   }
	   puts(input); /* Trust me, the output is in the input.  Get it? */
       }
       ln++;
   }
   return ret;
}
