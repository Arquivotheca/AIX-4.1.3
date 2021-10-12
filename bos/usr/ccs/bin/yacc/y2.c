static char sccsid[] = "@(#)23  1.7.2.11  src/bos/usr/ccs/bin/yacc/y2.c, cmdlang, bos411, 9428A410j 5/13/94 17:11:23";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: beg_debug, chfind, cpyact, cpycode, cpyunion,
                cstash, defin, defout, end_debug, end_reds,
                fdtype, finact, gettok, lhsfill, lrprnt, out_debug,
                rhsfill, setup, skipcom
 *
 * ORIGINS: 27 65 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */
/*static char rcsid[] = "RCSfile: y2.c,v Revision: 2.6  (OSF) Date: 90/
10/07 17:48:54 ";*/
# include "dextern"
extern char *calloc();
#ifndef _BLD /*osf compatibility */
#include "yacc_msg.h"
extern nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_YACC,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif
# define IDENTIFIER 257
# define MARK 258
# define TERM 259
# define LEFT 260
# define RIGHT 261
# define BINARY 262
# define PREC 263
# define LCURLY 264
# define C_IDENTIFIER 265  /* name followed by colon */
# define NUMBER 266
# define START 267
# define TYPEDEF 268
# define TYPENAME 269
# define UNION 270
# define ENDFILE 0

        /* communication variables between various I/O routines */

char *infile;   /* input file name */
int numbval;    /* value of an input number */
char tokname[NAMESIZE]; /* input token name */

        /* storage of names */

int cnamsz = CNAMSZ;    /* size of cnames */
char * cnamp;   /* place where next name is to be put in */
char *cnamb;            /* pointer to end of name buffer */
#define NDEFOUT 3
int ndefout = NDEFOUT;  /* number of defined symbols output */

        /* storage of types */
int ntypes;     /* number of types defined */
char * typeset[NTYPES]; /* pointers to type tags */

        /* symbol tables for tokens and nonterminals */

int ntokens = 0;
struct toksymb *tokset;
int *toklev;
int nnonter = -1;
struct ntsymb *nontrst;
int start_sym;  /* start symbol */

        /* assigned token type values */
int extval = 0;

        /* have seen a %union declaration - defect 122648 */
int seen_union = 0;

        /* input and output file descriptors */
FILE * fyinput;         /* .y input file */
FILE * faction;         /* file for saving actions */
FILE * fdefine;         /* file for # defines */
FILE * ftable;          /* y.tab.c file */
FILE * ftemp;           /* tempfile to pass 2 */
FILE * fdebug;          /* where the strings for debugging are stored */
FILE * foutput;         /* y.output file */

        /* storage for grammar rules */

int *mem0; /* production storage */
int *mem;
int nprod= 1;   /* number of productions */
int **prdptr;     /* pointers to descriptions of productions */
int *levprd;     /* precedence levels for the productions */
char *had_act;    /* set to 1 if the reduction has action code to do */

        /* variable buffer lengths */

int actsize = ACTSIZE;
int numprod = NPROD;
int nstates = NSTATES;
int nterms = NTERMS;
int nnonterms = NNONTERM;
int wsetsize;
int tempsize;

/*default location of the parser text file */

char *ParserPath = NULL;

int gen_lines = 1;      /* flag for generating the # line's default is yes */
int gen_testing = 0;    /* flag for whether to include runtime debugging */
int gen_cpp = 0;        /* flag for whether to generate .tab.c or .tab.C */

