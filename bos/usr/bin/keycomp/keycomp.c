/* @(#)61	1.9  src/bos/usr/bin/keycomp/keycomp.c, cmdimkc, bos411, 9428A410j 3/24/94 05:37:55 */
/*
 * COMPONENT_NAME : (cmdimkc) AIX Input Method Keymap Compiler
 *
 * FUNCTIONS : keycomp
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <X11/keysym.h>
#include "im.h"
#include "imP.h"
#include "dim.h"

/*
 *	Message Catalog stuffs.
 */
#include <nl_types.h>
#include "keycomp_msg.h"
nl_catd	catd;
#define	MSGSTR(Num,Str)	catgets(catd, MS_KEYCOMP, Num, Str)

#define	E_MEM	"keycomp : cannot allocate memory\n"
#define	E_LINE	"keycomp : error at line %d\n"
#define	E_ITEM	"keycomp : error at line %1$d item %2$d\n"
#define	E_MIX	"keycomp : %%M and #M/#S cannot be mixed\n"
#define E_USG	"Usage: keycomp {-c}\n"


/*
 *	CLine is a control line which start with #S, #M or %M.
 */
#define	CL_ALLOC_UNIT	32

typedef struct _CLine	{
	int	len;
	int	siz;
	short	*stat;
}	CLine;

/*
 *	sline is a line starting with #S
 */
static CLine	sline = {0, 0, NULL};

/*
 *	MLines is an array of mlines.
 */
#define	MLS_ALLOC_UNIT	32

typedef struct _MLines	{
	int	len;
	int	siz;
	CLine	*mline;
}	MLines;

static MLines	mlines = {0, 0, NULL};

/*
 *	PLine is a line containing keyboard mapping data.
 */
#define	PL_ALLOC_UNIT	32

typedef struct _PLine	{
	unsigned int	keysym;
	int	len;
	int	siz;
	KeyMapElement	*kme;
}	PLine;

static int	maxplen = 0;

/*
 *	PLines is an array of Pline.
 */
#define	PLS_ALLOC_UNIT	128

typedef struct _PLines	{
	int	len;
	int	siz;
	PLine	*pline;
}	PLines;

static PLines	plines = {0, 0, NULL};

/*
 *	mflag
 *	MFLAG_NONE : initial state
 *	MFLAG_NEW	: new format (%M)
 *	MFLAG_OLD	: old format (#S and #M)
 */
#define	MFLAG_NONE	0
#define	MFLAG_NEW	1
#define	MFLAG_OLD	2
static int	mflag = MFLAG_NONE;

/*
 *	current line number and current item number.
 *	used when issueing error message.
 */
static int	line = 0;
static int	item = 0;

/*
 *	Key String
 */
static char	*kstr = NULL;
static int	kstrsiz = 0;
static int	kstrlen = 0;

/*
 *	Other Keysyms
 */
static unsigned int	*oksym = NULL;
static int	oksymsiz = 0;		/* size in bytes */
static int	oksymlen = 0;		/* number of keysyms */

/*
 *	My memory allocator.
 *	exit on allocation error.
 */
static void	*myrealloc(void *base, size_t size)
{
	if (base)	base = realloc(base, size);
	else		base = malloc(size);
	if (!base) {
		fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
		exit(2);
	}
	return base;
}

/*
 *	get a next line from stdin.
 *	return NULL pointer on EOF.
 */

static char	*getline()
{
	int	i;
	int	c;
#define	LB_ALLOC_UNIT	512
	static char	*linebuf = 0;
	static int	linebufsiz = 0;
	static int	hiteof = False;

	if (hiteof) {
		if (linebuf) {
			free((void *)linebuf);
			linebuf = 0;
			linebufsiz = 0;
		}
		return NULL;
	}
	for (i = 0; (c = getchar()) != '\n' && c != EOF; i++) {
		if (i >= linebufsiz) {
			linebufsiz += LB_ALLOC_UNIT;
			linebuf = myrealloc(linebuf, linebufsiz);
		}
		linebuf[i] = c;
	}
	if (c == EOF)
		hiteof = True;
	if (i >= linebufsiz) {
		linebufsiz += LB_ALLOC_UNIT;
		linebuf = myrealloc(linebuf, linebufsiz);
	}
	linebuf[i] = '\n';
	line++;
	item = 0;
	return linebuf;
}

