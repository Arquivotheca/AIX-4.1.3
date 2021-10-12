static char sccsid[] = "@(#)03	1.14.1.7  src/bos/usr/bin/localedef/sem_chr.c, cmdnls, bos411, 9428A410j 4/27/94 10:53:36";
/*
* COMPONENT_NAME: (CMDNLS) Locale Database Commands
*
* FUNCTIONS:
*
* ORIGINS: 27, 85
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1991, 1994
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
* ALL RIGHTS RESERVED
*
* OSF/1 1.2
*
*/

#define _ILS_MACROS

#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <sys/limits.h>
#include <stdio.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "locdef.h"
#include "err.h"
#include "method.h"

/* static used by wchar_defined() and define_wchar() */
#define MAX_PC USHRT_MAX
static unsigned char defined_pcs[MAX_PC+1];


/*
*  FUNCTION: define_wchar
*
*  DESCRIPTION: 
*  Adds a wchar_t to the list of characters which are defined in the 
*  codeset which is being defined.
*/
void define_wchar(wchar_t wc)
{
    extern int warn;

    if (wc > MAX_PC)
	INTERNAL_ERROR;

    if (defined_pcs[wc]) {
	if (warn) 
	    diag_error(ERR_PC_COLLISION, wc);
    } else 
	    defined_pcs[wc] = TRUE;
}


/*
*  FUNCTION: wchar_defined
*
*  DESCRIPTION: 
*  Checks if a wide character has been defined.
*
*  RETURNS
*  TRUE if wide char defined, FALSE otherwise.
*/
int wchar_defined(wchar_t wc)
{
    return defined_pcs[wc];
}

/*
*  FUNCTION: is_special
*
*  DESCRIPTION: 
*  checks to see if character passed is a "special" character that is
*  supposed to be escaped.
*  < > \ " ; ,
*
*  RETURNS
*  TRUE if valid special character, false otherwise.
*/
int is_special(char c)
{
	if ((c == '>') || (c == '<') || (c == '\\') || (c == '\"') || 
	    (c == ';') || (c == ','))
	    return TRUE;
	else
	    return FALSE;
}

/* 
*  FUNCTION: copy_string
*
*  DESCRIPTION: 
*  Copies a string and replaces occurances of character references with
*  the encoding of that character.  For example:
*  "<A>" would get copied to "\x65".
*
*  If the value of the character is greater than 0x80 than the byte
*  value of the character will be copied into the string in the form
*  \x84"" instead of just putting the byte value into the string.
*
*  The testing for characters greater than 0x80 is to determine whether or
*  not characters are in the portable character set.  This test is very
*  dependent on ASCII, and will not work for things like EBCDIC.
*
*  This routine malloc()s the storage to contain the copied string.
*/
void copy_string(char **result, char *s2)
{
    extern char yytext[];	/* use yytext for str buffer! */
    extern symtab_t cm_symtab;
    extern char escape_char;

    symbol_t *s;
    char     id[ST_MAX_SYM_LEN+1];
    char     *s1;
    int      i, j;
    char junk[80]; 		/* junk array for holding temp info */
    char number[80];		/* array for holding the oct, hex, or
				   dec value */
    char *temp;			/* pointer to the number array */
    int value = 0;

    s1 = yytext;

    *s1 = '\0';			/* copy_string may be passed in a null string */

    while (*s2 != '\0') {

	while (*s2 != escape_char && *s2 != '<' && *s2 != '\0') 
	    if (((unsigned char) *s2 < 0x80) && *s2 != '\\' && *s2 != '"') {
	        *s1++ = *s2++;
	    }
	    else {
		sprintf(s1, "\\x%02x\"\"",*s2++);
		s1 += 6;
	    }

	if (*s2 == escape_char) {
	       /*  If the character pointed to is the escape_char 
		   see if it is the beginning of a character constant
			\d for decimal
			\x for hexidecimal
			\<digit> for octal
		   otherwise it is an escaped character that needs copied	
		   into the string */
	    *s2++;
	    if (*s2 == 'd') {  /* decimal constant */
		for (*s2++,temp=number; isdigit(*s2); *s2++)
		     *temp++ = *s2;
		*temp++ = '\0';
		if ((temp - number) > 4) {
		    /* too many digits in decimal constant */
		    junk[0] = escape_char;
		    junk[1] = '\0';
		    strcat(junk,number);
		    diag_error(ERR_ILL_DEC_CONST,junk);
		}
		value = strtol(number, &junk, 10);
		if ((value < 0x80) && value != '\\' && value != '"') {
		    *s1++ = value;
		    *s1 = '\0';
		}
	        else {
		    sprintf(s1, "\\x%02x\"\"", value);
		    s1 +=6;
		}
	    }
	    else if (*s2 == 'x' || *s2 == 'X'){ /* hex constant */
	        for (*s2++,temp=number; isxdigit(*s2); *s2++)
	   	    *temp++ = *s2;
		*temp++ = '\0';
		if ((temp - number) > 3){
		  /*  wrong number of digits in hex const */
		    junk[0]=escape_char;
		    junk[1]='\0';
		    strcat(junk,number);
		    diag_error(ERR_ILL_HEX_CONST,junk);
		}
		value = strtol(number, &junk, 16);
		if ((value < 0x80) && value != '\\'  && value != '"') {
		    *s1++ = value;
		    *s1 = '\0';
		}
	        else {
		    sprintf(s1, "\\x%02x\"\"", value);
		    s1 +=6;
		    *s1 = '\0';
		}
	    }
	    else if (isdigit(*s2)) {		/* octal constant */	
	        for (*s2,temp=number; 
	             *s2 >= '0' && *s2 <= '7' && *s2 != EOF; *s2++)
		     *temp++ = *s2;
		if (*s2 == '8' || *s2 == '9') { /* illegal oct */
		    while (isdigit(*s2)) 
		    *temp++ = *s2++;
		    *temp = '\0';
		    junk[0]=escape_char;
		    junk[1]='\0';
		    strcat(junk,number);
		    diag_error(ERR_ILL_OCT_CONST,junk);
		    *s1++=escape_char;
		    for (i = 0; number[i] != '\0'; i++)
		        *s1++ = number[i];
		    *s1++='"';
		    *s1++='"';
		    *s1 = '\0';
		}
	        else {
	            *temp++ = '\0';
		    if ((temp - number) > 4) {
		      /* too many digits in octal constant */
		        junk[0]=escape_char;
		        junk[1]='\0';
		        strcat(junk,number);
		        diag_error(ERR_ILL_OCT_CONST,junk);
		    }
		    value = strtol(number, &junk, 8);
		    if ((value < 0x80) && value != '\\' && value != '"') {
		        *s1++ = value;
		        *s1 = '\0';
		    }
	            else {
		        sprintf(s1, "\\x%02x\"\"", value);
		        s1 +=6;
			*s1 = '\0';
		    }
		}
	    }
            else {             /* escaped character, e.g. \\ */
	        if (is_special(*s2)  && (*s2 != '<')) {
	            *s1++ = '\\';  /* use real escape char for compiler */
		    *s1++ = *s2++;
                    *s1 = '\0';        /* just in case s2 is now null */
		}
	    	else if (*s2 != '\0') {
		    if (((unsigned char) *s2 < 0x80) 
                                            && (*s2 != '\\') && (*s2 != '"')) {
		        *s1++ = *s2++;
			*s1 = '\0';
		    }
		    else {
		        sprintf(s1,"\\x%02x\"\"", *s2++);
		        s1 +=6;
			*s1 = '\0';
		    }
		}
	    	else
		  /* \ cannot be last char in string. */
		    error(ERR_BAD_STR_FMT, s2);
	   }
	}
	else if (*s2 == '<') {
	    i = 0;
	    while (*s2 != '>') {
	        for (; *s2 != '>' && *s2 != escape_char; i++, s2++)
		     id[i] = *s2;

		if (*s2 == escape_char) {
		    id[i++] = *s2++;	/* use user defined escape here */
                    id[i++] = *s2++;
	        }

		if ( *s2 == '>') {
		    id[i++] = '>';
		    id[i] = '\0';
		    s = loc_symbol(&cm_symtab, id, 0);
		    if (s==NULL) 
		        error(ERR_SYM_UNDEF, id);
		    else if (s->sym_type != ST_CHR_SYM)
		        error(ERR_WRONG_SYM_TYPE, id);
		    else {
		        if (s->data.chr->len==1 && 
			    s->data.chr->wc_enc < 0x080 && 
			    s->data.chr->str_enc[0] != '\\' &&
			    s->data.chr->str_enc[0] != '"') {
			    *s1++ = s->data.chr->str_enc[0];
			    *s1 = '\0';
		        } 
			else {  
		            for (i=0; i<s->data.chr->len; i++) {
		                sprintf(s1, "\\x%02x\"\"", 
			                s->data.chr->str_enc[i]);{
			 	        s1 += 6;
					*s1 = '\0';
				}
			    }
			}   
		    }
		}
	        else
	            error(ERR_BAD_STR_FMT, s2);
	    }
	    s2++;
	}
	else {
	    *s1 = '\0';
	}
    }

    *result = MALLOC(char, strlen(yytext)+1);
    strcpy(*result, yytext);
}