setup(argc,argv) int argc; char *argv[];
{       int i,j,lev,t, ty;
        int c, len = 1, filed_flag = 0, fileu_flag = 0;
        int *p;
        char actname[8];
	char *symprefix; /* used for -p sym_prefix option, *symprefix will point to sym_prefix */
	int PFlag = 0;   /* used for -p sym_prefix option, default is NOT(0)                   */
        foutput = NULL;
        fdefine = NULL;
        i = 1;
        while( argc >= 2  && argv[1][0] == '-' ) {
		if( argv[1][1] == '-' && !argv[1][2] ){
			argv++;
                	argc--;
			break;
		}
                while( *++(argv[1]) ){
                        switch( *argv[1] ){
                        case 's':
                        case 'S':
                                SplitFlag++;
                                continue;
                        case 'v':
                        case 'V':
                                fileu_flag = 1;
                                continue;
                        case 'D':
                        case 'd':
                                filed_flag = 1;
                                continue;
                        case 'l':
                        case 'L':
                                gen_lines = !gen_lines;
				continue;
                        case 't':
                        case 'T':
                                gen_testing = !gen_testing;
                                continue;
                        case 'o':
                        case 'O':
                                fprintf( stderr, MSGSTR(OFLAG,
                                  "`o' flag now default in yacc\n") ); /*MSG*/
                                continue;

                        case 'r':
                        case 'R':
                                error( MSGSTR(NORATFOR,
                                  "Ratfor yacc not supported.") ); /*MSG*/
                        case 'b':       /* Must be followed by the prefix to use */
				if( !( *++argv[1] )) { /*SHIX(93.1) now yacc will allow -bfile_prefix */
					argc--; 
                                	argv++;
				}
                                if (argc > 1 ) {        /* prefix was given */
                                        prefix = argv[1];
                                        len = strlen(prefix);
                                        argv[1] = argv[1] + len - 1;
                                }
                                else {
                                        error( MSGSTR(PREMIS,
                                                "Option b must be followed by a prefix"));
                                }
                                continue;
                        case 'N':
                                do {
                                        int N_value = 0;
                                        int mflg = 0;
                                        int nflg = 0;
                                        int rflg = 0;
                                        for ( ;; ) {
                                                switch( *++argv[1] ) {
                                                case 'm' : mflg++; continue;
                                                case 'n' : nflg++; continue;
                                                case 'r' : rflg++; continue;
                                                }
                                                break;
                                        }
                                        if( !mflg & !nflg & !rflg) error(MSGSTR(NOPT, "Illegal format for -N option"));
                                        if( mflg ) {
                                                if( !isdigit( *argv[1] ))
                                                        error(MSGSTR(NMNUM, "Illegal value for -N option"));
                                                while( isdigit( *argv[1] )) {
                                                        N_value = N_value*10 + *argv[1] - '0';
                                                        argv[1]++;
                                                }
                                                if( N_value > MEMSIZE ) {
                                                        memsize = N_value;
                                                }
                                                else
                                                        error( MSGSTR(NMNUM, "Illegal value for -N option"));
                                        }
                                        /* GH 04/30/91 ix18500 - allowing the
                                           cnamsz to be grown */
                                        if( nflg ) {
                                                if( !isdigit( *argv[1] ))
                                                        error(MSGSTR(NNNUM, "Illegal value for -N option"));
                                                while( isdigit( *argv[1] )) {
                                                        N_value = N_value*10 + * argv[1] - '0';
                                                        argv[1]++;
                                                }
                                                if( N_value > CNAMSZ ) {
                                                        cnamsz = N_value;
                                                }
                                                else
                                                        error( MSGSTR(NNNUM, "Illegal value for -N option"));
                                        }
                                        /* IX37745, 99862 - allow the number of
                                           rules to be grown and grow related
                                           buffers */
                                        if( rflg ) {
                                                if( !isdigit( *argv[1] ))
                                                        error(MSGSTR(NRNUM, "Use an integer greater than %d with the -Nr flag."), NPROD);
                                                while( isdigit( *argv[1] )) {
                                                        N_value = N_value*10 + * argv[1] - '0';
                                                        argv[1]++;
                                                }
                                                if( N_value > NPROD ) {
                                                        float ratio;
                                                        numprod = N_value;
                                                        ratio = (float)numprod/(float)NPROD;
                                                        nstates = ratio*NSTATES;
                                                        nterms = ratio *NTERMS;
                                                        nnonterms = ratio*NNONTERM;
                                                        actsize = ratio*ACTSIZE;
                                                }
                                                else
                                                        error(MSGSTR(NRNUM, "Use an integer greater than %d with the -Nr flag."), NPROD);
                                        }

                                } while( *argv[1]-- != '\0' );
                                continue;
			case 'Y':   /* SHIX(93.1) -y option is now used to specify an alternate parser */		
                        case 'y':   /* yaccpar file                                                    */
				if( !(*++argv[1] )) {
					argc--;
					argv++;
				}
				if( argc > 1){
					ParserPath = argv[1];
					argv[1] += strlen(ParserPath) - 1;
				}
				else
					error( MSGSTR(YPREMIS, 
						"Specify a parser with the -y flag."));
				continue;
			 case 'P':   /* SHIX(93.1) -p sym_prefix option */
                         case 'p':
				if( !(*++argv[1] )) {/* also allow -psym_prefix */
					argc--;
					argv++;
				}
				if( argc > 1){               /* sym_prefix is given */
					symprefix = argv[1];
					PFlag = 1;           /* set up -p flag      */
					argv[1] = argv[1] + strlen(symprefix) - 1;
				}
				else
					error( MSGSTR(PPREMIS, 
						"option p must be followed by a prefix"));
				continue;

                        case 'c':
                        case 'C':
                                gen_cpp = !gen_cpp;
                                continue;
                        default:
                                error( MSGSTR(BADOPT, "illegal option: %s"),
                                        argv[1]); /*MSG*/  
                                }
                        }
                argv++;
                argc--;
                }
	
        wsetsize = nnonterms + BUFFER;
        tempsize = nterms + nnonterms + 1;
        if(nstates > tempsize) tempsize = nstates;
        tempsize += BUFFER;

        fdebug = fopen( DEBUGNAME, "w" );
        if ( fdebug == NULL )
                error( MSGSTR(NODEBUG, "cannot open yacc.debug") ); /*MSG*/  

        if (!prefix) prefix = "y";

        if (filed_flag) {
                filed = (char *)calloc(len + 7, sizeof(char));
                if( !filed ) error(MSGSTR(OUTMEM, "Out of Memory" ));
                sprintf(filed, "%s.tab.h", prefix);
                fdefine = fopen( filed, "w" );
                if ( fdefine == NULL )
                        error(MSGSTR(NOTOPN, "cannot open %s"), filed); /*MSG*/  
        }

        if (fileu_flag) {
                fileu = (char *)calloc(len + 8, sizeof(char));
                if( !fileu ) error(MSGSTR(OUTMEM, "Out of Memory" ));
                sprintf(fileu, "%s.output", prefix);
                foutput = fopen(fileu, "w" );
                if( foutput == NULL )
                        error( MSGSTR(NOTOPN, "cannot open %s"), fileu); /*MSG*/
        }

        ofile = (char *)calloc(len + 7, sizeof(char));
        if( !ofile ) error(MSGSTR(OUTMEM, "Out of Memory" ));
        if (gen_cpp)
            sprintf(ofile, "%s.tab.C", prefix);
        else
            sprintf(ofile, "%s.tab.c", prefix);
        ftable = fopen( ofile, "w" );
        if( ftable == NULL )
                error( MSGSTR(NOTOPN, "cannot open %s" ), ofile); /*MSG*/  

        ftemp = fopen( TEMPNAME, "w" );
        faction = fopen( ACTNAME, "w" );
        if( ftemp==NULL || faction==NULL )
                error(MSGSTR(NOTEMP, "cannot open temp file") ); /*MSG*/  

        if ( argc < 2)
                error( MSGSTR(NOINPUT, "No Input File given") );
        if( ((fyinput=fopen( infile=argv[1], "r" )) == NULL ) ){
                error( MSGSTR(NOTOPN, "cannot open %s"), argv[1] ); /*MSG*/  
                }

        /* Allocate storage for mem0 array depending on value of memsize */
        getmem();
        cnamb = &cnamp[cnamsz];
        mem = mem0;
        defin(0,"$end");
        extval = 0400;
        defin(0,"error");
        defin(1,"$accept");
        mem=mem0;
        lev = 0;
        ty = 0;
        i=0;
	lineno = 1;
        beg_debug();    /* initialize fdebug file */
        if ( PFlag && (fdefine != NULL))
        {
	    fprintf( fdefine,  "#define yychar %schar\n", symprefix);
	    fprintf( fdefine,  "#define yyerrflag %serrflag\n", symprefix);
	    fprintf( fdefine,  "#define yyact %sact\n", symprefix);
	    fprintf( fdefine,  "#define yychk %schk\n", symprefix);
	    fprintf( fdefine,  "#define yydebug %sdebug\n", symprefix);
	    fprintf( fdefine,  "#define yydef %sdef\n", symprefix);
	    fprintf( fdefine,  "#define yyerror %serror\n", symprefix);
	    fprintf( fdefine,  "#define yyexca %sexca\n", symprefix);
	    fprintf( fdefine,  "#define yylex %slex\n", symprefix);
	    fprintf( fdefine,  "#define yylval %slval\n", symprefix);
	    fprintf( fdefine,  "#define yynerrs %snerrs\n", symprefix);
	    fprintf( fdefine,  "#define yypact %spact\n", symprefix);
	    fprintf( fdefine,  "#define yyparse %sparse\n", symprefix);
	    fprintf( fdefine,  "#define yypgo %spgo\n", symprefix);
	    fprintf( fdefine,  "#define yyps %sps\n", symprefix);
	    fprintf( fdefine,  "#define yypv %spv\n", symprefix);
	    fprintf( fdefine,  "#define yypvt %spvt\n", symprefix);
	    fprintf( fdefine,  "#define yyr1 %sr1\n", symprefix);
	    fprintf( fdefine,  "#define yyr2 %sr2\n", symprefix);
	    fprintf( fdefine,  "#define yyreds %sreds\n", symprefix);
	    fprintf( fdefine,  "#define yys %ss\n", symprefix);
	    fprintf( fdefine,  "#define yystate %sstate\n", symprefix);
	    fprintf( fdefine,  "#define yytmp %stmp\n", symprefix);
	    fprintf( fdefine,  "#define yytoks %stoks\n", symprefix);
	    fprintf( fdefine,  "#define yyv %sv\n", symprefix);
	    fprintf( fdefine,  "#define yyval %sval\n", symprefix);
        }

        /* sorry -- no yacc parser here.....
                we must bootstrap somehow... */

        for( t=gettok();  t!=MARK && t!= ENDFILE; ){
                switch( t ){

                case ';':
                        t = gettok();
                        break;

                case START:
                        if( (t=gettok()) != IDENTIFIER ){
                                error( MSGSTR(BADSTART,
                                        "bad %%start construction") ); /*MSG*/
                                }
                        start_sym = chfind(1,tokname);
                        t = gettok();
                        continue;

                case TYPEDEF:
                        if( (t=gettok()) != TYPENAME )
                                error( MSGSTR(BADSYNTAX,
                                        "bad syntax in %%type") ); /*MSG*/  
                        ty = numbval;
                        for(;;){
                                t = gettok();
                                switch( t ){

                                case IDENTIFIER:
                                        if((t=chfind(1, tokname)) < NTBASE) {
                                                j = TYPE( toklev[t] );
                                                if( j!= 0 && j != ty ){
                                                        error( MSGSTR(EREDECLR, "type redeclaration of token %s"), /*MSG*/  
                                                          tokset[t].name);
                                                        }
                                                else SETTYPE( toklev[t],ty);
                                                }
                                        else {
                                                j = nontrst[t-NTBASE].tvalue;
                                                if( j != 0 && j != ty ){
                                                        error(MSGSTR(EREDEC2, "type redeclaration of nonterminal %s"), /*MSG*/  
                                                                nontrst[t-NTBASE].name );
                                                        }
                                                else
                                                 nontrst[t-NTBASE].tvalue = ty;
                                                }
                                case ',':
                                        continue;

                                case ';':
                                        t = gettok();
                                        break;
                                default:
                                        break;
                                        }
                                break;
                                }
                        continue;

                case UNION:
                        /* copy the union declaration to the output */
                        cpyunion();
                        seen_union = 1;
                        t = gettok();
                        continue;

                case LEFT:
                case BINARY:
                case RIGHT:
                        ++i;
                case TERM:
                        lev = t-TERM;  /* nonzero means new prec. and assoc. */
                        ty = 0;

                        /* get identifiers so defined */

                        t = gettok();
                        if( t == TYPENAME ){ /* there is a type defined */
                                ty = numbval;
                                t = gettok();
                                }

                        for(;;) {
                                switch( t ){

                                case ',':
                                        t = gettok();
                                        continue;

                                case ';':
                                        break;

                                case IDENTIFIER:
                                        j = chfind(0,tokname);
                                        if( lev ){
                                                if( ASSOC(toklev[j]) ) error( MSGSTR(EPRECID, "redeclaration of precedence of %s"), tokname ); /*MSG*/  
                                                SETASC(toklev[j],lev);
                                                SETPLEV(toklev[j],i);
                                                }
                                        if( ty ){
                                                if( TYPE(toklev[j]) ) error( MSGSTR(ERETYPE, "redeclaration of type of %s"), tokname ); /*MSG*/  
                                                SETTYPE(toklev[j],ty);
                                                }
                                        if( (t=gettok()) == NUMBER ){
                                                tokset[j].value = numbval;
                                                if( j < ndefout && j>2 ){
                                                        error( MSGSTR(LATETYPE, "please define type number of %s earlier"), /*MSG*/  
                                                          tokset[j].name);
                                                        }
                                                t=gettok();
                                                }
                                        continue;

                                        }

                                break;
                                }

                        continue;

                case LCURLY:
                        defout();
                        cpycode();
                        t = gettok();
                        continue;

                default:
                        error( MSGSTR(SYNTAX, "syntax error") ); /*MSG*/  

                        }

                }

        if( t == ENDFILE ){
                error(MSGSTR(PEOF, "unexpected EOF before %%%%") ); /*MSG*/  
                }

        /* t is MARK */

        defout();
	if(PFlag){
		fprintf( ftable,  "#define yychar %schar\n", symprefix);
		fprintf( ftable,  "#define yyerrflag %serrflag\n", symprefix);
		fprintf( ftable,  "#define yyact %sact\n", symprefix);
		fprintf( ftable,  "#define yychk %schk\n", symprefix);
		fprintf( ftable,  "#define yydebug %sdebug\n", symprefix);
		fprintf( ftable,  "#define yydef %sdef\n", symprefix);
		fprintf( ftable,  "#define yyerror %serror\n", symprefix);
		fprintf( ftable,  "#define yyexca %sexca\n", symprefix);
		fprintf( ftable,  "#define yylex %slex\n", symprefix);
		fprintf( ftable,  "#define yylval %slval\n", symprefix);
		fprintf( ftable,  "#define yynerrs %snerrs\n", symprefix);
		fprintf( ftable,  "#define yypact %spact\n", symprefix);
		fprintf( ftable,  "#define yyparse %sparse\n", symprefix);
		fprintf( ftable,  "#define yypgo %spgo\n", symprefix);
		fprintf( ftable,  "#define yyps %sps\n", symprefix);
		fprintf( ftable,  "#define yypv %spv\n", symprefix);
		fprintf( ftable,  "#define yypvt %spvt\n", symprefix);
		fprintf( ftable,  "#define yyr1 %sr1\n", symprefix);
		fprintf( ftable,  "#define yyr2 %sr2\n", symprefix);
		fprintf( ftable,  "#define yyreds %sreds\n", symprefix);
		fprintf( ftable,  "#define yys %ss\n", symprefix);
		fprintf( ftable,  "#define yystate %sstate\n", symprefix);
		fprintf( ftable,  "#define yytmp %stmp\n", symprefix);
		fprintf( ftable,  "#define yytoks %stoks\n", symprefix);
		fprintf( ftable,  "#define yyv %sv\n", symprefix);
		fprintf( ftable,  "#define yyval %sval\n", symprefix);
                 
	}

        fprintf( ftable,  "#define yyclearin yychar = -1\n" );
        fprintf( ftable,  "#define yyerrok yyerrflag = 0\n" );
        fprintf( ftable,  "extern int yychar;\nextern int yyerrflag;\n" );
        fprintf( ftable,
          "#ifndef YYMAXDEPTH\n#define YYMAXDEPTH 150\n#endif\n" );
        if( !ntypes && !seen_union )
          fprintf( ftable,  "#ifndef YYSTYPE\n#define YYSTYPE int\n#endif\n" );
        fprintf( ftable,  "YYSTYPE yylval, yyval;\n" );
        fprintf( ftable,  "typedef int yytabelem;\n" );
        
        fprintf( ftable, "#include <stdio.h>\n" ); /* SHIX (93.1): because the program section would be moved          */
                                                   /* to the end of code generated by yacc, declaration of <stdio.h>   */
                                                   /* embedded in yy.lex.c would be also moved to the end of code.     */
                                                   /* This will cause a msg since wu have used printf() before wu      */
                                                   /* declare it. So we have to declare here.                          */ 

        prdptr[0]=mem;
        /* added production */
        *mem++ = NTBASE;
        *mem++ = start_sym;  /* if start_sym is 0, we will
                              * overwrite with the lhs of the first rule */
        *mem++ = 1;
        *mem++ = 0;
        prdptr[1]=mem;

        while( (t=gettok()) == LCURLY ) cpycode();

        if( t != C_IDENTIFIER )
                error( MSGSTR(SNTAX1, "bad syntax on first rule" )); /*MSG*/  

        if( !start_sym ) prdptr[0][1] = chfind(1,tokname);

        /* read rules */

        while( t!=MARK && t!=ENDFILE ){

                /* process a rule */

                if( t == '|' ){
                        rhsfill( (char *) 0 , 0);  /* restart fill of rhs */
                        *mem++ = *prdptr[nprod-1];
                        }
                else if( t == C_IDENTIFIER ){
                        *mem = chfind(1,tokname);
                        if( *mem < NTBASE )
                          error( MSGSTR(BADLHS,
                            "token illegal on LHS of grammar rule") ); /*MSG*/
                        ++mem;
                        lhsfill( tokname );     /* new rule: restart strings */
                        }
                else error( MSGSTR(BADDRULE,
                        "illegal rule: missing semicolon or | ?") ); /*MSG*/  

                /* read rule body */


                t = gettok();
        more_rule:
                while( t == IDENTIFIER ) {
                        *mem = chfind(1,tokname);
                        if( *mem<NTBASE ) levprd[nprod] = toklev[*mem];
                        rhsfill( tokname , *mem);     /* add to rhs string */
                        ++mem;
                        t = gettok();
                        }


                if( t == PREC ){
                        if( gettok()!=IDENTIFIER)
                                error( MSGSTR(BADPREC,
                                        "illegal %%prec syntax") ); /*MSG*/  
                        j = chfind(2,tokname);
                        if( j>=NTBASE)error(MSGSTR(NONTERM2,
                                "nonterminal %s illegal after %%prec"),
                                nontrst[j-NTBASE].name); /*MSG*/  
                        levprd[nprod]=toklev[j];
                        t = gettok();
                        }

                if( t == '=' ){
                        had_act[nprod] = 1;
                        levprd[nprod] |= ACTFLAG;
                        if (SplitFlag)
                                 fprintf(faction,"\nint _yyf%d() {",nprod);
                            else fprintf( faction, "\ncase %d:", nprod );
                        cpyact( mem-prdptr[nprod]-1 );
/* PTM 45382 Requested that cgram.y of CMDPROG pass lint tests. The fix was
             to yacc y2.c file to have the action switch's breaks preceeded
             by the NOTREACHED comment for lint to ignore the possible error.
        JRW 13/07/90
*/
                        if (!SplitFlag) fprintf( faction, " /*NOTREACHED*/ break;" );
                            else fprintf(faction,"\nreturn(-1);}");
                        if( (t=gettok()) == IDENTIFIER ){
                                /* action within rule... */

                                lrprnt();               /* dump lhs, rhs */
                                sprintf( actname, "$$%d", nprod );
                                j = chfind(1,actname); 
                                /* make it a nonterminal */

                                /* the current rule will become rule
                                 * number nprod+1 */
                                /* move the contents down,
                                 * and make room for the null */

                                for( p=mem; p>=prdptr[nprod]; --p ) p[2] = *p;
                                mem += 2;

                                /* enter null production for action */

                                p = prdptr[nprod];

                                *p++ = j;
                                *p++ = -nprod;

                                /* update the production information */

                                levprd[nprod+1] = levprd[nprod] & ~ACTFLAG;
                                levprd[nprod] = ACTFLAG;

                                if( ++nprod >= numprod ) OUTBUFMSG;
                                prdptr[nprod] = p;

                                /* make the action appear
                                 * in the original rule */
                                *mem++ = j;

                                /* get some more of the rule */

                                goto more_rule;
                                }

                        }

                while( t == ';' ) t = gettok();

                *mem++ = -nprod;

                /* check that default action is reasonable */

                if( ntypes && !(levprd[nprod]&ACTFLAG) &&
                        nontrst[*prdptr[nprod]-NTBASE].tvalue ){
                        /* no explicit action, LHS has value */
                        register tempty;
                        tempty = prdptr[nprod][1];
                        if( tempty < 0 ) error( MSGSTR(NOVALUE, "must return a value, since LHS has a type") ); /*MSG*/  
                        else if( tempty >= NTBASE )
                                tempty = nontrst[tempty-NTBASE].tvalue;
                        else tempty = TYPE( toklev[tempty] );
                        if( tempty != nontrst[*prdptr[nprod]-NTBASE].tvalue ){
                                error( MSGSTR(TCLASH, "default action causes potential type clash") ); /*MSG*/  
                                }
                        }

                if( ++nprod >= numprod ) OUTBUFMSG;
                prdptr[nprod] = mem;
                levprd[nprod]=0;

                }

        /* end of all rules */

        end_reds();
        out_debug();
        end_debug();            /* finish fdebug file's input */
        finact();
        }

