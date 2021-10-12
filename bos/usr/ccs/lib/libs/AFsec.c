static char sccsid[] = "@(#)84	1.30  src/bos/usr/ccs/lib/libs/AFsec.c, libs, bos411, 9428A410j 1/29/93 09:16:44";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: afsave, aflookup, afrewind, afread, afopen, afnxtrec,
 *            afgetrec, afgetatr, afclose
 *
 * ORIGINS: 27
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
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <usersec.h>
#include <ctype.h>
#include <stdlib.h>	/* needed for ILS and ANSII things */
#include <locale.h>
#include <fcntl.h>      /* for close on exec fcntl */
#include "libs.h"
 
extern int      errno;
extern void free(void *);
static int afgreprec(char *name, FILE *fp);
ATTR_t	aflookup();
 
/*
 * These next two variables are set during AFopen()
 */
static int      mb_cur_max = 1;
static int      multibytecodeset = 0;
 
static char DEFAULT[] = "default";

/*
 * define DEBUG to compile in trace information
 */
#undef  DEBUG
#ifdef  DEBUG
        int     Debug;
#       define  DPRINTF(args)   { if (Debug) { fprintf args ; } ; }
#else
#       define  DPRINTF(args)
#endif
 
 
/*
 * this routine is not quite identical to strcpy() in that
 * it (permanently) advances the destination pointer
 */
#define STRCPY(to,from) \
        { \
                char    *q; \
                q = (from); \
                while (*to++ = *q++) \
                        ; \
        }
 
/*
 * count the length of a list of null-terminated strings.
 * the list ends with a null string;
 * i.e., either a leading '\0' or is "...\0\0"
 * the first parameter to the macro is set to the length.
 */
#define LISTLEN(n, s) \
        { \
                char    *p; \
                p = (s); \
                while (*p) \
                { \
                        while (*p++) \
                                ; \
                } \
                n = p - (s) + 1; \
        }
 
/*
 * copy a list of null-terminated strings.
 * a "list" is as defined above
 */
#define LISTCPY(to, s) \
        { \
                char    *p; \
                p = (s); \
                while (*p) \
                { \
                        while (*to++ = *p++) \
                                ; \
                } \
                *to++ = 0; \
        }
/*
 * compute a simple hash function of a string
 */
#define NHASH   1024
#define hash(n,s) \
        { \
                char    *p; \
                n = 0; \
                for (p = s; *p; p++) \
                        n += *p; \
                n &= (NHASH-1); \
        }
 
/*
 * the space for the hash table for a file follows
 * the AFILE structure for the file.
 * preceding the table itself is an integer which
 * indicates that we ran out of space trying to store
 * something in the table.
 * when this happens, we discard the contents of the hash table.
 * subsequently, we always rewind() the file instead of looking
 * in the hash table (and no more entries will be recorded in the
 * hash table).
 */
#define HOVERFLOW(af) \
       (*((int *)(((char *) af) + sizeof(struct AFILE))))
#define HTAB(af) \
       ((struct stanza **)(((char *) af) + sizeof(int) + sizeof(struct AFILE)))
 
/*
 * this is the format of AF stanzas saved in the hash table.
 * in each stanza:
 *      the first <attr,value> pair has a null string "" for the
 *              name.  the value is the name of the stanza
 *      pointers to subsequent <attr,value> pairs follow
 *      the string space for all <attr,value> pairs follows
 *              the array of <attr,value> pairs.
 * i.e., the memory layout of a saved stanza is:
 *      +------------------------+
 *      |         next           |
 *      +------------------------+
 *      |     ""    |   name     |
 *      +------------------------+
 *      | attribute |   value    |
 *      +------------------------+
 *      | attribute |   value    |
 *      +------------------------+
 *      |          ...           |
 *
 *      +------------------------+
 *      |     string space       |
 *      |          ...           |
 *      +------------------------+
 */
 
struct  stanza
{
        /* next on the linked list of saved stanzas */
        struct  stanza  *next;
        /* pointer to attribute name/value pairs */
        ATTR_t  atr;
        /* name/value pairs (struct ATTR) follow */
        /* string space is last */
};
 
/*
 * relase the space malloc()'d for each stanza entry
 * in a hash table.
 */
