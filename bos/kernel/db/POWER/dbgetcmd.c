static char sccsid[] = "@(#)56	1.21  src/bos/kernel/db/POWER/dbgetcmd.c, sysdb, bos411, 9428A410j 3/28/94 17:56:24";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: get_cmd, expression, get_ptr, setup_segid, diagnose, findvar
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include "debvars.h"
#include "parse.h"                      /* parser structure             */
#include "debaddr.h"                    /* debugger addr  structure     */
#include "vdberr.h"			/* Error message stuff		*/
#include "dbbreak.h"			/* for variable names */
#include "dbdebug.h"
#include "dbfunc.h"

#define VARNAME		"       "
#define VARVALUE	"                               "

/* MAXVARS defined in debvars.h as size of the var_name array */
char *var_name[MAXVARS] = {	/* array for modifiable variable names */
	VARNAME, VARNAME, VARNAME, VARNAME, VARNAME, VARNAME, VARNAME, VARNAME,
	VARNAME, VARNAME, VARNAME, VARNAME, VARNAME, VARNAME, VARNAME, VARNAME,
	"fx","org",
	"s0","s1","s2","s3","s4","s5","s6","s7","s8","s9","s10",
	"s11","s12","s13","s14","s15",
	"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10",
	"r11","r12","r13","r14","r15","r16","r17","r18","r19","r20",
	"r21","r22","r23","r24","r25","r26","r27","r28","r29","r30","r31",
	"fpr0","fpr1","fpr2","fpr3","fpr4","fpr5","fpr6","fpr7",
	"fpr8","fpr9","fpr10","fpr11","fpr12","fpr13","fpr14","fpr15",
	"fpr16","fpr17","fpr18","fpr19","fpr20","fpr21","fpr22","fpr23",
	"fpr24","fpr25","fpr26","fpr27","fpr28","fpr29","fpr30","fpr31",

	/* we KNOW here that NUM_KERNEL_BATS is 3 -- if it changes,
	   this list must be changed.  The alternative is to generate
	   the names dynamically, at run time */
#if (NUM_KERNEL_BATS != 3)
#error "Need to adjust the BAT names in dbgetcmd.c"
#else /* #if (NUM_KERNEL_BATS != 3) */
	"bat0u", "bat1u", "bat2u", "bat0l", "bat1l", "bat2l",
#endif /* #if (NUM_KERNEL_BATS != 3) */

	"iar","mq","msr","cr","lr","ctr","tid","xer",
	"fpscr","srr0","srr1","dsisr","dar","eim0","eim1",
	"eis0","eis1","peis0","peis1","ilcr",
	"xirr","cppr","dsier",
	"sdr0","sdr1","rtcu","rtcl","tbu","tbl","dec"};

/* array for user defined variable strings */
char *user_string[NUMUVARS] =
{
	VARVALUE, VARVALUE, VARVALUE, VARVALUE,
	VARVALUE, VARVALUE, VARVALUE, VARVALUE,
	VARVALUE, VARVALUE, VARVALUE, VARVALUE,
	VARVALUE, VARVALUE, VARVALUE, VARVALUE,
};

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int parse_linec();       /* parse the input line */
extern int strlen();
extern int strncmp();
extern int get_addr();

/*
 * LOCAL PROCEDURES CALLED:
 */

int get_cmd();
void expression();
ulong diagnose();
void setup_segid();
uchar findvar();
ulong get_ptr();

#define EXP_DELIMS	"()+->*/: "
#define FLAGS_INIT	0xc0

extern struct func func[];

/*
* NAME: get_cmd
*
* FUNCTION: this routine will search through the list of commands in the
*       command array of structures to find a command that matches the
*	command entered by the user. When a blank command is found then you
*	have reached the end of the list and the user entered an invalid
*	command. if the command was a valid one then check if the parameters
*	were expressions. For example in the command display 20+312e; the
*	second parameter or token must be computed and move into the proper
*	field within the token structure. 20+312e is an expression and
*	the token for this parameter will be marked accordingly after
*	being parsed. The expression token is passed to expression and
*	then reparsed.
*	  After the expression has been computed the parameter must be
*	sent to diagnose to make sure that the data entered is appropriate
*	for that parameter. For example if you only want hex numbers in
*	the second parameter diagnose will check to make sure the data
*	in this token is in a hex format or an error will be signaled. The
*	definitions for a specific parameter are found in dbfunc.h.
*
* RETURN VALUE DESCRIPTION: always 0
*
*/

