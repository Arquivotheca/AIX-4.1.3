#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)95	1.19  src/bos/usr/ccs/bin/ld/bind/ifiles.c, cmdld, bos41B, 9505A 1/23/95 16:02:01")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:	free_segments
 *		ifile_close_all
 *		ifile_close_for_good
 *		ifile_close_one
 *		ifile_open_and_map
 *		ifile_reopen_remap
 *		init_ifiles
 *		reserve_new_ifiles
 *
 *		safe_fread
 *		safe_fseek
 *		fseek_read
 *
 * STATIC FUNCTIONS:
 *		do_lru
 *		free_mmapped_segment
 *		ifile_close
 *		ifile_map
 *		ifile_open
 *		ifile_unmap
 *		map_anon
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "global.h"
#include "bind.h"
#include "ifiles.h"
#include "error.h"
#include "objects.h"
#include "strs.h"
#include "stats.h"
#include "util.h"

#define NUMBER_SEGMENTS 16
#define SEGMENT(addr) (((ulong)addr)>>28)
/* Round up to Pagesize, but add a page if length is exact
   multiple of Pagesize */
#define MMAP_ROUND(l) (ROUND((l)+1,Pagesize))

/* Forward declarations */
static void ifile_unmap(IFILE *);
static RETCODE ifile_open (IFILE *);
static RETCODE ifile_map  (IFILE *);
static void    ifile_close(IFILE *);

/* Global variables */
IFILE *first_ifile /*= NULL*/;

/* Static variables */
#ifdef READ_FILE
static IFILE *read_files_root /* = NULL*/;
#endif
#ifdef I_ACCESS_MALLOC
static IFILE *malloc_files_root /*= NULL*/;
#endif
static IFILE *ifiles_last;

/* Define an initialized IFILE structure, to try to catch structure changes
   without associated initialization changes. */
#ifdef DEBUG
static IFILE init_IFILE = {
    (FILEDES)1,				/* i_fd */
    &NULL_STR,				/* i_name */
    (OBJECT *)NULL,			/* i_objects */
    INT_MAX,				/* i_ordinal */
    I_T_UNKNOWN,			/* i_type */
    I_ACCESS_SHMAT,			/* i_access */
    0,					/* i_rebind */
    0,					/* i_reinsert */
    1,					/* i_library */
    1,					/* i_keepfile */
    1,					/* i_closed */
    NULL,				/* i_map_addr */
    INT_MAX,				/* i_filesz */
    &init_IFILE, &init_IFILE		/* i_next, i_auxnext */
};
#endif

static int ifile_num /*= 0*/;		/* Count of file structures. */
static int next_allocation_ifiles /* = 0 */;
static int num_free_ifiles /* = 0 */;
static IFILE *free_ifiles /* = NULL */;

static int lru[(SHMHISEG-SHMLOSEG+1)+1]; /* LRU table--big enough for all
					    "shmat-able" segments plus a
					    spare. */
static int lru_ptr = -1;
static int doing_lru /*= 0*/;

static struct {
    IFILE *f;				/* IFILE shmat-ed into this segment, or
					   first IFILE in chain mmap-ed here.*/
    IFILE *wrap;			/* mmaped IFILE overflowing into
					   this segment. */
    int files_mapped;			/* If < 0, "f" is shmat-ed into this
					   segment.  If > 0, # of files mmaped
					   into segment (counting wrap) */
    int anon;				/* Number of anonymous mmaped areas
					   mapped in this segment */
} segment_info[NUMBER_SEGMENTS];

/* ***********************************
 * Name:	init_ifiles()
 *
 * Purpose:	Initialize data structures for IFILE routines
 * ***********************************/
void
init_ifiles(void)
{
    int i;

    /* Initialize segment information for segments that can possible be
       used for shmat() and mmap(). */
    i = SHMLOSEG;
    segment_info[i].f = NULL;
    segment_info[i].wrap = NULL;
    segment_info[i].files_mapped = 0;
    segment_info[i].anon = 0;
    for (++i; i <= SHMHISEG; i++)
	segment_info[i] = segment_info[SHMLOSEG];
}
/* ***********************************
 * Name:	reserve_new_ifiles()
 * Purpose:	Make sure next allocation of an IFILE allocates "count" structs
 * Given:	count
 * ***********************************/
void
reserve_new_ifiles(int count)
{
    next_allocation_ifiles += count;
}
/************************************
 * Name:	ifile_open_and_map()
 * Purpose:	Open and map a file given its name.  Fill in i_ordinal and
 *		link IFILE structure onto first_ifile chain.
 * Given:	Filename
 ************************************/
IFILE *
ifile_open_and_map(char *filename)	/* Could be escaped */
{
    char		*id = "ifile_open_and_map";
    int			count;
    static IFILE	*ifile = NULL;	/* For saving a structure allocated but
					   not returned (because of a file
					   error). */
    IFILE		*r;
    char		unescaped_filename[PATH_MAX+1];

    if (*filename == '\\') {
	filename = unescape_pathname(unescaped_filename, PATH_MAX, filename);
	if (filename == NULL)
	    goto name_too_long;
    }
    else if (strlen(filename) > PATH_MAX) {
      name_too_long:
	bind_err(SAY_NORMAL, RC_NI_SEVERE,
		 NLSMSG(FILE_NAMETOOLONG,
		"%1$s: 0711-878 SEVERE ERROR: The pathname is too long.\n"
		"\tThe maximum length of a pathname is %2$d.\n"
		"\tBinder command %3$s cannot be executed."),
		 Main_command_name, PATH_MAX, Command_name);
	return NULL;
    }

    if (ifile == NULL) {
	if (num_free_ifiles == 0) {
	    if (next_allocation_ifiles > 0)
		count = next_allocation_ifiles;
	    else
		count = 1;
	    free_ifiles = get_memory(sizeof(IFILE), count, IFILES_ID, id);
	    num_free_ifiles = count;
	    next_allocation_ifiles = 0;
	}
	ifile = free_ifiles++;
	--num_free_ifiles;
	STAT_use(IFILES_ID, 1);

	ifile->i_objects = NULL;
	ifile->i_ordinal = ++ifile_num;
	ifile->i_type = I_T_UNKNOWN;
	ifile->i_rebind =
	    ifile->i_reinsert =
		ifile->i_library =
		    ifile->i_keepfile = 0;
	ifile->i_next =
	    ifile->i_auxnext = NULL;
    }

    /* Initialize (or reinitialize) fields that could have changed if we're
       re-using a previously allocated ifile.  Other fields will still be
       initialized */
    ifile->i_access = Switches.input_opt;
    ifile->i_closed = 0;
    ifile->i_map_addr = NULL;

    ifile->i_fd = -1;
    ifile->i_name = putstring(filename); /* Save name */
    ifile->i_filesz = -1;

    /* The i_objects, i_ordinal, i_auxnext, and i_next fields aren't
       needed by ifile_open or ifile_map */
    if (ifile_open(ifile) != RC_OK)
	return NULL;

    if (ifile_map(ifile) != RC_OK) {
	(void) close(ifile->i_fd);
#ifdef STATS
	++Bind_state.close_total;
	--Bind_state.open_cur;
#endif
	return NULL;
    }

    if (first_ifile)
	ifiles_last->i_next = ifile;
    else
	first_ifile = ifile;
    ifiles_last = ifile;

    r = ifile;
    ifile = NULL;			/* Can't reuse FILE */
    return r;
} /* ifile_open_and_map */
/* ***********************************
 * Name:	do_lru(s)
 * Purpose:	Put segment for file s at head of lru list.
 * Given:	IFILE pointer
 * ***********************************/
