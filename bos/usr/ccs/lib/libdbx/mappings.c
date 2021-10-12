static char sccsid[] = "@(#)60	1.45.4.1  src/bos/usr/ccs/lib/libdbx/mappings.c, libdbx, bos41J, 9511A_all 2/7/95 16:33:51";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: addrtoobj, checkentries, checktables, clrfunctab, cmpfunc,
 *	      cmplines, dataobj, dumpFileTable, dumpfunctab, findline,
 *	      linelookup, newfunc, nextline, objaddr, ordfunctab, ordlineaux,
 *	      skipentries, srcfilename, srcline, textobj, whatblock,
 *	      addrtoincl, checknestedincls, getnextincl
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Source-to-object and vice versa mappings.
 */

#include "defs.h"
#include "symbols.h"
#include "source.h"
#include "object.h"
#include "machine.h"
#include "languages.h"
#include "process.h"
#include <signal.h>
#include <ldfcn.h>
#include <exceptab.h>
#include <scnhdr.h>
#include <aouthdr.h>

public Filetab **filetab;
public Linetab **linetab;
public Lineaux *lineaux;
extern cases casemode, symcase;
extern Address minlineaddr;
extern int loadcnt;
extern boolean hascobolsrc;
extern unsigned int exceptptr;
extern Language cppLang;

/*
 * sys_siglist is provided through libc.a; It contains a message for each signal
 */
extern char	*sys_siglist[];

public AddrOfFunc *functab;
public int nfuncs = 0;
public int functablesize = 0;
private AddrOfFunc *foundfunc;
private Boolean searching = false;	/* Trace table search indicator */
private int lnnochunks = 0;         	/* # of funcs with line numbers */
private Address maxlineaddr = 0;

private skipentries();
boolean skipIncl();
boolean findtable();
static Lineaux * findchunk();
static Linetab * find_linetab_entry();

/*
 * Get the source file name associated with a given address.
 */

public String srcfilename(addr)
Address addr;
{
    register Address i, j, k;
    Address a;
    Filetab *fp;
    Boolean notminusg = false;
    Incfile *inc_file;

    if (nfiles_total != 0) {
    /* No longer have direct addr->file correlation.
     * Could have &file1.func1 < &file2.func2 < &file1.func3!
     * So, go from addr->func->file.
     */
	searching = true;
	whatblock(addr);
	searching = false;
	if ((foundfunc != nil) && (!(nosource(foundfunc->func)))) {
		fp = foundfunc->filep;
                curlang = foundfunc->func->language;
                if(casemode == filedep)
                     symcase = (*language_op(curlang, L_FOLDNAMES))();

		if (fp->incl_chain) { /* Check for includes */
		     inc_file = addrtoincl(fp->incl_chain, addr);
		     if (inc_file) {
			return inc_file->filename;
		     } else {
			return fp->filename;
		     }
		} else {
		     return fp->filename;
		}
	} else {
	        symcase = mixed;
		return nil;
	}
    } else {
	symcase = mixed;
	return nil;
    }
}

/*
 * NAME: findline
 *
 * FUNCTION: find the line associated with the given address.
 *
 * PARAMETERS: 
 *        addr     - the input address 
 *        location - flag to determine what should be returned
 *                   if this address does not map exactly to
 *                   a line number
 *                   EXACT - NULL is returned.
 *                   NEXT - the next line number below the address
 *                          is returned.
 *                   PREVIOUS  - the closest line number above the
 *                               address is returned, unless stopped
 *                               in a prolog, and then the closest
 *                               line number below the address is
 *                               returned.
 *
 * NOTES:
 *        lineaux is an array of Lineaux structures.  Each element
 *          represents a "chunk" of line numbers.  Each lineaux
 *          element, or chunk is an array of Lineno structures.
 *        It is important to note that there is still one
 *          physical line number table and the "auxiliary"
 *          line number table puts it in a sensible order.
 *
 *   ----------------------             linetab
 *   |                    |       _____________________
 *   |    lineaux         |       |                   |
 *   |   __________       ------->|                   |
 *   |   |        |----           |___________________|
 *   |   |________|   |---------->|                   |
 *   ----|        |               |                   |
 *       |________|               |                   |
 *       |        |------         |___________________|
 *       |________|     |         |                   |
 *                      --------->|                   |
 *                                |                   |
 *                                |___________________|
 * 
 *        This is confusing because it is not clear at first 
 *          glance that we are dealing with arrays.
 *
 *        A better picture from the point of view of this function
 *          follows
 *
 *
 *          lineaux[0]                        lchunk[0] 
 *      ____________________            _____________________
 *      |                  |            |                   |
 *      |                  |----------->|    addr           |
 *      |   lp             |            |    line           |
 *      |                  |            |___________________|
 *      |                  |
 *      |                  |                  lchunk[1]
 *      |                  |            _____________________
 *      |                  |            |                   |
 *      |                  |----------->|    addr           |
 *      |                  |            |    line           |
 *      |                  |            |___________________|
 *      |                  |                    .              
 *      |                  |                    .              
 *      |                  |                    .              
 *      |                  |                  lchunk[n]        
 *      |                  |            _____________________
 *      |                  |            |                   |
 *      |   lend           |----------->|    addr           |
 *      |                  |            |    line           |
 *      |__________________|            |___________________|
 *              .
 *              .
 *              .
 *     lineaux[lnnochunks - 1]                lchunk[0]
 *      ____________________            _____________________
 *      |                  |            |                   |
 *      |                  |----------->|    addr           |
 *      |   lp             |            |    line           |
 *      |                  |            |___________________|
 *      |                  |
 *      |                  |                  lchunk[1]        
 *      |                  |            _____________________
 *      |                  |            |                   |
 *      |                  |----------->|    addr           |
 *      |                  |            |    line           |
 *      |                  |            |___________________|
 *      |                  |                    .              
 *      |                  |                    .              
 *      |                  |                    .              
 *      |                  |                  lchunk[n]        
 *      |                  |            _____________________
 *      |                  |            |                   |
 *      |   lend           |----------->|    addr           |
 *      |                  |            |    line           |
 *      |__________________|            |___________________|
 *
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: filetab
 *
 * RETURNS: the index of the line table entry or NULL as defined
 *          by location parameter.
 *
 */