#define affree(af) \
        { \
                struct  stanza  **htab = HTAB(af); \
                int     j; \
                for (j = 0; j < NHASH; j++) \
                { \
                        while (htab[j]) \
                        { \
                                struct  stanza  *s; \
                                s = htab[j]; \
                                htab[j] = htab[j] -> next; \
                                free((char *) s); \
                        } \
                } \
        }
 
afclose(af)
register AFILE_t af;
{
        /* free the space malloc()'d for saved stanzas */
        affree(af);
        /* and close down the af file */
        fclose(af->AF_iop);
        free((char *)af->AF_datr);
        free((char *)af->AF_catr);
        free((char *)af->AF_dbuf);
        free((char *)af->AF_cbuf);
        free((char *)af);
}
 
 
/*
 * fetch the value of the named attribute in a stanza
 */
 
char *
afgetatr(at,name)
register ATTR_t at;
char *name;
{
        if (at == NULL || name == NULL)
                return (NULL);
        for (;;)
        { 
                if (at->AT_name == NULL)
                {
                        errno = ENOATTR;
                        return(NULL);
                }
                if (strcmp(at -> AT_name, name) == 0)
                        return(at->AT_value);
                at++;
        }
}
 
 
/*
 * Find first record in Attribute File
 * which has a given name.
 */
 
ATTR_t
afgetrec(af, name)
register AFILE_t af;
char *name;
{
	ATTR_t	cp;

        if (name == NULL)
                return NULL;
 
        /* check the hash table, first */
	if (cp = aflookup(af, name))
		return (cp);

	/*
	 * grep the file for the name -- don't mess around!
	 * while we hit defaults keep trying...
	 */
	(void)rewind(af->AF_iop);
	while (afgreprec(name, af->AF_iop))
        {
		(void)afnxtrec(af);
		/*
		 * since afgreprec() succeeded we know that we either found
		 * the correct stanza or a default stanza.  If we are
		 * actually looking for a default stanza then that is what
		 * we must have...
		 */
		if (strcmp(name, DEFAULT) == 0)
                	return (af->AF_datr);  /* we WANT the defaults */
		cp = af->AF_catr;
		if ((cp->AT_value) && (strcmp(cp->AT_value, name) == 0))
                	return (cp);  /* this is the stanza */
        }

        /* stanza not found so set errno */
        errno = ENOENT;
        return(NULL);
}
 
 
/*
 * Get the next record from an Attribute File.
 * Merge in any default record that preceeds it.
 */
 
extern char *afread(AFILE_t);
 