static int	addtokstr(char *str, int len)
{
#define	KSTR_ALLOC_UNIT	256
	int	id;

	if (kstrlen + len + 1 >= kstrsiz) {
		kstrsiz = ((kstrlen + len + 1) / KSTR_ALLOC_UNIT + 1) *
			KSTR_ALLOC_UNIT;
		kstr = (char *)myrealloc(kstr, kstrsiz);
	}
	kstr[kstrlen] = len;
	memcpy(kstr + kstrlen + 1, str, len);
	id = kstrlen;
	kstrlen += len + 1;
	return id;
}

static int	addtooksym(unsigned int keysym)
{
#define	OKSYM_ALLOC_UNIT	512
	if (oksymlen * sizeof (unsigned int) >= oksymsiz) {
		oksymsiz += OKSYM_ALLOC_UNIT;
		oksym = (unsigned int *)myrealloc((void *)oksym, oksymsiz);
	}
	oksym[oksymlen++] = keysym;
	return oksymlen - 1;
}

static void	placekeysym(KeyMapElement *kme, unsigned int keysym)
{
	int	idx;

	if ((keysym & 0xff000000) == AIX_PRIVKEYSYM) {
		kme->type = KME_PRIVKEYSYM;
		PUT3DIGITS(kme->data, keysym);
	}
	else if ((keysym & 0xff000000)) {
		kme->type = KME_OTHERKEYSYM;
		idx = addtooksym(keysym);
		PUT3DIGITS(kme->data, idx);
	}
	else {
		kme->type = KME_KEYSYM;
		PUT3DIGITS(kme->data, keysym);
	}
}

#define	isodigit(c)	('0' <= (c) && (c) <= '7')
#define	ddigit(c)	((c) - '0')
#define	odigit(c)	((c) - '0')
#define	xdigit(c) \
	((c) <= '9' ? (c) - '0' : (c) <= 'F' ? (c) - 'A' + 10 : (c) - 'a' + 10)

char	*escchar(char *ptr, char *c)
{
	int	val;
	int	cnt;

	/* ptr points to next of back slash */
	switch (*ptr) {
	case '\\': *c = '\\'; ptr++; break;
	case 'n': *c = '\n'; ptr++; break;
	case 'r': *c = '\r'; ptr++; break;
	case 't': *c = '\t'; ptr++; break;
	case 'b': *c = '\b'; ptr++; break;
	case 'f': *c = '\f'; ptr++; break;
	case 'x':
		if (!isxdigit(ptr[1]) || !isxdigit(ptr[2]))
			return NULL;
		*c = (xdigit(ptr[1]) << 4) + xdigit(ptr[2]);
		ptr += 3;
		break;
	case 'd':
		val = 0;
		cnt = 3;
		ptr++;
		while (isdigit(*ptr) && cnt--)
			val = val * 10 + ddigit(*ptr++);
		*c = val;
		break;
	default:
		if (!isodigit(ptr[0])) {
			*c = ptr[0];
			ptr++;
			break;
		}
		val = 0;
		cnt = 3;		/* \XXX : octal */
		while (isodigit(*ptr) && cnt--)
			val = (val << 3) + odigit(*ptr++);
		*c = val;
		break;
	}
	return ptr;
}

static int	process_DXX(KeyMapElement *kme, char c, char c2)
{
	static unsigned int	dead_keysyms[] = {
		XK_dead_acute,		XK_dead_acute, /* apostrophe */
		XK_dead_grave,		XK_dead_circumflex,
		XK_dead_diaeresis,	XK_dead_tilde,
		XK_dead_caron,		XK_dead_breve,
		XK_dead_doubleacute,	XK_dead_degree,
		XK_dead_abovedot,	XK_dead_macron,
		XK_dead_cedilla,	XK_dead_ogonek,
	};
		
	c = 10 * (c - '0') + c2 - '0';
	if (c > 15)
		return False;
	placekeysym(kme, dead_keysyms[c]);
	return True;
}

typedef struct _KTable	{
	char	*str;
	unsigned int	keysym;
}	KTable;

static int	kcomp(KTable *kt1, KTable *kt2)
{
	return strcmp(kt1->str, kt2->str);
}

