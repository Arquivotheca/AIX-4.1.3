static char sccsid[] = "@(#)23    1.5  src/bos/usr/bin/errlg/liberrlg/codeptutil.c, cmderrlg, bos411, 9428A410j  3/29/94  19:46:11";
/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: m_lookup, m_install, errm_codeptcatinit, display_set,
 *            iscodepoint, cread, cwrite, lookup, install, getmd,
 *            delete, listtoarray, mdcmp, next_mdinit, next_md
 *	      numchk, getcodepoint, fsettocode, fcodetoset	
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/err_rec.h>
#include <codepoint.h>
#include <errinstall.h>

extern FILE *Codeptfp;

char *fsettocode();
int fcodetoset();
int getcodepoint();
int numchk();
int cread();
int cwrite();
void c_delete();
void delete();
int errm_codeptcatinit();
void display_set();
void listtoarray();
void next_mdinit();
int iscodepoint();
struct md *getmd(), *c_lookup(), *c_install(), *next_md();
struct md *lookup(),  *install(), *m_lookup(), *m_install();

static displayflg = 0;
static createflg = 0; 
extern int Mdcount;
struct md *Mdarray;

static struct cp_hdr    C_hdr;          /* in-core file header */
static struct cp_entry *C_entry;        /* in-core cp_entry portion of file */
static char            *C_text;         /* in-core text portion of file */
static int              C_offset;       /* starting offset of C_text */

#define NHASH 31
#define HASHIDX(n) ((n) % NHASH)
#define ISSET(set) (1 <= (set) && (set) <= NERRSETS)
#define HASHHEAD(cflg,set,n) \
        cflg ? &c_mdtab[set][HASHIDX(n)] : &m_mdtab[set][HASHIDX(n)];

struct md *m_mdtab[NERRSETS+1][NHASH];   /* one per set, 1-based */
struct md *c_mdtab[NERRSETS+1][NHASH];   /* one per set, 1-based */

/*
 * NAME: 	cread()
 * FUNCTION:    Read the codepoint.cat header. 
 * RETURNS:     number of bytes read	
 */

int cread(buffer,count0)
char *buffer;
{
        char *cp;
        int c;
        int count;

        count = count0;
        cp = buffer;
        while(--count >= 0) {
                if((c = getc(Codeptfp)) == EOF)
                        break;
                *cp++ = c;
        }
        return(count0 - count - 1);
}

/*
 * NAME:	cwrite()
 * FUNCTION:    Write codepoint header.	
 * RETURN:	number bytes written
 */

int cwrite(buffer,count0)
char *buffer;
{
        char *cp;
        int c;
        int count;

        count = count0;
        cp = buffer;
        while(--count >= 0) {
                c = *cp++;
                if(putc(c,Codeptfp) == EOF)
                        break;
        }
        return(count0 - count - 1);
}

/*
 * NAME:	m_lookup()
 * FUNCTION:    Checks for valid set, and calls lookup() with 
 *		the cflg FALSE.
 * RETURNS:	pointer to md structure
 *		0	: codepoint not found
 */

struct md *m_lookup(set,codepoint)
{

        if(!ISSET(set)) {
                genexit(1);
                }
        return(lookup(0,set,codepoint));
}

/*
 * NAME:	m_install()
 * FUNCTION:	Calls install() and allocates space for codepoint text.
 * RETURNS:     pointer to md structure 
 */

struct md *m_install(set,codepoint,text)
char *text;
{
        struct md *mdp;

        mdp = install(0,set,codepoint);
        if(text) {
                mdp->md_text = jalloc(strlen(text)+1);
                strcpy(mdp->md_text,text);
        }
        return(mdp);
}

/*
 * NAME:	c_lookup()
 * FUNCTION:	Checks for valid set, and calls lookup() with
 *		cflg set to TRUE.
 * RETURNS:	pointer to md structure
 *		0	: invalid set or codepoint not found
 */

