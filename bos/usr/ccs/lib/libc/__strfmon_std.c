static char sccsid[] = "@(#)65	1.5.1.5  src/bos/usr/ccs/lib/libc/__strfmon_std.c, libcfmt, bos411, 9428A410j 3/30/94 14:56:47";
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  __strfmon_std
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

#define _ILS_MACROS
#include <sys/localedef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fp.h>
#include <errno.h>
#include <ctype.h>

#define max(a,b)   (a<b ? b : a)
#define min(a,b)   (a>b ? b : a)
#define PUT(c)     (strp < strend ? *strp++  = (c) : toolong++)
#define MAXFSIG 17 	/* Maximum significant figures in floating-point num */
#define SIZE 1000	/* Size of a working buffer */

extern char *fcvt();	/* libc routines for floating conversion */
extern int __ld(double);/* it returns number of digits to left of decimal */

typedef	signed char schar;

static int do_format (_LC_monetary_objhdl_t hdl, char *s,size_t maxsize, 
		      double dval);
static void bidi_output (char *infield, char **outfield, int dir);
static void out_cur_sign (_LC_monetary_objhdl_t hdl, char **begin, char **end,
			  int sign);
static void do_out_cur_sign (schar pn_cs_precedes, schar pn_sign_posn, 
 		      schar pn_sep_by_space, char *pn_sign, char **begin, 
		      char **end, int sign);


/*
 * global variables
 */

int w_width;		/* minimum width for the current format */
static int prec;		/* precision of current format */
int n_width;		/* width for the #n specifier */
int leftflag;		/* logical flag for left justification */
static int wflag;		/* logical flag for width */
static int pflag;		/* logical flag for precision */
static int nflag;		/* logical flag for #n specifier */
int no_groupflag;	/* logical flag for grouping */
int signflag;		/* logical flag for the + specifier */
static int cdflag;		/* logical flag for credit/debit specifier */
int parenflag;		/* logical flag for C parenthesis specifier */
int no_curflag;		/* logical flag for currency symbol supression*/
int byte_left;		/* number of byte left in the output buffer */
char *cur_symbol;	/* local or interantional currency symbol */
char fill_buf[MB_LEN_MAX+1];
			/* filling character for =f specifier */
char *fill_char=fill_buf; 
			/* pointer to the fill character buffer */
static int mb_length;		/* length of the multi-byte char of current locale */

/*
 * FUNCTION: This is the standard method for function strfmon().
 *	     It formats a list of double values and output them to the
 *	     output buffer s. The values returned are affected by the format
 *	     string and the setting of the locale category LC_MONETARY. 
 *
 * PARAMETERS:
 *           _LC_monetary_objhdl_t hdl - the handle of the pointer to 
 *			the LC_MONETARY catagory of the specific locale.
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output including the null
 *			      termination character.
 *           char *format - format that montary value is to be printed out
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize or error detected.
 */