/* 
*  FUNCTION: copy
*
*  DESCRIPTION: 
*  Copies a string and replaces occurances of character references with
*  the encoding of that character.  For example:
*  "<A>" would get copied to "\x65".
*  
*  This routine does a straight copy of symbol and character constants
*  to their byte values. Nothing is replaced.
*
*/
void copy(char* result, char *s2)
{
    extern symtab_t cm_symtab;
    extern char escape_char;

    symbol_t *s;
    char     id[ST_MAX_SYM_LEN+1];
    char     *s1;
    int      i, j;
    char junk[80]; 		/* junk array for holding temp info */
    char number[80];		/* array for holding the oct, hex, or
				   dec value */
    char *temp;			/* pointer to the number array */
    int value = 0;
    char string1[80];

    strcpy(result,s2);
    s1 = result;

    *s1 = '\0';			/* copy may be passed in a null string */
	
    while (*s2 != '\0') {

	while (*s2 != escape_char && *s2 != '<' && *s2 != '\0') 
	    *s1++ = *s2++;

	if (*s2 == escape_char) {
	       /*  If the character pointed to is the escape_char 
		   see if it is the beginning of a character constant
			\d for decimal
			\x for hexidecimal
			\<digit> for octal
		   otherwise it is an escaped character that needs copied	
		   into the string */
	    *s2++;
	    if (*s2 == 'd') {  /* decimal constant */
		for (*s2++,temp=number; isdigit(*s2); *s2++)
		     *temp++ = *s2;
		*temp++ = '\0';
		if ((temp - number) > 4) {
		    /* too many digits in decimal constant */
		    junk[0] = escape_char;
		    junk[1] = '\0';
		    strcat(junk,number);
		    diag_error(ERR_ILL_DEC_CONST,junk);
		}
		value = strtol(number, &junk, 10);
		*s1++ = value;
		*s1 = '\0';
	    }
	    else if (*s2 == 'x' || *s2 == 'X'){ /* hex constant */
	        for (*s2++,temp=number; isxdigit(*s2); *s2++)
	   	    *temp++ = *s2;
		*temp++ = '\0';
		if ((temp - number) > 3){
		  /*  wrong number of digits in hex const */
		    junk[0]=escape_char;
		    junk[1]='\0';
		    strcat(junk,number);
		    diag_error(ERR_ILL_HEX_CONST,junk);
		}
		value = strtol(number, &junk, 16);
		*s1++ = value;
		*s1 = '\0';
	    }
	    else if (isdigit(*s2)) {		/* octal constant */	
	        for (*s2,temp=number; 
	             *s2 >= '0' && *s2 <= '7' && *s2 != EOF; *s2++)
		     *temp++ = *s2;
		if (*s2 == '8' || *s2 == '9') { /* illegal oct */
		    while (isdigit(*s2)) 
		    *temp++ = *s2++;
		    *temp = '\0';
		    junk[0]=escape_char;
		    junk[1]='\0';
		    strcat(junk,number);
		    diag_error(ERR_ILL_OCT_CONST,junk);
		    *s1++=escape_char;
		    for (i = 0; number[i] != '\0'; i++)
		        *s1++ = number[i];
		    *s1++='"';
		    *s1++='"';
		    *s1 = '\0';
		}
	        else {
	            *temp++ = '\0';
		    if ((temp - number) > 4) {
		      /* too many digits in octal constant */
		        junk[0]=escape_char;
		        junk[1]='\0';
		        strcat(junk,number);
		        diag_error(ERR_ILL_OCT_CONST,junk);
		    }
		    value = strtol(number, &junk, 8);
		    *s1++ = value;
		    *s1 = '\0';
		}
	    }
            else {             /* escaped character, e.g. \\ */
	        if (is_special(*s2)) {
	            *s1++ = '\\';  /* use real escape char for compiler */
		    *s1++ = *s2++;
                    *s1 = '\0';        /* just in case s2 is now null */
		}
	    	else if (*s2 != '\0') {
		    *s1++ = *s2++;
		}
	    	else
		  /* \ cannot be last char in string. */
		    error(ERR_BAD_STR_FMT, s2);
	   }
	}
	else if (*s2 == '<') {
	    i = 0;
	    while (*s2 != '>') {
	        for (; *s2 != '>' && *s2 != escape_char; i++, s2++)
		     id[i] = *s2;

		if (*s2 == escape_char) {
		    id[i++] = *s2++;    /* use user defined escape here */
		    id[i++] = *s2++;
	        }

		if ( *s2 == '>') {
		    id[i++] = '>';
		    id[i] = '\0';
		    s = loc_symbol(&cm_symtab, id, 0);
		    if (s==NULL) 
		        error(ERR_SYM_UNDEF, id);
		    else if (s->sym_type != ST_CHR_SYM)
		        error(ERR_WRONG_SYM_TYPE, id);
		    else {
		        for (i=0; i<s->data.chr->len; i++) 
			    *s1++ = s->data.chr->str_enc[i];
			*s1 = '\0';
		    }
		}
	        else
	            error(ERR_BAD_STR_FMT, s2);
	    }
	    s2++;
	}
	else {
	    *s1 = '\0';
	}
    }

   return;
}

