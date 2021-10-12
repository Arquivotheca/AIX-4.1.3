static char sccsid[] = "@(#)20	1.48  src/bos/usr/ccs/lib/libc/catopen.c, libcmsg, bos411, 9428A410j 5/13/94 14:07:10";
/*
 *   COMPONENT_NAME: LIBCMSG
 *
 *   FUNCTIONS: _cat_do_open, _cat_hard_close, _cat_openfile, add_open_cat,
 *		cat_already_open, catclose, catopen, expand_catname, make_sets
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

#include <sys/errno.h>
#include <stdlib.h>
#include <sys/access.h>
#include <locale.h>
#include "catio.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _catalog_rmutex;
#endif /* _THREAD_SAFE */

static CATD *catdtbl[NL_MAXOPEN];	/* list of open catalogue pointers */

static int make_sets(struct _catset*, char*, int); /* create text tables */
static void add_open_cat(nl_catd);	/* remember which catalogue is open */
static nl_catd cat_already_open(char*);	/* find open catalogue */
static char *expand_catname(char*, int);	/* expand catname to full path */

/*
 * NAME: catopen 
 *                                                                    
 * FUNCTION: Set up a deferred open for a message catalog. If a catalog message
 *	is requested by catgets() or catgetmsg(), the partial open started here
 *	will be completed by _cat_do_open.  catgets() handles the problem of
 *	a child accessing a parent's catalogue.
 *
 * ARGUMENTS:
 *	catname		- name of message catalog to be opened
 *	oflag		- unused, but required by X/Open
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	catopen executes under a process.	
 *
 * RETURNS: Returns nl_catd, which is a pointer to a CATD
 *
 */  
 
nl_catd catopen (const char *catname, int oflag)
{ 
	int	errno_save;
	CATD	*catd;		/* returned catalogue descriptor */
	char	*lc_messages;	/* LC_MESSAGES true value */
	char	*full_catname;

	TS_LOCK(&_catalog_rmutex);
	errno_save = errno;

	TS_PUSH_CLNUP(&_catalog_rmutex);
	full_catname = expand_catname (catname, oflag);
	TS_POP_CLNUP(0);

	if (full_catname == NULL)
		RETURN(CATD_ERR);
/*
 * Return existing CATD if catalog already open
 * Just increment catopen() count
 */
	if ((catd = cat_already_open(full_catname)) != NULL) {
  		catd->_count++;
		RETURN(catd);
	}
/*
 * Allocate new CATD and add to catdtbl list - for future catopen()
 * of the same catalogue
 */
	if((catd=(CATD *)calloc(1, sizeof(CATD))) == NULL)
		RETURN(CATD_ERR);
	if((catd->_name=(char *)malloc(strlen(full_catname)+1)) == NULL) {
		free(catd);
		RETURN(CATD_ERR);
	}
	strcpy(catd->_name, full_catname);
	catd->_pid = getpid();
	catd->_count = 1;
	catd->_oflag = oflag;
	add_open_cat(catd);
	RETURN(catd);
}

 

/*
 * 
 * NAME: _cat_do_open
 *                                                                    
 * FUNCTION: Opens a catalog file, reads in and builds an index table.
 *
 * ARGUMENTS:
 *	catd		- catalog descripter obtained from catopen
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	_cat_do_open executes under a process.	
 *
 * RETURNS: Returns 1 if catalogue opened successfully
 *	If any error, returns 0
 *
 */  

