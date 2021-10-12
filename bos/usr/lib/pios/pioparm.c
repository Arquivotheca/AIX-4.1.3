static char sccsid[] = "@(#)37  1.13.1.6  src/bos/usr/lib/pios/pioparm.c, cmdpios, bos41B, 9504A 1/17/95 07:50:33";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: piocmdout, piogetstr, pioparm, _branchto, lerrorexit
 *
 * ORIGINS: 3, 9, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1979 Regents of the University of California
 *
 */

/* Note: the pioparm and _branchto functions are based on the tparm */
/* and _branchto functions in tparm.c from the Curses Library       */

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <curses.h>
#include <string.h>
#include <term.h>
#include <fcntl.h>
#include <errno.h>
#include "pioformat.h"

extern char             *regcmp (char *, ...);	/* should be in regex.h */
extern char             *regex (char *, char *, ...); /*should be in regex.h */
extern char		*__loc1;		/* should be in regex.h */

int			piocmdout (char *name, FILE *fileptr, int ptval,
				   error_info_t *err_infop);
int			piogetstr (char *name, char *strptr, int siz,
				   error_info_t *err_infop);
int			pioparm (char *instring, FILE *fileptr,
				 char *getstrbuf, int getstrsiz,
				 int passthru, int id,
				 error_info_t *err_infop);
static void		lerrorexit (int err_type, int msgnum,
				    char *stringval1, char *stringval2,
				    int integerval);
static char	 	*_branchto (char *cp, char *stop, char to);


/*****************************************************************************
*  piocmdout                                                                 *
*****************************************************************************/
int
piocmdout(char *name, FILE *fileptr, int ptval, error_info_t *err_infop)
{
   int rc;
   char namesave[20];

   /* In case we have to put out an error message */
   (void) strcpy(namesave, errinfo.subrname);
   (void) strcpy(errinfo.subrname, "piocmdout");

   /* If the supplied argument for error info is not NULL, perform
      preliminary checking of the given attribute.   In case, the given
      attribute doesn't exist, store error info in the supplied argument,
      and return. */
   if (err_infop  &&  !get_attrtab_ent(name))
   {
      STORE_ERROR_INFO (ERR_NOATTR, MSG_BADATTR, name, NULL, 0, err_infop);
      return -1;
   }

   /* Call the routine that actually does the work */
   rc = pioparm(name, fileptr, NULL, 30000, ptval, 1, err_infop);

   /* Put back caller's name */
   (void) strcpy(errinfo.subrname, namesave);

   return(rc);
}

/*****************************************************************************
*  piogetstr                                                                 *
*****************************************************************************/
int 
piogetstr(char *name, char *strptr, int siz, error_info_t *err_infop)
{
   int rc;
   char namesave[20];
   struct attr_info  *attr_tab_p;
   register char		*rname;		/* real attribute name */

   /* The attribute name is usually a two character name.  However, to
      support limit strings, a new quirk has been added to this name.
      If the name begins with a '#', then it is assumed that the attribute's
      limit string is to be returned instead of its value string. */
   rname = *name == LIMITDENOTER ? name+1 : name;

   /*
    * Is this a test call? (i.e. strptr is NULL) or should we really
    * go through the work of getting the value?
    */
   if ( strptr == NULL)
       if (( attr_tab_p = get_attrtab_ent( rname)) == NULL)
	   return 0;
       else
	   return attr_tab_p->datatype;

   /* In case we have to put out an error message */
   (void) strcpy(namesave, errinfo.subrname);
   (void) strcpy(errinfo.subrname, "piogetstr");

   /* If the supplied argument for error info is not NULL, perform
      preliminary checking of the given attribute.   In case, the given
      attribute doesn't exist, store error info in the supplied argument,
      and return. */
   if (err_infop  &&  !get_attrtab_ent(rname))
   {
      STORE_ERROR_INFO (ERR_NOATTR, MSG_BADATTR, rname, NULL, 0, err_infop);
      return -1;
   }

   /* Call the routine that actually does the work */
   rc = pioparm(name, NULL, strptr, siz, 0, 0, err_infop);

   /* Put back caller's name */
   (void) strcpy(errinfo.subrname, namesave);

   return(rc);
}


/*****************************************************************************
*  lerrorexit                                                                *
*****************************************************************************/
static void
lerrorexit (int err_type, int msgnum, char *stringval1, char *stringval2,
	    int integerval)
{
   ERREXIT (err_type, msgnum, stringval1, stringval2, integerval);
}


/*****************************************************************************
*  pioparm                                                                   *
*****************************************************************************/

/*
* Routine to perform parameter substitution.
* instring is a string containing printf type escapes.
* The whole thing uses a stack.
* The following escapes are defined:
*
*    %%         Generates a % (percent) character
*    %d         Pops an integer value from the stack and converts it to
*               ASCII, without leading zeros.  Produces a field width
*               large enough to hold the ASCII numeric digits.  Similar to
*               %d with printf().
*    %[1-9]d    Pops an integer value from the stack and converts it to
*               ASCII.  The result is 1 to 9 characters, depending on the
*               digit specified before the 'd'.  If the value does not
*               fill the specified field width, it is padded on the left
*               with zeros.  If the value will not fit in the field,
*               excess high order digits are discarded.
*    %c         Pops an integer value from the stack and discards all but
*               the low order byte.
*    %h         Pops an integer value from the stack and discards all but
*               the two low order bytes.
*    %a         Similar to %h, except that the two bytes from the stack
*               are in "exchanged" order; i.e., low order byte, then high
*               order byte.
*    %Ixx or    Includes the string attribute whose name is "xx".  %I can
*    %I[xx,...] be used recursively; i.e., the included string may also
*               contain a %I.  If multiple, contiguous includes are to be
*               done, the attribute names can be separated by commas and
*               enclosed in brackets.  For example, %Icp%Icc%IeW can be
*               specified as %I[cp,cc,eW]
*    %Gxx       Gets the integer attribute whose name is "xx" and pushes
*               it onto the stack. If the attribute is a string instead of
*               an integer, the string is assumed to be an ASCII integer
*               and is converted to a binary integer using atoi() and
*               pushed onto the stack.
*    %'c'       Pushes character constant c onto the stack, where it
*               becomes the low order byte of an integer value.  The
*               high order bytes are set to zero.
*    %{nn}      Pushes integer constant nn onto the stack. The constant
*               is a decimal value and may be either positive or negative.
*    %Lxx       Push the length of constant or variable string "xx"
*               onto the stack.
*    %P[a-z]    Pops an integer value from the stack and stores it in
*               the specified internal  variable.  Example: %Pf
*               moves an integer value from the stack to variable f.
*    %Z[a-z]    Zeros the specified internal variable.
*               Example: %Zg stores a value of zero in variable g.
*    %g[a-z]    Pushes the value of the specified internal variable onto the
*               stack.  The value of the internal variable is not changed.
*               Example: %gb reads the integer value in variable b and pushes
*               it onto the stack.
*    %"sss"     Pushes a pointer to the string constant sss onto the stack.
*    %Sxx       Pushes a pointer to the current string value for attribute 
*               xx onto the stack.
*    %+ %- %* % / %m   The result is pushed onto the stack.
*                   %+ - add the first two values on the stack
*                   %- - subtract the first value popped off the stack
*                        from the second value popped off the stack
*                   %* - multiply the first two values on the stack
*                   %/ - divide the first value popped off the stack
*                        into the second value popped off the stack
*                   %m - (modulus) similar to %/, except that the
*                        remainder, instead of the quotient, is pushed onto
*                        the stack.
*    %= %> %< % !  Pushes 1 if true, or 0 if false, onto the stack
*                   %= - are the first two values on the stack equal?
*                   %> - is the second value popped off the stack greater
*                        than the first value popped off the stack?
*                   %< - is the second value popped off the stack less
*                        than the first value popped off the stack?
*                   %! - negation: non-zero value to zero; zero value to 1
*    %& %| %^ % ~  The result is pushed onto the stack.
*                   %& - AND the first two values on the stack
*                   %| - OR the first two values on the stack
*                   %^ - EXCLUSIVE OR the first two values onto the stack
*                   %~ - one's complement: inverts the value of each bit
*    %? expr %t  thenpart %e elsepart %;
*               The %e elsepart is optional.
*               else-if construct:
*                   %? c1 %t b1 %e c2 %t b2 %e c3 %t b3 %e b4 %;
*               where ci denote conditions, bi denote bodies.
*    %wx...%;   "while" loop.  Whenever %; is reached, the value of the internal
*               variable 'x' ('x' can be 'a' - 'z') is decremented by one. If
*               the result is greater than zero, execution is transferred to the
*               character following %wx.
*    %ix        Used only in prefix filter pipelines (first character of
*               attribute name is 'f'). Indicates that the main pipeline
*               following the prefix filter pipeline should be obtained from
*               the ix (x=0-9,a-z,A-Z,!) attribute instead of from the pipeline
*               specified by the -d flag arg (or its default).  If x is !,
*               no main pipeline will be used.
*    %p         indicates where to embed the prefix filter pipeline into the
*               main pipeline.  If not present, it is assumed to be at the
*               beginning of the main pipeline.  Ignored if the first character
*               of the attribute name is not 'i' (i.e., is not a main pipeline)
*    %z         indicates where to embed the pioout (device driver interface
*               routine) string into the main pipeline.  If not present, it is
*               assumed to be at the end of the main pipeline.  Ignored if the
*               first character of the attribute name is not 'i' (i.e., is not
*               a main pipeline).
*    %x         (piocmdout() subroutine call only)  Pass through from
*               input to output the number of bytes specified by the
*               passthru argument to the piocmdout() subroutine.
*    %o         Start using only original values from the data base
*               instead of values that may have been updated from the
*               command line (or during formatter processing).
*    %r         Return to using the values that were being used before %o.
*    %Cy        Push a value of 1 (true) onto the stack if flag y was
*               specified on the command line.  Otherwise, push a value
*               of 0 (false) onto the stack.
*    %Fxy or    Shorthand for "%?%Cy%t -x %I_y%;".  If flag 'y' was
*    %F[y...]   specified on the command line, generate "-x yarg", where
*               "yarg" is the argument specified for flag "y". If '!' is
*               specified for 'x', "-x " will not be generated.  If "yarg"
*               contains an unprotected single or double quote (i.e., not
*               immediately preceeded by an odd number of backslashes),
*               an error message will be issued and the print job terminated.
*               If multiple flags are to be specified using %Fxy, and each
*               flag's 'x' and 'y' are identical, a list of flag letters
*               can be specified in brackets.  For example, %Faa%Fbb%Fcc
*               could be specified as %F[abc].
*    %fxy or    Similar to %Fxy, except that no space is placed between
*    %f[...]    the flag name and the argument.
*    %Ux or     Indicates to the Print Job Manager that flag x (or each flag
*    %U[x...]   in the string "...") is used even though it is not referenced
*               by a pipeline (for example, it may be referenced only by a
*               printer command instead of by a filter in a pipeline).  This
*               will prevent the Print Job Manager from rejecting the flag
*               when it is specified on the command line.
*    %vxy or    Similar to %fxy and %f[...], but used only in pioout command
*    %v[...]    string to generate flags and arguments for override values
*               specified by piobe.
*    %Dxx       Download to the printer the contents of the file whose name is
*               specified by attribute "xx".  The print job must have read
*               access to the file.  The primary use of this operator is to
*               download fonts to the printer.
*    %`xx       Inserts the stdout output produced when the command string
*               specified by attribute "xx" is passed to a shell (ksh).
*    %`"..."    Inserts the stdout output produced when the command string
*               specified by string enclosed in double-quotes is passed to a
*               shell (ksh).
*    %#xx"...@..." Includes the string (extracted from the value string of
*               "xx" attribute) positioned at location denoted by an
*               '@' character that is embedded between two patterns matching
*               the regular expressions before and after this character.
*               Missing preceding or succeeding regular expressions denotes
*               that no pattern is to be matched against for that part.
*
* all other characters are ``self-inserting''.  %% gets % output.
*/