/* 
*  FUNCTION: sem_set_chr
*
*  DESCRIPTION:
*  This routine assigns the value of the mutli-byte strings contained 
*  on the top of the stack to the string pointer 's' passed as an
*  argument.  This routine is used to assign character encodings to 
*  character values such as 'mon_decimal_point'. The productions using
*  this sem-action implement statements like:
*
*     mon_decimal_point     <period>
*
*  This routine performs a malloc() to acquire the memory to contain
*  the bytes forming the character.
*
*/
void sem_set_chr(char **s)
{
    item_t *it;
    int len;
    
    it = sem_pop();
    if (it->type == SK_CHR) {
        if (*it->value.chr->str_enc == '"' || *it->value.chr->str_enc == '\\') {
             /* need to escape this character for the compiler */
             *s = MALLOC(char, it->value.chr->len + 2);
             *s[0] = '\\';
             strncpy(*s+1, it->value.chr->str_enc, it->value.chr->len);
             (*s)[it->value.chr->len+1] = '\0';
        }
        else {
             *s = MALLOC(char, it->value.chr->len + 1);
             strncpy(*s, it->value.chr->str_enc, it->value.chr->len);
             (*s)[it->value.chr->len] = '\0';
        }

      /*INTERNAL_ERROR; */
    }
    else {
	if (it->type == SK_INT) {
	    *s = MALLOC(char, MB_LEN_MAX + 1);
	    len = mbs_from_fc(*s, it->value.int_no);
	}
	else
	    INTERNAL_ERROR;
    }

    destroy_item(it);
}

/* 
*  FUNCTION: sem_set_str_lst
*
*  DESCRIPTION:
*  Assign the values of the 'n' strings on the top of the semantic
*  stack to the array of strings 's' passed as a parameter. This routine
*  is used to assign the values to items such as 'abday[7]', i.e. the 
*  seven days of the week.  The productions using this sem-action 
*  implement statements are like:
*
*      abday  "Mon";"Tue";"Wed";"Thu";"Fri";"Sat";"Sun"
*
*  This routine performs a malloc() to acquire the memory to build
*  the array of strings.
*
*/
void sem_set_str_lst(char **s, int n)
{
    item_t *it;
    int i;
    int too_many=0;
    
    for (i=0, it=sem_pop(); it != NULL && i < n; i++, it=sem_pop()) {
	if (it->type != SK_STR)
	    INTERNAL_ERROR;
	
	if (it->value.str[0] != '\0')
	    copy_string(&(s[n-i-1]), it->value.str);
	else
	    s[n-i-1] = NULL;
	
	destroy_item(it);
    }
    
    if (i < n) 
	error(ERR_N_SARGS, n, i);
    else
	while ((it = sem_pop()) != NULL) {
	    destroy_item(it);
	    it = sem_pop();
	    too_many=1;
	}
    if (too_many)
        diag_error(ERR_TOO_MANY_ARGS,n);
}