int _cat_do_open(nl_catd catd)
{
	long int magic;		/* message catalog magic number */
	int i;			/*---- Misc counter(s) used for loop */
	struct _catset cs;

	catd->_fd = _cat_openfile(catd->_name);
	if (catd->_fd == FILE_UNUSED)
		return (0);
	fread((void *)&magic,(size_t)4,(size_t)1,catd->_fd);
	if (magic != CAT_MAGIC)
		goto error1;

#ifdef _FUTURE_MAPPED
/*	if ((catd->_mem = shmat((int)fileno(catd->_fd), (char *)0, SHM_MAP | SHM_RDONLY))
           == (char * )-1 ) {  */

	if (1) {      /* disable the shmat, share memory segemnt */
/*______________________________________________________________________
	If the file can not be mapped then simulate mapping for the index
	table so that make_sets cat set things up. (malloc an area big
 	enough for the index table and read the whole thing in)
  ______________________________________________________________________*/
#endif /* _FUTURE_MAPPED */

		/* reset the file pointer to the beginning of catalog */
		fseek(catd->_fd,(long)0,0);

		/* malloc the header, if fails return error */
		if((catd->_hd=(struct _header *)malloc(sizeof(struct _header))) == NULL)
			goto error1;

		/* read in the whole header */
		if (fread((void *)catd->_hd, (size_t)sizeof(struct _header), (size_t)1, catd->_fd) != 1)
			goto error2;

		/* cs is a dummpy to hold a set temperorily. The purpose of */
		/* this for loop is to fread the whole catalog so that the  */
		/* file pointer will be moved to the end of the catalog.    */
		for (i = 0 ; i < catd->_hd->_n_sets ; i++) {
			if (fread((void *)&cs,(size_t)4,(size_t)1,catd->_fd) != 1)
				goto error2;
			fseek(catd->_fd, (long)(cs._n_msgs * sizeof(struct _msgptr)),1);
		}

		/* after the for loop, ftell returns the byte offset of the */
		/* end of the catalog relative to the begining of the file. */
		/* i.e. i contains the byte offset of the whole catalog.    */
		i = ftell(catd->_fd);

		/* malloc _mem as a temp pointer to hold the entire catalog. */
		if((catd->_mem = (char *)malloc(i)) == NULL)
			goto error2;

		/* reset the file pointer to the begining. */
		fseek(catd->_fd,(long)0,0);

		/* read in the whole catalog into _mem */
		if (fread((void *)catd->_mem,(size_t)i,(size_t)1,catd->_fd) != 1)
			goto error3;

		/* malloc one extra set more than the max. set number */        
		if((catd->_set=(struct _catset *)calloc(1, (catd->_hd->_setmax+1)*
                              sizeof (struct _catset))) == NULL)
			goto error3;
	
		/* save the max. set number in catd->_setmax */
		catd->_setmax = catd->_hd->_setmax;

		/* call make_set to malloc memory for every message */
		if(make_sets(catd->_set,catd->_mem,catd->_hd->_n_sets) == -1)
			goto error4;
		free(catd->_mem);
		catd->_mem = NULL;
		return(1);
#ifdef _FUTURE_MAPPED
	}
	else {

/*______________________________________________________________________
	Normal mapping has occurred, set a few things up and call make_sets
  ______________________________________________________________________*/

		catd->_hd =( struct _header * )( catd->_mem );
		catd->_setmax = catd->_hd->_setmax;
		if((catd->_set=(struct _catset *)malloc((catd->_hd->_setmax+1)*
                             sizeof (struct _catset))) == NULL)
			return (0);
		if(make_sets(catd->_set,catd->_mem,catd->_hd->_n_sets) == -1)
			return (0);
		return(1);
	}
#endif /* _FUTURE_MAPPED */

error4:
/*
 * big trouble - malloc() error while allocating tables
 * but don't know where - so free() what we can
 */

error3:
	free(catd->_mem);
	catd->_mem = NULL;

error2:
	free(catd->_hd);
	catd->_hd = NULL;

error1:
	fclose(catd->_fd);
	return (0);
}



/*
 * 
 * NAME: make_sets
 *
 * FUNCTION: Expands the compacted version of the catalog index table into
 *	the fast access memory version.
 *
 * ARGUMENTS:
 *	cset		- message set to convert
 *	base		- message text storage
 *	n_sets		- number of sets in _header table
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Make_set executes under a process.	
 *
 * RETURNS: int
 */