static void
do_lru(IFILE *ifile)
{
    int seg, n, seg1, seg2;

    seg = SEGMENT(ifile->i_map_addr);

#ifdef DEBUG
    if (seg < SHMLOSEG || seg > SHMHISEG)
	internal_error();
    if (lru_ptr < -1 || lru_ptr > SHMHISEG-SHMLOSEG+1)
	internal_error();

    if (bind_debug & IFILES_DEBUG)
	if (seg != lru[0])
	    say(SAY_NO_NLS, "Additional access to file %s at address %x",
		ifile->i_name->name, ifile->i_map_addr);
#endif

    if (lru_ptr == -1)			/* List is empty */
	lru[++lru_ptr] = seg;
    else {
	seg1 = lru[0];			/* Save old head */
	lru[0] = seg;			/* Insert new head */
	lru[lru_ptr+1] = seg;		/* Sentinel */
	for (n = 1; seg != seg1; n++) { /* Shift others over */
	    seg2 = lru[n];
	    lru[n] = seg1;
	    seg1 = seg2;
	}
	if (n > lru_ptr + 1)		/* "seg" did not appear in lru, but has
					   been added.  Increment pointer. */
	    lru_ptr++;
    }
}
/* ***********************************
 * Name:	ifile_reopen_remap()
 * Purpose:
 * Given:	IFILE pointer
 * ***********************************/
RETCODE
ifile_reopen_remap(IFILE *s)
{
    int rc;

    if (s->i_map_addr) {
#ifdef NEW_IFILES
	/* File is already mapped or in memory */
	if (doing_lru && s->i_access == I_ACCESS_SHMAT)
	    do_lru(s);
#else

#ifdef I_ACCESS_MALLOC
	/* File is in memory */
	if (s->i_access == I_ACCESS_MALLOC)
	    return RC_OK;
#endif

	/* File already mapped */
	if (doing_lru)
	    do_lru(s);
#endif
	return RC_OK;
    }
    rc = ifile_open(s);
    if (rc == RC_OK) {
	rc = ifile_map(s);
#ifdef NEW_IFILES
	if (rc == RC_OK && doing_lru && s->i_access == I_ACCESS_SHMAT)
#else
#ifdef I_ACCESS_MALLOC
	if (rc == RC_OK && doing_lru && s->i_access != I_ACCESS_MALLOC)
#else
	if (rc == RC_OK && doing_lru)
#endif
#endif
	    do_lru(s);
    }
    return rc;
}
/* **********************************************
 * Name: 	ifile_close_one()
 * Purpose:	close some file.
 * Given:	IFILE pointer
 * *********************************************/
int
ifile_close_one(void)
{
    IFILE *f, *next_f;
    int i;

    if (first_ifile == NULL)
	return 0;			/* No open files */

#ifdef I_ACCESS_MALLOC
    /* Go through list of files with I_ACCESS_MALLOC */
    f = malloc_files_root;
    malloc_files_root = NULL;
    for (; f; f = next_f) {
	next_f = f->i_auxnext;
	if (f->i_fd == -1) {
	    if (f->i_map_addr == NULL)
		f->i_auxnext = NULL;
	    else if (malloc_files_root == NULL)
		malloc_files_root = f;
	}
	else {
	    ifile_close(f);
	    if (f->i_map_addr == NULL) {
		if (malloc_files_root == NULL)
		    malloc_files_root = next_f;
		f->i_auxnext = NULL;
	    }
	    return 1;
	}
    }
#endif

#ifdef READ_FILE
    /* Go through list of files with I_ACCESS_READ */
    for (f = read_files_root; f; f = next_f) {
	next_f = f->i_auxnext;
	if (f->i_fd == -1)
	    f->i_auxnext = NULL;
	else {
	    ifile_close(f);
	    read_files_root = next_f;
	    f->i_auxnext = NULL;
	    return 1;
	}
    }
    read_files_root = NULL;
#endif

    /* Check for real mmap-ed files */
    for (i = SHMLOSEG; i <= SHMHISEG; i++)
	if (segment_info[i].files_mapped > 0) {
	    f = segment_info[i].f;
	    segment_info[i].f = NULL;
	    for (; f; f = next_f) {
		next_f = f->i_auxnext;
		if (f->i_fd == -1) {
		    if (f->i_map_addr == NULL)
			f->i_auxnext = NULL;
		    else if (segment_info[i].f == NULL)
			segment_info[i].f = f;
		}
		else if (f->i_access == I_ACCESS_MMAP) {
		    ifile_close(f);
		    if (f->i_map_addr == NULL) {
			if (segment_info[i].f == NULL)
			    segment_info[i].f = next_f;
			f->i_auxnext = NULL;
		    }
		    else if (segment_info[i].f == NULL)
			segment_info[i].f = f;
		    return 1;
		}
		else if (segment_info[i].f == NULL)
		    segment_info[i].f = f;
	    }
	}

    /* Check for anonymous mmap-ed files */
    for (i = SHMLOSEG; i <= SHMHISEG; i++)
	if (segment_info[i].anon > 0) {
	    f = segment_info[i].f;
	    segment_info[i].f = NULL;
	    for (; f; f = next_f) {
		next_f = f->i_auxnext;
		if (f->i_fd == -1) {
		    if (f->i_map_addr == NULL)
			f->i_auxnext = NULL;
		    else if (segment_info[i].f == NULL)
			segment_info[i].f = f;
		}
		else if (f->i_access == I_ACCESS_ANONMMAP) {
		    ifile_close(f);
		    if (f->i_map_addr == NULL) {
			if (segment_info[i].f == NULL)
			    segment_info[i].f = next_f;
			f->i_auxnext = NULL;
		    }
		    else if (segment_info[i].f == NULL)
			segment_info[i].f = f;
		    return 1;
		}
		else if (segment_info[i].f == NULL)
		    segment_info[i].f = f;
	    }
	}

    /* Check for shmat-ed files */
    for (i = SHMLOSEG; i <= SHMHISEG; i++)
	if (segment_info[i].files_mapped < 0) {
	    f = segment_info[i].f;
	    ifile_unmap(f);
	    ifile_close(f);
	    return 1;
	}

    /* No files were closed */
    return 0;
} /* ifile_close_one */
/* **********************************************
 * Name: 	ifile_open()
 * Purpose:	Open the specified file.
 *		- Close other files first if too many files are open
 *		- Open the input file for reading.
 *		- update IFILE structure
 * Given:	IFILE pointer
 * *********************************************/