/* 
*  FUNCTION: sem_set_str
*
*  DESCRIPTION:
*  Assign a value to a char pointer passed as a parameter from the value
*  on the top of the stack.  This routine is used to assign string values
*  to locale items such as 'd_t_fmt'.  The productions which use this 
*  sem-action implement statements like:
*
*     d_t_fmt    "%H:%M:%S"
*
*  This routine performs a malloc() to acquire the memory to contain 
*  the string.
*
*/
void sem_set_str(char **s)
{
    item_t *it;
    
    it = sem_pop();
    if (it == NULL || it->type != SK_STR)
	INTERNAL_ERROR;
    
    if (it->value.str[0]!='\0')
	copy_string(s, it->value.str);
    else
	*s = NULL;
    
    destroy_item(it);
}


/* 
   FUNCTION: sem_set_int

   DESCRIPTION:
   Assign a value to an int pointer passed as a parameter from the value
   on the top of the stack. This routine is used to assign values to
   integer valued locale items such as 'int_frac_digits'.  The productions
   using this sem-action implement statements like:

     int_frac_digits      -1

   The memory to contain the integer is expected to have been alloc()ed
   by the caller.
*/
void sem_set_int(signed char *i)
{
    item_t *it;
    
    it = sem_pop();
    if (it == NULL || it->type != SK_INT)
	INTERNAL_ERROR;
    
    if (it->value.int_no != -1)	    /* -1 means default */
	*i = it->value.int_no;
    
    destroy_item(it);
}


/*
*  FUNCTION: sem_digit_list
*
*  DESCRIPTION:
*  Adds the next digit to the digit list on the semantic stack.  The digit 
*  list consists of n stacked int items and a count.  This routine expects
*  the next digit on top of the stack and the count immediately below it.
*  This routine swaps the top two stack items and increments the count.
*/
void sem_digit_list(void)
{
    item_t *n_digits;
    item_t *next_digit;
	    
    /* swap digit and count and increment count */
    next_digit = sem_pop();
    n_digits = sem_pop();
    if (next_digit->type != SK_INT || n_digits->type != SK_INT)
	INTERNAL_ERROR;
    
    n_digits->value.int_no++;
    sem_push(next_digit);
    sem_push(n_digits);
}


/*
*  FUNCTION: sem_set_diglist
*
*  DESCRIPTION:
*  Creates a string of digits (each less than 256) and sets the argument
*  'group' to point to this string.  
*
*  This routine calls malloc() to obtain the memory to contain the digit 
*  list.
*/
void sem_set_diglist(char **group)
{
    item_t *n_digits;
    item_t *next_digit;
    char   *buf;
    int    i;

    /* pop digit count off stack */
    n_digits = sem_pop();
    if (n_digits->type != SK_INT)
	INTERNAL_ERROR;

    /* allocate string to contain digit list */
    *group = MALLOC(char, (n_digits->value.int_no*5)+6);
    buf   = MALLOC(char, (n_digits->value.int_no*5)+6);

    (*group)[0] = '\0';
    buf[0] = '\0';

    for (i=n_digits->value.int_no-1; i>=0; i--) {
        int value;

	next_digit = sem_pop();
	if (next_digit->type != SK_INT)
	    INTERNAL_ERROR;

        value = next_digit->value.int_no;

	/* ignore values of zero if not the last element: (defect 128802) */
	if ((value == 0) && (i != n_digits->value.int_no-1))
		continue;

	/* 
	 * If -1 or 0 is present as last member, then use CHAR_MAX (255)
         * instead.  This indicates repeating of the last group is not done
         * per POSIX & defect 128802
	 *
         * exception case:  If only a single value is specified, and that
         * value is -1, then we will return the null string per defect 
         * 141803
	 */
	if ((i==n_digits->value.int_no-1) && (n_digits->value.int_no == 1) 
	    && (value==-1))
		value = 0;
	else
	    if (i==n_digits->value.int_no-1 && ((value==-1) || (value==0)))
	        value = CHAR_MAX;		/* ff */

	/*
	  Covert grouping digit to a char constant
	*/
	sprintf(buf, "\\x%02x", value);

	/* 
	  prepend this to grouping string 
	*/
	strcat(buf, *group);
	strcpy(*group, buf);

	destroy_item(next_digit);
    }

    destroy_item(n_digits);
    free(buf);
}


/* 
*  FUNCTION: sem_set_sym_val
*
*  DESCRIPTION:
*  Assigns a value to the symbol matching 'id'.  The type of the symbol
*  is indicated by the 'type' parameter.  The productions using this
*  sem-action implement statements like:
* 
*     <code_set_name>    "IBM-932"
*                 or
*     <mb_cur_max>       2
*
*  The function will perform a malloc() to contain the string 'type' is
*  SK_STR.
*/
void sem_set_sym_val(char *id, int type)
{
    extern symtab_t cm_symtab;
    item_t *i;
    symbol_t *s;
    
    i = sem_pop();
    if (i==NULL) INTERNAL_ERROR;
    
    s = loc_symbol(&cm_symtab, id, 0);
    if (s==NULL) INTERNAL_ERROR;
    
    switch (type) {
      case SK_INT:
	s->data.ival = i->value.int_no;
	break;
      case SK_STR:
	copy_string(&(s->data.str), i->value.str);
	break;
      default:
	INTERNAL_ERROR;
    }
    
    destroy_item(i);
}


/* 
*  FUNCTION: sem_char_ref
*
*  DESCRIPTION:
*  This function pops a symbol off the symbol stack, creates a semantic
*  stack item which references the symbol and pushes the item on the
*  semantic stack.
*/
void sem_char_ref(void)
{
    symbol_t *s;
    item_t   *it;
    
    s = sym_pop();
    if (s==NULL)
	INTERNAL_ERROR;
    
    if (s->sym_type == ST_CHR_SYM)
	it = create_item(SK_CHR, s->data.chr);
    else {
	it = create_item(SK_INT, 0);
	error(ERR_WRONG_SYM_TYPE, s->sym_id);
    }

    if (s->is_bogus == TRUE)
        it->is_bogus = TRUE;

    sem_push(it);
}