int
get_cmd(line,ps)
char *line;				/* ptr to char string */
struct parse_out *ps;			/* ptr to c parser */

{
	int i,j;
	ulong cmd_id=INVALID_ID;

	parse_line(line,ps," ");		/* parse line into tokens */
	if (ps->num_tok == -1)			/* ditto */
  	  	cmd_id = DITTO_ID;
	else {
		if (ps->token[0].sv[0] == '?')	/* special case for ? */
			strcpy(ps->token[0].sv,"help ");
		/* find the correct command  string  */
  		for (cmd_id=0; strcmp(func[cmd_id].label," "); cmd_id++) {
    	    		if(!strncmp(ps->token[0].sv,func[cmd_id].label,
		  	    strlen(ps->token[0].sv))) {
			    	if (cmd_id != DITTO_ID)
      					break;		/* cmd found, leave */
    	    		}
  	  	}
		if (!strcmp(func[cmd_id].label," "))
			cmd_id = INVALID_ID;

  		if ((cmd_id != INVALID_ID) && (ps->num_tok > 0))  {
    	  		for (j=1; j<=ps->num_tok; j++)  {
      	    			if ((ps->token[j].tflags!=ERR) &&
					(ps->token[j].tflags==EXP_TYPE))  {
#ifdef debug
					printf("calling expression");
#endif /* #ifdef debug */
					expression(&ps->token[j]);
				}
      	      			else
					ps->token[j].varidx = NOT_A_VAR; /* not var */
    	  		}
	  	}

	  	if (cmd_id == INVALID_ID)
     	 	  	vdbperr(ivc_text1,ps->token[0].sv,ivc_text2);
  		else {
#ifdef debug
		  	printf("calling diagnose  ");
#endif /* #ifdef debug */
    		  	cmd_id = diagnose(cmd_id,ps);
		}
	}
	return(cmd_id);
}

/*
* NAME: expression
*
* FUNCTION: this routine take a token that has the EXPRESSION flag
*	turned on and compute teh expression. The first thing to look
*	for in an expression is if the data is a variable name. Findvar
*	is called to determine this. If the expression is like this: +300
*	then it is an expression that is origin relative. The 300 is added
*	to the origin value. The expression operator is moved to a temporary
*	variable and then based on the operator the token value is computed
*	and moved into the parser structure.
*
* RETURN VALUE DESCRIPTION: always 0
*
*/

void
expression(tok)
struct token_def *tok;
{

	static struct parse_out exp;		/* temp parser structure */
	uchar exp_op;				/* expression operator */
	int i,hxv=0,dcv=0;

	/* token initialization */
	tok->dv = 0;
	tok->hv = 0;
	tok->tflags = FLAGS_INIT;		/* prepare flags */
	exp_op = '+';				/* default first operation */

	parse_line(tok->sv,&exp,EXP_DELIMS);	/* parse an expression */
#ifdef debug
	printf("inside expression tok->sv %s\n", tok->sv);
	printf("in exp after parse call: exp.num_tok %x", exp.num_tok);
#endif /* #ifdef debug */
	/* if it's a lone variable */
	if ((exp.num_tok == 0) && (exp.token[0].tflags == EXP_TYPE) &&
	   (exp.delim_char[0] == ' ')) {
	  	if (!findvar(tok)) 		/* resolve lone var ref */
	    		tok->tflags = ERR;	/* set err if var not fnd */
	}
	else  {					/* looks like a real expression */
	  	tok->varidx = NOT_A_VAR;	/* not a lone variable */
	  	if (exp.token[0].sv[0] == '+')
	  	  	tok->hv = debvars[IDORG].hv;	/* org relative exp */
#ifdef debug
	  	printf("exp_delimchar: %x ", exp.delim_char[0]);
	  	printf("before for loop exp.num_tok %x ", exp.num_tok);
	  	printf("exp operator %x ", exp_op);
#endif /* #ifdef debug */
	  	for (i=0; i<=exp.num_tok; i++)  {
	    	   if ((exp.token[i].tflags==ERR)||
			(exp.token[i].tflags==QUOTED)) {
	      		tok->tflags = ERR;
	      		break;
	    	    }
	    	    else  {
			/* get var data if there */
	      		if (exp.token[i].tflags==EXP_TYPE) {
				if (!findvar(&exp.token[i])) {
					tok->tflags = ERR;
					break;
				}
			}
	      		if (tok->tflags & HEX_VALID)
				hxv = 1;
	      		if (tok->tflags & DEC_VALID)
				dcv = 1;
	      		switch (exp_op)  {
			    case '+':
			    case '(':
			    case ')':
		  		tok->hv += exp.token[i].hv * hxv;
		  		tok->dv += exp.token[i].dv * dcv;
	 	  		break;
		   	   case '-':
		  		tok->hv -= exp.token[i].hv * hxv;
		  		tok->dv -= exp.token[i].dv * dcv;
	 	  		break;
			   case '*':
		  		tok->hv *= exp.token[i].hv * hxv;
		  		tok->dv *= exp.token[i].dv * dcv;
	 	  		break;
			   case '/':
		  		tok->hv /= exp.token[i].hv * hxv;
		  		tok->dv /= exp.token[i].dv * dcv;
	 	  		break;
			   case '>':		/* pointer reference */
		  		if (exp.token[i].tflags & DEC_VALID)
					/* get data from mem */
		    			get_ptr(tok,exp.token[i].dv);
		  		else
		    			tok->tflags == ERR; /* err bad length */
		  		break;
	      		}
	      		exp_op = exp.delim_char[i];	/* get next delimiter */
	          }
		}
	}