/*  if this is the last entry in a chunk
      return first entry of next chunk - else the next entry of this chunk
 */

#define get_next_lineentry(linetab_entry, chunkp) \
  (linetab_entry == chunkp->lend) ? ((chunkp+1)->lp) : (linetab_entry+1)

public Linetab *findline (addr, location)
Address addr;
line_loc location;
{
    int low, high, mid;
    Symbol f1, f2;
    Linetab *r = NULL, *linetab_entry;
    Address line_addr;                /*  address associated with line  */
    unsigned int ldndx;
    Lineaux *chunkp;

    /*  Line number table is no longer necessarily in ascending order
     *  due to ld() symbol table reordering.  Thus, an auxiliary line
     *  number index has been constructed to be able to binary search
     *  on chunks of line numbers, then finally to find the actual
     *  line. 
     */

    ldndx = addrtoobj(addr);

    /*  if there is no line number info  */
    if (nlhdr[ldndx].nlines == 0)
      return NULL;

    /*  if the addr is less than the smallest address with a line number  */
    if (addr < minlineaddr)
    {
      /*  if looking for exact match OR location is PREVIOUS and
            there is no line number info associated with the function
            this address is in  */
      if ((location == EXACT) 
       || ((location == PREVIOUS) && nosource(whatblock(addr))))
        return NULL;
    
      r = lineaux[0].lp;
    }
    else if (addr > lineaux[lnnochunks -1].lend->addr)
    {
      /*  if location is PREVIOUS and there is line number info
            associated with the function this address is in  */
      if ((location == PREVIOUS) && !nosource(whatblock(addr)))
        /*  must be in an epilog  */
        r = lineaux[lnnochunks -1].lend;
      else
        return NULL;
    }
    else
    {
      /*  find out which chunk the address is in  */
      chunkp = findchunk(addr);

      /*  find the closest linetab entry above addr  */
      linetab_entry = find_linetab_entry(chunkp->lp, chunkp->lend, addr);

      line_addr = linetab_entry->addr;

      if (line_addr == addr)
        r = linetab_entry;
      else if (location == EXACT)
        return NULL;
      else if (location == NEXT)
        r = get_next_lineentry(linetab_entry, chunkp);
      else if (location == PREVIOUS)
      {
        /*  find out which block we are stopped in  */
        f1 = whatblock(addr);

        /*  if this block does not have line number info  */
        if (nosource(f1))
          return NULL;

        /*  check out what block this line is in  */
        f2 = whatblock(line_addr);

        /*  if addr and line_addr are in the same block  */
        if (f1 == f2)
          r = linetab_entry;
        else
          /*  must be stopped in a prolog  */
          /*  get the "next" line number  */
          r = get_next_lineentry(linetab_entry, chunkp);
      }
    }

    /*  don't know why this needs to be here  */
    for (;(r < &linetab[ldndx][nlhdr[ldndx].nlines])
         && (r->addr == r[1].addr);r++)
      ;  /*  do nothing  */
    return r;
}

/*
 * NAME: findchunk
 *
 * FUNCTION: perform a binary search of the auxilliary line number   
 *           table to find the line number chunk containing the line
 *           number entry closest to the address passed in.
 *
 * PARAMETERS: 
 *        addr - the address passed in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: lineaux
 *
 * RETURNS: an entry in the auxillary line number table corresponding
 *          to a chunk in the line number table.
 *
 */

static Lineaux * findchunk(addr)
Address addr;
{
    int low, high, mid;

    low = 0;
    high = lnnochunks - 1;

    /*  use a binary search to find out which chunk the address is in  */
    do {
      mid = (low + high) / 2;

      if (addr < lineaux[mid].lp->addr)
        high = mid - 1;
      else if (addr > lineaux[mid].lend->addr)
        low = mid + 1;
      else
        break;
    } while (low <= high);

    if (addr < lineaux[mid].lp->addr)
      mid--;
    return &lineaux[mid];
}

/*
 * NAME: find_linetab_entry
 *
 * FUNCTION: perform a binary search of the entries in the chunk   
 *           to find the line number closest to the address passed in.
 *
 * PARAMETERS: 
 *        chunkp - the auxilliary line number table entry pointing
 *                 to a chunk in the line number table.
 *        addr - the address passed in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: lineaux
 *
 * RETURNS: an entry in the auxillary line number table corresponding
 *          to a chunk in the line number table.
 *
 */