getmem(){
        int tbitsize, i, j, *tmpwset;

        mem0 = (int *) calloc(sizeof(int), memsize);
        amem = (int *) calloc(sizeof(int), actsize);
        memp = amem;
        cnamp = (char *) calloc(sizeof(char), cnamsz);
        tokset = (struct toksymb *) calloc(sizeof(struct toksymb), nterms);
        toklev = (int *) calloc(sizeof(int), nterms);
        nontrst = (struct ntsymb *) calloc(sizeof(struct ntsymb), nnonterms);
        yypgo = &nontrst[0].tvalue;
        prdptr = (int **) calloc(sizeof(int *), numprod);
        levprd = (int *) calloc(sizeof(int), numprod);
        had_act = (char *) calloc(sizeof(char), numprod);
        pstate = (struct item **) calloc(sizeof(struct item), (nstates + 2));
        tystate = (int *) calloc(sizeof(int), nstates);
        defact = (int *) calloc(sizeof(int), nstates);
        tstates = (int *) calloc(sizeof(int), nterms);
        ntstates = (int *) calloc(sizeof(int), nnonterms);
        mstates = (int *) calloc(sizeof(int), nstates);
        wsets = (struct wset *) calloc(sizeof(struct wset), wsetsize);
        tbitsize = TBITSET;
        tmpwset = (int *) calloc(sizeof(int), wsetsize * tbitsize);
        clset.lset = (int *) calloc(sizeof(int), tbitsize);
        indgo = (int *) calloc(sizeof(int), nstates);
        temp1 = (int *) calloc(sizeof(int), tempsize);
        pres = (int ***) calloc(sizeof(int **), (nnonterms + 2));
        pfirst = (struct looksets **) calloc(sizeof(struct looksets *), (nnonterms + 2));
        pempty = (int *) calloc(sizeof(int), (nnonterms + 1));
        if (!mem0 || !amem || !cnamp || !tokset || !toklev || !nontrst || !prdptr || !levprd || !had_act || !pstate || !tystate || !defact || !tstates || !ntstates || !mstates || !wsets || !tmpwset || !clset.lset || !indgo || !temp1 || !pres || !pfirst || !pempty)
                error(MSGSTR(OUTMEM, "Out of Memory" ));

        for(i=0; i<wsetsize; i++, tmpwset+=tbitsize)
        {
                wsets[i].ws.lset = tmpwset;
        }
        pgo = wsets[0].ws.lset;
}