struct md *c_lookup(set,codepoint)
{

        if(!ISSET(set))
                return(0);
        return(lookup(1,set,codepoint));
}

/*
 * NAME:	c_install()
 * FUNCTION:	Calls install() assuming a preallocated text string.
 * RETURNS:     pointer to md structure
 */

struct md *c_install(set,codepoint,text)
char *text;
{
        struct md *mdp;

        mdp = install(1,set,codepoint);
        mdp->md_text = text;
        return(mdp);
}

/*
 * NAME:	c_delete()
 * FUNCTION:    Delete routine to delete codepoint from cdp table.
 * RETURNS:     None	
 */

void c_delete(set,codepoint)
{

        delete(1,set,codepoint);
}

/*
 * NAME:	lookup()
 * FUNCTION:    Searches the md/cd table for the given 
 *		codepoint.
 * RETURNS:     pointer to codepoint
 *		0 	: codepoint not found
 */

struct md *lookup(cflg,set,codepoint)
{
        struct md **mdpp;
        struct md *mdp;

        mdpp = HASHHEAD(cflg,set,codepoint);
        for(mdp = *mdpp; mdp; mdp = mdp->md_next)
                if(mdp->md_alertid == codepoint) {
                    return(mdp);
                }
        return(0);
}

/*
 * NAME:	install()
 * FUNCTION:    Initializes new md structure.
 * RETURNS:	pointer to the new md structure
 */

struct md *install(cflg,set,codepoint)
{
        struct md **mdpp;
        struct md *mdp;

        mdpp = HASHHEAD(cflg,set,codepoint);
        mdp = getmd();
        mdp->md_next    = *mdpp;
        *mdpp           = mdp;
        mdp->md_set     = set;
        mdp->md_alertid = codepoint;
        return(mdp);
}

/* 
 * NAME:	getmd()
 * FUNCTION:	Mallocs space for a md structure.
 * RETURNS:	pointer to the malloced space
 */

struct md *getmd()
{
        struct md *mdp;

        mdp = MALLOC(1,struct md);
        return(mdp);
}

/*
 * NAME:	delete()
 * FUNCTION:    Deletes a codepoint from the md/cdp table.
 * RETURNS:     None 
 */

void delete(cflg,set,codepoint)
{
        struct md **mdpp;
        struct md *mdpprev;
        struct md *mdp;

        mdpp = HASHHEAD(cflg,set,codepoint);
        mdpprev = 0;
        for(mdp = *mdpp; mdp; mdp = mdp->md_next) {
                if(mdp->md_alertid == codepoint) {
                        if(mdpprev)
                                mdpprev->md_next = mdp->md_next;
                        else
                                *mdpp = mdp->md_next;
                        continue;
                }
                mdpprev = mdp;
        }
}

/*
 * NAME:	errm_codeptcatinit()
 * FUNCTION:	Read in the codepoint catalog set n into cdp table.
 * RETURNS:     -1 	: failure
 *              -2 	: failure
 *		 0 	: success
 */

