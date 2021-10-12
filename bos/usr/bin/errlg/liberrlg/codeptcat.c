static char sccsid[] = "@(#)40  1.4  src/bos/usr/bin/errlg/liberrlg/codeptcat.c, cmderrlg, bos411, 9428A410j 4/29/94 11:06:36";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: codeptcatinit, codeptcatstr, codeptpath, codeptcreate
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

/*
 * Look up a codepoint message from the error message catalog.
 * The catalog is not in Message Catalog Services format. MCS is
 *   very inefficient in its memory usage when the message sets are
 *   sparse, which is the case with the codepoint catalog.
 * The errinstall/errmsg command creates and add/deletes the codepoint.cat
 *   file.
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/err_rec.h>
#include <sys/erec.h>
#include <locale.h>
#include <codepoint.h>
#include <errlg.h>

/*
 * use malloc() instead of jalloc()
 */
#undef  MALLOC
#define MALLOC(n,s)   ( (s *)malloc((n) * sizeof(s)) )

extern char *malloc();

static cecmp();
int compare(const struct cp_entry *, const struct cp_entry *);
struct cp_entry * codept_bsearch(register, struct cp_entry *, int);


static FILE *Codeptfp;

static struct cp_hdr    C_hdr;		/* in-core file header */
static struct cp_entry *C_entry;	/* in-core cp_entry portion of file */
static char            *C_text;		/* in-core text portion of file */
static                  C_offset;	/* starting offset of C_text */
static                  C_count;	/* number of C_entry's */
char *Codeptfile;

struct bset {					/* one per set */
	struct cp_entry *b_start;	/* start of sorted cp_entry array */
	int              b_count;	/* number of elements of this array */
};
static struct bset C_sets[NERRSETS+1];	/* set is 1-based */

#define ISSET(set) (1 <= (set) && (set) <= NERRSETS)

/*
 * NAME:      codeptcatinit
 * FUNCTION:  initialize the codepoint.cat file
 * INPUTS:    catalog   name of the catalog
 * RETURNS:   negative value if fail.
 *
 * 1. Convert 'catalog' to a full pathname by calling codeptpath()'
 * 2. Open and stat the file
 * 3. Read the header into C_hdr and check for valid magic string.
 * 4. Read the cp_entry's   into the malloc-ed C_entry array.
 * 5. Read the text strings into the malloc-ed C_text character array.
 * 6. Convert the text offsets of the cp_entry's into in-core pointers.
 * 7. Sort the array according to set and alertid. The array C_sets
 *    contains the start of the array and number of elements for each set.
 *    This array is then used by codeptstr() to do binary searches.
 */

codeptcatinit()
{
	register i;
	int n;
	register currset;
	register struct cp_entry *cep;
	struct stat statbuf;

	codeptpath(0);		/* set Codeptfile */
	if((Codeptfp = fopen(Codeptfile,"r")) == 0 || stat(Codeptfile,&statbuf)) 
			return(-1);
	
	if(cread(&C_hdr,sizeof(C_hdr)) != sizeof(C_hdr) ||
	   strncmp(C_hdr.cp_magic,MAGIC,sizeof(C_hdr.cp_magic)) != 0) {
		fclose(Codeptfp);
		Codeptfp = 0;
		return(-2);
	}
	if((C_count = C_hdr.cp_count) == 0) 
		return(-2);
	
	C_entry = MALLOC(C_count,struct cp_entry);
	if(C_entry == 0)
		cat_fatal(CAT_CDPT_MALLOC,"Unable to initialize %s because there is not enough\n\
memory available.\n",__FILE__,errstr());
	n = C_count * sizeof(struct cp_entry);
	if(cread(C_entry,n) != n) 
		return(-2);
	
	C_offset = C_count*sizeof(struct cp_entry) + sizeof(C_hdr); /* read text */
	C_text   = MALLOC(statbuf.st_size - C_offset,char);
	if(C_text == 0)
		cat_fatal(CAT_CDPT_MALLOC,"Unable to initialize %s because there is not enough\n\
memory available.\n", __FILE__,errstr());
	n = statbuf.st_size - C_offset;
	if(cread(C_text,n) != n) 
		return(-1);
	
	for(i = 0; i < C_count; i++) {		/* convert to in-core pointers */
		cep = &C_entry[i];				/* check for bad set number */
		cep->ce_text = &C_text[cep->ce_offset-C_offset];
	}
	qsort(C_entry,C_count,sizeof(C_entry[0]),cecmp);
	currset = 0;
	for(i = 0; i < C_count; i++) {		/* fill in C_sets[] */
		cep = &C_entry[i];
		if(!ISSET(cep->ce_set)) 
			continue;
		
		if(cep->ce_set != currset) {
			currset = cep->ce_set;
			C_sets[currset].b_start = cep;
			for(; i <= C_count; i++) {		/* note <=, not < */
				cep = &C_entry[i];
				if(cep->ce_set != currset || i == C_count) {
					C_sets[currset].b_count = cep-C_sets[currset].b_start + 1;
					i--;					/* compensate for i++ */
					break;
				}
			}
		}
	}
	return(0);
}

