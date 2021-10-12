static char sccsid[] = "@(#)80    1.19.3.14  src/bos/usr/ccs/lib/libdbx/source.c, libdbx, bos411, 9433A411a 8/15/94 14:21:10";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: canReadSource
 *		edit
 *		findsource
 *		free_seektab
 *		getpattern
 *		getsrcpos
 *		getsrcwindow
 *		opensource
 *		printlines
 *		printsrcpos
 *		re_exec
 *		search
 *		setsource
 *		sindex
 *		skimsource
 *		slot_alloc
 *		slotno
 *		srcaddr
 *		srclang
 *		update_source
 *		
 *
 *   ORIGINS: 26,27, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Copyright (c) 1982 Regents of the University of California */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Source file management.
 */

#include "defs.h"
#include "source.h"
#include "object.h"
#include "mappings.h"
#include "machine.h"
#include "keywords.h"
#include "tree.h"
#include "eval.h"
#include "main.h"
#include "envdefs.h"
#   define R_OK 04	/* read access */
#   define L_SET 01	/* absolute offset for seek */

#   define re_exec(buf) (regex(buf) != NULL)

extern char *regcmp();
extern char *regex();
extern char *__loc1;
extern int sourcefiles;

public String cursource;
public String curdir;		/* Directory current source is found in */
public String prevdir;		/* Directory previous source was found in */
public Lineno curline;
public Lineno cursrcline;

/*
 *  The current language of the source file being VIEWED.
 *  This was added for use by dpi_src_lang.
 */
public int cursrclang;
				   
public int windowsize = 10;
public int inst_windowsize = 10;
public List sourcepath;

Lineno lastlinenum;
public String prevsource = nil;


/*
 * Data structure for indexing source seek addresses by line number.
 *
 * The constraints are:
 *
 *  we want an array so indexing is fast and easy
 *  we don't want to waste space for small files
 *  we don't want an upper bound on # of lines in a file
 *  we don't know how many lines there are
 *
 * The solution is a "dirty" hash table.  We have NSLOTS pointers to
 * arrays of NLINESPERSLOT addresses.  To find the source address of
 * a particular line we find the slot, allocate space if necessary,
 * and then find its location within the pointed to array.
 */

typedef long Seekaddr;

#define NSLOTSALLOC 20
#define NLINESPERSLOT 500

#define slotno(line)    ((line) / NLINESPERSLOT)
#define sindex(line)	((line) % NLINESPERSLOT)
#define slot_alloc()    newarr(Seekaddr, NLINESPERSLOT)
#define srcaddr(line)	seektab[slotno(line)][sindex(line)]

public  File srcfp;
private Seekaddr **seektab = nil;
private int nslots = NSLOTSALLOC;

/*
 * Determine if the current source file is available.
 */

public boolean canReadSource ()
{
    boolean b;

    if (cursource == nil) {
	b = false;
    } else if ((cursource != prevsource) || (curdir != prevdir)) {
	skimsource();
	b = (boolean) (lastlinenum != 0);
    } else {
	b = true;
    }
    return b;
}

/*
 * Print out the given lines from the source.
 */

