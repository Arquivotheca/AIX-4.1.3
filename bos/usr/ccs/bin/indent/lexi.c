static char sccsid[] = "@(#)11  1.13.1.4  src/bos/usr/ccs/bin/indent/lexi.c, cmdprog, bos411, 9428A410j 4/9/94 12:25:58";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: addkey, has_trigraph, lexi
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *
 *                        Copyright (C) 1976
 *                              by the
 *                        Board of Trustees
 *                              of the
 *                      University of Illinois
 *
 *                       All rights reserved
 */

/*-
 *
 *
 * NAME:
 *      lexi
 *
 * FUNCTION:
 *      This is the token scanner for indent
 *
 * ALGORITHM:
 *      1) Strip off intervening blanks and/or tabs.
 *      2) If it is an alphanumeric token, move it to the token buffer "token".
 *         Check if it is a special reserved word that indent will want to
 *         know about.
 *      3) Non-alphanumeric tokens are handled with a big switch statement.  A
 *         flag is kept to remember if the last token was a "unary delimiter",
 *         which forces a following operator to be unary as opposed to binary.
 *
 * PARAMETERS:
 *      None
 *
 * RETURNS:
 *      An integer code indicating the type of token scanned.
 *
 * GLOBALS:
 *      buf_ptr =
 *      had_eof
 *      ps.last_u_d =   Set to true iff this token is a "unary delimiter"
 *
 * CALLS:
 *      fill_buffer
 *      printf (lib)
 *
 * CALLED BY:
 *      main
 *
 * NOTES:
 *      Start of comment is passed back so that the comment can be scanned by
 *      pr_comment.
 *
 *      Strings and character literals are returned just like identifiers.
 *
 * HISTORY:
 *      initial coding  November 1976   D A Willcox of CAC
 *      1/7/77          D A Willcox of CAC      Fix to provide proper handling
 *                                              of "int a -1;"
 *
 */

/*
 * Here we have the token scanner for indent.  It scans off one token and
 * puts it in the global variable "token".  It returns a code, indicating
 * the type of token scanned.
 */

#include "indent_msg.h"
#include "ind_globs.h"
#include "ind_codes.h"
#include <ctype.h>

#define alphanum 1
#define opchar 3

/*D46827 - Number of elements to expand the keyword table by.*/
#define NUMBER_OF_ELEMENTS 20 

struct templ {
    char       *rwd;
    int         rwcode;
};

/*D46827 - Change the name of specials to specials_table.*/
struct templ specials_table[100] =
{
    "switch", 1,
    "case", 2,
    "break", 0,
    "struct", 3,
    "union", 3,
    "enum", 9,
    "default", 2,
    "auto", 4,
    "int", 4,
    "char", 4,
    "float", 4,
    "double", 4,
    "long", 4,
    "short", 4,
    "typedef", 4,
    "unsigned", 4,
    "register", 4,
    "static", 4,
    "global", 4,
    "extern", 4,
    "void", 4,
    "const", 4,
    "noalias", 4,
    "signed", 4,
    "volatile", 4,
    "goto", 0,
    "return", 0,
    "if", 5,
    "while", 5,
    "for", 5,
    "else", 6,
    "do", 6,
    "sizeof", 7,
    "L", 8,
    0, 0
};

/*D46827 - Points to beginning of the keyword table (initially specials_table)*/
struct templ *specials = specials_table;

/*D46827-Index of last valid element in keyword table (overall table size - 1)*/
long specials_index = (sizeof(specials_table) / sizeof(specials_table[0])) - 1;

char        chartype[128] =
{                               /* this is used to facilitate the decision
                                 * of what type (alphanumeric, operator)
                                 * each character is */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 0, 0, 1, 3, 3, 0,
    0, 0, 3, 3, 0, 3, 3, 3,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 0, 3, 3, 3, 3,
    0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 3, 1,
    0, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 3, 0, 3, 0
};




