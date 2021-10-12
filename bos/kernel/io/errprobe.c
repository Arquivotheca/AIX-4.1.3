static char sccsid[] = "@(#)34	1.2  src/bos/kernel/io/errprobe.c, syserrlg, bos411, 9428A410j 11/3/93 09:56:43";

/*
 * COMPONENT_NAME: SYSERRLG   for probe data handling
 *
 * FUNCTIONS: probe, kprobe
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/low.h>
#include <sys/m_intr.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/erec.h>
#include <sys/rasprobe.h>
#include <sys/sstbl.h>
#include <sys/errids.h>
#include "errdd.h"


extern struct errc errc;		/* Error DD control info */
extern void errput(struct errvec *, int);

/*
 * The data is handled differently for application and kernel code,
 * probe() and kprobe().  This xfer vector is used to provide data
 * pointers and handling proceedures to common routines such as
 * probe_do().  See probe() and kprobe() for further explanation.
 */
struct xfer {
	/* The copydata routine just copies data from source to target. */
	int (*copydata)(char *,char *,int);
	/* copy_to_errvec copies data to the buffer accessed via errvec. */
	int (*copy_to_errvec)(char *, struct xfer *, int);
	/* copystr_to_errvec copies string data to the buffer accessed
	 * via errvec.  It essentially has the same interface as copyinstr().
	 */
	int (*copystr_to_errvec)(char *, struct xfer *, int, int *);
	/* copymine_to_errvec copies local data to the buffer accessed
	 * via errvec.  Local data refers to data declared here,
	 * in errprobe.
	 */
	int (*copymine_to_errvec)(char *, struct xfer *, int, void **);
	/* Perform any cleanup activities on errvec when finished. */
	void (*close_errvec)(struct xfer *);
	char *bufp;		/* Where to put data */
	char *datap;		/* Where last data was put */
	struct errvec *errvec;	/* error vector */
	struct avec *av;	/*   Vector element pointer */
	int len;		/* Overall data length */
	struct sympt_data0 *syp;/* Symptom data header */
};
/* For these macros to work, the xfer structure should be pointed
 * to by "xfer"
 */
#define BUFP xfer->bufp
#define DATAP xfer->datap
#define DATAL xfer->len
#define MAGIC xfer->syp->probe_magic
#define EV xfer->errvec
#define EVN EV->nel
#define EVE EV->error_id
#define EVP xfer->av
#define CRNTP EVP->p
#define CRNTL EVP->len
#define NEXTVEC EVN++; EVP++

/* Test to see if we can page fault */
#define CAN_FAULT (csa->intpri == INTBASE)

/* Origin flags */
#define FROM_APP 0
#define FROM_K 1

static int pagable_probe(probe_t *,int);
static int unpagable_probe(probe_t *);
static int probe_do(probe_t *,struct xfer *);
static int check_prec0(struct probe_rec0 *, struct sympt_data0 *);
static int check_and_copy_ssdata(struct sskwd *,struct xfer *,int,int);
static int copydata_app(char *,char *,int);
static int copydata_k(char *,char *,int);
static int copyin_app(char *, struct xfer *, int);
static int copyin_pagable_k(char *, struct xfer *, int);
static int copyin_k(char *, struct xfer *, int);
static int copyinstr_app(char *, struct xfer *, int, int *);
static int copyinstr_pagable_k(char *, struct xfer *, int, int *);
static int copyinstr_k(char *, struct xfer *, int, int *);
static int copy_pagable_local(char *, struct xfer *,int,void **);
static int copy_local(char *, struct xfer *,int,void **);
static void close_app(struct xfer *);
static void close_k(struct xfer *);

/*
 * NAME: probe
 *                                                                    
 * FUNCTION: Handle a probe() system call from an application.
 *
 * ENVIRONMENT:
 *	probe() is exported in syscalls.exp and is called by application
 *	programs.  Kernel services should call kprobe().
 *	It is assumed we can page fault.
 *
 * Input:
 *	A pointer to a probe_rec structure, see sys/rasprobe.h.
 *                                                                    
 * RETURNS:
 *	0 - Successful
 *	-1 - An error occurred.
 *
 *    errno values set if -1 returned:
 *	EINVAL - invalid data
 *	EFAULT - a bad address was passed.
 *	EAGAIN - Resources unavailable.
 *
 * Notes:
 *	probe() copies the user's data into a local buffer and
 *	passes it to errput... via an address vector.  The data
 *	handling routines are told to literally copy the data by means
 *	of the xfer vector.
 */  