finact(){
        /* finish action routine */

        fclose(faction);

        fprintf( ftable, "# define YYERRCODE %d\n", tokset[2].value );

        }

defin( t, s ) register char  *s; {
/*      define s to be a terminal if t=0
        or a nonterminal if t=1         */

        register val, i;

       if (t) {
                if( ++nnonter >= nnonterms ) OUTBUFMSG;
                nontrst[nnonter].name = cstash(s);
                return( NTBASE + nnonter );
                }
        /* must be a token */
        if( ++ntokens >= nterms ) OUTBUFMSG;
        tokset[ntokens].name = cstash(s);

        /* establish value for token */

        if( s[0]==' ' && s[2]=='\0' ) /* single character literal */
                val = s[1];
        else if ( s[0]==' ' && s[1]=='\\' ) { /* escape sequence */
                if( ISOCTAL(s[2]) ){ /* octal sequence */
                        i = 3;
                        val = s[2] - '0';
                        while( ISOCTAL(s[i]) && i<5 ) {
                                val = val * 8 + (s[i] - '0');
                                i++;
                                }
                        if(( s[i] != '\0' ) || ( val == 0 ))
                                error(MSGSTR(ESCINVAL,
                                "The character escape sequence is not valid."));
                        }
                else if( s[3] == '\0' ){ /* single character escape sequence */
                        switch ( s[2] ){
                                         /* character which is escaped */
                        case 'n': val = '\n'; break;
                        case 'r': val = '\r'; break;
                        case 'b': val = '\b'; break;
			case 'a': val = '\a'; break;/* SHIX(93.1) '\v' and '\a' are added to escape sequence */
			case 'v': val = '\v'; break;
                        case 't': val = '\t'; break;
                        case 'f': val = '\f'; break;
                        case '\'': val = '\''; break;
                        case '"': val = '"'; break;
                        case '?': val = '?'; break;
                        case '\\': val = '\\'; break;
                        default: error( MSGSTR(ESCINVAL,
                                        "invalid escape") ); /*MSG*/  
                                }
                        }
                else if( s[2] == 'x' ) { /* hexadecimal escape sequence */
                        i = 3;
                        val = 0;
                        while( ISHEX(s[i]) && i<5 ) {
                                if (s[i] >= '0' && s[i] <= '9')
                                        val = val * 16 + (s[i] - '0');
                                else if (s[i] >= 'A' && s[i] <= 'F')
                                        val = val * 16 + (s[i] - 'A' + 10);
                                else
                                        val = val * 16 + (s[i] - 'a' + 10);
                                i++;
                                }
                        if(( s[i] != '\0' ) || ( val == 0 ))
                                error(MSGSTR(ESCINVAL,
                                "The character escape sequence is not valid."));
                        }
                else
                        error(MSGSTR(ESCINVAL,
                        "The character escape sequence is not valid."));
                }
        else {
                val = extval++;
                }
        tokset[ntokens].value = val;
        toklev[ntokens] = 0;
        return( ntokens );
        }