static Linetab * find_linetab_entry(lchunk, endlchunk, addr)
Linetab *lchunk;
Linetab *endlchunk;
Address addr;
{
    int low, high, mid;
    Address a;

    low = 0;

    /*  find the number of elements in this chunk  */
    high = endlchunk - lchunk;

    /*  binary search of chunk to find linetab entry */
    do
    {
      mid = (low + high) / 2;
      a = lchunk[mid].addr;
      if (a == addr) 
        break;
      if (addr > a) {
        low = mid + 1;
      } else {
        high = mid - 1;
      }
    } while (low <= high);

    if (addr < lchunk[mid].addr)
      mid--;
    return &lchunk[mid];
}

/*
 * NAME: srcline
 *
 * FUNCTION: lookup the source line nearest to an address.   
 *           If between line numbers, usually pick the one above
 *           the address, unless in a prolog.  This is handled in
 *           findline routine.
 *
 * PARAMETERS: 
 *        addr - the address passed in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: lineaux
 *
 * RETURNS: a line number
 *
 */

public Lineno srcline (addr)
Address addr;
{
    Linetab *lptr;

    lptr = findline(addr, PREVIOUS);

    if (lptr == NULL)
      return 0;
    else
      return lptr->line;
}

/*
 * NAME: linelookup
 *
 * FUNCTION: lookup the source line corresponding to an address.   
 *
 * PARAMETERS: 
 *        addr - the address passed in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: lineaux
 *
 * RETURNS: a line number or 0 if the address does not match a
 *          line number exactly.
 *
 */

public Lineno linelookup(addr)
Address addr;
{
    Linetab *lptr;

    lptr = findline(addr, EXACT);

    if (lptr == NULL)
      return 0;
    else
      return lptr->line;
}

/*
 * NAME: nextline
 *
 * FUNCTION: lookup the source line on or after an address.   
 *           return the address associated with the source line.
 *
 * PARAMETERS: 
 *        addr - the address passed in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: lineaux
 *
 * RETURNS: an address
 *
 */

public Address nextline(addr)
Address addr;
{
    Linetab *lptr;

    lptr = findline (addr, NEXT);

    if (lptr == NULL)
      return 0;
    else
      return lptr->addr;
}

/*
 * NAME: get_address_list
 *
 * FUNCTION: convert a line number in a file to a list of addresses.
 *
 * PARAMETERS: 
 *        line       - the line number in the file 
 *        name       - the name of the file
 *        event_id   - event id            
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: filetab
 *
 * RETURNS: address associated with the line number in this file,
 *            if it exists.  If none exists, NOADDR (-1) is returned.
 *
 */

public cppSymList get_address_list (line, name, event_id)
Lineno line;
String name;
int event_id;
{
    Boolean file_flag = false;
    Address line_addr = NOADDR;
    char *found_file = NULL;
    char *filename;
    cppSymList address_list = NULL;

    /*  see if we can find the file in the file table  */

    found_file = resolveFilename(name, line, &file_flag, 
                                 &line_addr, NULL, &address_list);

    /*  if the name is not found in the filetable 
          and the name contained a path  */
    if ((found_file == NULL) 
     && ((filename = rindex (name, '/')) != NULL))
    {
      /*  call resolveFilename with just the filename  */
      found_file = resolveFilename(filename + 1, line, &file_flag,
                                   &line_addr, NULL, &address_list);
    }

    /*  if the file was not compiled -g (as opposed to the file
          existing, but the line number not in range)  */
    if (!found_file)
    {
      if (!file_flag)
      {
          if (event_id != 0)
          {
            if (!delevent(event_id)) {
	      panic (catgets(scmc_catd, MS_eval, MSG_107,
	             "unknown event %ld"), event_id);
            }
          }
          error( catgets(scmc_catd, MS_mappings, MSG_111,
			     "source file \"%s\" not compiled with -g"), name);
      }
    }
    return (address_list);
}

/*
 * NAME: objaddr
 *
 * FUNCTION: convert a line number in a file to an address. 
 *
 * PARAMETERS: 
 *        line - the line number in the file 
 *        name - the name of the file
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: filetab
 *
 * RETURNS: address associated with the line number in this file,
 *            if it exists.  If none exists, NOADDR (-1) is returned.
 *
 */

public Address objaddr(line, name)
Lineno line;
String name;
{
    Address line_addr = NOADDR;
    cppSymList list;

    if (nlines_total == 0) {
	return NOADDR;
    }
    if (name == nil) {
	name = cursource;
    }

    list = get_address_list (line, name, 0);

    if (list != NULL)
      line_addr = list->line_addr;
    else
      line_addr = NOADDR;

    free_element_chain(list);

    return (line_addr);
}

/*
 * throw-away function, just need it to look at file table
 */
dumpFileTable()
{
  register Filetab *ftp;
  register Linechunk *chunkp;
  int ldndx;

  for (ldndx = 0; ldndx < loadcnt; ldndx++) {
    for (ftp = filetab[ldndx]; 
			ftp < &filetab[ldndx][nlhdr[ldndx].nfiles]; ftp++) {
      if (ftp) {
          if (ftp->lineptr) {
              (*rpt_output)(stdout, "%s:", ftp->filename);
              for (chunkp = ftp->lineptr; chunkp; chunkp = chunkp->next)
	          (*rpt_output)(stdout, " (%d, %d)", chunkp->lp->line,
							   chunkp->lend->line);
          (*rpt_output)( stdout, "\n");
          }
      }
    }
  }
}

/*
 * Table for going from object addresses to the functions in which they belong.
 */