int	lookupkeysym(char *ptr, unsigned int *keysym)
{
	int	high, mid, low;
	int	ret;
	static int	initialized = False;
	/*
	 *	X11/ks_names.h contains array of keysym name/value.
	 *	Included it and append some private ones.
	 *	Later we qsort it for binary searching.
	 */
	static KTable	ktable[] = {
#include <X11/ks_names.h>
#include <X11/aix_ks_names.h>
		{ "BEEP", XK_BEEP },
		{ "UNBOUND", XK_UNBOUND },
		{ "IGNORE", XK_IGNORE },
		{ "*", XK_ALL },
	};

	if (!initialized) {
		qsort((void *)&ktable[0], sizeof (ktable) / sizeof (ktable[0]),
			sizeof (ktable[0]), (int (*)())kcomp);
		initialized = True;
	}
	low = 0;
	high = sizeof (ktable) / sizeof (ktable[0]);
	while (high >= low) {
		mid = (high + low) / 2;
		if ((ret = strcmp(ptr, ktable[mid].str)) < 0)
			high = mid - 1;
		else if (ret > 0)
			low = mid + 1;
		else {
			*keysym = ktable[mid].keysym;
			return True;
		}
	}
	return False;
}

static char	*process_item(KeyMapElement *kme, char *ptr)
{
	char	c;
	char	*bp;
	unsigned int	keysym;
	int	i;
	int	id;
	static char	buf[256];

	++item;
	switch (*ptr++) {
	case 'U':
		kme->type = KME_UNBOUND;
		PUT3BYTES(kme->data, '\0', '\0', '\0');
		return ptr;

	case 'X':	/* keysym */
		if (*ptr++ != 'K' || *ptr++ != '_')
			return NULL;
		bp = ptr;
		while (isalnum(*ptr) || *ptr == '_')
			ptr++;
		c = *ptr;
		*ptr = '\0';
		if (!lookupkeysym(bp, &keysym))
			return NULL;
		*ptr = c;
		placekeysym(kme, keysym);
		return  ptr;

	case '\'':	/* single quote */
		kme->type = KME_SSTR1;
		if (*ptr != '\\') {
			PUT3BYTES(kme->data, *ptr++, '\0', '\0');
			if (*ptr++ != '\'')
				return NULL;
			return ptr;
		}
		if (*++ptr == '\'') {
			PUT3BYTES(kme->data, *ptr++, '\0', '\0');
			if (*ptr++ != '\'')
				return NULL;
			return ptr;
		}
		ptr = escchar(ptr, &kme->data[0]);
		kme->data[1] = 0;
		kme->data[2] = 0;
		if (*ptr++ != '\'')
			return NULL;
		return ptr;

	case '"':
		if (*ptr == 'D' && isdigit(ptr[1]) && isdigit(ptr[2]) &&
			ptr[3] == '"') {
			if (!process_DXX(kme, ptr[1], ptr[2]))
				return NULL;
			return ptr + 4;
		}
		/*
		 *	loop until next double quote (")
		 */
		for (i = 0; i < 255; i++) {
			if (*ptr == '"')
				break;
			if (*ptr == '\\') {
				if (*++ptr == '"')
					kme->data[0] = *ptr++;
				else
					ptr = escchar(ptr, &buf[i]);
			}
			else
				buf[i] = *ptr++;
		}
		ptr++;
		switch (i) {		/* switch by length */
		case 0:
			kme->type = KME_UNBOUND;
			PUT3BYTES(kme->data, '\0', '\0', '\0');
			return ptr;
		case 1:
			kme->type = KME_SSTR1;
			PUT3BYTES(kme->data, buf[0], '\0', '\0');
			return ptr;
		case 2:
			if (buf[0] == 0x1b) {
				kme->type = KME_ESEQ1;
				PUT3BYTES(kme->data, buf[1], '\0', '\0');
			}
			else {
				kme->type = KME_SSTR2;
				PUT3BYTES(kme->data, buf[0], buf[1], '\0');
			}
			return ptr;
		case 3:
			if (buf[0] == 0x1b) {
				kme->type = KME_ESEQ2;
				PUT3BYTES(kme->data, buf[1], buf[2], '\0');
			}
			else {
				kme->type = KME_SSTR3;
				PUT3BYTES(kme->data, buf[0], buf[1], buf[2]);
			}
			return ptr;
		case 4:
			if (buf[0] == 0x1b) {
				kme->type = KME_ESEQ3;
				PUT3BYTES(kme->data, buf[1], buf[2], buf[3]);
				return ptr;
			}
			break;
		case 6:
			if (buf[0] == 0x1b) {
				if (buf[1] == '[' && isdigit(buf[2]) &&
					isdigit(buf[3]) && isdigit(buf[4])) {
					if (buf[5] == 'q') {
						kme->type = KME_QPFK;
						PUT3BYTES(kme->data,
							buf[2], buf[3], buf[4]);
						return ptr;
					}
					else if (buf[5] == 'z') {
						kme->type = KME_ZPFK;
						PUT3BYTES(kme->data,
							buf[2], buf[3], buf[4]);
						return ptr;
					}
				}
			}
		}
		kme->type = KME_LSTR;
		id = addtokstr(&buf[0], i);
		PUT3DIGITS(kme->data, id);
		return ptr;

	case 'D':
		if (isdigit(ptr[0]) && isdigit(ptr[1]) && ptr[2] == '"') {
			if (!process_DXX(kme, ptr[0], ptr[1]))
				return NULL;
			return ptr + 3;
		}
	case '0':
		if (*ptr == 'X' || *ptr == 'x') {
			keysym = 0;
			ptr++;
			while (isxdigit(*ptr)) {
				keysym = keysym * 16 + xdigit(*ptr);
				ptr++;
			}
			placekeysym(kme, keysym);
			return ptr;
		}
		c = 0;
		i = 3;		/* octal */
		while (isodigit(*ptr) && i--)
			c = (c << 3) + odigit(*ptr++);
		kme->type = KME_SSTR1;
		PUT3BYTES(kme->data, c, '\0', '\0');
		return ptr;
	}
	return NULL;
}

