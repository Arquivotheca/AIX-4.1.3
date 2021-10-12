/* @(#)74	1.3  src/bos/usr/bin/capture/vtparse.h, cmdsh, bos411, 9428A410j 2/11/94 17:08:52 */
/* COMPONENT_NAME: (CMDSH) Shell related commands 
 *
 * FUNCTIONS:
 *
 * ORIGINS: 10  26  27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * The following are #defines for different tokens recognized by this
 * parser.  This value is returned in the v_type field of the returned
 * structure of type "vcmd_t".

 * A "STRING" is a sequence of characters that do NOT match the machine
 * (ie, not a valid vt100 sequence).  This is used to allow the calling
 * routine to put the characters on the display to be displayed normally.
 * The characters to be put back will be on "v_data.string" and will only
 * be in the LSB of each int, so a printf will not work.

 * A "CMD" is a valid vt-100 "normal" escape sequence, such as "\033[2J"
 * or "\033[2;4r"  The arguments (all integers) will be stored on
 * "v_data.args" and the command letter will be the character addressed by
 * v_data.args[v_len] (in the above two cases, a 'J' and 'r' respectively).
 * This is a char, but automagically cast into an int and taking up a full
 * int's space.

 * An "SCMD1" is a short command.  There are only a couple of these.
 * It is of the form: "\033c" where 'c' is a character (either "DME78H=>c").
 * The 'c' is stored in "v_data.args[0]" and v_len == 1.

 * An "SCMD2" is another short command.  It is in the form of:
 * "\033#d" where '#' is the actual pigpen character and 'd' is
 * a digit from 0-9.  Again, "v_data.args[0]" holds the ascii representation
 * for the character (0x30-0x39) and v_len == 1.

 * An "SCMD3" is of the form "\033(C" where 'C' is any letter or digit. 
 * v_data.args[0] holds 'C' and v_len == 1.

 * An "CMDX" is of the form "\033[?nC" where 'n' is a number parameter, 'C'
 * is any *letter* and '?' is the literal character '?'.  Returns arguments
 * back in the same form as "CMD".

 * If an EOF is received during the input (indicated by a -1 from netinput()),
 * then we will return with v_len = 0 and v_type = V_STRING.
 * There are only a few SCMD1 and SCMD2 types in the vt-100 language,
 * but they must still be supported in a complete implementation of
 * an emulator.

 *
 * In summary:
 * Patterns recognized by this parser and values returned:
 *
 * \e[nn;nnL			V_CMD
 * \e[?nn;nnL  L = [A-Za-z]	V_CMDX
 * \eC	C = {FGDMIE78HZc=<>}	V_SCMD1
 * \e#D D = [0-9]		V_SCMD2
 * \e(C C = {[0-9A-Za-z]}	V_SCMD3
 */

#define V_STRING	0
#define V_CMD		1
#define V_CMDX		2
#define V_SCMD1		3
#define V_SCMD2		4
#define V_SCMD3		5
#define V_SCMD4		6

#define SCMD1STR	"FGDMIE78HZc=<>" /* valid \033c characters */

typedef struct {
	unsigned char	v_type;		/* 0 = STRING,1=CMD,2=SCMD1,3=SCMD2 */ 
	unsigned char	v_len;		/* length of {string_len|arg_array} */
	union {
		int	*args;		/* arguments for the cmd */
		char	*string;	/* string to be rejected sans ESC */
	} v_data;
} vcmd_t;	/* vt100 cmd */

extern int vtparse();