int
lexi()
{
    register char *tok;                         /* local pointer to next char in token */
    int         unary_delim;                    /* this is set to 1 if the current token forces a following operator to be unary */
    static int  last_code;                      /* the last token type returned */
    static int  l_struct;                       /* set to 1 if the last token was 'struct' */
    int         code;                           /* internal code to be returned */
    char        qchar;                          /* the delimiter character for a string */
    char        *cc;
    int         i;
    int         n;
    char        ttok;
    
    tok = token;                                /* point to start of place to save token */
    unary_delim = false;
    ps.col_1 = ps.last_nl;                      /* tell world that this token started in column 1 */
    ps.last_nl = false;                         /* ...iff the last thing scanned was nl */

    while (*buf_ptr == ' ' || *buf_ptr == '\t') /* get rid of blanks */
    {
        ps.col_1 = false;                       /* leading blanks imply token is not in column 1 */
        if (++buf_ptr >= buf_end)
            fill_buffer();
    }

    /*
     * Scan an alphanumeric token.  Note that we must also handle
     * stuff like "1.0e+03" and "7e-6".
     */
    if (has_iswalnum (buf_ptr))                 /* we have a character or number (identifier) */
    {
        register char *j;                       /* used for searching thru list of reserved words */
        register struct templ *p;
        register int c;
        do                                      /* copy it over */
        {
            n = cpymbc (tok, buf_ptr);
            buf_ptr += n;
            tok += n;
            if (buf_ptr >= buf_end)
                fill_buffer();
        } while (   has_iswalnum (buf_ptr)
                 || (   (isascii(*buf_ptr))
                     && (   (chartype[c = *buf_ptr & 0177] == alphanum)
                         || (   isdigit(token[0])
                             && ( (c == '.') || ((c == '+' || c == '-') &&
                                (tok[-1] == 'e' || tok[-1] == 'E')))))));
        *tok++ = '\0';
        while (*buf_ptr == ' ' || *buf_ptr == '\t') /* get rid of blanks */
        {
            if (++buf_ptr >= buf_end)
                fill_buffer();
        }
        ps.its_a_keyword = false;
        ps.sizeof_keyword = false;
        if (l_struct) {         /* if last token was 'struct', then this
                                 * token should be treated as a
                                 * declaration */
            l_struct = false;
            last_code = ident;
            ps.last_u_d = true;
            return (decl);
        }
        ps.last_u_d = false;    /* Operator after indentifier is binary */
        last_code = ident;      /* Remember that this is the code we will
                                 * return */

        /*
         * This loop will check if the token is a keyword.
         */
        for (p = specials; (j = p->rwd) != 0; p++)
        {
            tok = token;        /* point at scanned token */
            if (*j++ != *tok++ || *j++ != *tok++)
                continue;       /* This test depends on the fact that
                                 * identifiers are always at least 1
                                 * character long (ie. the first two bytes
                                 * of the identifier are always
                                 * meaningful) */
            if (tok[-1] == 0)
                break;          /* If its a one-character identifier */
            while (*tok++ == *j)
                if (*j++ == 0)
                    goto found_keyword; /* I wish that C had a multi-level
                                         * break... */
        }
        if (p->rwd) {           /* we have a keyword */
    found_keyword:
            ps.its_a_keyword = true;
            ps.last_u_d = true;
            switch (p->rwcode) {
                case 1: /* it is a switch */
                    return (swstmt);
                case 2: /* a case or default */
                    return (casestmt);

                case 9: /* an enum */
                    ps.enum_decl = 1;
                case 3: /* a "struct" */
                    if (ps.p_l_follow)
                        break;  /* inside parens: cast */
                    l_struct = true;

                    /*
                     * Next time around, we will want to know that we have
                     * had a 'struct'
                     */
                case 4: /* one of the declaration keywords */
                    if (ps.p_l_follow)
                    {
                        for (cc = buf_ptr;
                                *cc && (*cc == ' ' || *cc == '*'); cc++);
                        if (ps.leave_comma || *cc == ')' || (ps.ind_level > 0 && !ps.in_decl))
                        {
                            ps.cast_mask |= 1 << ps.p_l_follow;
                            break;      /* inside parens: cast */
                        }
                        else
                            new_func_proto = true;
                    }
                    last_code = decl;
                    return (decl);

                case 5: /* if, while, for */
                    return (sp_paren);

                case 6: /* do, else */
                    return (sp_nparen);

                case 8:                         /* L */
                    return (lstring);
                    
                case 7:
                    ps.sizeof_keyword = true;
                default:        /* all others are treated like any other
                                 * identifier */
                    return (ident);
            }                   /* end of switch */
        }                       /* end of if (found_it) */
        if (   *buf_ptr == '(' && ps.tos <= 1 && ps.ind_level == 0
            && (buf_ptr[1] != ')' || buf_ptr[2] != ';'))
        {
            strncpy(ps.procname, token, sizeof ps.procname - 1);
            ps.in_parameter_declaration = 1;
        }

        /*
         * The following hack attempts to guess whether or not the current
         * token is in fact a declaration keyword -- one that has been
         * typedefd
         */
        if (   ((*buf_ptr == '*' && buf_ptr[1] != '=') || isalpha(*buf_ptr))
            && !ps.p_l_follow
            && (   ps.last_token == rparen || ps.last_token == semicolon
                || ps.last_token == decl || ps.last_token == lbrace || ps.last_token == rbrace))
        {
            ps.its_a_keyword = true;
            ps.last_u_d = true;
            last_code = decl;
            return decl;
        }
        if (last_code == decl)  /* if this is a declared variable, then
                                 * following sign is unary */
            ps.last_u_d = true; /* will make "int a -1" work */
        last_code = ident;
        return (ident);         /* the ident is not in the list */
    }                           /* end of procesing for alpanum character */
    /*
     * Scan a non-alphanumeric token
     * if it is only a one-character token, it is moved here
     */
    n = cpymbc (tok, buf_ptr);
    tok += n;
    buf_ptr += n;
    *tok = '\0';
    if (buf_ptr >= buf_end)
        fill_buffer();

    switch (*token) {
        case '\n':
            unary_delim = ps.last_u_d;
            ps.last_nl = true;  /* remember that we just had a newline */
            code = (had_eof ? 0 : newline);

            /*
             * if data has been exausted, the newline is a dummy, and we
             * should return code to stop
             */
            break;

        case '\'':              /* start of quoted character */
        case '"':               /* start of string */
            qchar = *token;
            if (troff) {
                tok[-1] = '`';
                if (qchar == '"')
                    *tok++ = '`';
                *tok++ = BACKSLASH;
                *tok++ = 'f';
                *tok++ = 'L';
            }
            do {                /* copy the string */
                while (1) {     /* move one character or [/<char>]<char> */
                    if (*buf_ptr == '\n') {
                        printf(MSGSTR(NOENDLITERAL, "%d: Unterminated literal\n"), line_no); /*MSG*/
                        goto stop_lit;
                    }
                    buf_ptr += cpymbc (tok, buf_ptr);

                    if (buf_ptr >= buf_end)
                        fill_buffer();

                    if (had_eof || ((tok - token) > (bufsize - 2)))
                    {
                        printf(MSGSTR(NOENDLIT, "Unterminated literal\n")); /*MSG*/
                        tok = nextmbc (tok);
                        goto stop_lit;
                        /* get outof literal copying loop */
                    }
                    if (*tok == BACKSLASH) {    /* if escape, copy extra char */
                        if (*buf_ptr == '\n')   /* check for escaped newline */
                            ++line_no;
                        if (troff) {
                            *++tok = BACKSLASH;
                                                /* escaped regular escape */
                            if (*buf_ptr == BACKSLASH)
                                *++tok = BACKSLASH;
                                                /* or escaped trigraph escape */
                            else if (*buf_ptr == '?' && buf_ptr[1] == '?' && buf_ptr[2] == '/')
                            {
                                *++tok = BACKSLASH;
                                buf_ptr++;      /* skip trigraph */
                                buf_ptr++;
                                if (buf_ptr >= buf_end)
                                    fill_buffer();
                            }
                        }
                        tok = nextmbc (tok);    /* skip to the next slot */
                        i = cpymbc (tok, buf_ptr); /* copy in the escaped char */
                        tok += i;               /* skip the copied char */
                        buf_ptr += i;

                        if (buf_ptr >= buf_end)
                            fill_buffer();
                    }
                                                /* trigraph escape */
                    else if (*tok == '?' &&
                                has_trigraph(prevmbc(buf_ptr)) == '\\')
                    {
                        /* if escape, copy extra char */
                        *++tok = *buf_ptr++;    /* '?' */
                        *++tok = *buf_ptr++;    /* '/' */
                        if (buf_ptr >= buf_end)
                            fill_buffer();
                        if (*buf_ptr == '\n')   /* check for escaped newline */
                            ++line_no;
                        if (troff) {
                            tok[-2] = BACKSLASH;
                            tok[-1] = BACKSLASH;
                            tok--;
                            if (*buf_ptr == BACKSLASH)
                                *++tok = BACKSLASH;
                            else if (has_trigraph(buf_ptr) == '\\')
                            {
                                *++tok = BACKSLASH;
                                buf_ptr++;
                                buf_ptr++;
                                if (buf_ptr >= buf_end)
                                    fill_buffer();
                            }
                        }
                        if ( strlen(buf_ptr) > 3 && *buf_ptr == '?'
                             && buf_ptr[1] == '?' &&
                                (buf_ptr[2] == '/' || buf_ptr[2] == '\'') )
                        {
                            *++tok = *buf_ptr++;        /* '?' */
                            *++tok = *buf_ptr++;        /* '?' */
                        }

                        tok = nextmbc (tok);    /* skip to the next slot */
                        i = cpymbc (tok, buf_ptr); /* copy in the escaped char */
                        tok += i;               /* skip the copied char */
                        buf_ptr += i;

                        if (buf_ptr >= buf_end)
                            fill_buffer();
                    }
                    else
                        break;  /* we copied one character */
                }               /* end of while (1) */

                ttok = *tok;
                tok  = nextmbc (tok);
            } while (ttok != qchar);

            if (troff) {
                tok[-1] = BACKSLASH;
                *tok++ = 'f';
                *tok++ = 'R';
                *tok++ = '\'';
                if (qchar == '"')
                    *tok++ = '\'';
            }
    stop_lit:
            code = ident;
            break;

        case ('('):
        case ('['):
            unary_delim = true;
            code = lparen;
            break;

        case (')'):
        case (']'):
            code = rparen;
            break;

        case '#':
            unary_delim = ps.last_u_d;
            code = preesc;
            break;

        case '?':
            if (*buf_ptr == '?' )
            {
                *tok++ = *buf_ptr;
                if (++buf_ptr >= buf_end)
                    fill_buffer();
                if  ( *buf_ptr == '\''  ||  *buf_ptr == '-'
                                        ||  *buf_ptr == '/' )
                {
                    code = (ps.last_u_d ? unary_op : binary_op);
                    unary_delim = true;
                }
                else if ( *buf_ptr == '!' )
                {
                    if (has_trigraph(&buf_ptr[1]) == '|')
                    {
                        *tok++ = *buf_ptr++;    /* '!' */
                        *tok++ = *buf_ptr++;    /* '?' */
                        *tok++ = *buf_ptr++;    /* '?' */
                    }
                    code = (ps.last_u_d ? unary_op : binary_op);
                    unary_delim = true;
                }
                else if ( *buf_ptr == '=' )
                {
                    unary_delim = ps.last_u_d;
                    code = preesc;
                }
                else if ( *buf_ptr == '(' )
                {
                    unary_delim = true;
                    code = lparen;
                }
                else if ( *buf_ptr == ')' )
                    code = rparen;
                else if ( *buf_ptr == '<' )
                {
                    unary_delim = true;
                    code = ps.block_init ? lparen : lbrace;
                }
                else if ( *buf_ptr == '>' )
                {
                    unary_delim = true;
                    code = ps.block_init ? rparen : rbrace;
                }
                else
                    break;
                *tok++ = *buf_ptr;
                if (++buf_ptr >= buf_end)
                    fill_buffer();
                break;
            }
            else /* got a real ? : statement */
            {
                unary_delim = true;
                code = question;
                break;
            }

        case (':'):
            code = colon;
            unary_delim = true;
            break;

        case (';'):
            unary_delim = true;
            code = semicolon;
            break;

        case ('{'):
            unary_delim = true;

            /*
             * if (ps.in_or_st) ps.block_init = 1;
             */
            code = ps.block_init ? lparen : lbrace;
            break;

        case ('}'):
            unary_delim = true;
            code = ps.block_init ? rparen : rbrace;
            break;

        case 014:               /* a form feed */
            unary_delim = ps.last_u_d;
            ps.last_nl = true;  /* remember this so we can set 'ps.col_1'
                                 * right */
            code = form_feed;
            break;

        case (','):
            unary_delim = true;
            code = comma;
            break;

        case '.':
            unary_delim = false;
            code = period;
            break;

        case '-':
        case '+':               /* check for -, +, --, ++ */
            code = (ps.last_u_d ? unary_op : binary_op);
            unary_delim = true;

            if (*buf_ptr == token[0]) {
                /* check for doubled character */
                *tok++ = *buf_ptr++;
                /* buffer overflow will be checked at end of loop */
                if (last_code == ident || last_code == rparen) {
                    code = (ps.last_u_d ? unary_op : postop);
                    /* check for following ++ or -- */
                    unary_delim = false;
                }
            }
            else if (*buf_ptr == '=')
                /* check for operator += */
                *tok++ = *buf_ptr++;
            else if (token[0] == '-' && *buf_ptr == '>') {
                /* check for operator -> */
                *tok++ = *buf_ptr++;
                if (!pointer_as_binop) {
                    code = unary_op;
                    unary_delim = false;
                    ps.want_blank = false;
                }
            }
            /* buffer overflow will be checked at end of switch */

            break;

        case '=':
            if (ps.in_or_st && !ps.enum_decl)
                ps.block_init = 1;
                /* 10/17/90 GH fix for apar ix13936 */
            if (flip_assign_oper) {
                if (isascii(*buf_ptr) && (chartype[*buf_ptr] == opchar)) /* we have two char * assignment */
                {
                    tok[-1] = *buf_ptr++;
                    if(tok[-1] != '=')    /* GH 23130 */
                        diag(0,MSGSTR(FLIPOPER,"meaning may have been altered -- operators were flipped\n")); /*MSG*/ 
                    if ((tok[-1] == '<' || tok[-1] == '>') && tok[-1] == *buf_ptr)
                        *tok++ = *buf_ptr++;
                    *tok++ = '=';   /* Flip =+ to += */
                    *tok = 0;
                }
            }
		  /* 07/21/92 EH fix for defect 61041 */
            else { /* handle the case of '==' */
                if (*buf_ptr == '=') {
                    *tok++ = '=';
				*tok = 0;
				buf_ptr++;
                }
            }
		  /* end of defect 61041 fix */
            code = binary_op;
            unary_delim = true;
            break;
            /* can drop thru!!! */
        case '>':
        case '<':
        case '!':               /* ops like <, <<, <=, !=, etc */
            if (*buf_ptr == '>' || *buf_ptr == '<' || *buf_ptr == '=') {
                *tok++ = *buf_ptr;
                if (++buf_ptr >= buf_end)
                    fill_buffer();
            }
            if (*buf_ptr == '=')
                *tok++ = *buf_ptr++;
            code = (ps.last_u_d ? unary_op : binary_op);
            unary_delim = true;
            break;

        default:
            if (token[0] == '/' && *buf_ptr == '*') {
                /* it is start of comment */
                *tok++ = '*';

                if (++buf_ptr >= buf_end)
                    fill_buffer();

                code = comment;
                unary_delim = ps.last_u_d;
                break;
            }
            while (*(tok - 1) == *buf_ptr || *buf_ptr == '=') {
                /* handle ||, &&, etc, and also things as in int *****i */
                *tok++ = *buf_ptr;
                if (++buf_ptr >= buf_end)
                    fill_buffer();
            }
            code = (ps.last_u_d ? unary_op : binary_op);
            unary_delim = true;


    }                           /* end of switch */
    if (code != newline) {
        l_struct = false;
        last_code = code;
    }
    if (buf_ptr >= buf_end)     /* check for input buffer empty */
        fill_buffer();
    ps.last_u_d = unary_delim;
    *tok = '\0';                /* null terminate the token */
    return (code);
}