/*
 * NAME:     cecmp
 * FUNCTION: compare 2 cp_entry's to order in set,alertid ascending order
 * INPUTS:   cep1,cep2   pointers to cp_entry's to compare
 * RETURNS:  -1  if cep1 < cep2
 *            0  if cep1 = cep2
 *           +1  if cep1 > cep2
 *
 * This routine is the qsort comparison routine for codeptcatinit().
 */
static cecmp(cep1,cep2)
struct cp_entry *cep1,*cep2;
{

	if(cep1->ce_set < cep2->ce_set)
		return(-1);
	if(cep1->ce_set > cep2->ce_set)
		return(+1);
	if(cep1->ce_alertid < cep2->ce_alertid)
		return(-1);
	if(cep1->ce_alertid > cep2->ce_alertid)
		return(+1);
	return(0);
}

/*
 * NAME:      codeptstr
 * FUNCTION:  lookup the message in /usr/lpp/msg/LANG/codepoint.cat
 * INPUTS:    'cdpt'   message id 
 *            'cset'   message set
 * RETURNS:   pointer to message string if successful
 *            pointer to sprintf-ed %04X buffer of the 'cdpt' if fail
 */

char *codeptstr(alertid,set)
register alertid;
{
	register n,ln,rn;
	register talertid;
	register struct cp_entry *c_entry;
	struct cp_entry *rcp;
	static char buf[8];

	if(alertid == 0xFFFF)
		return("");
	if(Codeptfp == 0)
		{
		sprintf(buf,"%04X",alertid);
		return(buf);
		}
	if(!ISSET(set))
		return("-");
	c_entry = C_sets[set].b_start;

	/* Call bsearch to search for the codepoint. */
	rcp = codept_bsearch(alertid, c_entry, C_sets[set].b_count);

	/* If we get null, then the codepoint was not found.  */
	/* So we will just return the codepoint, and print it. */
	/* Otherwise, return the codepoint text of the matching item. */
	if (rcp == NULL)	
		{
		sprintf(buf,"%04X",alertid);
		return(buf);
		}
	else
		{
		return(rcp->ce_text);
		}
}

/*
 *  Binary search routine for codeptstr().  This will search
 *  a certain set for a given codepoint value.
 *
 *  Inputs:
 * 		key :  This is the codepoint we are looking for.
 *		base : This is a pointer to the first cp_entry in the set we are searching.
 *		number:  This is the number of cp_entries in this set.
 *  Returns:
 *		NULL : No match found
 *	  	If there is a match, then a pointer to the item that matches is returned.	
 */

struct cp_entry * codept_bsearch(key, base, number)
register key;
struct cp_entry *base;
int number;
{
struct cp_entry *rcp;
struct cp_entry search_key;
	
	search_key.ce_alertid = key;

	rcp = bsearch(&search_key, base, number, sizeof(struct cp_entry),
			 (int (*)(const void *, const void *))compare);
	
	if (rcp == NULL) 
		return(NULL);
	else
		return(rcp);
}