defout(){ /* write out the defines (at the end of the declaration section) */

        register int i, c;
        register char *cp;

        for( i=ndefout; i<=ntokens; ++i ){

                cp = tokset[i].name;
                if( *cp == ' ' )        /* literals */
                {
                        /*D50464 - yacc was including #defines for some
                        characters, but not others (eg. 'x' would be defined,
                        but not '+').  Now yacc does not include #defines for
                        any single character.
                        Note: This implementation of yacc allows multicharacter
                              literals (eg. '++')?; a single character literal
                              is defined as: (cp[0] == ' ' && cp[2] == '\0').*/
                        if (cp[2] == '\0')
                             goto nodef;

                        cp++; /* in my opinion, this should be continue */
                }

                for( ; (c= *cp)!='\0'; ++cp ){

                        if( islower(c) || isupper(c) || isdigit(c) || c=='_' );
                         /* VOID */
                        else goto nodef;
                        }

                fprintf( ftable, "# define %s %d\n", tokset[i].name,
                        tokset[i].value );
                if( fdefine != NULL ) fprintf( fdefine, "# define %s %d\n",
                        tokset[i].name, tokset[i].value );

        nodef:  ;
                }

        ndefout = ntokens+1;

        }

char *
cstash( s ) register char *s; {
        char *temp;

        temp = cnamp;
        do {
                if( cnamp >= cnamb ) error(MSGSTR(OUTNMSPC,"use -Nn flag to increase name space size") ); /*MSG*/
                else *cnamp++ = *s;
                }  while ( *s++ );
        return( temp );
        }

