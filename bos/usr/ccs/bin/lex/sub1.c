static char sccsid[] = "@(#)17	1.12.1.14  src/bos/usr/ccs/bin/lex/sub1.c, cmdlang, bos411, 9428A410j 6/10/94 12:24:41";
/**
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS:
 *      getl space digit error warning indexx alpha printable lgate scopy siconv
 *      slength scomp ctrans cclinter isnewccl addcclinter addwcclinter addwmatch
 *      addxccl usescape lookup cpyact gch munput dupl allprint strpt treedump
 *      getcollist prtcollist getcharclass getrange getcolsym existscollsym
 *      getcollset invcollist collsymtree compare invccl
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/*static char rcsid[] = "RCSfile: sub1.c,v Revision: 2.6  (OSF) Date: 9
0/10/07 17:42:30 ";*/

        /* Multi-byte support added by Michael S. Flegel, July 1991 */
           
# include "ldefs.h"

extern int asciipath;


/* --------------------------- getl --------------------------- */

/*
 *    1. Gets next line of input, throwing away trailing
 *       '\n'.
 *
 * inputs -  p      : pointer to buffer in which to store the
 *                    input line
 * outputs - 0      : return 0 if at eof
 *           s      : return a pointer to the buffer
 */

wchar_t *
getl(p)
  wchar_t *p;
        {
        register int c;
        wchar_t *s, *t;
        t = s = p;
        while(((c = gch()) != 0) && c != '\n')
                *t++ = c;
        *t = 0;
        if(c == 0 && s == t)
            return((wchar_t *)NULL);

        prev = '\n';
        pres = '\n';
        return(s);
        }

/* --------------------------- space --------------------------- */

space(c)
{
    return(iswspace(c));
}

/* --------------------------- digit --------------------------- */

digit(c)
{
    return(iswdigit(c));
}

/* --------------------------- error --------------------------- */

/*
 *    1. Prints error messages.
 *
 * inputs -  s      : error message string to be printed if the
 *                    message catalog cannot be opened
 *           p, d   : optional parameters for fprintf
 * outputs - none
 * globals - none
 */

error(s,p,d)
char *s;
        {
        if(!eof)fprintf(errorf,"%d: ",yyline);
        fprintf(errorf,MSGSTR(ERR, "(Error) "));
        fprintf(errorf,s,p,d);
        putwc('\n',errorf);
# ifdef DEBUG
        if(debug && sect != ENDSECTION) {
                sect1dump();
                sect2dump();
        }
# endif
        if(
# ifdef DEBUG
                debug ||
# endif
                report == 1) statistics();
        exit(1);        /* error return code */
        }

/* -------------------------- warning -------------------------- */

/*
 *    1. Prints warning messages.
 *
 * inputs -  s      : warning message string to be printed if the
 *                    message catalog cannot be opened
 *           p, d   : optional parameters for fprintf
 * outputs - none 
 * globals - none
 */

warning(s,p,d)
char *s;
        {
        if(!eof)fprintf(errorf,"%d: ",yyline);
        fprintf(errorf,MSGSTR(WARN, "(Warning) "));
        fprintf(errorf,s,p,d);
        putwc('\n',errorf);
        fflush(errorf);
        if (fout) fflush(fout);
        fflush(stdout);
        }

/* -------------------------- indexx -------------------------- */

/*
 *    1. Gets the index of the given element a in the array
 *       s[].
 *
 * inputs -  a      : element to search for in s[]
 *           s      : array to be searched
 * outputs -        : index of the given element in the given array
 * globals - none
 */

indexx(a,s)
        wchar_t a;
        wchar_t *s;
{
        register int k;
        for(k=0; s[k]; k++)
                if (s[k]== a)
                        return(k);
        return(-1);
        }

/* -------------------------- alpha -------------------------- */

/*
 * Determine if c belongs to the alphabet of the current locale.  It is assumed
 * that the alpha class may contain some, or all, multibyte extended characters
 * for the current locale.
 */

alpha(c)
wint_t c; 
{
        return(iswalpha(c));
}

/* ------------------------ printable ------------------------ */

printable(c)
wint_t c;
{
        return(iswprint(c));
}

/* --------------------------- lgate --------------------------- */

/*
 *    1. Opens the output file lex.yy.c, unless stdout is being
 *       used (-t option specified).
 *    2. Prints header code by calling phead().
 *
 * inputs -  nont
 * outputs - none
 * globals - fout   : pointer to ouput file descriptor
 */

lgate()
{
        char fname[20];

        if (lgatflg) return;
        lgatflg=1;
        if(fout == NULL){
                sprintf(fname, "lex.yy.%c", cplusplus ? 'C' :
                                (ratfor ? 'r' : 'c'));
                fout = fopen(fname, "w");
                }
        if(fout == NULL) error(MSGSTR(NOPEN, "Can't open %s"),fname);
        if(ratfor) fprintf( fout, "#\n");
        phead1();
        }

/* --------------------------- scopy --------------------------- */

/*
 *    1. Copies the string pointed to by s to the string
 *       pointed to by t.
 */

scopy(s,t)
wchar_t *s, *t;
{
wchar_t *i;

    i = t;
    while(*i++ = *s++);                         /* Empty */
    return;
}

/* -------------------------- siconv -------------------------- */

/*
 *    1. Converts a numerical string into an integer.
 *
 * inputs -  t      : pointer to numerical string
 * outputs -        : integer corresponding to the numerical
 *                    value in the string
 * globals - none
 */

siconv(t)
wchar_t         *t;
{
register int    i,sw;
wchar_t         *s;

    s = t;
    while(!(('0' <= *s && *s <= '9') || *s == '-') && *s)
        s++;
    sw = 0;
    if(*s=='-')                                 /* neg */
    {
        sw = 1;
        s++;
    }
    i = 0;
    while('0' <= *s && *s <= '9')
        i = i * 10 + (*(s++)-'0');
    return(sw ? -i : i);
}

/* -------------------------- slength -------------------------- */

/*
 *    1. Returns the integer length of the given string s
 *       excluding the '\0' terminator.
 */

slength(s)
wchar_t         *s;
{
wchar_t *t;

    t = s;
    return(wcslen(t));
}

/* --------------------------- scomp --------------------------- */

/*
 *    1. Compares two wide character strings and returns:
 *                -1  if  x < y
 *                 0  if  x == y
 *                 1  if  x > y
 */

scomp(x,y)
  wchar_t *x,*y; {
      register wchar_t *a, *d;
      a = x;
      d = y;
      while (*a || *d)
      {
          if (*a > *d)
              return (1);                       /* greater */
          if (*a < *d)
              return (-1);                      /* less */
          a++;
          d++;
      }
      return (0);                               /* equal */
  }

/* --------------------------- ctrans --------------------------- */

/*
 *    1. Converts escape sequences found in the translation
 *       section of the lex input file.
 */