static RETCODE
ifile_open(IFILE *file)
{
    int	opened = 0;
    struct stat	sbuf;
    int fd;

    fd = file->i_fd;

    /* See if some file must be closed */
    while (file->i_fd == -1) {		/* File may already be open */
	file->i_fd = open(file->i_name->name, O_RDONLY);
	if (file->i_fd != -1) {
	    fd = file->i_fd;
	    opened = 1;
#ifdef STATS
	    ++Bind_state.open_cur;
	    ++Bind_state.open_total;
	    Bind_state.open_max = max(Bind_state.open_max,
				      Bind_state.open_cur);
#endif
#ifdef READ_FILE
	    if (file->i_access == I_ACCESS_READ) {
		if ((file->i_file = fdopen(file->i_fd, "r")) == NULL) {
		    close(fd);
#ifdef STATS
		    --Bind_state.open_cur;
#endif
		    file->i_fd = -1;
		    file->i_closed = 1;
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
			     Main_command_name, "fdopen", strerror(errno));
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(WHILE_PROCESSING,
	    "%1$s: 0711-996 Error occurred while processing file: %2$s"),
			     Main_command_name, file->i_name->name);
		    return RC_SEVERE;
		}
	    }
#endif
	}
	else if (errno == EMFILE || errno == ENFILE) {
	    /* Too many files open--close 1 */
	    if (ifile_close_one() == 0) {
		file->i_closed = 1;
		return RC_SEVERE;
	    }
	}
	else {
	    file->i_closed = 1;
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(FILE_CANNOT_OPEN,
		    "%1$s: 0711-160 SEVERE ERROR: Cannot open file: %2$s\n"
		    "\t%1$s:%3$s() %4$s"),
		     Main_command_name, file->i_name->name, "open",
		     strerror(errno));
	    return RC_NI_SEVERE;
	}
    }

    if (file->i_filesz < 0 || opened) {
	if (fstat(fd, &sbuf) < 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "fstat", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING,
	    "%1$s: 0711-996 Error occurred while processing file: %2$s"),
		     Main_command_name, file->i_name->name);
	    ifile_close_for_good(file);
	    return RC_SEVERE;
	}

	if (!(S_ISREG(sbuf.st_mode))) {
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(INPUT_BAD_FILE,
			    "%1$s: 0711-168 SEVERE ERROR: Input file: %2$s\n"
			    "\tInput files must be regular files."),
		     Main_command_name, file->i_name->name);
	    ifile_close_for_good(file);
	    return RC_NI_SEVERE;
	}

	if (file->i_filesz >= 0 && sbuf.st_size != file->i_filesz) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(IFILE_SIZE_ERROR,
			    "%1$s: 0711-166 SEVERE ERROR: Size of file %2$s\n"
			    "\thas changed since it was first opened."),
		     Main_command_name, file->i_name->name);
	    /* Allow processing to continue. */
	}

	/* Save size */
	file->i_filesz = sbuf.st_size;
    }

    return RC_OK;
} /* ifile_open */
/* **********************************************
 * Name: 	free_mmapped_segment
 * Purpose:	Free up segment 'seg' by munmapping all files in the segment.
 * Given:	IFILE pointer
 * *********************************************/
static void
free_mmapped_segment(int seg)
{
    IFILE	*f;

    /* Unmap any file beginning in a previous segment and continuing into
       segment 'seg'. */
    if (f = segment_info[seg].wrap)
	ifile_unmap(f);

    for (f = segment_info[seg].f; f; f = f->i_auxnext) {
	/* We don't take mmapped files off chain when
	   they are unmapped, so they may already be unmapped. */
	if (f->i_map_addr == NULL)
	    continue;
	ifile_unmap(f);
    }
    if (segment_info[seg].files_mapped != 0 || segment_info[seg].anon != 0)
	internal_error();
    segment_info[seg].f = NULL;		/* Clear chain */
}
/* **********************************************
 * Name: 	free_segments
 * Purpose:	Free up "count" consecutive segments
 * Given:	count
 * Returns:	Return 1 if successful; 0 if unsuccessful
 * *********************************************/