static int	process_line(char *ptr)
{
	unsigned int	keysym;
	int	c;
	char	*bp;
	PLine	*pline;

	if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
		/*
		 *	keysym : hexdicimal value
		 */
		ptr += 2;
		keysym = 0;
		while (isxdigit(*ptr)) {
			keysym = keysym * 16 + xdigit(*ptr);
			ptr++;
		}
	}
	else if (ptr[0] == 'X' && ptr[1] == 'K' && ptr[2] == '_') {
		/*
		 *	keysym : symbolic
		 */
		ptr += 3;
		bp = ptr;
		while (isalnum(*ptr) || *ptr == '_')
			ptr++;
		c = *ptr;
		*ptr = '\0';
		if (!lookupkeysym(bp, &keysym)) {
			fprintf(stderr, MSGSTR(MN_LINE, E_LINE), line);
			exit(1);
		}
		*ptr = c;
	}
	else {
		fprintf(stderr, MSGSTR(MN_LINE, E_LINE), line);
		exit(1);
	}

	if (plines.len >= plines.siz) {
		plines.siz += PLS_ALLOC_UNIT;
		plines.pline = (PLine *)myrealloc((void *)plines.pline,
			plines.siz * sizeof (PLine));
	}
	pline = &plines.pline[plines.len++];
	pline->len = pline->siz = 0;
	pline->kme = NULL;
	pline->keysym = keysym;

	while (1) {
		while (1) {
			if (*ptr == ' ' || *ptr == '\t' || *ptr == ',') {
				ptr++;
				continue;
			}
			if (ptr[0] == '/' && ptr[1] == '*') {
				ptr += 2;
				while (1) {
					if (*ptr != '*' && *ptr != '\n') {
						ptr++;
						continue;
					}
					if (*ptr == '\n') {
						if (maxplen < pline->len)
							maxplen = pline->len;
						return True;
					}
					if (*ptr++ == '/')
						break;;
				}
			}
			else
				break;
		}
		if (*ptr == '\n') {
			if (maxplen < pline->len)
				maxplen = pline->len;
			return True;
		}
		if (*ptr == '\\') {
			if (ptr[1] != '\n')
				return False;
			item = 0;
			if (!(ptr = getline())) {
				fprintf(stderr, MSGSTR(MN_LINE, E_LINE), line);
				exit(1);
			}
			continue;
		}
		if (pline->len >= pline->siz) {
			pline->siz += PL_ALLOC_UNIT;
			pline->kme =
				(KeyMapElement *)myrealloc((void *)pline->kme,
				pline->siz * sizeof (KeyMapElement));
		}
		if (!(ptr = process_item(&pline->kme[pline->len++], ptr)))
			return False;
	}
}