#define NFUNCS 512	/* initial size of function table */

/*
 * Insert a new function into the table.
 */

public newfunc(f, addr, ft)
Symbol f;
Address addr;
Filetab *ft;
{
    register AddrOfFunc *af;
    AddrOfFunc *newfunctab;

    if (nfuncs >= functablesize) {
	if (functablesize == 0) {
	    functab = newarr(AddrOfFunc, NFUNCS);
	    functablesize = NFUNCS;
	} else {
	    functablesize *= 2;
	    newfunctab = newarr(AddrOfFunc, functablesize);
	    memcpy(newfunctab, functab, nfuncs * sizeof(AddrOfFunc));
	    dispose(functab);
	    functab = newfunctab;
	}
    }
    af = &functab[nfuncs];
    af->func = f;
    af->addr = addr;
    af->filep = ft;
    af->tbcontents[0] = TBINIT;
    af->exceptptr = exceptptr;
    ++nfuncs;
}

/*
 * NAME: untouch_file
 *
 * FUNCTION: make sure "untouched" is set after functions unloaded. 
 *
 * PARAMETERS: 
 *        s - the symbol for function being unloaded.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: filetab
 *
 * RETURNS: nothing
 *
 */

void untouch_file(s)
{
  Symbol file_sym;

  for (file_sym = s;
       (file_sym != nil) && (file_sym->class != MODULE);
        file_sym = file_sym->block)
    ;

  if (file_sym)
    file_sym->symvalue.funcv.untouched = true;
}

/*
 *  Delete functions from unloaded modules.
 *  Actually we move them to the end of the list after the sort by
 *  setting the addresses to 0xffffffff.
 */

public delete_unloaded_funcs (ldndx, prev_info)
int ldndx;
struct ldinfo prev_info[];
{
    
    Address text_base, text_end, data_base, data_end;
    Address funcaddr;
    int i;

    text_base = prev_info[ldndx].textorg;
    text_end = text_base + prev_info[ldndx].textsize;
    data_base = prev_info[ldndx].dataorg;
    data_end = data_base + prev_info[ldndx].datasize;

    searching = true;
    whatblock(text_base);
    searching = false;

    /* Delete functions from the text section, if any exist. */
    if (foundfunc) {
        i = foundfunc - &functab[0];
        for (; i < nfuncs; i++) {
	    funcaddr = functab[i].addr;
	    if (funcaddr == (Address) -1)
		continue;
	    if (funcaddr > text_end) 
	        break;
	    if (funcaddr >= text_base) {
                functab[i].addr = (Address) -1;
                untouch_file(functab[i].func);
            }
        }
    }

    searching = true;
    whatblock(data_base);
    searching = false;

    /* Delete functions from the data section, if any exist */
    if (foundfunc) {
        i = foundfunc - &functab[0];
        for (; i < nfuncs; i++) { /* Skip the program entry */
	    funcaddr = functab[i].addr;
	    if (funcaddr == (Address) -1)
		continue;
	    if (funcaddr > data_end) 
	        break;
	    if (funcaddr >= data_base) {
                functab[i].addr = (Address) -1;
                untouch_file(functab[i].func);
            }
        }
    }
}


/*
 * Return whether or not the frame information for a function has been 
 * found already, and if so, where?
 */

public Boolean checktables(addr, top, bottom, contents)
Address addr, **top, **bottom; 
unsigned int *contents[];
{
    AddrOfFunc *f;
    searching = true;
    whatblock(addr);
    searching = false;
    if (foundfunc == nil) {
	*top = nil;
        return false;
    } 
    *top = &foundfunc->tbsearch;
    *bottom = &foundfunc->tbfound;
    *contents = foundfunc->tbcontents;
    return (*contents[0] != (unsigned int) TBINIT);
}

public Boolean getfuncinfo(addr, start, end)
Address addr, *start, *end; 
{
    AddrOfFunc *nextfunc;

    searching = true;
    whatblock(addr);
    searching = false;
    if (foundfunc == nil) {
	*start = nil;
        return false;
    } 
    *start = foundfunc->addr;
    nextfunc = foundfunc+1;
    while( nextfunc->addr == foundfunc->addr ) {
	nextfunc++;
    }
    *end = nextfunc->addr - 4;
    return true;
}


#define checkentries(n) \
	if (functab[n].func->symvalue.funcv.isentry) skipentries(&n)
	
/*
 * When qsort was used to sort the functab, entries can be switched around
 * from their normal order if they have the same addr values (usually at end
 * of blocks). This function goes thru these entries of the same addresses
 * (assuming we are dealing with ending addresses, end of block) and
 * return the outer-most block.
 * (Note: Use this on C (unnamed blocks) only - Pascal and fortran program 
 *        can have main and the first (main) procedure having the same 
 *        entry address and we cannot tell which is wanted...)
 */
private integer getOuterBlock(i)
integer i;
{
   integer p = i;
   /* We only want to do this for C stuff... */
   if ((functab[i].func->language != cLang)
    && (functab[i].func->language != cppLang))
      return p;
   while (functab[p].addr == functab[i-1].addr) {
        if (functab[i-1].func->symvalue.funcv.beginaddr <
            functab[p].func->symvalue.funcv.beginaddr) {
           p = i-1;
        } else if (functab[i-1].func->symvalue.funcv.beginaddr ==
                   functab[p].func->symvalue.funcv.beginaddr) {
                  /* only a real function should have proladdr */
                  /* not unnamed blocks...                     */
                  if (functab[i-1].func->symvalue.funcv.proladdr)
                     return i-1;
        }
        i--;
   }
   return p;
}

