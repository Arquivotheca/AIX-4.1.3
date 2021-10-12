static char sccsid[] = "@(#)10  1.26  src/bos/usr/bin/trcrpt/gettmplt.c, cmdtrace, bos411, 9428A410j 6/7/94 23:22:17";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: gettmpltinit, gettmplt, yygetchar, yyungetchar
 *            gettraceid, pass1excp, gldebug, gllist
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information 
 */

/*
 * Read a "canonicalized" template by removing backslash continuations
 * and extra whitespace to be analyzed by lex.c
 *                                                                    
 *
 * NAME:     gettmpltinit
 * FUNCTION: read in the template file and save the start of each template
 *           int the structure Tindexes[]
 * INPUTS:   none
 * RETURNS:  none
 *
 * The Tindexes array is an array of 4096 structures, one for each possible
 * traceid. It records the version/release, starting offset in the template
 * file, the line number in the template filem and the size in bytes of the
 * template.
 * The Traceids[] array is a table of traceids starting from the first
 * template in the template file.
 *
 *
 * NAME:     gettmplt
 * FUNCTION: read a complete template from the template file, squeezing
 *           out whitespace and '\\' continuation characters. Strip off
 *           the template number and version into separate buffers.
 *           Put the template into the buffer 'glbuf'.
 *           The number of elements in the buffer is 'glsize'.
 * INPUTS:   traceid   When 0, get the next template from the template file.
 *                     Otherwise, get the template whose traceid is 'traceid'.
 * RETURNS:  RV_GOOD      Template was read into glbuf
 *           RV_EOF       No more templates
 *           RV_BADFORMAT Template did not have a proper traceid or version.
 *
 *
 * Read a trcfmt line into glbuf.
 * Strip off entry number and version. (getvers)
 * Skip comments and blank lines.      (getvers)
 * Combine lines that are broken by '\\'.
 * Squeeze out multiple tabs and blanks.
 */

#define _ILS_MACROS

#include <stdio.h>
#include <ctype.h>
#include <sys/trchkid.h>
#include "rpt.h"
#include "parse.h"

extern FILE *Tmpltfp;
char *Default_tmplt = "\
007 1.0 \"UNDEFINED TRACE ID\" idx $LOGIDX0 \
	G0 {{ $hookword = X4 }} \
	G0 {{ $traceid = B0.12 }} \
	G2 {{ $length = X2 }} \
	traceid $traceid \n\
	hookword $hookword type HT \n\
	HT & 0x07, \
	01 { hookdata X2 }, \
	02 { hookdata X2 X4 }, \
	06 { hookdata X2 X4 X4 X4 X4 X4 }, \
	00 { d1 $D1 G8 LOOP $length {X0} } \
";

static char Traceidbuf[8];
static char Versionbuf[8];
static Traceid;

static char *fgetp;	/* used by settmplt */
static fgetmode;	/* used by settmplt */

static char *glbuf = NULL;
static glidx;
static glsize;
static gllineno;
static prev_nlc = EOF;
static localeinitflg;

gettmpltinit()
{
	int rv;
	int ti;
	
	if(!localeinitflg) {
		localeinitflg++;
	}
	Debug("gettmpltinit\n");
	vprint("Initializing template file %s\n",Tmpltfile);
	Lineno = 1;
	fseek(Tmpltfp,0,0);
	Tmpltidx = 0;
	memset(Tindexes,0,sizeof(Tindexes));
	memset(Traceids,0,sizeof(Traceids));
	for(ti = 0; ti < MAXTEMPLATES; ti++) {
		rv = gettmplt(0);
		switch(rv) {
		case RV_EOF:
			fseek(Tmpltfp,0,0);
			Tmpltidx = 0;
			return;
		case RV_BADFORMAT:
			Ntemplates++;
			Errflg++;
			continue;
		case RV_GOOD:
			Ntemplates++;
			break;
		default:
			Debug("gettmpltinit: bad completion code %d from gettmplt",rv);
			genexit(1);
		}
		Debug2("Traceid: %s index: %x Lineno: %d\n",
			hexstr(Traceid),Toffset,gllineno);
		if(Tindexes[Traceid].t_state & T_DEFINED)
			cat_lwarn(CAT_TPT_MULT,
				"Multiple occurrences of trace id %03x",Traceid);
		Tindexes[Traceid].t_state  |= T_DEFINED;
		Tindexes[Traceid].t_version = Version.v_version;
		Tindexes[Traceid].t_release = Version.v_release;
		Tindexes[Traceid].t_offset  = Toffset;
		Tindexes[Traceid].t_lineno  = gllineno;
		Tindexes[Traceid].t_size    = ftell(Tmpltfp) - Toffset;
		Traceids[ti] = Traceid;
	}
	Debug("Too many templates. > %d",MAXTEMPLATES);
	fseek(Tmpltfp,0,0);
	Tmpltidx = 0;
}