gettok() {
        register i, base;
        static int peekline; /* number of '\n' seen in lookahead */
        register c, match, reserve;

begin:
        reserve = 0;
        lineno += peekline;
        peekline = 0;
        c = getc(fyinput);
        while( c==' ' || c=='\n' || c=='\t' || c=='\f' ){
                if( c == '\n' ) ++lineno;
                c=getc(fyinput);
                }
        if( c == '/' ){ /* skip comment */
                lineno += skipcom();
                goto begin;
                }

        switch(c){

        case EOF:
                return(ENDFILE);
        case '{':
                ungetc( c, fyinput );
                return( '=' );  /* action ... */
        case '<':  /* get, and look up, a type name (union member name) */
                i = 0;
                while( (c=getc(fyinput)) != '>' && c>=0 && c!= '\n' ){
                        tokname[i] = c;
                        if( ++i >= NAMESIZE ) --i;
                        }
                if( c != '>' ) error( MSGSTR(UNTERM,
                        "unterminated < ... > clause" )); /*MSG*/  
                tokname[i] = '\0';
                for( i=1; i<=ntypes; ++i ){
                        if( !strcmp( typeset[i], tokname ) ){
                                numbval = i;
                                return( TYPENAME );
                                }
                        }
                typeset[numbval = ++ntypes] = cstash( tokname );
                return( TYPENAME );

        case '"':       
        case '\'':
                match = c;
                tokname[0] = ' ';
                i = 1;
                for(;;){
                        c = getc(fyinput);
                        if( c == '\n' || c == EOF )
                                error(MSGSTR(MISSQUOTE,
                                  "illegal or missing ' or \"") ); /*MSG*/  
                        if( c == '\\' ){
                                c = getc(fyinput);
                                tokname[i] = '\\';
                                if( ++i >= NAMESIZE ) --i;
                                }
                        else if( c == match ) break;
                        tokname[i] = c;
                        if( ++i >= NAMESIZE ) --i;
                        }
                break;

        case '%':
        case '\\':

                switch(c=getc(fyinput)) {

                case '0':       return(TERM);
                case '<':       return(LEFT);
                case '2':       return(BINARY);
                case '>':       return(RIGHT);
                case '%':
                case '\\':      return(MARK);
                case '=':       return(PREC);
                case '{':       return(LCURLY);
                default:        reserve = 1;
                        }

        default:

                if( isdigit(c) ){ /* number */
                        numbval = c-'0' ;
                        base = (c=='0') ? 8 : 10 ;
                        for( c=getc(fyinput); isdigit(c) ; c=getc(fyinput) ){
                                numbval = numbval*base + c - '0';
                                }
                        ungetc( c, fyinput );
                        return(NUMBER);
                        }
                else if( islower(c) || isupper(c) || c=='_' ||
                         c=='.' || c=='$' ){
                        i = 0;
                        while( islower(c) || isupper(c) || isdigit(c) ||
                               c=='_' || c=='.' || c=='$' ){
                                tokname[i] = c;
                                if( reserve && isupper(c) )
                                        tokname[i] += 'a'-'A';
                                if( ++i >= NAMESIZE ) --i;
                                c = getc(fyinput);
                                }
                        }
                else return(c);

                ungetc( c, fyinput );
                }

        tokname[i] = '\0';

        if( reserve ){ /* find a reserved word */
                if( !strcmp(tokname,"term")) return( TERM );
                if( !strcmp(tokname,"token")) return( TERM );
                if( !strcmp(tokname,"left")) return( LEFT );
                if( !strcmp(tokname,"nonassoc")) return( BINARY );
                if( !strcmp(tokname,"binary")) return( BINARY );
                if( !strcmp(tokname,"right")) return( RIGHT );
                if( !strcmp(tokname,"prec")) return( PREC );
                if( !strcmp(tokname,"start")) return( START );
                if( !strcmp(tokname,"type")) return( TYPEDEF );
                if( !strcmp(tokname,"union")) return( UNION );
                error(MSGSTR(ILLWORD,
                        "invalid escape, or illegal reserved word: %s"),
                        tokname ); /*MSG*/  
                }

        /* look ahead to distinguish IDENTIFIER from C_IDENTIFIER */

        c = getc(fyinput);
        while( c==' ' || c=='\t'|| c=='\n' || c=='\f' || c== '/' ) {
                if( c == '\n' ) ++peekline;
                else if( c == '/' ){ /* look for comments */
                        peekline += skipcom();
                        }
                c = getc(fyinput);
                }
        if( c == ':' ) return( C_IDENTIFIER );
        ungetc( c, fyinput );
        return( IDENTIFIER );
}