public printlines(l1, l2)
Lineno l1, l2;
{
    register int c, j;
    register Lineno i, lb, ub;
    register File f;
    char line[MAXLINESIZE];
    char *cptr;
#ifdef KDBX
    register Lineno cur_line;
#endif /* KDBX */

    if (cursource == nil) {
	beginerrmsg();
	(*rpt_error)(stderr, "no source file\n");
    } else {
        /*  if the source file has changed since the last time
              we attempted to list source or we were unable
              to find the source file the last time we
              attempted to list source - last check is necessary
              for the case where the user has copied the file
              to a known location after being unable to read it  */
	if ((cursource != prevsource) || (curdir != prevdir)
          || (lastlinenum == 0))
        {
	    skimsource();
	}
	if (lastlinenum == 0) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_source, MSG_272,
					 "couldn't read \"%s\"\n"), cursource);
	} else {
	    lb = (l1 == LASTLINE) ? lastlinenum : l1;
	    ub = (l2 == LASTLINE) ? lastlinenum : l2;
	    if (lb < 1) {
		   beginerrmsg();
		   (*rpt_error)(stderr,  catgets(scmc_catd, MS_source,
				   MSG_273, "line number must be positive\n"));
	    } else if (lb > lastlinenum) {
		beginerrmsg();
		if (lastlinenum == 1) {
		    (*rpt_error)(stderr,  catgets(scmc_catd, MS_source,
			      MSG_327, "\"%s\" has only 1 line\n"),
                              basefile(cursource));
		} else {
		    (*rpt_error)(stderr,  catgets(scmc_catd, MS_source,
			      MSG_328, "\"%s\" has only %d lines\n"),
                              basefile(cursource), lastlinenum);
		}
	    } else if (ub < lb) {
		beginerrmsg();
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_source, MSG_340,
				"second number must be greater than first\n"));
	    } else {
		if (ub > lastlinenum) {
		    ub = lastlinenum;
		}
		f = srcfp;
		fseek(f, srcaddr(lb), 0);
#ifdef KDBX
                cur_line = srcline(pc);
#endif /* KDBX */
		for (i = lb; i <= ub; i++)
		{
#ifdef KDBX
		    (*rpt_output)(stdout, "%c%c%4d  ",
					(i == cursrcline)? '>' : ' ',
					(i == cur_line)  ? '*' : ' ', i);
#else /* KDBX */
		    (*rpt_output)(stdout, "%5d   ", i);
#endif /* KDBX */
		    j = 0;
	 	    while (((line[j++] = getc( f )) != '\n') &&
		           (j < MAXLINESIZE - 1))
			 ;
		    line[j] = '\0';
		    (*rpt_output)(stdout, "%s", line );
		}
		cursrcline = ub + 1;
	    }
	}
    }
}

/*
 * NAME: findsource
 *
 * FUNCTION: findsource uses the 'use' path to resolve the source    
 *             location. 
 *
 * NOTES: The search algorithm follows :
 *
 *          Use the filename or relative path
 *          in conjunction with the use path to search for the
 *          file.
 * 
 *          If not found, and a relative path was input, use the
 *          filename of the relative path in conjunction with
 *          the use path to search for the file.
 * 
 * PARAMETERS:
 *       filename : input filename
 *       search_filetable : pointer to 3 way flag
 *                          = NULL if file table should not be searched
 *                          = -> FILE_CMD if called because of the 
 *                            file command - file table may be searched
 *                          If cursource gets set in findsource, the
 *                          value pointed to is set to zero to let
 *                          the caller know.
 *                          = -> EDIT_CMD if called because of the 
 *                            edit command - file table may be searched
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: calls resolveFilename which looks at filetab
 *
 * RETURNS: pointer to a constructed path or pointer to String
 *          filename in the file table or input filename or
 *          NULL.
 *
 */

/*  fileNameBuf has to be large enough to hold the maximum path
      size (PATH_MAX) + the maximum relative path size (PATH_MAX)
      + the '/' in between + the null terminator  */
static char fileNameBuf[(2 * PATH_MAX) + 2];