/*
 * Return the function that begins at the given address.
 */

public Symbol whatblock(addr)
Address addr;
{
    int i, j, k;
    Address a;

    if (nfuncs == 0)	/* Possibly nonexecutable */
	return nil;
    if (nfuncs == 1) {	/* Completely stripped */
	i = 0;
    } else {
        i = 1;	/* array member 0 is program; no need to use in search */
    }
    j = nfuncs - 1;
    if (addr < functab[i].addr) {
	if (searching)
		foundfunc = nil;
	return program;
    } else if (addr == functab[i].addr) {
	checkentries(i);
	if (searching)
	    foundfunc = &functab[i];
	return functab[i].func;
    } else if (addr >= functab[j].addr) {
	checkentries(j);
	if (searching)
	    foundfunc = &functab[j];
	return functab[j].func;
    }
    while (i <= j) {
	k = (i + j) / 2;
	a = functab[k].addr;
	if (a == addr) { /* get the last one */
	    while ((k < nfuncs-1) && (addr == functab[k+1].addr))
	        ++k;
            k = getOuterBlock(k);
	    checkentries(k);
	    if (searching)
	        foundfunc = &functab[k];
	    return functab[k].func;
	} else if (addr > a) {
	    i = k+1;
	} else {
	    j = k-1;
	}
    }
    if (addr > functab[i].addr) {
        while (functab[i].addr == functab[i+1].addr) /* get the last one */
	    ++i;
        i = getOuterBlock(i);
	checkentries(i);
	if (searching)
	    foundfunc = &functab[i];
	return functab[i].func;
    } else {
        int z;
        while (functab[i].addr == functab[i-1].addr) /* get the first one */
	    --i;
	z = i-1;
        z = getOuterBlock(z);
	checkentries(z);
	if (searching)
	    foundfunc = &functab[z];
	return functab[z].func;
    }
    /* NOTREACHED */
}

/*
 * Order the functab.
 */

private int cmpfunc(f1, f2)
AddrOfFunc *f1, *f2;
{
	/*
	 * compare addresses...
	 */
	if (f1->addr < f2->addr)
		return -1;

	if (f1->addr > f2->addr)
		return 1;

	/*
	 * addresses were the same.
	 */

	/*
	 * compare who has source line info.
	 */
	if (f1->func->symvalue.funcv.src < f2->func->symvalue.funcv.src)
		return -1;

	if (f1->func->symvalue.funcv.src > f2->func->symvalue.funcv.src)
		return 1;

	/*
	 * source line info same.
	 */

	/*
	 * compare exception ptr info.
	 */
	if (f1->exceptptr < f2->exceptptr)
		return -1;
 
	if (f1->exceptptr > f2->exceptptr)
		return 1;

	/*
	 * exception ptr info same.
	 */

	/*
	 * compare stack level.
	 */
	if (f1->func->level < f2->func->level)
		return -1;

	if (f1->func->level > f2->func->level)
		return 1;

	return 0;
}

public ordfunctab()
{
    qsort(functab, nfuncs, sizeof(AddrOfFunc), cmpfunc);
}

/*
 * Order the auxiliary line table.
 */

private int cmplines(l1, l2)
Lineaux *l1, *l2;
{
    return ((l1->lp->addr < l2->lp->addr) ? -1 :
				((l1->lp->addr == l2->lp->addr) ? 0 : 1 ));
}

public ordlineaux(nl)
unsigned int nl;
{
    if (nl == 0)  return;
    qsort(lineaux, lnnochunks = nl, sizeof(Lineaux), cmplines);
    minlineaddr = lineaux[0].lp->addr;
    maxlineaddr = lineaux[nl-1].lend->addr;
}

/*
 * Clear out the functab, used when re-reading the object information.
 */

public clrfunctab()
{
    nfuncs = 0;
}

/*
 * Return the function entry actually associated with an ENTRY statement
 */

private skipentries(n)
int *n;
{
    int i = *n;
    for (i--;functab[i].func->symvalue.funcv.isentry;i--);
    *n = i;
}	

public dumpfunctab()
{
    int i;

    for (i = 0; i < nfuncs; i++) { 
	psym(functab[i].func);
    }
}

/*
 * textobj - Return the index of the object file a text address maps into.
 */
public int textobj(addr)
Address addr;
{
    int text_object;
    int ldcnt;
    struct ldinfo *ld_ptr;

    for (text_object = 0; text_object < loadcnt; text_object++) {
	ld_ptr = &loader_info[text_object];
	if ((addr >= ld_ptr->textorg) &&
	    (addr <= (ld_ptr->textorg + ld_ptr->textsize)))
		return text_object;
    }
#ifdef KDBX
    /*
     * dbx might not have all the loader information for all
     * the objects, we consider any address in segment 0 to
     * be a valid textobj that belongs to the /unix text object.
     */
    if (addr < 0x10000000)
	return 0;
#endif
    return -1;
}

/*
 * dataobj - Return the index of the object file a data address maps into.
 */