ssize_t
__strfmon_std(_LC_monetary_objhdl_t hdl, char *s, size_t maxsize, 
		  const char *format, va_list ap)
{
	char *strp;		/* pointer to output buffer s */
	char *fbad;		/* points to where format start to be invalid */
	char *strend;		/* last availabel byte in output buffer s */
	char ch;		/* the current character in the format string */
	int toolong = 0;	/* logical flag for valid output length */
	int i;			/* a working variable */

	prec = wflag = pflag = nflag = cdflag = 0;
	mb_length = 1;

	byte_left = maxsize;
	if (!s || !__OBJ_DATA(hdl) || !format)
		return (-1);
	strp = s;
	strend = s + maxsize - 1;
	while ((ch = *format++) && !toolong) {
		if (ch != '%') {
			PUT(ch);
			byte_left--;
		}
		else { 
			fbad = format;
			fill_buf[0] = ' ';
			fill_buf[1] = '\0';
			prec = w_width = n_width = 0;
			leftflag = wflag = pflag = nflag = no_groupflag = 0;
			signflag = cdflag = parenflag = no_curflag = 0;
			/*-------- scan for flags --------*/
			i=0;
			while(1) {
				switch(*format) {
				case '=': case '^': case '~': case '!': case 'S':
				case '+': case '(': case 'C': case '-':
					break;
				default: 
					i=1;	/* there are no more optional flags */
				}
				if (i) break;	/* exit loop */
				if (*format == '=') {	/* =f fill character */
					format++;
					if ((mb_length=mblen(format, MB_CUR_MAX))!=-1){
						for (i = mb_length; i>0; i--)
							*fill_char++ = *format++;
						*fill_char = '\0';
						fill_char = fill_buf;
					}
					else
						return (-1);	/* invalid char */
				}	
				if (*format == '^' ||	/* X/open or AIX format */
				    *format == '~') {	/* no grouping for thousands */
					format++;
					no_groupflag++;
				}
				if (*format == '+') {	/* locale's +/- sign used */
					format++;
					signflag=1;
					parenflag=0;
				}
				else if (*format == 'C') { /* locale's CR/DB sign used*/
					format++;
					cdflag++;
				}
				else if (*format == '(') { /* locale's paren. for neg.*/
					format++;
					parenflag=1;
					signflag=0;
				} 
				if (*format == '!' || *format == 'S') { /* suppress currency symbol */
					format++;
					no_curflag++;
				}
				if (*format == '-') {		/* - left justify */
					format++;
					leftflag++;
				}
			} /* end of while(1) loop */
			/*-------- end scan flags --------*/
			while (isdigit (*format)) {	/* w width field */
				w_width *= 10;
				w_width += *format++ - '0';
				wflag++;
			}
			if (*format == '#') {	/* #n digits preceeds decimal (left precision) */
				nflag++;
				format++;
				while (isdigit(*format)) {
					n_width *= 10;
					n_width += *format++ - '0';
				}
			}
			if (*format == '.') {		/* .p precision (right precision) */
				pflag++;
				format++;
				while (isdigit(*format)) {
					prec *= 10;
					prec += *format++ - '0';
				}
			}
			switch (*format++) {
			case '%':
				PUT('%');
				byte_left--;
				break;

			case 'i':	/* international currency format */
			case 'a':	/* international currency format */
				cur_symbol = __OBJ_DATA(hdl)->int_curr_symbol;
				if (!pflag && 
				    (prec = __OBJ_DATA(hdl)->int_frac_digits) < 0)
					prec = 2;
				if ((i = do_format(hdl, strp,maxsize,
					           va_arg(ap,double))) == -1)
					return (-1);
				else {
					strp += i;
					byte_left -= i;
				}
				break;

			case 'n':	/* local currency format */
			case 'c':	/* local currency format */
				cur_symbol = __OBJ_DATA(hdl)->currency_symbol;
				if (!pflag && 
				    (prec = __OBJ_DATA(hdl)->frac_digits)< 0)
					prec = 2;
				if ((i = do_format(hdl, strp,maxsize,
				    		va_arg(ap,double))) == -1)
					return (-1);
				else {
					strp += i;
					byte_left -= i;
				}
				break;
			default:
				format = fbad;
		   		PUT('%');
				byte_left--;
				break;
			}
		} /* else */
		
	} /* while */

	if (*strp!='\0')	/* Is NULL in place already?  If not, then add one before    */
		PUT('\0');	/* returning if it will fit.  If it doesn't fit, set toolong */
	if (toolong) {
		errno = E2BIG;
		return (-1);
		}
	else
		return (strp - s);
}


/*
 * FUNCTION: do_format()
 *	     This function performs all the necessary formating for directives
 *	     %i (or %a) and %n (or %c). The output will be written to the output
 *	     buffer str and the number of formatted bytes are returned.
 *
 * PARAMETERS:
 *           _LC_monetary_objhdl_t hdl - the handle of the pointer to 
 *		the LC_MONETARY catagory of the specific locale.
 *           char *str - location of returned string
 *           size_t maxsize - maximum length of output including the null
 *                            termination character.
 *           char *dval - the double value to be formatted.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize or any error.
 */


int do_format (_LC_monetary_objhdl_t hdl, char *str, size_t maxsize, 
	       double dval)

