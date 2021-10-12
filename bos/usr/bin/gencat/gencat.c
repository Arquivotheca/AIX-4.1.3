static char sccsid[] = "@(#)48	1.32  src/bos/usr/bin/gencat/gencat.c, cmdmsg, bos411, 9428A410j 4/2/94 10:36:52";
/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: main, bump_msg, set_quote, set_message, store_msg, set_set,
 *            set_len, delset, get_text, write_msg, msg_comp, load_cat,
 *            open_source, rmalloc 
 *
 * ORIGINS: 27, 18, 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
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
 * EXTERNAL PROCEDURES CALLED: standard library functions
 */

#define _ILS_MACROS

#include <stdio.h>
#include <stdlib.h>
#include "catio.h"
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <langinfo.h>

#include <ctype.h>
#ifndef _BLD_OSF
#include <locale.h>
#include "msgfac_msg.h" /* include file for message texts */
#endif

#define MAXMSG 32
#define isaoct(c) (c >= '0' && c <= '7')
#ifndef iswblank
#define iswblank(wc)      is_wctype(wc, blank_type) /* not defined by X/Open */
#endif
#define SLOP	20

wctype_t blank_type;	/* value from get_wctype("blank") */

/***************************************************************
 * defines for the 'bootstrap' (no NLS/MSG) build version
***************************************************************/
#ifdef _BLD_OSF

#define catgets(a,b,c,s)      s

#ifndef PATH_MAX
#define PATH_MAX	1024
#endif
#ifndef NL_TEXTMAX
#define NL_TEXTMAX 8192
#endif
#ifndef NL_MAXOPEN
#define NL_MAXOPEN	10
#endif
#ifndef NL_SETD
#define NL_SETD	1
#endif
#ifndef CAT_MAGIC
#define CAT_MAGIC	505
#endif

/*** structures ***/
struct _message {
        unsigned short  _set,
                        _msg;
        char            *_text;
        unsigned        _old;
};
struct _header {
        int             _magic;
        unsigned short  _n_sets,
                        _setmax;
        char            _filler[20];
};
struct _msgptr {
        unsigned short  _msgno,
                        _msglen;
        unsigned long   _offset;
};
struct _catset {
        unsigned short  _setno,
                        _n_msgs;
        struct _msgptr  *_mp;
        char    **_msgtxt;
};
struct catalog_descriptor {
        char            *_mem;
        char            *_name;
        FILE            *_fd;
        struct _header  *_hd;
        struct _catset  *_set;
        int             _setmax;
        int             _count;
        int             _pid;
};


typedef struct catalog_descriptor *nl_catd;
typedef struct catalog_descriptor CATD;
#ifndef CATD_ERR
#define CATD_ERR	((nl_catd)-1)
#endif

static CATD *catsopen[NL_MAXOPEN];      /*---- list of open catalog pointers -*/
static int  catpid[NL_MAXOPEN];         /*---- list of pid associated with
                                               the corresponding to each
                                               catalog pointer in catsopen --*/
static void make_sets();
static FILE *opencatfile();
static void add_open_cat();
static void memmove();
static void substitute();
static void cat_hard_close();           /*---- phyically closes the cat ----*/
static nl_catd cat_already_open();      /*---- used to see if the cat has 
                                                   already been opened ----*/
nl_catd	catopen();
nl_catd	NLcatopen();
nl_catd	_do_open();

extern char *strchr();

extern int errno;
#endif /* _BLD_OSF */
/*******************end if boot strap stuff***************/



wchar_t		quote;			/*---- current quote character ----*/
unsigned short	set = 0;		/*---- current set number  ----*/
unsigned short	msglen = NL_TEXTMAX;	/*---- current msglen ----*/
int		current = -1;		/*---- current _message index into 
                                               emsg[] ----*/
int 		msgmax  = 0;		/*---- current dimension of emsg[] ---*/
struct _message	*emsg;			/*---- array of _message structs 
                                               (holds all _messages --*/
nl_catd	catderr;			/* gencat error catalog descriptor */


/*______________________________________________________________________
	These internal routines all have void data types.  (i.e. if
	they fail there is no recovery (they die()).
  ______________________________________________________________________*/

#ifdef _NO_PROTO
void set_quote() ;
void get_message() ;
void store_msg() ;
void set_set() ;
void set_len() ;
void delset() ;
int get_text() ;
void write_msg() ;
int msg_comp() ;
void load_cat() ;
FILE *open_source() ;
char *rmalloc() ;
char *skip_to_nwhite() ;
char *skip_to_white() ;
#else
void set_quote(char *line) ;
void get_message(char *line, FILE *file) ;
void store_msg(struct _message *msg) ;
void set_set(char *line) ;
void set_len(char *line) ;
void delset(char *line) ;
int get_text(char *source, char *target, FILE *file) ;
void write_msg(struct _message msg[], FILE *file) ;
int msg_comp(struct _message *a, struct _message *b) ;
void load_cat(char *tcat) ;
FILE *open_source(char *file) ;
char *rmalloc(int n) ;
char *skip_to_nwhite(char *p) ;
char *skip_to_white(char *p) ;
#endif /* _NO_PROTO */


/*
 * NAME: main 
 *                                                                    
 * FUNCTION: Parses the arguments, reads the input stream and
 *           drives the rest of the program.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 0 on success, 1 on failure.
 *
 */  

