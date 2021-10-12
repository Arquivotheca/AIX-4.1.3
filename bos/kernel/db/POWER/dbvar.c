static char sccsid[] = "@(#)61	1.19  src/bos/kernel/db/POWER/dbvar.c, sysdb, bos412, 9445B412 11/2/94 11:05:18";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: listvars, set_var, add_var, reset_var, atofpr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include "parse.h"                      /* parser structure             */
#include "debvars.h"
#include "vdberr.h"
#include "dbdebug.h"
#include "dbfunc.h"

void atofpr ();
/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern strcmp();
extern int parse_line();
extern parse_cvrt();
extern vdbperr();

extern user_string[];
extern var_name[];

/*
 *	These functions list and modify the var_name array, which is
 *  	the list of variables that the user can redefine in the 
 *	debugger.
 */

int list_vars();
void set_var();
uchar add_var();
int reset_var();

/* array for non updateable system registers */
char *no_update[] =
{
#ifdef _POWER_MP
        "ilcr", "dsier", "xirr", "srr0", "srr1", "rtcu", "rtcl", "tbu", "tbl"
#else
        "ilcr", "dsier", "xirr", "srr0", "srr1"
#endif /* _POWER_MP */
};

/* macro to set a bit in the io allocation mask copy (used in add_var()) */
#define SET_BIT(value, bit)	(value) = (value) | ((uint)0x80000000 >> (bit))

/* macro to clear a bit in the io allocation mask copy (used in add_var()) */
#define CLEAR_BIT(value, bit)	(value) = (value) & ~((uint)0x80000000 >> (bit))

/* allocation mask declaration (defined in dbdebug.c) */
extern ulong_t ioalloc;

/*==================================================================
* 
* list_vars: this function will traverse the user variable area
*	of the var_name array looking for any user defined variables.
*	If any are found, display them. Also display FX and ORG
*	every time
*
===================================================================*/

int
list_vars()
{
	int i, free;

	printf("Listing of the User-defined variables: ");/* header msg */
	free = LAST_UVAR - FIRST_UVAR;		/* # free slots */
	for (i=FIRST_UVAR; i<LAST_DVAR; i++) {
	  	if (strcmp(var_name[i], bl8) != 0) {/* check for var names */
	    		printf("\n  %s ", var_name[i]);	/* print var name */
	    		if (i < LAST_UVAR)	/* reduce free var count */
	      			free--;
				/* valid as hex ? */
	    		if (debvars[IDUVARS+i].type & HEX_VALID) {
	      			printf("HEX");
	      			if (debvars[IDUVARS+i].type & DEC_VALID)
					printf("/DEC");/* decimal as well ? */
				/* print the number */
	      			printf("=%08x",debvars[IDUVARS+i].hv); 
  			} 
			else 
				if (debvars[IDUVARS+i].type & DEC_VALID) 
					printf("DEC=%08d",
					       debvars[IDUVARS+i].dv);
	    		if ((debvars[IDUVARS+i].type==QUOTED) && (i<LAST_UVAR)) 
	      			printf("String value = %s", user_string[i]);
	  	}
	}
	printf("\nThere are %d free variable slots.\n", free);
	return(0);
}

/*==================================================================
*
* set_var(): this is the driver routine for setting new variables
*	or refining old ones. The parser structure is passed in
*	and re-parsed. This is to set the variable up as EXP_TYPE.
*	Call add_var() to actually add the new variable to the
*	var_name array.
*
===================================================================*/

void
set_var(ps, line) 
struct parse_out *ps;				/* ptr to c parser struct */
uchar *line;					/* input string */
{

	struct token_def  *tmptoken;		/* tmp holder of 3rd parm */
	struct parse_out  tmpparse;

	tmptoken = &ps->token[2];
	if (ps->num_tok < 2)			/* need 2 tokens */
		  vdbperr(ins_text);
	else  {
	  	parse_line(line,&tmpparse," ");	/* re-parse the line */
		/* the new var will be EXP_TYPE via debparse */
	  	if ((tmpparse.token[1].tflags == EXP_TYPE)  ||
			(!strcmp(tmpparse.token[1].sv,"dec"))) {
	    		/* add the var to the tab */
	    		add_var(tmpparse.token[1].sv,tmptoken);
	  	}
	  	else		/* wrong type */
	    		vdbperr(ivd_text1, tmpparse.token[1].sv, ivd_text2);
	}
	return;
}

/*=============================================================
*
* add_var(): this function will find a variable name or a blank
*	position in the var_name array and move the data into
*	the array.
*
===============================================================*/

uchar
add_var(name,data)
uchar	*name;
struct token_def *data;
{
	char strbuf[20];		/* string form of # variables */
	register int i;			/* index into var_name array */

