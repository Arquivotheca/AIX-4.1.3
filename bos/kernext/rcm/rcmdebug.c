static char sccsid[] = "@(#)43  1.1  rcmdebug.c, rcm, bos412 4/28/95 15:31:46";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager
 *
 *   FUNCTIONS: xmalloc_trace
 *		xmfree_trace
 *		xmalloc_trace_report
 *		talloc_trace
 *		tfree_trace
 *		talloc_trace_report
 *		pinu_trace
 *		unpinu_trace
 *		pinu_trace_report
 *		d_master_trace
 *		d_complete_trace
 *		d_master_trace_report
 *		xmattach_trace
 *		xmdetach_trace
 *		xmattach_trace_report
 *		dump_rcm_trace
 *		all_trace_reports
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef  RCMDEBUG

#include <sys/types.h>			/* Defines for data types(uchar)*/
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/timer.h>
#include <sys/pin.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include "rcm_mac.h"

#define  SOFFSET	0x0fffffff	/* however many bits we compare */
#define  EOFFSET	0x0fffffff	/* should be 28 bits for eff. addr. */

#define  SEGMATCH(A,B)	( (       (A) & SOFFSET) == (       (B) & SOFFSET) )
#define  EFFMATCH(A,B)	( ((long) (A) & EOFFSET) == ((long) (B) & EOFFSET) )

#define  EFFCOMPARE(A,OP,B)		\
	( ((long) (A) & EOFFSET) OP ((long) (B) & EOFFSET) )

/******************************************************************************/
/*                 Run all trace reports                                      */
/******************************************************************************/
int  all_trace_reports (
int  pid)
{
	xmalloc_trace_report  (pid);
	talloc_trace_report   (pid);
	pinu_trace_report     (pid);
	d_master_trace_report (pid);
	xmattach_trace_report (pid);
}

/******************************************************************************/
/*                 Trace xmalloc/xmfree                                       */
/******************************************************************************/

#define  MALLOC_TRACE_SIZE  8000

static  int xmalloc_trace_overflow = 0;
static  struct xmalloc_trace
	{
		char   *prog;
		int     line;
		int     pid;
		int     size;
		int     align;
		caddr_t heap;
		caddr_t ptr;	/* virtual address in the kernel space */
	} xmalloc_trace_buf[MALLOC_TRACE_SIZE];

caddr_t  xmalloc_trace (
int  size,
int  align,
caddr_t  heap,
char  *prog,
int  line)
{
	int  i, old_int;
	struct xmalloc_trace  *p;
	caddr_t  ptr;
static	int  first_time = 1;

	if (i_disable (INTBASE) != INTBASE)
		printf ("xmalloc: not at INTBASE: %s[#%d]\n", prog, line);

	/*
	 *  Check for bad args.
	 */
	if (heap == NULL)
	{
		printf (
		    "xmalloc: NULL heap: %s[#%d] pid %d size 0x%x align %d\n",
			prog, line, getpid (), size, align);

		return (NULL);
	}

	/*
	 *  Perform the real allocation call.
	 *  Log any failure.
	 */
	ptr = xmalloc (size, align, heap);
	if (ptr == NULL)
	{
		printf (
	"xmalloc: out of mem: %s[#%d] pid %d size 0x%x align %d heap 0x%x\n",
				prog, line, getpid (), size, align, heap);

		return (NULL);
	}

	/*
	 *  Initialize the table, the first time.
	 *
	 *  Scan the trace buffer to be sure that this newly malloc'd
	 *  buffer isn't contained inside any other active buffer.
	 *
	 *  Since we normally scan to the end, keep track of the first
	 *  empty slot for later.
	 */
	old_int = i_disable (INTMAX);

	if (first_time)			/* first time init */
	{
		first_time = 0;

		for (i=0,p=xmalloc_trace_buf; i<MALLOC_TRACE_SIZE; i++,p++)
			p->ptr = NULL;
	}

	/*
	 *  Record the new parameters inside a free slot.
	 */
	for (i=0,p=xmalloc_trace_buf; i<MALLOC_TRACE_SIZE; i++,p++)
		if (p->ptr == NULL)
			break;

	/*
	 *  If no free slot, log it and set flag.
	 */
	if (i >= MALLOC_TRACE_SIZE)
	{
		printf (
"xmalloc: trace ovflow: %s[#%d] pid %d size 0x%x align %d heap 0x%x ptr 0x%x\n",
				prog, line, getpid (), size, align, heap, ptr);

		xmalloc_trace_overflow = 1;
	}
	else				/* log data in trace buffer */
	{
		p->prog  = prog;
		p->line  = line;
		p->pid   = getpid ();
		p->size  = size;
		p->align = align;
		p->heap  = heap;
		p->ptr   = ptr;
	}

	i_enable (old_int);

	return (ptr);
}