int make_sets(struct _catset *cset, char *base, int n_sets)
{
	int 	i;	/*---- Misc counter(s) used for loops ----*/
	int	j;	/*---- Misc counter(s) used for loops ----*/
	int 	msgmax;	/*---- The maximum number of _messages in a set ----*/
	char 	*cmpct_set_ptr;	/*---- pointer into the index table ----*/
	struct _catset	cs;	/*---- used to look at the sets in the table -*/

	cmpct_set_ptr = base + sizeof(struct _header);

	for (i = 0 ; i < n_sets ; i++) {
	    /* loop through each compacted set */

		cs = *(struct _catset *)cmpct_set_ptr;	
                /* set the _catset ptr to the base of the current 
                   compacted set.        */

		cs._mp = (struct _msgptr *)(cmpct_set_ptr +
                          2 * sizeof(unsigned short));
                          /* set the ms array ptr to the base of
			     compacted array of _msgptr's     */

		for (j = 0 ,msgmax = 0 ; j < cs._n_msgs ; j++) {
	            /* find the highest msgno in the set */

			if (cs._mp[j]._msgno > msgmax)
				msgmax = cs._mp[j]._msgno;
		}
		msgmax++;   /* allocate memory for the expanded 
			       array (this one will have holes) */

		if((cset[cs._setno]._mp = (struct _msgptr *) calloc(1, msgmax *
                                       sizeof(struct _msgptr))) == NULL) 
			return (-1);
		if((cset[cs._setno]._msgtxt = (char **)calloc(1, msgmax *
                                           sizeof (char **))) == NULL)
			return (-1);

		for (j = 0 ; j < cs._n_msgs ; j++) {    
                     /* Fill the appropriate ones with data */

			cset[cs._setno]._mp[cs._mp[j]._msgno] = cs._mp[j];
		}
		cset[cs._setno]._n_msgs = msgmax - 1;	
		cset[cs._setno]._setno = cs._setno;
       	        /* Superfluous but should have the correct data. Increment 
                   the base of the set pointer.          */

		cmpct_set_ptr += 2 * sizeof(unsigned short) + cs._n_msgs *
                                 sizeof(struct _msgptr);
	}
	return (0);
}



/*
 * 
 * NAME: expand_catname
 *
 * FUNCTION: Expand the catalog name to a full path name using both
 *           the LANG and NLSPATH environment variables and check to
 *           see if the catalog is present.
 *                                                                    
 * ARGUMENTS:
 *	catname		- message catalog file name
 *      oflag		-
 *
 * NOTE: The C locale with NLSPATH=NULL will always return an error so
 *	force programs to use their default messages. NOTE: this is
 *	still not true as some programs still install default messages
 *	in the C message directory.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	executes under a process.	
 *
 * RETURNS:  Returns a character pointer to the catalog name, a NULL
 *           if the name does not exist.
 */

