/* @(#)16       1.7.3.13  src/bos/usr/ccs/bin/lex/parser.y, cmdlang, bos411, 9432B411a 8/10/94 14:01:22 */
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: yylex, freturn
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

        /* Multi-byte support added by Michael S. Flegel, July 1991 */


/* --------------------------- NOTES --------------------------- */

/*
 *    1. This file consists of the yacc source and the lexical
 *       analyser for parsing the lex input file and building
 *       the parse tree.  The parser parses the following in
 *       the definitions section of the source: definitions with
 *       substitutions, start condition names, and translations.
 *       In the rules section, the parser recognizes extended
 *       regular expressions (rules).
 *
 *    Definitions
 *       Definitions and their correspoding substitutions are
 *       stored in defchar[].  A pointer to the definition
 *       is stored in def[dptr] and a pointer to the substitution
 *       is stored in subs[dptr].  When a definition is seen
 *       later on in a rule, lookup() is used to search for the
 *       substitution.
 *
 *    Start Conditions
 *       Start condition names are stored in schar[] where sp
 *       points to the next available position in schar[].
 *       A pointer to the start condition name is stored in
 *       sname[sptr].  A flag indicating whether the start
 *       condition is an exclusive start condition is set in
 *       excl[sptr].  When a start condition name is seen
 *       in a rule, lookup() is used to search for it in sname.
 *       The position of the start condition name in sname
 *       indicates the start state for any rule beginning with
 *       that start condition.  For example if a start condition
 *       name is found at sname[3], the start state is 2*3 or
 *       6.  Two states are allocated for each start condition
 *       to allow for beginning of the line matches (^).  Those
 *       expressions that must start on a new line belong to
 *       odd start states and the rest to even start states.
 *
 *    Translation Sets
 *       Character translations are stored in ctable[], an array of
 *       size 256 or one for each character (wide characters are
 *       currently not supported in translation sets).  Translation
 *       of characters is handled when the final state machine
 *       is written to lex.yy.c by layout().
 *
 *    Extended Regular Expressions
 *       For the following set of regular expressions:
 *
 *               ^abc
 *               a*|b+
 *
 *       the parse tree would be built up in the following manner:
 *
 *
 *                      (6) RCAT
 *                         /    \
 *                  (4) CARAT  (5) FINAL
 *                        |
 *                  (3) RSTR(c)
 *                        |
 *                  (2) RSTR(b)
 *                        |
 *                  (1)  (a)
 *
 *
 *
 *                      (14) RNEWE
 *                         /      \
 *                  -------        -------
 *                 /                      \
 *          (6) RCAT                (13) RCAT 
 *             /    \                   /    \
 *       (4) CARAT   (5) FINAL     (11) BAR   (12) FINAL
 *            |                        /   \
 *       (3) RSTR(c)            (8) STAR  (10) PLUS
 *            |                     |          |
 *       (2) RSTR(b)           (7)  a      (9) b
 *            |
 *       (1)  a
 *
 *       The parse tree is stored in the following set of arrays:
 *
 *           name[] : identifies the parse tree node type
 *                    (ie. STAR, RSTR, RNEWE, RSCON, etc.)
 *           left[] : contains the node number of the left child
 *                    of the current node except for:
 *                      FINAL, S1FINAL, S2FINAL - contains the
 *                          case number of the action
 *                      RCCL - initially contains the list of
 *                          characters belonging to the CCL,
 *                          later, it contains a list of CCL
 *                          identifiers corresponding to the
 *                          characters in the CCL
 *                    for example, left[4] = 3, left[6] = 4
 *           right[]: contains the node number of the right child
 *                    of the current node, for example, right[6] = 5
 *                    except for:
 *                      RCCL - may contain the xccl type if there is
 *                          one
 *                      RSCON - contains the list of start states
 *                          associated with this start condition(s)
 *                      RSTR - contains the associated character,
 *                          for example, right[2] = 'b' or RWCHAR
 *                          in the case of a wide character
 *           parent[]: contains the node number of the parent of
 *                    the current node, for example, parent[5] = 6
 *           nullstr[]: identifies whether zero ocurrences of the
 *                    subtree rooted at this node is acceptable,
 *                    for example, in the case above, nullstr[8]
 *                    and nullstr[10] would both be true.
 *           wname[]: contains the wide character associated with a
 *                    RWCHAR or RSTR node
 *
 *    Special (extended) Character Classes
 *        Currently special character classes consist only of ".",
 *        however, the code is set up to allow further extended
 *        character classes if needed.  The array xccl[] is used to
 *        store xccl types active in the current set of regular
 *        expressions.  The xccl type is also stored in right[] for
 *        RCCL nodes which define an xccl.  The variable xccltop
 *        indicates the next available position in xccl[].  Members
 *        of the xccl[] array belong to the current set of character
 *        class intersections and xccl[].cindex holds the character
 *        class identifier.
 */

           
%token CHAR                                     /* character            (a)     */
%token CCL                                      /* character class      ([a-z]) */
%token NCCL                                     /* not character class  ([^ab]) */
%token STR                                      /* string               ("aaa") */
%token DELIM                                    /* section delimiter    (%%)    */
%token SCON                                     /* start condition      (%START)*/
%token ITER                                     /* iteration            (a{m,n})*/
%token NEWE                                     /* new rule                     */
%token NULLS                                    /* empty string         ("")    */