/* 
 *  Compare routine for bsearch(). 
 *  	
 *	Inputs:
 *		id1 :	The codepoint we are searching for.
 *		id2 :   The codepoint we are checking for a match.
 *
 *	Returns:
 *		0 : match
 *		1 : id1 > id2
 *		2 : id2 > id1
 */

int compare(id1, id2)
const struct cp_entry  *id1;
const struct cp_entry *id2;
{

	if ((*id1).ce_alertid == (*id2).ce_alertid)
		return(0);

	if ((*id1).ce_alertid < (*id2).ce_alertid)
		return (-1);

	if ((*id1).ce_alertid > (*id2).ce_alertid)
		return (1);
}


/* 
 * NAME:      codeptpath
 * FUNCTION:  convert a catalog filename into a full NLS pathname.
 * INPUTS:    catalog   name of the catalog
 * RETURNS:   NLS pathnane of catalog.
 *            This pathname might not be a real file, so that open()
 *            can still fail if it does not exist.
 * 
 * This routine uses a search algorithm to find the catalog file:
 *  If 'catalog' is NULL, set catalog to "codepoint.cat"
 *  If 'catalog' contains a '/', override path searching and return 'catalog'
 *  Otherwise
 *    search NLSPATHs
 *    If the file is not found, use /usr/lib/ras/codepoint.cat
 */

codeptpath(catalog)
char *catalog;
{
	char *lang, *paths, *catpath, *ptr;
	struct stat sbuf;
	int	found;

	extern char *setlocale();
	extern char *getenv();

	if(catalog == 0 && Codeptfile)	/* already set */
		return;
	if(catalog) 					/* override */
		Codeptfile = stracpy(catalog);
	 else {						/* default */
		catalog = CODEPOINT_CAT;

		paths=getenv("NLSPATH");
		lang = setlocale(LC_MESSAGES,0);
		found = 0;
		if((strcmp(lang,"C")==0) || (strcmp(lang,"POSIX")==0))
			catpath = NULL;
		else
			catpath = strtok(paths,":");
		while (catpath != NULL) {	 /* for all paths */
 			Codeptfile = jalloc(strlen(catalog) + strlen(lang) + 
								strlen(catpath) + 1);
			if ((ptr = strstr(catpath,"%L")) == NULL) { 
				/* using prime directory so no fill in of lang */
				strcpy(Codeptfile,catpath);
				/* truncate off the %N */
				Codeptfile[strlen(catpath) - 3] = '\0';
			}
			else {
				/* copy upto %L, and concatenate language. */
				strncpy(Codeptfile,catpath,(size_t)ptr - (size_t)catpath);
				strcat(Codeptfile,lang);
			}
					
			strcat(Codeptfile,"/");
			strcat(Codeptfile,catalog);

			if (stat(Codeptfile,&sbuf) == 0)	{	/* found it */
				found = 1;
				break;
			}
			else
				jfree(Codeptfile);

			catpath = strtok((char*)0,":");

		}	/* end while */
		if (!found) {		/* use backup copy of codepoint.cat */
			Codeptfile = jalloc(sizeof(CODEPOINT_BAK)+1);
			strcpy(Codeptfile,CODEPOINT_BAK);
		}
	}
}

static cread(buffer,count0)
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

codeptcreate()
{
	char *cp;
	char cmd[128];
	char descfile[128];

	if(Codeptfile == 0)
		return(-1);
	codeptpath(0);
	strcpy(descfile,Codeptfile);
	if((cp = strrchr(descfile,'/')) == 0)
		return(-1);
	strcpy(cp+1,CODEPOINT_DESC);
	sprintf(cmd,"errinstall -q %s",descfile);
	if(shell(cmd))
		return(-1);
	return(codeptcatinit());
}