char *expand_catname (char *catname, int oflag)
{
	char 	fl[PATH_MAX];	/*---- place to hold full path ----*/
	char 	*nlspath;	/*---- pointer to the nlspath val ----*/
	FILE	*fp;		/*---- file pointer ----*/
	char	cpth[PATH_MAX]; /*---- current value of nlspath ----*/
	char    *p,*np;
	char	*fulllang;	/* %L language value */
	char	lang[PATH_MAX]; /* %l language value */
	char	*territory;	/* %t language value */
	char	*codeset;	/* %c language value */
	char	*ptr;		/* for decompose of $LANG */
	char	*lc__fastmsg;	/* LC__FASTMSG  value */
	char *str;
	char *optr;
	int nchars;
	int lenstr;
	char outptr[PATH_MAX];

	if (strchr(catname,'/')) {
                if (access (catname, R_OK) == 0) {
			strcpy (cpth, catname);	
			return (cpth);	
		}
		else
			return (NULL);	
	}

	if (oflag) {
   		fulllang = setlocale(LC_MESSAGES, NULL);
	}
	else 
		fulllang = getenv("LANG");

	if ((fulllang == NULL) || (fulllang[0] == 'C' &&        \
	     fulllang[1] == '\0') || (fulllang[0] == 'P' &&	\
	     !strcmp(fulllang, "POSIX"))) {
		lc__fastmsg = getenv("LC__FASTMSG"); 
		if (lc__fastmsg == NULL || !strcmp(lc__fastmsg, "true"))
			return (NULL);
	}

	if ((nlspath = getenv("NLSPATH")) == NULL || *nlspath == '\0')
		nlspath = PATH_FORMAT; 

	/*
	** LC_MESSAGES is a composite of three fields:
	** language_territory.codeset
	** and we're going to break it into those
	** three fields.
	*/

	strcpy(lang, fulllang);

	territory = "";
	codeset = "";

	ptr = (char*) strchr( lang, '_' );
	if (ptr != NULL) {
		territory = ptr+1;
		*ptr = '\0';
		ptr = (char*) strchr(territory, '.');
		if (ptr != NULL) {
			codeset = ptr+1;
			*ptr = '\0';
		}
	} else {
		ptr = (char*) strchr( lang, '.' );
		if (ptr != NULL) {
			codeset = ptr+1;
			*ptr = '\0';
		}
	}

	np = nlspath;
	while (*np) {
		p = cpth;
		while (*np && *np != ':')
			*p++ = *np++;
		*p = '\0';
		if (*np)	/*----  iff on a colon then advance --*/
			np++;
		if (strlen(cpth)) {
			ptr = cpth;
			optr = outptr;

			nchars = 0;
			while (*ptr != '\0') {
				while ((*ptr != '\0') && (*ptr != '%') 
					      && (nchars < PATH_MAX)) {
					*(optr++) = *(ptr++);
					nchars++;
				}
				if (*ptr == '%') {
					switch (*(++ptr)) {
						case '%':
							str = "%";
							break;
						case 'L':
							str = fulllang;
							break;
						case 'N':
		    					str = catname;
							break;
						case 'l':
							str = lang;
							break;
						case 't':
							str = territory;
							break;
						case 'c':
							str = codeset;
							break;
						default:
							str = "";
							break;
					}
					lenstr = strlen(str);
					nchars += lenstr;
					if (nchars < PATH_MAX) {
						strcpy(optr, str);
						optr += lenstr;
					} else {	
						break;
					} 
					ptr++;
				} else {
					if (nchars >= PATH_MAX) {
						break;
					}
				}
			}
			*optr = '\0';
			strcpy(cpth, outptr);
		}
		else {		/*----  iff leading | trailing | 
                                               adjacent colons ... --*/
			strcpy(cpth,catname);
		}
		if (access(cpth, F_OK) == 0) {
			return (cpth);
		}
	}
	return (NULL);
}

 
/*
 * 
 * NAME: _cat_openfile
 *
 * FUNCTION: Open the catalog file.
 *                                                                    
 * ARGUMENTS:
 *	catname	- message catalog file name
 *
 * NOTE: The C locale will always return an error so force programs to
 *	use their default messages. NOTE: this is still not true as
 *	some programs still install default messages in the C message
 *	directory.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	executes under a process.	
 *
 * RETURNS:  Returns a pointer to the file stream, or -1 on any error.
 */

FILE *_cat_openfile (char *catname)
{
	FILE *fp;

	if (fp = fopen (catname, "r")) {
		/* set the close-on-exec flag for child process */
		fcntl (fileno(fp), F_SETFD, 1);
		return (fp);
	}	
	return (FILE_UNUSED);
}

/*
 * 
 * NAME: cat_already_open
 *
 * FUNCTION: Check to see if a message catalog has already been opened.
 *
 * ARGUMENTS:
 *	catname		- name of message catalog to be opened
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	cat_already_open executes under a process.
 *
 * RETURNS: Returns a pointer to the existing CATD if one exists, and 
 *	a NULL pointer if no CATD exists.
 */

static nl_catd cat_already_open(char *catname)
{
	int	i;
	
	for (i = 0; i < NL_MAXOPEN; i++) {
		if (catdtbl[i] != NULL && strcmp(catname, catdtbl[i]->_name) == 0 && getpid() == catdtbl[i]->_pid)
		       	return (catdtbl[i]);
	}
	return(NULL);
}