public String findsource(filename, search_filetable)
String filename;
unsigned char *search_filetable;
{
    register String src, dir;
    String filename_ptr;
    String save_filename = NULL;
    Boolean full_path = false;
    Boolean relative_path = false;
    String out_filename;
    Boolean done = false;
    
    if ((filename_ptr = rindex (filename, '/')) != NULL)
    {
      /*  set filename_ptr to point to the basename
          or relative pathname  */

      if ((filename[0] == '/')
       || (!strncmp(filename, "./", 2))
       || (!strncmp(filename, "../", 3)))
      {
        /*  treat anything starting with "/", "./" or "../" as
              a fullpath  */
        filename_ptr++;
        full_path = true;
      }
      else
      {
        save_filename = filename_ptr++;
        filename_ptr = filename;
        relative_path = true;
      }
    }
    else
    {
      filename_ptr = filename;
    }

    /*  if the name came from the file command or the edit command
          and it is a full path  */
    if (((filecmdcursrc == filename) || search_filetable) && full_path)
    {
      /*  attempt to locate the file as input - ignore the use path  */

      if (access(filename, R_OK) == 0) 
        return(filename);
      else 
        return (NULL);
    }
    
    while (!done)
    {
      foreach (String, dir, sourcepath)
        /*  if the use path says to use the full-path  */
        if (streq(dir, "@"))
        {
          /*  if called because of the file command or the edit command  */
          if (search_filetable)
          {
            if ((out_filename = resolveFilename(filename_ptr, 0, NULL,
                                         NULL, NULL, NULL)) != NULL)
            {
              if (access(out_filename, R_OK) == 0)
              {
                /*  if called because of the file command  */
                if (*search_filetable == FILE_CMD)
                {
                  /*  reset cursource  */
                  setsource(out_filename);
                  *search_filetable = 0;
                }
                curdir = dir; 
                return(out_filename);
              }
            }
          }
          /*  if the object contains path info  */
          else if (full_path)
          {
            /*  if the file exists in this location  */
            if (access(filename, R_OK) == 0)
            {
              curdir = dir;
              return (filename);
            }
          }
          /*  else - no path info - go to the next entry  */
        }
        else
        {
          /*  concatenate the path and the filename into fileNameBuf  */
          sprintf(fileNameBuf, "%s/%s", dir, filename_ptr);

          /*  if the file exists in this location  */
          if (access(fileNameBuf, R_OK) == 0)
          {
            /*  use this file  */
            curdir = dir;
            return (fileNameBuf);
          }
        }
      endfor

      /*  if a relative path was input and we haven't checked
            the base filename yet  */
      if (relative_path && (filename_ptr != save_filename))
        filename_ptr = save_filename;
      else
        done = true;
    }

    curdir = dir;
    return (NULL);
}

/*
 * NAME: opensource
 *
 * FUNCTION: open a source file looking in the appropriate places    
 *
 * PARAMETERS:
 *       filename : input filename
 *       search_filetable : pointer to 3 way flag
 *                          = NULL if file table should not be searched
 *                          = -> FILE_CMD if called because of the 
 *                            file command - file table may be searched
 *                          If cursource gets set in findsource, the
 *                          value pointed to is set to zero to let
 *                          the caller know.
 *                        
 *                          EDIT_CMD should not ever be set in
 *                          this routine
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none
 *
 * RETURNS: file pointer or nil if the file could not be found
 *
 */

public File opensource(filename, search_filetable) 
String filename;
unsigned char *search_filetable;
{
    String s;
    File f;

    s = findsource(filename, search_filetable);
    if (s == nil) {
	f = nil;
    } else {
	f = fopen(s, "r");
    }
    return f;
}

/*
 * Set the current source file.
 */

public setsource(filename)
String filename;
{
    if (filename != nil and filename != cursource) {
	prevsource = cursource;
	cursource = filename;
	cursrcline = 1;
	cursrclang = srclang(cursource);
    }
}

public update_source ()
{
	findsource(cursource, NULL);
	if( isXDE ) {
	    cursrcline++;
	}
}

/*
 * Erase information and release space in the current seektab.
 * This is in preparation for reading in seek pointers for a
 * new file.  It is possible that seek pointers for all files
 * should be kept around, but the current concern is space.
 */

void free_seektab()
{
    register int slot;

    if( seektab != NULL ) {
	for (slot = 0; slot < nslots; slot++) {
	    if (seektab[slot] != nil) {
		dispose(seektab[slot]);
	    }
	}
    }
}

/*
 * Read the source file getting seek pointers for each line.
 */