int
free_segments(int count)
#ifdef NEW_IFILES
{
    int save_count = count;
    int j, low, high, num_segs, seg;

    switch(Bind_state.state & (STATE_RBD_DBG_CALLED | STATE_RESOLVE_CALLED)) {
      case STATE_RESOLVE_CALLED:
	if (!doing_lru) {
	    doing_lru = 1;
	    /* Initialize lru list with all shmatted files. */
	    for (seg = SHMLOSEG; seg <= SHMHISEG; ++seg) {
		/* Don't add anonymously-mapped segments to lru list */
		if (segment_info[seg].files_mapped < 0) {
		    lru[++lru_ptr] = seg;
		    /* Bump 'seg' by number of segments used by file */
		    seg += (-segment_info[seg].files_mapped) - 1;
		}
	    }
	}
	break;

      case STATE_RESOLVE_CALLED | STATE_RBD_DBG_CALLED:
	doing_lru = 0;
	break;
    }

    if (count == 1) {
	if (doing_lru) {
	    if (lru_ptr < 0)
		goto free_mmapped_files;
	    seg = lru[lru_ptr--];
	    if (shmdt(segment_info[seg].f->i_map_addr)) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
				"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "shmdt", strerror(errno));
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
			 Main_command_name,
			 segment_info[seg].f->i_name->name,
			 segment_info[seg].f->i_map_addr);
	    }
	    DEBUG_MSG(IFILES_DEBUG,
		      (SAY_NO_NLS, "Detached file %s at address %x",
		       segment_info[seg].f->i_name->name,
		       segment_info[seg].f->i_map_addr));
#ifdef STATS
	    ++Bind_state.shmdt_total;
	    --Bind_state.shmat_cur;
#endif
	    segment_info[seg].f->i_map_addr = NULL;
	    for (num_segs = -segment_info[seg].files_mapped;
		 num_segs;
		 num_segs--, seg++) {
		segment_info[seg].files_mapped = 0;
		segment_info[seg].f = NULL;
	    }
	    return 1;
	}
	else {
	    /* Try to free a shmatted segment */
	    for (seg = SHMLOSEG; seg <= SHMHISEG; ++seg) {
		if (segment_info[seg].files_mapped < 0) {
		    if (shmdt(segment_info[seg].f->i_map_addr)) {
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
					"\t%1$s:%2$s() %3$s"),
				 Main_command_name, "shmdt", strerror(errno));
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
				 Main_command_name,
				 segment_info[seg].f->i_name->name,
				 segment_info[seg].f->i_map_addr);
		    }
		    DEBUG_MSG(IFILES_DEBUG,
			      (SAY_NO_NLS, "Detached file %s at address %x",
			       segment_info[seg].f->i_name->name,
			       segment_info[seg].f->i_map_addr));
#ifdef STATS
		    ++Bind_state.shmdt_total;
		    --Bind_state.shmat_cur;
#endif
		    segment_info[seg].f->i_map_addr = NULL;
		    for (j = -segment_info[seg].files_mapped; j>0; j--,seg--) {
			segment_info[seg].files_mapped = 0;
			segment_info[seg].f = NULL;
		    }
		    return 1;
		}
	    }

	  free_mmapped_files:
	    /* Try to free a segment containing mmapped files (but no
	       anonymously-mmapped files). */
	    for (seg = SHMLOSEG; seg <= SHMHISEG; ++seg) {
		if (segment_info[seg].files_mapped > 0
		    && segment_info[seg].anon == 0) {
		    free_mmapped_segment(seg);
		    return 1;
		}
	    }
	    /* Free segments containing anonymously-mmapped files. */
	    for (seg = SHMLOSEG; seg <= SHMHISEG; ++seg) {
		if (segment_info[seg].files_mapped > 0
		    || segment_info[seg].anon > 0) {
		    free_mmapped_segment(seg);
		    return 1;
		}
	    }
	    return 0;
	}
    }
    else {				/* count != 1 */
	internal_error();		/* Ignore for now */
#if 0
	if (doing_lru) {
	}
	else {
	    for (low = SHMLOSEG; low <= SHMHISEG; low++) {
		if (segment_info[low].files_mapped == 0)
		    continue;

		for (high = low + 1;
		     high <= SHMHISEG && high - low < count;
		     ++high) {
		    if (segment_info[high].files_mapped == 0)
			goto next_low;
		}
		if (high > SHMHISEG)
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(IFILE_CANNOT_FREE_SEG,
		    "%s: 0711-165 SEVERE ERROR: Cannot free memory segments."),
			     Main_command_name, save_count, Command_name);
		/* Now we know that low..high are segments that can be freed.
		   If "low" is mmaped, and high+1 is shmat-ed,
		   move our range up. */
		while (high < SHMHISEG
		       && segment_info[high].files_mapped < 0
		       && segment_info[low].files_mapped > 0) {
		    low++;
		    high++;
		}

		for (seg = low; seg <= high; seg++) {
		    if (segment_info[seg].files_mapped < 0) {
			if (shmdt(segment_info[seg].f->i_map_addr)) {
			    bind_err(SAY_NORMAL, RC_SEVERE,
				     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
				     Main_command_name, "shmdt",
				     strerror(errno));
			    bind_err(SAY_NORMAL, RC_SEVERE,
				     NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
				     Main_command_name,
				     segment_info[seg].f->i_name->name,
				     segment_info[seg].f->i_map_addr);
			}
			DEBUG_MSG(IFILES_DEBUG,
				  (SAY_NO_NLS, "Detached file %s at address %x",
				   segment_info[seg].f->i_name->name,
				   segment_info[seg].f->i_map_addr));
#ifdef STATS
			++Bind_state.shmdt_total;
			--Bind_state.shmat_cur;
#endif
			segment_info[seg].f->i_map_addr = NULL;
			for (num_segs = -segment_info[seg].files_mapped;
			     num_segs;
			     count--, num_segs--, seg++) {
			    segment_info[seg].files_mapped = 0;
			    segment_info[seg].f = NULL;
			}
		    }
		    else {
			free_mmapped_segment(seg);
			count--;
		    }
		}
	      next_low:
		;
	    }
	}