/* 
*  FUNCTION: sem_symbol
*
*  DESCRIPTION:
*  Attempts to locate a symbol in the symbol table - if the symbol is not
*  found, then it creates a new symbol and pushes it on the symbol stack. 
*  Otherwise, the symbol located in the symbol table is pushed on the
*  symbol stack.  This routine is used for productions which may define or
*  redefine a symbol.
*/
void sem_symbol(char *s)
{
    extern symtab_t cm_symtab;
    symbol_t *sym;
    char saved_char; 

    /* look for symbol in symbol table */
    sym = loc_symbol(&cm_symtab, s, 0);
    
    /* if not found, and symbol is too long, look for truncated symbol */
    if ((sym==NULL) && (strlen(s) > ST_MAX_SYM_LEN)) { 
            saved_char=s[ST_MAX_SYM_LEN];
            s[ST_MAX_SYM_LEN]='\0';
            sym = loc_symbol(&cm_symtab, s, 0);
	    if (sym==NULL)
		s[ST_MAX_SYM_LEN]=saved_char;  /* not found, restore string */
    }

    /* if still not found, create a symbol */
    if (sym==NULL) {
	sym = create_symbol(s, 0);
	if (sym == NULL) 
	    INTERNAL_ERROR;
	
	sym->sym_type = ST_CHR_SYM;
    }

    
    /* whether new or old symbol, push on symbol stack */
    sym_push(sym);
}


/* 
*  FUNCTION: sem_existing_symbol
*
*  DESCRIPTION:
*  This function locates a symbol in the symbol table, creates a 
*  semantic stack item from the symbol, and pushes the new item on
*  the semantic stack.  If a symbol cannot be located in the symbol
*  table, an error is reported and a dummy symbol may be pushed on the stack.
*  Error handing values are defined as FATAL and SKIP in locdef.h.
*  FATAL causes sem_existing_symbol() to generate a fatal error when it
*  encounters an undefined character.  SKIP causes sem_existing_symbol() to
*  mark the symbol it creates as bogus, and the character or possible range
*  expression is then ignored in context later on in the grammar processing.
*/
void sem_existing_symbol(char *s, int push_dummy, int error_handling)
{
    extern symtab_t cm_symtab;
    symbol_t *sym;
    char *s1;
    int i;
    
    /* look for symbol in symbol table */
    sym = loc_symbol(&cm_symtab, s, 0);
    
    /* If symbol was not found at first, try again by adding <> around sym */
    /* this satisfies XPG4 page 56 and 44 re: valid expressions of weights */

    if (sym == NULL) {
       if ((s1 = malloc(strlen(s)+3)) == NULL)
           error(ERR_MEM_ALLOC_FAIL);

       sprintf(s1,"<%s>",s);

       /* look for symbol in symbol table */
       sym = loc_symbol(&cm_symtab, s1, 0);

       free (s1);

       if (sym != NULL)
          {
            sym_push(sym);
            return;
          }
       }

    /* if still not found, take appropriate error recovery steps.  */
    if (sym==NULL) {
	if (error_handling == FATAL)
	    error(ERR_SYM_UNDEF, s);
	else
	    diag_error(ERR_SKIP_UNDEF_SYM, s);
	
        if (push_dummy == FALSE)
            return;

	sym = create_symbol(s, 0);
	if (sym == NULL) 
	    INTERNAL_ERROR;
	
	sym->is_bogus = TRUE;
	sym->sym_type = ST_CHR_SYM;
	sym->data.chr = MALLOC(chr_sym_t, 1);

	sym->data.chr->wgt = MALLOC(_LC_weight_t, 1);
	for (i=0;i<=_COLL_WEIGHTS_MAX;i++)
	    sym->data.chr->wgt->n[i]= 0;
	set_coll_wgt(sym->data.chr->wgt, UNDEFINED, -1);

    }
    
    /* whether new or old symbol, push on symbol stack */
    sym_push(sym);
}