static int	process_cntl(char *ptr, CLine *cline, int shift)
{
	short	stat;

	cline->len = cline->siz = 0;
	cline->stat = NULL;
	while (1) {
		while (*ptr == ' ' || *ptr == '\t') ptr++;
		if (*ptr == '\n')
			return True;
		if (*ptr == '\\') {
			if (ptr[1] != '\n')
				return False;
			if (!(ptr = getline())) {
				fprintf(stderr, MSGSTR(MN_LINE, E_LINE), line);
				exit(1);
			}
			continue;
		}
		if (!isdigit(*ptr))
			return False;
		stat = 0;
		do {
			stat = stat * 10 + *ptr++ - '0';
		} while (isdigit(*ptr));
		if (cline->len >= cline->siz) {
			cline->siz += CL_ALLOC_UNIT;
			cline->stat = (short *)myrealloc((void *)cline->stat,
				cline->siz * sizeof (short));
		}
		cline->stat[cline->len++] = stat - shift;
	}
}

static int	comp(PLine *pl1, PLine *pl2)
{
	if (pl1->keysym > pl2->keysym)
		return 1;
	else if (pl2->keysym > pl1->keysym)
		return -1;
	return 0;
}

static void	outputkeymap()
{
	int	maxstat;
	int	i, j;
	CLine	*mline;
	short	*stat;
	short	idx;
	KeyMapElement	*pkme;
	IMKeymapFile	imfile;
	int	nstat;
	static KeyMapElement	unboundkme = {KME_UNBOUND, '\0', '\0', '\0'};

	/*
	 *	If there was no #S, #M or %M, default to the old format.
	 *	(#S and #M)
	 */
	if (mflag == MFLAG_NONE)
		mflag = MFLAG_OLD;

	/*
	 *	If it's the new format or there was no #S line with the old
	 *	format, create a default sline.
	 */
	if (mflag == MFLAG_NEW || sline.len == 0) {
		sline.siz = maxplen > 32 ? maxplen : 32;
		sline.stat = (short *)malloc(sline.siz * sizeof (short));
		if (!sline.stat) {
			fprintf(stderr, MSGSTR(MN_MEM, E_MEM));
			exit(2);
		}
		for (i = 0; i < sline.siz; i++)
			sline.stat[i] = i;
		sline.len = sline.siz;
	}

	/*
	 *	invalidate states (sline) whose value exceed the number of
	 *	elements listed and count the number of valid states.
	 */
	for (nstat = i = 0; i < sline.len; i++)
		if (sline.stat[i] >= maxplen)
			sline.stat[i] = -1;
		else
			nstat++;

	/*
	 *	get maximum state number and allocate the state mapping table.
	 */
	maxstat = 0;
	for (i = 0; i < sline.len; i++)
		if (maxstat < sline.stat[i])
			maxstat = sline.stat[i];
	for (i = 0; i < mlines.len; i++) {
		mline = &mlines.mline[i];
		for (j = 0; j < mline->len; j++)
			if (maxstat < mline->stat[j])
				maxstat = mline->stat[j];
	}
	maxstat += ++maxstat % 2;
	stat = malloc(maxstat * sizeof (short));

	/*
	 *	fill the state mapping table.
	 */
	for (i = 0; i < maxstat; i++)
		stat[i] = -1;
	for (i = 0; i < sline.len; i++)
		if (sline.stat[i] >= 0)
			stat[sline.stat[i]] = i;
	for (i = 0; i < mlines.len; i++) {
		mline = &mlines.mline[i];
		for (j = 1; j < mline->len; j++)
			stat[mline->stat[j]] = mflag == MFLAG_NEW
				? mline->stat[0]
				: stat[mline->stat[0]];
	}

	/*
	 *	imkeymap file header stuffs.
	 */
	/* round up to next int boundary */
	kstrlen = ((kstrlen + sizeof (int) - 1) / sizeof (int)) * sizeof (int);
	imfile.magic = ILS_KEYMAP_MAGIC;
	imfile.dummy[0] = imfile.dummy[1] = imfile.dummy[2] = '\0';
	imfile.statsiz = maxstat * sizeof (short);
	imfile.ksymsiz = plines.len * sizeof (unsigned int);
	imfile.elmtsiz = nstat * plines.len * sizeof (KeyMapElement);
	imfile.kstrsiz = kstrlen;
	imfile.oksymsiz = oksymlen * sizeof (unsigned int);
	imfile.stat = sizeof (IMKeymapFile);
	imfile.ksym = imfile.stat + imfile.statsiz;
	imfile.elmt = imfile.ksym + imfile.ksymsiz;
	imfile.kstr = imfile.elmt + imfile.elmtsiz;
	imfile.oksym = imfile.kstr + imfile.kstrsiz;

	/*
	 *	write the header
	 */
	fwrite((char *)&imfile, sizeof (IMKeymapFile), 1, stdout);

	/*
	 *	write the state mapping table.
	 */
	fwrite((char *)stat, sizeof (short), maxstat, stdout);

	/*
	 *	sort lines by keysym value order.
	 */
	qsort((void *)plines.pline, plines.len, sizeof (PLine),
		(int (*)())comp);

	/*
	 *	write the keysym table.
	 */
	for (i = 0; i < plines.len; i++)
		fwrite((char *)&plines.pline[i].keysym,
			sizeof (unsigned int), 1, stdout);

	/*
	 *	write the elements.
	 */
	for (i = 0; i < sline.len; i++) {
		if ((idx = sline.stat[i]) < 0)
			continue;
		for (j = 0; j < plines.len; j++) {
			if (idx >= plines.pline[j].len)
				pkme = &unboundkme;
			else
				pkme = &plines.pline[j].kme[idx];
			fwrite((char *)pkme, sizeof (KeyMapElement), 1, stdout);
		}
	}

	/*
	 *	write the strings if exist.
	 */
	if (kstrlen > 0)
		fwrite((char *)kstr, sizeof (char), kstrlen, stdout);

	/*
	 *	write the other keysyms if exist.
	 */
	if (oksymlen > 0)
		fwrite((char *)oksym, sizeof (unsigned int), oksymlen, stdout);
}