#endif
    }
} /* free_segments */
#else
{
    int i, low, high, num_segs, seg;

    if ((Bind_state.state & (STATE_RBD_DBG_CALLED | STATE_RESOLVE_CALLED))
	== STATE_RESOLVE_CALLED
	&& !doing_lru) {
	doing_lru = 1;
	/* Initialize lru list */
	for (seg = SHMLOSEG; seg <= SHMHISEG; seg++) {
	    if (segment_info[seg].files_mapped != 0
		|| segment_info[seg].anon > 0) {
		lru[++lru_ptr] = seg;
		/* See if file takes more than one segment. */
		if (segment_info[seg].files_mapped < -1)
		    seg += (-segment_info[seg].files_mapped - 1);
	    }
	}
    }

  again:
    if (doing_lru) {
	if (Bind_state.state & STATE_RBD_DBG_CALLED) {
	    for (seg = SHMLOSEG; seg <= SHMHISEG; seg++) {
		if (segment_info[seg].files_mapped < 0) {
		    if (shmdt(segment_info[seg].f->i_map_addr)) {
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
				 Main_command_name, "shmdt", strerror(errno));
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
				 Main_command_name,
				 segment_info[seg].f->i_name->name,
				 segment_info[seg].f->i_map_addr);
		    }
		    DEBUG_MSG(IFILES_DEBUG,
			      (SAY_NO_NLS, "Detached file %s at address %x",
			       segment_info[seg].f->i_name->name,
			       segment_info[seg].f->i_map_addr));
#ifdef STATS
		    ++Bind_state.shmdt_total;
		    --Bind_state.shmat_cur;
#endif
		    segment_info[seg].f->i_map_addr = NULL;
		    for (num_segs = -segment_info[seg].files_mapped;
			 num_segs;
			 count--, num_segs--, seg++) {
			segment_info[seg].files_mapped = 0;
			segment_info[seg].f = NULL;
		    }
		}
	    }
	    doing_lru = 0;
	    if (count > 0)
		goto again;
	}
	while (count > 0) {
	    seg = lru[lru_ptr--];
	    if (segment_info[seg].files_mapped > 0
		|| segment_info[seg].anon > 0) {
		free_mmapped_segment(seg);
		count--;
	    }
	    else {
		if(shmdt(segment_info[seg].f->i_map_addr)) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
			     Main_command_name, "shmdt", strerror(errno));
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
			     Main_command_name,
			     segment_info[seg].f->i_name->name,
			     segment_info[seg].f->i_map_addr);
		}
		DEBUG_MSG(IFILES_DEBUG,
			  (SAY_NO_NLS, "Detached file %s at address %x",
			   segment_info[seg].f->i_name->name,
			   segment_info[seg].f->i_map_addr));
#ifdef STATS
		++Bind_state.shmdt_total;
		--Bind_state.shmat_cur;
#endif
		segment_info[seg].f->i_map_addr = NULL;
		for (num_segs = -segment_info[seg].files_mapped;
		     num_segs;
		     count--, num_segs--, seg++) {
		    segment_info[seg].files_mapped = 0;
		    segment_info[seg].f = NULL;
		}
	    }
	}
	if (count > 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(IFILE_CANNOT_FREE_SEG,
		    "%s: 0711-165 SEVERE ERROR: Cannot free memory segments."),
		     Main_command_name);
	    return 0;
	}
	else if (lru_ptr < 0)
	    lru[++lru_ptr] = 0;
	return 1;
    }
    else if (count == 1) {
	for (i = SHMLOSEG; i <= SHMHISEG; i++) {
	    if (segment_info[i].files_mapped < 0) {
		if (shmdt(segment_info[i].f->i_map_addr)) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
			     Main_command_name, "shmdt", strerror(errno));
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(WHILE_PROCESSING_ADDR,
 "%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
			     Main_command_name,
			     segment_info[i].f->i_name->name,
			     segment_info[i].f->i_map_addr);
		}
		DEBUG_MSG(IFILES_DEBUG,
			  (SAY_NO_NLS, "Detached file %s at address %x",
			   segment_info[i].f->i_name->name,
			   segment_info[i].f->i_map_addr));
#ifdef STATS
		++Bind_state.shmdt_total;
		--Bind_state.shmat_cur;
#endif
		segment_info[i].f->i_map_addr = NULL;
		for (num_segs = -segment_info[i].files_mapped;
		     num_segs;
		     --num_segs, ++i) {
		    segment_info[i].files_mapped = 0;
		    segment_info[i].f = NULL;
		}
		return 1;
	    }
	}
	for (i = SHMLOSEG; i <= SHMHISEG; i++) {
	    if (segment_info[i].files_mapped > 0 && segment_info[i].anon == 0) {
		/* Segment contains no anonymous mmaped files.
		   Unmap all of them. */
		free_mmapped_segment(i);
		return 1;
	    }
	}
	for (i = SHMLOSEG; i <= SHMHISEG; i++) {
	    if (segment_info[i].files_mapped > 0 || segment_info[i].anon > 0) {
		free_mmapped_segment(i);
		return 1;
	    }
	}
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(IFILE_CANNOT_FREE_SEG,
		"%s: 0711-165 SEVERE ERROR: Cannot free memory segments."),
		 Main_command_name);
	return 0;
    }
    else {
	for (low = SHMLOSEG; low <= SHMHISEG; low++) {
	    if (segment_info[low].files_mapped == 0)
		continue;

	    for (high = low + 1; high <= SHMHISEG && high-low < count; high++){
		if (segment_info[high].files_mapped == 0)
		    goto next_low;
	    }
	    if (high > SHMHISEG) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(IFILE_CANNOT_FREE_SEG,
		"%s: 0711-165 SEVERE ERROR: Cannot free memory segments."),
			 Main_command_name);
		return 0;
	    }
	    /* Now we know that low..high are segments that can be freed.  If
	       "low" is mmaped, and high+1 is shmat-ed, move our range up. */
	    while (high < SHMHISEG
		   && segment_info[high].files_mapped < 0
		   && segment_info[low].files_mapped > 0) {
		low++;
		high++;
	    }

	    for (i = low; i <= high; i++) {
		if (segment_info[i].files_mapped < 0) {
		    if (shmdt(segment_info[i].f->i_map_addr)) {
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
				 Main_command_name, "shmdt", strerror(errno));
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
				 Main_command_name,
				 segment_info[i].f->i_name->name,
				 segment_info[i].f->i_map_addr);
		    }
		    DEBUG_MSG(IFILES_DEBUG,
			      (SAY_NO_NLS, "Detached file %s at address %x",
			       segment_info[i].f->i_name->name,
			       segment_info[i].f->i_map_addr));
