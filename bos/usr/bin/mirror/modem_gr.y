%{
static char sccsid[] = "@(#)57  1.3  src/bos/usr/bin/mirror/modem_gr.y, cmdmirror, bos411, 9430C411a 7/12/94 02:52:54";
/*
 * COMPONENT_NAME: CMDMIRROR: Console mirroring
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <fcntl.h>
#include "modem.h"

typedef union {
	int intval;
	char *strng;
	strlst_t *strlst;
	command_t *cmd;
	} YYSTYPE;

typedef struct st_ptrlst_t {
	char *ptr;
	struct st_ptrlst_t *next;
	} ptrlst_t;

static FILE *modem_file;

script_t *first_script = NULL;
int icdelay = -1;
int defaultto = 0;
int calldelay = 0;
int line_number = 1;

static ptrlst_t *first_ptr = NULL;	/* list of all memory allocated */
static int last_c;

static char *memory_alloc (size_t sz);

%}

%term ICDELAY DEFAULTTO CALLDELAY
%term <intval>    NUMBER
%term <strng>    STRING NAME
%term SEND EXPECT DELAY BUSY TIMEOUT IGNORE DONE NONE N ABS OR COLON
%term ERROR


%{

typedef struct key_word {
	char *name;
	int id;
	};

struct key_word tab_key_word [] = {
	{"send", SEND},
	{"expect", EXPECT},
	{"abs", ABS},
	{"or", OR},
	{"busy", BUSY},
	{"timeout", TIMEOUT},
	{"none", NONE},
	{"N", N},
	{"delay", DELAY},
	{"ignore", IGNORE},
	{"done", DONE},
	{"ICDelay", ICDELAY},
	{"DefaultTO", DEFAULTTO},
	{"CallDelay", CALLDELAY},
	{NULL, 0}
	};
%}

%type <intval>   numberofsecondes delayvalue timeoutvalue optionaltimeout
%type <intval>   optionalabs
%type <strng>   optionalbusy scriptname
%type <strlst>   stringlist
%type <cmd>   commandlist command
%type <cmd>   sendcommand expectcommand ignorecommand delaycommand

%%

controlfile :  itemlist 
	;

itemlist :  itemlist  script 
	|  itemlist  variablesetting
	|
	;

variablesetting :  icdelaysetting 
	|  defaulttosetting
	|  calldelaysetting
	;

icdelaysetting :  ICDELAY  numberofsecondes
	{
		icdelay = $2;
	}
	;

defaulttosetting :  DEFAULTTO  numberofsecondes
	{
		defaultto = $2;
	}
	;

calldelaysetting :  CALLDELAY  numberofsecondes
	{
		calldelay = $2;
	}
	;

script :  scriptname  COLON  commandlist  DONE
		{
			script_t *sc;
			script_t *p;

			sc = (script_t *)memory_alloc (sizeof (script_t));
			sc->name = $1;
			sc->cmd = $3;
			sc->next = NULL;
			if (first_script == NULL) {
				first_script = sc;
			}
			else {
				p = first_script;
				while (p->next != NULL) p = p->next;
				p->next = sc;
			}
		}
	;

commandlist :  commandlist  command
		{
			command_t *p;

			if ($1 == NULL) {
				$$ = $2;
			}
			else {
				p = $1;
				while (p->next != NULL) p = p->next;
				p->next = $2;
				$$ = $1;
			}
		}
	| 
		{
			$$ = NULL;
		}
	;

command :  sendcommand
		{ $$ = $1; }
	|  expectcommand
		{ $$ = $1; }
	|  ignorecommand
		{ $$ = $1; }
	|  delaycommand
		{ $$ = $1; }
	;

sendcommand :  SEND  STRING
		{
			$$ = (command_t *)memory_alloc (sizeof (command_t));
			$$->type = CMD_SEND;
			$$->str = $2;
			$$->next = NULL;
		}
	;

delaycommand :  DELAY  delayvalue
		{
			$$ = (command_t *)memory_alloc (sizeof (command_t));
			$$->type = CMD_DELAY;
			$$->val = $2;
			$$->next = NULL;
		}
	;

delayvalue :  numberofsecondes
		{
			$$ = $1;
		}
	|  N
		{
			$$ = N_VALUE;
		}
	;

expectcommand :  EXPECT  optionalabs  stringlist  optionalbusy  optionaltimeout
		{
			$$ = (command_t *)memory_alloc (sizeof (command_t));
			$$->type = CMD_EXPECT;
			$$->abs = $2;
			$$->strlst = $3;
			$$->str = $4;
			$$->val = $5;
			$$->next = NULL;
		}
	;

optionalabs :  ABS 
		{
			$$ = 1;
		}
	|
		{
			$$ = 0;
		}
	;

optionalbusy :  BUSY  STRING 
		{
			$$ = $2;
		}
	|
		{
			$$ = NULL;
		}
	;

optionaltimeout :  TIMEOUT  timeoutvalue
		{
			$$ = $2;
		}
	|
		{
			$$ = DEFAULT_VALUE;
		}
	;

timeoutvalue :  delayvalue
		{
			$$ = $1;
		}
	|  NONE
		{
			$$ = NONE_VALUE;
		}
	;

stringlist :  stringlist  OR  STRING
		{
			strlst_t *p;

			p = $1;
			while (p->next != NULL) p = p->next;
			p->next = (strlst_t *)memory_alloc (sizeof (strlst_t));
			p->next->str = $3;
			p->next->next = NULL;
			$$ = $1;
		}
	|  STRING
		{
			$$ = (strlst_t *)memory_alloc (sizeof (strlst_t));
			$$->str = $1;
			$$->cur_char = 0;
			$$->next = NULL;
		}
	;