public int dataobj(addr)
Address addr;
{
    int data_object;
    int ldcnt;
    struct ldinfo *ld_ptr;

    for (data_object = 0; data_object < loadcnt; data_object++) {
	ld_ptr = &loader_info[data_object];
	if ((addr >= ld_ptr->dataorg) &&
	    (addr <= (ld_ptr->dataorg + ld_ptr->datasize)))
		return data_object;
    }
    return -1;
}

/*
 * addrtoobj - Return the index of the object file an address maps into.
 */
public int addrtoobj(addr)
Address addr;
{
    int object_index;

    object_index = textobj(addr);
    if (object_index == -1)
        object_index = dataobj(addr);
    return object_index;
}

/*
 * Given an include file and a line number entry, find the next include
 */
public getnextincl (curink, lp) 
Incfile **curink;
Linetab *lp;
{
    Linetab *nextlp;
    Incfile *inkt, *inkp; 	/* Include file traversers */

    /* Find child with lowest address > lp */
    /* If no such child, find parent or uncle with lowest address > lp */
    /* If no such parent or uncle, return nil */

    nextlp = lp+1;
    inkp = *curink;

    if (nextlp > inkp->lend) { /* must be sibling, parent, or nil */
	if ((inkp->incl_chain) && (inkp->incl_chain->lp == nextlp)) {
	    inkp = inkp->incl_chain;
	    checknestedincls(&inkp, inkp->lp);
	    *curink = inkp;
	    return;
	} else if (inkp->incl_parent) {
	    while ((inkp->incl_parent) &&
				(inkp->incl_parent->lend == inkp->lend)) {
		inkp = inkp->incl_parent;
		if ((inkp->incl_chain) && (inkp->incl_chain->lp == nextlp)) {
	    	    inkp = inkp->incl_chain;
	    	    checknestedincls(&inkp,inkp->lp);
		    *curink = inkp;
	    	    return;
	    	}
	    }
	    inkp = inkp->incl_parent;
	    if ((!inkp) || (nextlp > inkp->lend)) { /* No more. */
		*curink = nil;
	    } else {
		*curink = inkp;
	    }
	} else {  /* No siblings in range, and no incl_parent */
		if (inkp->incl_chain) {
		    *curink = inkp->incl_chain;
	    	    checknestedincls(curink,inkp->lp);
		} else {
		    *curink = nil;
		}
	}
    } else { /* can be child or parent */
	/* If next line is in range of siblings, then is in children. *
	 * Else is in parentage somewhere, or possibly nil. 	      */

	 if (inkp->incl_child) {
	     for (inkt = inkp->incl_child; 
		  (inkt) && (inkt->lend < nextlp);
		  inkt = inkt->incl_chain);
	     if (inkt) {	/* Found proper child tree. */		  
		  checknestedincls(&inkt, inkt->lp);
		  *curink = inkt;
		  return;
	     }
	 }
	 /* If we got here, then the next line is in the sibling 
	    or parent. */
	 nextlp = inkp->lend; 
	 /* Find the include file for the line after the last in this one */
	 getnextincl(&inkp, nextlp);
	 *curink = inkp;
    }
}

/*
 * Find for nested include which has the first executable statement.
 */
checknestedincls(curinkp, lp) 
Incfile **curinkp;
Linetab *lp;
{
    Incfile *inkp = *curinkp;

/* Check one level down to see if statement in an include, then call
   recursively to find deepest nested include with statement in range. */

    if ((inkp->incl_child) && (inkp->incl_child->lp <= lp)) {
	 inkp = inkp->incl_child;
	 while ((inkp) && (inkp->lend < lp)) {
	      inkp = inkp->incl_chain;
	 }
	 if ((inkp) && (inkp->lp <= lp)) {
	     *curinkp = inkp;
	     checknestedincls(curinkp,lp);
	 }
    }
}


/*
 * NAME: is_match  
 *
 * FUNCTION: Called by nametoincl and resolveFilename to determine
 *           if file1 is a potential match to what was passed in.
 *
 * PARAMETERS: 
 *       file1      - String filename as known in filetable   
 *       file2      - String filename as passed in to resolveFilename
 *       
 * NOTES:
 *     to better handle relative paths, we compare file2 as    
 *       passed in to the end of the file1
 *       to determine if this is a potential match. The offset
 *       variable is used to make sure what is compared is 
 *       actually a path, not a bisected filename or directory
 *       name.
 *
 *       For example, a program may include both include.h and 
 *       e/include.h.  In the filetable, they are stored as
 *       /u/test/testcase/include.h and /u/test/testcase/e/include.h.
 *       If a user specifies e/include.h, comparing at the end
 *       without considering path info would result in both
 *       being matched.  This is not what the user really wanted
 *       and would be annoyed at being prompted to pick between
 *       the two.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none
 *
 * RETURNS: true if base filenames match, false otherwise
 *
 */

Boolean is_match (file1, file2)
char *file1;
char *file2;
{
  int offset;
  /*  find the difference in the lengths of the filenames  */
  offset = strlen (file1) - strlen (file2);

  /*  if the filenames are the same length
        or comparing at the end will not result in a split
        directory or filename  */

  if ((offset == 0) ||
      ((offset > 0) && (file1[offset - 1] == '/')))
  {
    /*  if the filenames match - add to the list  */
    return (streq(file1 + offset, file2));
  }
  return (false);
}

/*
 * NAME: already_in_list
 *
 * FUNCTION: scan the list to see if the filename is already in it.
 *
 * PARAMETERS: 
 *       file       - String pointer to filename   
 *       list       - pointer to list of matching filenames
 *       
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: cppSymList
 *
 * RETURNS: the matching list element if filename is already in the list, 
 *          NULL, otherwise.
 *
 */