%left SCON '/' NEWE
%left '$' '^'
%left '|'
%left CHAR CCL NCCL '(' '.' STR NULLS
%left ITER
%left CAT                                       /* sets up rule catenation precedence */
%left '*' '+' '?'

%{
#include <stddef.h>
#define YYSTYPE union _yystype_
union _yystype_
{
        int             i;
        wchar_t         *wcp;
};
# include "ldefs.h"
#define YACC_MSG

extern int asciipath;
int UNIQ_ORDER = _UCW_ORDER;

%}
%%
%{
int i;
int j,k;
int g;
wchar_t *p;
wchar_t *tp;
%}
acc     :       lexinput
        ={      
# ifdef DEBUG
                if(debug) sect2dump();
# endif
        }
        ;
lexinput:       defns delim prods end
        |       defns delim end
        ={
                if(!funcflag)phead2();
                funcflag = TRUE;
        }
        | error
        ={
# ifdef DEBUG
                if(debug) {
                        sect1dump();
                        sect2dump();
                        }
# endif
                }
        ;
end:            delim | ;
defns:  defns STR STR
        ={      j = slength($2)+1;
                k = slength($3)+1;
                if((dp+j+k) >= dchar+defchar)
                {
                    defchar = defchar * 2;
                    tp = dchar;
                    dchar = (wchar_t *)realloc(dchar, defchar * sizeof(*dp));
                    if (!dchar)
                        error(MSGSTR(CALLOCFAILED, "There is not enough memory available."));
                    if (tp != dchar)
                    {
                        for(i=0; i<dptr; i++)
                        {
                            def[i] = (def[i] - tp) + dchar;
                            subs[i] = (subs[i] - tp) + dchar;
                        }
                        dp = (dp - tp) + dchar;
                    }
                }
                scopy($2,dp);
                def[dptr] = dp;
                dp += j;
                scopy($3,dp);
                subs[dptr++] = dp;
                dp += k;
                if(dptr >= defsize)
                {
                    defsize = defsize * 2;
                    def = (wchar_t **)realloc(def, defsize * sizeof(*def));
                    subs = (wchar_t **)realloc(subs, defsize * sizeof(*subs));
                    if (!def || !subs)
                        error(MSGSTR(CALLOCFAILED, "There is not enough memory available."));
                }
                subs[dptr]=def[dptr]=0; /* for lookup - require ending null */
        }
        |
        ;
delim:  DELIM
        ={
# ifdef DEBUG
                if(sect == DEFSECTION && debug) sect1dump();
# endif
                sect++;
                }
        ;
prods:  prods pr
        ={      $$.i = mn2(RNEWE,$1.i,$2.i);
                }
        |       pr
        ={      $$.i = $1.i;}
        ;
pr:     r NEWE
        ={
                if(divflg == TRUE)
                    i = mn1(S1FINAL,casecount);
                else
                    i = mn1(FINAL,casecount);
                $$.i = mn2(RCAT,$1.i,i);
                divflg = FALSE;
                casecount++;
                }
        | SCON r NEWE
        ={
                if(divflg == TRUE)
                    i = mn1(S1FINAL,casecount);
                else
                    i = mn1(FINAL,casecount);
                i = mn2(RCAT,$2.i,i);
                $$.i = mn2(RSCON, i, $1.i);
                divflg = FALSE;
                casecount++;
                }
        | error NEWE
        ={
                $$.i = 0;
# ifdef DEBUG
                if(debug) sect2dump();
# endif
                }