ctrans(ss)
wchar_t         **ss;
{
register int    k;
wchar_t         c;

    if ((c = **ss) != '\\')
        return((int)c);
    switch(c= *++*ss)
    {
    case 'n': c = '\n'; break;
    case 't': c = '\t'; break;
    case 'r': c = '\r'; break;
    case 'b': c = '\b'; break;
    case 'f': c = 014; break;                   /* form feed for ascii */
    case 'a': c = '\a'; break;
    case 'v': c = '\v'; break;
    case '\\': c = '\\'; break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        c -= '0';
        while((k = *(*ss+1)) >= '0' && k <= '7')
        {
            c = c*8 + k -'0';
            (*ss)++;
        }
        break;
    }
    return(c);
}

/* ------------------------- cclinter ------------------------- */

/*
 *    1. Checks first to see if the given ccl (in symbol[]) is
 *       already contained in the ccl intersection set by calling
 *       isnewccl().  Returns if isnewccl() returns a 0.
 *    2. Assigns all new characters to a new ccl number (ie.
 *       characters that do not as yet belong to any other ccl,
 *       cindex[<character>] == 0) and turns off those characters
 *       in symbol[].
 *    3. Calculates the ccl intersection if necessary as follows:
 *       - For each character left in symbol[] after step (2),
 *         get its current character class and change the ccl # for
 *         all characters in that ccl except those for whom
 *         symbol[<character>] is true.
 *       - The process can be visualized as follows:
 *
 *                  Given 3 ccls : abc, efg, cde
 *
 *                  cindex ->    a   b   c   d   e   f   g
 *                   
 *                  at start     0   0   0   0   0   0   0
 *                  1st ccl      1   1   1   0   0   0   0
 *                  2nd ccl      1   1   1   0   2   2   2
 *              3rd ccl, step 2  1   1   1   3   2   2   2
 *         1st iteration step 3  4   4   1   3   2   2   2
 *         2nd iteration step 3  4   4   1   3   2   5   5
 *
 * inputs -  ccltype: ccl type for special ccls, 1 for ".", 0 if
 *                     none
 *           t      : parse tree node number for the ccl
 * outputs - none
 * globals - ccount : keeps track of the next available ccl number,
 *                    after the above example ccount would be 6
 *           symbol : array of flags, one element for each 8 bit
 *                    character, to indicate which characters belong
 *                    to the current ccl
 *           wsymbol: list of wide characters belonging to the current
 *                    ccl
 *           cindex : array, one element for each 8 bit character,
 *                    indicates to which ccl each character belongs,
 *                    0 if it belongs to none
 *           wmatch : hash table which keeps track of information
 *                    associated with each wide character seen
 *                    including the associated ccl number
 */

cclinter(ccltype,t)
  int           ccltype;                        /* special ccl */
  int           t;                              /* associated transition tree index */
{
register int    i, j, k;
int             m, n;
wchar_t         *p;

    /*
     * if the CCL is wholly contained, then nothing to do
     */
    if (!isnewccl(ccltype))
        return;
    /*
     * assign all new and unique characters to a new CCL
     */
    m = 0;
    k = 0;
    for(i=1;i<NCH;i++)                          /* add the 8-bit chars */
    {
        if(symbol[i])                           /* char in CCL */
        {
            if(!cindex[i])                      /* CCL for char not defined */
            {
                cindex[i] = ccount;             /* assign CCL for char */
                symbol[i] = 0;                  /* turn off char */
                m = 1;
            }
            else                                /* CCL for char defined => CCL intersection */
                k = 1;
        }
    }
    if (ccltype)                                /* add the special CCLs */
    {
        if (i = addxccl (ccount, ccltype))      /* record the associated tree index */
        {                                       /* 35368 (MF) - assign 'm' IFF xccl added */
            m = 1;
           /*
            * 89312 - xccl is now associated with a node
            *         following line is no longer needed
            * xccl[i-1].verify = t;
            */
        }
    }
    /*
     * Defect 89312 - keep multi-byte and single byte character
     *                classes separate
     */
    if(m)
    {
        ccount++;
        m = 0;
    }
    for (i = 0; i < wsymboli; i++)              /* add the MB chars */
    {
        j = hashfind (wsymbol[i], wmatch);
        if (!PMATCH(j))                         /* CCL for char not defined */
        {
            addwmatch(ccount,wsymbol[i]);       /* assign the CCL for the char */
            wsymbol[i] = 0;                     /* turn off the char */
            m = 1;
        }
        else if (PMATCH(j)->cindex != ccount)   /* CCL for MB defined => CCL intersection */
            k = 1;
    }
    /*
     * what was done ?
     */
    if(m)                                       /* m == 1 implies last value of */
        ccount++;                               /* ... ccount has been used */

    if(k == 0)                                  /* is now in as ccount wholly */
        return;                                 /* ...ie no CCL intersection */
    /*
     * intersection must be computed; each intersecting character belonging to
     * a particular CCL will be assigned a new CCL; 
     * symbol and wsymbol contains only chars which have not been recorded in CCL
     */
    for(i=1;i<NCH;i++)                          /* compute the 8-bit char intersections */
    {
        if(symbol[i])                           /* char in CCL intersection */
        {
            n = cindex[i];                      /* will be non-zero */
            m = addcclinter (n, ccount);
            m += addwcclinter (n, ccount);

            if(m)                               /* CCL intersection created */
                ccount++;
        }
    }
    for (i = 0; i < wsymboli; i++)              /* compute the MB char intersections */
    {
        if (wsymbol[i])
        {
            n = PMATCH(hashfind (wsymbol[i], wmatch))->cindex; /* will be non-zero */
            m = addcclinter (n, ccount);
            m += addwcclinter (n, ccount);
            if (m)                              /* CCL intersection created */
                ccount++;
        }
    }

    return;
}

/* ------------------------- isnewccl ------------------------- */

/*
 *    1. Determines first if there are any characters in the
 *       given ccl (could be null) and if not returns 0.
 *    2. Gets the ccl number of the first character in the
 *       ccl from cindex[].
 *    3. Looks through symbol[] (wsymbol[]) and cindex[] (wmatch[])
 *       to determine if any of the characters in symbol[] belong
 *       to a different ccl than the one in step (2) or are not
 *       yet assigned to a ccl, or if there are any characters
 *       belonging to the ccl from (2) that are not in symbol[].
 *
 * inputs -  ccltype: ccl type for special ccls, 0 if none
 * outputs - 0      : if the given ccl in symbol[] is identical
 *                    to a ccl already entered in cindex[]
 *           1      : if the given ccl is unique
 * globals   symbol : array of flags, one element for each 8 bit
 *                    character, to indicate which characters belong
 *                    to the current ccl
 *           wsymbol: list of wide characters belonging to the current
 *                    ccl
 *           cindex : array, one element for each 8 bit character,
 *                    indicates to which ccl each character belongs,
 *                    0 if it belongs to none
 *           wmatch : hash table which keeps track of information
 *                    associated with each wide character seen
 *                    including the associated ccl number
 */