/*
 * Add the given keyword to the keyword table, using val as the keyword type
 * Space needs to be created to store the key (as it may have come from a
 * temporary buffer used to read the profile.
 */
addkey (key, val)
char       *key;
{
    register struct templ *p = specials;
    while (p->rwd)
        if (p->rwd[0] == key[0] && strcmp(p->rwd, key) == 0)
            return;
        else
            p++;

    /*D46827 - The current keyword (key) was not found in the keyword 
    table pointed to by specials.  If there are no more free entries in
    the keyword table, then increase its' size.  (The keyword table use
    to be a fixed size and overflows were silently ignored.)*/
    if (p >= (specials + specials_index)) {
         specials_index += NUMBER_OF_ELEMENTS;

         /*If needed, allocate and initialize new space for pointer specials.
         (Cannot realloc() specials initially as it points to specials_table;
         do not want realloc() to attempt to free specials_table!)*/
         if (specials == specials_table) { /*Initially same address.*/
              specials = (struct templ *)malloc(sizeof(specials_table[0]) *
                   (specials_index + 1));
              memcpy((void *)specials, (void *)specials_table,
                   sizeof(specials_table)); /*Copy initial values*/
         }
         else { /*Pointer specials already initialized - simply realloc()*/
              specials = (struct templ *)realloc((void *)specials,
                   sizeof(specials[0]) * (specials_index + 1));
         }

         /*After allocation, reset p to point to last element again.*/
         p = specials + specials_index - NUMBER_OF_ELEMENTS;

         if (specials == NULL) {
              fprintf(stderr, MSGSTR(OUTOFMEMORY, "indent: Out of memory.\n"));
              exit(1);
         }
    }

                                                /* Make space for the key */
    if ((p->rwd = (char *)malloc(strlen(key))) == 0) { /*D46827 - Add MSGSTR*/
        fprintf(stderr, MSGSTR(OUTOFMEMORY, "indent: Out of memory.\n"));
        exit(1);
    }
    strcpy (p->rwd, key);

    p->rwcode = val;
    p[1].rwd = 0;
    p[1].rwcode = 0;
    return;
}


/* match trigraph sequences */
int has_trigraph(s)
char *s;
{
    int         c;
    if ( strlen (s) < 4 || *s != '?' || s[1] != '?' )
        return (0);
    c = s[2];
    return (
                ( c == '=' ? '#' :
                 ( c == '/' ? '\\' :
                  ( c == '\'' ? '^' :
                   ( c == '(' ? '[' :
                    ( c == ')' ? ']' :
                     ( c == '!' ? '|' :
                      ( c == '<' ? '{' :
                       ( c == '>' ? '}' :
                        ( c == '-' ? '~' : 0
                )))))))))                                 );
}       /* has_trigraph */
/*
 * return whether or not the [MB] character pointed to is in the alnum class.
 */
int
has_iswalnum (p)
  char          *p;
{
wchar_t         wc;

    if ((indent_mb_cur_max <= 1) || (isascii(*p))) /* 8-bit */
        return (chartype[*p & 0177] == alphanum);

    if (mbtowc (&wc, p, indent_mb_cur_max) < 1) /* error => false */
        return (0);

    return (iswalnum(wc));                      /* what does system say */
}