#ifdef _NO_PROTO
main(argc,argv,envp)
int argc; 
char *argv[]; 
char *envp[];
#else
main(int argc, char *argv[], char *envp[])
#endif /* _NO_PROTO */
{
	char 	*target,		/*---- Target file name ----*/
		line[NL_TEXTMAX+SLOP],	/*---- current line of text ----*/
 		*p; 			/*---- dummy string variable ----*/
	FILE	*sf,			/*---- source stream ----*/
		*tf;			/*---- target stream ----*/
	int file_no = 2;
	
/*______________________________________________________________________
	Check the input arguments, open the input and output files
  ______________________________________________________________________ */
#ifndef _BLD_OSF
	setlocale(LC_ALL,"");
#endif
	catderr = catopen(MF_MSGFAC, NL_CAT_LOCALE);
	if (argc < 2) 		/*---- die if no target cat and source specified ----*/
		die( catgets(catderr, MS_GENCAT, M_MSG_0, "Usage: gencat CatalogFile SourceFile...") );
	if (argc == 2)
		sf = stdin;	/*---- read from stdin ----*/
	else {
		sf = (FILE *)open_source(argv[file_no]);
	}

	/* get_wctype() is expensive, so call it once rather than in a loop */
	blank_type = get_wctype("blank");

	target = argv[1];
	if (strcmp(target,"-") != 0)
		load_cat(target);	/*-- Load any existing catalog into memory --*/

	do {
		fgets(line,NL_TEXTMAX,sf);
		while (!feof(sf)) {	/*- read through the input and 
                                            branch on any keywords -*/
			if (!memcmp(line,"",1))
				;
			else if (!memcmp(line,"\n",1))
				;
			else if (!memcmp(line,"$quote",strlen("$quote"))) {
				set_quote(line);
			}
			else if (!memcmp(line,"$delset",strlen("$delset"))){
				delset(line);
			}
			else if (!memcmp(line,"$set",strlen("$set"))) {
				set_set(line);
			}
			else if (!memcmp(line,"$len",strlen("$len"))) {
				set_len(line);
			}
			else if (!memcmp(line,"$ ",2) || !memcmp(line,"$\t",2)||
    				 !memcmp(line,"$\n",2))
				;	       /*----  check for comment  ---*/
			else {
				p = line;
				p = skip_to_nwhite(p);
				if (isdigit(*p)) {
					if (set == 0) {
						set = NL_SETD;
					}
					get_message(p,sf);		
				}
				else if (!memcmp(line," ",1))
					;
				else if (!memcmp(line,"\t",1))
					;
				else  { 
					fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_19, "gencat: Symbolic message identifier used:\n \t %s"),line);
					exit (1);
				}
			}
			fgets(line,NL_TEXTMAX,sf);
		}
	}
	while (++file_no < argc && (sf = (FILE *)open_source(argv[file_no]))); 

	if (current == -1) 
		die(catgets(catderr,MS_GENCAT,M_NOMSG, "No messages defined in source file."));
	if (strcmp(target,"-") == 0)
		tf = stdout;
	else
		tf = fopen(target,"w");
	if (!tf) {
		fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_1,
			"gencat: Unable to open target file %s.\n") ,target);
		exit(1);
	}
	write_msg(emsg,tf);	/*---- Write the output ----*/

	if (sf != stdin)
		fclose(sf);
	if (tf != stdout)
		fclose(tf);

	exit(0);
}

/*
 * NAME: bump_msg
 *
 * FUNCTION: Increments the current _message pointer.
 *           Checks for room in emsg. If there is not enough, it 
 *           calls realloc().
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS:  void
 */


void bump_msg()	/*----  incements the current _message pointer, 
			checks for room in emsg[], if there 
			is not enough it will realloc() ----*/
{
	extern struct _message 	*emsg;
	extern int 		current;
	extern int 		msgmax;

	if (current >= msgmax - 2) {
		register int i;

		if (msgmax > 0) {	/*---- if emsg exists ----*/
			msgmax += MAXMSG;
			if (!(emsg = (struct _message *)realloc(emsg,msgmax * 
                            sizeof (struct _message))))
				die( catgets(catderr, MS_GENCAT, M_MSG_2, "gencat:  Unable to realloc().") );
		}
		else {			/*---- if this is the first time ----*/
			msgmax += MAXMSG;
			if (!(emsg = (struct _message *)rmalloc(msgmax *
                            sizeof (struct _message))))
				die( catgets(catderr, MS_GENCAT, M_MSG_2, "gencat:  Unable to realloc().") );
		}
		for (i = current + 1 ; i < msgmax ; i++) {
 			/*-- set up the new _messages --*/
			emsg[i]._text = FALSE;
			emsg[i]._set = emsg[i]._msg = emsg[i]._old = FALSE;
		}
	}
	current++;	/*---- bump current ----*/
}

/*
 * NAME: set_quote
 *
 * FUNCTION: Reset the current quote character.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */


#ifdef _NO_PROTO
void set_quote(line)
char *line; 
#else
void set_quote(char *line)
#endif /*_NO_PROTO */
	/*---- line: input line (must include a $quote ----*/

{
	int	len;		/* length of quote character */

	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	len = mbtowc(&quote, line, MB_CUR_MAX);
	if (len < 0)
		quote = *line & 0xff;
	else if (len == 0)
		quote = 0;
	else if (len == 1)
		quote = (*line == '\n') ? 0 : *line;
}

/*
 * NAME: get_message()
 *
 * FUNCTION: Gets the _message starting on the current line
 *           and store the resulting _message structure in emsg[].
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

#ifdef _NO_PROTO
void get_message(line, file)
char *line; 
FILE *file;
#else
void get_message(char *line, FILE *file)
#endif /* _NO_PROTO */

	/*---- line: Line the where the _message begins ----*/
	/*---- file: File it came from (in case of a continuation) ----*/

{
	char 			ttxt[NL_TEXTMAX+SLOP]; /* place to store the text */
	struct _message 	msg;		  /* _message we are getting */
	static struct _message	omsg;		  /* old _message(order check)*/
	static char		started = 'N';	  /* is there an old _message?*/
	int  			i,j;

	sscanf(line,"%u",&i);
	if (i < 1 || i > NL_MSGMAX) {
		fprintf(stderr,catgets(catderr, MS_GENCAT, M_MSG_19, "gencat: Symbolic message identifier used:\n \t %s"),line);
		exit (1);
	}
	msg._msg = i;
	if (get_text(line,ttxt,file) == -1)  {
	 	for (i=0; i<=current && emsg[i]._text; i++) {
			if (emsg[i]._set == set && emsg[i]._msg == msg._msg) {
				free(emsg[i]._text);
				for (j=i; j<current; j++) {
					emsg[j] = emsg[j+1];
				}
				emsg[j]._text = FALSE;
				current--;
				break;
			}
		}
		return;
	}
	if (strlen(ttxt) > msglen)  {
		fprintf (stderr,catgets(catderr, MS_GENCAT, M_MSG_3,
			"gencat: Message text is longer than $len value.\n \t %s\n"),line);
		exit (1);
	}
 	msg._text = (char *)rmalloc(strlen(ttxt) + 1);
	strcpy(msg._text,ttxt);
	msg._set = set;

	if (started == 'Y' && msg_comp(&msg,&omsg) <= 0) {
		fprintf(stderr, catgets(catderr, MS_GENCAT, M_ORDER,
                        "gencat:  The message numbers/sets became out of \
order just after:\n msg:  %d,  set %d\n %s\n"), omsg._msg,omsg._set, omsg._text);
		exit(1);
	}
	omsg = msg;
	started = 'Y';
	store_msg(&msg);	/*---- Store the _message (used to replace 
                                       old ones) ----*/
}