int
isnewccl (ccltype)
  int           ccltype;
{
register int    i, j, k;
int             n;

    /*
     * Make sure there are chars in the CCL, remember the first char found
     */
    for(i=1;i<NCH;i++)
    {
        if (symbol[i])
            break;
    }                   
    if (   (i >= NCH)                           /* no 8-bits */
        && (wsymboli == 0)                      /* no MBs */
        && ((ccltype == 0) || (wmatchsize == 0))) /* no CCLs or MBs not allowed */
    {
        return (0);
    }
    /*
     * check 8-bit characters
     */
    n = 0;
    if (i < NCH)                                /* an 8-bit char was found */
        n = cindex[i];
    if(n)                                       /* if it has been defined */
    {
        for(j=1;j<NCH;j++)
        {
            if (   ( symbol[j] && cindex[j] != n)       /* new char in other CCL */
                || (!symbol[j] && cindex[j] == n))      /* CCL contains char from another CCL */
            {
                return (1);
            }
        }
        if (   (wsymboli == 0)                  /* no MBs */
            && ((ccltype == 0) || (wmatchsize == 0))) /* no CCLs or MBs not allowed */
            return (0);
    }
    /*
     * the 8-bits are equivalent, now check the MBs
     */
    if (n == 0 && wsymboli)                                 /* need a CCL number */
    {
        i = hashfind (wsymbol[0], wmatch);
        n = (i==0) ? 0 : PMATCH(i)->cindex;
    }
    if (n)                                      /* it's there */
    {
        for (k = 0; k < wsymboli; k++)
        {
            j = hashfind (wsymbol[k], wmatch);
            if (PMATCH(j) && (PMATCH(j)->cindex != n)) /* new char in other CCL */
                return (1);
        }
        for (k = wmatchlist; PMATCH(k); k = PMATCH(k)->list)
        {                                       /* CCL contains a char from another CCL */
            if (!(wcschr (wsymbol, wmatch->table[k].id)) && (PMATCH(k)->cindex == n))
                return (1);
        }
        if (ccltype == 0)                       /* no CCLs */
            return (0);
    }
    /*
     * 8-bits and MBs are equivalent, now check the ccltypes
     */
    if (n == 0)
    {
        for (i = 0; i < xccltop; i++)
            if (xccl[i].type == ccltype)
                break;
        n = (i < xccltop) ? xccl[i].cindex : 0;
    }
    /* 89312 - if xccltop empty then we have new ccl */
    if (n && xccltop)
    {
        for (i = 0; i < xccltop; i++)
        {
            if (   ((xccl[i].type == ccltype) && (xccl[i].cindex != n))
                || ((xccl[i].type != ccltype) && (xccl[i].cindex == n)))
            {
                return (1);
            }
        }
        return (0);
    }
    return (1);                                 /* n==0, so its a new xccl */
}

/* ------------------------ addcclinter ------------------------ */

/*
 *    1. Called by cclinter to add a ccl intersection for a
 *       given 8 bit character in symbol[] (performs one
 *       iteration of step (3) in cclinter).
 *    2. Assigns characters belonging to the given ccl but
 *       which are not active in symbol[] to a new ccl
 *       number.
 *    3. Turns off the characters in symbol[] which belong to
 *       the given ccl.
 *
 * inputs -  n      : ccl number
 *           ccl    : next available ccl number
 * outputs - 0      : if no characters had their ccl number changed
 *           1      : if the ccl number was changed for 1 or more
 *                    characters
 * globals - symbol : array of flags, one element for each 8 bit
 *                    character, to indicate which characters belong
 *                    to the current ccl
 *           cindex : array, one element for each 8 bit character,
 *                    indicates to which ccl each character belongs,
 *                    0 if it belongs to none
 */

int
addcclinter (n, ccl)
  int           n, ccl;
{
register int    i;
int             r;

    r = 0;
    for(i = 1; i < NCH; i++)                    /* for each 8-bit char in current CCL */
    {
        if (cindex[i] == n)
        {
            if(symbol[i])                       /* char already recorded in CCL */
                symbol[i] = 0;
            else                                /* else record it into the new CCL */
            {
                cindex[i] = ccl;
                r = 1;
            }
        }
    }
    return (r);
}

/* ------------------------ addwcclinter ------------------------ */

/*
 *    1. Same as addcclinter for wide characters.
 *           symbol[] -> wsymbol[]
 *           cindex[] -> wmatch[]
 */

int
addwcclinter (n, ccl)
  int           n, ccl;
{
register int    i, j;
int             r;

    r = 0;
    for (i = wmatchlist; PMATCH(i); i = PMATCH(i)->list)
    {
        if (PMATCH(i)->cindex == n)
        {                                       /* see if the MB is in wsymbol */
            for (j = 0; (j < wsymboli) && (wmatch->table[i].id != wsymbol[j]); j++) 
                ; /* Empty */
            if (j < wsymboli)                   /* char already recorded in CCL */
                wsymbol[j] = 0;
            else                                /* else record it into the new CCL */
            {
                PMATCH(i)->cindex = ccl;
                r = 1;
            }
        }
    }
    return (r);
}

/* ------------------------- addwmatch ------------------------- */

/*
 *    1. Adds a new wide character to the wmatch[] table along
 *       with its associated ccl.
 *
 * inputs -  ccl    : ccl number for the character
 *           c      : wide character to be added
 * outputs - none
 * globals - wmatch : hash table which keeps track of information
 *                    associated with each wide character seen, such
 *                    as its ccl number
 */

int
addwmatch (ccl, c)
  int           ccl;
  wchar_t       c;
{
int             n;

    if ((n = hashnew (c, wmatch)) == 0)
    {
        error(MSGSTR(EMAXMBM, "Not enough multi-byte character class character output slots %s"),
              MSGSTR(EMAXMBM1, "\n\tTry using the following: %%m Number"));
    }

    if (PMATCH(n) == 0)
    {
        wmatch->table[n].info = (void *)myalloc(1,sizeof(match_t));
        if (wmatch->table[n].info == 0)
            error(MSGSTR(CALLOCFAILED,"There is not enough memory to create a hash entry."));
    }

    PMATCH(n)->cindex = ccl;
    PMATCH(n)->cmatch = c;
    PMATCH(n)->list = wmatchlist;
    wmatchlist = n;
}

/* -------------------------- addxccl -------------------------- */

/*
 *    1. Adds a ccl type to the ccl table and its associated
 *       ccl number.
 *
 * inputs -  ccount : current ccl number
 *           ccltype: ccl type
 * outputs -        : index of the next available slot in the
 *                    ccl table
 * globals - xccl   : table of information identifying special
 *                    ccls seen
 */