#define ISSPACE(c) ( (c) == ' ' || (c) == '\t' )

gettmplt(traceid)
{

	if(traceid != 0) {
		if(!(Tindexes[traceid].t_state & T_DEFINED)) {
			Debug("gettmplt: request for undefined template");
			genexit(1);
		}
		Lineno = Tindexes[traceid].t_lineno;
		fseek(Tmpltfp,Tindexes[traceid].t_offset,0);
	}
	fgetmode = 0;
	return(igettmplt(traceid));
}

static igettmplt(traceid)
{
	int c;
	int quoteflg;
	int rv;
	int syncflg;
	char *gp;
	static unsigned int glsize_alloc = 0;

	syncflg = 0;
	rv = getvers();
	gllineno = Lineno;
	switch(rv) {
	case RV_EOF:
		return(rv);
	case RV_BADFORMAT:
		syncflg++;
		break;
	case RV_GOOD:
		break;
	default:
		Debug("bad completion code from getvers %d",rv);
		genexit(1);
	}
	glidx = 0;
	glsize = 0;
	quoteflg = 0;
	if ( glbuf )
		bzero( gp = glbuf, glsize_alloc );
	else
		gp = glbuf = jalloc( glsize_alloc = LINESIZE );
	for(;;) {
                if ( (unsigned)(gp - glbuf +4) >= glsize_alloc )
		{
                        char *tmp ;
                        gp  = gp - glbuf ;
                        tmp = jalloc( glsize_alloc + LINESIZE );
                        bcopy( glbuf, tmp, glsize_alloc );
                        free( glbuf );
                        gp   += (int)(glbuf = tmp);
                        glsize_alloc += LINESIZE ;
                }
		c = fgetchar();
		if(c == EOF) {
			if(gp == glbuf)
				return(RV_EOF);
			*gp++ = '\0';
			goto lineexit;
		}
		if(quoteflg) {
			switch(c) {
			case '`':
				if(quoteflg == '`')
					quoteflg = 0;
				break;
			case '"':
				if(quoteflg == '"')
					quoteflg = 0;
				break;
			case '\\':
				*gp++ = '\\';
				c = fgetchar();
				break;
			}
			*gp = c;
			gp++;
			continue;
		}
		switch(c) {
		case ' ':				/* squeeze out whitespace */
		case '\t':
			c = skipspace();	/* next non-space character */
			if(c != '\n')
				*gp++ = ' ';
			continue;
		case '\n':
			*gp++ = '\n';
			*gp++ = '\0';
			goto lineexit;
		case '\\':
			c = fgetchar();
			switch(c) {
			case ' ':
			case '\t':				/* ignore '\ ' */
				c = skipspace();	/* c = next non-space character */
				if(c == '\n') {
					c = fgetchar();	/* eat the '\n' */
					*gp++ = '\n';
				}
				continue;
			case '\n':
				*gp++ = '\n';
				continue;
			default:
				*gp++ = '\\';
				*gp = c;
				gp++;
				continue;
			}
		case '`':					/* quoted string */
		case '"':					/* quoted string */
			quoteflg = c;
		default:
			*gp = c;
			gp++;
			continue;
		}
	}
lineexit:
	if(syncflg)
		return(RV_BADFORMAT);
	glidx = 0;
	glsize = gp - glbuf;
	if(traceid != 0)
		Lineno = Tindexes[traceid].t_lineno;
	return(RV_GOOD);
}

/*
 * NAME:     getvers
 * FUNCTION: Read in the traceid and version
 * INPUTS:   none
 * RETURNS:  RV_EOF        EOF encountered
 *           RV_BADFORMAT  The version or traceid was omitted or bad
 *           RV_GOOD       Traceid and version read id OK
 *           Fill in the global Traceid with the traceid.
 *           Fill in the global Version with the version.
 *
 * Skip blank line and comments.
 * Be very strict about the exact input format of the traceid and version
 * because this is the only separator between templates.
 */