public skimsource()
{
    register int c;
    register Seekaddr count;
    register File f;
    register Lineno linenum;
    register Seekaddr lastaddr;
    register int slot;

    f = opensource(cursource, NULL);
    if (seektab == nil) {
	seektab = (Seekaddr **) calloc(nslots, sizeof(Seekaddr *));
    }
    if (f == nil) {
	lastlinenum = 0;
        prevsource = cursource;
        prevdir = curdir;
    } else {
	if (prevsource != nil) {
	    free_seektab();
	    if (srcfp != nil) {
		fclose(srcfp);
	    }
	}
	prevsource = cursource;
	prevdir = curdir;

	linenum = 0;
	count = 0;
	lastaddr = 0;
	while ((c = getc(f)) != EOF) {
	    ++count;
	    if (c == '\n') {
		slot = slotno(++linenum);
		if (slot >= nslots) {
		    nslots += NSLOTSALLOC;
		    seektab = (Seekaddr **) realloc(seektab, nslots*sizeof(Seekaddr *));
		    bset0(&seektab[nslots-NSLOTSALLOC],NSLOTSALLOC*sizeof(Seekaddr *));
		}
		if (seektab[slot] == nil) {
		    seektab[slot] = slot_alloc();
		}
		seektab[slot][sindex(linenum)] = lastaddr;
		lastaddr = count;
	    }
	}
	lastlinenum = linenum;
	srcfp = f;
    }
}

/*
 * Figure out current source position.
 */

public getsrcpos()
{
    String filename;

    curline = srcline(pc);
    if (curline != 0) {
        filename = srcfilename(pc);
        setsource(filename);
	cursrcline = curline;
    }
}

/*
 * Print out the current source position.
 */

public printsrcpos()
{
    extern int inclfiles;

    (*rpt_output)(stdout, "at line %d", curline);
    if ((sourcefiles > 1) || (inclfiles)){
	(*rpt_output)(stdout, " in file \"%s\"", basefile(cursource));
    }
}

#define DEF_EDITOR  "vi"

/*
 * Invoke an editor on the given file.  Which editor to use might change
 * installation to installation.  For now, we use "vi".  In any event,
 * the environment variable "EDITOR" overrides any default.
 */

public edit(filename)
String filename;
{
    extern String getenv();
    String ed, src, s;
    Symbol f;
    Address addr;
    char lineno[10];
    unsigned char search_filetable = EDIT_CMD;

    ed = getenv("EDITOR");
    if (ed == nil) {
	ed = DEF_EDITOR;
    }
    if (filename != nil)
    {
      src = findsource(filename, &search_filetable);
    }
    else
    {
      src = findsource(cursource, NULL);
    }

    if (src == nil) {
        Name fpName = identname(filename, true);
        Node n = resolveName(nil, build(O_NAME, fpName), nil, WOTHER, true);
        if (filename == nil)
          filename = cursource;

	if (n == nil)
	    error(catgets(scmc_catd, MS_source, MSG_352,
		  "cannot read \"%s\""), filename);

        assert(n->op == O_SYM);
        f = n->value.sym;

	if (not isblock(f)) {
	    error( catgets(scmc_catd, MS_source, MSG_352,
					       "cannot read \"%s\""), filename);
	}
	addr = firstline(f);
	if (addr == NOADDR) {
	    error(catgets(scmc_catd, MS_source, MSG_353,
		  "no source for \"%s\""), filename);
	}
	src = srcfilename(addr);
	if (src == nil) {
	    error(catgets(scmc_catd, MS_source, MSG_353,
		  "no source for \"%s\""), filename);
	}
	s = findsource(src, NULL);
	if (s != nil) {
	    src = s;
	}
	sprintf(lineno, "+%d", srcline(addr));
    } else {
	if (filename != nil) {
	     sprintf(lineno, "+1");
	} else {
	     sprintf(lineno, "+%d", curline);
	}
    }
    if (streq(ed, "vi") or streq(ed, "ex"))	/* vi or ex */
	call(ed, stdin, stdout, lineno, src, nil);
    else if (!strncmp(ed, "emacs", 5))		/* emacs or emacsclient */
	call(ed, stdin, stdout, lineno, src, nil);
    else if (strlen(ed) > 5 &&			/* gnuemacs, etc. */
	!strcmp(ed + (strlen(ed) - 5), "emacs"))
	call(ed, stdin, stdout, lineno, src, nil);
    else
	call(ed, stdin, stdout, src, nil);
}

/*
 * Strip away portions of a given pattern not part of the regular expression.
 */