/*
 * NAME: store_msg
 *
 * FUNCTION: Insterts a _message into emsg[]. Overwrites an existing
 *           _message if the catalog is being updated and there is a 
 *           duplicate _message in the old version of the catalog.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

#ifdef _NO_PROTO
void store_msg(msg)
struct _message *msg;
#else
void store_msg(struct _message *msg)
#endif /* _NO_PROTO */

	/*---- mesg: _message to be inserted in emsg ----*/

{
	extern struct _message 	*emsg;
	extern int		current,
				msgmax;
	int 	i;

/*______________________________________________________________________
	Search to see if there is a duplicate in the old _messages
  ______________________________________________________________________*/

	for (i = 0 ; i <= current ; i++) {
		if (!msg_comp(msg,&emsg[i]))
			break;
	}
	if (i <= current) {
		emsg[i] = *msg;		/* If there is an old one, replace it */
	}
	else {
		bump_msg();		/*---- else add a new one ----*/
		emsg[current] = *msg;
	}
}
			

/*
 * NAME: set_set
 *
 * FUNCTION: Sets the current set number and stores the value in the
 *           global variable 'set'.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

#ifdef _NO_PROTO
void set_set(line)
char *line;
#else
void set_set(char *line)
#endif /* _NO_PROTO */

	/*---- line: line with $set n command  ----*/

{
	int n;

	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	if (!isdigit(line[0])) {
		fprintf(stderr,catgets(catderr,MS_GENCAT,M_MSG_18, "gencat: Symbolic set identifier used. \n \t %s \n"),line);
	        exit (1);
	}
	sscanf(line,"%d",&n);
	if (n < SETMIN || n > SETMAX) {
		fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_5, "gencat:  Invalid set number. \n \t %s\n"),line);
 		exit (1);
	}
	set = (unsigned short)n;
}



/*
 * NAME: set_len
 *
 * FUNCTION: Sets the current len number and stores the value in the
 *           global variable 'msglen'.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */



#ifdef _NO_PROTO
void set_len(line) 
char *line; 
#else
void set_len(char *line)
#endif /* _NO_PROTO */

	/*---- line: line with $len n command  ----*/

{
	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	
        if (isdigit(*line))
            sscanf(line,"%hu",&msglen);
        else
            msglen = NL_TEXTMAX;

}


/*
 * NAME: delset
 *
 * FUNCTION: Delete an existing set of _messages from emsg[].
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */


#ifdef _NO_PROTO
void delset(line) 
char *line; 
#else
void delset(char *line)
#endif /* _NO_PROTO */

 	/*----line: line with $delset n command	        ----*/

{
	extern struct _message 	*emsg;
	extern int		current,
				msgmax;
	unsigned short 		dset;	/*---- set to be deleted ----*/
	int			i;	/*- Misc counter(s) used for loops -*/
	int			n;

	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	sscanf(line,"%d",&n);	/*---- get set to be removed ----*/
	if (n < SETMIN || n > SETMAX) {
                fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_5, "gencat  Invalid set number. \n \t %s\n"),line);
                exit (1);
        }
	dset = (unsigned short)n;


/*______________________________________________________________________
	Shuffle the _messages to delete any existing sets
  ______________________________________________________________________*/

	for (i = 0 ; i <= current && emsg[i]._text ; i++) {
		if (emsg[i]._set == dset) {
			int j;
			free(emsg[i]._text);
			for (j = i ; j < current ; j++) {
				emsg[j] = emsg[j + 1];
			}
			emsg[j]._text = FALSE;
			current--;
			i--;
		}	
	}
}
	

		
/*
 * NAME: get_text
 *
 * FUNCTION: Assembles a string of _message text which has been stored in the 
 *           gencat (._msg file) format.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 0 if  successful.
 *	    -1 if source is an empty string which has to be deleted from
 *          the soruce.
 */


#ifdef _NO_PROTO
int get_text(source, target, file)
char *source; 
char *target; 
FILE *file;
#else
int get_text(char *source, char *target, FILE *file)
#endif /* _NO_PROTO */

	/*---- source: source string ----*/
	/*---- target: target string ----*/
	/*---- file: file (used for multi-line _messages) ----*/