{
	char *s;		/* work buffer for string handling */
	char *number;		/* work buffer for converted string of fcvt() */
	int dig_to_left;	/* number of digits to the left of decimal in
				   the actual value of dval */
	int fcvt_prec;		/* number of digits to the right of decimal for 
				   conversion of fcvt */
	int decpt;		/* a decimal number to show where the radix 
				   character is when counting from beginning */
	int sign;		/* 1 for negative, 0 for positive */
	char *b;		/* points to the beginning of the output */
	char *e;		/* points to the end of the output */
	char lead_zero[MAXFSIG];/* leading zeros if necessary */
	char *t;		/* the grouping string of the locale */
	char *separator;	/* thousand separator string of the locale */
	char *radix;		/* locale's radix character */
	int i,j,k;		/* working variable as index */
	int gap;		/* number of filling character needed */
	int sep_width;		/* the width of the thousand separator */
	int radix_width;	/* the width of the radix character */
	int sv_i;
	char temp_buff[SIZE];	/* a work buffer for the whole formatted str */
	char *temp;		/* pointer to temp_buff */
	char *temp_m = NULL ;	/* pointer to temp buffer created by malloc */
	char temp_buff2[SIZE];	/* a work buffer added for d84736 */

	/*
	/* First, convert the double value into a character string by
	/* using fcvt(). To determine the precision used by fcvt():
	/*
	/* if no digits to left of decimal,
	/* 	then all requested digits will be emitted to right of decimal.
	/*	hence, use max of (max-sig-digits,user's-requested-precision).
	/*
	/* else if max-sig-digits <= digits-to-left
	/* 	then all digits will be emitted to left of decimal point.
	/*  	Want to use zero or negative prec value to insure that rounding
	/* 	will occur rather than truncation.
	/*
	/* else
	/*	digits can be emitted both to left and right of decimal, but
	/*	only potential for rounding/truncation is to right of decimal.
	/*	Hence, want to use user's request precision if it will not 
	/*	cause truncation, else use largest prec that will round 
	/*	correctly when max. number of digits is emitted.
	 */
  
	if ( 2*maxsize < SIZE)
		temp = temp_buff;
	else if (temp_m = (char *) malloc (2*maxsize))
		temp = temp_m;
	else
		return (-1);		/* malloc failed */
	dig_to_left = __ld(dval);	/* get the num of digit to the left */
	if (dig_to_left <= 0)		/* determine the precision to be used */
		fcvt_prec = min (prec, MAXFSIG);
	else if (dig_to_left >= MAXFSIG) 
		fcvt_prec = MAXFSIG - dig_to_left;
	else
		fcvt_prec = min (prec, MAXFSIG - dig_to_left);
	number = fcvt (dval, fcvt_prec, &decpt, &sign);

	/*
	/* Fixing the position of the radix character (if any). Output the
	/* number using the radix as the reference point. When ouptut grows to
	/* the right, decimal digits are processed and appropriate precision (if
	/* any) is used. When output grows to the left, grouping of digits
	/* (if needed), thousands separator (if any), and filling character
 	/* (if any) will be used.
	/* 
	/* Begin by processing the decimal digits.
	 */

	b = temp + maxsize;
	if (prec) {
		if (*(radix = __OBJ_DATA(hdl)->mon_decimal_point)) {
					 /* set radix character position */
			radix_width = strlen (radix);
			for (i=1; i <= radix_width; i++)
				*(b+i) = *radix++;
			e = b + radix_width + 1;
		}
		else 			/* radix character not defined */
			e = b + 1;
		if (decpt < 0) {
			strcpy(temp_buff2, number);
			sprintf(number, "%*.*d%s", abs(decpt), abs(decpt), 0, temp_buff2);
			decpt=0;
		}
		s = number + decpt; /* copy the digits after the decimal */
		while (*s)
			*e++ = *s++;
	}
	else 			/* zero precision, no radix character needed */
		e = b + 1;

	/*
	/* Output the digits preceeding the radix character. Leading zeros 
	/* (if dig_to_left < #n and filling char is zero) are padded in front
 	/* of number.  
	 */

	if ((i = j = (n_width - dig_to_left)) > 0) {	/* pad with lead_zero */
		s = lead_zero;
		for (; i > 0; i--)
			*s++ = '0';
		*s = '\0';
		number = strcat (lead_zero, number);
		s = number + j + decpt -1;
	}
	else
		s = number + decpt - 1;	/* points to the digit preceed radix */

	if (no_groupflag) 	/* no grouping is needed */
		for (i=0; i < dig_to_left; i++) 
			*b-- = *s--;
	else {
		if (*(t = __OBJ_DATA(hdl)->mon_grouping)) {
			/* get grouping format,eg: "^C^B\0" == "3;2" */
			j = dig_to_left;
			separator = __OBJ_DATA(hdl)->mon_thousands_sep;
			sep_width = strlen(separator);
			while (j > 0) {
				if (*t) {
					i = *t++; 	/* get group size */
					sv_i = i;
				}
				else
					i = sv_i;
					/* otherwise, repeat 'n reuse old i */
				if (i>j || i==-1)
					i=j;		/* no more grouping */
				for (; i > 0; i--, j--)  
					*b-- = *s--;	/* copy group */
				for (i=sep_width-1;j && *separator && i>=0;i--)	
					*b-- = *(separator + i);
			} /* while */
			/* 
			 * Note that in "%#<x>n", <x> is the number of digits,
			 *   not spaces, so after the digits are in place, the
			 *   rest of the field must be the same size as if the
			 *   number took up the whole field.  This means 
			 *   adding fill chars where there would have been a
			 *   separator.  e.g.: "%#10n"
			 *       $9,987,654,321.00    
			 *	 $@@@@@@@54,321.00    correct
			 *	 $@@@@@54,321.00      incorrect
			 *   So, n_width should be n_width (i.e. digits) + 
			 *   number of separators to be inserted.
			 * Solution:  just increment n_width for every
			 *   separator that would have been inserted.  In this
			 *   case, 3.  Also, follow mon_grouping rules about
			 *   repeating and the -1.
			 */
			t = __OBJ_DATA(hdl)->mon_grouping;
			k = n_width;
			if (*t) {
				if((i=*t++)==-1)
					i=k;
				while(k-i>0) {
					k-=i;
					n_width++;
					if (*t && (i=*t++)==-1)
						break;
					}
				}
		}
		else /* the grouping format is not defined in this locale */
                	for (i=0; i < dig_to_left; i++)
				*b-- = *s--;
	} /* else */ 

	/*
	/* Determine if padding is needed. 
	/* If the number of digit prceeding the radix character is greater
	/* than #n (if any), #n is ignored. Otherwise, the output is padded
	/* with filling character (=f, if any) or blank is used by default.
	 */
	
	if (nflag && (gap = n_width - (temp + maxsize - b)) > 0) {
		for (i=0; i < gap; i++)
			for (j=mb_length-1; j>=0; j--) 
				*b-- = fill_buf[j]; /* copy fill char */
	}

	/* 
	/* At here, the quantity value has already been decided. What comes
	/* next are the positive/negative sign, monetary symbol, parenthesis,
	/* and CR/DB symbols. The signflag, cdflag, parenflag, and no_curflag
	/* will be considered first to determine the sign and currency format.
	/* If none of them are defined, the locale's defaults are used.
	 */

	if (signflag) {		/* use locale's +/- sign repesentation */
		if (sign)
			i = __OBJ_DATA(hdl)->n_sign_posn;
		else
			i = __OBJ_DATA(hdl)->p_sign_posn;
	     	switch (i) {
		case 0: 	/* parens enclose currency and quantity */
		case 1: 	/* sign preceed currency and quantity */
		case 2:		/* sign succeed currency and quantity */
		case 3:		/* sign preceed currency symbol */
		case 4:		/* sign succeed currency symbol */
		    	out_cur_sign (hdl, &b, &e, sign);
			break;
	    	}
	}
	else if (parenflag) {
		if (sign && *__OBJ_DATA(hdl)->left_parenthesis 
		    && *__OBJ_DATA(hdl)->right_parenthesis) {
				out_cur_sign (hdl, &b, &e, sign);
				bidi_output (__OBJ_DATA(hdl)->left_parenthesis, 
					     &b, 0);
				bidi_output (__OBJ_DATA(hdl)->right_parenthesis, 
					     &e, 1);
				}
		else
			out_cur_sign (hdl, &b, &e, sign);
	}
	else if (cdflag) {	/* CR/DB sign is used */
		if (sign) {	/* negative number */
			if (*__OBJ_DATA(hdl)->debit_sign)
				bidi_output (__OBJ_DATA(hdl)->debit_sign, &e, 1);
		}
		else {
			if (*__OBJ_DATA(hdl)->credit_sign)
				bidi_output (__OBJ_DATA(hdl)->credit_sign, &e, 1);
		}
		out_cur_sign (hdl, &b, &e, sign);
	}
	else {	/* use all the attributes of the locale's default */
		if (sign)
			i = __OBJ_DATA(hdl)->n_sign_posn;
		else
			i = __OBJ_DATA(hdl)->p_sign_posn;
   	    	switch (i) {
		case 0:		/* Paren. around currency and quantity*/
			out_cur_sign (hdl, &b, &e, sign);
			if (*__OBJ_DATA(hdl)->left_parenthesis 
			    && *__OBJ_DATA(hdl)->right_parenthesis) {
				bidi_output (__OBJ_DATA(hdl)->left_parenthesis, 
					     &b, 0);
				bidi_output (__OBJ_DATA(hdl)->right_parenthesis, 
					     &e, 1);
			}
			break;
				
		case 1: 	/* sign preceed currency and quantity */
		case 2:		/* sign succeed currency and quantity */
		case 3:		/* sign preceed currency symbol */
		case 4:		/* sign succedd currency symbol */
		    	out_cur_sign (hdl, &b, &e, sign);
			break;

		case 5:
			if (sign && *__OBJ_DATA(hdl)->credit_sign)
				bidi_output (__OBJ_DATA(hdl)->credit_sign, &e, 1);
			else if (!sign && *__OBJ_DATA(hdl)->debit_sign)
				bidi_output (__OBJ_DATA(hdl)->debit_sign, &e, 1);
			out_cur_sign (hdl, &b, &e, sign);
			break;
	    	}
	} /* else */

	/* 
	/* By setting e (the last byte of the buffer) to \0 and increment 
	/* b (the first byte of the buffer), now the temp buffer should 
 	/* have a completely formatted null terminated string starting and
	/* ending at b and e. Before the result is copied into the s buffer,
	/* check if the formatted string is less than the w-field width and
	/* determine its left or right justification.
	 */
				
	b++;
	*e = '\0';
	i = strlen (b);
	if (max(i, w_width) > byte_left) {
		errno=E2BIG;
		return (-1);
		}
	if (wflag && i < w_width) {	/* justification is needed */
		if (leftflag) {
			while (*b)
				*str++ = *b++;
			for (i = w_width-i; i > 0; i--)
				*str++ = ' ';
		}
		else {
			for (i = w_width-i; i > 0; i--)
				*str++ = ' ';
			while (*b)
				*str++ = *b++;
		}
		*str = '\0';
		if (temp_m)
			free (temp_m);
		return (w_width);
	}
	else {
		strcpy (str, b);
		if (temp_m)
			free (temp_m);
		return (i);
	}
}
			    