int		piocfapi;		/* kludge variable for silencing the
					   error messages for colon file API */

/************  Redefine ERREXIT and MALLOC as a kludge for colon file API
	       so that the control just returns back to the calling
	       program.  *************/
#undef ERREXIT
#define ERREXIT(err_type, msgnum, stringval1, stringval2, integerval) \
      { if (piocfapi) \
	   return -1; \
	errinfo.errtype = err_type; \
	strncpy(errinfo.string1, stringval1, sizeof(errinfo.string1) - 1); \
	strncpy(errinfo.string2, stringval2, sizeof(errinfo.string2) - 1); \
	errinfo.integer = integerval; \
	(void) piogenmsg(msgnum, TRUE); \
	(void) pioexit(PIOEXITBAD); }

#undef MALLOC
#define MALLOC(memptr, size) \
   {if ((memptr = malloc(size)) == NULL) { \
	if (piocfapi) \
	   return -1; \
	(void) piomsgout(getmsg(MF_PIOBE,1,MSG_MALLOC)); \
	(void) pioexit(PIOEXITBAD); \
    }}

#define push(i) 	do \
			{ \
			   if (top >= STACKSIZE) \
			   { \
			      if (piocfapi) \
				 return -1; \
			      lerrorexit (0, MSG_STACK_OVERFLOW, \
					   instring, cp - 1, cp - start); \
			   } \
			   else \
			   { \
			      stack[top].type = 1;  \
		 	      stack[top++].entry.val = i; \
			   } \
			} while (0)

/* Commented out so that a lot of existing colon files won't break
   because of an extra pop off the stack */
/*
#define pop()    	( \
			   top <= 0 ? \
			      (lerrorexit (0, MSG_STACK_UNDERFLOW, \
					   instring, cp - 1, cp - start), \
			       0) : \
			      stack[--top].entry.val \
			)
 */

/* If stack is already bottomed out, we just pop off the last element */
#define pop()    	( \
			   top <= 0 ? \
			      stack[0].entry.val : \
			      stack[--top].entry.val \
			)

#define PIOGETSTR 0
#define PIOCMDOUT 1

#define STACKSIZE 20
#define MAXLEVELS 100

#define REGPTRN_DELIMITER	'"'
#define REGPTRN_DIVIDER		'@'
#define MAXREGEXPLEN		10000

struct stackfmt {
    int type;
    union {
	int val;
	char *ptr;
    } entry;
};

unsigned char shellchar[128] = {    /* shell special characters */
0, 0, 0, 0, 0, 0, 0, 0,   1, 1, 0, 0, 0, 0, 0, 0,    /* 00 - 0f */
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    /* 10 - 1f */
1, 0, 1, 1, 1, 0, 1, 1,   1, 1, 1, 0, 0, 0, 0, 0,    /* 20 - 2f */
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 0, 1, 1,    /* 30 - 3f */
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    /* 40 - 4f */
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 1, 0, 0,    /* 50 - 5f */
1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    /* 60 - 6f */
0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 1, 0, 1, 0,    /* 70 - 7f */
};

static int nest_level = 0;
static int piomode_saved;
static int bytes_to_passthru;
static int vars[26];
static char attr_nest[MAXLEVELS+1][2];
static char *strbuf = NULL;

char piopipeline = '\0';                /* ID of main pipeline that prefix
					   filter pipeline wants used */
int piofilterloc = 0;                   /* offset into attribute value string
				           where %p is found */
int piodevdriloc = 0;                   /* offset into attribute value string
                                           where %z is found */
short usedbysomeone = USEDBYSOMEONE;    /* use of variable allows caller to
					   override whether flag is turned on */
int force_recalc = 0;                   /* force recalculation of VAR_INTs
					   (set by piogetopt for refresh) */
static int refsflagarg;                 /* attribute references flag argument */
static struct attr_info *attr_tab_p_lv0;/* ptr to attr tab entry for level 0 */
char *outp;
struct attr_info *attr_pthru_p = NULL;  /* initialized by pioformat.c */
struct int_info *int_pthru_p = NULL;    /* initialized by pioformat.c */

extern char *mem_start;