int
probe(probe_t *p)
{
	int rv;

	/* We're in trouble if we can't page fault. */
	assert(CAN_FAULT);

	rv = pagable_probe(p,FROM_APP);

	if (rv!=0) {
		/* Error from probe_do */
		setuerror(rv);
		return(-1);
	}
	return(0);
}

/*
 * NAME: kprobe
 *                                                                    
 * FUNCTION: Handle a kprobe() kernel subroutine call.
 *
 * ENVIRONMENT:
 *	kprobe() is called from kernel services to issue probe statements.
 *	Application programs must call probe()
 *	kprobe() may or may not be able to page fault.
 *                                                                    
 * INPUT:
 *	A pointer to a probe_rec structure.
 *
 * RETURNS:
 *	0 - Successful
 *	EINVAL - invalid data
 *	EFAULT - a bad address was passed.
 *
 * Notes:
 *	For Kernel calls, their data isn't literally copied, but
 *	pointed to by an errvec structure.  Like probe(), we
 *	have errvec entries for erec0 and sympt_data0, but also
 *	separate vector entries are used for the individual keywords
 *	and their values, where probe() copies the user data to a
 *	contiguous area.
 *	In kprobe() the "buf" area is used for the errvec, while
 *	in the probe() routine, the data itself is copied into "buf".
 */  
int
kprobe(probe_t *p)
{
	/* TEMPORARY:  We can't fault here because this isn't pinned code. */
	assert(CAN_FAULT);

	/* We may or may not be able to page fault. */
	if (CAN_FAULT) return(pagable_probe(p,FROM_K));
	else return(unpagable_probe(p));
}

/*
 * When the issuer can page fault, the probe data must be copied
 * into a pinned buffer before doing the errput.
 * Note that different copying routines are used if called from
 * an application or the kernel.
 * This routine allocates the buffer, sets up the transfer vector
 * and calls probe_do.
 *
 * This routine is called from probe() and kprobe() where the
 * caller can page fault.
 *
 * INPUT:
 *	A pointer to a probe_rec structure.
 *	A flag indicating whether called from from an application or not.
 *
 * RETURNS:
 *	0 - Successful
 *	EINVAL - invalid data
 *	EFAULT - a bad address was passed.
 *	EAGAIN - Memory wasn't available.
 *
 */
static int
pagable_probe(probe_t *p, int orig_f)
{
	int rv;
	char *buf;
	struct xfer x, *xfer = &x;
	/*
	 * We allocate one vector element because everything must
	 * be copied to a buffer anyway.
	 */
#define VECSIZE sizeof(struct errvec)
#define MEMSIZE (EREC_MAX+VECSIZE)
	struct errvec *evp;		/* error vector for errput */

	/* This buffer must be able to hold symptom and error log data */
	if ((buf = xmalloc(MEMSIZE,BPWSHIFT,pinned_heap)) == NULL) {
		/* Couldn't get the space. */
		return(EAGAIN);
	}
	evp = (struct errvec *)(buf+EREC_MAX);	/* Point to address vec */

	/* Setup for an application probe. */
	bzero(&x,sizeof(x));
	bzero(evp,VECSIZE);
	if (orig_f == FROM_APP) {
		x.copydata = copydata_app;
		x.copy_to_errvec = copyin_app;
		x.copystr_to_errvec = copyinstr_app;
	}
	else {	/* Called from kprobe() */
		x.copydata = copydata_k; /* No special pagable routine here */
		x.copy_to_errvec = copyin_pagable_k;
		x.copystr_to_errvec = copyinstr_pagable_k;
	}
	x.copymine_to_errvec = copy_pagable_local;
	x.close_errvec = close_app;
	EV = evp;		/* Point to this errvec structure. */
	EVP = &EV->v[0];	/*   pt. to the vector elements */
	EVN = 1;		/* Set # elements in vector */
	CRNTP = BUFP = buf;	/* pt the one element to the buffer. */

	rv = probe_do(p,xfer);

	/* Free the pinned buffer */
	xmfree(buf,pinned_heap);

	return(rv);
}

/*
 * When called from a routine that can not page fault, we don't need
 * to copy the data, rather we can assume it's pinned, or the caller
 * has a bug.  Therefore, point to the data rather than copying it.
 *
 * This is always called from kprobe (kernel environment).
 *
 * INPUT:
 *	A pointer to a probe_rec structure.
 *	A flag indicating whether called from from an application or not.
 *
 * RETURNS:
 *	0 - Successful
 *	EINVAL - invalid data
 *	EFAULT - a bad address was passed.
 */