int
addxccl (ccount, ccltype)
  int   ccount, ccltype;
{
register int i;

    if (xcclsize == 0)
    {
        if(multibytecodeset && !noxmsg)
        {
            warning(MSGSTR(NOZ,". will not match multibyte characters unless %z is set greater than zero."));
            noxmsg++;
        }
        return (0);
    }

    if (xccltop+1 > xcclsize)
    {
        error(MSGSTR(EMAXMBCCL,"There are too many multi-byte character classes.%s"),
              MSGSTR(EMAXMBCCL1,"\n\tTry using the following: %z Number"));
    }

    xccl[xccltop].type = ccltype;
    xccl[xccltop].cindex = ccount;
    xccl[xccltop].verify = -1;
    xccl[xccltop].advance = -1;
    xccltop += 1;

    return (xccltop);
}

/* ------------------------- usescape ------------------------- */

/*
 *    1. Convert an escape sequence including octal and hex
 *       sequences to their character code equivalent.
 *
 * inputs -  c      : first character of the input stream after
 *                    the \
 * outputs -        : character code equivalent of the escape
 *                    sequence
 * globals - none
 */

wchar_t usescape(c)
  wchar_t c;
{
wchar_t d, e;
int     i, j;
char    mbs[32];

    switch(c)
    {
    case 'n': e = '\n'; break;
    case 'r': e = '\r'; break;
    case 't': e = '\t'; break;
    case 'b': e = '\b'; break;
    case 'f': e = 014; break;                   /* form feed for ascii */
    case 'a': e = '\a'; break;
    case 'v': e = '\v'; break;
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        d = c;
        for(i=0; i<mbcurmax; i++)
        {
            e=0;
            for(j=0; j<3; j++)
            {
                e = e * 8 + (d - '0');
                if (!ISOCTAL(peek))
                    break;
                d = gch();
            }
            if(mbcurmax > 1)
            {
                mbs[i] = e;
                mbs[i+1] = 0;
                if(mbtowc(&e, mbs, i+1) > 0)
                    break;
                if((d=gch()) != '\\' || i == (mbcurmax - 1))
                    error(MSGSTR(BADMBCHAR,"Invalid multi-byte character."));
                d = gch();
            }
        } /* end of for i */
        break;
    case 'x':
        if (!ISHEX(peek))
            e = c;
        else
        {
            for(i=0; i<mbcurmax; i++)
            {
                e=0;
                j=0;
                while(ISHEX(peek) && j<2)
                {
                    d=gch();
                    if (d >= '0'  &&  d <= '9')
                        e = e * 16 + (d - '0');
                    else if (d >= 'A'  &&  d <= 'F')
                        e = e * 16 + (d - 'A' + 10);
                    else
                        e = e * 16 + (d - 'a' + 10);
                    j++;
                }
                if (mbcurmax > 1)
                {
                    mbs[i] = e;
                    mbs[i+1] = 0;
                    if (mbtowc(&e, mbs, i+1) > 0)
                        break;
                    d = gch();
                    if (!(d == '\\'  &&  peek == 'x') || i == (mbcurmax - 1))
                        error(MSGSTR(BADMBCHAR,"Invalid multi-byte character."));
                    d = gch();
                }
            } /* end of for i */
        } /* end of else */
        break;          
    default:
        e = c;
    }
    return (e);
}

/* -------------------------- lookup -------------------------- */

/*
 *    1. Look for a given string in an array of strings.
 *    2. Called from parser.y to look for definitions
 *       and start condition names.
 *
 * inputs -  s      : string to search for
 *           t      : array to be searched
 * outputs -        : index in array of the located string,
 *                    -1 if not found
 * globals - none
 */

lookup(s,t)
  wchar_t *s;
  wchar_t **t; {
        register int i;
        i = 0;
        while(*t){
                if(scomp(s,*t) == 0)
                        return(i);
                i++;
                t++;
                }
        return(-1);
        }

/* -------------------------- cpyact -------------------------- */

/*
 *    1. Copies a C action to the next ; or closing }
 */

cpyact(int isexec){
        register int brac, c, mth;
        int savline, sw;

        brac = 0;
        sw = TRUE;

while(!eof){
        c = gch();
swt:
        switch( c ){

case '|':       if(brac == 0 && sw == TRUE){
                        if(peek == '|')gch();   /* eat up an extra '|' */
                        return(0);
                        }
                break;

case ';':
                if( brac == 0 ){
                        putwc(c,fout);
                        putwc('\n',fout);
                        return(1);
                        }
                break;

case '{':
                brac++;
                savline=yyline;
                break;

case '}':
                brac--;
                if( brac <= 0 ){
                        putwc(c,fout);
                        putwc('\n',fout);
                        return(1);
                        }
                break;

case '/':       /* look for comments */
                putwc(c,fout);
                c = gch();
                if( c != '*' ) goto swt;

                /* it really is a comment */

                putwc(c,fout);
                savline=yyline;
                while( c=gch() ){
                    /* following line changed for IX32392 Defect 74364 */
                    while( c=='*' ){
                        putwc(c,fout);
                        if( (c=gch()) == '/' ) {
                            if(isexec && !brac) {
                                putwc(c, fout);
                                putwc('\n', fout);
                                return(0);
                             }                   
                             goto loop;
                        }
                    }
                    putwc(c,fout);
                }
                yyline=savline;
                error(MSGSTR(EOFCOM, "EOF inside comment" ));

case '\'':      /* character constant */
                mth = '\'';
                goto string;

case '"':       /* character string */
                mth = '"';

        string:  /*@@*/

                putwc(c,fout);

                while( c=gch() ){
                        if( c=='\\' ){
                                putwc(c,fout);
                                c=gch();
                                }
                        else if( c==mth ) goto loop;
                        putwc(c,fout);
                        if (c == '\n')
                                {
                                yyline--;
                                error(MSGSTR(ENDLESSTR,
                                "Non-terminated string or character constant"));
                                }
                        }

                error(MSGSTR(EOFSTR, "EOF in string or character constant" ));

case '\0':
                yyline = savline;
                error(MSGSTR(NONTERM, "Action does not terminate"));
default:
                break;          /* usual character */
                }
loop:
        if(c != ' ' && c != '\t' && c != '\n' && c != '\v' && c != '\f') sw = FALSE;
        putwc(c,fout);
        if(c == '\n' && sw && isexec) return(0);
        }
error(MSGSTR(PEOF, "Premature EOF"));
}

/* --------------------------- gch --------------------------- */

/*
 *    1. Stores the current character in prev, the peek
 *       character in pres and gets the next character from
 *       the input stream and stores it in peek.
 *    2. Returns the character in pres.
 *    3. Handles closing of the input file if eof is reached.
 *
 * inputs -  none
 * outputs -        : the current character
 * globals - prev   : the previous character
 *           pres   : the current character
 *           peek   : the lookahead character
 *           pushc  : input buffer
 *           pushptr: pointer to the next character in pushc
 */