/* 
*  FUNCTION: sem_symbol_def
*
*  DESCRIPTION:
*  This routine is map a codepoint to a character symbol to implement
*  the 
*      <j0104>     \x81\x51
*  construct.
*
*  The routine expects to find a symbol and a numeric constant on the
*  stack.  From these two, the routine builds a character structure which
*  contains the length of the character in bytes, the file code and
*  process code representations of the character.  The character
*  structure is then pushed onto the semantic stack, and the
*  symbolic representation of the character added to the symbol table.
*
*  The routine also checks if this is the max process code yet
*  encountered, and if so resets the value of max_wchar_enc;
*/
void sem_symbol_def()
{
    extern symtab_t cm_symtab;
    extern wchar_t  max_wchar_enc;
    extern char *   max_wchar_symb;
    extern int      max_disp_width;
    extern int      mb_cur_max;
    int      width;
    symbol_t *s, *t;
    item_t   *it;
    int      fc;		/* file code for character */
    wchar_t  pc;		/* process code for character */
    int      rc;		/* return value from mbtowc_xxx */
    _LC_charmap_objhdl_t null_object={ 0, 0};
    void *   old__lc_ctype_obj;
    _LC_ctype_t bogus_ctype_obj;
    unsigned int  bogus_ctype = _ISPRINT;
    unsigned char bogus_index = 0;
    
    s = sym_pop();		/* pop symbol off stack */
    it = sem_pop();		/* pop integer to assign off stack */

    /* get file code for character off semantic stack */
    fc = it->value.int_no;

    t = loc_symbol(&cm_symtab, s->sym_id, 0);
    if (t != NULL)
	if (t->data.chr->fc_enc != fc)
	    diag_error(ERR_DUP_CHR_SYMBOL, s->sym_id);

    /* create symbol */
    s->sym_type = ST_CHR_SYM;
    s->data.chr = MALLOC(chr_sym_t, 1);

    /* save integral file code representation of character */
    s->data.chr->fc_enc = fc;

    /* turn integral file code into character string */
    s->data.chr->len = mbs_from_fc(s->data.chr->str_enc, fc);

    if (s->data.chr->len > mb_cur_max)
	error(ERR_CHAR_TOO_LONG, s->sym_id);
    
    /* get process code for this character */
    rc = CALL_METH(METH_OFFS(CHARMAP_MBTOWC))(null_object, &pc,
					      s->data.chr->str_enc, 
					      MB_LEN_MAX);
								
    if (rc < 0)
	error(ERR_UNSUP_ENC,s->sym_id);

    s->data.chr->wc_enc = pc;

    /* wcwidth() methods now call iswprint().  iswprint() requires       */
    /* a ctypes classification table, which does not exist yet.          */
    /* Unless we provide some kind of a table for isprint() to use       */
    /* iswprint() will either core dump or return the wrong value.       */
    /* We will bunt, and create an object for each character which will  */
    /* return true for iswprint().  If iswprint(), wcwidth(), or the     */
    /* locale data structures change much, this may need to be modified. */
       
    /* Commentary on this modification:  After saving the global pointer */
    /* __lc_ctype.obj, create a bogus_ctype_obj that is solely intended  */
    /* to cause iswprint() or isprint() to return true on character pc.  */
    /* The obvious solution is to create a large ctype table, but since  */
    /* this is wasteful of memory, simply calculate a pointer the        */
    /* specific character classification value, bogus_ctype.             */
    /* This code will ONLY support an isprint() operation for "pc".      */

    old__lc_ctype_obj = __OBJ_DATA(__lc_ctype);
    bogus_ctype_obj.core=__OBJ_DATA(__lc_ctype)->core;
    bogus_ctype_obj.min_wc=0;
    bogus_ctype_obj.max_wc=pc;
    if (pc<=255)
        bogus_ctype_obj.mask  = &bogus_ctype-pc;
    else {
        bogus_ctype_obj.qmask = &bogus_ctype;
        bogus_ctype_obj.qidx  = &bogus_index-(pc-256);
        }

    __OBJ_DATA(__lc_ctype) = &bogus_ctype_obj;

    /* check display width and reset max if necessary */
    width = CALL_METH(METH_OFFS(CHARMAP_WCWIDTH))(null_object, pc);

    __OBJ_DATA(__lc_ctype) = old__lc_ctype_obj;

    if (width > max_disp_width)
	max_disp_width = width;

    /* mark character as defined */
    define_wchar(pc);

    destroy_item(it);
    
    /* reset max process code in codeset */
    if (s->data.chr->wc_enc > max_wchar_enc) {
	max_wchar_enc = s->data.chr->wc_enc;
	max_wchar_symb = s->sym_id;
    }
    
    add_symbol(&cm_symtab, s);
}


/*
*  FUNCTION: extract_digit_list
*
*  DESCRIPTION:
*  This function returns the digit list which may be present in a symbol 
*  of the form <j0102> where 0102 is the desired digit list.
*
*  RETURNS:
*  Number of digits in digit list or 0 if no digit list is present.
*/
int extract_digit_list(char *s, int *value)
{
    char *endstr;
    int  i;
    int  count;

    /* skip to first digit in list */
    for (i=0; s[i] != '\0' && !isdigit(s[i]); i++);

    /* digit list present? */
    if (s[i] == '\0') 
	return 0;

    /* determine value of digit list */
    *value = strtol(&(s[i]), &endstr, 10);

    /* make sure '>' immediately follows digit list */
    if (*endstr != '>') 
	return 0;

    /* return length of digit list */
    return endstr - &(s[i]);
}


/*
*  FUNCTION: build_symbol_fmt
*
*  DESCRIPTION:
*  This function builds a format strings which describes the symbol
*  passed as an argument.  This format is used to build intermediary
*  symbols required to fill the gaps in charmap statements like:
*     <j0104>...<j0106>
*
*  RETURNS:
*  Format string and 'start/end' which when used with sprintf() results
*  in symbol that looks like sym0->sym_id.
*/
char *build_symbol_fmt(symbol_t *sym0, symbol_t *sym1, int *start, int *end)
{
    static char fmt[ST_MAX_SYM_LEN+1];

    char *s;
    char *s1;
    int  i;
    int  d_start;
    int  n_dig0;
    int  n_dig1;

    n_dig0 = extract_digit_list(sym0->sym_id, start);
    n_dig1 = extract_digit_list(sym1->sym_id, end);

    /* digit list present and same length in both symbols ? */
    if (n_dig0 != n_dig1 || n_dig0 == 0) 
	return NULL;

    /* the starting symbol is greater than the ending symbol */
    if (*start > *end)
	return NULL;

    /* build format from the start symbol */
    for (i=0, s=sym0->sym_id; !isdigit(s[i]); i++)
	fmt[i] = s[i];

    /* add to end of format "%0nd>" where n is no. of digits in list" */
    fmt[i++] = '%';
    sprintf(&(fmt[i]), "0%dd>", n_dig0);

    return fmt;
}