int errm_codeptcatinit()
{
        int i;
        int n;
        char *text;
	char *dirpath;
        struct cp_entry *cep;
        struct stat statbuf;

	/* If the codeptfile is not an absolute path name, then 
	   put a "./" in front of the filename.  This is required 
	   for dirname to work.  */

	if (strncmp(Codeptfile,"/",1) != 0) {
       	        dirpath = malloc(strlen(Codeptfile) + 3);
		strcpy(dirpath,"./");
		strcat(dirpath,Codeptfile);
		strcpy(Codeptfile,dirpath);
	}		

	/* If the codepoint.cat file does not exist, check to see if
	   the directory to the file exists.  If it does, create the
	   file.  If the directory does not exist, give an error.  */

	if ((stat(Codeptfile,&statbuf) != 0) &&      
	    (stat(dirname(Codeptfile),&statbuf) != 0)) 
	       	return (-1);

        if (((Codeptfp = fopen(Codeptfile,"r")) == 0) &&
	    (codeptcat_create())) 
                return(-1);

        if(cread(&C_hdr,sizeof(C_hdr)) != sizeof(C_hdr) ||
           strncmp(C_hdr.cp_magic,MAGIC,sizeof(C_hdr.cp_magic)) != 0) {
                fclose(Codeptfp);
                Codeptfp = 0;
                return(-2);
        }

	/* If the codepoint.cat file was just created, we don't
	   have any codepoint entries to process, so just return. */

	if (createflg)
		return (0);

        C_entry  = MALLOC(C_hdr.cp_count,struct cp_entry);
        n = C_hdr.cp_count * sizeof(struct cp_entry);
        if(cread(C_entry,n) != n) 
                return(-2);
        
        C_offset = C_hdr.cp_count * sizeof(struct cp_entry) + sizeof(C_hdr);
        C_text   = MALLOC(statbuf.st_size - C_offset,char);
        n = statbuf.st_size - C_offset;
        if(cread(C_text,n) != n) 
                return(-2);
       
        for(i = 0; i < C_hdr.cp_count; i++) {
                cep = &C_entry[i];
                if(c_lookup(cep->ce_set,cep->ce_alertid)) {
                        continue;
                }
                text = &C_text[cep->ce_offset-C_offset];
                c_install(cep->ce_set,cep->ce_alertid,text);
        }
        jfree(C_entry);
        return(0);
}

/* 
 * NAME:	display_set()
 * FUNCTION:	Prints the given set sorted by codepoint id.
 * RETURNS:	None
 */

void display_set(set)
{
        int i;
        static initflg;
        extern mdcmp();

	displayflg = 1;
        if(!initflg) {
                initflg++;
                if(errm_codeptcatinit() < 0) {
                        perror(Codeptfile);
                        exit(1);
                }
        }
        listtoarray(set);
        if(Mdcount > 0) {
                printf("SET %s\n",fsettocode(set));
                qsort(Mdarray,Mdcount,sizeof(struct md),mdcmp);
                for(i = 0; i < Mdcount; i++)
                        printf("%04X \"%s\"\n",Mdarray[i].md_alertid,Mdarray[i].md_text);
        }
}

/*
 * NAME:     listtoarray
 * FUNCTION: Convert the symbol table list to a continguous array, so that
 *           the qsort routine can use it.
 * INPUTS:   set number
 * RETURNS:  Number of elements in the array.
 *           The start of the array is filled in the global variable Mdset[].
 */

void listtoarray(set)
{
        struct md *mdp;
        int count;

        Mdcount = 0;
        next_mdinit(1,set);
        while(next_md())
                Mdcount++;
        if(Mdarray) {
                jfree(Mdarray);
                Mdarray = 0;
        }
        Mdarray = MALLOC(Mdcount,struct md);
        count = 0;
        next_mdinit(1,set);
        while(mdp = next_md())
                memcpy(&Mdarray[count++],mdp,sizeof(*mdp));
}

/*
 * NAME:	mdcmp()
 * FUNCTION:	md comparison routine for qsort
 * RETURNS:	-1 	: mdp1 < mdp2
 *		 0	: mdp1 = mdp2
 *		 1	: mdp1 > mdp2
 */

int mdcmp(mdp1,mdp2)
struct md *mdp1,*mdp2;
{

        if(mdp1->md_alertid < mdp2->md_alertid)
                return(-1);
        if(mdp1->md_alertid > mdp2->md_alertid)
                return(+1);
        return(0);
}

/*
 * NAME:	iscodepoint()
 * FUNCTION:	Checks to see if an entry exists for the given
 *		set and codepoint.
 * RETURNS:	1	: entry exists
 *		0	: entry does not exist 
 */

int iscodepoint(codepoint,set)
{
        return(c_lookup(set,codepoint) ? 1 : 0);
}