ATTR_t
afnxtrec(af)
register AFILE_t af;
{       
        register ATTR_t cp, pp;
        while (afread(af))
        {
                /* if default  */
                if ((af->AF_catr->AT_value)				&& 
                    (strcmp(af->AF_catr->AT_value, DEFAULT) == 0))
                {
                        /* then swap current and default */
                        register char *bp;
                        bp = af->AF_cbuf;  
                        af->AF_cbuf = af->AF_dbuf;  
                        af->AF_dbuf = bp;
                        cp = af->AF_catr;  
                        af->AF_catr = af->AF_datr;  
                        af->AF_datr = cp;
			continue;
                }
                /* if default exists... */
                if (af->AF_datr->AT_value != NULL)
                {
                        /* merge default into current end of attr list... */
                        for (pp = af->AF_datr+1; pp->AT_name; pp++)
                        {
                                for (cp = af->AF_catr+1; ; cp++)
                                {
                                        if (cp->AT_name == NULL)
                                        {
                                                /* append this default attr */
                                                cp->AT_name = pp->AT_name;
                                                cp->AT_value = pp->AT_value;
                                                cp++;
                                                /* terminate list */
                                                cp->AT_name = NULL;
                                                break;
                                        }
                                        if ((pp->AT_name) && 
                                            (cp->AT_name) && 
                                            (strcmp(pp->AT_name, cp->AT_name) 
                                                                        == 0))
                                                /* this attribute overridden */
                                                break;
                                }
                        }
                }
                if (af->AF_catr && af->AF_catr->AT_name &&
		    !aflookup(af, af->AF_catr->AT_value))
                        afsave(af, af->AF_catr);
 
                return(af->AF_catr);
        }
        return(NULL);
}
 
 
AFILE_t 
afopen(filename)
char    *filename;              /* file name of an attribute file               */
{    
        register AFILE_t af;
        register FILE *file;
        register int n;
        register int flags;
        register int    siz;
 
        /* Open the file specified */
        if ((file = fopen(filename,"r")) == NULL)
                return(NULL);
 
	/*
	 * initialize mb_cur_max and multibytecodeset (a boolean) in order
	 * to support SSDP ILS
	 */
        if ((mb_cur_max = MB_CUR_MAX) > 1)
                multibytecodeset = 1;

        /* open these files with a close on exec */
        flags = fcntl(file->_file,F_GETFD,0);
        flags |= FD_CLOEXEC;
        if (fcntl(file->_file,F_SETFD,flags) < 0)
                return ((AFILE_t)NULL);
        /*
         * Allocate an Attribute file structure.
         * space for the hash table is also allocated.
         */
        n = sizeof(int) + (NHASH * sizeof(struct stanza *));
        if ((af = (AFILE_t)malloc(sizeof(struct AFILE) + n)) == NULL)
        {
                fclose(file);
                return(NULL);
        }
        /* clear the hash table */
        memset(((char *)af) + sizeof(struct AFILE), 0, n);
 
        /* Setup the file pointer and max sizes */
        af->AF_iop = file;
        {
        struct  stat    sbuf;
 
                if (stat(filename, &sbuf) < 0)
                {
                        free((char *)af);
                        fclose(file);
                        return (NULL);
                }
                if (sbuf.st_size < BUFSIZ)
                        af->AF_rsiz = BUFSIZ;
                else
                        af->AF_rsiz = sbuf.st_size;
                af->AF_natr = 512;
        }
 
        /* Allocate the memory to hold the names of the attributes      */
        /*   in this attribute file and their corresponding values.     */
        siz = af->AF_rsiz;
        if ((af->AF_cbuf = (char *)malloc(siz)) == NULL)
        {
                free((char *)af);
                fclose(file);
                return(NULL);
        }
 
        /* Allocate the memory to hold the names of the attributes      */
        /*   in this attribute file and their default values.           */
        if ((af->AF_dbuf = (char *)malloc(siz)) == NULL)
        {
                free((char *)af->AF_cbuf);
                free((char *)af);
                fclose(file);
                return(NULL);
        }
        
 
        /* allocate an array of attribute structures (ATTR's) for       */
        /*  current values                                              */
        n = sizeof(struct ATTR) * (af->AF_natr);
        if ((af->AF_catr = (ATTR_t)malloc(n)) == NULL)
        {
                free((char *)af->AF_dbuf);
                free((char *)af->AF_cbuf);
                free((char *)af);
                fclose(file);
                return(NULL);
        }
 
        /* allocate an array of attribute structures (ATTR's) for       */
        /*  default values                                              */
        if ((af->AF_datr = (ATTR_t)malloc(n)) == NULL)
        {
                free((char *)af->AF_catr);
                free((char *)af->AF_dbuf);
                free((char *)af->AF_cbuf);
                free((char *)af);
                fclose(file);
                return(NULL);
        }
 
        /* clear the buffers */
        memset(af->AF_cbuf, 0, siz);
        memset(af->AF_dbuf, 0, siz);
        memset(af->AF_datr, 0, n);
        memset(af->AF_catr, 0, n);
 
        /* Initialize ATTR arrays (current and default)                 */
        af->AF_catr->AT_name = NULL;
        af->AF_catr->AT_value = NULL;
        af->AF_datr->AT_name = NULL;
        af->AF_datr->AT_value = NULL;
 
        return(af);
}
 
 
static void
blankstrip(char *string)
{
        if (multibytecodeset)   /* do extra stuff for multi-byte */
        {
                wchar_t	wcs[BUFSIZ];
                wchar_t *wcp;
                int     nc = strlen(string)+1;

                /*
                 * convert and copy multibyte string to wide character
                 * process code
                 */
                mbstowcs(wcs, string, nc);
 
                /*
                 * go to end of string
                 */
                wcp = wcs;
                while (*wcp)
                        wcp++;
 
                /*
                 * backspace over white space...
                 */
                while (wcp-- != wcs)
                        if (!iswspace(*wcp))
                                break;
                        else
                                *wcp = (wchar_t)0;
 
                /*
                 * now convert and copy from process code back into
                 * multi-byte code
                 */
                wcstombs(string, wcs, nc);
        }
        else    /* normal single byte characters... */
        {
                char    *cp;
 
                cp = string;
                while (*cp)
                        cp++;
 
                while (cp-- != string)
                        if (!isspace(*cp))
                                break;
                        else
                                *cp = NULL;
 
        }
}
 
 
static char *
getkey(char *buf, char *out)
{
        if (multibytecodeset)   /* do extra stuff for multi-byte */
        {
                wchar_t	wcs[BUFSIZ];
        	wchar_t  special[] = { '\"', '\'', ':', ',', '=', 0 };
                wchar_t *wcset; /* pointer to end of token */
                wchar_t *wcsp;
                int     nc;

                /*
                 * convert and copy multibyte string to wide character
                 * process code
                 */
		nc = strlen(buf)+1;
                mbstowcs(wcs, buf, nc);
 
                /*
                 * if the string starts off with a control character
                 * then abort...
                 */
                if (iswcntrl(*wcs))
                        return 0;
 
                /*
                 * find end of token...
                 */
                wcset = wcs;
                while ( !iswspace(*wcset) && 
                        !iswcntrl(*wcset)  && 
                        !wcsrchr(special, *wcset) )
                        	wcset++;

		/*
		 * was something valid found?
		 */
		if ((*wcset == (wchar_t)0) || (wcset == wcs))
			return 0;
 
                /*
                 * make sure next (non-whitespace) character is a ':'
                 */
                wcsp = wcset;
                while (iswspace(*wcsp))
                        wcsp++;
                if (*wcsp != L':')
                        return 0;
 
                *wcset = (wchar_t)0;
 
                wcstombs(out, wcs, nc);
        }
        else    /* normal single-byte characters */
        {
        	char  special[] = { '\"', '\'', ':', ',', '=', 0 };
                char    *cset;  /* pointer to end of token */
                char    *csp;   /* misc. pointer */
 
                /*
                 * if the string starts off with a control character
                 * then abort...
                 */
                if (iscntrl(*buf))
                        return 0;
 
                /*
                 * find end of token...
                 */
                cset = buf;
                while ( !isspace(*cset) && 
                        !iscntrl(*cset)  && 
                        !strchr(special, *cset) )
				cset++;
 
		/*
		 * was something valid found?
		 */
		if ((*cset == (char)0) || (cset == buf))
			return 0;
                /*
                 * make sure next (non-whitespace) character is a ':'
                 */
                csp = cset;
                while (isspace(*csp))
                        csp++;
 
                if (*csp != ':')
                        return 0;
 
                *cset = (char)0;
                strcpy(out, buf);
        }
 
        return out;
}
        
 
/*
 * NAME:        getname
 *
 * FUNCTION:
 *      Locate the name of an attribute in a line which is supposed to
 *      contain an <attribute, value> pair
 *
 * NOTES:
 *      This routines supports multi-byte characters.
 *
 * RETURNS:
 *      NULL on failure, pointer to result array on success.
 */
 