{
	char 	quoted = FALSE,
		*base,
		*next,
		*targetbase;
	int j;
	int	len;
	wchar_t	wc;

	base = source;
	targetbase = target;
	source = skip_to_white(source);
	next = source;
	next = skip_to_nwhite(next);
	len = mbtowc(&wc, next, MB_CUR_MAX);
	if (len < 0) {
		len = 1;
		wc = *next & 0xff;
	}
	if (wc == quote) {
		quoted = TRUE;
		source = next + len;
	}
	else if (*source == ' ' || *source == '\t')
		source++;
	else if (*source == '\n')     /* source is an empty string */
		return(-1);
	while (*source && target - targetbase <= NL_TEXTMAX) {
		len = mbtowc(&wc, source, MB_CUR_MAX);
		if (len < 0) {
			len = 1;
			wc = *source & 0xff;
		}
		if (wc == '\\') {	/*---- Process backslash codes ----*/
			source++;
			if (isaoct(*source)) {	/*---- Octal number ----*/
				int octal;
				sscanf(source,"%3o",&octal);
				*target++ = octal;
				for (octal = 0 ; octal < 3 && isaoct(*(source +
                                     octal)) ; octal++)
					;
				source += octal;
			}
			else {
				switch (*source) {
					case 'n': {
						*target++ = '\n';
						source++;
						break;
					}
					case 't': {	/*---- tab ----*/
						*target++ = '\t';
						source++;
						break;
					}
					case 'r': {	/*---- return ----*/
						*target++ = '\r';
						source++;
						break;
					}
					case 'b': {	/*---- backspace ----*/
						*target++ = '\b';
						source++;
						break;
					}
					case 'f': {	/*---- form feed ----*/
						*target++ = '\f';
						source++;
						break;
					}
					case 'v': {	/*--- vertical tab ---*/
						*target++ = '\v';
						source++;
						break;
					}
					case 'x': {	/*--- hex number (two 
                                                         or four digits) ---*/
						int 	hex,
							hexlen = 0;
						source++;
						while (isxdigit(*(source +
                                                       hexlen)))
							hexlen++;
						if (hexlen == 2) {
							sscanf(source,"%2x",
                                                               &hex);
							*target++ = hex;
							source += hexlen;
						}
						else if (hexlen == 4) {	
							sscanf(source,"%4hx",
                                                               target);
							target += 2;
							source += 4;
						}
						else {
						 	fprintf(stderr, catgets( catderr, MS_GENCAT, M_MSG_7, "Bad hex len (the length of a hex number must be either two or four digits.)\n \t %s\n"),base);
							exit (1);
						}
						break;
					}
					case '\n': {	/*-- continuation --*/
						source = base;
						fgets(source,NL_TEXTMAX,file);
						break;	
					}
					default: {
						len = mblen(source, MB_CUR_MAX);
						if (len < 0)
							len = 1;
						do
							*target++ = *source++;
						while (--len > 0);
					}
				}
			}
		}
		else if (quoted && wc == quote) 
			break;
		else if (wc == '\n')
			if (quoted) {
				fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_8, "gencat:  Unexpected newline within quotes. \n \t %s \n"),base);
				exit (1);
			}
			else
				break; 
		else {
			do
				*target++ = *source++;
			while (--len > 0);
		}
		if (!(target - targetbase <= NL_TEXTMAX)) {
			fprintf (stderr, catgets(catderr, MS_GENCAT, M_MSG_9, "gencat:  Message string longer than NL_TEXTMAX.\n \t %s \n"),base);
			exit (1);
		}
	}
	if (!*source) { 
		fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_10, "gencat: Unexpected end of string, (no newline or end of quotes) \n \t %s\n"),base);
		exit (1);
	}
	*target = '\0';
	return (0);
}


/*
 * NAME: write_msg
 *
 * FUNCTION:  Converts emsg[] into a format suitable for fast access and write
 *            the result to the target file (file).
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

#ifdef _NO_PROTO
void write_msg( msg, file)
struct _message msg[]; 
FILE *file;
#else
void write_msg(struct _message msg[], FILE *file)
#endif /* _NO_PROTO */

	/*---- mesg: _message array to be written 
	      (this is actually equal to emsg[]) ---*/
	/*---- file: File to write msg[] to ----*/

{
	short		i,j;		/*---- Misc counter(s) used for loops */
	int 		total_sets;	/*---- total sets used ----*/
	int 		total_msgs;	/*---- total _messages in msg[] ----*/
	int 		header_size = sizeof(struct _header);
     					/*---- header size ----*/
	int 		msg_offset;	/*---- place to write the text of the
   					       next  _message ----*/
	struct _msgptr 	mp;		/*---- structure used to accellerate 
     				  	       the _message retrieval ----*/
	struct _header	hd;		/*---- _header record of the .cat file*/
	char		*codeset;	/*---- name of present codeset ----*/

/*______________________________________________________________________
	Use qsort to sort msg[] by _message within set
  ______________________________________________________________________*/

	for (i = 0 ; msg[i]._text ; i++) 
		;
	qsort ((void *)msg,(size_t)i,sizeof(struct _message),msg_comp); 

/*______________________________________________________________________
	Set up:
		total_sets,
		total_msgs,
		setmax
  ______________________________________________________________________*/

	for (i = 0 , total_sets = 0 , hd._setmax = 0 ; msg[i]._text ; i++) {
		if (!i || msg[i]._set != msg[i - 1]._set)
			total_sets++;
		if (msg[i]._set > hd._setmax)
			hd._setmax = msg[i]._set;
	}

	total_msgs = i;

	msg_offset = total_msgs * sizeof(struct _msgptr) + 	
	 	     /*- base of the _message text -*/
	 	     sizeof(struct _header) + 
		     total_sets * 2 * sizeof(unsigned short);
	hd._magic = CAT_MAGIC;
	hd._n_sets = total_sets;
	
	/*---- mark catalog with name of present codeset ----*/
	codeset = nl_langinfo(CODESET);
	strncpy(hd._filler, codeset, sizeof(hd._filler));
	hd._filler[sizeof(hd._filler)-1] = '\0';

	fwrite(&hd,header_size,1,file);	
	/*---- write the header to the file ----*/

	for (i = 0 ; i < total_msgs ; i++) {
	/*---- write the index table to the file ----*/

		if (!i || msg[i]._set != msg[i - 1]._set) {
		/*---- when the set changes ----*/

			fwrite(&msg[i]._set,2,1,file);	
			/*---- set number ----*/

			for (j = 0 ; j + i < total_msgs ; j++) {
				if (msg[i + j]._set != msg[i]._set) 
					break;
			}
			fwrite(&j,2,1,file);	
			/*---- number of _messages ----*/
		}
		mp._msgno = msg[i]._msg;	
                /*---- write an 'mp' for each _message -----*/

		mp._msglen = strlen(msg[i]._text);
		mp._offset = msg_offset;
		fwrite(&mp,sizeof(mp),1,file);
		msg_offset += mp._msglen + 1;
	}
	if ((file != stdout) && (ftell(file) != total_msgs * sizeof(struct _msgptr) + 
				sizeof(struct _header) + 
				total_sets * 2 * sizeof(unsigned short)))
		die( catgets(catderr, MS_GENCAT, M_MSG_11, "gencat:  internal error.") );
                /*---- file pointer consistency check ----*/

	for (i = 0 ; i < total_msgs ; i++) {
		fwrite(msg[i]._text,strlen(msg[i]._text) + 1,1,file);
	}
	if ((file != stdout) && (ftell(file) != msg_offset))
		die( catgets(catderr, MS_GENCAT, M_MSG_12, 
                                          "gencat: internal error (bad file position)") );
                     /*---- file pointer consistency check  ----*/
}

/*
 * NAME: msg_comp
 *
 * FUNCTION: Compare _message structures and return a value which is
 *           appropriate for qsort.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 	a > b : 1
 *		a < b : -1
 *		a = b : 0
 */