int  xmfree_trace (
caddr_t  ptr,
caddr_t  heap,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct xmalloc_trace  *p;

	if (i_disable (INTBASE) != INTBASE)
		printf ("xmfree: not at INTBASE: %s[#%d]\n", prog, line);

	/*
	 *  Validate arguments.
	 */
	if (ptr == NULL)
	{
		printf ("xmfree: NULL ptr: %s[#%d] pid %d\n",
			prog, line, getpid ());

		return (0);
	}

	if (heap == NULL)
	{
		printf ("xmfree: NULL heap: %s[#%d] pid %d\n",
			prog, line, getpid ());

		return (EINVAL);
	}

	/*
	 *  Search trace array for item to be freed.
	 *  This must be done under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=xmalloc_trace_buf; i<MALLOC_TRACE_SIZE; i++,p++)
		if (p->ptr == ptr && p->heap == heap)
			break;

	/*
	 *  If the entry not found, log it.
	 */
	if (i >= MALLOC_TRACE_SIZE)
		printf ("xmfree: untraced free: %s[#%d] pid %d ptr 0x%x\n",
					prog, line, getpid (), ptr);

	i_enable (old_int);

	/*
	 *  Now free the memory.
	 */
	err = xmfree (ptr, heap);

	/*
	 *  Log any error.
	 */
	if (err)
	{
		printf ("xmfree: error %d: %s[#%d] pid %d\n",
					err, prog, line, getpid ());

		if (i < MALLOC_TRACE_SIZE)
		{
			printf (
    "from xmalloc: %s[#%d] pid %d size 0x%x align %d heap 0x%x ptr 0x%x\n",
				p->prog, p->line, p->pid,
				p->size, p->align, p->heap, p->ptr);
		}
	}

	if (i < MALLOC_TRACE_SIZE)	/* now clear the trace entry */
		p->ptr = NULL;

	return  err;
}