#ifdef STATS
		    ++Bind_state.shmdt_total;
		    --Bind_state.shmat_cur;
#endif
		    segment_info[i].f->i_map_addr = NULL;
		    for (num_segs = -segment_info[i].files_mapped;
			 num_segs;
			 count--, num_segs--, i++) {
			segment_info[i].files_mapped = 0;
			segment_info[i].f = NULL;
		    }
		}
		else {
		    free_mmapped_segment(i);
		    count--;
		}
	    }
	  next_low:
	    ;
	}
	return 1;
    }
} /* free_segments */
#endif
/* ***********************************
 * Name:	map_anon()
 * Purpose:	Given a file structure pointer, map a region of memory large
 *		enough to hold file and read file into memory.
 * ***********************************/
static RETCODE
map_anon(IFILE *file,
	 long len_file)
{
    int		seg;
    int		rc;
    ulong	file_end;
    FILEDES	fd;
    FILE	*ffile;

    /* Use options for anonymous memory. */
  retry_mmap:
    file->i_map_addr =
	mmap(NULL, len_file, PROT_WRITE, MAP_ANON | MAP_VARIABLE, -1, 0);

    if (file->i_map_addr == (caddr_t)-1) {
	if (errno == ENOMEM && free_segments(SEGMENT(len_file) + 1) != 0)
	    goto retry_mmap;
	else {
#ifdef STATS
	    ++Bind_state.close_total;
	    --Bind_state.open_cur;
#endif
	    file->i_fd = -1;
	    file->i_closed = 1;
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "mmap", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING,
	    "%1$s: 0711-996 Error occurred while processing file: %2$s"),
		     Main_command_name, file->i_name->name);
	    return RC_SEVERE;
	}
    }

    /* Read file into anonymous area. */
    fd = file->i_fd;
    if ((ffile = fdopen(fd, "r")) == NULL) {
	close(fd);
#ifdef STATS
	++Bind_state.close_total;
	--Bind_state.open_cur;
#endif
	file->i_fd = -1;
	file->i_closed = 1;
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
		 Main_command_name, "fdopen", strerror(errno));
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(WHILE_READING,
		"%1$s: 0711-987 Error occurred while reading file: %2$s"),
		 Main_command_name, file->i_name->name);
	return RC_SEVERE;
    }

    /* We can't use fseek_read() because the IFILE structure contains
       a file descriptor instead of a FILE pointer */
    if (file->i_filesz != 0) {
	/* Duplicate function of safe_fseek() */
	while (fseek(ffile, 0L, SEEK_SET) != 0) {
	    if (errno != EINTR) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "fseek", strerror(errno));
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_PROCESSING,
		"%1$s: 0711-996 Error occurred while processing file: %2$s"),
			 Main_command_name, file->i_name->name);
		goto munmap_area;
	    }
	}
	while ((rc = fread(file->i_map_addr, file->i_filesz, 1, ffile)) != 1) {
	    if (rc == 0) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(READ_EOF,
		"%1$s: 0711-983 SEVERE ERROR: File %2$s\n"
		"\tPremature end-of-file reached while reading %3$d bytes."),
			 Main_command_name,
			 file->i_name->name, file->i_filesz);
		goto munmap_area;
	    }
	    if (errno != EINTR) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "fread", strerror(errno));
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_READING_LEN,
 "%1$s: 0711-986 Error occurred while reading %2$d bytes from file: %3$s"),
			 Main_command_name,
			 file->i_filesz, file->i_name->name);
	      munmap_area:
		if (munmap(file->i_map_addr, len_file) != 0) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
			     Main_command_name, "munmap", strerror(errno));
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
			     Main_command_name, file->i_name->name,
			     file->i_map_addr);
		}
		fclose(ffile);
		file->i_map_addr = NULL;
		file->i_closed = 1;
		file->i_fd = -1;
#ifdef STATS
		++Bind_state.close_total;
		--Bind_state.open_cur;
#endif
		return RC_SEVERE;
	    }
	}
    }

#ifdef STATS
    ++Bind_state.mmap_total;
    ++Bind_state.mmap_cur;
    Bind_state.mmap_max = max(Bind_state.mmap_max, Bind_state.mmap_cur);
#endif

    if (mprotect(file->i_map_addr, len_file, PROT_READ) != 0) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
		 Main_command_name, "mprotect", strerror(errno));
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
		 Main_command_name, file->i_name->name, file->i_map_addr);
    }

    file->i_access = I_ACCESS_ANONMMAP;
    seg = SEGMENT(file->i_map_addr);
    file->i_auxnext = segment_info[seg].f;
    segment_info[seg].f = file;

    segment_info[seg].anon++;
    for (file_end = ((ulong)file->i_map_addr & (SHMLBA-1)) + len_file;
	 file_end > SHMLBA;
	 seg++, file_end -= SHMLBA) {
	segment_info[seg].wrap = file;
	segment_info[seg].anon++;
    }

    DEBUG_MSG(IFILES_DEBUG,
	      (SAY_NO_NLS,
	       "Read non-mapped file %s into anonymous area at %x.",
	       file->i_name->name, file->i_map_addr));
    return RC_OK;
}
/* ***********************************
 * Name:	ifile_map()
 * Purpose:	Given a file structure pointer,
 *		issue mmap() to map the file, or read it into memory.
 * ***********************************/
