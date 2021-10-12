static char sccsid[] = "@(#)60	1.2  src/bldenv/errlg/errprefix/errprefix.c, cmderrlg, bos412, GOLDA411a 2/22/94 16:57:28";
/*
 *   COMPONENT_NAME: CMDERRLG
 *
 *   FUNCTIONS: main
 *		msg_list
 *		skip_to_nwhite
 *		skip_to_white
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

#define skip_to_white(s) 	while (*s && !isspace(*s)) s++
#define skip_to_nwhite(s) 	while (*s && isspace(*s)) s++

#define		MSG_LEN	   4096 
extern 	int	msg_list();


main(argc,argv)
int argc;
char **argv;
{
	if(argc < 2) {
		printf("\n Function: Prefix Message Utility\n");
		printf(" Usage:  %s filename  [+|-]\n\n", argv[0]);
		printf(" eg.     %s /usr/lpp/msgsrc/En_US/com/cmd/as/as.msg\n", argv[0]);
		exit(1);
	}
	if( argc > 2 )
		msg_list(argv[1],(*argv[2] != '-') );
	else
		msg_list(argv[1], 1 );
}
 


/*
 * NAME: msg_list(file, op )
 *
 * FUNCTION: read a .msg file and output a prefixed message file
 *
 * RETURN VALUE DESCRIPTION: returns ERROR on failure.
 */
#ifdef	_ANSI
int msg_list(char    file[], int op )
#else		/*_ANSI */
int msg_list( file, op)
char			file[];	/*----  the name of the file to be read ----*/
int			op;
#endif		/*_ANSI */
{
	FILE *fp;		/*----  file stream for the .msg  ----*/
	char	*line,		/*----  line point to bline buffer ----*/
		bline[MSG_LEN],	/*----  the line currently being read in  ----*/
		line1[MSG_LEN],	/*----  an un-macro-expanded version of line  ----*/
		quote = NULL, 	/*----  the value of the current quote character  ----*/
		*p;		/*----  pointer to a spot in line[1]  ----*/
	int	msg_length = MSG_LEN,	/*----  current msg len ----*/
		h=0,i,j,k,f;		/*----  Misc counter ----*/
	char	filler[60];
	int	l;

	strcpy( filler, "XNULL_" );
	l = strlen(filler);
/*......................................................................
  Open the message source file.  Return an error code if non existant.
  ......................................................................*/
	fp = fopen( file, "r" );
	if ( fp == NULL ) {
		printf( " can't open file: %s \n", file);
		return( -1 );
	}

	while ( feof( fp ) == 0 ) {
/*......................................................................
  Read in a line.
    while line terminated in a newline, append next line to end of current.
    preprocess the constants into integers.
  ......................................................................*/
		bline[0] = NULL;
		if ( fgets( bline, MSG_LEN, fp ) == NULL && ( feof( fp ) == 0 ) ) {
			printf("Unable to read message file: %s",file );
			fclose( fp );
			return( -1 );
		}
		line = &bline[0];
		while( isspace(*line)) line++;
		while ( strlen( line ) >= 2 && 
		    line[strlen( line ) - 2] == '\\' && 
		    feof( fp ) == 0 ) {
			if (( p = fgets( &line[strlen( line ) - 2], 
			    MSG_LEN - strlen( line ), fp )) == NULL 
			    && ( feof( fp ) == 0 ) ) {
				printf("Unable to read message file: %s",file );
				fclose( fp );
				return( -1 );
			}
		}

/*......................................................................
  check for kewords ( $quote, $set, $msg, $len, $ comment )
  ......................................................................*/


/*......................................................................
  Skip comment lines and blank lines.
  ......................................................................*/
		if ( line[0] == NULL ) {
		/* Do nothing */	;
		}
		else if ( line[0] == '$' && line[1] <= ' ' ) {
			printf( "%s",bline );
		}
		else if ( line[0] == '$' && line[1] == '$' ) {
			printf( "%s",bline );
		}
/*......................................................................
  $len processing
   Set message length 
  ......................................................................*/
		else if ( strncmp( "$len", line, 4 ) == 0 ) {
			p = line;
			skip_to_white(p);
			skip_to_nwhite(p);
			if ( !*p )
				msg_length = MSG_LEN ;	
			else  {
				if ( atoi( p ) ) 
					msg_length = atoi( p );
				else {
					printf( "Invalid length." );
					return (-1);
				}
			}
			printf( "%s",bline );
				
		}

/*......................................................................
  $Quote processing
   Set quote character.
  ......................................................................*/
		else if ( strncmp( "$quote", line, 6 ) == 0 ) {
			p = line;
			skip_to_white(p);
			skip_to_nwhite(p);
			if ( !*p ) {
				printf(  "Invalid quote character." );
				return(-1);
			}
			else 
				quote = *p;
			printf( "%s",bline );
		}

/*......................................................................
  $set processing ...
  ......................................................................*/
		else if ( strncmp( "$set", line, 4 ) == 0 ) {
			printf( "%s",bline );
			p = line;
			skip_to_white(p);
			skip_to_nwhite(p);
			sprintf( filler, "X%s", p );
			p = filler;
			skip_to_white(p);
			*(p)='_'; *(p+1)=0;
			l = strlen(filler);
		}
/*......................................................................
  $msg processing ...
  prefixed message using prefix_char
  ......................................................................*/

		else {
			i = 0;
			if( op )
				printf("%s", filler ); 
			else i = l;
			for(  j = 0; bline[i] && bline[i] != '\n'; i++ ) {
				line1[j++] = bline[i];
				if( bline[i] == '\\' && bline[i+1] == 'n') {
					line1[j] = '\0';
					printf( "%sn\\\n", line1);
					j = 0;
					i += 1;
				}
			}
			line1[j] = '\0';
			printf( "%s\n", line1);
		}
	}
	fclose( fp );
	return(0);
}