#ifdef _NO_PROTO
int msg_comp(a, b)
struct _message *a; 
struct _message *b;
#else
int msg_comp(struct _message *a, struct _message *b)
#endif /* _NO_PROTO */

	/*---- msg_comp: used by qsort and get_msg ----*/
	/*---- a,b: the two _messages to be compared ----*/

{
	if (a->_set != b->_set)
		return(a->_set - b->_set);
	else 
		return(a->_msg - b->_msg);
}




/*
 * NAME: load_cat
 *
 * FUNCTION: Uses catopen to open a .cat file. Reformats the catd 
 *           structure into the emsg array, and closes the .cat file.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */


#ifdef _NO_PROTO
void load_cat(tcat) 
char *tcat; 
#else
void load_cat(char *tcat)
#endif /* _NO_PROTO */

	/*---- tcat: catalog name to be loaded ----*/

{
	nl_catd catd;		/*---- catalog descriptor ----*/
	struct _msgptr mpt;	/*---- catd style _message pointer ----*/
	int i,j;		/*---- Misc counter(s) used for loops ----*/
	char cat[PATH_MAX];

	if (strchr(tcat,'/')) {
		strcpy(cat,tcat);
	}
	else {
		sprintf(cat,"./%s",tcat);
	}
	

	if (access(cat,R_OK))
		return;
	catd = catopen(cat , NL_CAT_LOCALE);
	catgets(catd, 1, 1, "");
	if (catd == CATD_ERR || catd->_fd == (FILE *)-1) {
		fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_13, "Unable to load specified catalog. \n \t %s \n"),tcat);
		exit (1);
 	             /*---- target cat exists, but is not a real cat ----*/
	}

/*______________________________________________________________________
	Reorder the catd structures into the emsg style[] structure
	while expanding emsg as needed.
  ______________________________________________________________________*/

	for (i = 0 ; i <= catd->_hd->_setmax ; i++ ) {
		for (j = 1 ; j <= catd->_set[i]._n_msgs ; j++) {
			if (catd->_set[i]._mp[j]._offset) {
				mpt = catd->_set[i]._mp[j];
				fseek(catd->_fd,mpt._offset,0);
				bump_msg();
				emsg[current]._text = (char *) rmalloc(mpt._msglen + 1);
				if (!fread(emsg[current]._text,mpt._msglen + 1,1,catd->_fd)) {
					fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_14, "Unable to read old catalog file. \n \t %s\n"),tcat);
					exit (1);
				}
				emsg[current]._set = i;
				emsg[current]._msg = j;
				emsg[current]._old = TRUE;
			}
		}
	}
	catclose(catd);
}



/*
 * NAME: open_source
 *
 * FUNCTION: Opens a source stream.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the source stream.
 */


#ifdef _NO_PROTO
FILE *open_source(file) 
char *file; 
#else
FILE *open_source(char *file)
#endif /* _NO_PROTO */

{
	FILE *f;

	if (strcmp(file,"-") == 0)
		f = stdin;
	else
		if (!(f = fopen(file,"r"))) {
			fprintf(stderr, catgets(catderr, MS_GENCAT, M_MSG_1,
				"Gencat: Unable to open %s\n") ,file);
			exit(1);
		}
	return(f);
}


/*
 * NAME: rmalloc
 *
 * FUNCTION: Performs a malloc with some error checking.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the result of the malloc.
 */ 


#ifdef _NO_PROTO
char *rmalloc(n) 
int n; 
#else
char *rmalloc(int n)
#endif /* _NO_PROTO */

	/*----  n: the number of bytes to be malloc'ed  ----*/
{
	char *t;

	t = (char *) malloc(n);
	if (!t)
		die( catgets(catderr, MS_GENCAT, M_MSG_15, "Unable to malloc memory.") );
	return(t);
}


/*
 * NAME: skip_to_white
 *
 * FUNCTION: Locate the next character less than or equal to a blank
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the next blank (or lesser) character
 */ 


#ifdef _NO_PROTO
char *skip_to_white(p)
char *p;
#else
char *skip_to_white(char *p)
#endif /* _NO_PROTO */
{
	int	len;		/* # bytes in next character */
	wchar_t	wc;		/* process code of next character */

	while (*p) {
		len = mbtowc(&wc, p, MB_CUR_MAX);
		if (len < 0) {
			len = 1;
			wc = *p & 0xff;
		}
		if (wc <= ' ')
			return (p);
		p += len;
	}
	return (p);
}


/*
 * NAME: skip_to_nwhite
 *
 * FUNCTION: Locate the next "nonblank" character
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the next nonblank character
 */ 


#ifdef _NO_PROTO
char *skip_to_nwhite(p)
char *p;
#else
char *skip_to_nwhite(char *p)
#endif /* _NO_PROTO */
{
	int	len;		/* # bytes in next character */
	wchar_t	wc;		/* process code of next character */

	while (*p) {
		len = mbtowc(&wc, p, MB_CUR_MAX);
		if (len < 0) {
			len = 1;
			wc = *p & 0xff;
		}
		if (iswblank(wc) == 0)
			return (p);
		p += len;
	}
	return (p);
}



#ifdef _BLD_OSF /*** routines needed for boot strap ****/



/************************************************************************/
/*									*/
/*				NOTE TO OSF				*/
/*									*/
/* THE FOLLOWING CODE BOUND BY #IFDEF _BLD_OSF HAS NOT BEEN MODIFIED	*/
/* FOR AIX V3.2								*/
/*									*/
/************************************************************************/

/*
 * 
 * NAME: catopen
 *                                                                    
 * FUNCTION: Opens a catalog and return a valid nl_catd
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	Catopen executes under a process.	
 *
 * NOTES:  Catopen does not always open the catalog. When a user issues a 
 * 	close on a catalog, the file is not closed.  The close on exec flag 
 * 	is set, but the file is not closed, nor are the data structures 
 *	assocciated with the catalog freed. If a second open is issued against 
 * 	the same catalog name, the catalog does not actually have to be opened.
 *      This is implemented primarily to prevent overuse of NLgetamsg from 
 *	producing an unacceptable number of opens and closes.
 *
 * RETURNS: Returns a pointer to a CATD. 
 *          Returns a (nl_catd) -1 if fails.
 *
 */  

