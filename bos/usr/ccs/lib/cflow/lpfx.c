static char sccsid[] = "@(#)40  1.6.1.3  src/bos/usr/ccs/lib/cflow/lpfx.c, cmdprog, bos411, 9428A410j 11/30/93 09:02:19";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: CflowPass2, CloseFile, GetName, InHeader, InSymbol, InType,
	      InUsage, main, OpenFile, OutputDef, OutputProto, OutputRef,
	      tprint
 *
 * ORIGINS: 3 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "mfile1.h"
#include "error.h"
#include <stdio.h>
#include <locale.h>

/* Basic symbol table entry. */
typedef struct smtbl {
	char	sname[BUFSIZ];	/* symbol name */
	char	ifname[BUFSIZ];	/* included filename */
	TPTR	type;		/* type information */
	short	line;		/* line number (definition) */
	short	usage;		/* usage flags */
} SMTAB;

/* Global symbols. */
SMTAB theSym, theFtn;
SMTAB *curSym;			/* current symbol */
SMTAB *curFtn;			/* current encapsulating function */
char *fname;			/* actual file name opened */
int markerEOF;			/* end of file marker */
int xflag;			/* external symbol disabled */
int uflag;			/* underscore names disabled */
int pflag;			/* prototypes enabled */

/* Function declarations. */
char *GetName();
TPTR InType();

main(argc, argv)
int argc;
register char *argv[];
{
	register int i, c;
	register char *cp;
	register int r;
	int fdef = 0;

	setlocale(LC_ALL, "");
	xflag = 0; uflag = 0; pflag = 1;	/* default toggles */

	for( i=1; i<argc; ++i ){
		if( *(cp=argv[i]) == '-' )
		while( *++cp ){
			if (*cp == 'i') {	/* process toggles */
				switch (*++cp) {
				case 'x':
					xflag = 1;	/* external symbols */
					break;
				case '_':
					uflag = 1;	/* allow _name */
					break;
				case 'p':
					pflag = 0;	/* ignore prototypes */
					break;
				}
			}
		}

		else {
			/* First-time initializations. */
			if (fdef == 0) {
				InitTypes();
				fdef = 1;
				curSym = &theSym;
				curFtn = &theFtn;
			}
			fname = argv[i];
			OpenFile();
			CflowPass2();
			CloseFile();
		}
	}
        return(0);
}

CflowPass2()
{
	int inftn = 0;

	/* Read and process all symbol records. */
	while (1) {
		switch (InHeader()) {
		case CFLOWEOF:
			/* EOF delimiter. */
			if (markerEOF)
				return;
			markerEOF = 1;
			continue;

		case LINTADD:
			/* End of function definition. */
			InUsage();
			inftn = 0;
			break;

		case LINTSYM:
			/* Fetch symbol. */
			InSymbol();

			/* Default: omit names with underscores. */
			if (!uflag && *curSym->sname == '_')
				break;

			/* Default: ignore non-function symbols. */
			if (!xflag && !ISFTN(curSym->type))
				break;

			/* Emit reference. */
			if (inftn && strcmp(curFtn->sname, curSym->sname))
				OutputRef(curFtn, curSym);

			/* Emit definitions. */
			if (curSym->usage&LINTDEF) {
				if (!inftn && ISFTN(curSym->type)) {
					memcpy((char *)curFtn, (char *)curSym,
						sizeof(SMTAB));
					inftn = 1;
				}
				OutputDef(curSym);
				break;
			}

			/* Function prototypes. */
			if (pflag && !inftn && ISFTN(curSym->type) &&
				curSym->usage&LINTREF)
				OutputProto(curSym);
			break;

		default:
			cerror("unknown record directive in file %s", fname);
		}
	}
}

FILE *fp;			/* current file pointer */

#define BIO 1			/* binary i/o selected */