r:      CHAR
        ={      $$.i = mn0($1.i,RWCHAR); }
        | STR
        ={
                p = $1.wcp;
                i = mn0(*p++,RWCHAR);
                while(*p)
                        i = mn2(RSTR,i,*p++);
                $$.i = i;
                }
        | '.'
        ={      symbol['\n'] = 0;
                wsymboli = 0;
                if(psave == FALSE){
                        if(cclarrayi >= cclarray_size)
                        {
                            cclarray_size += CCLARRAY_SIZE;
                            cclarray = (ccltypedef *)realloc(cclarray, cclarray_size * sizeof(*cclarray));
                            if(!cclarray)
                                error(MSGSTR(CALLOCFAILED, "There is not enough memory available."));
                            for(i=cclarrayi; i<cclarray_size; i++)
                            {
                                cclarray[i].cclptr = 0;
                                cclarray[i].len = 0;
                            }
                        }
                        if (!(cclarray[cclarrayi].cclptr = (wchar_t *)myalloc(NCH, sizeof(*cclarray[0].cclptr))))
                            error(MSGSTR(CALLOCFAILED, "There is not enough memory available."));
                        psave = cclarray[cclarrayi].cclptr;
                        for(i=1;i<'\n';i++){
                                symbol[i] = 1;
                                *psave++ = i;
                                }
                        for(i='\n'+1;i<NCH;i++){
                                symbol[i] = 1;
                                *psave++ = i;
                                }
                        psave = cclarray[cclarrayi++].cclptr;
                }
                $$.i = mn2(RCCL,psave,CCLDOT);
                cclinter(CCLDOT,$$.i);
                }
        | CCL
        ={
                if(mccollist && mccollist[0])
                {
                    i = collsymtree(mccollist);
                    if($1.wcp[0])
                        i = mn2(BAR, i, mn2(RCCL, $1.i,0));
                }
                else
                    i = mn2(RCCL, $1.i,0);
                $$.i = i;
                }
        | r '*'
        ={      $$.i = mn1(STAR,$1.i); }
        | r '+'
        ={      $$.i = mn1(PLUS,$1.i); }
        | r '?'
        ={      $$.i = mn1(QUEST,$1.i); }
        | r '|' r
        ={      $$.i = mn2(BAR,$1.i,$3.i); }
        | r r %prec CAT
        ={      $$.i = mn2(RCAT,$1.i,$2.i); }
        | r '/' r
        ={      if(!divflg){
                        j = mn1(S2FINAL,-casecount);
                        i = mn2(RCAT,$1.i,j);
                        $$.i = mn2(DIV,i,$3.i);
                        }
                else {
                        $$.i = mn2(RCAT,$1.i,$3.i);
                        warning(MSGSTR(XTRASLASH, "Extra slash removed"));
                        }
                divflg = TRUE;
                }
        | r ITER ',' ITER '}'
        ={      if($2.i > $4.i){
                        i = $2.i;
                        $2.i = $4.i;
                        $4.i = i;
                        }
                if($4.i <= 0)
                        warning(MSGSTR(NEGINTR,
                                "Iteration range must be positive"));
                else {
                        j = $1.i;
                        for(k = 2; k<=$2.i;k++)
                                j = mn2(RCAT,j,dupl($1.i));
                        for(i = $2.i+1; i<=$4.i; i++){
                                g = dupl($1.i);
                                for(k=2;k<=i;k++)
                                        g = mn2(RCAT,g,dupl($1.i));
                                j = mn2(BAR,j,g);
                                }
                        $$.i = j;
                        }
        }
        | r ITER '}'
        ={
                if($2.i < 0)warning(MSGSTR(NEGINTR,
                                "Can't have negative iteration"));
                else if($2.i == 0) $$.i = mn0(RNULLS,0);
                else {
                        j = $1.i;
                        for(k=2;k<=$2.i;k++)
                                j = mn2(RCAT,j,dupl($1.i));
                        $$.i = j;
                        }
                }
        | r ITER ',' '}'
        ={
                                /* from n to infinity */
                if($2.i < 0)warning(MSGSTR(NEGINTR,
                                "Can't have negative iteration"));
                else if($2.i == 0) $$.i = mn1(STAR,$1.i);
                else if($2.i == 1)$$.i = mn1(PLUS,$1.i);
                else {          /* >= 2 iterations minimum */
                        j = $1.i;
                        for(k=2;k<$2.i;k++)
                                j = mn2(RCAT,j,dupl($1.i));
                        k = mn1(PLUS,dupl($1.i));
                        $$.i = mn2(RCAT,j,k);
                        }
                }
        | '^' r
        ={      $$.i = mn1(CARAT,$2.i); }
        | r '$'
        ={      i = mn0('\n',0);
                if(!divflg){
                        j = mn1(S2FINAL,-casecount);
                        k = mn2(RCAT,$1.i,j);
                        $$.i = mn2(DIV,k,i);
                        }
                else $$.i = mn2(RCAT,$1.i,i);
                divflg = TRUE;
                }
        | '(' r ')'
        ={      $$.i = $2.i; }
        |       NULLS
        ={      $$.i = mn0(RNULLS,0); }
        ;