nl_catd catopen (cat, dummy)
char *cat; 
int dummy;
	/*---- char *cat:  the name of the cat to be opened ----*/
	/*---- int dummy:  dummy variable  ----*/

{
        int errno_save;
	nl_catd _do_open();  	    /*---- routine that actually opens 
					   the catalog ---- */
	CATD *catd;

        errno_save = errno;

	if (catd = cat_already_open(cat)) {
		catd->_count = catd->_count + 1;
		return(catd);
	}
	catd = (CATD *)rmalloc (sizeof(CATD));
	if ( catd == NULL )
		return(CATD_ERR);
	catd->_name = (char *)rmalloc(strlen(cat) + 1);
	if ( catd->_name == NULL )
		return(CATD_ERR);
	strcpy(catd->_name,cat);
	catd->_fd = FALSE;
	catd->_mem = FALSE;
	catd->_pid = getpid();
	catd->_count = 1;
	if (_do_open(catd) != CATD_ERR)  
		return(catd);
	else {
		free(catd->_name);
		free(catd); 
		return(CATD_ERR);
	}
}



/*
 * NAME: NLcatopen
 *                                                                    
 * FUNCTION: Sets up a deferred open for a catalog. If the catalog 
 *	is referenced, the partial open started here will be completed by
 *	_do_open.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	NLcatopen executes under a process.	
 *
 * RETURNS: Returns a pointer to CATD
 *
 */  
 
nl_catd NLcatopen(cat, dummy)
char *cat; 
int dummy;

	 	/*---- name of the catalog to be opened ----*/
		/*----  dummy variable  ----*/

{ 
        int errno_save;
	CATD *catd;

        errno_save = errno;

	if (catd = cat_already_open(cat)) {
  		catd->_count = catd->_count + 1;
		return(catd);
	}
	catd = (CATD *)rmalloc (sizeof(CATD));
	if ( catd == NULL )
		return(CATD_ERR);
	catd->_name = (char *)rmalloc(strlen(cat) + 1);
	if ( catd->_name == NULL )
		return(CATD_ERR);
	strcpy(catd->_name,cat);
	catd->_fd = FALSE;
	catd->_mem = FALSE;
	catd->_pid = getpid();
	catd->_count = 1;
	return(catd);
}

 


/*
 * 
 * NAME: _do_open
 *                                                                    
 * FUNCTION: Opens a catalog file, reads in and builds an index table.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 * 	_do_open executes under a process.	
 *
 * NOTES: _do_open does all the necessary operations for catopen() 
 *	and NLcatopen().
 *
 * RETURNS: Returns a pointer to a CATD structure (nl_catd)
 *	If the open fails, _do_open returns a NULL pointer.
 *
 */  


nl_catd _do_open(catd)
nl_catd catd;

	/*---- pointer to the partially set up cat descriptor ----*/

{
	void make_sets();	/*---- routine to unpack the sets into 
						fast acccess mode ----*/
	void add_open_cat();	/*---- routine to keep a list of 
                                               opened cats ----*/
	long int magic;
	int i;			/*---- Misc counter(s) used for loop */
	struct _catset cs;
        int errno_save;

        errno_save = errno;

	catd->_fd = opencatfile( catd->_name );
	if ( !catd->_fd ) {
		return( CATD_ERR );
	}
	fread((void *)&magic,(size_t)4,(size_t)1,catd->_fd);
	if (magic != CAT_MAGIC){
		fclose(catd->_fd);
		catd->_fd = NULL;
		return( CATD_ERR );
	}

	if (1) {      /* disable the shmat, share memory segemnt */

/*______________________________________________________________________
	If the file can not be mapped then simulate mapping for the index
	table so that make_sets cat set things up. (rmalloc an area big
 	enough for the index table and read the whole thing in)
  ______________________________________________________________________*/

		fseek(catd->_fd,(long)0,0);
		catd->_hd = (struct _header *) rmalloc(sizeof(struct _header));
		if ( catd->_hd == NULL )
			return(CATD_ERR);
		fread((void *)catd->_hd,(size_t)sizeof(struct _header),(size_t)1,catd->_fd);

		for (i = 0 ; i < catd->_hd->_n_sets ; i++) {
			fread((void *)&cs,(size_t)4,(size_t)1,catd->_fd);
			fseek(catd->_fd, (long)(cs._n_msgs * sizeof(struct _msgptr)),1);
		}

		i = ftell(catd->_fd);
		catd->_mem = (char *)rmalloc(i);
		if ( catd->_mem == NULL )
			return(CATD_ERR);
		fseek(catd->_fd,(long)0,0);
		fread((void *)catd->_mem,(size_t)i,(size_t)1,catd->_fd);
		catd->_set = (struct _catset *) rmalloc((catd->_hd->_setmax+1)* 
                              sizeof (struct _catset));
		if ( catd->_set == NULL )
			return(CATD_ERR);
		catd->_setmax = catd->_hd->_setmax;
		make_sets(catd->_set,catd->_mem,catd->_hd->_n_sets);
		free(catd->_mem);
		catd->_mem = FALSE;
		add_open_cat(catd);
		return(catd);
	}
	else {

/*______________________________________________________________________
	Normal mapping has occurred, set a few things up and call make_sets
  ______________________________________________________________________*/

		catd->_hd =( struct _header * )( catd->_mem );
		catd->_setmax = catd->_hd->_setmax;
		catd->_set = (struct _catset *) rmalloc((catd->_hd->_setmax+1)*
                             sizeof (struct _catset));
		if ( catd->_set == NULL )
			return(CATD_ERR);
		make_sets(catd->_set,catd->_mem,catd->_hd->_n_sets);
		add_open_cat(catd);
		return(catd);
	}
}



/*
 * 
 * NAME: make_sets
 *
 * FUNCTION: Expands the compacted version of the catalog index table into
 *	the fast access memory version.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Make_set executes under a process.	
 *
 * RETURNS: void
 */


static void make_sets(cset, base, n_sets)
struct _catset *cset; 
char *base; 
int n_sets;

	/*---- cset: place to store the sets ----*/
	/*---- base: base of the catalog memory (mapped or not) ----*/
	/*---- n_sets: number of sets in _header table ----*/