static char *
getname(char *buf, char *out)
{
        if (multibytecodeset)   /* do extra stuff for multi-byte */
        {
        	wchar_t	special[] = { '\"', '\'', ':', ',', '=', 0 };
                wchar_t	wcs[BUFSIZ];
                wchar_t	*wcsbt; /* pointer to beginning of token */
                wchar_t *wcset; /* pointer to end of token */
                wchar_t *wcp;
                int     nc = strlen(buf)+1;

                /*
                 * convert and copy multibyte string to wide character
                 * process code
                 */
                mbstowcs(wcs, buf, nc);
 
                /*
                 * position "wcsbt" to beginning of token...
                 */
                wcsbt = wcs;
                while (iswspace(*wcsbt))
                        wcsbt++;
 
                /*
                 * if first non-whitespace is not alpha, abort...
                 */
                if (!iswalpha(*wcsbt))
                        return 0;
 
                /*
                 * position past token
                 */
                wcset = wcsbt;
                while ( !iswspace(*wcset) && 
                        !iswcntrl(*wcset)  &&
                        !wcsrchr(special, *wcset) )
				wcset++;
 
		/*
		 * was something valid found?
		 */
		if ((*wcset == (wchar_t)0) || (wcset == wcsbt))
			return 0;
 
                /*
                 * if next character is not a '=' (or white space), abort...
                 */
                if ( (*wcset != L'=') && (!iswspace(*wcset)) )
                        return 0;
 
                /*
                 * NULL terminate
                 */
                *wcset = (wchar_t)0;
                wcstombs(out, wcsbt, nc);
        }
        else    /* normal single-byte characters */
        {
		/*
		 * we need to use a tmp buffer here because we don't
		 * want to modify the input buffer (by slipping in a NULL)
		 */
                char	cs[BUFSIZ];
        	char	special[] = { '\"', '\'', ':', ',', '=', 0 };
                char	*csp;
		char	*bp;
 
                /*
                 * position "bp" to beginning of token...
                 */
                bp = buf;
                while (isspace(*bp))
                        bp++;
 
                /*
                 * if first non-whitespace is not alpha, abort...
                 */
                if (!isalpha(*bp))
                        return 0;
 
                /*
                 * position past token (coping as we go)
                 */
                csp = cs;
                while ( !isspace(*bp) && 
                        !iscntrl(*bp)  &&
                        !strchr(special, *bp) )
                        	*csp++ = *bp++;
 
                /*
                 * if next character is not a '=' (or white space), abort...
                 */
                if ( (*bp != '=') && (!isspace(*bp)) )
                        return 0;
 
                /*
                 * NULL terminate
                 */
                *csp = (char)0;
                strcpy(out, cs);
        }
 
        return out;
}
 