static int
unpagable_probe(probe_t *p)
{
	/* The buffer must include space for the address vector used
	 * by the errput... routines.
	 * I needs to point to erec0, err_rec, sympt_data0, and symptom data.
	 * Note that each keyword takes 2 vector elements, 1 for the
	 * keyword and 1 for the data.
	 */
	char buf[sizeof(struct errvec) + (2*SSKWD_MAX+3)*sizeof(struct avec)];
	struct xfer x, *xfer = &x;

	/* Setup for an application probe. */
	bzero(&x,sizeof(x));
	bzero(buf,sizeof(buf));
	x.copydata = copydata_k;
	x.copy_to_errvec = copyin_k;
	x.copystr_to_errvec = copyinstr_k;
	x.copymine_to_errvec = copy_local;
	x.close_errvec = close_k;
	/* Setup the address vector */
	EV = (struct errvec *)buf; /* Use the buffer */
	EVP = &EV->v[0];	/* pt to the address vector */
	EVN = 1;		/* Set # elements */

	return(probe_do(p,xfer));
}

/*
 * NAME: probe_do()
 *                                                                    
 * FUNCTION: Handle probe statements.  Build an erec and hand it to errput().
 *	Validate the data.
 *
 * ENVIRONMENT:
 *	Called internally from probe() and kprobe()
 *                                                                    
 * INPUT:
 *	pointer to the probe data.
 *	Pointer to a transfer address vector (struct xfer).
 *
 * RETURNS:
 *	0 - Successful
 *	EINVAL - invalid data
 *	EFAULT - a bad address was passed.
 */  
static int
probe_do(probe_t *p,struct xfer *xfer)
{
	struct probe_rec0 pr0;		/* Probe header */
	struct erec0 e;			/* Main error header */
	struct erec *ep;		/* Ptr to complete error record */
	struct sympt_data0 sy0;		/* Symptom data header */
	struct err_rec0 *errp;
	struct err_rec0 err_rec =
		{0,"SYSERRLG"};		/* Used if need to generate an error */
	int rv = 0;			/* Return value */
	int i;
	uint len = SSDATA_MAX;		/* Max symp data length.
					 * Contains actual length later. */

	/* Copy the probe header to local storage for now. */
	if (rv=(*xfer->copydata)(p,&pr0,sizeof(pr0))) return(rv);

	/* Validate the header and setup sympt_data0. */
	if (rv=check_prec0(&pr0,&sy0)) return(rv);

	/* Quit if there's no data. */
	if (!pr0.erecl && !pr0.nsskwd) return(0);

	/* At this point we know we have a valid probe header,
	 * and that we have some error log or symptom data.
	 */
	/* Setup the error header (erec0) structure. */
	(*xfer->copymine_to_errvec)(&e,xfer,sizeof(e),&ep);

	/* Process user's error log entry, or use our own if none. 
	 * We'll fill in the error (SW or HW) later if it's our own.
	 */
	if (pr0.erecl) {
		/* User-supplied error log entry */
		ep->erec_rec_len = pr0.erecl;
		/* Get User's error id */
		if (rv=(*xfer->copydata)(&pr0.erecp->error_id,&EVE,sizeof(EVE)))
			return(rv);
		/* Get User's error log data.*/
		if (rv = (*xfer->copy_to_errvec)(pr0.erecp,xfer,pr0.erecl))
			return(rv);
	}
	else {
		/* Put in our own (default) error. */
		/* errp will point to the error if we need to change
		 * the error id */
		(*xfer->copymine_to_errvec)(&err_rec,xfer,sizeof(err_rec),&errp);
		ep->erec_rec_len = sizeof(err_rec);
	}

	if (pr0.nsskwd) {
		/* Symptom data is present */
		i = DATAL;		/* Save for erec_symp_len calculation */
		(*xfer->copymine_to_errvec)(&sy0,xfer,sizeof(sy0),&xfer->syp);
		/* Validate and copy the user's symptom data. */
		rv = check_and_copy_ssdata(&p->sskwds[0],xfer,pr0.nsskwd,len);
		if (rv) return(rv);	/* Quit on error */
		/* Setup the symptom data's length in the erec0 header */
		ep->erec_symp_len = DATAL-i;
	}
	else {	/* No symptom data. */
		ep->erec_symp_len = 0;
	}

	/* Fill in the error if we generated our own log entry */
	if (!pr0.erecl) {
		/* Get error set by check_and_copy_ssdata */
		errp->error_id = EVE;
	}

	/* Call error put routine. */
	ep->erec_len = DATAL;
	(*xfer->close_errvec)(xfer);
	errput(xfer->errvec,DATAL);
	return(0);
}