	if (strlen (name) > strlen (bl8))
	{
		vdbperr (varsize_limit);
		return (FALSE);
	}


	for (i=(sizeof(no_update)/sizeof(char *)-1); i >= 0; i--)  {	
	  	/* check if var is updateable */
	  	if ((!strcmp(no_update[i],name))) {
			vdbperr(ivd_text1,name,ivd_text2);
			return(FALSE);
		}
	}

	for (i=MAXVARS-1; i >= 0; i--)  {	
	  	/* find a var to redefine or a blank spot for a new var */
	  	if ((!strcmp(var_name[i],name)) || (!strcmp(var_name[i],bl8)))
	    		break;
	}

	/* i will be set to the index of the element to setup, or i = -1 if
	   no spaces are free */

	if (i < 0)		/* no more free variables */
	{
		sprintf (strbuf, "%d", NUMUVARS);
	      	vdbperr(nomore_vars1, strbuf, nomore_vars2);
	}
	else
	{
	  	debvars[IDUVARS+i].rsv1 = i;			/* save varidx */
	  	debvars[IDUVARS+i].type = data->tflags;		/* put in the data */
		if ((i>=IDFPRS) && (i<IDFPRS+NUMFPRS)) { 	/* value is a double */
			atofpr(data->sv,i-(IDFPRS));
		}
		else {
	  		debvars[IDUVARS+i].hv = data->hv;	/* store data */
	  		debvars[IDUVARS+i].dv = data->dv;	/* store data */
			/* If an upper BAT value is being set to zero clear the
			   appropriate bit in the io allocation mask.
			   Otherwise set the appropriate bit to show that the 
			   BAT is in use.  Note that marking a BAT as in use
			   removes it from the group available to the kernel
			   iomem_att service and can potentially cause a panic
			*/
			if ((i>=(IDBATU)) && (i<(IDBATU+NUM_KERNEL_BATS))) {
				if (data->hv == 0)
					CLEAR_BIT(ioalloc, i-(IDBATU));
				else
					SET_BIT(ioalloc, i-(IDBATU)); 
			}
	  		if (i < LAST_UVAR) {
	    			strcpy(user_string[i], data->sv);/* save str */
	  			strcpy(var_name[i],name); /* might be new so */
							/* add var to array */
			}
		}
	  	return(TRUE);
	}
}
	  
/*=============================================================
*
* reset_var(): this function will remove a variable name from the 
*	var_name array.
*
===============================================================*/

int
reset_var(ps)
struct parse_out *ps;		/* ptr to parser structure */
{

	if (ps->num_tok < 1)	/* not enough tokens */
	  	vdbperr(ins_text);
	else {
	  	if ((ps->token[1].varidx == NOT_A_VAR) || /* bad variable name */
			(ps->token[1].varidx >= LAST_UVAR))
	    		vdbperr(ivd_text1,ps->token[1].sv,ivd_text2);
	  	else
			/* blank out the name in the array */
	    		strcpy(var_name[ps->token[1].varidx], bl8);
	}
	return(0);
}

/*
 *  Name: atofpr()
 *  	convert ascii string into hex number one byte at a time
 *  	and place into fr[], which holds the doubles that are debugger's
 *  	copy of the floating point registers.
 *
 *  Input:	str	- string to convert
 *  		i	- index of floating point register to set
 */
void
atofpr(str,i)
char *str;
uint i;
{
	char  *in, *out;
	uchar c;
	int nib1 = 0;
	
	in = str;
	out = (char *)&fr[i];		/* line up output pointer */
	out += sizeof(double) - 1;	/* now pointing at least sig char */

	while(*in != '\0')	/* go to end of string */
		in++;
	if(in == str)
		return ;	/* null string */
	in--;			/* reset to last chat in string */

	/* fill in from least significant nibble, to most significant */
	while((in >= str)&&(out >=(char *)&fr[i])){/*until str gone or fr[] filled*/
		if(!nib1)		/* decide which nibble we are on */
			nib1 = 1;
		else 
			nib1 = 0;
		if(*in>='0' && *in<='9') {	/* xlate ascii to hex number */
			c = *in - '0';
		}
		else if(*in>='a' && *in<='f') {
			c = *in - 0x57;
		}
		else 
			return ; /* error */ 

		/* load the value into the "register", fr[] */
		if(nib1) {
			*out &= 0xF0;	/* set lower nibble to 0 */
			*out |= c;	/* or in hex digit */
		}
		else {
			*out &= 0x0F;		/* set upper nibble to 0 */
			*out |= (c << 4);	/* or in hex digit */
			out--;			/* move fr[] pointer */
		}
		in--; 	/* do next nibble */
	}
}