static RETCODE
ifile_map(IFILE *file)
{
    static char	*id = "ifile_map";
    size_t	len_file;
    int		i;
    int		num_segs;
    int		seg;
    size_t	size;
    ssize_t	s;
    caddr_t	base;
    ulong	file_end;

    /* Is the file already mapped? */
    if (file->i_map_addr)
	return RC_OK;

    /* File must be open */
    switch(file->i_access) {
      case I_ACCESS_SHMAT:
#ifdef READ_FILE
      case I_ACCESS_SHMAT_READ:
#endif
	len_file = ROUND(file->i_filesz, Pagesize);

      retry_shmat:
	file->i_map_addr = shmat(file->i_fd, NULL, SHM_MAP | SHM_RDONLY);
	if (file->i_map_addr == (caddr_t)-1) {
	    if (errno == EMFILE && free_segments(SEGMENT(len_file) + 1) != 0)
		goto retry_shmat;	/* Try again */
	    else {
		/* shmat failed.  The returned errno is not always accurate,
		   so rather than see what the problem is, we just read or
		   map the file anonymously. */
#ifdef READ_FILE
		if (file->i_access == I_ACCESS_SHMAT_READ) {
		    file->i_access = I_ACCESS_READ;
		    goto get_read_access;
		}
		else
#endif
		    map_anon(file, len_file);
		break;
	    }
	}
#ifdef READ_FILE
	file->i_access = I_ACCESS_SHMAT; /* In case access was SHMAT_READ */
#endif
	seg = SEGMENT(file->i_map_addr);
	for (i = num_segs = SEGMENT(file->i_filesz) + 1; i > 0; i--) {
	    segment_info[seg].f = file;
	    segment_info[seg].files_mapped = -num_segs;
	}
#ifdef STATS
	++Bind_state.shmat_total;
	++Bind_state.shmat_cur;
	Bind_state.shmat_max = max(Bind_state.shmat_max, Bind_state.shmat_cur);
#endif
	DEBUG_MSG(IFILES_DEBUG, (SAY_NO_NLS, "Shmat-ed file %s at address %x",
				 file->i_name->name, file->i_map_addr));
	break;

      case I_ACCESS_MMAP:
#ifdef READ_FILE
      case I_ACCESS_MMAP_READ:
#endif
	/* Round length to pagesize  */
	/* If the length of the file is exactly a multiple of pagesize,
	   we round the length up extra page to avoid pathological problems
	   with reading one byte past the end of the file. */
	len_file = MMAP_ROUND(file->i_filesz);

      retry_mmap:
	file->i_map_addr = mmap(NULL,
				len_file,
				PROT_READ,
				MAP_FILE | MAP_VARIABLE,
				file->i_fd,
				0);
	if (file->i_map_addr == (caddr_t)-1) {
	    /* Couldn't mmap file */
	    if (errno == ENOMEM && free_segments(SEGMENT(len_file) + 1) != 0)
		goto retry_mmap;	/* Retry */
	    else {
		/* mmap failed.  The returned errno is not always accurate,
		   so rather than see what the problem is, we just read or
		   map the file anonymously. */
#ifdef READ_FILE
		if (file->i_access == I_ACCESS_MMAP_READ) {
		    file->i_access = I_ACCESS_READ;
		    goto get_read_access;
		}
		else
#endif
		    map_anon(file, len_file);
		break;
	    }
	}
	file->i_access = I_ACCESS_MMAP;
	seg = SEGMENT(file->i_map_addr);
	file->i_auxnext = segment_info[seg].f;
	segment_info[seg].f = file;
	segment_info[seg].files_mapped++;
	for (file_end = ((ulong)file->i_map_addr & (SHMLBA-1))+ len_file;
	     file_end > SHMLBA;
	     ++seg, file_end -= SHMLBA) {
	    segment_info[seg].wrap = file;
	    segment_info[seg].files_mapped++;
	}
#ifdef STATS
	++Bind_state.mmap_total;
	++Bind_state.mmap_cur;
	Bind_state.mmap_max = max(Bind_state.mmap_max, Bind_state.mmap_cur);
#endif
	DEBUG_MSG(IFILES_DEBUG, (SAY_NO_NLS, "Mapped file %s at address %x",
				 file->i_name->name, file->i_map_addr));
	break;

#ifdef READ_FILE
      get_read_access:
	{
	    int fd = file->i_fd;
	    if ((file->i_file = fdopen(fd, "r")) == NULL) {
		close(fd);
		file->i_closed = 1;
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(FILE_CANNOT_OPEN,
			"%1$s: 0711-160 SEVERE ERROR: Cannot open file: %2$s\n"
			"\t%1$s:%3$s() %4$s"),
			 Main_command_name, file->i_name->name, "fdopen",
			 strerror(errno));
		return RC_SEVERE;
	    }
	}
	/* Fall through */

      case I_ACCESS_READ:
	/* Add file to chain, if not already on chain */
	if (file->i_auxnext == NULL) {
	    file->i_auxnext = read_files_root;
	    read_files_root = file;
	}
	file->i_map_addr = NULL;
	DEBUG_MSG(IFILES_DEBUG,
		  (SAY_NO_NLS, "File %s will be read.", file->i_name->name));
	break;
#endif

#ifdef I_ACCESS_MALLOC
      case I_ACCESS_MALLOC:
	file->i_map_addr = emalloc(file->i_filesz, id);

	/* Read file. */
	base = file->i_map_addr;
	size = file->i_filesz;
	while (size > 0) {
	    s = read(file->i_fd, file->i_map_addr, size);
	    if (s > 0) {
		size -= s;
		base += s;
	    }
	    else if (s == -1) {
		if (errno == EINTR)
		    continue;
		/* Error. */
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "read", strerror(errno));
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_READING_LEN,
 "%1$s: 0711-986 Error occurred while reading %2$d bytes from file: %3$s"),
			 Main_command_name, size, file->i_name->name);
		goto bad_read;
	    }
	    else {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(READ_EOF,
		"%1$s: 0711-983 SEVERE ERROR: File %2$s\n"
		"\tPremature end-of-file reached while reading %3$d bytes."),
			 Main_command_name, file->i_name->name, size);
	      bad_read:
		disclaim(file->i_map_addr, file->i_filesz, ZERO_MEM);
		efree(file->i_map_addr);
		file->i_map_addr = NULL;
		ifile_close_for_good(file);
		return RC_SEVERE;
	    }
	}
	file->i_auxnext = malloc_files_root;
	malloc_files_root = file;
	DEBUG_MSG(IFILES_DEBUG,
		  (SAY_NO_NLS, "Read file into allocated area."));
	break;
#endif

      case I_ACCESS_ANONMMAP:
	len_file = ROUND(file->i_filesz, Pagesize);
	return map_anon(file, len_file);

      default:
	internal_error();
    }
    return RC_OK;
} /* ifile_map */
/* ***********************************
 * Name:	ifile_close_for_good()
 * Purpose:	Close and unmap the specified file.  Used for bad input files.
 * Given:	File structure pointer.
 * ***********************************/