/*
** Header distinguishes file partitions from symbol data.
*/
InHeader()
{
	char iocode;

	/* Read file delimiter record. */
#ifdef	BIO
	if (fread((char *) &iocode, sizeof(char), 1, fp) < 1) {
#else
	if (fscanf(fp, "%d", &iocode) == EOF) {
#endif
		if (markerEOF)
			return ((int) CFLOWEOF);
		cerror("unexpected EOF in file %s", fname);
	}
	
	/* Handle case where delimiter indicates a new file. */
	if (iocode == CFLOWBOF) {
		(void) GetName();
		/* Read for next record (assumed symbol). */
#ifdef	BIO
		fread((char *) &iocode, sizeof(char), 1, fp);
#else
		fscanf(fp, "%d", &iocode);
#endif
		markerEOF = 0;
	}
	return ((int) iocode);
}

/*
** Read characer string from intermediate file.
*/
char *
GetName()
{
	static char buf[BUFSIZ];
	register char *cp = (char *) buf;
	register int c;

#ifdef	BIO
	while ((c = fgetc(fp)) != EOF) {
		*cp++ = c;
		if (c == '\0')
			break;
	}
#else
	fscanf(fp, "%s", cp);
#endif
	return (buf);
}

/*
** Read function usage symbol record.
*/
InUsage()
{
	short usage;

	(void) GetName();
#ifdef	BIO
	fread((char *) &usage, sizeof(short), 1, fp);
#else
	fscanf(fp, "%d", &usage);
#endif
}

/*
** Read symbol record.
*/
InSymbol()
{
	int line;

	strcpy(curSym->sname, GetName());
	strcpy(curSym->ifname, GetName());

#ifdef	BIO
	fread((char *) &curSym->line, sizeof(short), 1, fp);
	fread((char *) &line, sizeof(short), 1, fp);
	fread((char *) &curSym->usage, sizeof(short), 1, fp);
#else
	fscanf(fp, "%d", &curSym->line);
	fscanf(fp, "%d", &line);
	fscanf(fp, "%d", &curSym->usage);
#endif
	curSym->type = InType();
}

/*
** Get the type of the current symbol.
*/
TPTR
InType()
{
	register TPTR t;
	register PPTR p;
	TPTR ot;
#ifdef	BIO
	struct tyinfo ty;
#else
	unsigned tnext, info, type, pnext;
#endif

#ifdef	BIO
	fread((char *) &ty, sizeof(struct tyinfo), 1, fp);
	if ((t = FindBType(ty.tword)) == TNIL)
		t = tynalloc(ty.tword);
#else
	fscanf(fp, "%o %o %o", &type, &tnext, &info);
	if ((t = FindBType(type)) == TNIL)
		t = tynalloc(type);
#endif
	ot = t;

#ifdef	BIO
	while (ty.next != TNIL) {
 		if (ISFTN(t)) {
			if (ty.ftn_parm != PNIL) {	/* is a PPTR */
#else
	while (tnext) {
 		if (ISFTN(t)) {
			if (info) {	/* is a PPTR */
#endif
				t->ftn_parm = p = parmalloc();
				do {
#ifdef	BIO
					fread((char *) p,
					       sizeof(struct parminfo), 1, fp);
					p->type = InType();
					if (p->next == PNIL)
#else
					fscanf(fp, "%o %o", &type, &pnext);
					p->type = InType();
					if (!pnext)
#endif
						break;
					p->next = parmalloc();
				} while (p = p->next);
			}
 		}
		else if (ISARY(t))
#ifdef	BIO
			t->ary_size = ty.ary_size;
#else
			t->ary_size = info;
#endif
#ifdef	BIO
		fread((char *) &ty, sizeof(struct tyinfo), 1, fp);
		if ((t->next = FindBType(ty.tword)) == TNIL)
			t->next = tynalloc(ty.tword);
#else
		fscanf(fp, "%o %o %o", &type, &tnext, &info);
		if ((t->next = FindBType(type)) == TNIL)
			t->next = tynalloc(type);
#endif
		t = t->next;
	}
	return (ot);
}

/*
** Simple file open/close functions.
*/
OpenFile()
{
	if ((fp = fopen(fname, "r")) == NULL) {
		cerror("can't open file %s\n", fname);
		exit(1);
	}
	markerEOF = 0;
}

CloseFile()
{
	fclose(fp);
}

/*
** Output a symbol reference.
*/
OutputRef(p, q)
	SMTAB *p, *q;
{
	printf("%s : %s\n", p->sname, q->sname);
}

/*
** Output a symbol definition.
*/
OutputDef(p)
	SMTAB *p;
{
	printf("%s = ", p->sname);
	tprint(p->type);
	printf(", <%s %d>\n", p->ifname, p->line);
}

/*
** Output a function prototype.
*/
OutputProto(p)
	SMTAB *p;
{
/* PTM 45503 changed the id char from '=' to '#'. See dag.c for complete
	fix note.
	JRW 13/03/90
*/
	printf("%s # ", p->sname);
	tprint(p->type);
	printf(", <>\n");
}

/*
** Output a description of the type t.  This function must remain
** consistent with the ordering in pcc/m_ind/mfile1.h .  The same
** function exists in m_ind/treewalk.h .
*/
tprint(t)
	TPTR t;
{
	register PPTR p;
	TWORD bt;
	static char *tnames[NBTYPES] = {
		"null",
		"ellipsis",
		"int",		/* well, really "farg" */
		"moety",
		"signed",
		"undef",
		"void",
		"char",
		"signed char",
		"short",
		"int",
		"long",
                "long long",
		"float",
		"double",
		"long double",
		"unsigned char",
		"unsigned short",
		"unsigned",
		"unsigned long",
                "unsigned long long",
		"enum",
		"struct",
		"union",
		"class"
	};

	for( ;; t = DECREF(t) ){

		if( ISCONST(t) ) printf( "const " );
		if( ISVOLATILE(t) ) printf( "volatile " );

		if( ISPTR(t) ) printf( "* " );
 		else if( ISFTN(t) ){
 			printf( "(" );
			if( ( p = t->ftn_parm ) != PNIL ){
				for( ;; ){
					tprint( p->type );
					if( ( p = p->next ) == PNIL ) break;
					printf( ", " );
				}
 			}
			printf( ") " );
 		}
		else if( ISARY(t) ) printf( "[%.0d] ", t->ary_size );
		else if( ISREF(t) ) printf( "& " );
		else if( ISMEMPTR(t) ) printf( "::* " );
		else {
			if( ISTSIGNED(t) ) printf( "<signed> " );
			if( HASCONST(t) ) printf( "<has const> " );
			if( HASVOLATILE(t) ) printf( "<has volatile> " );
			printf( tnames[bt = TOPTYPE(t)] );
			return;
		}
	}
}