gch()
{
register int c;
extern int errno;

    prev = pres;
    c = pres = peek;
    if (pushptr > pushc)
        peek = (*--pushptr);
    else {
        /*D42832 - BADCHAR message generated if errno previous non-zero
        before the call to getwc().  Typically, this occurred when the
        locale was not installed or LANG set incorrectly; errno was set to
        non-zero when the call to setlocale() failed and was never reset.*/
        errno = 0;
        peek = getwc(fin);
    }

    if ((peek == WEOF) && (errno != 0))
        error(MSGSTR(BADCHAR,"Invalid character")); /* 33630(MF) - used BADCHAR, not errno */

    if(peek == WEOF && sargc > 1)
    {
        fclose(fin);
        fin = fopen(sargv[++fptr],"r");
        if(fin == NULL)
            error(MSGSTR(NOPEN2, "Cannot open file %s"),sargv[fptr]);
        peek = getwc(fin);
        sargc--;
    }
    if(c == WEOF)
    {
        eof = TRUE;
        fclose(fin);
        return(0);
    }
    if(c == '\n')yyline++;
    return(c);
}

/* --------------------------- mn2 --------------------------- */

/*
 *    1. Create a new node of type a with left child d
 *       and right child c.  Used for compound rule nodes
 *       such as RSTR, BAR, RNEWE, RCAT, DIV, RSCON.
 *    2. Sets the parent[] of the child nodes to the new
 *       node number.
 *    3. Sets nullstr[] for the new node if applicable.
 *    4. Called by the parser when building the parse tree.
 *
 * inputs -  a      : type of node to be created
 *           d      : left child node number
 *           c      : right child node number or for RSTR nodes,
 *                    the character associated with the node
 * outputs -        : node number of the new node
 * globals -        : parse tree
 *           tptr   : next available node number
 */

mn2(a,d,c)
  int   a;                                      /* token type */
  int   d;                                      /* previous (left) token index */
  int   c;                                      /* token being added */
{
    name[tptr] = a;
    wname[tptr] = 0;
    left[tptr] = d;
    right[tptr] = c;
    parent[tptr] = 0;
    nullstr[tptr] = 0;

    switch(a)
    {
    case RCCL:
    case RNCCL:
        if(slength(d) == 0)
            nullstr[tptr] = TRUE;
        break;
    case RSTR:                                  /* RSTRs are built downwards? */
        parent[d] = tptr;
        if (c>=NCH)                             /* wide character */
        {
            right[tptr] = RWCHAR;               /* tokenize it */
            wname[tptr] = c;                    /* remember it */
        }
        break;
    case BAR:
    case RNEWE:
        if(nullstr[d] || nullstr[c])
            nullstr[tptr] = TRUE;
        parent[d] = parent[c] = tptr;
        break;
    case RCAT:
    case DIV:
        if(nullstr[d] && nullstr[c])
            nullstr[tptr] = TRUE;
        parent[d] = parent[c] = tptr;
        break;
    case RSCON:
        parent[d] = tptr;
        nullstr[tptr] = nullstr[d];
        break;
# ifdef DEBUG
    default:
        warning("bad switch mn2 %d %d",a,d);
        break;
# endif
    }
    if(tptr > treesize)
        error(MSGSTR(BIGPAR, "Parse tree too big %s"),
              ((treesize == TREESIZE)?MSGSTR(BIGPAR2, "\nTry using %%e num"):""));
    return(tptr++);
}

/* --------------------------- mn1 --------------------------- */

/*
 *    1. Create a new node of type a with left child d
 *       and right child null.  Used for complex rule
 *       nodes such as RCCL, STAR, QUEST, PLUS, CARAT, FINAL.
 *    2. Sets the parent[] of the child node to the new
 *       node number.
 *    3. Sets nullstr[] for the new node if applicable.
 *    4. Called by the parser when building the parse tree.
 *
 * inputs -  a      : type of node to be created
 *           d      : left child node number
 * outputs -        : node number of the new node
 * globals -        : parse tree
 *           tptr   : next available node number
 */

mn1(a,d)
int             a;                              /* node type */
int             d;                              /* left node information */
{
    name[tptr] = a;
    wname[tptr] = 0;
    left[tptr] = d;
    parent[tptr] = 0;
    nullstr[tptr] = 0;

    switch(a)
    {
    case STAR:
    case QUEST:
        nullstr[tptr] = TRUE;
        parent[d] = tptr;
        break;
    case PLUS:
    case CARAT:
        nullstr[tptr] = nullstr[d];
        parent[d] = tptr;
        break;
    case S2FINAL:
        nullstr[tptr] = TRUE;
        break;
# ifdef DEBUG
    case FINAL:
    case S1FINAL:
        break;
    default:
        warning("bad switch mn1 %d %d",a,d);
        break;
# endif
    }
    if(tptr > treesize)
        error(MSGSTR(BIGPAR, "Parse tree too big %s"),
              (treesize == TREESIZE ? MSGSTR(BIGPAR2, "\nTry using %%e num") : ""));
    return(tptr++);
}

/* --------------------------- mn0 --------------------------- */

/*
 *    1. If a is an 8 bit character, create a new node
 *       of type a, otherwise create a new node of type
 *       RWCHAR and set wname[] to a.
 *    2. Sets nullstr[] for the new node if applicable.
 *    3. Called by the parser when building the parse tree.
 *
 * inputs -  a      : character associated with the node
 *           c      : wide character flag
 * outputs -        : node number of the new node
 * globals -        : parse tree
 *           tptr   : next available node number
 */

mn0(a,c)
  int a;                                        /* character token */
  int c;                                        /* wide character flag */
        {
        name[tptr] = a;
        wname[tptr] = 0;
        parent[tptr] = 0;
        nullstr[tptr] = 0;
        if ((c==RWCHAR)&&(a >= NCH))            /* 'a' is a wide character */
        {
            name[tptr] = RWCHAR;                /* tokenize it as wide */
            wname[tptr] = a;                    /* remember the character */
        }
        else if (a >= NCH)                      /* else its a defined token */
            switch(a)
            {
            case RNULLS: nullstr[tptr] = TRUE; break;
# ifdef DEBUG
            default:
                warning("bad switch mn0 %d",a);
                break;
# endif
            }
        if(tptr > treesize)
            error(MSGSTR(BIGPAR, "Parse tree too big %s"),
                  ((treesize == TREESIZE) ? MSGSTR(BIGPAR2, "\nTry using %%e num"):""));
        return(tptr++);
    }

/* -------------------------- munput -------------------------- */

/*
 *    1. Returns the current peek character to the input
 *       stream and the current character is placed in peek.
 *                          OR
 *       Returns an entire string to the input stream.
 *
 * inputs -  t      : indicates whether to return a character 'c'
 *                    or a string 's'
 *           p      : pointer to the character or string
 * outputs - none
 * globals - pushc  : input buffer
 *           pushptr: pointer to the current position in pushc
 *           peek   : current lookahead character
 */

/*
 * implementation dependent
 */

munput(t,p)
wchar_t         *p;
int             t;
{
register int    i,j;

    if(t == 'c')
    {
        *pushptr++ = peek;                      /* watch out for this */
        peek = (int)p;
    }
    else if(t == 's')
    {
        *pushptr++ = peek;
        peek = p[0];
        i = slength(p);
        for(j = i-1; j>=1; j--)
            *pushptr++ = p[j];
    }
# ifdef DEBUG
    else error("Unrecognized munput option %c",t);
# endif
    if(pushptr >= pushc+TOKENSIZE)
        error(MSGSTR(CHARPUSH, "Too many characters pushed"));
    return;
}