void
ifile_close_for_good(IFILE *file)
{
    ifile_unmap(file);
    ifile_close(file);
    file->i_closed = 1;
}
/* ***********************************
 * Name:	ifile_close()
 * Purpose:	Close the specified file.  If shmat() was used on the file,
 *		detach it, but do not munmap or free memory otherwise.
 * Given:	File structure pointer
 * ***********************************/
static void
ifile_close(IFILE *file)
{
    if (file->i_fd != -1) {
#ifdef READ_FILE
	if (file->i_access == I_ACCESS_READ) {
	    fclose(file->i_file);
	    file->i_map_addr = NULL;
	}
	else {
#endif
	    if (file->i_access == I_ACCESS_SHMAT)
		ifile_unmap(file);
	    (void) close(file->i_fd);	/* Should we check for errors? */
#ifdef READ_FILE
	}
#endif
	file->i_fd = -1;
#ifdef STATS
	++Bind_state.close_total;
	--Bind_state.open_cur;
#endif
    }
}
/* ***********************************
 * Name:	ifile_close_all()
 * Purpose:	Close all files for good.
 * ***********************************/
void
ifile_close_all(void)
{
    IFILE *f;

    for (f = first_ifile; f; f = f->i_next)
	ifile_close_for_good(f);
}
/* ***********************************
 * Name:	ifile_unmap()
 * Purpose:	Given a file structure pointer, unmap the file.
 * ***********************************/
static void
ifile_unmap(IFILE *file)
{
    off_t	len_f;
    int		seg;
    ulong	file_end;

    if (file->i_map_addr == NULL)
	return;

    switch(file->i_access) {
#ifdef READ_FILE
      case I_ACCESS_READ:
	/* Nothing mapped */
	/* Return to prevent i_map_addr from being nulled */
	return;
#endif

#ifdef I_ACCESS_MALLOC
      case I_ACCESS_MALLOC:
	disclaim(file->i_map_addr, file->i_filesz, ZERO_MEM);
	efree(file->i_map_addr);
	break;
#endif

      case I_ACCESS_MMAP:
      case I_ACCESS_ANONMMAP:
	len_f = MMAP_ROUND(file->i_filesz);

	if (munmap(file->i_map_addr, len_f) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "munmap", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
		     Main_command_name, file->i_name->name, file->i_map_addr);
	}

#ifdef STATS
	++Bind_state.munmap_total;
	--Bind_state.mmap_cur;
#endif

	seg = SEGMENT(file->i_map_addr);
	if (file->i_access == I_ACCESS_MMAP)
	    segment_info[seg].files_mapped--;
	else
	    segment_info[seg].anon--;

	for (file_end = ((ulong)file->i_map_addr & (SHMLBA-1))+len_f;
	     file_end > SHMLBA;
	     seg++, file_end -= SHMLBA) {
	    segment_info[seg].wrap = NULL;
	    if (file->i_access == I_ACCESS_MMAP)
		segment_info[seg].files_mapped--;
	    else
		segment_info[seg].anon--;
	}
	break;

      case I_ACCESS_SHMAT:
	if (shmdt(file->i_map_addr) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "shmdt", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING_ADDR,
"%1$s: 0711-995 Error occurred while processing file: %2$s (at address 0x%3$X)"),
		     Main_command_name, file->i_name->name, file->i_map_addr);
	}
	DEBUG_MSG(IFILES_DEBUG,
		  (SAY_NO_NLS, "Detached file %s from address %x",
		   file->i_name->name, file->i_map_addr));
#ifdef STATS
	++Bind_state.shmdt_total;
	--Bind_state.shmat_cur;
#endif
	seg = SEGMENT(file->i_map_addr);
	segment_info[seg].files_mapped = 0;
	segment_info[seg].f = NULL;
	break;
      default:
	internal_error();
	break;
    }

    file->i_map_addr = NULL;
} /* ifile_unmap */
#ifdef READ_FILE
/* *********************************************************************
 * Name:    fseek_read
 * Purpose: Seek to and read part of a file.
 * Returns: If seek and fread are successful,	0
 *	    Otherwise,				1
 **********************************************************************/
int
fseek_read(IFILE *f,
	   off_t offset,
	   void *area,
	   size_t length)
{
    if (length == 0)
	return 0;

    if (safe_fseek(f, offset, SEEK_SET) == 1)
	return 1;
    return safe_fread(area, length, f);
}
/* ***********************************
 * Name:	safe_fread
 *
 * Purpose:	Same function as fread, except that the call is retried if
 *		EINTR is returned.  If fread fails, an error is printed.
 *
 * Returns: If seek is successful,	0
 *	    Otherwise,			1
 * ***********************************/
int
safe_fread(void *area,
	   size_t length,
	   IFILE *f)
{
    int rc;

    if (length == 0)
	return 0;

    while ((rc = fread(area, length, 1, f->i_file)) != 1) {
	if (rc == 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(READ_EOF,
	    "%1$s: 0711-983 SEVERE ERROR: File %2$s\n"
	    "\tPremature end-of-file reached while reading %3$d bytes."),
		     Main_command_name, f->i_name->name, length);
	    return 1;
	}
	if (errno != EINTR) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "fread", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_READING_LEN,
 "%1$s: 0711-986 Error occurred while reading %2$d bytes from file: %3$s"),
		     Main_command_name, length, f->i_name->name);
	    return 1;
	}
    }
    return 0;
}
/* ***********************************
 * Name:	safe_fseek
 *
 * Purpose:	Same function as fseek, except that the call is retried if
 *		EINTR is returned.  If fseek fails, an error is printed.
 *
 * Returns: If seek is successful,	0
 *	    Otherwise,			1
 * ***********************************/
int
safe_fseek(IFILE *f,
	   off_t offset,
	   int whence)
{
    while (fseek(f->i_file, offset, whence) != 0) {
	if (errno != EINTR) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "fseek", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING,
	    "%1$s: 0711-996 Error occurred while processing file: %2$s"),
		     Main_command_name, f->i_name->name);
	    return 1;
	}
    }
    return 0;
}
#endif