/*
*  FUNCTION: sem_symbol_range_def
*
*  DESCRIPTION:
*  This routine defines a range of symbol values which are defined via
*  the 
*     <j0104> ... <j0106>   \x81\x50
*  construct.
*/
void sem_symbol_range_def()
{
    extern symtab_t cm_symtab;
    extern wchar_t  max_wchar_enc;
    extern char *   max_wchar_symb;
    extern int      max_disp_width;
    extern int      mb_cur_max;
    symbol_t *s, *s0, *s1;	/* symbols pointers */
    item_t   *it;		/* pointer to mb encoding */
    int      width;		/* character display width */
    int      fc;		/* file code for character */
    int      tfc;		/* temporary for file code */
    wchar_t  pc;                /* process code for character */
    char     *fmt;		/* symbol format, e.g. "<%s%04d>" */
    char     tmp_name[ST_MAX_SYM_LEN + 1];
				/* temporary holding array for symbol name */
    int      start;		/* starting symbol number */
    int      end;		/* ending symbol number */
    int      rc;
    int      i;
    _LC_charmap_objhdl_t null_object={ 0, 0};
    void *   old__lc_ctype_obj;
    _LC_ctype_t bogus_ctype_obj;
    unsigned int  bogus_ctype = _ISPRINT;
    unsigned char bogus_index = 0;


    s1 = sym_pop();		/* symbol at end of symbol range */
    s0 = sym_pop();		/* symbol at start of symbol range */
    it = sem_pop();		/* starting encoding */

    
    /* get file code for character off semantic stack 
     */
    fc = it->value.int_no;

    /* Check if beginning symbol has already been seen
     */
    s = loc_symbol(&cm_symtab, s0->sym_id, 0);
    if (s != NULL)
	if (s->data.chr->fc_enc != fc)
	    diag_error(ERR_DUP_CHR_SYMBOL, s0->sym_id);

    /* Determine symbol format for building intermediary symbols 
     */
    fmt = build_symbol_fmt(s0, s1, &start, &end);
 
    /* Check if ending symbol has already been seen
     */
    s = loc_symbol(&cm_symtab, s1->sym_id, 0);
    if (s != NULL)
	if (s->data.chr->fc_enc != fc + (end - start))
 	    diag_error(ERR_DUP_CHR_SYMBOL, s1->sym_id);

    /* invalid symbols in range ?
     */
    if (fmt==NULL)
	error(ERR_INVALID_SYM_RNG, s0->sym_id, s1->sym_id);
    
    for (i=start; i <= end; i++) {
	
	/* reuse previously allocated symbol 
	 */
	if (i==start)
	    s = s0;
	else if (i == end)
	    s = s1;
	else {
	    sprintf(tmp_name, fmt, i);
	    s = loc_symbol(&cm_symtab,tmp_name,0);
	    if (s != NULL) {
	        if (s->data.chr->fc_enc != fc) {
		   diag_error(ERR_DUP_CHR_SYMBOL,tmp_name);
	        }
	    }
	    else {
	        s = create_symbol(tmp_name, 0);
	        if (s0==NULL)
		    INTERNAL_ERROR;
	    }
	}

	/* flesh out symbol definition 
	 */
	s->sym_type = ST_CHR_SYM;
	s->data.chr = MALLOC(chr_sym_t, 1);
	
	/* save file code */
	s->data.chr->fc_enc = fc;

	/* turn ordinal file code into character string 
	 */
	s->data.chr->len = mbs_from_fc(s->data.chr->str_enc, fc);

	if (s->data.chr->len > mb_cur_max)
	    error(ERR_CHAR_TOO_LONG, s->sym_id);
	
	/* get process code for this character 
	 */
	rc=CALL_METH(METH_OFFS(CHARMAP_MBTOWC))(null_object, &pc, 
						s->data.chr->str_enc, 
						MB_LEN_MAX);

	if (rc>=0) {
	    s->data.chr->wc_enc = pc;
	
    /* wcwidth() methods now call iswprint().  iswprint() requires       */
    /* a ctypes classification table, which does not exist yet.          */
    /* Unless we provide some kind of a table for isprint() to use       */
    /* iswprint() will either core dump or return the wrong value.       */
    /* We will bunt, and create an object for each character which will  */
    /* return true for iswprint().  If iswprint(), wcwidth(), or the     */
    /* locale data structures change much, this may need to be modified. */
       
    /* Commentary on this modification:  After saving the global pointer */
    /* __lc_ctype.obj, create a bogus_ctype_obj that is solely intended  */
    /* to cause iswprint() or isprint() to return true on character pc.  */
    /* The obvious solution is to create a large ctype table, but since  */
    /* this is wasteful of memory, simply calculate a pointer the        */
    /* specific character classification value, bogus_ctype.             */
    /* This code will ONLY support an isprint() operation for "pc".      */

            old__lc_ctype_obj = __OBJ_DATA(__lc_ctype);
            bogus_ctype_obj.core=__OBJ_DATA(__lc_ctype)->core;
            bogus_ctype_obj.min_wc=0;
            bogus_ctype_obj.max_wc=pc;
            if (pc<=255)
                bogus_ctype_obj.mask  = &bogus_ctype-pc;
            else {
                bogus_ctype_obj.qmask = &bogus_ctype;
                bogus_ctype_obj.qidx  = &bogus_index-(pc-256);
                }

 	    __OBJ_DATA(__lc_ctype) = &bogus_ctype_obj;

	    /* check display width and reset max if necessary 
	    */
	    width = CALL_METH(METH_OFFS(CHARMAP_WCWIDTH))(null_object, pc);

	    __OBJ_DATA(__lc_ctype) = old__lc_ctype_obj;

	    if (width > max_disp_width)
	      max_disp_width = width;

	    /* mark character as defined 
	     */
	    define_wchar(pc);

	    /* reset max process code in codeset 
	     */
	    if (s->data.chr->wc_enc > max_wchar_enc) {
	      max_wchar_enc = s->data.chr->wc_enc;
	      max_wchar_symb = s->sym_id;
            }
	
	    add_symbol(&cm_symtab, s);
	} else
	  diag_error(ERR_ILL_CHAR, s->data.chr->str_enc[0]);

	/* get next file code */
	fc ++;
    }
    
    destroy_item(it);
}