/*
 * check the probe header and fill in the sympt_data0 structure.
 * Return EFAULT on error.
 */
static int
check_prec0(struct probe_rec0 *p, struct sympt_data0 *s)
{
	/* Check magic # */
	if (!((p->probe_magic == SYSPROBE_MAGIC) ||
	      (p->probe_magic == CUSTPROBE_MAGIC)))
		return(EINVAL);
	
	/* Truncate error log data if too long. */
	if (p->erecl > ERR_REC_MAX_SIZE)
		p->erecl = ERR_REC_MAX_SIZE;
	
	/* Check the flags. */
	if ((p->flags & ~SSFLAGS_MASK) != 0)
		return(EINVAL);
	
	/* Check the # keywords */
	if ((p->nsskwd > SSKWD_MAX) || (p->nsskwd < 0))
		return(EINVAL);
	
	/* Fill in sympt_data0 */
	s->probe_magic = p->probe_magic;
	s->flags = p->flags;
	s->nsskwd = p->nsskwd;

	return(0);
}

/*
 * Validate and copy symptom string data
 */
static int
check_and_copy_ssdata(struct sskwd *in,struct xfer *xfer,int nkwds,int lenleft)
{
	int i, max, rv, len, kwd, sev;
	/* Presents flags */
	int pids=0, lvls=0, pcss=0;
	int numsymp=0;

	/* Default error is software error. */
	EVE = ERRID_SOFTWARE_SYMPTOM;

	/* For each keyword */
	for (; nkwds; nkwds--, in++) {
		/* Quit if not enough buffer space. */
		if ((lenleft-=sizeof(in->sskwd_id))<0) return(EINVAL);

		/* Copy in the keyword, exit if error. */
		if (rv=(*xfer->copy_to_errvec)(&in->sskwd_id,xfer,sizeof(in->sskwd_id)))
			return(rv);

		/* Search the keyword table for a match. */
		for (i=0, kwd=*(int *)DATAP;
		     kwdtbl[i].kwd && (kwdtbl[i].kwd != kwd); i++);
		if (!kwdtbl[i].kwd) return(EINVAL);	/* Not found */

		/* At this point we know we have a valid keyword
		 * and room to put it in the buffer.
		 */

		/* Copy the data into *out */
		if (kwdtbl[i].type == DT_STR) {
			/* It's a string (be careful of the length!) */
			/* Get the length and allow for the trailing '\0' */
			max = kwdtbl[i].len + 1;
			/* the max is also governed by what space is left. */
			if (max > lenleft) max = lenleft;
			/* If no room, we quit. */
			if (max == 0) return(EINVAL);

			/* Copy the string */
			rv = (*xfer->copystr_to_errvec)(in->SSKWD_PTR,xfer,max,&len);
			/* Error if null string data, len == 1 */
			if (len == 1) return(EINVAL);
			if (rv == E2BIG) return(EINVAL);
			else if (rv) return(EFAULT);	/* Copy error */
			/* Decrease the space left. */
			lenleft -= len;
		}
		else {
			/* Not a string. */
			if ((lenleft-=sizeof(in->SSKWD_VAL))<0) return(EINVAL);
			if (rv=(*xfer->copy_to_errvec)(&in->SSKWD_VAL,xfer,sizeof(in->SSKWD_VAL)))
				return(rv);
		}

		/* The keyword's data has been copied ok. */

		/* If the keyword is part of a symptom string, there
		 * may be only be MAX_SYMPTOM_KWDS of them.
		 * Also, string data may not contain imbeded blanks.
		 */
		if (kwdtbl[i].flags & SYMP) {
			/* Check for too many symptom keywords. */
			if (++numsymp > MAX_SYMPTOM_KWDS) return(EINVAL);
			/* Can't be imbeded blanks in string data. */
			if ((kwdtbl[i].type == DT_STR) && strchr(DATAP,' '))
				return(EINVAL);
		}

		/* Check severity value. */
		if (kwd==SSKWD_SEV) {
			/* Make a copy to insure word allignment. */
			bcopy(DATAP,&sev,sizeof(int));
			if ((sev<1) || (sev>4)) return(EINVAL);
		}

		/* Set presents flags */
		if (kwd==SSKWD_PIDS) pids = 1;
		else if (kwd==SSKWD_LVLS) lvls = 1;
		else if (kwd==SSKWD_PCSS) pcss = 1;
		else if (kwd==SSKWD_SRN) EVE = ERRID_HARDWARE_SYMPTOM;
	} /* end of the for loop (for each keyword) */

	/* All keywords have been copied and data validated. */

	/* Make sure required keywords are present.
	 * The probe id, pcss, is always required.
	 * For internal code, pids and lvls are also required.
	 */
	if (!pcss) return(EINVAL);
	if ((MAGIC==SYSPROBE_MAGIC) && !(pids && lvls))
		return(EINVAL);

	return(0);
}

