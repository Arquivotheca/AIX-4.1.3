/*
 * "@(#)56  1.3  src/bos/usr/bin/mirror/modem.h, cmdmirror, bos411, 9428A410j 4/22/94 02:31:59"
 * COMPONENT_NAME: CMDMIRROR: Console mirroring
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>

#define MAX_SIZE_NAME 50	/* max identifier length */
#define MAX_SIZE_STRING 256	/* max characters between double quote */

/* values for timeout */
#define NONE_VALUE	-1
#define N_VALUE		-2
#define DEFAULT_VALUE	-3

/* types of commandes */
#define CMD_SEND 1
#define CMD_EXPECT 2
#define CMD_IGNORE 3
#define CMD_DELAY 4

/* list of strings structure */
typedef struct st_strlst_t {
	char *str;
	int cur_char;	/* use to scan the result from the modem */
	struct st_strlst_t *next;
	} strlst_t;

typedef struct st_command_t {
	int type;	/* type of command : CMD_SEND, CMD_EXPECT ... */
	int val;	/* value for delay or timeout */
	char *str;	/* string for send or for busy */
	strlst_t *strlst;	/* string list for expect or ignore */
	int abs;	/* boolean value for expect */
	struct st_command_t *next;
	} command_t;

typedef struct st_script_t {
	char *name;
	command_t *cmd;
	struct st_script_t *next;
	} script_t;


extern script_t *first_script;	/* contain the firrst script after yyparse() */
extern int icdelay;	/* inter-commands delay */
extern int defaultto;	/* default time-out */
extern int calldelay;	/* call delay */
extern int line_number;	/* current line number in the modem file */

extern void init_parser (FILE *fd);		/* must be run before yyparse */
extern int yyparse ();		/* run the analises */