int  xmalloc_trace_report (
int  pid)
{
	int  i, old_int, count = 0;
	struct xmalloc_trace  *p;

	/*
	 *  Make report under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=xmalloc_trace_buf; i<MALLOC_TRACE_SIZE; i++,p++)
	{
		if ((p->pid == pid || pid == -1) && p->ptr != NULL)
		{
			count++;

			printf (
		"%s[#%d] pid %d size 0x%x align %d %sheap 0x%x ptr 0x%x\n",
			p->prog, p->line, p->pid, p->size, p->align,
			p->heap == pinned_heap  ? "pinned " :
			(p->heap == kernel_heap ? "kernel"  : "unknown "),
			p->heap, p->ptr);
		}
	}

	if (pid == -1)
		printf ("all pids: ");
	else
		printf ("pid %d: ", pid);

	if (!count)
		printf ("no active xmalloc trace entries\n");
	else
		printf ("%d active xmalloc trace entries\n", count);

	i_enable (old_int);

	return (count);
}

/******************************************************************************/
/*                 Trace talloc/tfree                                         */
/******************************************************************************/

#define  TALLOC_TRACE_SIZE  2000

static  int talloc_trace_overflow = 0;
static  struct talloc_trace
	{
		char   *prog;
		int     line;
		int     pid;
		struct trb  *ptr;
	} talloc_trace_buf[TALLOC_TRACE_SIZE];

struct trb  *talloc_trace (
char  *prog,
int  line)
{
	int  i, old_int;
	struct talloc_trace  *p;
	struct trb  *ptr;
static	int  first_time = 1;

	if (i_disable (INTBASE) != INTBASE)
		printf ("talloc: not at INTBASE: %s[#%d]\n", prog, line);

	/*
	 *  Perform the real allocation call.
	 *  Log any failure.
	 */
	ptr = talloc ();
	if (ptr == NULL)
	{
		printf (
		"talloc: out of mem: %s[#%d] pid %d\n", prog, line, getpid ());

		return (NULL);
	}

	/*
	 *  Record the new parameters inside a free slot.
	 *  This must be done under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	if (first_time)			/* clear the table, the first time */
	{
		first_time = 0;

		for (i=0,p=talloc_trace_buf; i<TALLOC_TRACE_SIZE; i++,p++)
			p->ptr = NULL;
	}

	/* scan for first free one */
	for (i=0,p=talloc_trace_buf; i<TALLOC_TRACE_SIZE; i++,p++)
		if (p->ptr == NULL)
			break;

	/*
	 *  If no free slot, log it and set flag.
	 */
	if (i >= TALLOC_TRACE_SIZE)
	{
		printf ("talloc: trace overflow: %s[#%d] pid %d ptr 0x%x\n",
						prog, line, getpid (), ptr);

		talloc_trace_overflow = 1;
	}
	else				/* log data in trace buffer */
	{
		p->prog  = prog;
		p->line  = line;
		p->pid   = getpid ();
		p->ptr   = ptr;
	}

	i_enable (old_int);

	return (ptr);
}

void  tfree_trace (
struct trb  *ptr,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct talloc_trace  *p;

	/*  No check for interrupt environment is required.  */

	/*
	 *  Validate arguments.
	 */
	if (ptr == NULL)
	{
		printf ("tfree: NULL ptr: %s[#%d] pid %d\n",
			prog, line, getpid ());

		return;
	}

	/*
	 *  Search trace array for item to be freed.
	 *  This must be done under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=talloc_trace_buf; i<MALLOC_TRACE_SIZE; i++,p++)
		if (p->ptr == ptr)
			break;

	/*
	 *  If the entry not found, log it and check to see if
	 *  overflow has occurred.  Don't crash if it has.
	 */
	if (i >= TALLOC_TRACE_SIZE)
	{
		printf ("tfree: untraced tfree: %s[#%d] pid %d ptr 0x%x\n",
					prog, line, getpid (), ptr);
	}
	else				/* entry found, validate heaps */
	{
		p->ptr = NULL;
	}

	i_enable (old_int);

	/*
	 *  Now free the timer memory.
	 */
	tfree (ptr);

	return;
}

int  talloc_trace_report (
int  pid)
{
	int  i, old_int, count = 0;
	struct talloc_trace  *p;

	/*
	 *  Make report under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=talloc_trace_buf; i<TALLOC_TRACE_SIZE; i++,p++)
	{
		if ((p->pid == pid || pid == -1) && p->ptr != NULL)
		{
			count++;

			printf ("%s[#%d] pid %d ptr 0x%x\n",
					p->prog, p->line, p->pid, p->ptr);
		}
	}

	if (pid == -1)
		printf ("all pids: ");
	else
		printf ("pid %d: ", pid);

	if (!count)
		printf ("no active talloc trace entries\n");
	else
		printf ("%d active talloc trace entries\n", count);

	i_enable (old_int);

	return (count);
}

/******************************************************************************/
/*                 Trace pin/unpin                                            */
/******************************************************************************/

#define  PIN_TRACE_SIZE  500

static  int pinu_trace_overflow = 0;
static  struct pinu_trace
	{
		char      *prog;
		int        line;
		int        pid;
		vmhandle_t seg;
		caddr_t    base;
		int        size;
		short	   segflg;
	} pinu_trace_buf[PIN_TRACE_SIZE];

int  pinu_trace (
caddr_t  base,
int  size,
short segflg,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct pinu_trace  *p;
static	int  first_time = 1;
	adspace_t  *adsp;

	if (i_disable (INTBASE) != INTBASE)
		printf ("xmattach: not at INTBASE: %s[#%d]\n", prog, line);

	adsp = getadsp ();	/* get address space ptr */

	/*
	 *  Do some parameter checking.
	 */
	if (base == NULL || size < 0)
	{
		printf (
	"pin%s: invalid base or size: %s[#%d] pid %d base 0x%x size 0x%x\n",
		segflg == (short) -1 ? "" : "u",
		prog, line, getpid (), base, size);

		return (EINVAL);
	}

	/*
	 *  Perform the real allocation call.
	 *  Log any failure.
	 */
	if (segflg == (short) -1)
	    err = pin (base, size);
	else
	    err = pinu (base, size, segflg);

	if (err)
	{
		printf (
		 "%s: failed: %s[#%d] pid %d base 0x%x size 0x%x segflg 0x%x\n",
			segflg == (short) -1 ? "pin" : "pinu",
			prog, line, getpid (), base, size, segflg);

		return (err);
	}

	/*
	 *  Record the new parameters inside a free slot.
	 *  This must be done under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	if (first_time)			/* first time only */
	{
		first_time = 0;

		for (i=0,p=pinu_trace_buf; i<PIN_TRACE_SIZE; i++,p++)
			p->base = NULL;
	}

	for (i=0,p=pinu_trace_buf; i<PIN_TRACE_SIZE; i++,p++)
		if (p->base == NULL)
			break;

	/*
	 *  If no free slot, log it and set flag.
	 */
	if (i >= PIN_TRACE_SIZE)
	{
		printf (
	"%s: trace overflow: %s[#%d] pid %d base 0x%x size 0x%x segflg 0x%x\n",
			segflg == (short) -1 ? "pin" : "pinu",
			prog, line, getpid (), base, size, segflg);

		pinu_trace_overflow = 1;
	}
	else				/* log data in trace buffer */
	{
		p->prog   = prog;
		p->line   = line;
		p->pid    = getpid ();
		if (segflg == (short) -1)		/* if kernel space */
			p->seg = (vmhandle_t) rcmvm_getsrval (base);
		else
			p->seg = (vmhandle_t) as_getsrval (adsp, base);
		p->base   = (caddr_t) base;
		p->size   = size;
		p->segflg = segflg;
	}

	i_enable (old_int);

	return (0);
}

int  unpinu_trace (
caddr_t  base,
int  size,
short  segflg,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct pinu_trace  *p;

	if (i_disable (INTBASE) != INTBASE)
		printf ("xmattach: not at INTBASE: %s[#%d]\n", prog, line);

	/*
	 *  Validate arguments.
	 */
	if (base == NULL || size < 0)
	{
		printf (
	    "%s: invalid base or size: %s[#%d] pid %d base 0x%x size 0x%x\n",
			segflg == (short) -1 ? "unpin" : "unpinu",
			prog, line, getpid (), base, size);

		return (EINVAL);
	}

	/*
	 *  Search trace array for item to be freed.
	 *  This must be done under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	/*
	 *  Match effective address offsets (lower 28 bits) and segment
	 *  register content, so as to be independent of the actual segment
	 *  register being used.  This is because an unpin operation may use
	 *  a different segment register than the pin.  Also, a pinu () may
	 *  be undone with an unpin (), not an unpinu ().
	 *
	 *  We omit testing the some bits at the top of segment registers.
	 */
	for (i=0,p=pinu_trace_buf; i<PIN_TRACE_SIZE; i++,p++)
	{
	    if (p->base != NULL && EFFMATCH (p->base, base) && p->size == size)
	    {
		if (segflg == -1)		/* kernel seg reg */
		{
		    if (SEGMATCH (p->seg, rcmvm_getsrval (base)))
			break;
		}
		else				/* process seg reg */
		{
		    if (SEGMATCH (p->seg, as_getsrval (getadsp (), base)))
			break;
		}
	    }
	}

	/*
	 *  If the entry not found, log it.
	 */
	if (i >= PIN_TRACE_SIZE)
	{
		printf (
    "%s: untraced unpin(u): %s[#%d] pid %d base 0x%x size 0x%x segflg 0x%x\n",
			segflg == (short) -1 ? "unpin" : "unpinu",
			prog, line, getpid (), base, size, segflg);
	}

	/* 'i' is still sacred! */

	i_enable (old_int);

	/*
	 *  Now unpin the memory.
	 */
	if (segflg == -1)
	    err = unpin (base, size);
	else
	    err = unpinu (base, size, segflg);

	/*
	 *  Log any error and crash.
	 */
	if (err)
	{
		printf ("unpin%s: error %d: %s[#%d] pid %d\n",
				segflg == (short) -1 ? "" : "u",
				err, prog, line, getpid ());

		if (i < PIN_TRACE_SIZE)
		{
			printf (
    "from pin%s: %s[#%d] pid %d base 0x%x size 0x%x segflg 0x%x\n",
				p->segflg == (short) -1 ? "" : "u",
				p->prog, p->line, p->pid,
				p->base, p->size, p->segflg);
		}
	}

	if (i < PIN_TRACE_SIZE)
		p->base = NULL;

	return  err;
}

int  pinu_trace_report (
int  pid)
{
	int  i, old_int, count = 0;
	struct pinu_trace  *p;

	/*
	 *  Make report under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=pinu_trace_buf; i<PIN_TRACE_SIZE; i++,p++)
	{
		if ((p->pid == pid || pid == -1) && p->base != NULL)
		{
			count++;

			printf (
		"%s[#%d] pid %d base 0x%x size 0x%x segflg 0x%x\n",
			p->prog, p->line, p->pid, p->base, p->size, p->segflg);
		}
	}

	if (pid == -1)
		printf ("all pids: ");
	else
		printf ("pid %d: ", pid);

	if (!count)
		printf ("no active pin(u) trace entries\n");
	else
		printf ("%d active pin(u) trace entries\n", count);

	i_enable (old_int);

	return (count);
}

/******************************************************************************/
/*                 Trace d_master/d_complete                                  */
/******************************************************************************/

#define  D_MASTER_TRACE_SIZE  1000
#define  D_MASTER_MAX_CHAN    0x10

static  int d_master_trace_overflow = 0;
static  struct d_master_trace
	{
		char   *prog;
		int     line;
		int	pid;
		int	chan;
		int	flags;
		caddr_t	baddr;
		unsigned baddr_seg;
		size_t	count;
		struct xmem * xmem;
		unsigned xmem_seg;
		caddr_t	daddr;
		int     dbg_flags;
		int	pad;
	} d_master_trace_buf[D_MASTER_TRACE_SIZE];

void  d_master_trace (
int  chan,
int  flags,
caddr_t  baddr,
size_t  count,
struct xmem *xmem,
caddr_t  daddr,
char  *prog,
int  line)
{
	int  i, old_int;
	struct d_master_trace  arg;
	struct d_master_trace  *p, *parg = &arg;
static	int  first_time = 1;

	/* stash args in local structure */
	parg->prog  = prog;
	parg->line  = line;
	parg->pid   = getpid ();

	parg->chan  = chan;
	parg->flags = flags;
	parg->baddr = baddr;
	parg->count = count;
	parg->xmem  = xmem;
	parg->daddr = daddr;

	parg->baddr_seg = xmem->subspace_id;
	parg->xmem_seg  = rcmvm_getsrval (xmem);

	parg->dbg_flags = 0;

	/*
	 *  Log any bad args.
	 */
	check_d_master_record ("d_master: bad args: ", parg);

#if 0					/* for hard debug cases only */
	log_d_master_record (parg);

	print_d_master_record ("d_master log to tty: ", parg);
#endif

	/*
	 *  Perform the real setup call.
	 */
	d_master (parg->chan, parg->flags, parg->baddr, parg->count,
						parg->xmem, parg->daddr);

	/*
	 *  Initialize the table, the first time.
	 */
	old_int = i_disable (INTMAX);

	if (first_time)			/* first time init */
	{
		first_time = 0;

		for (i=0,p=d_master_trace_buf; i<D_MASTER_TRACE_SIZE; i++,p++)
			p->xmem = NULL;
	}

	/*
	 *  Scan for buffer overlap, assuming that these are systemwide unique.
	 */
	for (i=0,p=d_master_trace_buf; i<D_MASTER_TRACE_SIZE; i++,p++)
	{
		if (p->xmem != NULL                                     &&
		    SEGMATCH (p->baddr_seg, parg->baddr_seg)            &&
		    EFFCOMPARE (parg->baddr + parg->count, >, p->baddr) &&
		    EFFCOMPARE (parg->baddr, <, p->baddr + p->count)       )
		{
			print_d_master_record (
				"d_master: buffer overlap: ", parg);
			print_d_master_record (
				"                    with: ", p);
			print_d_master_delta  (parg, p);
		}
	}

	/* locate empty slot */
	for (i=0,p=d_master_trace_buf; i<D_MASTER_TRACE_SIZE; i++,p++)
	{
		if (p->xmem == NULL)
			break;
	}

	if (i >= D_MASTER_TRACE_SIZE)		/* not found */
	{
		print_d_master_record ("d_master: trc ovfl: ", parg);

		d_master_trace_overflow = 1;
	}
	else
	{
		copy_d_master_record (p, parg);
	}

	i_enable (old_int);

	return;
}

int  d_complete_trace (
int  chan,
int  flags,
caddr_t  baddr,
size_t  count,
struct xmem *xmem,
caddr_t  daddr,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct d_master_trace  arg;
	struct d_master_trace  *p, *parg = &arg;

	/* stash args in local structure */
	parg->prog  = prog;
	parg->line  = line;
	parg->line  = getpid ();

	parg->chan  = chan;
	parg->flags = flags;
	parg->baddr = baddr;
	parg->count = count;
	parg->xmem  = xmem;
	parg->daddr = daddr;

	parg->baddr_seg = xmem->subspace_id;
	parg->xmem_seg  = rcmvm_getsrval (xmem);

	parg->dbg_flags = 0;

	/*
	 *  Check for bad args.
	 */
	check_d_master_record ("d_complete: bad args: ", parg);

#if  0					/* for hard debug cases only */
	log_d_master_record (parg);

	print_d_master_record ("d_complete log to tty: ", parg);
#endif

#ifdef  RCMDEBUG				/* nested */
	/*
	 *  D_complete must exactly match d_master for the RCM.
	 */
#else
	/*
	 *  D_complete must be positioned within one of the d_master calls.
	 *
	 *  Consider issueing warning if flags do not indicate DMA_NOHIDE.
	 */
#endif
	old_int = i_disable (INTMAX);

	for (i=0,p=d_master_trace_buf; i<D_MASTER_TRACE_SIZE; i++,p++)
	{
#ifdef  RCMDEBUG
		if (p->xmem != NULL &&
		    SEGMATCH (p->baddr_seg, parg->baddr_seg) &&
		    EFFMATCH (p->baddr    , parg->baddr    )    )
		{
			break;
		}
#else
		if (p->xmem != NULL                                  &&
		    SEGMATCH (p->baddr_seg, parg->baddr_seg)         &&
		    EFFCOMPARE (parg->baddr, >=, p->baddr)           &&
		    EFFCOMPARE (parg->baddr + parg->count, <=,
						p->baddr + p->count)    )
		{
			break;
		}
#endif
	}

	if (i >= D_MASTER_TRACE_SIZE)
	{
		print_d_master_record ("d_complete: untraced: ", parg);
	}
	else
	{
		/* compare some things that shouldn't change */
		if (p->chan  != parg->chan                    ||
		    p->flags != parg->flags                   ||
		    !EFFMATCH (p->xmem, parg->xmem)           ||
		    !SEGMATCH (p->xmem_seg, parg->xmem_seg)       )
		{
			print_d_master_record ("d_complete: parm mismatch: ",
									parg);

			print_d_master_record ("        original d_master: ",
									p);
			print_d_master_delta (parg, p);
		}
	}

	i_enable (old_int);

	/*
	 *  Now perform the d_complete.
	 */
	err = d_complete (parg->chan, parg->flags,
			parg->baddr, parg->count, parg->xmem, parg->daddr);

	/*
	 *  Log any error.
	 */
	if (err)
	{
		printf ("d_complete: error %d: %s[#%d]\n",
					err, prog, line);

		if (i < D_MASTER_TRACE_SIZE)
		{
			print_d_master_record ("original d_master: ", p);
			print_d_master_delta (parg, p);
		}
	}

	if (i < D_MASTER_TRACE_SIZE)
	{
#ifdef  RCMDEBUG
		p->xmem = NULL;			/* clear out entry */
#else
		p->dbg_flags = 1;		/* say d_complete occurred */
#endif
	}

	return  err;
}

int  d_master_trace_report (
int  pid)
{
	int  i, old_int, count = 0;
	struct d_master_trace  *p;

	/*
	 *  Make report under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=d_master_trace_buf; i<D_MASTER_TRACE_SIZE; i++,p++)
	{
		if (p->xmem != NULL && (pid == -1 || p->pid == pid))
		{
			print_d_master_record (NULL, p);

			count++;
		}
	}

	if (pid == -1)
		printf ("all pids: ");
	else
		printf ("pid %d: ", pid);

	if (!count)
		printf ("no active d_master trace entries\n");
	else
		printf ("%d active d_master trace entries\n", count);

	i_enable (old_int);

	return (count);
}

copy_d_master_record (
struct d_master_trace *pto,
struct d_master_trace *pfrom)
{
	pto->prog  = pfrom->prog;
	pto->line  = pfrom->line;
	pto->pid   = pfrom->pid;

	pto->chan  = pfrom->chan;
	pto->flags = pfrom->flags;
	pto->baddr = pfrom->baddr;
	pto->count = pfrom->count;
	pto->xmem  = pfrom->xmem;
	pto->daddr = pfrom->daddr;

	pto->baddr_seg = pfrom->baddr_seg;
	pto->xmem_seg  = pfrom->xmem_seg;

	pto->dbg_flags = pfrom->dbg_flags;
}

int check_d_master_record (
char  *s,
struct d_master_trace *p)
{
#define MAX_DMA_ID 0x10
	int  rc       = 0;
	int  chanerr;
	int  baddrerr;
	int  counterr;
	int  xmemerr, xmemspacerr;
	int  daddrerr;

	chanerr     = (p->chan < 0 || p->chan >= MAX_DMA_ID);
	baddrerr    = (p->baddr == NULL);
	counterr    = (p->count <= 0);
	xmemerr     = (p->xmem == NULL || ((unsigned long) p->xmem >> 28) != 0);
	xmemspacerr = (p->xmem != NULL && p->xmem->aspace_id == XMEM_INVAL);
	daddrerr    = (p->daddr == NULL);

	if (chanerr || baddrerr || counterr || xmemerr || xmemspacerr ||
								daddrerr)
	{
		print_d_master_record (s, p);
		rc = EINVAL;
	}

	if (chanerr)
		printf ("chan out of range\n");

	if (baddrerr)
		printf ("baddr NULL\n");

	if (counterr)
		printf ("count <= 0\n");

	if (xmemerr)
		printf ("xmem NULL or not in kernel space\n");

	if (xmemspacerr)
		printf ("xmem->aspace_id marked XMEM_INVAL\n",
							p->xmem->aspace_id);

	if (daddrerr)
		printf ("daddr NULL\n");

	return  rc;
}

int log_d_master_record (
struct d_master_trace  *p)
{
	static int  firsttime = 1;
	static char id[] = { "RCMT" };
	static int  i = 0;
	static struct d_master_trace d_master_trace_circle[0x10];

	if (firsttime)
	{
		printf ("RCMT trace at 0x%x\n", d_master_trace_circle);
		firsttime = 0;
	}

	copy_d_master_record (&d_master_trace_circle[i & 0xf], p);

	i++;

	return (i);
}

print_d_master_record (
char  *s,
struct d_master_trace  *p)
{
	printf ("%s", s);

	printf ("%s[#%d] pid %d chan 0x%x\n", p->prog, p->line,
							p->pid, p->chan);
	printf ("  flags 0x%x baddr 0x%x count 0x%x xmem 0x%x daddr 0x%x\n",
			    p->flags, p->baddr, p->count, p->xmem, p->daddr);
	printf ("  baddr_seg 0x%x xmem_seg 0x%x dbg_flags 0x%x\n",
			    p->baddr_seg, p->xmem_seg, p->dbg_flags);
}

print_d_master_delta (
struct d_master_trace  *p1,
struct d_master_trace  *p2)
{
	printf ("delta:");

	if (p1->chan != p2->chan)
		printf (" chan");

	if (p1->flags != p2->flags)
		printf (" flags");

	if (!EFFMATCH (p1->baddr, p2->baddr)         ||
	    !SEGMATCH (p1->baddr_seg, p2->baddr_seg)    )
		printf (" baddr/baddr_seg");

	if (p1->count != p2->count)
		printf (" count");

	if (!EFFMATCH (p1->xmem, p2->xmem)         ||
	    !SEGMATCH (p1->xmem_seg, p2->xmem_seg)    )
		printf (" xmem/xmem_seg");

	if (p1->daddr != p2->daddr)
		printf (" daddr");

	printf ("\n");
}

/******************************************************************************/
/*                 Trace xmattach/xmdetach                                    */
/******************************************************************************/

#define  XMATTACH_TRACE_SIZE  100

static  int xmattach_trace_overflow = 0;
static  struct xmattach_trace
	{
		char   *prog;
		int	line;
		int     pid;
		char   *addr;
		int     count;
		struct xmem  *xmem;
		int     segflg;
	} xmattach_trace_buf[XMATTACH_TRACE_SIZE];

int  xmattach_trace (
char  *addr,
int  count,
struct xmem  *xmem,
int   segflg,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct xmattach_trace  *p;
static	int  first_time = 1;

	if (i_disable (INTBASE) != INTBASE)
		printf ("xmattach: not at INTBASE: %s[#%d]\n", prog, line);

	/*
	 *  Check for bad args.
	 */
	if (addr == NULL || xmem == NULL || count <= 0)
	{
		printf (
"xmattach: bad args: %s[#%d] pid %d addr 0x%x count 0x%x xmem 0x%x segflg %d\n",
			prog, line, getpid (), addr, count, xmem, segflg);

		return (XMEM_FAIL);
	}

	/*
	 *  Perform the real allocation call.
	 *  Log any failure.
	 */
	err = xmattach (addr, count, xmem, segflg);
	if (err)
	{
		printf (
   "xmattach: error: %s[#%d] pid %d addr 0x%x count 0x%x xmem 0x%x segflg %d\n",
			prog, line, getpid (), addr, count, xmem, segflg);

		return (err);
	}

	/*
	 *  Initialize the table, the first time.
	 */
	old_int = i_disable (INTMAX);

	if (first_time)			/* first time init */
	{
		first_time = 0;

		for (i=0,p=xmattach_trace_buf; i<XMATTACH_TRACE_SIZE; i++,p++)
			p->xmem = NULL;
	}

	/*
	 *  Record the new parameters inside a free slot.
	 */
	for (i=0,p=xmattach_trace_buf; i<XMATTACH_TRACE_SIZE; i++,p++)
		if (p->xmem == NULL)
			break;

	/*
	 *  If no free slot, log it and set flag.
	 */
	if (i >= XMATTACH_TRACE_SIZE)
	{
		printf (
"xmattach: trace ovflow: %s[#%d] pid %d addr 0x%x count 0x%x xmem 0x%x int %d\n",
			prog, line, getpid (), addr, count, xmem, segflg);

		xmattach_trace_overflow = 1;
	}
	else				/* log data in trace buffer */
	{
		p->prog   = prog;
		p->line   = line;
		p->pid    = getpid ();
		p->addr   = addr;
		p->count  = count;
		p->xmem   = xmem;
		p->segflg = segflg;
	}

	i_enable (old_int);

	return (0);
}

int  xmdetach_trace (
struct xmem *xmem,
char  *prog,
int  line)
{
	int  i, old_int, err;
	struct xmattach_trace  *p;

	/*
	 *  Validate arguments.
	 */
	if (xmem == NULL)
	{
		printf ("xmdetach: bad arg: %s[#%d] pid %d xmem 0x%x\n",
			prog, line, getpid (), xmem);

		return (XMEM_FAIL);
	}

	/*
	 *  Search trace array for item to be freed.
	 *  This must be done under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=xmattach_trace_buf; i<XMATTACH_TRACE_SIZE; i++,p++)
		if (p->xmem == xmem)
			break;

	/*
	 *  If the entry not found, log it.
	 */
	if (i >= XMATTACH_TRACE_SIZE)
		printf (
		    "xmdetach: untraced xmdetach: %s[#%d] pid %d xmem 0x%x\n",
					prog, line, getpid (), xmem);

	i_enable (old_int);

	/*
	 *  Now free the memory.
	 */
	err = xmdetach (xmem);

	/*
	 *  Log any error.
	 */
	if (err)
	{
		printf ("xmdetach: error %d: %s[#%d] pid %d\n",
					err, prog, line, getpid ());

		if (i < XMATTACH_TRACE_SIZE)
		{
			printf (
   "from xmattach: %s[#%d] pid %d addr 0x%x count 0x%x xmem 0x%x segflg 0x%x\n",
				p->prog, p->line, p->pid,
				p->addr, p->count, p->xmem, p->segflg);
		}
	}

	if (i < XMATTACH_TRACE_SIZE)	/* now clear the trace entry */
		p->xmem = NULL;

	return  err;
}

int  xmattach_trace_report (
int  pid)
{
	int  i, old_int, count = 0;
	struct xmattach_trace  *p;

	/*
	 *  Make report under interrupt protection.
	 */
	old_int = i_disable (INTMAX);

	for (i=0,p=xmattach_trace_buf; i<XMATTACH_TRACE_SIZE; i++,p++)
	{
		if ((p->pid == pid || pid == -1) && p->xmem != NULL)
		{
			count++;

			printf (
		"%s[#%d] pid %d addr 0x%x count 0x%x xmem 0x%x segflg %d\n",
			p->prog, p->line, p->pid, p->addr, p->count,
			p->xmem, p->segflg);
		}
	}

	if (pid == -1)
		printf ("all pids: ");
	else
		printf ("pid %d: ", pid);

	if (!count)
		printf ("no active xmattach trace entries\n");
	else
		printf ("%d active xmattach trace entries\n", count);

	i_enable (old_int);

	return (count);
}
#endif

/******************************************************************************/
/*                 Dump rcm trace buffer                                      */
/******************************************************************************/

#ifdef RCMDEBUG
int  dump_rcm_trace (arg)
int  arg;
{
	int  i, kt, start, indx, old_int;
	struct _trace  *tr;
static	int  state = 1;			/* trace on, initially */

	old_int = i_disable (INTMAX);

	if (arg == -1)			/* remote turn off */
		state = 0;

	if (arg == -2)			/* remote turn on */
		state = 1;

	if (state == 0)			/* if off, don't do it */
	{
		i_enable (old_int);
		return;
	}

	kt = trace_all.tcount;		/* total number of trace calls */
	start = (kt - 1) - arg;		/* relative entry to start with */
	start = start % TRACE_MAX;	/* offset in circular buffer */
	tr = &trace_all.trace[start];	/* point to entry in circ buf */
	indx = kt - arg;		/* initialize entry counter */
	printf ("RCMDEBUG: &trace_all = 0x%x    entry %d\n", &trace_all, indx);
	printf (" code   type              pid        datum1        datum2\n");
	for (i=0; i<10 && indx>=0; i++,indx--)
	{
		int  sw;

		sw = tr->type;
/**/	/*	sw = 0;	*/	/* lots of edits are wrong */
		switch (sw)
		{
		    case 0x900:
			printf (
			    "0x%03x   MAKE_GP (begin)   %4d    pproc 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x901:
			printf (
			    "0x%03x   MAKE_GP (end)     %4d    pproc 0x%08x    pcproc 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x910:
			printf (
			    "0x%03x   UNMAKE_GP (begin) %4d    pprocd 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x911:
			printf (
			    "0x%03x   UNMAKE_GP (end)   %4d    devHead.flags 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x909:
			printf (
			    "0x%03x   UNMAKE_GP (state) %4d    pproc 0x%08x    pdev 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

/*		    case DMA_SERVICE:
			printf (
			    "0x%03x   DMA_SERVICE       %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;
*/
		    case 0x300:
			printf (
			    "0x%03x   CREATE_RCX        %4d    prcx 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x310:
			printf (
			    "0x%03x   DELETE_RCX (gsc)  %4d    prcx 0x%08x    prcx->pDomain->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x340:
			printf (
			    "0x%03x   DELETE_RCX (rcm)  %4d    prcx 0x%08x    del_proc 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

/*		    case CREATE_RCXP:
			printf (
			    "0x%03x   CREATE_RCXP       %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;
*/
		    case 0x360:
			printf (
			    "0x%03x   DELETE_RCXP (rcm) %4d    prcxp 0x%08x    del_proc 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

/*		    case ASSOCIATE_RCXP:
			printf (
			    "0x%03x   ASSOCIATE_RCXP    %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;
*/
/*		    case DISASSOCIATE_RCXP:
			printf (
			    "0x%03x   DISASSOCIATE_RCXP %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;
*/
		    case 0x400:
			printf (
			    "0x%03x   CREATE_WIN_GEOM   %4d    pwg 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x410:
			printf (
			    "0x%03x   DELETE_WIN_GEOM   %4d    pwg 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x420:
			printf (
			    "0x%03x   UPDATE_WIN_GEOM   %4d    pwg 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x430:
			printf (
			    "0x%03x   CREATE_WIN_ATTR   %4d    pwa 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x440:
			printf (
			    "0x%03x   DELETE_WIN_ATTR   %4d    pwa 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x450:
			printf (
			    "0x%03x   UPDATE_WIN_ATTR   %4d    pwa 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x320:
			printf (
			    "0x%03x   BIND_WINDOW       %4d    prcx 0x%08x    pwg 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x330:
			printf (
			    "0x%03x   SET_RCX           %4d    prcx 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x220:
			printf (
			    "0x%03x   RCM_LOCK_PDEV     %4d    flags 0x%08x    devHead->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x222:
			printf (
			    "0x%03x   RCM_LOCK_PDEV (gu)%4d    flags 0x%08x    devHead->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x230:
			printf (
			    "0x%03x   RCM_UNLOCK_PDEV   %4d    flags 0x%08x    devHead->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x231:
			printf (
			    "0x%03x   RCM_UNLOCK_PDEV-ug%4d    flags 0x%08x    devHead->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x240:
			printf (
			    "0x%03x   LOCK_DOMAIN       %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x241:
			printf (
			    "0x%03x   UNLOCK_DOMAIN     %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x261:
			printf (
			    "0x%03x   GIVE_UP_TIMESLICE %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x263:
			printf (
			    "0x%03x   GP_GIVE_UP_TIMESL %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x207:
			printf (
			    "0x%03x   GP_GIVE_UP_TIMlcp %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0xffffffff:
			printf (
			    "0x%03x   GP_GIVE_UP_TIMtim %4d    pdom 0x%08x    pdom->pDevTimer 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0xfffff112:
			printf (
			    "0x%03x   GP_GIVE_UP_TIMgur %4d    pdom 0x%08x    pdom->guardlist 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x800:
			printf (
			    "0x%03x   COMMAND_LIST      %4d\n",
				tr->type, tr->pid);
			break;

		    case 0x810:
			printf (
			    "0x%03x   GSC COMMAND       %4d    0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

/*		    case CREATE_COLORMAP:
			printf (
			    "0x%03x   CREATE_COLORMAP   %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case DELETE_COLORMAP:
			printf (
			    "0x%03x   DELETE_COLORMAP   %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case UPDATE_COLORMAP:
			printf (
			    "0x%03x   UPDATE_COLORMAP   %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;
*/
		    case 0x200:
			printf (
			    "0x%03x   GUARD_DOM (in)    %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x205:
			printf (
			    "0x%03x   GUARD_DOM (lckcpy)%4d    pdom 0x%08x    2\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x201:
			printf (
			    "0x%03x   GUARD_DOM (out)   %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x210:
			printf (
			    "0x%03x   UNGUARD_DOM       %4d    pdom 0x%08x    pdom->flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x950:
			printf (
			    "0x%03x   FIX_MSTSAVE       pproc 0x%08x    pdom->auth 0x%08x    flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x100:
			printf (
			    "0x%03x   PROC_FAULT        %d    pdom 0x%08x    pproc 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x101:
			printf (
			    "0x%03x   PROC_FAULT (more) except[0] 0x%08x    except[1] 0x%08x    except[3] 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x104:
			printf (
			    "0x%03x   FAULT HANDLER     pdom->pCur 0x%08x    pproc->pDomainCur[0] 0x%08x    procHead.flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x105:
			printf (
			    "0x%03x   FAULT BLOCK REQ   %d    pdom 0x%08x    procHead.flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x106:
			printf (
			    "0x%03x   FAULT BLOCK REQ dn%d    pdom 0x%08x    procHead.flags 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x206:
			printf (
			    "0x%03x   FAULT LOCKCPY     %d    pdom 0x%08x    2\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x102:
			printf (
			    "0x%03x   FAULT UNSUSPEND   pdom->flags 0x%08x\n",
				tr->type, tr->pid);
			break;

		    case 0x350:
			printf (
			    "0x%03x   MK_CUR & GD_DOM   %d    prcx 0x%08x\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x110:
			printf (
			    "0x%03x   GP_DISPATCH       %d    pdom 0x%08x    ptimer 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x111:
			printf (
			    "0x%03x   GP_DSP, GD LIST   %d    pdom 0x%08x    pdom->guardlist 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x208:
			printf (
			    "0x%03x   GP_DSP, LCKCPY    %d    pdom 0x%08x    1\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x120:
			printf (
			    "0x%03x   RCX_SWITCH_DONE   %d    prcx 0x%08x    switches 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x122:
			printf (
			    "0x%03x   RCX_SWDN, STRT TMR tv_sec %d   tv_nsec %d\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x121:
			printf (
			    "0x%03x   RCX_SWDN, UNBLK   %d   target pid %d\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x123:
			printf (
			    "0x%03x   RCX_SWDN, WAKE    %d   target pid %d\n",
				tr->type, tr->pid, tr->a);
			break;

		    case 0x124:
			printf (
			    "0x%03x   RCX_SWDN, WAKE GD %d     pdom 0x%08x    pdom->guardlist 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x130:
			printf (
			    "0x%03x   RCX_SWITCH        %d     pold 0x%08x    pnew 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x140:
			printf (
			    "0x%03x   RCX_DISPATCH      %d     pdom 0x%08x    switches 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x150:
			printf (
			    "0x%03x   RCX_FAULT_LIST    %d     prcx 0x%08x    switches 0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;

		    case 0x156:
			printf (
			    "0x%03x   RCX_FAULT_LIST blk%d     flags 0x%08x\n",
				tr->type, tr->pid, tr->b);
			break;

		    case 0x180:
			printf (
			    "0x%03x   GP_BLOCK_GP       %d     flags 0x%08x\n",
				tr->type, tr->pid, tr->b);
			break;

		    case 0x190:
			printf (
			    "0x%03x   GP_UNBLOCK_GP     %d     flags 0x%08x\n",
				tr->type, tr->pid, tr->b);
			break;

		    default:
			printf (
			    "0x%03x                     %4d    0x%08x    0x%08x\n",
				tr->type, tr->pid, tr->a, tr->b);
			break;
		}

		tr--;
		if (tr < &trace_all.trace[0])
			tr = &trace_all.trace[TRACE_MAX - 1];
	}

	i_enable (old_int);
}

#endif