static void
interp(char *in, char *out)
{
        enum { quote, normal } state = normal;
        int     octal;
 
        while (*in) 
	{
                if (*in == '\\') 
		{
                        in++;
                        switch (*in) 
			{
                            case 'a':
                                *out++ = '\a';
                                in++;
                                break;
                            case 'b':
                                *out++ = '\b';
                                in++;
                                break;
                            case 'f':
                                *out++ = '\f';
                                in++;
                                break;
                            case 'n':
                                *out++ = '\n';
                                in++;
                                break;
                            case 'r':
                                *out++ = '\r';
                                in++;
                                break;
                            case 't':
                                *out++ = '\t';
                                in++;
                                break;
                            case 'v':
                                *out++ = '\v';
                                in++;
                                break;
                            case '0':
                            case '1':
                            case '2':
                            case '3':
                                if ('0' <= in[0] && in[0] <= '3' &&
                                    '0' <= in[1] && in[1] <= '7' &&
                                    '0' <= in[2] && in[2] <= '7')
				{
                                        octal = ((in[0] - '0') << 6) +
                                                ((in[1] - '0') << 3) +
                                                 (in[2] - '0');
                                        *out++ = octal;
                                        in += 3;
                                        break;
                                }
                            default:
                                *out++ = *in++;
                                break;
                        }
                } 
		else if (*in == '"') 
		{
                        if (state == quote)
                                state = normal;
                        else
                                state = quote;
                        
                        in++;
                }
                else if (isspace(*in))
                {
                        if (state != quote)
                                break;
                        *out++ = *in++;
                }
                else if (state != quote && *in == ',')
                {
                        *out++ = '\0';
                        in++;
			while(isspace(*in))
				in++;
                }
                else
                        *out++ = *in++;
        }
        *out++ = '\0';
        *out = '\0';
}
 