{
	int 	i;	/*---- Misc counter(s) used for loops ----*/
	int	j;	/*---- Misc counter(s) used for loops ----*/
	int 	msgmax;	/*---- The maximum number of _messages in a set ----*/
	char 	*cmpct_set_ptr;	/*---- pointer into the index table ----*/
	struct _catset	cs;	/*---- used to look at the sets in the table -*/

	cmpct_set_ptr = base + sizeof(struct _header);

	for (i = 0 ; i < n_sets ; i++) {
	    /* loop through each compacted set */

		cs = *(struct _catset *)cmpct_set_ptr;	
                /* set the _catset ptr to the base of the current 
                   compacted set.        */

		cs._mp = (struct _msgptr *)(cmpct_set_ptr +
                          2 * sizeof(unsigned short));
                          /* set the ms array ptr to the base of
			     compacted array of _msgptr's     */

		for (j = 0 ,msgmax = 0 ; j < cs._n_msgs ; j++) {
	            /* find the highest msgno in the set */

			if (cs._mp[j]._msgno > msgmax)
				msgmax = cs._mp[j]._msgno;
		}
		msgmax++;   /* allocate memory for the expanded 
			       array (this one will have holes) */

		cset[cs._setno]._mp = (struct _msgptr *) rmalloc(msgmax *
                                       sizeof(struct _msgptr));
		cset[cs._setno]._msgtxt = (char **)rmalloc(msgmax *
                                           sizeof (char **));

		for (j = 0 ; j < msgmax ; j++)  
                    /* mark all the _msgptr's as being empty */

			cset[cs._setno]._mp[j]._offset = FALSE;
		for (j = 0 ; j < cs._n_msgs ; j++) {    
                     /* Fill the appropriate ones with data */

			cset[cs._setno]._mp[cs._mp[j]._msgno] = cs._mp[j];
		}
		cset[cs._setno]._n_msgs = msgmax;	
		cset[cs._setno]._setno = cs._setno;
       	        /* Superfluous but should have the correct data. Increment 
                   the base of the set pointer.          */

		cmpct_set_ptr += 2 * sizeof(unsigned short) + cs._n_msgs *
                                 sizeof(struct _msgptr);
	}
}



/*
 * 
 * NAME: opencatfile
 *
 * FUNCTION: Opens a catalog file, looking in the language path first (if 
 *	there is no slash) and returns a pointer to the file stream.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	Opencatfile executes under a process.	
 *
 * RETURNS:  Returns a pointer to the file stream, and a NULL pointer on
 *	failure.
 */

static FILE *opencatfile(file)
char *file;
{
	extern char *getenv();
	char 	fl[PATH_MAX];	/*---- place to hold full path ----*/
	char 	*nlspath;	/*---- pointer to the nlspath val ----*/
	FILE	*fp;		/*---- file pointer ----*/
	char	cpth[PATH_MAX]; /*---- current value of nlspath ----*/
	char    *p,*np;
	char	*fulllang;	/* %L language value */
	char	lang[PATH_MAX]; /* %l language value */
	char	*territory;	/* %t language value */
	char	*codeset;	/* %c language value */
	char	*ptr;		/* for decompose of $LANG */
	char *str;
	char *optr;
	int nchars;
	int lenstr;
	char outptr[PATH_MAX];
	int valid;

	if (strchr(file,'/')) {
                if ((fp = fopen(file,"r")))
                   {  fcntl(fileno(fp),F_SETFD,1);
                                /* set the close-on-exec flag for
                                    child process                */
                      return(fp);
                   }
	}
	else {
		if (!(nlspath = getenv("NLSPATH")))
			nlspath = PATH_FORMAT; 
   		if (!(fulllang = getenv("LANG"))) 
			fulllang = DEFAULT_LANG;
		if (fulllang == DEFAULT_LANG)	                      
			nlspath = PATH_FORMAT;  /* if fullang is C, use the 
	   					   the default nlspath:    */

		/*
		** LANG is a composite of three fields:
		** language_territory.codeset
		** and we're going to break it into those
		** three fields.
		*/

		strcpy(lang, fulllang);

		territory = "";
		codeset = "";

		ptr = strchr( lang, '_' );
		if (ptr != NULL) {
			territory = ptr+1;
			*ptr = '\0';
			ptr = strchr(territory, '.');
			if (ptr != NULL) {
				codeset = ptr+1;
				*ptr = '\0';
			}
		} else {
			ptr = strchr( lang, '.' );
			if (ptr != NULL) {
				codeset = ptr+1;
				*ptr = '\0';
			}
		}

		np = nlspath;
		while (*np) {
			p = cpth;
			while (*np && *np != ':')
				*p++ = *np++;
			*p = '\0';
			if (*np)	/*----  iff on a colon then advance --*/
				np++;
			valid = 0;
			if (strlen(cpth)) {
				ptr = cpth;
				optr = outptr;

				nchars = 0;
				while (*ptr != '\0') {
					while ((*ptr != '\0') && (*ptr != '%') 
						      && (nchars < PATH_MAX)) {
						*(optr++) = *(ptr++);
						nchars++;
					}
					if (*ptr == '%') {
						switch (*(++ptr)) {
							case '%':
								str = "%";
								break;
							case 'L':
								str = fulllang;
								break;
							case 'N':
								valid = 1;
			    					str = file;
								break;
							case 'l':
								str = lang;
								break;
							case 't':
								str = territory;
								break;
							case 'c':
								str = codeset;
								break;
							default:
								str = "";
								break;
						}
						lenstr = strlen(str);
						nchars += lenstr;
						if (nchars < PATH_MAX) {
							strcpy(optr, str);
							optr += lenstr;
						} else {	
							break;
						} 
						ptr++;
					} else {
						if (nchars >= PATH_MAX) {
							break;
						}
					}
				}
				*optr = '\0';
				strcpy(cpth, outptr);
			}
			else {		/*----  iff leading | trailing | 
                                                adjacent colons ... --*/
				valid = 1;
				strcpy(cpth,file);
			}
			if (valid == 1 && (fp = fopen(cpth,"r")))
                            {  fcntl(fileno(fp),F_SETFD,1);
                                        /* set the close-on-exec flag for
                                           child process                */
                               return(fp);
                            }
		}
                if (fp = fopen(file,"r"))
                   {  fcntl(fileno(fp),F_SETFD,1);
                                 /* set the close-on-exec flag for
                                    child process                */
                       return(fp);
                    }
	}
	return (NULL);
}

 

 