        if (exp_op == '>')
  	  	get_ptr(tok, 4);		/* last thing was a ptr ref */
        else if (exp_op == ':')
  	  	setup_segid(tok);		/* segment id value */
	return;
}

/*
* NAME: get_ptr
*
* FUNCTION: this routine handles a command line pointer reference. Example
*	display 200>, the > means display the data at the address pointed
*	to by the contents of address 200. Just another level of indirection.
*	This routine will handle the pointer reference and retrieve the data
*	from memory and move it into the parser structure.
*
* RETURN VALUE DESCRIPTION: always 0
*
*/

ulong
get_ptr(tok,len)
struct token_def *tok;				/* ptr to token data */
ulong len;					/*  # of bytes  */
{
	int j,i;
	uchar byte;

	if (tok->tflags & HEX_VALID)  {		/* must be a valid hex num */
	  	if ((len<1) || (len>4))
	    		len = 4;		/* default is 4 */
	  	j = tok->hv;			/* value of ptr */
	  	tok->hv = 0;
	  	for (i=0; i<len; i++)  {  	/* get data from memory */
	   		if (!get_from_memory(j,T_BIT,&byte,sizeof(byte))) {
	      			tok->tflags = ERR;
	      			break;		/* leave loop */
	    		}
			/* include new data */
	    		tok->hv = (tok->hv*0x100)+byte;
	    		j++;			/* next byte */
	  	}
	}
	else
	  	tok->tflags = ERR;		/* not a valid hex express */
	return;
}

/*
* NAME: setup_segid
*
* FUNCTION: this routine setup the expression as a segment id. Multiply
*	by 4 to remove the low order 2 bits and add 10000 for proper
*	index into the segment.
*
* RETURN VALUE DESCRIPTION: always 0
*
*/

void
setup_segid(temptoken)
struct token_def *temptoken;
{
	if (temptoken->tflags & HEX_VALID)  {
	  	temptoken->hv = (temptoken->hv * 4) + 0x10000;
	}
	else
	  	temptoken->tflags = ERR;
	return;
}

/*
* NAME: diagnose
*
* FUNCTION: this routine checks the parameter descriptions for each command
*	that are set in in the command description structure in dbfunc.h.
*	For more information on the structure check the file dbfunc.h.
*	Each parameter is checked to make sure it is of the proper type
*	described in dbfunc.h. Check each parameter and turn on the found flag
*	once the parameter description is satisfied. This routine just
*	checks to make sure the parameter entered by teh user is of the
*	correct type.
*
* RETURN VALUE DESCRIPTION: always 0
*
*/

ulong
diagnose(cmd,ps)
ulong cmd;				/* index into cmd table */
struct parse_out *ps;			/* ptr to parser struct */
{

	char c, cc[3];
	int msgout=0;
	int j,found;