private String getpattern (pattern)
String pattern;
{
    register char *p, *r;

    p = pattern;
    while (*p == ' ' or *p == '\t') {
	++p;
    }
    r = p;
    while (*p != '\0') {
	++p;
    }
    --p;
    if (*p == '\n') {
	*p = '\0';
	--p;
    }
    if (*p == *r) {
	*p = '\0';
	--p;
    }
    return r + 1;
}

/*
 * Search the current file for a regular expression.
 */

public search (direction, pattern)
char direction;
String pattern;
{
    register int c;
    register String p;
    register File f;
    String re;
    static String pat;
    Lineno line;
    Lineno marker;
    String matched;
    char buf[512];

    if (cursource == nil) {
	beginerrmsg();
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_source, MSG_356,
							  "no source file\n"));
    } else {
	if (cursource != prevsource) {
	    skimsource();
	}
	if (lastlinenum == 0) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_source, MSG_272,
					 "couldn't read \"%s\"\n"), cursource);
	} else {
	    marker = (cursrcline > lastlinenum ||
		      cursrcline == 1) ? lastlinenum : cursrcline - 1;
	    re = getpattern(pattern);
	    /* circf = 0; */
	    if (re != nil and *re != '\0') {
		pat = regcmp(re,0);
		if (pat == nil) {
		    beginerrmsg();
		    (*rpt_error)(stderr,  catgets(scmc_catd, MS_source,
				     MSG_358, "invalid regular expression\n"));
		    return;
		}
	    }
	    matched = false;
	    f = srcfp;

	    /*
	     * Since cursrcline points to the first line to be printed by the
	     * list command, set line to point to the line preceding cursrcline.
	     * This sets line to indicate where to start the search, the do loop
	     * will either increment or decrement this depending on the search
	     * direction.
	     */
	    line = cursrcline - 1;
	    do {
		if (direction == '/') {
		    ++line;
		    if (line > lastlinenum) {
			line = 1;
		    }
		} else {
		    --line;
		    if (line < 1) {
			line = lastlinenum;
		    }
		}
		fseek(f, srcaddr(line), 0);
		p = buf;
		c = getc(f);
		while (c != '\n' and c != EOF and p < &buf[511]) {
		    *p++ = c;
		    c = getc(f);
		}
		*p = '\0';
		matched = regex(pat,buf);
	    } while ((matched == NULL) and line != marker);
	    if (matched == NULL) {
		beginerrmsg();
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_source, MSG_359,
								"no match\n"));
	    } else {
		printlines(line, line);

		/*
		 * cursrcline should be set to the line after the last line
		 * printed.  In this case, line + 1
		 */
		cursrcline = line + 1;
	    }
	}
    }
}

/*
 * Compute a small window around the given line.
 */

public getsrcwindow (line, l1, l2)
Lineno line, *l1, *l2;
{
    Node s;
    integer size;

    /* read current source file if different */
    if ((cursource != prevsource) || (curdir != prevdir)) 
    {
      skimsource();
    }

    s = findvar(identname("$listwindow", true));
    if (s == nil) {
      size = 10;
    } else {
      eval(s);
      size = pop(integer);
    }

    *l1 = line - (size / 2);
    if (*l1 < 1) {
	*l1 = 1;
    }
    *l2 = *l1 + size;
    if (lastlinenum != LASTLINE and *l2 > lastlinenum) {
	*l2 = lastlinenum;
    }
}

/*
 *  Find the source language of the given file.
 *  This is used to update the global cursrclang whenever
 *  the source file being viewed is changed.
 */

public srclang(filename)
String filename;
{
    char *suffix;

    suffix = strrchr(filename, '.');

    if (strcmp(suffix, ".c") == 0)
	return C_LANG;
    else if (strcmp(suffix, ".h") == 0 || strcmp(suffix, ".C") == 0)
	return CPLUSPLUS_LANG;
    else if (strcmp(suffix, ".pas") == 0)
	return PASCAL_LANG;
    else if (strcmp(suffix, ".f") == 0)
	return FORTRAN_LANG;
    else if (strcmp(suffix, ".cbl") == 0)
	return COBOL_LANG;
    else
	return UNSUPPORTED_LANG;
}