/*
 * 
 * NAME: cat_already_open
 *
 * FUNCTION: Checkes to see if a specific cat has already been opened.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	Cat_already_open executes under a process.
 *
 * RETURNS: Returns a pointer to the existing CATD if one exists, and 
 *	a NULL pointer if no CATD exists.
 */

static nl_catd cat_already_open(cat)
char *cat;
			/*---- name of the catalog to be opened ----*/

{
	int i;			/*---- Misc counter(s) used for loops ----*/
	
	for (i = 0 ; i < NL_MAXOPEN && catsopen[i] ; i++)  {
		if (!strcmp(cat,catsopen[i]->_name) && getpid()==catsopen[i]->_pid) {
		       	return(catsopen[i]);
		}
	}
	return(0);
}



/*
 * 
 * NAME: add_open_cat
 *
 * FUNCTION: Adds a cat to the list of already opened catalogs.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	Add_open_cat executes under a process.	
 *
 * RETURNS: void
 */

static void add_open_cat(catd)
nl_catd catd;
	 	/*---- catd to be added to the list of catalogs ----*/

{
	int i = 0;	/*---- Misc counter(s) used for loops ----*/
	while (i < NL_MAXOPEN && catsopen[i]) {
		if (!strcmp(catd->_name,catsopen[i]->_name) && 
		    getpid()==catsopen[i]->_pid)
			return;	/*---- The catalog is already here ----*/
		i++;
	}

	if (i < NL_MAXOPEN) {
		catsopen[i] = catd;
		catpid[i] = getpid();
	}
}



/*
 * 
 * NAME: catclose
 *                                                                    
 * FUNCTION: Closes catalog. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	catclose executes under a process.	
 *
 * NOTES: Catclose closes the stream and frees the memory of the catalog.
 *
 * RETURNS: 0 on success, -1 on failure.
 *
 */  


int catclose(catd)		/*---- the catd to be closed ----*/
nl_catd catd;		/*---- the catd to be closed ----*/

{
	int i;


	if (catd == CATD_ERR)
		return(-1);
	for (i=0; i< NL_MAXOPEN && catsopen[i]; i++) {
		if (catd == catsopen[i] && getpid()==catsopen[i]->_pid)
			break;
	}
	if (i == NL_MAXOPEN || catsopen[i] == NULL)
		return (-1);
	if (catd->_fd == (FILE *)NULL)	
				/*----  return if this is an extra open or
					a bad catalog discriptor         ----*/
		return(-1);
	if (cat_already_open(catd->_name)) {
		if (catd->_count == 1) {
			cat_hard_close(catd);
	        	return (0); 	/*--- the last legal clsoe ---*/
		}
	 	else if (catd->_count > 1) {
			catd->_count = catd->_count - 1;
			return(0);	/*--- a legal close ---*/
		}
		else	
			return (-1);	/*--- an extra illegal close ---*/
	}
	else {
		return(-1);
	}
}
 



/*
 * NAME: cat_hard_close
 *
 * FUNCTION: Closes a catalog and frees the memory with it. Deletes the catd
 *	from the list of open catalogs.
 *
 * EXECUTION ENVIRONMENT:
 * 	Cat_hard_close executes under a process.	
 *
 * RETURNS: void
 */


static void cat_hard_close(catd)
nl_catd catd;
		/*---- the catd to be closed ----*/

{
	int i;			/*---- Misc counter(s) used for loops ----*/
	int j;			/*----  Misc counter ----*/

	if (catd == CATD_ERR)
		return;
	
/*______________________________________________________________________
	remove any entry for the catalog in the catsopen array
  ______________________________________________________________________*/

	for (i = 0 ; i < NL_MAXOPEN && catsopen[i] ; i++) {
		if (catd == catsopen[i]) {
			for (; i < NL_MAXOPEN-1; i++) {
			    catsopen[i] = catsopen[i+1];
			    catpid[i] = catpid[i+1];
			}
			catsopen[i] = NULL;
			catpid[i] = 0;
		}
	}

/*______________________________________________________________________
	close the cat and free up the memory
  ______________________________________________________________________*/
	if (catd->_mem == FALSE)
	{
	for (i = 0 ; i < catd->_hd->_setmax ; i++) {
		if (catd->_set[i]._mp) 
			free(catd->_set[i]._mp);   
                        /*---- free the _message pointer arrays ----*/

		if (catd->_set[i]._msgtxt) {
			for (j = 0 ; j < catd->_set[i]._n_msgs ; j++) {
				if (catd->_set[i]._msgtxt[j])
					free(catd->_set[i]._msgtxt[j]);
			}
			free(catd->_set[i]._msgtxt);
		}
	}
	}

	if (catd->_fd)
		fclose(catd->_fd); 	/*---- close the ctatlog ----*/
	if (catd->_set) 
		free(catd->_set);	/*---- free the sets ----*/
	if (catd->_name)
		free(catd->_name);      /*---- free the name ----*/
	if (catd->_hd)
		free(catd->_hd);	/*---- free the header ----*/
	if (catd)
		free(catd);		/*---- free the catd ----*/
}

/*
 * NAME: substitute
 *
 * FUNCTION: Replaces occurrences of one string within another.
 *           Substitute value1 in line with value2.
 *
 * EXECUTION ENVIRONMENT:
 *  User mode.
 *
 * RETURNS: void
 */


static void substitute( line, val1, val2 )
char line[];
char val1[];
char val2[];
{
  int i, j;
  int v1_len,
  v2_len,
  ln_len;

  v1_len = strlen( val1 );
  v2_len = strlen( val2 );
  ln_len = strlen( line );

  for ( i = 0 ; i < ln_len ; i++ ) {
    if ( !memcmp( &line[i], val1, v1_len ) ) {
      memmove(&line[i + v2_len], &line[i + v1_len], strlen(&line[i + v1_len]) + 1 );
      memcpy(&line[i],val2,v2_len);
      return;
    }
  }
}

/*
 * NAME: memmove
 *
 * FUNCTION: Moves memory within overlapping areas.
 *
 * EXECUTION ENVIRONMENT:
 *  User mode.
 *
 * RETURNS: void
 */


static void memmove(t,s,n)
char *t;
char *s;
int n;
{
  char buf[1028];

  memcpy(buf,s,n);
  memcpy(t,buf,n);
}

#endif /*** _BLD_OSF ***/