cppSymList already_in_list (file, list)
char *file;
cppSymList list;
{
   cppSymList listp;

   for (listp = list; listp != NULL; listp = listp->next)
   {
     if (streq (listp->filename, file))
       return (listp);
   }
   return (NULL);
}

/*
 * NAME: add_to_list
 *
 * FUNCTION: add a filename pointer to the linked list of pointers.
 *
 * PARAMETERS: 
 *       list             - pointer to list of matching filenames
 *       filename         - string pointer to filename
 *       line             - line number
 *       line_addr        - address corresponding to line number
 *       nmatches         - pointer to area counting number of
 *                          matches so far.
 *       
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: cppSymList
 *
 * RETURNS: nothing
 *
 */

add_to_list (list, filename, line, 
             line_addr, nmatches, basename_matches)
cppSymList *list;
char *filename;
Lineno line;
Address line_addr;
int *nmatches;
{
  cppSymList element_in_list;
 
  /*  if the line number is valid  */
  if ((line == 0) || (line_addr != NOADDR))
  {
    cppSymList list_element;

    list_element = new(cppSymList);
    list_element->filename = filename;
    list_element->file_list = true;
    list_element->line_addr = line_addr;
    list_element->chain = NULL;

    /*  if list_element is not already in the list  */
    if ((element_in_list = already_in_list(filename, *list)) == NULL)
    {
      /*  add list_element to beginning of list  */
      list_element->next = *list;
      *list = list_element;
      *nmatches = *nmatches + 1;
    }
    else
    {
      /*  add list_element to chain  */
      list_element->chain = element_in_list->chain;
      element_in_list->chain = list_element;
    }
  }
}

/*
 * NAME: nametoincl
 *
 * FUNCTION: Add pointer to include file to list of filenames if 
 *             fname matches a filename in this include file
 *             chain. 
 *
 * PARAMETERS: 
 *       inkp       - pointer to include file chain
 *       fname      - filename passed in 
 *       line       - line number
 *       list       - pointer to list of filenames
 *       file_found - pointer to flag to say if file found
 *       nmatches   - pointer to number of filename matches so far
 *       basename_matches - pointer to number of basename matches so far
 *                          (does not include those counted in nmatches)
 *       
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: filetab, cppSymList
 *
 * RETURNS: void
 *
 */

void nametoincl (inkp, fname, line, list, file_found, nmatches,
                 basename_matches)
Incfile *inkp;
String fname;
Lineno line;
cppSymList *list;
Boolean *file_found;
int *nmatches;
int *basename_matches;
{
     Boolean      nearestline = varIsSet("$inexact");
     Address      line_addr = NOADDR;
     int linediff = 0x7fffffff;

     /*  if the pointer passed in is NULL  */
     if (inkp == NULL)
        return;

     /*  if the filenames match  */
     if (is_match (inkp->filename, fname)) 
     {
       if (file_found != NULL) 
         *file_found = true;

       if (line != 0)
       {
         Linetab *end_linetable_ptr;
         Linetab *linetable_ptr;
         Address nl = NOADDR; 

         linetable_ptr = inkp->lp;
         end_linetable_ptr = inkp->lend;
         while (linetable_ptr <= end_linetable_ptr) {
           /* skip over any include file entries */
           if (inkp->incl_chain) {
             if (skipIncl(inkp->incl_chain, &linetable_ptr))
               continue;
           }
           if (linetable_ptr->line == line) {
             line_addr = linetable_ptr->addr;
             break;
           }
           if (nearestline) {
             if (linetable_ptr->line - line < linediff) {
               linediff = linetable_ptr->line - line;
               nl = linetable_ptr->addr;
             }
           }
           linetable_ptr++;
         } /* line number search */
         line_addr = (line_addr == NOADDR) ? nl : line_addr;
       }
       add_to_list (list, inkp->filename, line, line_addr, nmatches);
     }
     else if (is_match(inkp->filename, basefile(fname)))
     {
       *basename_matches = *basename_matches + 1;
     }

     nametoincl (inkp->incl_child, fname, line, list, file_found,
                 nmatches, basename_matches);
     nametoincl (inkp->incl_chain, fname, line, list, file_found,
                 nmatches, basename_matches);
     return;
}

/*
 * Skip over line entries for include file when finding source lines.
 * Since line number entries for include file are intermixed with
 * normal line entries. There could be more than one entry with the
 * the same line number within a chunk. Normally, dbx should ignore
 * entries from include when no filename is given and check only
 * entries from the current file.
 */
Boolean skipIncl(incl, i)
Incfile *incl;
Linetab **i;
{
  while (incl != nil) {
      if ((*i >= incl->lp) && (*i <= incl->lend)) {
         *i = incl->lend;
         (*i)++;
         return (1);
      }
      incl = incl->incl_chain;
  }
  return (0);
}


/*
 * Return include file corresponding to an address, if any.
 */