static nx_hidx;                         /* current hash bucket index */
static struct md  *nx_mdp;              /* current md structure */
static struct md **nx_mdpp;             /* start of hash buckets */

/*
 * NAME:	next_mdinit()
 * FUNCTION:	Initializes the next_md() routine for a 
 *		given set and table.
 * RETURNS:	None
 */

void next_mdinit(cflg,set)
{

        nx_hidx = -1;
        nx_mdp  = 0;
        nx_mdpp = cflg ? c_mdtab[set] : m_mdtab[set];
}

/*
 * NAME:	next_md()
 * FUNCTION:    Finds the next md structure for a set.
 * RETURNS:	next md structure for a set
 *		0 	: no more structures are left
 */

struct md *next_md()
{
        struct md *mdp;

loop:
        if(nx_mdp) {
                mdp = nx_mdp;
                nx_mdp = nx_mdp->md_next;
                return(mdp);
        }
        nx_hidx++;
        if(nx_hidx >= NHASH)
                return(0);
        nx_mdp = nx_mdpp[nx_hidx];
        goto loop;
}

/*
 * NAME:        numchk()
 * FUNCTION:    Given a string this function checks to see if it
 *              is a valid hex number.
 * RETURNS:     0       : valid hex number
 *              1       : not valid hex number
 */

int numchk(str)
char *str;
{
        int c;
        char *cp;

        cp = str;
        while(c = *cp++)
                if(!isxdigit(c))
                        return(0);
        return(1);
}

/*
 * NAME:        getcodepoint()
 * FUNCTION:    Allocates a codepoint, in the End User range, from the
 *              message catalog.
 * RETURNS:     -1        : failure
 *              codepoint : success
 */

int getcodepoint(set)
{
        int codepoint;

        for(codepoint = 0xE000; codepoint <= 0xE7FF; codepoint++) {
                if(iscodepoint(codepoint,set) ||
                                           m_lookup(set,codepoint))
                        continue;
                else
                        return(codepoint);
        }

        return(-1);
}

/*
 * NAME:        fcodetoset()
 * FUNCTION:    Given a set code, this function returns a set
 *              description.
 * RETURNS:     -1               : invalid set code
 *               set_description : set code found
 */

int fcodetoset(str)
char *str;
{
        int i;

        for(i = 0; i < NERRSETS; i++)
                if(streq(codetoset[i].nc_code,str))
                        return(codetoset[i].nc_set);
        return(-1);
}

/*
 * NAME:        fsettocode()
 * FUNCTION:    Given a set description, this function returns a set code.
 * RETURNS:     ?         : set description not found
 *              set code  : set description found
 */

char *fsettocode(set)
{
        int i;
        static char buf[4];

        strcpy(buf,"?");
        for(i = 0; i < NERRSETS; i++) {
                if(codetoset[i].nc_set == set) {
                        strcpy(buf,codetoset[i].nc_code);
                        break;
                }
        }
        return(buf);
}

/*
 * NAME:        codeptcat_create()
 * FUNCTION:    Create the codepoint.cat file is it doesn't exist. 
 * RETURNS:      0        : success
 *              -1        : failure 
 */

int codeptcat_create()
{
struct cp_hdr cp_hdr;

	/* Don't create the file if called with errmsg -w <set>.
	   It just displays the contents of the codepoint.cat
	   file.  In that case, an error will be given to the 
	   user that the file doesn't exist.  */

	if (displayflg)   
		return (-1);
	createflg = 1;
	if ((Codeptfp = fopen(Codeptfile,"w+")) == NULL) 
		return (-1);
	strncpy(cp_hdr.cp_magic,MAGIC,sizeof(cp_hdr.cp_magic));
	cp_hdr.cp_count = 0;
	cwrite(&cp_hdr,sizeof(cp_hdr));
	rewind(Codeptfp);
	return (0);
}

