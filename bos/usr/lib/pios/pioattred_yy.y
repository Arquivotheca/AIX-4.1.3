/* @(#)97       1.4  src/bos/usr/lib/pios/pioattred_yy.y, cmdpios, bos411, 9428A410j 7/23/93 16:23:14 */
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

%token COMMA R_BRAK ATTR CHAR
%token PERC_PERC PERC_d PERC_num_d PERC_c PERC_h PERC_a
%token PERC_I PERC_I_brak PERC_G PERC_sq PERC_dq PERC_brac
%token PERC_S PERC_L PERC_P PERC_Z PERC_g
%token PERC_math PERC_if PERC_then PERC_else PERC_endif PERC_x PERC_w
%token PERC_o PERC_r PERC_i PERC_p PERC_z PERC_C
%token PERC_F PERC_f PERC_F_brak PERC_f_brak PERC_v PERC_v_brak PERC_U PERC_U_brak
%token PERC_D PERC_bq
%token PERC_REGEXP

%start prparms

%{

#   include "pioattred.h"

#define REGPTRN_DELIMITER       '"'
#define REGPTRN_DIVIDER		'@'

extern int yytchar;

%}

%%

prparms     :   paramseq
            ;

paramseq    :   param
            |   paramseq param
            ;

param       :   PERC_PERC
                    {
                    if ( disp )
                        show_op(OP_PERC,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_d
                    {
                    if ( disp )
                        show_op(OP_d,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_num_d
                    {
                    if ( disp )
                        show_op(OP_num_d,tok_name,CHR_FIELD,"",$1);
                    }

            |   PERC_c
                    {
                    if ( disp )
                        show_op(OP_c,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_h
                    {
                    if ( disp )
                        show_op(OP_h,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_a
                    {
                    if ( disp )
                        show_op(OP_a,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_I
                    {
                    if ( disp )
                      show_op(OP_I,tok_name,STR_FIELD,find_desc(tok_name+2),0);
                    }

            |   PERC_I_brak attrlist R_BRAK
                    {
                    if ( disp )
                        {
                        char *ptr1 = guts;
                        char *ptr2 = guts;

                        do{ while ( *ptr2 && *ptr2 != ',' ) ptr2++;
                            if ( *ptr2 )
                            {
                            *ptr2 = NULL;
                            *operator = '%'; /* so SCCS won't see % I % */
                            sprintf(operator + 1,"I%s",ptr1);
                            show_op(OP_I,operator,STR_FIELD,find_desc(ptr1),0);
                            ptr1 = (++ptr2);
                            }
                            else
                            {
                            *operator = '%'; /* so SCCS won't see % I % */
                            sprintf(operator + 1,"I%s",ptr1);
                            show_op(OP_I,operator,STR_FIELD,find_desc(ptr1),0);
                            ptr1 = NULL;
                            }
                            } while ( *ptr1 );
                        }
                    }

            |   PERC_I_brak error R_BRAK
            |   PERC_I_brak error
                    {
                    if ( disp )
                        {
                        sprintf(operator,"\'%s\'",this);
                        parseerr = TRUE, show_op(OP_SYNTAX,operator,NO_FIELD,"",0);
                        }
                    synt_err += 1;
                    yyerrok;
                    yyclearin;
                    }

            |   PERC_G
                    {
                    if ( disp )
                      show_op(OP_G,tok_name,STR_FIELD,find_desc(tok_name+2),0);
                    }

            |   PERC_sq
                    {
                    if ( disp )
                        show_op(OP_char,tok_name,CHR_FIELD,"",$1);
                    }

            |   PERC_dq
                    {
                    if ( disp )
                        {
                        char *ptr = guts;
                        sprintf(guts,"%s",tok_name+1);
                        while ( *ptr++ );
                        *(ptr-1) = NULL;
                        show_op(OP_STRING_CONST,tok_name,STR_FIELD,guts,0);
                        }
                    }

            |   PERC_brac
                    {
                    if ( disp )
                        show_op(OP_INTEGER_CONST,tok_name,NUM_FIELD,"",$1);
                    }

            |   PERC_S
                    {
                    if ( disp )
                      show_op(OP_S,tok_name,STR_FIELD,find_desc(tok_name+2),0);
                    }

            |   PERC_L
                    {
                    if ( disp )
                      show_op(OP_L,tok_name,STR_FIELD,find_desc(tok_name+2),0);
                    }

            |   PERC_P
                    {
                    if ( disp )
                        show_op(OP_P,tok_name,CHR_FIELD,"",$1);
                    }

            |   PERC_Z
                    {
                    if ( disp )
                        show_op(OP_Z,tok_name,CHR_FIELD,"",$1);
                    }

            |   PERC_g
                    {
                    if ( disp )
                        show_op(OP_g,tok_name,CHR_FIELD,"",$1);
                    }

            |   PERC_math
                    {
                    if ( disp )
                        {
                        char math_symb[] = "+-*/m=><!&|^~";
                        show_op(OP_add+(int)strchr(math_symb,$1)-(int)math_symb,
                                tok_name,NO_FIELD,"",0);
                        }
                    }

            |   PERC_x
                    {
                    if ( disp )
                        show_op(OP_x,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_o
                    {
                    if ( disp )
                        show_op(OP_o,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_r
                    {
                    if ( disp )
                        show_op(OP_r,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_i
                    {
                    if ( disp )
                        {
                        if ( $1 == '!' )
                            show_op(OP_i_null,tok_name,NO_FIELD,"",0);
                        else
                            show_op(OP_i,tok_name,CHR_FIELD,"",$1);
                        }
                    }

            |   PERC_p
                    {
                    if ( disp )
                        show_op(OP_p,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_z
                    {
                    if ( disp )
                        show_op(OP_z,tok_name,NO_FIELD,"",0);
                    }

            |   PERC_C
                    {
                    if ( disp )
                        show_op(OP_C,tok_name,CHR_FIELD,"",$1);
                    }

            |   PERC_F
                    {
                    if ( disp )
                        show_op(OP_F,tok_name,STR_FIELD,tok_name+2,0);
                    }

            |   PERC_F_brak
                    {
                    if ( disp )
                        {
                        char *ptr = guts;
                        sprintf(guts,"%s",tok_name+3);
                        while ( *ptr++ );
                        *(ptr-1) = NULL;
                        show_op(OP_F,tok_name,STR_FIELD,guts,0);
                        }
                    }

            |   PERC_f
                    {
                    if ( disp )
                        show_op(OP_f,tok_name,STR_FIELD,tok_name+2,0);
                    }

            |   PERC_f_brak
                    {
                    if ( disp )
                        {
                        char *ptr = guts;
                        sprintf(guts,"%s",tok_name+3);
                        while ( *ptr++ );
                        *(ptr-1) = NULL;
                        show_op(OP_f,tok_name,STR_FIELD,guts,0);
                        }
                    }

            |   PERC_v
                    {
                    if ( disp )
                        show_op(OP_v,tok_name,STR_FIELD,tok_name+2,0);
                    }

            |   PERC_v_brak
                    {
                    if ( disp )
                        {
                        char *ptr = guts;
                        sprintf(guts,"%s",tok_name+3);
                        while ( *ptr++ );
                        *(ptr-1) = NULL;
                        show_op(OP_v,tok_name,STR_FIELD,guts,0);
                        }
                    }

            |   PERC_U
                    {
                    if ( disp )
                        show_op(OP_U,tok_name,STR_FIELD,tok_name+2,0);
                    }

            |   PERC_U_brak
                    {
                    if ( disp )
                        {
                        char *ptr = guts;
                        sprintf(guts,"%s",tok_name+3);
                        while ( *ptr++ );
                        *(ptr-1) = NULL;
                        show_op(OP_U,tok_name,STR_FIELD,guts,0);
                        }
                    }

            |   PERC_w
                    {
                    if ( disp )
                        show_op(OP_w,tok_name,NO_FIELD,"",0);
                    indent += 4;
                    if_while += 1;
                    }

            |   PERC_if
                    {
                    if ( disp )
                        show_op(OP_if,tok_name,NO_FIELD,"",0);
                    indent += 4;
                    if_while += 1;
                    }

            |   PERC_then
                    {
                    indent -= 4;
                    if ( disp )
                        show_op(OP_then,tok_name,NO_FIELD,"",0);
                    indent += 4;
                    }

            |   PERC_else
                    {
                    indent -= 4;
                    if ( disp )
                        show_op(OP_else,tok_name,NO_FIELD,"",0);
                    indent += 4;
                    }

            |   PERC_endif
                    {
                    indent -= 4;
                    if ( disp )
                        show_op(OP_endif,tok_name,NO_FIELD,"",0);
                    if_while -= 1;
                    }

            |   PERC_D
                    {
                    if ( disp )
                        show_op(OP_D,tok_name,STR_FIELD,tok_name+2,0);
                    }

            |   PERC_bq
                 {
		    /* If the operator (%`) was followed by an attribute
		       name, say so.  Else (followed by a double-quoted
		       string), parse the specified string to look for
		       end delimiter, and then display the string. */
		    if (*(tok_name + 2) != REGPTRN_DELIMITER)
		    {
                       if ( disp )
                          show_op (OP_BACKQUOTE, tok_name, STR_FIELD,
				   tok_name + 2, 0);
                    }
		    else
		    {
   		       register char 		*ptr = guts;
		       int			enddelimit_found = FALSE;
		       register unsigned int	noof_bkslshs = 0;
   
		       *(tok_name + 2) = 0;
		       for (*ptr++ = REGPTRN_DELIMITER;
			    !enddelimit_found  && (*ptr = input ()); ptr++)
		       {
	                  switch (*ptr)
	                  {
	                     case '\\':
	     	                noof_bkslshs++;
	     	                break;
   
	                     case REGPTRN_DELIMITER:
	     	                if (!((colonf_input ? noof_bkslshs / 2 : 
				       noof_bkslshs) % 2))
	     	                   enddelimit_found = TRUE;
		                noof_bkslshs = 0;
	     	                break;
   
	                     default:
	     	                noof_bkslshs = 0;
	     	                break;
	                  }
		       }
		       *ptr = 0;
		          
                       if ( disp )
		       {
		          if (enddelimit_found)
		          {
			     (void) sprintf (operator, "\n '%s'", guts);
                             show_op (OP_BACKQUOTE1, tok_name, STR_FIELD,
				      operator, 0);
		          }
		          else
		          {
			     (void) strncpy (operator, tok_name,
					     sizeof (operator) -1);
			     (void) strncat (operator, guts, sizeof (operator)
					     - 1 - strlen (operator));
                             parseerr = TRUE,
			     show_op (OP_SYNTAX, operator, NO_FIELD, "", 0);
		          }
		       }
   
		       if (!enddelimit_found)
		       {
		          synt_err += 1;
		          yyerrok;
		          yyclearin;
		       }
		    }
                 }			/* end - PERC_bq */

            |   PERC_REGEXP
                 {
		    register char 		*ptr = guts;
		    int				proper_syntax = TRUE;
		    int				enddelimit_found = FALSE;
	            int				divider_found = FALSE;
		    register unsigned int	noof_bkslshs = 0;

		    if (!(*ptr = input ())  ||  *ptr != REGPTRN_DELIMITER)
		    {
		       if (*ptr)
			  (void) unput ((int) *ptr), ptr++;
		       proper_syntax = FALSE;
		    }
		    else
		    {
		       for (ptr++; !enddelimit_found  && (*ptr = input ());
			    ptr++)
		       {
	                  switch (*ptr)
	                  {
	                     case '\\':
	     	                noof_bkslshs++;
	     	                break;

	                     case REGPTRN_DIVIDER:
	     	                if (!divider_found  &&
				    !((colonf_input ? noof_bkslshs / 2 :
				       noof_bkslshs) % 2))
	     	                   divider_found = TRUE;
		                noof_bkslshs = 0;
	     	                break;

	                     case REGPTRN_DELIMITER:
	     	                if (!((colonf_input ? noof_bkslshs / 2 : 
				       noof_bkslshs) % 2))
	     	                   enddelimit_found = TRUE;
		                noof_bkslshs = 0;
	     	                break;

	                     default:
	     	                noof_bkslshs = 0;
	     	                break;
	                  }
	               }
		       *ptr = 0;
		       
		       if (!enddelimit_found  ||  !divider_found)
			  proper_syntax = FALSE;
		    }

                    if ( disp )
		    {
		       if (proper_syntax)
		       {
			  (void) sprintf (operator, "\n '%s'", guts);
                          show_op (OP_REGEXP, tok_name, STR_FIELD, operator, 0);
		       }
		       else
		       {
			  (void) strncpy (operator, tok_name,
					  sizeof (operator) -1);
			  (void) strncat (operator, guts, sizeof (operator) - 1
					  - strlen (operator));
                          parseerr = TRUE,
			  show_op (OP_SYNTAX, operator, NO_FIELD, "", 0);
		       }
		    }

		    if (!proper_syntax)
		    {
		       synt_err += 1;
		       yyerrok;
		       yyclearin;
		    }
                 }			/* end - PERC_REGEXP */

            |   error
                    {
/*
**  Hang on for some strangeness!
**
**  We can only be here because none of the rules for forming valid
**  operators could be obeyed.  Therefore, what we must do is gather
**  up the gorp that has accumulated in 'guts', add to it the rest
**  of whatever crud may follow, then display it.
**
**  We can always know that all will be well if we can at least restart
**  when we find another percent character.
**
**  We will first glance at the first character in guts.  If that
**  character is a percent character, then the user has probably tried
**  to enter some sort of operator and fouled it up.  Since no spaces
**  can be included in an operator, both the space and percent character
**  can be considered 'reset' characters.
**
**  If the first character in guts is anything else, we will treat it and
**  whatever follows as text, and protect it with single quotes.  In this
**  case, only the percent character is considered a 'reset' character.
**
**  As we begin sucking characters off the input data stream, we compare
**  the character just read with the string of reset characters.  When
**  a reset character is found, it is stuffed back onto the data stream
**  (actually into an 'unput buffer' where it will be found again the next
**  time a character is read) then the string is displayed.  A closing
**  quote is added if needed to protect a text string.
*/
                    char *ptr = guts;
                    char reset[3], *c;

                    if ( *tok_name == '%' )
                        {
                        strcpy(reset," %");
                        strcpy(guts,tok_name);
                        }
                    else
                        {
                        strcpy(reset,"%");
                        sprintf(guts,"\'%s",tok_name);
                        }

                    /*
                    **  Move pointer to terminating null char
                    */
                    while ( *ptr ) ptr++;
                    while ( *ptr = input() )
                        {
                        if ( c = strchr(reset,(int)*ptr) )
                            {
                            unput((int)*c);
                            break;
                            }
                        ptr++;
                        }
                    if ( !(*ptr) || c )
                        {
                        if ( *guts != '%' ) *ptr++ = '\'';
                        *ptr = NULL;
                        }

                    if ( disp )
                      show_op(strchr(guts,'%')?(parseerr = TRUE, OP_SYNTAX):0,guts,NO_FIELD,"",0);

                    synt_err += strchr(guts,'%')?1:0;
                    yyerrok;
                    yyclearin;
                    }
            ;

attrlist    :   ATTR
                    { strcpy(guts,tok_name); }

            |   attrlist COMMA ATTR
                    {
                    strcat(guts,",");
                    strcat(guts,tok_name);
                    }
            ;

%%

void yyerror(msg)
char *msg;
{
/*
**  This is here to do nothing but suppress the normal error processing.
**  Without it, unwanted messages would be written and crazy behavior
**  would ensue from a syntax error.
*/
}