public Incfile *addrtoincl(inkp,addr)
Incfile *inkp;
Address addr;
{
     Incfile *inkt, *inkchild;

     /* Check to see if addr is in range of current incl or siblings. */
     /* If so, check if addr in range of subsequent children. */
     for (inkt = inkp;
	  (inkt) && ((inkt->lp->addr > addr) || (inkt->lend->addr < addr));
	  inkt = inkt->incl_chain);
     if (inkt == nil) {
	  return nil;
     } else {
         /* Check for strangeness such as end of 1 inc = beg of another */
	 if ((addr == inkt->lend->addr) &&
		(inkt->incl_chain) && (addr == inkt->incl_chain->lp->addr))
	     inkt = inkt->incl_chain;

         if (inkt->incl_child) {
	      if (inkchild = addrtoincl(inkt->incl_child,addr)) {
	          return inkchild;
	      } else {
	          return inkt;
	      }
         } else {
	      return (inkt);
         }
     }
}


char except_cat[13] = "e_msg000.cat";
nl_catd  except_catalog;
#define MS_except 1

/*
 * printExcept() - Look for a run-time exception message from the compiler.
 *		   If you find one, print it and return true,
 *		   else return false.
 */
public boolean printExcept()
{
    boolean rc = false;
    boolean found = false;
    enum { text, data } section;
    int i, j;
    int objndx;
    unsigned int reloc;
    unsigned int trap_code = 0;
    unsigned int lang_code = 0;
    char *m;
    LDFILE *ld_ptr;
    Symbol f;
    struct exceptab etab;
    SCNHDR ehdr, hdr;
    AOUTHDR opt_hdr;
    ARCHDR arhdr;
    String filename;

        /* find current function */
	searching = true;
	f = whatblock(pc);
	searching = false;
	if (foundfunc == nil || foundfunc->exceptptr == nil)
	    return false;

	/* get index of loaded object */
	objndx = textobj(pc);
	if (objndx == -1) {
            objndx = dataobj(pc);
	    if (objndx == -1)
		return false;
	    else
		section = data;
	}
	else
	    section = text;
	filename = (objndx == 0) ? objname : fd_info[objndx].pathname;

	/* open loaded object */
	if ((ld_ptr = ldopen(filename, ld_ptr)) == FAILURE) /* use pathname */
		return false; /* unable to open loaded module */

        /* if this is an archive, read down to the proper member */
        if ((fd_info[objndx].membername) &&
	    (fd_info[objndx].membername[0] != '\0')) { 
	    do {
	        if (ldahread(ld_ptr,&arhdr) == FAILURE)
	            fatal(catgets(scmc_catd, MS_object, MSG_167,
		        "cannot open member %s in file %s"),
			  fd_info[objndx].membername, filename);
	        if (streq(arhdr.ar_name,fd_info[objndx].membername))
		    break;
	        if (ldclose(ld_ptr) == SUCCESS)
	            fatal(catgets(scmc_catd, MS_object, MSG_167,
		       "cannot open member %s in file %s"),
			  fd_info[objndx].membername, filename);
	    } while (1);
        }

        /* determine relocation information */
	if (ldnshread(ld_ptr, _TEXT, &hdr) == SUCCESS) {
	    if (ldohseek(ld_ptr) != SUCCESS) {
		opt_hdr.data_start = 0;
		opt_hdr.text_start = hdr.s_scnptr;
	    }
	    else {
	        if (!FREAD((void *) &opt_hdr, (size_t) sizeof(opt_hdr),
							(size_t) 1, ld_ptr)) {
		    opt_hdr.data_start = 0;
		    opt_hdr.text_start = hdr.s_scnptr;
		}
	    }
	}
	else
	    return false;

	if (section == text)
	    reloc = loader_info[objndx].textorg - opt_hdr.text_start
		+ hdr.s_scnptr;
	else
	    reloc = loader_info[objndx].dataorg - opt_hdr.data_start;

	/* sanity check except pointer */
	if (ldnshread(ld_ptr, _EXCEPT, &ehdr) == SUCCESS &&
	    foundfunc->exceptptr >= ehdr.s_scnptr &&
	    foundfunc->exceptptr < ehdr.s_scnptr + ehdr.s_size)
	    ;	/* passed sanity check */
	else
	    return false;

	/* seek to exception table entry */
	if (FSEEK(ld_ptr, (long int) foundfunc->exceptptr, 0) != 0)
	    return false;

	/* read entries, trying to find a trap address matching pc */
	if (FREAD((void *) &etab, (size_t) EXCEPTSZ,
					      (size_t) 1, ld_ptr) != FAILURE) {
	    if (etab.e_reason == 0) {
		while (FREAD((void *) &etab, (size_t) EXCEPTSZ,
					      (size_t) 1, ld_ptr) != FAILURE) {
		    if (etab.e_reason == 0)
			break;
		    if (etab.e_addr.e_paddr == (pc-reloc)) {
			found = true;
			lang_code = etab.e_lang;
			trap_code = etab.e_reason;
			break;
		    }
		}
	    }
	}

	if (!found)
	    return false;	/* no trap address <-> pc match found */

	/* form message catalog file name */
	if (lang_code > 999)
	    lang_code = 0;
	for (i = 0, j = 100; i < 3; ++i, j /= 10) {
	    except_cat[5+i] = '0' + (lang_code / j);
	    lang_code = lang_code - (lang_code / j) * j;
	}

	/* get and display error message from catalog */
	except_catalog = catopen(except_cat, NL_CAT_LOCALE);
	m = catgets(except_catalog, MS_except,
					(int) trap_code, sys_siglist[SIGTRAP]);
	if (m) {
	    (*rpt_output)(stdout, m);
	    rc = true;		/* everything worked */
	}
	catclose(except_catalog);
	return rc;
}