fdtype( t ){ /* determine the type of a symbol */
        register v;
        if( t >= NTBASE ) v = nontrst[t-NTBASE].tvalue;
        else v = TYPE( toklev[t] );
        if( v <= 0 ) error( MSGSTR(NOTYPE, "must specify type for %s"),
                (t>=NTBASE)?nontrst[t-NTBASE].name:
                        tokset[t].name ); /*MSG*/  
        return( v );
        }

chfind( t, s ) register char *s; {
        int i;

        if (s[0]==' ')t=0;
        TLOOP(i){
                if(!strcmp(s,tokset[i].name)){
                        return( i );
                        }
                }
        NTLOOP(i){
                if(!strcmp(s,nontrst[i].name)) {
                        return( i+NTBASE );
                        }
                }
        /* cannot find name */
        if( t>1 )
                error( MSGSTR(NONAME, "%s should have been defined earlier"),
                        s ); /*MSG*/  
        return( defin( t, s ) );
        }

cpyunion(){
        /* copy the union declaration to the output,
         * and the define file if present */

        int level, c;
        if ( gen_lines )
                fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
        fprintf( ftable, "typedef union " );
        if( fdefine ) fprintf( fdefine, "\ntypedef union " );

        level = 0;
        for(;;){
                if( (c=getc(fyinput)) < 0 ) 
                        error( MSGSTR(EOFENC,
                          "EOF encountered while processing %%union")); /*MSG*/
                putc( c, ftable );
                if( fdefine ) putc( c, fdefine );

                switch( c ){

                case '\n':
                        ++lineno;
                        break;

                case '{':
                        ++level;
                        break;

                case '}':
                        --level;
                        if( level == 0 ) { /* we are finished copying */
                                fprintf( ftable, " YYSTYPE;\n" );
                                if( fdefine ) fprintf( fdefine,
                                  " YYSTYPE;\nextern YYSTYPE yylval;\n" );
                                return;
                                }
                        }
                }
        }

cpycode(){ /* copies code between \{ and \} */

        int c;
        c = getc(fyinput);
        if( c == '\n' ) {
                c = getc(fyinput);
                lineno++;
                }
        if ( gen_lines )
                fprintf( ftable, "\n# line %d \"%s\"\n", lineno, infile );
        while( c>=0 ){
                if( c=='\\' )
                        if( (c=getc(fyinput)) == '}' ) return;
                        else putc('\\', ftable );
                if( c=='%' )
                        if( (c=getc(fyinput)) == '}' ) return;
                        else putc('%', ftable );
                putc( c , ftable );
                if( c == '\n' ) ++lineno;
                c = getc(fyinput);
                }
        error(MSGSTR(FEOF, "eof before %%}") ); /*MSG*/  
        }

skipcom(){ /* skip over comments */
        register c, i=0;  /* i is the number of lines skipped */

        /* skipcom is called after reading a / */

        if( getc(fyinput) != '*' ) error( MSGSTR(BADCOMNT,
                "illegal comment") ); /*MSG*/  
        c = getc(fyinput);
        while( c != EOF ){
                while( c == '*' ){
                        if( (c=getc(fyinput)) == '/' ) return( i );
                        }
                if( c == '\n' ) ++i;
                c = getc(fyinput);
                }
        error( MSGSTR(COMTEOF, "EOF inside comment") ); /*MSG*/  
        /* NOTREACHED */
        }

cpyact(offset){ /* copy C action to the next ; or closing } */
        int brac, c, match, j, s, tok;

        if ( gen_lines )
                fprintf( faction, "\n# line %d \"%s\"\n", lineno, infile );

        brac = 0;

loop:
        c = getc(fyinput);
swt:
        switch( c ){

case ';':
                if( brac == 0 ){
                        putc( c , faction );
                        return;
                        }
                goto lcopy;

case '{':
                brac++;
                goto lcopy;

case '$':
                s = 1;
                tok = -1;
                c = getc(fyinput);
                if( c == '<' ){ /* type description */
                        ungetc( c, fyinput );
                        if( gettok() != TYPENAME ) error( MSGSTR(BADIDENT,
                          "bad syntax on $<ident> clause") ); /*MSG*/  
                        tok = numbval;
                        c = getc(fyinput);
                        }
                if( c == '$' ){
                        fprintf( faction, "yyval");
                        if( ntypes ){ /* put out the proper tag... */
                                if( tok < 0 ) tok = fdtype( *prdptr[nprod] );
                                fprintf( faction, ".%s", typeset[tok] );
                                }
                        goto loop;
                        }
                if( c == '-' ){
                        s = -s;
                        c = getc(fyinput);
                        }
                if( isdigit(c) ){
                        j=0;
                        while( isdigit(c) ){
                                j= j*10+c-'0';
                                c = getc(fyinput);
                                }

                        j = j*s - offset;
                        if( j > 0 ){
                                error( MSGSTR(ILLOFF, "Illegal use of $%d"),
                                        j+offset ); /*MSG*/  
                                }

                        fprintf( faction, "yypvt[-%d]", -j );
                        if( ntypes ){ /* put out the proper tag */
                                if( j+offset <= 0 && tok < 0 )
                                        error( MSGSTR(NOTYPE2,
                                                "must specify type of $%d"),
                                                j+offset ); /*MSG*/  
                                if( tok < 0 )
                                  tok = fdtype( prdptr[nprod][j+offset] );
                                fprintf( faction, ".%s", typeset[tok] );
                                }
                        goto swt;
                        }
                putc( '$' , faction );
                if( s<0 ) putc('-', faction );
                goto swt;

case '}':
                if( --brac ) goto lcopy;
                putc( c, faction );
                return;


case '/':       /* look for comments */
                putc( c , faction );
                c = getc(fyinput);
                if( c != '*' ) goto swt;

                /* it really is a comment */

                putc( c , faction );
                c = getc(fyinput);
                while( c != EOF ){
                        while( c=='*' ){
                                putc( c , faction );
                                if( (c=getc(fyinput)) == '/' ) goto lcopy;
                                }
                        putc( c , faction );
                        if( c == '\n' ) ++lineno;
                        c = getc(fyinput);
                        }
                error( MSGSTR(COMTEOF, "EOF inside comment") ); /*MSG*/  

case '\'':      /* character constant */
                match = '\'';
                goto string;

case '"':       /* character string */
                match = '"';

        string:

                putc( c , faction );
                while( c=getc(fyinput) ){

                        if( c=='\\' ){
                                putc( c , faction );
                                c=getc(fyinput);
                                if( c == '\n' ) ++lineno;
                                }
                        else if( c==match ) goto lcopy;
                        else if( c=='\n' ) error( MSGSTR(NLINSTR,
                                "newline in string or char. const.") ); /*MSG*/
                        putc( c , faction );
                        }
                error( MSGSTR(EOFINSTR,
                        "EOF in string or character constant") ); /*MSG*/  

case EOF:
                error(MSGSTR(NONTERMACT, "action does not terminate")); /*MSG*/

case '\n':      ++lineno;
                goto lcopy;

                }

lcopy:
        putc( c , faction );
        goto loop;
        }