%%
yylex(){
        wchar_t *p;
        register int c, i;
        wchar_t *t, *xp;
        int n, j, k, x;
        static int sectbegin;
        static int iter;
        static wchar_t token[TOKENSIZE];
        int first_char;
        int prev_prev;
        char tmpvar[16];
        int tmpvarsize;
        int dashflag;                           /* flag indicates range expression */
        int max_ucoll, min_ucoll;           /* used to store range boundaries */
        char *wgt_str;
        wchar_t delim;                          /* used to store bracket expr. delimiter */
        wchar_t *collptr;                       /* pointer to current pos. in collname */
        char *dummy1=NULL;
        int dummy2;

# ifdef DEBUG
        yylval.i = 0;
# endif

        if(sect == DEFSECTION)                  /* definitions section */
        {
                while(!eof)
                {
                        if(prev == '\n')        /* next char is at beginning of line */
                        {
                                getl(p=buf);
                                switch(*p)
                                {
                                case '%':
                                        switch(c= *(p+1))
                                        {
                                        case '%':
                                                lgate();
                                                if(!ratfor)cfollow();  /* defect 69866 */
                                                if(!ratfor)fprintf(fout,"# ");
                                                fprintf(fout,
                                                  "define YYNEWLINE %d\n",
                                                  ctable['\n']);
                                                if(!ratfor)
                                                {
                                                        fprintf(fout,"#ifdef __cplusplus\nextern \"C\"\n#endif /*__cplusplus */\n");
                                                        fprintf(fout,"int yylex(){\nint yynstr; extern int yyprevious;\n");
                                                }
                                                sectbegin = TRUE;
                                                i = treesize*(sizeof(*name)
                                                              + sizeof(*left)
                                                              + sizeof(*right)
                                                              + sizeof(*nullstr)
                                                              + sizeof(*parent)
                                                              + sizeof(*wname))
                                                    + wmatchsize*(sizeof(match_t))
                                                    + xcclsize*(sizeof(xccl_t))
                                                    + ALITTLEEXTRA;
                                                c = (int)myalloc(i,1);
                                                if(c == 0)
                                                    error(MSGSTR(NOCORE4, "Too little core for parse tree"));
                                                p = (wchar_t *)c;
                                                cfree((void *)p,i,1);
                                                name = (int *)myalloc(treesize,
                                                        sizeof(*name));
                                                wname = (wchar_t *)myalloc(treesize,
                                                        sizeof(*wname));
                                                left = (int *)myalloc(treesize,
                                                        sizeof(*left));
                                                right=(int *)myalloc(treesize,
                                                        sizeof(*right));
                                                nullstr = (wchar_t *)
                                                        myalloc(treesize,
                                                        sizeof(*nullstr));
                                                parent=(int *)myalloc(treesize,
                                                        sizeof(*parent));
                                                wmatch=(hash_t *)hashalloc(wmatchsize);
                                                xccl=(xccl_t *)myalloc(xcclsize,sizeof(*xccl));
                                                if(   name == 0 || left == 0 || right == 0
                                                   || parent == 0 || nullstr == 0 || wname == 0
                                                   || (wmatchsize && (wmatch==0))
                                                   || (xcclsize && (xccl == 0)))
                                                {
                                                    error(MSGSTR(NOCORE4, "Too little core for parse tree"));
                                                }
                                                return(freturn(DELIM));
                                        case 'p': case 'P':
                                        /*has overridden number of positions*/
                                                if(!wcsncmp(L"%pointer", p, 8))
                                                    yytext_type = TRUE;
                                                else
                                                {
                   
                                                    while(*p && !digit(*p))p++;
                                                    maxpos = siconv(p);
# ifdef DEBUG
                                                    if (debug) printf("positions (%%p) now %d\n",maxpos);
# endif
                                                    if(report == 2)report = 1;
                                                }
                                                continue;
                                        case 'n': case 'N':     
                                        /* has overridden number of states */
                                                while(*p && !digit(*p))p++;
                                                nstates = siconv(p);
# ifdef DEBUG
                                                if(debug) printf( "no. states (%%n) now %d\n",nstates);
# endif
                                                if(report == 2)report = 1;
                                                continue;
                                        case 'e': case 'E':
                                        /*has overridden number of tree nodes*/
                                                while(*p && !digit(*p))p++;
                                                treesize = siconv(p);
# ifdef DEBUG
                                                if (debug) printf("treesize (%%e) now %d\n",treesize);
# endif
                                                if(report == 2)report = 1;
                                                continue;
                                        case 'o': case 'O':
                                                while (*p && !digit(*p))p++;
                                                outsize = siconv(p);
                                                if (report ==2) report=1;
                                                continue;

                                        /* has overridden number of wcrank hash slots */
                                        case 'h': case 'H':
                                                while (*p && !digit(*p))p++;
                                                wcranksize = siconv(p);
# ifdef DEBUG
                                                if (debug)
                                                {
                                                    printf("no. multi-byte hash slots(%%h) now %d\n",
                                                           wcranksize);
                                                    if (!xcclsize)
                                                        printf ("no. multi-byte character class slots(%%x) defaulted to %d\n",
                                                                xcclsize);
                                                }
# endif
                                                if (xcclsize == 0)
                                                    xcclsize = 50;
                                                if (report ==2) report=1;
                                                continue;

                                        /* has overridden number of wmatch hash slots */
                                        case 'm': case 'M':
                                                while (*p && !digit(*p))p++;
                                                wmatchsize = siconv(p);
# ifdef DEBUG
                                                if (debug) printf("no. multi-byte character class character hash slots(%%h) now %d\n", wmatchsize);
# endif
                                                if (report ==2) report=1;
                                                continue;

                                        /* has overridden multi-byte CCL output slots */
                                        case 'z': case 'Z':
                                                while (*p && !digit(*p))p++;
                                                xcclsize = siconv(p);
# ifdef DEBUG
                                                if (debug) printf("no. special character class slots(%%x) now %d\n", xcclsize);
# endif
                                                if (report ==2) report=1;
                                                continue;
                                                
                                        /* has overridden vacancy percentage in hash */
                                        case 'v': case 'V':
                                                while (*p && !digit(*p))p++;
                                                whspace = siconv(p);
                                                if ((whspace >= 100) || (whspace < 0))
                                                    whspace = NWHSPACE;
# ifdef DEBUG
                                                if (debug) printf("hash table vacancy(%%v) now %d\n", whspace);
# endif
                                                if (report ==2) report=1;
                                                continue;

                                        case 'a': case 'A':     
                                        /* has overridden number of transitions */
                                                if(!wcsncmp(L"%array", p, 6))
                                                    yytext_type = FALSE;
                                                else
                                                {
                                                    while(*p && !digit(*p))p++;
                                                    if(report == 2)report = 1;
                                                    ntrans = siconv(p);
# ifdef DEBUG
                                                    if (debug) printf("No. trans (%%a) now %d\n",ntrans);
# endif
                                                }
                                                continue;
                                        case 'k': case 'K':
                                        /* overriden packed char classes */
                                                while (*p && !digit(*p))p++;
                                                if (report==2) report=1;
                                                cfree((void *)pchar, pchlen,
                                                        sizeof(*pchar));
                                                pchlen = siconv(p);
# ifdef DEBUG
                                                if (debug) printf( "Size classes (%%k) now %d\n",pchlen);
# endif
                                                pchar=pcptr=(wchar_t *)
                                                        myalloc(pchlen,
                                                        sizeof(*pchar));
                                                continue;
                                        case 't': case 'T':
                                        /* character set specifier */
                                                ZCH = siconv((wchar_t *)p+2);
                                                if (ZCH < NCH)
                                                    ZCH = NCH;
                                                if (ZCH > 2*NCH)
                                                    error(MSGSTR(CHTAB1, "ch table needs redeclaration"));
                                                chset = TRUE;
                                                for(i = 0; i<ZCH; i++)
                                                    ctable[i] = 0;
                                                while(   getl(p)
                                                      && scomp(p,L"%T") != 0
                                                      && scomp(p,L"%t") != 0)
                                                {
                                                    if ((n=siconv(p)) <= 0 || n > ZCH) /* illegal translation */
                                                    {
                                                        warning(MSGSTR(CHARRANGE, "Character value %d out of range"),n);
                                                        continue; /* ignore it */
                                                    }
                                                    while(!space(*p) && *p)
                                                        p++;
                                                    while(space(*p)) p++;
                                                    t = p;
                                                    while(*t)
                                                    {
                                                        c = ctrans(&t);
                                                        if (c <= 0 || c >= ZCH) /* MB chars not supported */
                                                        {
                                                            warning(MSGSTR(CHARRANGE, "Character value %d out of range"),c);
                                                        }
                                                        else if(ctable[c])
                                                        {
                                                            if (printable(c))
                                                                warning(MSGSTR(DCHARC, "Character '%c' used twice"),c);
                                                            else
                                                                warning(MSGSTR(DCHAR0, "Character %o used twice"),c);
                                                        }
                                                        else
                                                            ctable[c] = n;
                                                        t += 1;
                                                    }
                                                    p = buf;
                                                }
                                                {
                                                    char chused[2*NCH]; int kr;
                                                    for(i=0; i<ZCH; i++)
                                                        chused[i]=0;
                                                    for(i=0; i<NCH; i++)
                                                        chused[ctable[i]]=1;
                                                    for(kr=i=1; i<NCH; i++)
                                                    {
                                                        if (ctable[i]==0)
                                                        {
                                                            while (chused[kr] == 0)
                                                                kr++;
                                                            ctable[i]=kr;
                                                            chused[kr]=1;
                                                        }
                                                    }
                                                }
                                                lgate();
                                                continue;
                                        case 'r': case 'R':
                                                c = 'r';
                                        case 'c': case 'C':
                                                if(lgatflg)
                                                    error(MSGSTR(LANG2LATE, "Too late for language specifier"));
                                                ratfor = (c == 'r');
                                                continue;
                                        case '{':
                                                lgate();
                                                while(getl(p) &&
                                                        scomp(p,L"%}") != 0)
                                                        fprintf(fout,"%S\n",p);
                                                if(p[0] == '%') continue;
                                                error(MSGSTR(PEOF,"Premature eof"));
                                        case 's': case 'S':
                                        case 'x': case 'X':
                                        /* start conditions */
                                                lgate();
                                                while(*p &&
                                                        indexx(*p,L" \t,") < 0)
                                                        p++;
                                                n = TRUE;
                                                while(n){
                                                        while(*p &&
                                                          indexx(*p,L" \t,")>=0)
                                                                p++;
                                                        t = p;
                                                        while(*p && indexx(*p,L" \t,") < 0)p++;
                                                        if(!*p) n = FALSE;
                                                        *p++ = 0;
                                                        if (*t == 0) continue;
                                                        i = sptr*2;
                                                        if(!ratfor)
                                                         fprintf(fout, "# ");
                                                        fprintf(fout,
                                                         "define %S %d\n",t,i);
                                                        scopy(t,sp);
                                                        if ((c == 'x') || (c == 'X'))
                                                                excl[sptr] = 1;
                                                        else
                                                                excl[sptr] = 0;
                                                        sname[sptr++] = sp;
                                                        sname[sptr] = 0;
                                                        /*required by lookup*/
                                                        if(sptr >= STARTSIZE)
                                                                error(MSGSTR(TSTART1, "Too many start conditions"));
                                                        sp += slength(sp) + 1;
                                                        if(sp >= schar+ STARTCHAR)
                                                            error(MSGSTR(LONGSTART, "Start conditions too long"));
                                                        }
                                                continue;
                                        default:
                                                warning(MSGSTR(INVALREQ,"Invalid request %S"),p);
                                                continue;
                                        } /* end of switch after seeing '%' */
                                case ' ': case '\t':    /* must be code */
                                        lgate();
                                        fprintf(fout, "%S\n",p);
                                        continue;
                                default:                /* definition */
                                        while(*p && !space(*p)) p++;
                                        if(*p == 0)
                                                continue;
                                        prev = *p;
                                        *p = 0;
                                        bptr = p+1;
                                        yylval.wcp = buf;
                                        if(digit(buf[0]))
                                                warning(MSGSTR(NOLDIGITS, "Substitution strings may not begin with digits"));
                                        return(freturn(STR));
                                        }
                                }
                        /* still sect 1, but prev != '\n' */
                        else {
                                p = bptr;
                                while(*p && space(*p)) p++;
                                if(*p == 0)
                                        warning(MSGSTR(NOTRANS, "No translation given - null string assumed"));
                                scopy(p,token);
                                yylval.wcp = token;
                                prev = '\n';
                                return(freturn(STR));
                                }
                        }
                /* end of section one processing */
                }
        else if(sect == RULESECTION){           /* rules and actions */
                while(!eof){
                        prev_prev = prev;
                        switch(c=gch())
                        {
                        case '\0':
                            return(freturn(0));
                        case '\n':
                            if(prev == '\n') continue;
                            x = NEWE;
                            break;
                        case ' ':
                        case '\t':
                            if(sectbegin == TRUE){
                                cpyact(1);
                                while((c=gch()) && c!= '\n');
                                continue;
                            }
                            if(!funcflag)phead2();
                            funcflag = TRUE;
                            if(ratfor)fprintf(fout,"%d\n",30000+casecount);
                            else fprintf(fout,"case %d:\n",casecount);
                            if(cpyact(0)){
                                if(ratfor)fprintf(fout,"goto 30997\n");
/**
 * PTM 45382 request that lextab.l of struct pass lint. This required breaks
 *           to be clean from not reached problems.
 *           JRW 13/07/90
 */
                                else fprintf(fout,"/*NOTREACHED*/ break;\n");
                            }
                            while((c=gch()) && c != '\n');
                            while(peek == ' ' || peek == '\t'|| peek == '\n') {
                                while(peek == ' ' || peek == '\t' || peek == '\v' || peek == '\f') c = gch();
                                if (peek == '\n') c = gch();
                                else {
                                    warning(MSGSTR(EXECS, "Executable statements should occur right after %%%%"));
                                    while((c = gch()) && c != '\n');
                                }
                            }
                            x = NEWE;
                            break;
                        case '%':
                            if(prev != '\n') goto character;
                            if(peek == '{'){    /* included code */
                                getl(buf);
                                while(!eof && getl(buf) && scomp(L"%}",buf) != 0)
                                    fprintf(fout,"%S\n",buf);
                                continue;
                            }
                            if(peek == '%'){
                                c = gch();
                                c = gch();
                                x = DELIM;
                                break;
                            }
                            goto character;
                        case '|':
                            if(peek == ' ' || peek == '\t' || peek == '\n'){
                                if(ratfor)
                                    fprintf(fout,"%d\n",30000+casecount++);
                                else fprintf(fout,"case %d:\n", casecount++);
                                continue;
                            }
                            x = '|';
                            break;
                        case '$':
                            if (prev_prev == '\n' && prev == '^')
                            {
                                munput(L'c', 'n');
                                munput(L'c', '\\');
                                continue;
                            }
                            if(peek == '\n' || peek == ' ' ||
                               peek == '\t' || peek == '|' || peek == '/')
                            {
                                x = c;
                                break;
                            }
                            goto character;
                        case '^':
                            if(prev != '\n' && scon != TRUE && 
                               !(prev_prev == '\n' && prev == '}' && first_char))   /* valid only at line begin */
                            {
                                goto character; 
                            }
                            if(prev_prev == '\n' && prev == '}' && first_char)
                                prev = '\n';
                            x = c;
                            break;
                        case '?':
                        case '+':
                        case '.':
                        case '*':
                        case '(':
                        case ')':
                        case ',':
                        case '/':
                            x = c;
                            break;
                        case '}':
                            iter = FALSE;
                            x = c;
                            break;
                        case '{':       /* either iteration or definition */
                            if (prev == '\n')
                                first_char = 1;
                            else
                                first_char = 0;
                            c = gch();
                            if(digit(c)){               /* iteration */
                                iter = TRUE;
                            ieval:
                                i = 0;
                                while(digit(c))
                                {
                                    token[i++] = c;
                                    c = gch();
                                }
                                token[i] = 0;
                                yylval.i = siconv(token);
                                munput(L'c',c);
                                x = ITER;
                                break;
                            }
                            else                /* definition */
                            {
                                i = 0;
                                while(c && c!='}')
                                {
                                    token[i++] = c;
                                    c = gch();
                                }
                                token[i] = 0;
                                i = lookup(token,def);
                                if(i < 0)
                                    warning(MSGSTR(NODEF, "Definition %S not found"),token);
                                else
                                {
                                    munput(L's',subs[i]);
                                    if (first_char && *subs[i] != '\n')
                                        prev = '\n';
                                }
                                continue;
                            }
                        case '<':               /* start condition ? */
                            if(prev != '\n')
                                /* not at line begin, not start */
                                goto character;
                            t = slptr;
                            do {
                                i = 0;
                                c = gch();
                                while(c != ',' && c && c != '>'){
                                    token[i++] = c;
                                    c = gch();
                                }
                                token[i] = 0;
                                if(i == 0)
                                    goto character;
                                i = lookup(token,sname);
                                if(i < 0) {
                                    warning(MSGSTR(NOSTART,
                                                   "Undefined start condition %S"),token);
                                    continue;
                                }
                                *slptr++ = i+1;
                            } while(c && c != '>');
                            *slptr++ = 0;
                            /* check if previous value re-usable */
                            for (xp=slist; xp<t; )
                            {
                                if (wcscmp(xp, t)==0)
                                    break;
                                while (*xp++);
                            }
                            if (xp<t)
                            {
                                /* re-use previous pointer to string */
                                slptr=t;
                                t=xp;
                            }
                            if(slptr > slist+STARTSIZE) 
                                /* note not packed ! */
                                error(MSGSTR(TSTART,
                                             "Too many start conditions used"));
                            yylval.wcp = t;
                            x = SCON;
                            break;
                        case '"':
                            i = 0;
                            while((c=gch()) && c != '"' && c != '\n'){
                                if(c == '\\') c = usescape(c=gch());
                                token[i++] = c;
                                if(i >= TOKENSIZE){
                                    warning(MSGSTR(STRLONG,
                                                   "String too long"));
                                    i = TOKENSIZE-1;
                                    break;
                                }
                            }
                            if(c == '\n') {
                                yyline--;
                                warning(MSGSTR(ENDLESSTR,
                                               "Non-terminated string"));
                                yyline++;
                            }
                            token[i] = 0;
                            if(i == 0)x = NULLS;
                            else if(i == 1){
                                yylval.i = token[0];
                                x = CHAR;
                            }
                            else {
                                yylval.wcp = token;
                                x = STR;
                            }
                            break;
                        case '[':
                            for(i=1;i<NCH;i++)
                                symbol[i] = 0;
                            wsymboli=0;
                            if(mccollist)
                            {
                                mccollist[0]=0;
                                mccollisti = 1;
                            }
                            /*
                             * check for NCCL
                             */
                            x = CCL;
                            if ((c = gch()) == '^')
                            {
                                x = NCCL;
                                c = gch();
                            }
                            /*
                             * get all the characters of the CCL
                             */
                            dashflag = 0;
                            max_ucoll = 0;
                            if (c == '-' || c == ']')
                            {
                                symbol[c] = 1;
				if (asciipath)
				    max_ucoll=(int)c + co_col_min;
				else
                                    max_ucoll = COLLWGT(c, UNIQ_ORDER);
                                c = gch();
                            }
                            while (c != ']')
                            {
                                if (!c || c == '\n')
                                    error(MSGSTR(EBRACKET,"Invalid bracket expression."));
                                min_ucoll = max_ucoll;
                                switch(c)
                                {
                                default:
                                coll_ele:
                                    if (c == '\\')
                                        c = usescape(c = gch());
                                    if (c < NCH)
                                        symbol[c] = 1;
                                    else
                                        addwsymbol(c,1);
				    if (asciipath)
					max_ucoll=(int)c+co_col_min;
				    else
                                        max_ucoll = COLLWGT(c, UNIQ_ORDER);
                                    break;
                                case '-':
                                    if (dashflag || (peek == ']'))
                                        goto coll_ele;
                                    dashflag++;
                                    c = gch();
                                    continue;
                                case '[':
                                    if ((peek != '.') && (peek != ':') && (peek != '='))
                                        goto coll_ele;
                                    delim = gch();
                                    c = gch();
                                    collptr = collname;
                                    *collptr = 0;
                                    while((c != delim) || (peek != ']'))
                                    {
                                        if (!c || c == '\n') 
                                            error(MSGSTR(EBRACKET,"Invalid bracket expression."));
                                        if (c == '\\')
                                            c = usescape(c = gch());
                                        *collptr++ = c;
                                        if (collptr > &collname[COLLNAME_SIZE])
                                            error(MSGSTR(LONGNAME,"Class name or collating element name is too long."));
                                        c = gch();
                                    }      /* end of while */
                                    *collptr = 0;
                                    c = gch();      /* get rid of ']' */
                                    if(!collname[0])
                                        error(MSGSTR(EBRACKET,"Invalid bracket expression."));
                                    switch(delim)
                                    {
                                    case '.':
                                        max_ucoll = getcolsym(collname, UNIQ_ORDER, &wgt_str);
                                        if ((max_ucoll < co_col_min) || (max_ucoll > co_col_max))
                                            error(MSGSTR(EBRACKET,"Invalid bracket expression."));
                                        if(!collname[1])
                                        {
                                            if (collname[0] < NCH)
                                                symbol[collname[0]] = 1;
                                            else
                                                addwsymbol(collname[0], 1);
                                        }
                                        else
                                        {
                                            if(!mccollist)
                                                getcollist(&mccollist, &mccollisti, mcsize);
                                            if(!existscollsym(collname))
                                            {
                                                wcscpy(&mccollist[mccollisti], collname);
                                                mccollisti += (wcslen(collname) +1);
                                                mccollist[0]++;
                                            }
                                        }
                                        break;
                                    case ':':
                                        wcstombs(mbcollname, collname, COLLNAME_SIZE * mbcurmax + 1);
                                        getcharclass(mbcollname);
                                        /*
                                         * do not allow a character class
                                         * to begin or end a range 
                                         */
                                        max_ucoll = 0;
                                        break;
                                    case '=':
                                        max_ucoll = getcolsym(collname, 0, &wgt_str);
                                        getrange(max_ucoll, max_ucoll, 0, wgt_str);
                                        /*
                                         * do not allow an equivalence class
                                         * to begin or end a range 
                                         */
                                        max_ucoll = 0;
                                        break;
                                    }      /* end of switch(delim) */
                                }      /* end of switch(c) */
                                if (dashflag)
                                {
                                    getrange(min_ucoll, max_ucoll, UNIQ_ORDER, NULL);
                                    dashflag = 0;
                                }
                                c = gch();
                            }      /* end of while */
                            /*
                             * invert the ccl if necessary
                             */
                            if(x == NCCL)
                            {
                                invccl();
                                if(mccollset)
                                    invcollist();
                                x = CCL;
                            }
                            /*
                             * increase the size of cclarray if necessary
                             */
                            if(cclarrayi >= cclarray_size)
                            {
                                cclarray_size += CCLARRAY_SIZE;
                                cclarray = (ccltypedef *)realloc(cclarray, cclarray_size * sizeof(*cclarray));
                                if(!cclarray)
                                    error(MSGSTR(CALLOCFAILED, "There is not enough memory available."));
                                for(i=cclarrayi; i<cclarray_size; i++)
                                {
                                    cclarray[i].cclptr = 0;
                                    cclarray[i].len = 0;
                                }
                            }
                            j = 0;
                            /*
                             * get size of the ccl
                             */
                            for(i=0; i<NCH; i++)
                            {
                                if(symbol[i])
                                    j++;
                            }
                            cclarray[cclarrayi].len = j + wsymboli + 1;
                            cclarray[cclarrayi].cclptr = (wchar_t *)myalloc(cclarray[cclarrayi].len, sizeof(*cclarray[0].cclptr));
                            if(!cclarray[cclarrayi].cclptr)
                                error(MSGSTR(CALLOCFAILED, "There is not enough memory available."));
                            i = 0;
                            for(j=0;j<NCH;j++) 
                            {
                                if(symbol[j])
                                    cclarray[cclarrayi].cclptr[i++] = j;
                            }
                            for (j = 0; j<wsymboli; j++) /* copy in the MBs */
                                cclarray[cclarrayi].cclptr[i++] = wsymbol[j];
                            cclarray[cclarrayi].cclptr[i] = 0;
                            /*
                             * see if ccl already exists
                             */
                            for(j=0; j<cclarrayi; j++)
                            {
                                if(scomp(cclarray[cclarrayi].cclptr, cclarray[j].cclptr) == 0)
                                    break;
                            }
                            /*
                             * if so return pointer to identical ccl and
                             * free the new one
                             */
                            if (j < cclarrayi)
                            {
                                yylval.wcp = cclarray[j].cclptr;
                                cfree((void *)cclarray[cclarrayi].cclptr, cclarray[cclarrayi].len, sizeof(*cclarray[0].cclptr));
                                cclarray[cclarrayi].cclptr = 0;
                                cclarray[cclarrayi].len = 0;
                            }
                            /*
                             * otherwise return the new ccl and enter it
                             */
                            else
                            {
                                yylval.wcp = cclarray[cclarrayi++].cclptr;
                                cclinter(0,-1);
                            }
                            

#ifdef DEBUG
                            if(mccollist && mccollist[0])
                            {
                                printf("Collating symbol list:\n");
                                prtcollist(mccollist);
                            }
#endif

                            break;
                        case '\\':
                            c = usescape(c=gch());
                        default:
                        character:
                            if(iter)    /* second part of an iteration */
                            {
                                iter = FALSE;
                                if('0' <= c && c <= '9')
                                    goto ieval;
                            }
                            if(alpha(peek))
                            {
                                i = 0;
                                yylval.wcp = token;
                                token[i++] = c;
                                while(alpha(peek))
                                    token[i++] = gch();
                                if(peek == '?' || peek == '*' || peek == '+')
                                    munput(L'c',token[--i]);
                                token[i] = 0;
                                if(i == 1)
                                {
                                    yylval.i = token[0];
                                    x = CHAR;
                                }
                                else x = STR;
                            }
                            else
                            {
                                yylval.i = c;
                                x = CHAR;
                            }
                        }                       /* switch */

                        scon = FALSE;
                        if(x == SCON)scon = TRUE;
                        sectbegin = FALSE;
                        return(freturn(x));
                    }
            }
        /* section three */
        if (eof && sect == DEFSECTION)
                error(MSGSTR(NORULESEND, "Rules section delimiter missing"));
        ptail();
# ifdef DEBUG
        if(debug)
                fprintf(fout,"\n/*this comes from section three - debug */\n");
# endif
        while(getl(buf) && !eof)
                fprintf(fout,"%S\n",buf);
        return(freturn(0));
        }
/* end of yylex */
# ifdef DEBUG
freturn(i)
  int i; {
        if(yydebug) {
                printf("now return ");
                if(i < NCH) allprint(i);
                else printf("%d",i);
                printf("   yylval = ");
                switch(i){
                        case STR: case CCL: case NCCL:
                                strpt(yylval.wcp);
                                break;
                        case CHAR:
                                allprint(yylval.i);
                                break;
                        default:
                                printf("%d",yylval.i);
                                break;
                        }
                putchar('\n');
                }
        return(i);
        }
# endif