/*ARGSUSED*/
main(int argc, char **argv)
{
	char	*ptr;

	setlocale(LC_MESSAGES, "");
#ifndef	NL_CAT_LOCALE
	catd = catopen(MF_KEYCOMP, 0);
#else
	catd = catopen(MF_KEYCOMP, NL_CAT_LOCALE);
#endif	/* NL_CAT_LOCALE */

	if(argc > 2){
		fprintf(stderr, MSGSTR(MN_USG, E_USG));
		exit(1);
	}else if(argc == 2){
		if(strcmp(argv[1], "-c") != 0){
			fprintf(stderr, MSGSTR(MN_USG, E_USG));
			exit(1);
		}
		if(CompileCompose() == 0){
			return(0);
		}else{
			return(1);
		}
	}
		
	line = 0;
	while (ptr = getline()) {
		if (*ptr == '%') {
			if (ptr[1] != 'M') {
				fprintf(stderr, MSGSTR(MN_LINE, E_LINE), line);
				exit(1);
			}
			if (mflag == MFLAG_OLD) {
				fprintf(stderr, MSGSTR(MN_MIX, E_MIX));
				exit(1);
			}
			if (mlines.len >= mlines.siz) {
				mlines.siz += MLS_ALLOC_UNIT;
				mlines.mline =
					(CLine *)myrealloc((void *)mlines.mline,
					mlines.siz * sizeof (CLine));
			}
			if (!process_cntl(ptr + 2,
				&mlines.mline[mlines.len++], 0)) {
				fprintf(stderr, MSGSTR(MN_LINE, E_LINE), line);
				exit(1);
			}
			mflag = MFLAG_NEW;
			continue;
		}
		if (*ptr == '#') {
			if (ptr[1] == 'M') {
				if (mflag == MFLAG_NEW) {
					fprintf(stderr, MSGSTR(MN_MIX, E_MIX));
					exit(1);
				}
				if (mlines.len >= mlines.siz) {
					mlines.siz += MLS_ALLOC_UNIT;
					mlines.mline =
						(CLine *)myrealloc(
						(void *)mlines.mline,
						mlines.siz * sizeof (CLine));
				}
				if (!process_cntl(ptr + 2,
					&mlines.mline[mlines.len++], 1)) {
					fprintf(stderr, MSGSTR(MN_LINE, E_LINE),
						line);
					exit(1);
				}
				mflag = MFLAG_OLD;
			}
			else if (ptr[1] == 'S') {
				if (mflag == MFLAG_NEW) {
					fprintf(stderr, MSGSTR(MN_MIX, E_MIX));
					exit(1);
				}
				if (!process_cntl(ptr + 2, &sline, 1)) {
					fprintf(stderr, MSGSTR(MN_LINE, E_LINE),
						line);
					exit(1);
				}
				mflag = MFLAG_OLD;
			}
			continue;
		}
		while (*ptr == ' ' || *ptr == '\t') ptr++;
		if (*ptr == '\n')
			continue;
		if (!process_line(ptr)) {
			fprintf(stderr, MSGSTR(MN_ITEM, E_ITEM), line, item);
			exit(1);
		}
	}
	outputkeymap();
	return 0;
}