/*
 * FUNCTION: out_cur_sign()
 *	     This function ouputs the sign related symbol (if needed) and 
 *	     the currency symbol (if needed) to the ouput buffer. It also
 *	     updates the beginning and ending pointers of the formatted 
 *	     string. This function indeed extract the sign related information
 *	     of the current formatting value and pass them to the sign  
 *	     independent formatting function do_out_cur_sign().
 *
 * PARAMETERS:
 *           _LC_monetary_objhdl_t hdl - the handle of the pointer to the 
 *			LC_MONETARY catagory of the specific locale.
 *	     char **begin - The address of the pointer which points to the 
 *			    begining of the output buffer.
 *	     char **end - The address of the pointer which points to the end
 *			  of the output buffer.
 *	     int sign - The sign of the current formatting monetary value.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     void.
 */

static char blanks[20];
void out_cur_sign(_LC_monetary_objhdl_t hdl, char **begin, char **end, int sign)
{
	schar pn_cs_precedes;
	schar pn_sign_posn;
	schar pn_sep_by_space;
	char *pn_sign;
	char *padded, *p;
	int   maxlen;
	int   i;

	if (sign) {	/* negative number with sign and currency symbol */ 
		pn_cs_precedes = __OBJ_DATA(hdl)->n_cs_precedes;
		pn_sign_posn = __OBJ_DATA(hdl)->n_sign_posn;
		pn_sep_by_space = __OBJ_DATA(hdl)->n_sep_by_space;
		pn_sign = __OBJ_DATA(hdl)->negative_sign;
	    }
	else {	/* positive number with sign and currency symbol */
		pn_cs_precedes = __OBJ_DATA(hdl)->p_cs_precedes;
		pn_sign_posn = __OBJ_DATA(hdl)->p_sign_posn;
		pn_sep_by_space = __OBJ_DATA(hdl)->p_sep_by_space;
		pn_sign = __OBJ_DATA(hdl)->positive_sign;
	    }

	if (nflag) {	/* align if left precision is used (i.e. "%#10n") */
		i=0;
		maxlen=strlen(__OBJ_DATA(hdl)->negative_sign);
		if (maxlen<strlen(__OBJ_DATA(hdl)->positive_sign)) {
			maxlen=strlen(__OBJ_DATA(hdl)->positive_sign);
			i=1;	/* positive sign string is longer */
			}
		if (maxlen<20)
			p=padded=blanks;
		else
			p=padded=(char *)malloc(maxlen*sizeof(char));
		if (i==sign)
			for (i=maxlen-strlen(pn_sign); i; i--) *padded++=' ';
		*padded='\0';
		padded=p;
		strcat(padded, pn_sign);
		}
	else
		padded = pn_sign;

        do_out_cur_sign ( pn_cs_precedes, pn_sign_posn,
                pn_sep_by_space, padded, begin, end, sign);

	if (nflag && maxlen >= 20)
		free(padded);
}