static void
interp_wcs(wchar_t *in, char *out)
{
        enum { quote, normal } state = normal;
        int     octal;
	wchar_t	wcs[BUFSIZ];
	wchar_t	*inp;
	wchar_t	*wcsp;
 
	inp = in;
	wcsp = wcs;

        while (*inp) 
	{
                if (*inp == (wchar_t)'\\') 
		{
                        inp++;
                        switch (*inp) 
			{
                            case L'a':
                                *wcsp++ = L'\a';
                                inp++;
                                break;
                            case L'b':
                                *wcsp++ = L'\b';
                                inp++;
                                break;
                            case L'f':
                                *wcsp++ = L'\f';
                                inp++;
                                break;
                            case L'n':
                                *wcsp++ = L'\n';
                                inp++;
                                break;
                            case L'r':
                                *wcsp++ = L'\r';
                                inp++;
                                break;
                            case L't':
                                *wcsp++ = L'\t';
                                inp++;
                                break;
                            case L'v':
                                *wcsp++ = L'\v';
                                inp++;
                                break;
                            case L'0':
                            case L'1':
                            case L'2':
                            case L'3':
                                if (L'0' <= inp[0] && inp[0] <= L'3' &&
                                    L'0' <= inp[1] && inp[1] <= L'7' &&
                                    L'0' <= inp[2] && inp[2] <= L'7')
				{
                                        octal = ((inp[0] - L'0') << 6) +
                                                ((inp[1] - L'0') << 3) +
                                                 (inp[2] - L'0');
                                        *wcsp++ = octal;
                                        inp += 3;
                                        break;
                                }
                            default:
                                *wcsp++ = *inp++;
                                break;
                        }
                } 
		else if (*inp == L'"') 
		{
                       	if (state == quote)
                       		state = normal;
                       	else
                               	state = quote;
                        	inp++;
               	}
               	else if (iswspace(*inp))
               	{
               		if (state != quote)
                       		break;
               		*wcsp++ = *inp++;
               	}
               	else if ((state != quote) && (*inp == L','))
		{
               		*wcsp++ = L'\0';
               		inp++;
			while(iswspace(*inp))
				inp++;
		}
               	else
                	*wcsp++ = *inp++;
        }
        *wcsp++ = L'\0';
        *wcsp++ = L'\0';

	{	/* copy string of strings (wchar_t to multbyte) */
		char	*op = out;
		wchar_t	*wp = wcs;

		while (*wp != L'\0')
		{
			wcstombs(op, wp, BUFSIZ);
			while (*op++) ;
			while (*wp++ != L'\0') ;
		}
		*op = '\0';
	}
}

 
static int
listlen(char *cp)
{
        char    *start = cp;
        int     i = 0;
 
        while (*cp) {
                while (*cp++)
                        ;
 
                cp++;
        }
        return (cp - start) + 1;
}
 