static getvers()
{
	int c;
	int va,vb;
	int rv;
	char *cp;

	for(;;) {
		c = fgetchar();
		switch(c) {
		case EOF:
			rv = RV_EOF;
			goto versexit;
		case '\n':
			continue;
		case ' ':
		case '\t':
			cat_lerror(CAT_TPT_SOL,
"Expected a 3 hex digit trace id at the start of the line.\n\
This could mean that the beginning of this template is wrong,\n\
or that the previous template is missing a '\\' continuation\n\
character.\n");
			rv = RV_BADFORMAT;
			goto versexit;
		case '#':			/* comment */
		case '*':			/* comment */
			while((c = fgetchar()) != '\n') {
				if(c == EOF) {
					rv = RV_EOF;
					goto versexit;
				}
			}
			continue;
		}
		fungetchar(c);
		break;
	}
	/*
	 * look for traceid
	 */
	cp = Traceidbuf;
	for(;;) {
		c = fgetchar();
		switch(c) {
		case EOF:
			rv = RV_EOF;
			goto versexit;
		case '\n':
			cat_lerror(CAT_TPT_UNEXPEOL,
"Unexpected end of line encountered.\n\
A template must begin with a 3 hex digit template id,\n\
a version field, and a trace label.\n");
			rv = RV_BADFORMAT;
			goto versexit;
		case ' ':
		case '\t':
			skipspace();
			*cp++ = '\0';
			break;
		default:
			if(cp == Traceidbuf)
				Toffset = Tmpltidx-1;
			if(!isxdigit(c))
				c = 'Z';
			if(cp < &Traceidbuf[sizeof(Traceidbuf)])
				*cp++ = c;
			continue;
		}
		break;
	}
	/*
	 * look for version
	 */
	cp = Versionbuf;
	for(;;) {
		c = fgetchar();
		switch(c) {
		case EOF:
			rv = RV_EOF;
			goto versexit;
		case '\n':
			cat_lerror(CAT_TPT_UNEXPEOL,
"Unexpected end of line encountered.\n\
A template must begin with a 3 hex digit template id,\n\
a version field, and a trace label.\n");
			rv = RV_BADFORMAT;
			goto versexit;
		case ' ':
		case '\t':
			*cp++ = '\0';
			break;
		default:
			if(!isdigit(c) && c != '.')
				c = '.';
			if(cp < &Versionbuf[sizeof(Versionbuf)])
				*cp++ = c;
			continue;
		}
		break;
	}
	if((Traceid = strtoid(Traceidbuf)) < 0) {
		rv = RV_BADFORMAT;
		goto versexit;
	}
	if(strlen(Versionbuf) > 5 ||
	   sscanf(Versionbuf,"%d.%d",&va,&vb) != 2) {
		cat_lerror(CAT_TPT_VFMT,
			"Bad format for version field %s\n\
			A version field must be of the form :\n\
			vv.rr where vv is a 1 or 2 digit version number, and\n\
			rr is a 1 or 2 digit release number.\n",
			Versionbuf);
		rv = RV_BADFORMAT;
		goto versexit;
	}
	Version.v_version = va;
	Version.v_release = vb;
	rv = RV_GOOD;
versexit:
	return(rv);
}

gldebug()
{

	Debug("gldebug '%s'\n",glbuf);
}

/*
 * Print the template to the list file (or stderr if -c)
 * with a pointer to the syntax error.
 */

gllist(errflg)
{
	int lim;
	int idx;
	int c;
	char *s;
	int col;
	int n;
	int i;
	char buf[32];

	idx = 0;
	lim = errflg ? glidx : glsize;
	sprintf(buf,"%s %d.%d ",
		hexstr(Traceid),Version.v_version,Version.v_release);
	List("%s",buf);
	col = strlen(buf);
	while(idx < lim) {
		switch(c = glbuf[idx++]) {
		case '\0':
			break;
		case '\n':
			col = 0;
			Listc(c);
			continue;
		case '\t':
			n = 8 - col % 8;
			while(--n >= 0) {
				col++;
				Listc(' ');
			}
			continue;
		default:
			if(c < ' ' || c > 0x7E) {
				List("(%02X)",c);
				col += 4;
				continue;
			}
			col++;
			Listc(c);
			continue;
		}
	}
	if(errflg) {
		Listc('\n');
		if(col > 60) {
			List("SYNTAX ERROR");
			for(i = sizeof("SYNTAX ERROR"); i < col; i++)
				Listc('-');
			List("^\n");
		} else {
			for(i = 0; i < col-1; i++)
				Listc('-');
			List("^ SYNTAX ERROR\n");
		}
	}
}