/*
 * 
 * NAME: add_open_cat
 *
 * FUNCTION: Add a catlog to the list of already opened catalogs.
 *
 * ARGUMENTS:
 *	catd		- catalog descripter returned from catopen
 *
 * NOTE:
 *	If this catalogue descriptor cannot be added to catdtbl[],
 *	future catopen() of the same catalogue will result in a new
 *	catalogue descriptor.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 * 	add_open_cat executes under a process.	
 *
 * RETURNS: void
 */

static void add_open_cat(nl_catd catd)
{
	int	i;

	for (i = 0; i < NL_MAXOPEN && catdtbl[i] != NULL; i++)
		;
	if (i < NL_MAXOPEN)
		catdtbl[i] = catd;
	return;
}



/*
 * 
 * NAME: catclose
 *                                                                    
 * FUNCTION: Close a message catalog. 
 *
 * ARGUMENTS:
 *	catd		- catalog descripter returned from catopen
 *
 * EXECUTION ENVIRONMENT:
 *
 * 	catclose executes under a process.	
 *
 * NOTES: Catclose closes the stream and frees the memory of the catalog
 *	when "open" count is zero.  Otherwise the catalog file is left
 *	open for continued access.
 *
 * RETURNS: 0 on success, -1 on failure.
 *
 */  


int catclose(nl_catd catd)
{
	int	i;

	TS_LOCK(&_catalog_rmutex);

	/*
	 * return error if catd is invalid or catalogue already closed
	 */
	if (catd == NULL || catd->_count <= 0) {
		TS_UNLOCK(&_catalog_rmutex);
		return (-1);
	}

	/*
	 * return ok if catalogue never opened successfully (perhaps LANG=C)
	 */
	if (catd == CATD_ERR) {
		TS_UNLOCK(&_catalog_rmutex);
		return (0);
	}

	/*
	 * return ok if catalogue opened more than once
	 */
	if (--catd->_count > 0) {
		TS_UNLOCK(&_catalog_rmutex);
		return (0);
	}


	/*
	 * return ok after deleting catdtbl entry and closing catalogue
	 */
	for (i = 0; i < NL_MAXOPEN; i++)
		if (catdtbl[i] == catd) {
			catdtbl[i] = NULL;
			break;
		}
	TS_PUSH_CLNUP(&_catalog_rmutex);		
	_cat_hard_close(catd);
	TS_POP_CLNUP(0);		
	TS_UNLOCK(&_catalog_rmutex);
	return (0);
}


/*
 * NAME: _cat_hard_close
 *
 * FUNCTION: Closes a message catalogue and frees associated memory.
 *
 * ARGUMENTS:
 *	catd		- catalog descripter returned from catopen
 *
 * EXECUTION ENVIRONMENT:
 * 	_cat_hard_close executes under a process.	
 *
 * RETURNS: void
 */


void _cat_hard_close(nl_catd catd)
{
	int	i;
	int	j;

	/*
	 * only free/close if file descriptor was created
	 */
	if (catd->_fd != NULL && catd->_fd != FILE_UNUSED) {

		/*
	 	* free message offset table, set table, and message text
	 	*/
		for (i = 0; i <= catd->_setmax; i++) {
			if (catd->_set[i]._mp) 
				free(catd->_set[i]._mp);   
			if (catd->_set[i]._msgtxt) {
				for (j = 0; j <= catd->_set[i]._n_msgs; j++) {
					if (catd->_set[i]._msgtxt[j])
						free(catd->_set[i]._msgtxt[j]);
				}
				free(catd->_set[i]._msgtxt);
			}
		}
	}

	/*
	 * close valid file descriptor and set to FILE_UNUSED
	 */
	if (catd->_fd != NULL && catd->_fd != FILE_UNUSED && catd->_fd != FILE_DUMMY)
		fclose(catd->_fd);
	catd->_fd = FILE_UNUSED;

	/*
	 * free memory associated with catalogue descriptor
	 */
	if (catd->_set)
		free(catd->_set);
	if (catd->_hd)
		free(catd->_hd);
	free(catd->_name);
	free(catd);
	return;
}