int
pioparm(char *instring, FILE *fileptr, char *getstrbuf, int getstrsiz,
	int passthru, int id, error_info_t *future)
{
    register char *cp;
    register int c;
    register int op;

    char strname[3];
    char *start, *stop;
    int tmpint, cnt, rc, num, brackets;
    int in_strlen;
    int altflag   = FALSE;
    int alttest   = FALSE;
    int dont_translate = FALSE;
    int sign	  = 0;
    int strlength = 0;
    int totcount  = 0;
    int top = 0;

    struct stackfmt stack[STACKSIZE];

    int val1, val2, ln;
    char *p1, *p2, *tp;
    char *xp;
    char *rbracket_p;
    char *lbracket_p;
    static char *cmdoutbuf;

    char *begwhile[10], *endwhile[10];
    int whilelevel = 0;
    int varindx[10];
    int getlimitstr;		/* flag to denote if limit string is fetched */
    short sav_usedbysomeone;	/* flag to save entry value of usedbysomeone */

    struct attr_info  *attr_tab_p;
    struct attr_info  *attr_tab_p2;

    struct odm *odm_tab_p;

    /* The attribute name is usually a two character name.  However, to
       support limit strings, a new quirk has been added to this name.
       If the name begins with a '#', then it is assumed that the attribute's
       limit string is to be returned instead of its value string.
       Also, ensure that flags of referenced attributes are not set to
       USEDBYSOMEONE during limit string evaluation. */
    getlimitstr = !nest_level  &&  instring  &&  *instring == LIMITDENOTER ?
		  ++instring, sav_usedbysomeone = usedbysomeone,
		  usedbysomeone = 0, TRUE : FALSE;

    if (instring == NULL || *instring == '\0'
    || (attr_tab_p = get_attrtab_ent(instring)) == NULL)
	ERREXIT(0, MSG_BADATTR, instring, NULL, 0);

    nest_level++;
    strncpy(&attr_nest[nest_level][0], instring, 2);
    refsflagarg = FALSE; /* want to know if attr references flag argument */

    if (nest_level == 1)
    {
	if (id == PIOCMDOUT)
	{
	    MALLOC(strbuf, getstrsiz);
	    cmdoutbuf = strbuf;
	    if (passthru > 0)
	    {
		/* caller specified a passthru value, so put the passthru */
		/* byte count in the pseudo data base variable "@2" which */
		/* can be referenced by the comand being output           */
		if (attr_pthru_p != NULL)    /* safety valve */
		    for (cnt = 0; cnt < PIO_NUM_MODES; cnt++)
			int_pthru_p->value[cnt] = passthru;
	    }
	}
	else
	    strbuf = getstrbuf;
	outp = strbuf;
	top = 0;  /* make sure stack is clear */
	piomode_saved = piomode;  /* will put it back when we're through */
	bytes_to_passthru = passthru;
	attr_tab_p_lv0 = attr_tab_p;
    }

    attr_tab_p->flags |= usedbysomeone;

    /* get pointer and length for string to be processed */
    if (getlimitstr)
    {
	odm_tab_p = get_odmtab_ent(instring);
	cp = GET_ATTR_LIMSTR(odm_tab_p);
	in_strlen = odm_tab_p->limlength;
    }
    else if (attr_tab_p->datatype == VAR_STR)
    {
	cp        = (attr_tab_p->ptypes.sinfo + piomode)->ptr;
	in_strlen = (attr_tab_p->ptypes.sinfo + piomode)->len;
	/*if (attr_tab_p->flags & ONCMDLINE && piomode != PIO_DBASE_MODE) D38946 */
	dont_translate = TRUE;
    }
    else if (attr_tab_p->datatype == CONST_STR)
    {
	cp        = attr_tab_p->ptypes.sinfo->ptr;
	in_strlen = attr_tab_p->ptypes.sinfo->len;
    }
    else  /* VAR_INT (%G is getting string for recalculation of VAR_INT) */
    {
	odm_tab_p = get_odmtab_ent(instring);  /* %G already validated it */
	cp = (char *) GET_OFFSET(odm_tab_p->str);
	in_strlen = odm_tab_p->length;
    }


    /* loop to process each input character */
    start = cp;
    stop = start + in_strlen;
    for (c = *cp++; cp <= stop; c = *cp++)
    {
	if (c != '%' || dont_translate) {
	    *outp++ = c;
	    continue;
	}
	op = top > 0 ? stack[top - 1].entry.val : 0;

	/* process %nd, where n = 1 - 9 */
	if ((c = *cp++) <= '9' && c >= '1') {
	    (void) pop ();
	    if (*cp != 'd')
		ERREXIT(0, MSG_ESC_ND, instring, cp, cp - start);
	    cp++;
	    tmpint = 1;
	    if (op >= 0)
		cnt = 0;
	    else {
		if (c == '1') {
		    *outp++ = '-';
		    continue;
		}
		cnt = 1;
	    }
	    for (; cnt < (c - '0'); cnt++)
		tmpint *= 10;
	    if (abs(op) >= tmpint)
		op %= tmpint;
	    if (op < 0)
		cnt--;
	    outp += sprintf(outp, "%*.*d", cnt, cnt, op);
	    continue;
	}

	switch (c) {

	/* PRINTING CASES */
	case 'd':
	    outp += sprintf(outp, "%d", op);
	    (void) pop ();
	    break;

	case 'c':
	    *outp++ = op;
	    (void) pop();
	    break;

	case 'H':
	case 'h':
	    *outp++ = op >> 8;
	    *outp++ = op;
	    (void) pop();
	    break;

	case 'E':
	case 'a':
	    *outp++ = op;
	    *outp++ = op >> 8;
	    (void) pop();
	    break;

	/* push (pointer to) attribute string onto stack */
	case 'S':
	    attr_tab_p2 = get_attrtab_ent(cp); /* get ptr to attr table entry*/
	    if (attr_tab_p2 == NULL)
		ERREXIT(0, MSG_BADATTR2, instring, cp - 1, cp - start);
	    attr_tab_p2->flags |= usedbysomeone;
	    xp = outp;  /* save it */
	    if ((rc = pioparm(cp, NULL, strbuf, getstrsiz, 0, id, future)) < 0)
		return(rc);
	    ln = outp - xp;
	    outp = xp;  /* put it back */
	    MALLOC(tp, ln + 2);
	    stack[top].type = 3;
	    stack[top++].entry.ptr = tp;
	    *tp = (unsigned char) ln>>8;
	    *(tp + 1) = (unsigned char) ln;
	    for (tp += 2, ln--; ln >= 0; ln--)
		*(tp + ln) = *(xp + ln);

	    /* housekeeping */
	    if (!strncmp(cp, FLAGCHAR, 1))
		refsflagarg = TRUE;  /* flag value is referenced */
            cp += 2;
	    break;

	case 'C':
	    {
		char strname[3];
		struct attr_info *attr_tab_p2;

		(void) strncpy(strname, FLAGCHAR, 1);
		(void) strncpy(strname+1, cp, 1);
		attr_tab_p2 = get_attrtab_ent(strname);

		if (attr_tab_p2 != NULL)
		{
		    attr_tab_p2->flags |= usedbysomeone;
		    if (attr_tab_p2->flags & ONCMDLINE)
			push(TRUE);
		    else
			push(FALSE);
		}
		else
		    push(FALSE);

		cp ++;
	    }
	    break;

	case 'o':
	    piomode = 0;    /* start using only data base values */
			    /* (without updates from command line) */
	    break;

	case 'D':
	    {
	    char wkstr[10], *pass_p;
            FILE *fp;
	    int ch;

	    /* Validate the attribute name */
	    attr_tab_p2 = get_attrtab_ent(cp);
	    if (attr_tab_p2 == NULL)
		ERREXIT(0, MSG_BADATTR2, instring, cp - 1, cp - start);
	    attr_tab_p2->flags |= usedbysomeone;

	    /* Get the attribute string (with variables resolved) */
	    xp = outp;   /* save it */
	    if ((rc = pioparm(cp,NULL,strbuf,getstrsiz,0,id, future)) < 0)
		return(rc);

	    /* Perform downloading only if the resolved string is not null */
	    if (outp != xp)
	    {
	       *outp = '\0';
	       outp = xp;   /* put it back */

	       if ((fp = fopen(outp, "r")) == NULL)
		   {
		   (void) sprintf(wkstr,"%d",errno);
		   ERREXIT(0, MSG_OPEN_FONTS2, cp, outp, errno);
		   }

	       /* clean out the buffer */
	       for (pass_p = strbuf; pass_p < outp; pass_p++)
	          pioputchar(*pass_p);
	       outp = strbuf;

	       /* output the file */
	       while((ch = getc(fp)) != EOF)
	       pioputchar((char) ch);
	    }				/* if outp != xp */

	    /* housekeeping */
	    if (!strncmp(cp, FLAGCHAR, 1))
		refsflagarg = TRUE;  /* flag value is referenced */
	    cp += 2;
	    }
	    break;


	/******************************************************************
		%`xx | %`"..." Operator	   -   For Command Execution
		----------------------------------------------------
							03/20/92
	    The %` operator supports two syntaxes: one followed by an
	    attribute name (ex. %`xx), and the other followed by a
	    quoted string (ex. %`"banner hello").

	    In the former, value string of the specified attribute is
	    considered to be a shell command and passed to a sub shell.
	    In the latter, the quoted string is passed as a command to
	    a sub shell.

	    In both cases, the command string is evaluated for any '%'
	    operators before being passed to a sub shell.

	    Examples:
		a.	Given that
			e8	=	echo %Idd|wc -c|tr -d '\012'
			e9	=	%`e8

			the value of e9 results in	21

		b.	Given that
			e9	=	%`"echo %Idf|tr 'r' '\\"'|tr -d '\012'"

			the value of e9 results in	/us"/lib/lpd/pio/fmt"s

	   Note:	Any double-quotes within the quoted string should be
			back-quoted so that they are not misconstrued as
			delimiters for the quoted string.

	 ******************************************************************/
	case '`':
	    {
	       register int		attrnm_spcd_flag = TRUE;
					/* flag to denote if attr name is
					   specified */
	       register char		*tmpstr;
					/* temp char ptr */
	       register char		*tmp1str;
					/* temp1 char ptr */
	       size_t			cmdstrlen;
					/* length of specified command str */
	       register int		enddelimit_found = FALSE;
					/* flag - end delimit found */
	       register int		noof_bkslshs = 0;
					/* no of succ. back slashes */
	       struct attr_info		sav_attr_tab_p;
					/* attribute table record to
					   be saved for dummy attr */
               FILE 			*cmdp_fp;
					/* file pointer for command pipe */
	       int			ch;
					/* char read from pipe */

	       /* Check if a command string is specified or if an attribute
		  name is specified.  If the former is true, calculate the
		  end of string (bypassing back-quoted double-quotes, if any).
		*/
	       if (*cp == REGPTRN_DELIMITER)
	       {
		  attrnm_spcd_flag = FALSE;
                  for (tmpstr = cp + 1; *tmpstr  &&  !enddelimit_found;
		       tmpstr++)
	          {
	             switch (*tmpstr)
	             {
	                case '\\':
	     	           noof_bkslshs++;
	     	           break;

	                case REGPTRN_DELIMITER:
	     	           if (!(noof_bkslshs % 2))
	     	              enddelimit_found = TRUE;
		           noof_bkslshs = 0;
	     	           break;

	                default:
	     	           noof_bkslshs = 0;
	     	           break;
	             }
	          }

		  /* If string is not delimited properly, display an error
		     message, and exit. */
	          if (!enddelimit_found)
	          {
	             ERREXIT (0, MSG_MISS_PTRNDELIMIT, instring,
	     	              cp - 1, cp - 2 - start);
	          }

		  /* If length of the specified string is zero, skip the
		     subsequent code and break out of this case body. */
		  if (!(cmdstrlen = tmpstr - 1 - (cp + 1)))
		  {
		     cp += 2;		/* for two successive delimiters */
		     break;		/* break out of case '`' */
		  }

		  /* Check if the dummy attribute exists in the data base.
		     If not, display an error message (since we are dealing
		     with new ddi files - version 00001) and exit. */
	          if (!(attr_tab_p2 = get_attrtab_ent (DUMMY_ATTR)))
	          {
		      ERREXIT (0, MSG_BADATTR2, instring, cp - 1, cp - start);
	          }

		  /* Save the attribute table record so that the
		     original values can be restored even in cases
		     of recursive invocations. */
		  sav_attr_tab_p = *attr_tab_p2;

		  /* Initialize the dummy attribute values */
		  attr_tab_p2->datatype = CONST_STR;
		  attr_tab_p2->flags = 0;
	          MALLOC (attr_tab_p2->ptypes.sinfo->ptr, cmdstrlen + 1);
		  (void) strncpy (attr_tab_p2->ptypes.sinfo->ptr, cp + 1,
				  cmdstrlen);
		  *(attr_tab_p2->ptypes.sinfo->ptr + cmdstrlen) = 0;

		  /* Strip off any extra backslashes that were placed in
		     the specified string to back-quote characters like '"'
		     so as not to upset our search of delimiters before. */
		  for (noof_bkslshs = 0, tmp1str = tmpstr =
		       attr_tab_p2->ptypes.sinfo->ptr; *tmpstr; tmpstr++)
		     switch (*tmpstr)
		     {
	                case '\\':
			   *tmp1str++ = *tmpstr;
	     	           noof_bkslshs++;
	     	           break;

	                case REGPTRN_DELIMITER:
			   if (noof_bkslshs % 2)
			      *--tmp1str = *tmpstr,
			      tmp1str++;
			   else
			      *tmp1str++ = *tmpstr;
			   noof_bkslshs = 0;
			   break;

	                default:
			   *tmp1str++ = *tmpstr;
	     	           noof_bkslshs = 0;
			   break;
		     }
		  *tmp1str = 0;
		  attr_tab_p2->ptypes.sinfo->len =
		     strlen (attr_tab_p2->ptypes.sinfo->ptr);
	       }		/* if - *cp == '"' */
	       else
	       {
	          /* Validate the attribute name */
	          if (!(attr_tab_p2 = get_attrtab_ent (cp)))
	          {
		     ERREXIT (0, MSG_BADATTR2, instring, cp - 1, cp - start);
	          }
	          attr_tab_p2->flags |= usedbysomeone;
	       }		/* if - *cp != '"' */

	       /* Get the attribute string (with variables resolved) */
	       xp = outp;   /* save it */
	       if ((rc = pioparm (attrnm_spcd_flag ? cp : DUMMY_ATTR, NULL,
				  strbuf, getstrsiz, 0, id, future)) < 0)
		  return(rc);

	       /* Execute the command only if the resolved string is not null */
	       if (outp != xp)
	       {
	          *outp = 0;
	          outp = xp;   /* put it back */

	          /* Pass the string to a shell, capturing the stdout through
		     a pipe */
		  if (!(cmdp_fp = popen (outp, "r")))
	          {
		     ERREXIT (0, MSG_POPEN_ERR, instring, outp, errno);
	          }
		  while ((ch = getc (cmdp_fp)) != EOF &&
			 outp < strbuf + getstrsiz - 1)
		     *outp++ = (char) ch;
		  (void) pclose (cmdp_fp);
	       }				/* if outp != xp */

	       /* If dummy attribute was used, restore its values. */
	       if (!attrnm_spcd_flag)
	       {
		  free (attr_tab_p2->ptypes.sinfo->ptr);
		  *attr_tab_p2 = sav_attr_tab_p;
	       }

	       /* housekeeping */
	       if (attrnm_spcd_flag  &&  !strncmp(cp, FLAGCHAR, 1))
		   refsflagarg = TRUE;  /* flag value is referenced */
	       cp += attrnm_spcd_flag ? 2 : cmdstrlen + 2;
	    }
	    break;


	case 'v':
	/* Like %fxy, except that checks OVERRIDE flag (used */
	/* for "@" variables) instead of for ONCMDLINE flag. */
	/* Used for override flags passed to pioout by piobe */
		alttest = TRUE;  /* note that alternate test is to be used */
		/*(fall through)*/
	case 'f':
	/* like %Fxy below, except no space between flag letter and argument */
		altflag = TRUE;  /* note that alternate op code is being used */
		/*(fall through)*/
	case 'F':
	/* %Fxy - if arg specified on command line, generate "-x yarg" */
	    {
		char wkbuf[BUFSIZ], *saved_outp, *c2, *c3;
		char flag1, flag2;
		int retval, len, i, needquotes;

		if (alttest)
		    (void) strncpy(strname, EXTRA_ATTRNAME, 1);
		else
		    (void) strncpy(strname, FLAGCHAR, 1);
		if (*cp == '[')
		{
		    cp++;
		    if ((rbracket_p = strchr(cp, ']')) == NULL)
			ERREXIT(0, MSG_CLOSEBRACK,
			      instring, cp - 2, cp - 1 - start)  /* no ; */
		    else if ((lbracket_p = strchr(cp, '[')) != NULL)
			if (lbracket_p - cp < rbracket_p - cp)
			    ERREXIT(0, MSG_CLOSEBRACK,
				  instring, cp - 2, cp - 1 - start);

		    num = rbracket_p - cp;
		    if (num < 1)
			ERREXIT(0, MSG_NOFLAGLETTER,
			      instring, cp - 2, cp - 1 - start);
		    flag1 = flag2 = *cp++;
		}
		else
		{
		    flag1 = *cp;
		    flag2 = *(cp+1);
		    cp++;
		    num = 1;
		}

		for (cnt = 1; cnt <= num; cnt++)
		{
		    strname[1] = flag2;
		    attr_tab_p2 = get_attrtab_ent(strname);

		    if (attr_tab_p2 != NULL)
		    {
			attr_tab_p2->flags |= usedbysomeone;
			if ((!alttest && attr_tab_p2->flags & ONCMDLINE) ||
			    ( alttest && attr_tab_p2->flags & OVERRIDE ))
			{
			    if (flag1 != '!') {
				if (outp > strbuf && *(outp-1) != ' ')
				    *outp++ = ' ';
				*outp++ = '-';
				*outp++ = flag1;
			    }
			    if (!altflag) /* if op code is 'F' instead of 'f' */
				*outp++ = ' ';
			    saved_outp = outp;
			    retval = pioparm(strname,fileptr,
				strbuf,getstrsiz,passthru,id, future);
			    if (retval < 0)
				return(retval);
			    needquotes = FALSE;
			    if (saved_outp == outp)  /* if null arg string */
				needquotes = TRUE;
			    else
			    {
				/* scan the arg looking for blanks or quotes */
				for (c2 = saved_outp; c2 < outp; c2++)
				{
				    if (*c2 == '"' || *c2 == '\'')
				    {
					i = 0;
					for (c3 = c2 - 1; c3 >= strbuf; c3--)
					    if (*c3 == '\\')
						i++;
					    else
						break;
					if (!(i & 1))  /* if even number */
					    ERREXIT(0,MSG_BADQUOTE,strname+1,
						saved_outp,0)
				    }
				    /* if blank or shell special character */
				    if (*c2 < 0x80 && shellchar[(int)*c2])
					needquotes = TRUE;
				}
			    }
			    if (needquotes)
			    {
				len = outp - saved_outp;
				strncpy(wkbuf, saved_outp, len);
				outp = saved_outp;
				if (len == 0 && altflag) /* if 'v' or 'f' op */
				  *outp++ = ' ';
				*outp++ = '\'';
				c2 = wkbuf;
				for (i = 0; i < len; i++)
				    *outp++ = *c2++;
				*outp++ = '\'';
			    }/*if*/
			}/*if*/
		    }/*if*/
		    flag1 = flag2 = *cp++;
		}/*for*/
	    }
	    break;

	case 'G':
	    /* get pointer to entry in attribute table */
	    attr_tab_p2 = get_attrtab_ent(cp);
	    if (attr_tab_p2 == NULL)
		ERREXIT(0, MSG_BADATTR2, instring, cp - 1, cp - start);
	    attr_tab_p2->flags |= usedbysomeone;

	    /* process according to data type */
	    if (attr_tab_p2->datatype == VAR_INT &&
		(attr_tab_p2->flags & ONCMDLINE || !force_recalc ||
		(force_recalc && !(attr_tab_p2->flags & REFSFLAGARG))))
		push(attr_tab_p2->ptypes.ivals->value[piomode]);
	    else
	    {   /* CONST_STR or VAR_STR or forced recalculation of VAR_INT */
		size_t		avlen;

		if (force_recalc)
		{   /* get ptr to original string from data base */
		    odm_tab_p = get_odmtab_ent(cp);
		    xp = (char *) GET_OFFSET(odm_tab_p->str);
		    avlen = odm_tab_p->length;
		}
		else if (attr_tab_p2->datatype == CONST_STR)
		{
		    xp    = attr_tab_p2->ptypes.sinfo->ptr;
		    avlen = attr_tab_p2->ptypes.sinfo->len;
		}
		else
		{
		    xp    = (attr_tab_p2->ptypes.sinfo + piomode)->ptr;
		    avlen = (attr_tab_p2->ptypes.sinfo + piomode)->len;
		}

		/* see if the string value is a boolean */
		if (!avlen)
		    push(0);
		else if (!strncmp(xp, YES_STRING, avlen))
		    push(1);
		else if (!strncmp(xp, NO_STRING, avlen))
		    push(0);
		else
		{
		    /* not a boolean, so make sure any variables are resolved */
		    xp = outp;   /* save it */
		    if ((rc = pioparm(cp,NULL,strbuf,getstrsiz,0,id, future))
			< 0)
			return(rc);
		    *outp = '\0';
		    push(atoi(xp));
		    outp = xp;   /* put it back */
		}
	    }

	    /* housekeeping */
	    if (!strncmp(cp, FLAGCHAR, 1))
		refsflagarg = TRUE;  /* flag value is referenced */
	    cp += 2;
	    break;

	case 'i':
	    piopipeline = *cp++;
	    break;

	case 'I':
	    xp = cp;  /* save for error msg */
	    if (*cp == '[')
	    {
		cp++;
                brackets = TRUE;
		if ((rbracket_p = strchr(cp, ']')) == NULL)
		    ERREXIT(0, MSG_CLOSEBRACK,
			  instring, cp - 2, cp - 1 - start)  /* no ; */
		else if ((lbracket_p = strchr(cp, '[')) != NULL)
		    if (lbracket_p - cp < rbracket_p - cp)
			ERREXIT(0, MSG_CLOSEBRACK,
			      instring, cp - 2, cp - 1 - start);

		num = ((rbracket_p + 1) - cp) / 3;  /* number of attr names */
		if (num < 1)
		    ERREXIT(0, MSG_NOATTRNAME, instring, cp-2, cp - 1 - start);
	    }
	    else
            {
		num = 1;
                brackets = FALSE;
            }

	    for (cnt = 1; cnt <= num; cnt++)
	    {
		/* get pointer to entry in attribute table */
		attr_tab_p2 = get_attrtab_ent(cp);
		if (attr_tab_p2 == NULL)
		    ERREXIT(0, MSG_BADATTR2, instring, xp - 1, cp - start);
		attr_tab_p2->flags |= usedbysomeone;

		/* process according to data type */
		if (attr_tab_p2->datatype == VAR_INT && !force_recalc)
		    outp += sprintf(outp, "%d",
			  attr_tab_p2->ptypes.ivals->value[piomode]);
		else
		{   /* CONST_STR or VAR_STR or forced recalculation of VAR_INT*/

		    /* go get it */
		    rc = pioparm(cp,fileptr,strbuf,getstrsiz,passthru,id,
				 future);
		    if (rc < 0)
			return(rc);
		}

		/* housekeeping */
		if (!strncmp(cp, FLAGCHAR, 1))
		    refsflagarg = TRUE;  /* flag value is referenced */

		cp += 3;
	    }
	    if (!brackets)
		cp--;
	    break;

	case 'L':
	    /* get pointer to entry in attribute table */
	    attr_tab_p2 = get_attrtab_ent(cp);
	    if (attr_tab_p2 == NULL)
		ERREXIT(0, MSG_BADATTR2, instring, cp - 1, cp - start);
	    attr_tab_p2->flags |= usedbysomeone;

	    /* if current attribute, use length of string constructed so far */
	    if (!strncmp(cp, instring, 2))
		push(outp - strbuf);
	    else
	    {
		/* process according to data type */
		switch (attr_tab_p2->datatype)
		{
		    case CONST_STR:
			push(attr_tab_p2->ptypes.sinfo->len);
			break;

		    case VAR_STR:
			push((attr_tab_p2->ptypes.sinfo + piomode)->len);
			break;

		    default:
			ERREXIT(0, MSG_NOTSTRING, instring, cp - 1, cp - start);
		}
	    }

	    /* housekeeping */
	    if (!strncmp(cp, FLAGCHAR, 1))
		refsflagarg = TRUE;  /* flag value is referenced */
	    cp += 2;
	    break;

	case 'U':
	    (void) strncpy(strname, FLAGCHAR, 1);
	    if (*cp == '[')
	    {
		cp++;
                brackets = TRUE;
		if ((rbracket_p = strchr(cp, ']')) == NULL)
		    ERREXIT(0, MSG_CLOSEBRACK,
			  instring, cp - 2, cp - 1 - start)  /* no ; */
		else if ((lbracket_p = strchr(cp, '[')) != NULL)
		    if (lbracket_p - cp < rbracket_p - cp)
			ERREXIT(0, MSG_CLOSEBRACK,
			      instring, cp - 2, cp - 1 - start);

		num = rbracket_p - cp;
		if (num < 1)
		    ERREXIT(0, MSG_NOFLAGLETTER, instring, cp-2, cp-1-start);
	    }
	    else
            {
		num = 1;
                brackets = FALSE;
            }

	    for (cnt = 1; cnt <= num; cnt++, cp++)
	    {
		strname[1] = *cp;
		attr_tab_p2 = get_attrtab_ent(strname);
		if (attr_tab_p2 != NULL)
		    attr_tab_p2->flags |= usedbysomeone;
	    }
	    if (brackets)
		cp++;
	    break;

	case 'R':
	case 'r':
		piomode = piomode_saved; /* return to using data base values */
					 /* that were being used before %o   */
	    break;

	case 'X':
	case 'x':
	    {
		int ch;
		char *pass_p;

		/* clean out the buffer */
		for (pass_p = strbuf; pass_p < outp; pass_p++)
		   pioputchar(*pass_p);
		outp = strbuf;

		/* pass on through the specified number of bytes */
		for(; bytes_to_passthru; bytes_to_passthru--)
		    if ((ch = piogetc(fileptr)) == EOF)
		    {
			bytes_to_passthru = 0;
			break;
		    }
		    else
			pioputchar((char) ch);

		/* zero the value provided to command strings */
		if (attr_pthru_p != NULL)    /* safety valve */
		    for (cnt = 0; cnt < PIO_NUM_MODES; cnt++)
			int_pthru_p->value[cnt] = 0;
	    }
	    break;

	case '%':
	    *outp++ = c;
	    break;

        /* %p: save current offset into constructed string (used by caller) */
        case 'p':
            piofilterloc = outp - strbuf;
            break;

        /* %z: save current offset into constructed string (used by caller) */
        case 'z':
            piodevdriloc = outp - strbuf;
            break;

	/* %Pi: pop from stack into variable i (a-z) */
	case 'P':
	    vars[*cp++ - 'a'] = pop();
	    break;

	/* %Zi: store zero value in variable i (a-z) */
	case 'Z':
	    vars[*cp++ - 'a'] = 0;
	    break;

	/* %gi: push variable i (a-z) */
	case 'g':
	    push(vars[*cp++ - 'a']);
	    break;
	
	/* %'c' : character constant */
	case '\'':
	    push(*cp++);
	    if (*cp++ != '\'')
		ERREXIT(0, MSG_CLOSEQUOTE, instring, NULL, cp - 1 - start);
	    break;
	
	/* %"...": push (pointer to) string constant onto stack */
	case '"':
	    if ((xp = strchr(cp, '"')) == NULL)
		ERREXIT(0, MSG_CLOSEQUOTE2, instring, cp - 1, cp - 1 - start);
	    ln = xp - cp;
	    MALLOC(tp, ln + 2);
	    stack[top].type = 2;
	    stack[top++].entry.ptr = tp;
	    *tp = (unsigned char) ln>>8;
	    *(tp + 1) = (unsigned char) ln;
	    for (tp += 2, ln--; ln >= 0; ln--)
		*(tp + ln) = *(cp + ln);
            cp = xp + 1;
	    break;

	/* %{nn} : integer constant.  */
	case '{':
	    xp = cp;  /* save for error msg */
	    op = 0; sign = 1;
	    if (*cp == '-') {
		sign = -1;
		cp++;
	    } else if (*cp == '+')
		cp++;
	    while ((c = *cp++) >= '0' && c <= '9') {
		op = 10*op + c - '0';
	    }
	    if (c != '}')
		ERREXIT(0, MSG_CLOSEBRACE, instring, NULL, xp - 1 - start);
	    push(sign * op);
	    break;
	
	/* binary operators */
	case '+': c=pop(); op=pop(); push(op + c); break;
	case '-': c=pop(); op=pop(); push(op - c); break;
	case '*': c=pop(); op=pop(); push(op * c); break;
	case '/': c=pop(); op=pop();
		  if (!c)
		      ERREXIT(0, MSG_ZERODIVIDE, instring, NULL, cp-2-start);
		  push(op / c); break;
	case 'm': c=pop(); op=pop(); push(op % c); break; /* %m: mod */
	case '&': c=pop(); op=pop(); push(op & c); break;
	case '|': c=pop(); op=pop(); push(op | c); break;
	case '^': c=pop(); op=pop(); push(op ^ c); break;
	case '>': c=pop(); op=pop(); push(op > c); break;
	case '<': c=pop(); op=pop(); push(op < c); break;

	/* = operator */
	case '=':
	    val1 = stack[top - 1].type;
	    val2 = stack[top - 2].type;
	    if (val1 == 1 && val2 == 1)
	    {
		c=pop(); op=pop(); push(op == c);  /* comparing integers */
	    }
	    else if (val1 > 1 && val2 > 1)
	    {
		p1 = stack[top - 1].entry.ptr;         /* comparing strings */
		p2 = stack[top - 2].entry.ptr;
		val1 = (*p1 << 8) + *(p1 + 1);     /* length of string 1 */
		val2 = (*p2 << 8) + *(p2 + 1);     /* length of string 2 */
		if (val1 == val2
		      && !(rc = memcmp(p1 + 2, p2 + 2, val1))) /* strings = */
		    rc = TRUE;
		else
		    rc = FALSE;
		top -= 2;     /* pop the two strings off the stack */
		push(rc);     /* push the result onto the stack */
		free(p1);
		free(p2);
	    }
	    else
		ERREXIT(0, MSG_BADEQUAL, instring, cp - 1, cp - 2 - start);
	    break;

	/* Unary operators. */
	case '!': stack[top - 1].entry.val = !stack[top - 1].entry.val; break;
	case '~': stack[top - 1].entry.val = ~stack[top - 1].entry.val; break;
	/* Sorry, no unary minus, because minus is binary. */

	/* Beginning of while loop */
	case 'w':
	    xp = cp;  /* save for error msg */
	    if (++whilelevel > 9)
		ERREXIT(0, MSG_WHILENEST, instring, NULL, xp - 2 - start);
	    if (*cp < 'a' || *cp > 'z')
		ERREXIT(0, MSG_WHILEVAR, instring, NULL, xp - start);
	    varindx[whilelevel] = *cp++ - 'a';
	    begwhile[whilelevel] = cp;
	    if ((endwhile[whilelevel] = _branchto(cp, stop, ';')) == NULL)
		ERREXIT(0, MSG_NOENDING1, instring, "w", xp - 2 - start);
	    break;

	/*
	 * If-then-else.  Implemented by a low level hack of
	 * skipping forward until the match is found, counting
	 * nested if-then-elses and whiles.
	 */
	case '?':    /* IF - just a marker */
	    break;

	case 't':    /* THEN - branch if false */
	    if (!pop()) {
		xp = cp;
		if ((cp = _branchto(cp, stop, 'e')) == NULL)
		    ERREXIT(0, MSG_NOENDING2, instring, NULL, xp - 2 - start);
	    }
	    break;

	case 'e':    /* ELSE - branch to ENDIF */
	    xp = cp;
	    if ((cp = _branchto(cp, stop, ';')) == NULL)
		ERREXIT(0, MSG_NOENDING1, instring, "e", xp - 2 - start);
	    break;

	case ';':    /* ENDIF */
	    /* check for end of while loop */
	    if (whilelevel > 0 && cp == endwhile[whilelevel]) {
		if ((vars[varindx[whilelevel]] -= 1) > 0) {
		    cp = begwhile[whilelevel];
		    break;
		}
		whilelevel--;
	    }
	    break;

	 /*******************************************************************
		"%#" Operator	-	For Regular Expressions
		-----------------------------------------------
							03/05/92


           %#xx"...@..."  Extracts a selected portion of the string attribute
	   whose name is xx.  The selection criteria is defined by the
	   pattern ...@... enclosed in double-quotes immediately following xx.
	   The selection pattern consists of three parts:
		. a prefix regular expression, which specifies the pattern of
		  the string immediately preceding the string to be extracted.
		  If the prefix regular expression is missing, the extracted
		  string will consist of the entire string preceding the
		  pattern specified by the suffix regular expression.

		. @, which designates the string to be extracted.  The 
		  extracted string replaces the %#xx"...@..." operator
		  sequence in the attribute string currently being processed.

		. a suffix regular expression, which specifies the pattern of
		  the string immediately following the string to be extracted.
		  If the suffix regular expression is missing, the extracted
		  string will consist of the entire string following the
		  pattern specified by the prefix regular expression.

	   No string is extracted in the following cases:
		-	the value string of attribute xx is null;
		-	prefix regular expression is not null and does not
			have a corresponding match in the attribute value
			string;
		-	suffix regular expression is not null and does not
			have a corresponding match in the attribute value
			string;

	  Examples:
		a.	Given that
			mU	=	courier[10,12(ibm.850,ibm.437)]
			_I	=	%#mU"[a-z]*\\[[0-9]*,@\\("

			the value of _I results in 	12

	  Note that each pair of backslashes results in one backslash
	  after being digested by "piodigest" at the time of creating or
	  modifying a virtual printer.  And an extra pair is also
	  needed to quote a character to prevent it from being construed as
	  a regular expression special character like ('*', '[', etc.), if
	  desired.  Also note that the characters @ and " will have to
	  be extra-quoted if they need to retain their literal meaning.

	  One can also use % operators within a regular expression. However,
	  there is a caveat regarding usage of embedded "%#" operator within
	  a regular expression part of a "%#" operator in the sense special
	  characters @ and " can not be used for their literal meaning
	  within the embedded regular expression (see example below).  But
	  this result can be achieved by placing the embedded (inner) "%#"
	  operator in a separate attribute value and by including this
	  attribute within the reg exp of the outer "%#" operator.

	  Examples:
		a.	Given that
			mU      =       courier[10,12;35;45(ibm.850,ibm.437)]
			_s	=	courier
			_p	=	10
			_I	=	%#mU"%I_s\\[%I_p,@[];]"
			t0	=	%#mU"%I_s\\[%I_p,[a-zA-Z0-9;]*\\(@\\)"

			the value of _I results in 	12
			and the value of t0 results in 	ibm.850,ibm.437

		b.	Given that
			e4	=	042444555fffabc@KjFm\\"222333ggg
			e7	=	ajklsfbbbafjklbbb890234abc
			e5	=	%#e4"%#e7\\"bbb[0-9]+\\@\\"\\@[a-zA-Z]+
					\\\\\\"@[a-zA-Z]+"

			the value of e5 results in	222333
			(this is done by first evaluating the inner "%#"
			 operator sequence,
				%#e7\\"bbb[0-9]+\\@\\"
			 , which evaluates to
				abc
			 , and then by evaluating the outer "%#" operator
			 sequence,
				 %#e4"abc\\@[a-zA-Z]+\\\\\\"@[a-zA-Z]+"

			 Notice that special characters @ and " in the
			 embedded reg exp are quoted so that they are not
			 meaningful to the outer reg exp.)

		c.	Given that
			wi	=	aflj@test007discarded
			wj	=	asfjkdl007@garbage20384junk
			wk	=	%wj"[a-z]*%#wi\\"[a-z]*\\\\@[a-z]*\\@[a-
					z]*\\"\\@@[0-9]*"
							       ----
					wrong in this context because the
					desired effect of considering @ as
					a literal character in the inner reg
					exp is countermanded by the fact that
					this character is preceded by even pair
					of backquotes which results in this
					character being taken as a special
					character (extraction character) by the
					outer reg exp.  This can be avoided by
					the following way.

			wl	=	%#wi"[a-z]*\\@[a-z]*@[a-z]*"
			wk	=	%wj"[a-z]*%Iwl\\@@[0-9]*"

			the value of wk results in	garbage

	 ********************************************************************/
	case '#':
	   {
	      char			*attrvalstr = NULL;
						/* attribute value string */
	      unsigned int		attrvallen = 0;
						/* attribute value string len */
	      unsigned int		ptrnlen = 0;
						/* reg exp pattern len */
	      char			*ptrnbegptr;
						/* reg exp pattern beg ptr */
	      char			*ptrndivptr;
						/* reg exp pattern divider ptr;
						   location of string to be
						   extracted (specified by
						   '@' character in reg exp);
						   also a divider of reg exp
						   into two sub parts, prefix
						   and suffix expressions; */
	      char			*mtchbegptr;
						/* beg ptr of string to be
						   extracted from the attr
						   value string that matches
						   with the position of '@'
						   in the reg exp */
	      char			*mtchendptr;
						/* end ptr of string to be
						   extracted from the attr
						   value string that matches
						   with the position of '@'
						   in the reg exp */
	      register char		*tmpstr;
						/* temp char ptr */
	      register int		enddelimit_found = FALSE;
						/* flag - end delimit found */
	      register int		divider_found = FALSE;
						/* flag - divider found */
	      register int		noof_bkslshs = 0;
						/* no of succ. back slashes */
	      char			regexpbuf[MAXREGEXPLEN + 1];
						/* reg exp tmp buf */
	      size_t			tmpval = 0UL;
						/* tmp variable */
	      char			*regcmpptr;
						/* ptr returned from "regcmp" */
	      char			*regexptr;
						/* ptr returned from "regex" */
	      register char		*tmp1str;
						/* temp1 char ptr */
	      char			*regexpptr;
						/* ptr to reg exp part */
	      struct attr_info		sav_attr_tab_p;
						/* attribute table record to
						   be saved for dummy attr */
	      register int		dummy_attr_used;
						/* flag for usage of dummy
						   attribute for reg exp's */

	      /* Check if the attribute exists in the data base */
	      if (!(attr_tab_p2 = get_attrtab_ent (cp)))
	      {
		 ERREXIT (0, MSG_BADATTR2, instring, cp - 1, cp - start);
	      }
	      attr_tab_p2->flags |= usedbysomeone;

	      xp = outp;		/* save the output pointer */
	      if ((rc = pioparm (cp, NULL, strbuf, getstrsiz, 0, id, future))
		  < 0)
		 return rc;

	      /* Extract the matching string only if the attribute value
		 string is not null and a pattern extraction character
		 '@' is specified in the regular expression */

	      *outp = 0;
	      attrvallen = outp - xp;
	      outp = xp;

	      /* save the attribute value string */
	      if (attrvallen  &&  !(attrvalstr = strdup (outp)))
	      {
	         ERREXIT (0, MSG_MALLOC, instring, cp - 1, cp - 2 - start);
	      }

	      /* determine pattern divider (position of '@') and limits  */
	      if (ptrnbegptr = cp + 2, *ptrnbegptr++ != REGPTRN_DELIMITER)
	      {
	         ERREXIT (0, MSG_MISS_PTRNDELIMIT, instring,
	     	          cp - 1, cp - 2 - start);
	      }
              for (tmpstr = ptrnbegptr; *tmpstr  &&  !enddelimit_found;
		   tmpstr++)
	      {
	         switch (*tmpstr)
	         {
	            case '\\':
	     	       noof_bkslshs++;
	     	       break;

	            case REGPTRN_DIVIDER:
	     	       if (!divider_found  &&  !(noof_bkslshs % 2))
	     	       {
	     	          ptrndivptr = tmpstr, divider_found = TRUE;
	     	       }
		       noof_bkslshs = 0;
	     	       break;

	            case REGPTRN_DELIMITER:
	     	       if (!(noof_bkslshs % 2))
	     	          enddelimit_found = TRUE;
		       noof_bkslshs = 0;
	     	       break;

	            default:
	     	       noof_bkslshs = 0;
	     	       break;
	         }
	      }
	      if (!enddelimit_found)
	      {
	         ERREXIT (0, MSG_MISS_PTRNDELIMIT, instring,
	     	          cp - 1, cp - 2 - start);
	      }
	      if (!divider_found)
	      {
	         ERREXIT (0, MSG_MISS_PTRNEXTRACT, instring,
	     	          cp - 1, cp - 2 - start);
	      }
	      ptrnlen = tmpstr - 1 - ptrnbegptr;

	      /* Extract the pattern only if attribute value string is not
		 empty and if regular expression is not null and if string
		 extraction character ('@') is specified */
	      if (attrvallen  &&  ptrnlen  &&  divider_found)
	      {
		 do
		 {
		    if (ptrnbegptr == ptrndivptr)	/* no prefix reg exp */
		       mtchbegptr = attrvalstr;
		    else
		    {
		       tmpval = ptrndivptr - ptrnbegptr >
				sizeof (regexpbuf) - 1 ?
			        sizeof (regexpbuf) - 1 :
				ptrndivptr - ptrnbegptr;
		       (void) strncpy (regexpbuf, ptrnbegptr, tmpval);
		       *(regexpbuf + tmpval) = 0;

		       /* strip off any extra backslashes that were placed in
			  the regular expression (preceding our magic
			  characters '@', '"' as well as other characters)
			  so as not to upset our search of patterns before */
		       for (noof_bkslshs = 0, tmp1str = tmpstr = regexpbuf;
			    *tmpstr; tmpstr++)
			  switch (*tmpstr)
			  {
	                     case '\\':
				*tmp1str++ = *tmpstr;
	     	                noof_bkslshs++;
	     	                break;

	                     case REGPTRN_DIVIDER:
	                     case REGPTRN_DELIMITER:
				if (noof_bkslshs % 2)
				   *--tmp1str = *tmpstr,
				   tmp1str++;
				else
				   *tmp1str++ = *tmpstr;
				noof_bkslshs = 0;
				break;

	                     default:
				*tmp1str++ = *tmpstr;
	     	                noof_bkslshs = 0;
				break;
		          }
		       *tmp1str = 0;

		       /* Resolve the references (if any) in the prefix
			  regular expression part.  Use a dummy attribute
			  to store the result. */
	               /* Check if the attribute exists in the data base.
		          If not, use the regular expression as is. */
	               if (!(attr_tab_p2 = get_attrtab_ent (DUMMY_ATTR)))
	               {
			  dummy_attr_used = FALSE;
			  regexpptr = regexpbuf;
	               }
		       else
		       {
			  dummy_attr_used = TRUE;

			  /* Save the attribute table record so that the
			     original values can be restored even in cases
			     of recursive invocations. */
			  sav_attr_tab_p = *attr_tab_p2;

			  /* Initialize the dummy attribute values */
			  attr_tab_p2->datatype = CONST_STR;
			  attr_tab_p2->flags = 0;
	                  if (!(attr_tab_p2->ptypes.sinfo->ptr =
				strdup (regexpbuf)))
	                  {
	                     ERREXIT (0, MSG_MALLOC, instring, cp - 1,
				      cp - 2 - start);
	                  }
			  attr_tab_p2->ptypes.sinfo->len =
			     strlen (attr_tab_p2->ptypes.sinfo->ptr);
			  
			  /* Resolve the references in the reg exp */
			  xp = outp;
	                  if ((rc = pioparm (DUMMY_ATTR, NULL, strbuf,
			       getstrsiz, 0, id, future)) < 0)
		             return rc;
			  *outp = 0, regexpptr = outp = xp;
		       }

		       /* Compile the prefix regular expression and find
			  a match in the attribute value string */
		       if (!(regcmpptr = regcmp (regexpptr, (char *) NULL)))
		       {
	                  ERREXIT (0, MSG_REGCMPERR, instring, cp - 1,
				   cp - 2 - start);
		       }
		       regexptr = regex (regcmpptr, attrvalstr);
		       free (regcmpptr);
		       if (dummy_attr_used)
		       {
		          free (attr_tab_p2->ptypes.sinfo->ptr);
		          *attr_tab_p2 = sav_attr_tab_p;
		       }
		       if (!regexptr)
		       {
			  /* No match was found, skip next (suffix) match */
			  continue;
		       }
		       mtchbegptr = regexptr;
		       if (!*mtchbegptr)
		       {
			  /* Null matching string, hence skip next match */
			  continue;
		       }
		    }

		    if (ptrnbegptr + ptrnlen == ++ptrndivptr)
						/* no suffix reg exp */
		       mtchendptr = attrvalstr + attrvallen;
		    else
		    {
		       tmpval = (ptrnbegptr + ptrnlen) - ptrndivptr >
				sizeof (regexpbuf) - 1 ?
			        sizeof (regexpbuf) - 1 :
				(ptrnbegptr + ptrnlen) - ptrndivptr;
		       (void) strncpy (regexpbuf, ptrndivptr, tmpval);
		       *(regexpbuf + tmpval) = 0;

		       /* strip off any extra backslashes that were placed in
			  the regular expression (preceding our magic
			  characters '@', '"' as well as other characters)
			  so as not to upset our search of patterns before */
		       for (noof_bkslshs = 0, tmp1str = tmpstr = regexpbuf;
			    *tmpstr; tmpstr++)
			  switch (*tmpstr)
			  {
	                     case '\\':
				*tmp1str++ = *tmpstr;
	     	                noof_bkslshs++;
	     	                break;

	                     case REGPTRN_DIVIDER:
	                     case REGPTRN_DELIMITER:
				if (noof_bkslshs % 2)
				   *--tmp1str = *tmpstr,
				   tmp1str++;
				else
				   *tmp1str++ = *tmpstr;
				noof_bkslshs = 0;
				break;

	                     default:
				*tmp1str++ = *tmpstr;
	     	                noof_bkslshs = 0;
				break;
		          }
		       *tmp1str = 0;

		       /* Resolve the references (if any) in the suffix
			  regular expression part.  Use a dummy attribute
			  to store the result. */
	               /* Check if the attribute exists in the data base.
		          If not, use the regular expression as is. */
	               if (!(attr_tab_p2 = get_attrtab_ent (DUMMY_ATTR)))
	               {
			  dummy_attr_used = FALSE;
			  regexpptr = regexpbuf;
	               }
		       else
		       {
			  dummy_attr_used = TRUE;

			  /* Save the attribute table record so that the
			     original values can be restored even in cases
			     of recursive invocations. */
			  sav_attr_tab_p = *attr_tab_p2;

			  /* Initialize the dummy attribute values */
			  attr_tab_p2->datatype = CONST_STR;
			  attr_tab_p2->flags = 0;
	                  if (!(attr_tab_p2->ptypes.sinfo->ptr =
				strdup (regexpbuf)))
	                  {
	                     ERREXIT (0, MSG_MALLOC, instring, cp - 1,
				      cp - 2 - start);
	                  }
			  attr_tab_p2->ptypes.sinfo->len =
			     strlen (attr_tab_p2->ptypes.sinfo->ptr);
			  
			  /* Resolve the references in the reg exp */
			  xp = outp;
	                  if ((rc = pioparm (DUMMY_ATTR, NULL, strbuf,
			       getstrsiz, 0, id, future)) < 0)
		             return rc;
			  *outp = 0, regexpptr = outp = xp;
		       }

		       /* Compile the suffix regular expression and find
			  a match in the attribute value string */
		       if (!(regcmpptr = regcmp (regexpptr, (char *) NULL)))
		       {
	                  ERREXIT (0, MSG_REGCMPERR, instring, cp - 1,
				   cp - 2 - start);
		       }
		       regexptr = regex (regcmpptr, mtchbegptr);
		       free (regcmpptr);
		       if (dummy_attr_used)
		       {
		          free (attr_tab_p2->ptypes.sinfo->ptr);
		          *attr_tab_p2 = sav_attr_tab_p;
		       }
		       if (!regexptr)
		       {
			  /* No match was found, hence skip extraction */
			  continue;
		       }
		       else
		       {
			  /* Match was found  */
			  mtchendptr = __loc1;
		       }
		    }

		    /* Copy the extracted string into the output buffer */
		    if (mtchbegptr != mtchendptr)
		    {
		       (void) memcpy (outp, mtchbegptr,
				      mtchendptr - mtchbegptr),
		       outp += mtchendptr - mtchbegptr;
		    }
		 } while (0);
	      }				/* end if attrvallen */

	      if (attrvalstr)
	         free (attrvalstr);

	      /* housekeeping */
	      if (!strncmp (cp, FLAGCHAR, 1))
		 refsflagarg = TRUE;	/* flag value is referenced */
	      cp += 2 + 2 + ptrnlen;	/* 2 for attrname; 2 for delimiters */
	   }				/* end of block for '%#' processing */
	   break;

	default:
	    ERREXIT(0, MSG_BADSEQ, instring, cp - 1, cp - 2 - start);
	}
    }

    /* If flag value referenced, mark this and all ancestor */
    /* attributes to indicate they are affected by a flag value */
    /* Do not do this, if an attribute limit string is being evaluated */
    if (!getlimitstr  &&  refsflagarg) /* if references a flag argument value */
	for (cnt = 1; cnt <= nest_level; cnt++)
	    if ((attr_tab_p = get_attrtab_ent(&attr_nest[cnt][0])))
		attr_tab_p->flags |= REFSFLAGARG; /* references a flag arg */

    if (!(--nest_level))
    {
	strlength = (int) outp - (int) strbuf;
	if (id == PIOCMDOUT)
	{
	    /* We are reusing variable cp here */
	    for (cp = strbuf; cp < outp; cp++)
		pioputchar(*cp);
	    free(cmdoutbuf);

	    /* bleed off any remaining passthru bytes */
	    for(; bytes_to_passthru; bytes_to_passthru--)
		if ((tmpint = piogetc(fileptr)) == EOF)
		{
		    bytes_to_passthru = 0;
		    break;
		}

	    /* zero the value provided to command strings */
	    if (attr_pthru_p != NULL)    /* safety valve */
		for (cnt = 0; cnt < PIO_NUM_MODES; cnt++)
		    int_pthru_p->value[cnt] = 0;
	}
	else
	    *outp = '\0';
	piomode = piomode_saved; /* put it back in case %o or %U was done */
    }

    if (getlimitstr)
        usedbysomeone = sav_usedbysomeone;

    return (strlength);
}

static char *
_branchto (register char *cp, char *stop, char to)
{
    register int level = 0;
    register char c;

    for (c = *cp++; cp <= stop; c = *cp++) {
	if (c == '%') {
	    if ((c = *cp++) == to || c == ';') {
		if (level == 0) {
		    return cp;
		}
	    }
	    if (c == '?' || c == 'w')
		level++;
	    if (c == ';')
		level--;
	}
    }
    return (NULL);  /* didn't find what we're looking for */
}