/* --------------------------- dupl --------------------------- */

/*
 *    1. Duplicate the subtree whose root is n and return
 *       a pointer to it.  Called by the parser in the
 *       case of a fixed iteration (ie. a{1,3}).
 *
 * inputs -  n      : node number of the root of the subtree
 *                    to be duplicated
 * outputs -        : node number of the duplicate tree
 * globals -        : parse tree
 */

dupl(n)
  int n;
{
        register int i;
        i = name[n];
        if(i < NCH)                             /* 8-bit character */
            return(mn0(i,0));
        else if (i == RWCHAR)                   /* wide character */
            return(mn0(wname[n],RWCHAR));

        switch(i)
        {
        case RNULLS:
            return(mn0(i,0));
        case RCCL: case RNCCL:
            return(mn2(i,left[n],right[n]));
        case FINAL: case S1FINAL: case S2FINAL:
            return(mn1(i,left[n]));
        case STAR: case QUEST: case PLUS: case CARAT:
            return(mn1(i,dupl(left[n])));
        case RSTR: case RSCON:
            return(mn2(i,dupl(left[n]),((right[n]!=RWCHAR)?right[n]:wname[n])));
        case BAR: case RNEWE: case RCAT: case DIV:
            return(mn2(i,dupl(left[n]),dupl(((right[n]!=RWCHAR)?right[n]:wname[n]))));
# ifdef DEBUG
        default:
            warning("bad switch dupl %d",n);
# endif
        }
        return(0);
}

# ifdef DEBUG
/* ------------------------- allprint ------------------------- */

/*
 *    1. Debug code - print a character.
 */

allprint(c)
  wchar_t c;
{
    switch(c)
    {
    case 014:
        printf("\\f");
        charc++;
        break;
    case '\n':
        printf("\\n");
        charc++;
        break;
    case '\t':
        printf("\\t");
        charc++;
        break;
    case '\b':
        printf("\\b");
        charc++;
        break;
    case ' ':
        printf("\\\bb");
        break;
    default:
        if(!printable(c))
        {
            if (c<=NCH)
            {
                printf("\\%-3d",c);
                charc += 3;
            }
            else
            {
                printf("\\%-6d",c);
                charc += 6;
            }
        }
        else 
        {
            putwchar(c);
            charc += wcwidth (c);
        }
        break;
    }
    charc++;
    return;
}

/* --------------------------- strpt --------------------------- */

/*
 *    1. Debug code - print a string.
 */

strpt(s)
wchar_t         *s;
{
charc = 0;

    while(*s)
    {
        allprint(*s++);
        if(charc > LINESIZE)
        {
            charc = 0;
            printf("\n\t");
        }
    }
    return;
}

/* ------------------------- sect1dump ------------------------- */

/*
 *    1. Debug code - print the information gathered from the
 *       definitions section if there is any such as definitions,
 *       start condition names and character translation sets.
 */

sect1dump(){
        register int i;
        printf("Sect 1:\n");
        if(def[0]){
                printf("str     trans\n");
                i = -1;
                while(def[++i])
                        printf("%S\t%S\n",def[i],subs[i]);
                }
        if(sname[0]){
                printf("start names\n");
                i = -1;
                while(sname[++i])
                        printf("%S\n",sname[i]);
                }
        if(chset == TRUE){
                printf("char set changed\n");
                for(i=1;i<NCH;i++){
                        if(i != ctable[i]){
                                allprint(i);
                                putchar(' ');
                                printable(ctable[i]) ? putchar(ctable[i]) :
                                        printf("%d",ctable[i]);
                                putchar('\n');
                                }
                        }
                }
        }

/* ------------------------- sect2dump --------------------------- */

/*
 *    1. Debug code - print the final parse tree by calling
 *       treedump().
 */

sect2dump(){
        printf("Sect 2:\n");
        treedump();
        }

/* ------------------------- treedump ------------------------- */

/*
 *    1. Debug code - print the parse tree.
 */

treedump()
{
        register int t;
        register unsigned char *p;
        printf("treedump %d nodes:\n",tptr);
        for(t=0;t<tptr;t++)
        {
                printf("%4d ",t);
                parent[t] ? printf("p=%4d",parent[t]) : printf("      ");
                printf("  ");
                if(name[t] < NCH) 
                {
                    printf ("char ");
                    allprint(name[t]);
                }
                else if (name[t] == RWCHAR)
                {
                    printf ("rchar ");
                    allprint(wname[t]);
                }
                else
                {
                    switch(name[t])
                    {
                    case RSTR:
                        printf("%d ",left[t]);
                        if (right[t]<NCH)       /* 8-bit character */
                            allprint(right[t]);
                        else                    /* wide character */
                            allprint(wname[t]);
                        break;
                    case RCCL:
                        printf("ccl [:%d:] ", right[t]);
                        strpt(left[t]);
                        break;
                    case RNCCL:
                        printf("nccl [:%d:] ", right[t]);
                        strpt(left[t]);
                        break;
                    case DIV:
                        printf("/ %d %d",left[t],right[t]);
                        break;
                    case BAR:
                        printf("| %d %d",left[t],right[t]);
                        break;
                    case RCAT:
                        printf("cat %d %d",left[t],right[t]);
                        break;
                    case PLUS:
                        printf("+ %d",left[t]);
                        break;
                    case STAR:
                        printf("* %d",left[t]);
                        break;
                    case CARAT:
                        printf("^ %d",left[t]);
                        break;
                    case QUEST:
                        printf("? %d",left[t]);
                        break;
                    case RNULLS:
                        printf("nullstring");
                        break;
                    case FINAL:
                        printf("final %d",left[t]);
                        break;
                    case S1FINAL:
                        printf("s1final %d",left[t]);   
                        break;
                    case S2FINAL:
                        printf("s2final %d",left[t]);
                        break;
                    case RNEWE:
                        printf("new %d %d",left[t],right[t]);
                        break;
                    case RSCON:
                        p = (unsigned char *)right[t];
                        printf("start %S",sname[*p++-1]);
                        while(*p)
                            printf(", %S",sname[*p++-1]);
                        printf(" %d",left[t]);
                        break;
                    default:
                        printf("unknown %d %d %d",name[t],left[t],
                               right[t]);
                        break;
                    }
        
                    if(nullstr[t])
                        printf("\t(null poss.)");
                }
                putchar('\n');
            }
    }

# endif

/* ------------------------ getcollist ------------------------ */

/*
 * getcollist - allocate space for a list of multi-character collating
 *              elements.
 *            - this is done only if there are multi-character collating
 *              elements since most locales do not have them.
 * inputs     - addresses of the list pointers
 * outputs    - none
 *
 */