/*
 * FUNCTION: do_out_cur_sign()
 *	     This is a common function to process positive and negative 
 *	     monetary values. It outputs the sign related symbol (if needed)
 * 	     and the currency symbol (if needed) to the output buffer.
 *
 * PARAMETERS:
 *	     pn_cs_precedes - The p_cs_precedes or n_cs_precedes value.
 *	     pn_sign_posn - The p_sign_posn or n_sign_posn value.
 *	     pn_sep_by_space - The p_sep_by_space or n_sep_by_space value.
 *	     pn_sign - The positive_sign or negative_sign value.
 *	     char **begin - The address of the pointer which points to the 
 *			    begining of the output buffer.
 *	     char **end - The address of the pointer which points to the end
 *			  of the output buffer.
 *	     int sign - The sign of the current formatting monetary value.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     void.
 */

void do_out_cur_sign (schar pn_cs_precedes, schar pn_sign_posn, 
		      schar pn_sep_by_space, char *pn_sign, char **begin, 
		      char **end, int sign)
{
	char *b = *begin;
	char *e = *end;
	int i;

	if (pn_cs_precedes == 1) {	/* cur_sym preceds quantity */
		switch (pn_sign_posn) {
		case 0:
		case 1:	
		case 3:
			if (!no_curflag) {	
				if (pn_sep_by_space == 1)
					*b-- = ' ';
				bidi_output (cur_symbol, &b, 0);
			}
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &b, 0);
			break;
		case 2:
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*b-- = ' ';
				bidi_output (cur_symbol, &b, 0);
			}
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &e, 1);
			break;
		case 4:
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &b, 0);
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*b-- = ' ';
				bidi_output (cur_symbol, &b, 0);
			}
			break;
		}
	}
	else if (pn_cs_precedes == 0) { /* cur_sym after quantity */
		switch (pn_sign_posn) {
		case 0:
		case 1:
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &b, 0);
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*e++ = ' ';
				bidi_output (cur_symbol, &e, 1);
			}
			break;
		case 2:
		case 4:
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*e++ = ' ';
				bidi_output (cur_symbol, &e, 1);
			}
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &e, 1);  
			break;
		case 3:
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &e, 1);
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*e++ = ' ';
				bidi_output (cur_symbol, &e, 1);
			}
			break;
		}
	}
	else {		/* currency position not defined */
		switch (pn_sign_posn) {
		case 1:
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &b, 0);
			break;
		case 2:
			if (!parenflag && !cdflag)
				bidi_output (pn_sign, &e, 1);
			break;
		}
	}
	*begin = b;
	*end = e;
}

/*
 * FUNCTION: bidi_output()
 *	     This function copies the infield to output buffer, outfield, 
 *	     either by appending data to the end of the buffer when the 
 *	     direction (dir) is 1 or by inserting data to the beginning 
 *	     of the buffer when the direction (dir) is 0.
 *
 * PARAMETERS:
 *	    char *infield - The character string to be copied into the 
 *			    output buffer outfield.
 *	    char *infield - The output buffer.
 *	    int dir - When dir is 1, infield is appended to the end of 
 *		      the output buffer. 
 *	       	      When dir is 0, infield is inserted in front of the
 *		      output buffer.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	    void.
 */


void bidi_output (char *infield, char **outfield, int dir)

{
	int field_width;
	int i;
	char *out = *outfield;
	
	if ( !(*infield))
		return;
	field_width = strlen (infield);
	if (dir)			/* output from left to right */
		for (i = field_width ; i > 0; i--)
			*out++ = *infield++;
	else {				/* output from right to left */
		infield += (field_width - 1);
		for (i = field_width; i > 0; i--)
			*out-- = *infield--;
	}
	*outfield = out;
}