ignorecommand :  IGNORE  stringlist  optionaltimeout
		{
			$$ = (command_t *)memory_alloc (sizeof (command_t));
			$$->type = CMD_IGNORE;
			$$->strlst = $2;
			$$->val = $3;
			$$->next = NULL;
		}
	;
	;

numberofsecondes :  NUMBER
		{
			if ($1 <= 0) {
				printf ("number of secondes error \n");
				$$ = 0;
			}
			else {
				$$ = $1;
			}
		}
	;

scriptname :  NAME
		{
			$$ = $1;
		}
	;

%%


/*************************************************************************
 * NAME		: yylex
 * DESCRIPTION	: This function is called by yyparse
 * PARAMETERS	: none
 * RETURN VALUE	: A token of the grammar
 ************************************************************************/

yylex ()
{
	struct key_word *p;
	int c;
	int in_name = 0;
	int in_number = 0;
	int in_string = 0;
	int in_comments = 0;
	static char name [MAX_SIZE_NAME+1];
	int size_name;
	static char string [MAX_SIZE_STRING+1];
	int size_string;
	int number;
	int backslash = 0;

	for (;;) {
		if (last_c == 0) {
			c = fgetc (modem_file);
		}
		else {
			c = last_c;
			last_c = 0;
		}
		if (in_name) {
			if (isalnum (c) || c == '_') {
				if (size_name >= MAX_SIZE_NAME) {
					return ERROR;
				}
				name [size_name] = c;
				size_name ++;
				continue;
			}
			else {  /* not an alphanumerique character */
				last_c = c;
				name [size_name] = '\0';
				in_name = 0;
				p = tab_key_word;
				while ((*p).name != NULL) {
					if (!strcmp (name, (*p).name)) {
						return (*p).id;
					}
					p ++;
				}
				yylval.strng = strdup (name);
				return NAME;
			}
		}
		else if (in_number) {
			if (isdigit (c)) {
				number *= 10;
				number += c - '0';
				continue;
			}
			else {  /* not a digit */
				last_c = c;
				in_number = 0;
				yylval.intval = number;
				return NUMBER;
			}
		}
		else if (in_string) {
			if (backslash) {
				switch (c) {
				case 'r' :
					c = '\r';
					break;
				case 'n' :
					c = '\n';
					break;
				case 't' :
					c = '\t';
					break;
				}
				string [size_string] = c;
				size_string ++;
				backslash = 0;
				continue;
			}
			else if (c == '\"') {
				string [size_string] = '\0';
				yylval.strng = strdup (string);
				in_string = 0;
				return STRING;
			}
			else {
				if (size_string >= MAX_SIZE_STRING) {
					return ERROR;
				}
				if (c == '\\') {
					backslash = 1;
					continue;
				}
				string [size_string] = c;
				size_string ++;
				continue;
			}
		}
		else if (in_comments) {
			if (c == '\n' || c == EOF) {
				last_c = c;
				in_comments = 0;
			}
			continue;
		}
		switch (c) {
		case EOF :
			return EOF;
		case ' ' :
		case '\t' :
			break;
		case '\n' :
			line_number ++;
			break;
		case '\"' :
			in_string = 1;
			size_string = 0;
			break;
		case '#' :
			in_comments = 1;
			break;
		case ':' :
			return COLON;
		default :
			if (isalpha (c) || c == '_') {
				in_name = 1;
				name [0] = c;
				size_name = 1;
			}
			else if (isdigit (c)) {
				in_number = 1;
				number = c - '0';
			}
			else {
				return ERROR;
			}
		}  /* end switch */
	}  /* end for */
}	/* yylex */



/*************************************************************************
 * NAME		: memory_alloc
 * DESCRIPTION	: Get somme memory and update the list of memory used
 * PARAMETERS	: <sz> is the size of memory
 * RETURN VALUE	: A pointer on the allocated memory if successful,
 *		NULL else
 ************************************************************************/

char *memory_alloc (size_t sz)
{
	ptrlst_t *ptrlst;	

	ptrlst = (ptrlst_t *)malloc (sizeof (ptrlst_t));
	if (ptrlst == NULL) {
		return NULL;
	}
	ptrlst->ptr = malloc (sz);
	ptrlst->next = first_ptr;
	first_ptr = ptrlst;
	return ptrlst->ptr;
}	/* memory_alloc */


/*************************************************************************
 * NAME		: free_all_memory
 * DESCRIPTION	: Free all memory allocated with the memory_alloc function
 * PARAMETERS	: none
 * RETURN VALUE	: none
 ************************************************************************/

void free_all_memory ()
{
	ptrlst_t *ptrlst;
	ptrlst_t *next;

	ptrlst = first_ptr;
	while (ptrlst != NULL) {
		free (ptrlst->ptr);
		next = ptrlst->next;
		free (ptrlst);
		ptrlst = next;
	}
	first_ptr = NULL;
	first_script = NULL;
}	/* free_all_memory */


/*************************************************************************
 * NAME		: init_parser
 * DESCRIPTION	: initializ the lexical and syntaxic analyses
 * PARAMETERS	: <fd> is the file descriptor of the file to analyse
 * RETURN VALUE	: none
 ************************************************************************/

void init_parser (FILE *fd)
{
	free_all_memory ();
	modem_file = fd;
	line_number = 1;
	icdelay = -1;
	defaultto = 0;
	calldelay = 0;
	last_c = 0;
}	/* init_parser */