static char *
getvalue(char *buf, char *out)
{
        if (multibytecodeset)   /* do extra stuff for multi-byte */
        {
                wchar_t	wcs[BUFSIZ];
                wchar_t	*wcp;
                int     nc = strlen(buf)+1;

                /*
                 * convert and copy multibyte string to wide character
                 * process code
                 */
                mbstowcs(wcs, buf, nc);
 
                /*
                 * position wcp to beginning of value string...
                 */
		wcp = wcs;
		while (*wcp)
		{
                	if (*wcp == L'=')
                        	break;
			wcp++;
        	}

		if (*wcp == (wchar_t)0)
			return 0;

        	wcp++;
        	while (iswspace(*wcp))
			wcp++;

                interp_wcs(wcp, out);
        }
        else    /* normal single-byte characters */
        {
                char	*cp;

                /*
                 * position cp to beginning of value string...
                 */
		cp = buf;
		while (*cp)
		{
                	if (*cp == '=')
                        	break;
			cp++;
        	}

		if (*cp == (char)0)
			return 0;

        	cp++;
        	while (isspace(*cp))
			cp++;

                interp(cp, out);
        }
 
        return out;
}
 
 
char *
afread(AFILE_t af)
{
        int     cnt = 0;
        int     i;
        int     len;
        int     error = 0;
        int     size;
        long    mark;
        enum { looking, key, values } state = looking;
        char    buf[BUFSIZ];
        char    *cp;
        struct  ATTR    current;
 
        /*
         * All of the attribute values are stored in a buffer that
         * is associated with the attribute file.  The size of the
         * buffer is set at open time when the storage is allocated.
         */
 
        cp = af->AF_cbuf;
        size = sizeof buf > af->AF_rsiz ? af->AF_rsiz:sizeof buf;
 
        /*
         * Read lines.  Each line is stripped of trailing blanks
         * and tested for being a comment.  Comment lines are
         * tossed as are leading blank lines.  The key [ stanza
         * name ] is then searched for and assumed to be the
         * first non-blank, non-comment line.  After the key a
         * list of attributes is searched for.  This process is
         * terminated with a blank line or end of file.
         */
 
        while (mark = ftell(af->AF_iop), fgets(buf, size, af->AF_iop)) {
 
                /*
                 * Remove the trailling newline and any trailing
                 * whitespace.  Initialize the current attribute
                 * entry to NULL.
                 */
 
                buf[len = strlen(buf) - 1] = '\0';
                blankstrip(buf);
 
                current.AT_name = 0;
                current.AT_value = 0;
 
                /*
                 * See if this is a blank line.  If we have already
                 * found the key then we break as the stanza has been
                 * read in completely.  Otherwise we keep skipping
                 * leading blank lines and comment lines as well.
                 */
 
                if (buf[0] == '\0') {
                        if (state != looking)
                                break;
                        else
                                continue;
                }
                if (buf[0] == '*')
                        continue;
 
 
                /*
                 * All lines after an error is found are skipped.  The
                 * goal is to resynchronize after a blank line.
                 */
                 
                if (error)
                        continue;
 
                /*
                 * The current line is non-blank and must either be the
                 * beginning of a stanza [ state == looking ] or the
                 * next attribute in a stanza [ state == values ].
                 */ 
 
                if (state == looking)
                        state = key;
 
                if (state == key) {
 
                        /*
                         * This line must contain a valid stanza name.
                         * Get the name and save it off in the buffer.
                         */
 
                        if (!(current.AT_value = getkey(buf, cp))) {
                                error++;
                        } else {
                                cp = cp + strlen(current.AT_value) + 1;
                                current.AT_name = "";
                        }
 
                        /*
                         * The remaining lines must be attribute values.
                         */
 
                        state = values;
                } else if (state == values) {
 
                        /*
                         * See if I can have any more attributes values
                         * in this stanza.  Flag the error if not.
                         */
 
                        if (cnt >= af->AF_natr - 1) {
                                error++;
                                continue;
                        }
 
                        /*
                         * An attribute consists of a name, followed by
                         * a value.  They are separated from each other
                         * by an equal sign.
                         *
                         * Get the attribute name and space the buffer
                         * pointer forward by the size of the name.
                         */
 
                        if (!(current.AT_name = getname(buf, cp))) {
                                if (strchr(buf, ':')) {
                                        fseek(af->AF_iop, mark, 0);
                                        break;
                                }
                                error++;
                                continue;
                        } else {
                                cp = cp + strlen(current.AT_name) + 1;
                        }
 
                        /*
                         * Now go search for the value.  The value can
                         * be a double-null terminated list so the
                         * buffer pointer is moved forward using listlen.
                         */
 
                        if (!(current.AT_value = getvalue(buf, cp))) {
                                error++;
                                continue;
                        } else {
                                cp = cp + listlen(current.AT_value) + 1;
                        }
                }
 
                /*
                 * Save the info for the current attribute and bump
                 * the count.
                 */
 
                af->AF_catr[cnt++] = current;
        }
 
        /*
         * All errors are treated as EINVALs.  The return code for
         * and actual error is rc == 0 and errno == EINVAL.  For
         * end-of-file the return code is rc == 0 and errno == 0.
         * The programmer can not merely test for rc == 0!
         */
 
        if (error) {
                errno = EINVAL;
                return 0;
        }
        if (cnt == 0) {
                errno = 0;
                return 0;
        }
 
        /*
         * Terminate the attribute list with a NULL attribute entry
         * and return a pointer to the end of the buffer.  This seems
         * fairly pointless but is for backwards compatibility.
         */
 
        current.AT_name = 0;
        current.AT_value = 0;
        af->AF_catr[cnt++] = current;
 
        return cp;
}
 
 
afrewind(af)
register AFILE_t af;
{
        /* deallocate the hash table */
        affree(af);
        fseek(af->AF_iop,0L,0);
}
 
 
ATTR_t
aflookup(af, name)
AFILE_t af;
char    *name;
{
        struct  stanza  *s;
        int     h;
 
        /* if we ran out of memory saving stanzas, scan the whole file again */
        if (HOVERFLOW(af))
        {
                afrewind(af);
                return (NULL);
        }
 
        hash(h, name);
        for (s = HTAB(af)[h]; s; s = s -> next)
        {
                if (strcmp(s -> atr[0].AT_value, name) == 0)
                {
                        return (s -> atr);
                }
        }
        return (NULL);
}
 
 
/*
 * save the contents of a stanza in a malloc()'d structure.
 * the result is linked into the hash table,
 * keyed by the name of the stanza
 */
 