/*
*  FUNCTION set_csid
*
*  DESCRIPTION:
*  Assigns a csid to a character.  The routine checks if a csid table
*  has been allocated yet, and if not allocates the table.
*/
void set_csid(_LC_charmap_t *cmap, wchar_t wc, int cs_id)
{
    extern wchar_t max_wchar_enc;

    /* assign the character set id to the character */
    if (cmap->cm_cstab == NULL) {
	cmap->cm_cstab = calloc(max_wchar_enc+1, sizeof(unsigned char));
	if (cmap->cm_cstab == NULL)
	    INTERNAL_ERROR;
    }

    cmap->cm_cstab[wc] = cs_id;
}


/*
*  FUNCTION: wc_from_fc
*
*  DESCRIPTION
*  Convert a character encoding (as an integer) into a wide char
*/
int wc_from_fc(int fc)
{
    int i, j;
    wchar_t pc;
    unsigned char s[6];
    _LC_charmap_objhdl_t null_object={ 0, 0};

    bzero(s,6);
    /* shift first non-zero byte to high order byte of int */
    for(i=0; ((fc & 0xFF000000) == 0) && i < sizeof(int); i++)
	fc <<= 8;

    /* shift remaining bytes in int off top and into string */
    for (j=0; i < sizeof(int); j++,i++) {
	s[j] = (fc & 0xff000000) >> 24;
	fc <<= 8;
    }

    /* perform some sanity checking here that can't be done by mbtowc() */
    if (j >= 2) {
	/* if first byte doesn't have high bit or second byte in restricted
	   range.
	*/
	if (s[0] < 0x80 || s[1] < 0x40)
	    return -1;
    }

    i = CALL_METH(METH_OFFS(CHARMAP_MBTOWC))(null_object, &pc, s, MB_LEN_MAX);

    if (i < 0)
	return i;
    else
	return pc;
}


/*
*  FUNCTION: mbs_from_fc
*
*  DESCRIPTION
*  Convert an integral file code to a string.  The length of the character is 
*  returned.
*/
int mbs_from_fc(char *s, int fc)
{
    int     i, j;

    /* shift first non-zero byte to high order byte of int */
    for(i=0; ((fc & 0xFF000000) == 0) && i < sizeof(int); i++)
	fc <<= 8;
    
    if (i==sizeof(int)) {	/* special case for NUL character */
	s[0] = '\0'; s[1] = '\0';
	return 1;
    }

    /* shift remaining bytes in int off top and into string */
    for (j=0; i < sizeof(int); j++,i++) {
	s[j] = (fc & 0xff000000) >> 24;
	fc <<= 8;
    }
    
    return j;
}


/*
*  FUNCTION: sem_charset_range_def
*
*  DESCRIPTION:
*  This routine assigns a range of symbols to the charset specified, e.g.
*
*     <j0104> ... <j0106>   1
*/
void sem_charset_range_def(_LC_charmap_t *cmap)
{
    extern symtab_t cm_symtab;
    symbol_t *s0, *s1;	        /* symbols pointers */
    item_t   *it;		/* pointer to item containing csid */
    int      cs_id;		/* character set id */
    char     *sym_key;
    int      start;		/* starting symbol number */
    int      end;		/* ending symbol number */
    int      i;
    wchar_t  wc;

    s1 = sym_pop();		/* symbol at end of symbol range */
    s0 = sym_pop();		/* symbol at start of symbol range */
    it = sem_pop();		/* starting encoding */

    /* get csid for character range off semantic stack */
    cs_id = it->value.int_no;

    start = s0->data.chr->fc_enc;
    end = s1->data.chr->fc_enc;

    /* invalid symbols in range ?*/
    if (start > end)
	error(ERR_INVALID_SYM_RNG, s0->sym_id, s1->sym_id);
    
    for (i=start; i <= end; i++)
	if ((wc=wc_from_fc(i)) >= 0)
	    set_csid(cmap, wc, cs_id);
    
    destroy_item(it);
}


/*
*  FUNCTION: sem_charset_def
*
*  DESCRIPTION:
*  This routine assigns a character symbol to the charset specified, e.g.
*
*     <j0104>  1
*/
void sem_charset_def(_LC_charmap_t *cmap)
{
    extern symtab_t cm_symtab;
    symbol_t *s;		/* symbols pointers */
    item_t   *it;		/* pointer to item containing csid */
    int      cs_id;		/* character set id */

    s = sym_pop();		/* symbol to assign to cs */
    it = sem_pop();		/* charset id */
    
    /* get csid for character off semantic stack */
    cs_id = it->value.int_no;

    /* assign character to charset */
    set_csid(cmap, s->data.chr->wc_enc, cs_id);

    destroy_item(it);
}

/* check for digits */
void 
check_digit_values(void)
{
	extern symtab_t cm_symtab;
	symbol_t *s;
	int fc0, fc1;
	wchar_t pc0, pc1;
	int i;
	char *digits[]={
	"<zero>",
	"<one>",
	"<two>",
	"<three>",
	"<four>",
	"<five>",
	"<six>",
	"<seven>",
	"<eight>",
	"<nine>" };

	s = loc_symbol(&cm_symtab,digits[0],0);
	if (s == NULL) {
            diag_error(ERR_NO_SYMBOLIC_DIGIT,digits[0]);
	    return;
            }
	fc0 = s->data.chr->fc_enc;
	pc0 = s->data.chr->wc_enc;
	for (i = 1; i <= 9; i++) {
	    s = loc_symbol(&cm_symtab,digits[i],0);
	    if (s == NULL) {
                diag_error(ERR_NO_SYMBOLIC_DIGIT,digits[i]);
		return;
                }
	    fc1 = s->data.chr->fc_enc;
	    pc1 = s->data.chr->wc_enc;
	    if ((fc0 + 1) != fc1)
		diag_error(ERR_DIGIT_FC_BAD,digits[i],digits[i-1]);
	    if ((pc0 + 1) != pc1)
		diag_error(ERR_DIGIT_PC_BAD,digits[i],digits[i-1]);
	    fc0 = fc1;
	    pc0 = pc1;
	}
	return;
}