void getcollist(wchar_t **list, int *listi, int size)
{
    *list = (wchar_t *)myalloc(size, sizeof(**list));
    if (!*list)
        error(MSGSTR(CALLOCFAILED,"Insufficient memory for multi-character collating elements."));
    *listi = 1;
    list[0][0] = 0;
}

            
#ifdef DEBUG

/* ------------------------ prtcollist ------------------------ */

/*
 * prtcollist - print the members of a multi-character collating
 *              element list
 * inputs     - pointer to a multi-character collating element list
 * outputs    - none
 * globals    - none
 *
 */

void prtcollist(wchar_t *collist)
{
    int i, len = 0;
    wchar_t *collistp;

    if (!collist)
        return;

    collistp = &collist[1];

    for(i=0; i<collist[0]; i++)
    {
        len = wcslen(collistp) + 1;
        printf("%S\n", collistp);
        collistp += len;
    }
}

#endif

/* ----------------------- getcharclass ----------------------- */

/*
 * getcharclass - get all the characters in a specified character class.
 *              - the class may be a known POSIX class such as 'alpha'
 *                or a user defined class
 * inputs       - name of class
 * outputs      - none
 * globals      - symbol and wsymbol (mccollist is not used since character
 *                classes can not include multi-character collating elements).
 */

void getcharclass(char *classname)
{
    wctype_t wctype;
    wchar_t wc;

    if ((wctype = get_wctype(classname)) == (wctype_t)(-1))
        error(MSGSTR(ECLASSNAME,"Invalid character class name."));
    for (wc = co_wc_min; wc < NCH; wc++)
    {
        if (is_wctype(wc, wctype))
            symbol[wc]=1;
    }
    for (wc = NCH; wc <= co_wc_max; wc++)
    {
        if (is_wctype(wc, wctype))
            addwsymbol(wc, 1);
    }
}

/* -------------------------- getrange -------------------------- */

/*
 * getrange     - get all the characters within a given range according to
 *                the collating weights of the characters, not their values.
 *              - if the minimum collating value and the maximum collating
 *                weight are equivalent and order is 0, then getrange will
 *                get all the characters belonging to an equivalence class.
 *                1-many mappings are supported for equivalence classes.
 * inputs       - minimum collating value, maximum collating value, order,
 *                weight string for 1-many mappings
 * outputs      - none
 * globals      - symbol, wsymbol and mccollist
 *
 */

void getrange(int min, int max, char order, char *wgt_str1)
{
    wchar_t wc;
    int colval;
    int i, len = 0;
    char *wgt_str2;

    if (!wgt_str1)
        if ((min < co_col_min) || (max > co_col_max) || (min > max))
            error(MSGSTR(EBRACKET,"Invalid bracket expression."));
    
    for (wc = co_wc_min; wc <= co_wc_max; wc++)
    {
        wgt_str2 = NULL;
	if (asciipath)
	    colval=(int)wc+co_col_min;
	else
        {
	    colval=COLLWGT(wc, order);
            if (wgt_str1 && EXISTS_WGT_STR(colval))
                wgt_str2 = WGT_STR(WGT_STR_INDEX(wc), order);
        } 
    
        if ((!wgt_str1 && (colval >= min) && (colval <= max)) || (wgt_str2 && !(strcmp(wgt_str1, wgt_str2))))
        {
            if (wc < NCH)
                symbol[wc] = 1;
            else
                addwsymbol(wc, 1);
        }
        /* check for multi-character collating elements and store them
           in mccollist if their weights are within the range */
        if (!asciipath && EXISTSCOLLEL(wc))
        {
            i=0;
            while (CE_SYM(wc, i) != (uchar *)NULL)
            {
                wgt_str2 = NULL;
	        colval=CE_WGT(wc, i, order);
                if (wgt_str1 && EXISTS_WGT_STR(colval))
                    wgt_str2 = WGT_STR(CE_WGT_STR_INDEX(wc, i), order);
                if ((!wgt_str1 && (colval >= min) && (colval <= max)) || (wgt_str2 && !(strcmp(wgt_str1, wgt_str2))))
                {
                    if (!mccollist)
                        getcollist(&mccollist, &mccollisti, mcsize);
                    len = strlen(CE_SYM(wc, i)) + 1;
                    mccollist[mccollisti] = wc;
                    /* the +2 in the next statement is for the first
                       character (wc) and the terminating null, the string
                       stored in CE_SYM does not contain the first character
                       of the multi-character collating element */
                    len = mbstowcs(&mccollist[mccollisti + 1], CE_SYM(wc, i), len) + 2;
                    if (!existscollsym(&mccollist[mccollisti]))
                    {
                        mccollisti += len;            
                        mccollist[0]++;
                    }
                }      /* end of if - colval */
                i++;
            }      /* end of while */
        }      /* end of if */
    }      /* end of for */
}

/* ------------------------- getcolsym ------------------------- */

/*
 * getcolsym - get the collating value for a collating symbol, order
 *             specifies which level of collating weight to return
 *           - if there is a 1-many mapping, return a pointer to
 *             the weight string in wgt_str
 * inputs    - collating name, collating order, weight string
 *             pointer for 1-many mappings
 * outputs   - returns the appropriate collating value
 * globals   - none
 *
 */

int getcolsym(wchar_t *collname, char order, char **wgt_str)
{

    int i = 0;
    int colval = 0;
    
    *wgt_str = NULL;

    if (!collname[1]) {
	if (asciipath)
	    colval=(int)collname[0]+co_col_min;
	else
        {
	    colval=COLLWGT(collname[0], order);
            if (EXISTS_WGT_STR(colval))
                *wgt_str = WGT_STR(WGT_STR_INDEX(collname[0]), order);
        }
    }
    else
    {
        if (!asciipath && EXISTSCOLLEL(collname[0]))
        {
            /* convert the collating element since it is stored in multi-byte
               form in CE_SYM, the first character of the element is not stored
               in CE_SYM */
            wcstombs(mbcollname, &collname[1], COLLNAME_SIZE*mbcurmax + 1);
            i = 0;
            while (CE_SYM(collname[0], i) != (uchar *)NULL)
            {
                if (!strcmp(mbcollname, CE_SYM(collname[0], i)))
                {
	            colval=CE_WGT(collname[0], i, order);
                    break;
                }
                i++;
            }   
            if (EXISTS_WGT_STR(colval))
                *wgt_str = WGT_STR(CE_WGT_STR_INDEX(collname[0], i), order);
        }
    }
    return(colval);
}
                    
/* ------------------------ existscollsym ------------------------ */

/*
 * existscollsym - determines if the given collating symbol is already
 *                 present in mccollist
 * inputs        - collating symbol name
 * outputs       - true (1) or false (0)
 * globals       - mccollist
 *
 */

int existscollsym(wchar_t *collname)
{
    int i;
    wchar_t *mccollptr;
    
    if(mccollist)
    {
        mccollptr = &mccollist[1];

        for (i=0; i < mccollist[0]; i++)
        {
            if (!wcscmp(mccollptr, collname))
                return(1);
            mccollptr += (wcslen(mccollptr) + 1);
        }
    }
    return(0);
}