afsave(af,at)
AFILE_t af;
ATTR_t  at;
{
struct  stanza  **htab = HTAB(af);
struct  stanza  *stz;
int     n;
char    *bufp;
 
        /* don't try saving stanzas if we ran out of memory */
        if (HOVERFLOW(af))
                return;
 
        /* allocate structure to hold stanza */
        {
                ATTR_t  atp;
                int     len;
                int     i;
 
                /* space to hold a null string ("") */
                len = 1;
                /* space to hold name of stanza */
                len += strlen(at -> AT_value) + 1;
 
                for (atp = at + 1;(atp->AT_name && atp->AT_name[0]);atp++)
                {
                        /* space to hold attribute name */
                        len += strlen(atp -> AT_name) + 1;
                        /* space to hold attribute value */
                        LISTLEN(i, atp -> AT_value);
                        len += i;
                }
                /* increment for trailing NULL,NULL */
                len += 2;
                /*
                 * number of <name,value> pairs:
                 * include leading <"",name> and trailing <NULL,NULL>
                 */
                n = atp - at + 1;
                /* allocate save structure with buffer following */
                len = sizeof(struct stanza) + n * sizeof(struct ATTR) + len;
                DPRINTF(("AFsave(): allocate %d bytes\n", len));
                stz = (struct stanza *) malloc(len);
                if (stz == NULL)
                {
                        /* out of memory! */
                        affree(af);
                        HOVERFLOW(af) = 1;
                        return;
                }
        }
 
        {
                /* name and value pointers follow save structure */
                bufp = (char *) stz + sizeof(struct stanza);
                stz -> atr = (ATTR_t) bufp;
                /* strings follow name and value pointers */
                bufp += n * (sizeof(stz -> atr[0]));
        
                /*
                 * copy name into attribute 0 of save structure:
                 * "name" is ""
                 * "value" is stanza name
                 */
                stz -> atr[0].AT_name = bufp;
                *bufp++ = 0;
                stz -> atr[0].AT_value = bufp;
                STRCPY(bufp, at -> AT_value);
        
                /* copy attributes into save structure */
                {
                        ATTR_t  atp, atq;
        
                        atp = &(stz -> atr[1]);
                        atq = at + 1;
                        for ( ; (atq->AT_name && atq->AT_name[0]); atp++, atq++)
                        {
                                /* copy name into save structure */
                                atp -> AT_name = bufp;
                                STRCPY(bufp, atq -> AT_name);
                                /* copy value into save structure */
                                atp -> AT_value = bufp;
                                LISTCPY(bufp, atq -> AT_value);
                        }
                        atp -> AT_name = atp -> AT_value = 0;
                }
 
                DPRINTF(("afsave(): used %d bytes\n", bufp - (char *) stz)); 
        
                /* insert name into hash table */
                {
                        int     h;
        
                        hash(h, stz -> atr[0].AT_value);
                        stz -> next = htab[h];
                        htab[h] = stz;
                }
        }
}


/*
 * fast grep through stanza files; stop at given name or default
 * return 1 if found; 0 if EOF
 */
static int
afgreprec(char *name, FILE *fp)
{
#	define iswhite(c) ((c) == ' ' || (c) == '\t')

	char	line[BUFSIZ+1];
	char	tname[128], dname[128];
	long	offset = 0L;
	int	slen, dlen;

	strcpy(tname, name);
	strcat(tname, ":");
	slen = strlen(tname);

	strcpy(dname, DEFAULT);
	strcat(dname, ":");
	dlen = strlen(dname);

	offset = ftell(fp);

	while (fgets(line, BUFSIZ, fp)) 
	{
		if ((!iswhite(*line)) &&
		    ((strncmp(line, tname, slen) == 0) ||
		     (strncmp(line, dname, dlen) == 0)))
		{
			fseek(fp, offset, 0);
			return 1;
		}
		offset += strlen(line);
	}

	return 0;
}