/*
 * Copy data from "s" to "t" for an application.
 */
static int
copydata_app(char *s,char *t,int len)
{
	return((copyin(s,t,len)!=0)?EFAULT:0);
}

/*
 * Copy data from "s" to "t" for a Kernel routine.
 */
static int
copydata_k(char *s,char *t,int len)
{
	bcopy(s,t,len);
	return(0);
}

/*
 * Copy data using copyin and update an address vector.
 */
static int
copyin_app(char *s, struct xfer *xfer, int len)
{
	/* Pt to the place the data will go. */
	DATAP = BUFP;
	/* Copy the data to the current position. */
	if (copyin(s,BUFP,len)) return(EFAULT);
	/* Update the length in the vector */
	CRNTL += len;
	/* Update the overall length */
	DATAL += len;
	/* Pt. to the next buffer position. */
	BUFP += len;
	return(0);
}

/*
 * Copy data using bcopy and update an address vector.
 * A bad address will abend the system.
 * This works just like copy_pagable_app above.
 */
static int
copyin_pagable_k(char *s, struct xfer *xfer, int len)
{
	DATAP = BUFP;
	bcopy(s,BUFP,len);
	CRNTL += len;
	DATAL += len;
	BUFP += len;
	return(0);
}

/*
 * Logically Copy data from pinned Kernel space.
 * Actually just updates an address vector.
 */
static int
copyin_k(char *s, struct xfer *xfer, int len)
{
	/* Pt the vector at the data. */
	DATAP = CRNTP = s;
	/* Put it's length in the vector */
	CRNTL = len;
	/* Add to the total length. */
	DATAL += len;
	/* Go to the next vector element. */
	NEXTVEC;	/* Go to the next vector element */
	return(0);
}

/*
 * If local (on-stack) data is used when we can fault, copy it as if 
 * in a pagable Kernel environment.
 * This routine is used to copy our own data (copymine_to_errvec).
 */
static int
copy_pagable_local(char *s, struct xfer *xfer, int len,void **p)
{
	/* If the copy fails, our own address space is bad! */
	assert(copyin_pagable_k(s,xfer,len)==0);
	*p = DATAP;
}

/*
 * If local (on-stack) data, and we can't fault, then the stack
 * is pinned and we'll behave lkke a disable kernel routine.
 */
static int
copy_local(char *s, struct xfer *xfer, int len,void **p)
{
	copyin_k(s,xfer,len);
	*p = DATAP;
	return(0);
}

/*
 * Copy in a string from application's memory and
 * update the address vector.
 * This works just like copyin_app above.
 */
static int
copyinstr_app(char *s, struct xfer *xfer, int max, int *lenp)
{
	int rv;

	DATAP = BUFP;
	rv = copyinstr(s,BUFP,max,lenp);
	CRNTL += *lenp;
	DATAL += *lenp;
	BUFP += *lenp;
	return(rv);
}

/*
 * Copy in a string from kernel memory and
 * update the address vector.
 * This works like copyin_pagable_k above.
 */
static int
copyinstr_pagable_k(char *s, struct xfer *xfer, int max, int *lenp)
{
	int rv;

	DATAP = BUFP;
	rv = copystr(s,BUFP,max,lenp);
	CRNTL += *lenp;
	DATAL += *lenp;
	BUFP += *lenp;
	return(rv);
}

/*
 * Logically copy in a string from Kernel space.
 * Actually figures out the length and updates an address vector.
 * This handles the address vector like copyin_k above.
 */
static int
copyinstr_k(char *s, struct xfer *xfer, int max, int *lenp)
{
	int rv = 0;

	DATAP = CRNTP = s;
	/* Get the length */
	for (*lenp=1; (*s!='\0') && (*lenp<=max); s++, (*lenp)++);
	/* Too big if we ran out of bytes but not out of string. */
	if ((*lenp==max) && *s) rv = E2BIG;
	CRNTL = *lenp;
	DATAL += *lenp;
	NEXTVEC;
	return(rv);
}

/*
 * Close the address vector for a probe call.
 */
static void
close_app(struct xfer *xfer)
{
}

/*
 * Close address vector for a kprobe call.
 * If we're sitting at a zero-length vector element, don't include it.
 */
static void
close_k(struct xfer *xfer)
{
	if (!CRNTL) EVN--;
}