/* ------------------------- getcollset ------------------------- */

/*
 * getcollset - get all the multi-character collating elements for
 *              the given locale and return the final size of 
 *              the array used to store them (mccollset)
 * inputs     - none
 * outputs    - size of mccollset
 * globals    - mccollset
 *
 */

int getcollset()
{
    wchar_t wc;
    int i, len = 0;
    int size;
    
    getcollist(&mccollset, &mccollseti, NCH);
    size = NCH;

    for (wc = co_wc_min; wc <= co_wc_max; wc++)
    {
        if (!asciipath  && EXISTSCOLLEL(wc))
        {
            i=0;
            while (CE_SYM(wc, i) != (uchar *)NULL)
            {
                len = strlen(CE_SYM(wc, i));
                if((mccollseti + len + 2) >= size)
                {
                    size += NCH;
                    mccollset = (wchar_t *)realloc(mccollset, size * sizeof(*mccollset));
                    if (!mccollset)
                        error(MSGSTR(CALLOCFAILED,"Insufficient memory for multi-character collating elements."));
                }
                /* CE_SYM does not contain the first character of a multi-character
                   collating element - so store the first character */
                mccollset[mccollseti++] = wc;
                /* then convert to wchar_t and store the rest */
                len = mbstowcs(&mccollset[mccollseti], CE_SYM(wc, i), len) + 1;
                mccollseti += len;            
                mccollset[0]++;
                i++;
             }      /* end of while */
         }      /* end of if */
    }      /* end of for */
    if (mccollset[0] == 0)
    {
        cfree((void *)mccollset, size, sizeof(*mccollset));
        mccollset = 0;
        return(0);
    }
    return(mccollseti + 1);
}
    
/* ------------------------ invcollist ------------------------ */

/* 
 * invcollist - invert mccollist and store the inverted list in
 *              mccollist
 * inputs     - none
 * outputs    - none
 * globals    - mccollist, nmccollist, mccollset, mccollisti, nmccollisti
 *
 */

void invcollist()
{
    int len = 0, i;
    wchar_t *collsetp;

    collsetp = &mccollset[1];

    if (!nmccollist)
        getcollist(&nmccollist, &nmccollisti, mcsize);
    else
    {
        nmccollist[0] = 0;
        nmccollisti = 1;
    }
    for (i=0; i<mccollset[0]; i++)
    {
        len = wcslen(collsetp) + 1;
        if(!existscollsym(collsetp))
        {
            wcscpy(&nmccollist[nmccollisti], collsetp);
            nmccollisti += len;
            nmccollist[0]++;
        }
        collsetp += len;
    }
    /* switch the pointers so that the inverted list is pointed to
       by mccollist */
    collsetp = mccollist;
    mccollist = nmccollist;
    nmccollist = collsetp;
}

/* ------------------------ collsymtree ------------------------ */

/*
 * collsymtree - create a sub-tree structure which matches the
 *               multi-character collating elements in collistp
 *             - the tree is built in the following manner for a
 *               list containing: ch, ll, ae
 *
 *                    c
 *                    |
 *                    h
 *
 *                      BAR
 *                     /   \
 *                    c     l
 *                    |     |
 *                    h     l
 *
 *                          BAR
 *                         /   \
 *                       BAR    a
 *                      /   \   |
 *                     c     l  e
 *                     |     |  
 *                     h     l     
 *
 * inputs      - pointer to a list of multi-character collating
 *               elements
 * outputs     - node index of the base of the sub-tree
 * globals     - none
 *
 */

int collsymtree(wchar_t *collistp)
{
    wchar_t *curpos;
    int i, j, prev;

    curpos = &collistp[1];
    i = mn0((int)*curpos++, RWCHAR);
    while(*curpos)
    {
        i = mn2(RSTR, i, (int)*curpos++);
    }
    curpos++;
    prev = i;
    for (j=1; j<collistp[0]; j++)
    {
        i = mn0((int)*curpos++, RWCHAR);
        while(*curpos)
        {
            i = mn2(RSTR, i, (int)*curpos++);
        }
        curpos++;
        prev = mn2(BAR, prev, i);
    }
    return(prev);
}
                
/* -------------------------- compare -------------------------- */

/*
 * compare - compare two wide characters
 *         - used for call to qsort in invccl
 * inputs  - pointers to two wide characters to be compared
 * outputs - result of comparison
 * globals - none
 *
 */

int compare(const wchar_t *a, const wchar_t *b)
{
    if (*a < *b)
        return(-1);
    if (*a > *b)
        return(1);
    return(0);
}

/* -------------------------- invccl -------------------------- */

/*
 * invccl  - invert a character class list contained in symbol
 *           and wsymbol
 *         - symbol and wsymbol will contain the inverted list
 * inputs  - none
 * outputs - none
 * globals - symbol, wsymbol, wsymboli
 *
 */

invccl()
{
    int i, count;
    wchar_t wc, min, max;
    wchar_t *cclist;
    char *mb;

    /* invert 8 bit chars */
    for(i=1; i<NCH; i++)
        symbol[i] ^= 1;
   
    /* invert multi-byte chars */
    if(mbcurmax>1)
    {
        if((!wmatchsize || !wcranksize) && !wsymboli)
        {
            if (!nohormmsg)
            {
                warning(MSGSTR(NOHORM,"[^ ] will not match multibyte characters unless %h and %m are set greater than zero."));
                nohormmsg++;
            }
        }
        else
        {
            /* store the list of multi-byte characters in cclist so that
               it can then be inverted */
            cclist = (wchar_t *)myalloc(wsymboli+1, sizeof(*cclist));
            mb = (char *)myalloc(mbcurmax+1, sizeof(*mb));
            if(!cclist || !mb)
                error(MSGSTR(CALLOCFAILED,"There is not enough memory available."));
            count = wsymboli;
            for(i=0; i<wsymboli; i++)
                cclist[i] = wsymbol[i];
            /* add lower bound to list */
            cclist[i] = NCH-1;
            qsort((void *)cclist, (size_t)(count+1), (size_t)sizeof(*cclist), compare);
            wsymboli = 0;
            
            for(i=0;i<count;i++)
            {
    
                if (cclist[i] == co_wc_max)
                    break;
                min = cclist[i] + 1;
    
                max = cclist[i+1] - 1;

                for(wc = min; wc <= max; wc++) 
                {
                    if(wctomb(mb, wc))
                    {
                        if(mblen(mb, mbcurmax) > 0)
                            addwsymbol(wc,1);
                    }
                }
            }
            if (cclist[count] < co_wc_max)
            {
                for(wc = cclist[count]+1; wc<=co_wc_max; wc++)
                {
                    if(wctomb(mb, wc))
                    {
                        if(mblen(mb, mbcurmax) > 0)
                            addwsymbol(wc,1);
                    }
                }
            }
        }
    }
}