#define RHS_TEXT_LEN            ( BUFSIZ * 4 )  /* length of rhstext */

char lhstext[ BUFSIZ ];         /* store current lhs (non-terminal) name */
char rhstext[ RHS_TEXT_LEN ];   /* store current rhs list */

lhsfill( s )    /* new rule, dump old (if exists), restart strings */
        char *s;
{
        rhsfill( (char *) 0 , 0);
        strcpy( lhstext, s );   /* don't worry about too long of a name */
}

/*
 * Getstring - gets the appropriate escape sequence for a character.
 *           - called from either rhsfill() or out_debug()
 *
 * The following strings are processed by the compiler twice, once
 * when this file is compiled and once when y.tab.c is compiled
 * with -DYYDEBUG, thus the extra '\' characters in order to get
 * the correct final output.
*/

char* getstring(char c)
{
        switch(c) {
        case '\n':
                return("\\\\n");
        case '\r':
                return("\\\\r");
        case '\b':
                return("\\\\b");
        case '\a':
                return("\\\\a");
        case '\v':
                return("\\\\v");
        case '\t':
                return("\\\\t");
        case '\f':
                return("\\\\f");
        case '"' :
                return("\\\"");
        case '\'' :
                return("\\\\'");
        case '\\' :
                return("\\\\\\\\");
        }
}

rhsfill( s , index)
        char *s;        /* either name or 0 */
        int index;
{
        static char *loc = rhstext;     /* next free location in rhstext */
        register char *p;
        char c;

        if ( !s )       /* print out and erase old text */
        {
                if ( *lhstext )         /* there was an old rule - dump it */
                        lrprnt();
                ( loc = rhstext )[0] = '\0';
                return;
        }
        /* add to stuff in rhstext */
        if ( loc > &rhstext[ RHS_TEXT_LEN ] - 8 )
                return;
        p = s;
        *loc++ = ' ';
        if ( *s == ' ' )        /* special quoted symbol */
        {
                *loc++ = '\'';  /* add first quote */
                c = tokset[index].value;
                switch (c) {
                case '\n':
                case '\r':
                case '\b':
                case '\a':
                case '\v':
                case '\t':
                case '\f':
                case '"' :
                case '\'' :
                case '\\' :
                        strcpy(loc, getstring(c));
                        loc += strlen(loc);
                        break;
                default :
                        if (!isprint(c) || c == ' ')
                        {
                                sprintf(loc, "\\\\%03o", c);
                                loc += strlen(loc);
                        }
                        else
                        {
                                *loc = c;
                                loc++;
                        }
                } /* end switch(c) */
                        
                *loc++ = '\'';
        }
        else
        {
                while ( *loc = *p++ )
                        if ( loc++ > &rhstext[ RHS_TEXT_LEN ] - 2 )
                                break;
        } /* end if(*s == ' ') */
        *loc = '\0';            /* terminate the string */
}


lrprnt()        /* print out the left and right hand sides */
{
        char *rhs;

        if ( !*rhstext )                /* empty rhs - print usual comment */
                rhs = " /* empty */";
        else
                rhs = rhstext;
        fprintf( fdebug, "      \"%s :%s\",\n", lhstext, rhs );
}

out_debug()
{
        int i;

#define PDEBUG(str, val) fprintf(fdebug, "\t\"%s\",\t%d,\n", str, val)
#define PLITERAL(str, val) fprintf(fdebug, "\t\"\'%s\'\",\t%d,\n", str, val)

        for(i=NDEFOUT; i<=ntokens; ++i)
        {
                if(tokset[i].name[0] == ' ')
                {
                        switch (tokset[i].value) {
                        case '\n':
                        case '\r':
                        case '\b':
                        case '\a':
                        case '\v':
                        case '\t':
                        case '\f':
                        case '"' :
                        case '\'' :
                        case '\\' :
                                PLITERAL(getstring(tokset[i].value), tokset[i].value);
                                break;
                        default:
                                if (!isprint(tokset[i].value) || tokset[i].value == ' ')
                                {
                                        fprintf(fdebug, "\t\"\'\\\\%03o\'\",\t%d,\n", tokset[i].value, tokset[i].value);
                                }
                                else
                                {
                                        PLITERAL(tokset[i].name + 1, tokset[i].value);
                                }
                        }
                }
                else
                        PDEBUG(tokset[i].name, tokset[i].value);
        }
#undef PDEBUG
#undef PLITERAL

}


beg_debug()     /* dump initial sequence for fdebug file */
{
        fprintf( fdebug,
                "typedef struct { char *t_name; int t_val; } yytoktype;\n" );
        fprintf( fdebug,
                "#ifndef YYDEBUG\n#\tdefine YYDEBUG\t%d", gen_testing );
        fprintf( fdebug, "\t/*%sallow debugging */\n#endif\n\n",
                gen_testing ? " " : " don't " );
        fprintf( fdebug, "#if YYDEBUG\n\n" );
        fprintf( fdebug, "char * yyreds[] =\n{\n" );
        fprintf( fdebug, "\t\"-no such reduction-\",\n" );
}


end_reds()      /* finish yyreds array, get ready for yytoks array */
{
        lrprnt();               /* dump last lhs, rhs */
        fprintf( fdebug, "};\n" );
        fprintf( fdebug, "yytoktype yytoks[] =\n{\n" );
}


end_debug()     /* finish yytoks array, close file */
{
        fprintf( fdebug, "\t\"-unknown-\",\t-1\t/* ends search */\n" );
        fprintf( fdebug, "};\n#endif /* YYDEBUG */\n" );
        fclose( fdebug );
}