	for (j=1; j<=3; j++)  {		/* for each parm we can diagnose */

		/* if no restrictions on the parameter then break */
	  	if ((j>ps->num_tok) || (func[cmd].field[j]==0))
			continue;
		found = FALSE;

		/* is it a valid quoted string ?		*/

	  	if ((func[cmd].field[j] & STV) &&
		    (ps->token[j].tflags & QUOTED))
			found = TRUE;

		/* is it a valid hex number ?			*/

	  	if ((func[cmd].field[j] & HXV) &&
		    (ps->token[j].tflags & HEX_VALID) &&
		    !(ps->token[j].tflags & HEX_OVERFLOW))
			found = TRUE;

		/* is it a valid decimal number	?		*/

		if ((func[cmd].field[j] & DCV) &&
		    (ps->token[j].tflags & DEC_VALID) &&
		    !(ps->token[j].tflags & DEC_OVERFLOW))
			found = TRUE;

	  	/* check and process address */
	  	/* if address check and nothing found */
	  	if ((func[cmd].field[j] & ADV)) {
	    		found = get_addr(j,ps); /* valid addr? */
#ifdef debug
			printf("j= %x ps->token[j].sv=%s\n",j,ps->token[j].sv);
	  		printf("adv found = %x ", found);
#endif /* #ifdef debug */
	    		if ((found) && (!ps->token[j].debaddr->inmem)) {
				    /* addr req'd to be in mem?*/
	      			    if (func[cmd].field[j] & RMV) {
					vdbperr(not_in_real);
					msgout = 1;
					found = 0;
	      			    }
	      			    else
					 found = 1; /* not required in memory */
			}
	    	}
  		if (found)
    			continue;
  		else  {
			/* get first character */
    			c = ps->token[j].sv[0];
    			/* asterisk check  and B/T check */
/*
    			found = (((func[cmd].field[j]&ASV) && (c=='*')) ||
    				((func[cmd].field[j] & BTV) &&
				((c=='b') | (c=='t'))));
 */
                        found = (((func[cmd].field[j] & ASV) && (c=='*')) ||
                                 ((func[cmd].field[j] & BTV) &&
                                   ((c=='b') | (c=='t'))) ||
                                 ((func[cmd].field[j] & BRT) && (c=='b')) ||
                                 ((func[cmd].field[j] & WTC) && (c=='w')) ||
                                 ((func[cmd].field[j] & LSV) &&
                                        (c=='l' | c=='s')));

			/* yes/no diagnostic */
	    		if ((func[cmd].field[j]&YNV) && !found)  {
	      			if (strlen(ps->token[j].sv) > 1) {
					strncpy(cc,ps->token[j].sv,2);
					cc[2] = '\0';
				}
	      			else
					cc[0] = '\0';
	      			/* yes/no on/off check */
	      			found = (((c == 'y') || (c == 'n')) ||
	      			    ((!strcmp(cc,"of"))||(!strcmp(cc,"on"))));
	      			if ((c == 'y') || (!strcmp(cc,"on")))
	    			    	ps->token[j].tflags = YES_ON;
	    		}
  		}
  		if (!found) {	/* means no condition satis*/
    			if (!msgout) {	/* no prev msg displayed */
      			   	msgout = 1;
      			   	vdbperr(ivd_text1,ps->token[j].sv,ivd_text2);
    			}
    			return(cmd = INVALID_ID);
  		}
	}
	/* check that correct number of tokens entered */
	if ((ps->num_tok < (func[cmd].field[0])) && (cmd != DITTO_ID)) {
  		vdbperr(ins_text);
 		cmd = INVALID_ID;
	}
	return(cmd);
}

/*
* NAME: findvar
*
* FUNCTION: this routine finds a variable in the var_name array. If the
*	the variable is found or space is available then TRUE is returned
*	and the information associated with the variable is moved into
*	the parser structure.
*
* RETURN VALUE DESCRIPTION: True if variable found or space available
*			    False if not found
*/

uchar
findvar(data)
struct token_def *data;
{

	uchar string;
	int i;

	for (i=0; i<MAXVARS; i++)  {
	  	if (!strcmp(var_name[i],data->sv)) {
	  		data->tflags = debvars[IDUVARS+i].type;
	  		data->hv = debvars[IDUVARS+i].hv;
	  		data->dv = debvars[IDUVARS+i].dv;
	    		data->varidx = i;
	    		if (i<LAST_UVAR)
	      			strcpy(data->sv, user_string[i]);
			else
	  			data->tflags = HEX_VALID;   /* system info */
	    		return(TRUE);
	  	}
	}
	data->varidx = NOT_A_VAR;
	return(FALSE);				/* not found */
}