static char *ctrlstr(c)
char c;
{
	static char buf[8];

	if(' ' <= c && c < 0x7F) {
		buf[0] = c;
		buf[1] = '\0';
		return(buf);
	} 
	switch(c) {
	case 0x7F: return("DEL");
	case '\n': return("<LF>");
	case '\r': return("<CR>");
	case '\t': return("<TAB>");
	}
	sprintf(buf,"^%c",c + 'A');
	return(buf);
}

#ifdef TRCRPT
settmplt(str)
char *str;
{

	fgetmode = 1;
	fgetp = str;
	return(igettmplt(0));
}

#endif

yygetchar()
{
	int c,c1;

	if(glidx == glsize)
		return(EOF);
	c = glbuf[glidx++];
	Debug2("yyget: '%s'\n",ctrlstr(c));
	return(c);
}
yyungetchar(nlc)
{
	char *cp;

	glidx--;
	if(glidx < 0) {
		Debug("yyungetchar");
		genexit(1);
	}
}


gettraceid()
{

	return(Traceid);
}

/*
 * Skip over blanks and tabs.
 * Return the last character read. (It will not be a blank or tab)
 */
static skipspace()
{
	int c;

	for(;;) {
		c = fgetchar();
		if(!ISSPACE(c))
			break;
	}
	fungetchar(c);
	return(c);
}

static fgetchar()
{
	int c,c1;

	if(fgetmode == 1) {
		c = prev_nlc;
		prev_nlc = EOF;
		if(c != EOF)
			return(c);
		c = *fgetp++;
		if(c == '\0') {
			fgetp--;
			return(EOF);
		}
		return(c);
	}
	c = prev_nlc;
	prev_nlc = EOF;
	if(c != EOF) {
		Tmpltidx++;
		if(c == '\n')
			Lineno++;
		return(c);
	}
	c = getc(Tmpltfp);
	if(c == EOF)
		return(EOF);
	Tmpltidx++;
	if(c == '\n')
		Lineno++;
	return(c);
}

static fungetchar(nlc)
{

	if(fgetmode == 1) {
		if(prev_nlc != EOF) {
			Debug("fungetchar");
			genexit(1);
		}
		prev_nlc = nlc;
		return;
	}
	if(Tmpltidx == 0) {
		Debug("fungetchar");
			genexit(1);
		}
	if(nlc == '\n')
		Lineno--;
	if(prev_nlc != EOF) {
		Debug("fungetchar");
		genexit(1);
	}
	prev_nlc = nlc;
	Tmpltidx--;
}


#ifdef TRCRPT

pass1excp()
{

	Debug("segmentation error. pass 1. Line %d\n",Lineno);
	genexit(2);
}


#endif


#ifdef TRCRPT

/*
 * Convert the arg 'cp' into a traceid.
 * Return -1 if the traceid is invalid (non-numeric or too big).
 */
strtoid_cmd(id)
char *id;
{
	int c;
	int n;
	char *cp;

	switch(strlen(id)) {
	case 2:
	case 3:
		break;
	default:
		goto bad;
	}
	cp = id;
	while(c = *cp++) {
		if(isxdigit(c))
			continue;
		if(c == 'x' || c == 'X')
			continue;
		goto bad;
	}
	n = strtol(id,0,16);
	if((unsigned)n >= NHOOKIDS)
		goto bad;
	return(n);
bad:
	cat_lerror(CAT_TPT_TFMT_C,
"Bad format for traceid %s\n\
A traceid consists of 2 or 3 hexadecimal digits.\n\
A 2 digit traceid represents a range of 16 traceids.\n",id);
	return(-1);
}

#endif

/*
 * Convert the arg 'cp' into a traceid.
 * Return -1 if the traceid is invalid (non-numeric or too big).
 */
strtoid(id)
char *id;
{
	int c;
	int n;
	char *cp;

	if(strlen(id) != 3)
		goto bad;
	if(!(isxdigit(id[0]) && isxdigit(id[1]) && isxdigit(id[2])))
		goto bad;
	n = strtol(id,0,16);
	if((unsigned)n >= NHOOKIDS)
		goto bad;
	return(n);
bad:
	cat_lerror(CAT_TPT_TFMT,
"Bad format for trace id %s.\n\
A trace id must consist of exactly 3 hex numbers.\n",id);
	return(-1);
}
