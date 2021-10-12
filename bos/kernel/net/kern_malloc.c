static char sccsid[] = "@(#)09	1.29  src/bos/kernel/net/kern_malloc.c, sysnet, bos41J, 9511A_all 3/10/95 09:21:04";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: aix_kmem_alloc
 *              aix_kmem_suballoc
 *              kmem_alloc
 *              kmem_free
 *              kmem_suballoc
 *              kmeminit
 *              kmeminit_thread
 *              malloc_loan
 *              malloc_thread
 *              net_free
 *              net_malloc
 *              threadtimer
 *              
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      Base: kern_malloc.c     7.18 (Berkeley) 6/28/90
 */



void threadtimer();
void *  aix_kmem_alloc();
void	prime_the_buckets();
void *	empty_bucket();
void *	aix_kmem_suballoc();
#define kmem_alloc(map, size, cpu)              aix_kmem_alloc(map, size, cpu)
#define kmem_suballoc(map, minp, maxp, size, pageable)  \
                                        aix_kmem_suballoc(minp, maxp, size)
#define kmem_free(map, addr, size)      aix_kmem_free(map, addr, size)

#define thread_set_timeout(tmo)         malloc_thread_set_timeout(tmo)

#define microtime(tv) {                                         \
        struct timestruc_t              ct;                     \
                                                                \
        curtime(&ct);                                           \
        (tv)->tv_sec = (int) ct.tv_sec;                         \
        (tv)->tv_usec = (int) ct.tv_nsec / 1000;                \
}

#define        EVENT_NETMALLOC    0x0912

#define MACH_ASSERT     1       /* enable error checking */

#define MAXINDX 14      /* WARNING! Must change if MAXALLOCSAVE changes */

#include "sys/types.h"
#include "sys/errids.h"
#include "sys/systm.h"
#include "sys/intr.h"
#include "sys/xmalloc.h"
#include "sys/sleep.h"
#include "sys/time.h"
#include "sys/syspest.h"
#include "sys/vmker.h"
#include "net/spl.h"
#include "net/net_globals.h"
#include "net/net_malloc.h"
#include "sys/errno.h"
#include "sys/trchkid.h"
#include "sys/systemcfg.h"

extern struct heap      *pinned_heap;
extern struct heap      *kernel_heap;
extern u_int            mbclusters, mbufs;
extern int              thewall, thewall_dflt;
int                     allocated;
extern struct xmem      mclxmemd;
simple_lock_data_t      alloc_lock;

#include "sys/param.h"
#include "sys/time.h"

#include "sys/atomic_op.h"	/* for fetch_and_add() */

/*
 * Reference:
 *   "Design of a General Purpose Memory Allocator for the 4.3BSD UNIX Kernel"
 *      Summer Usenix '88 proceedings pp 295-301.
 *
 * OSF enhanced to include
 *      Parallelization
 *      Asynchronous allocation and freeing of memory buckets
 *      Borrowing of memory for small allocations from larger buckets
 *      Garbage collection and freeing of "extra" memory
 *      Tunable
 * AIX enhancements include:
 *      FOR UP only:
 *              Serialization done via disable_ints() and enable_ints().  
 *              Reduced granularity to:
 *                      serialize the mallod_thread() code.
 *                      serialize all bucket access, 
 *                              all usage structs, and all kmemstats stuff.
 *      Thewall enforcement (with errlogging when thewall is exceeded).
 *      Initial pin of memory based on actual memory size.
 *      Thewall initial value based on actual memory size.
 *      
 */

#ifdef _POWER_MP

/* 
 * For SMP systems, disabling the current CPU is done explicitly via splhigh()
 * as was done by OSF.  Thus, these lock defines only grab the locks. 
 */
#define MALLOCTHREAD_LOCK(s)    simple_lock(&bucket[0][0].kb_lock);
#define MALLOCTHREAD_UNLOCK(s)  simple_unlock(&bucket[0][0].kb_lock);
#define KBP_LOCK(kbp)           simple_lock(&kbp->kb_lock);     
#define KBP_UNLOCK(kbp)         simple_unlock(&kbp->kb_lock);   
#else
#define MALLOCTHREAD_LOCK(s)    s = disable_ints();
#define MALLOCTHREAD_UNLOCK(s)  enable_ints(s);
#define BUCKET_LOCK(s)          s = disable_ints();
#define BUCKET_UNLOCK(s)        enable_ints(s);
#define FAST_BUCKET_LOCK(s)     s = disable_ints();
#define FAST_BUCKET_UNLOCK(s)   enable_ints(s);
#endif

struct kmemstats kmemstats[M_LAST];
const char kmemnames[M_LAST][KMEMNAMSZ] = INITKMEMNAMES;


/*
 * bucket is a 2 dimensional array creating one bucket array structure
 * for each CPU on the system...
 */
struct kmembuckets bucket[MAXCPU][MINBUCKET + 16];
struct kmemusage *kmemusage;
void *kmembase, *kmemlimit;
void *kmemfreelater;
long wantkmemmap;
vm_map_t kmemmap;
thread_t kmemthread;

/*
 * Tuning parameters.
 *      kmempages       Size (in pages) of malloc map. Configurable only
 *                      at startup, unused after.
 *      kmemreserve     The malloc reserve is some number of pages which
 *                      are held as some number of elements in the largest
 *                      bucket. This must not be larger than the bucket's
 *                      highwater. This is verified at startup, but not if
 *                      changed later.
 *      kmemgcintvl     Frequency in seconds of "normal" garbage collection
 *                      passes. Settable to 0 to disable all gc's.
 *      kmemgcscale     Scaling factor for gc aggressiveness. Any overage
 *                      detected in gc is reduced linearly by this slope
 *                      until it reaches 0, then constantly until done.
 */
int kmempages = 0;
int kmemreserve;        /* set in kmeminit() */
int kmemgcintvl = 2;
int kmemgcscale = 64;

/*
 * RESERVE is the number of 16K chunks to keep in each 16K bucket...
 */
#define RESERVE         ((kmemreserve * PAGE_SIZE) / MAXALLOCSAVE)
static void *malloc_loan(u_long, int, int);
static long pagindx, maxindx;

int nrpages;

/*
 * Below is code to help debug net_malloc() abusers.  These are kernel
 * modules and extensions who free memory twice, pass in bogus addresses,
 * muck with memory after freeing it, etc...
 * 
 * The main check here is to verify at net_free() time, that a chunk 
 * being freed hasn't already been freed.  This is non trivial for 
 * chunks < 4K.  This is implemented by a 4 word bit array in a struct
 * parallel to the  kmem usage struct.  The new struct, called police_usage
 * gets allocated when a user turns on net_malloc_police via the no command.
 * This bit array has one position for each chunk that makes
 * up the page in question. When a chunk from the page is allocated via
 * net_malloc(), the corresponding bit is reset in the bit array for that
 * page.  When the net_free() for that chunk happens, the bit array is 
 * checked to ensure that the chunk is really allocated.  If not, then
 * we assert, else we turn on the appropriate bit.  We handle buckets 
 * from 32B to 2K.
 */

/*
 * net_malloc_police is a runtime boolean that enables/disables policing
 * net_malloc()/net_free() users and abusers.  Settable via the no command.
 */
int     net_malloc_police_dflt = 0;
int     net_malloc_police = 0;

/*
 * Part of policing the users is to keep a list of past malloc/frees.
 * Memory for police_events is allocated when net_malloc_police is turned
 * on via the no command.  The array consists of this struct:
 */
struct police_event {
        char    bogus;                  /* Alignment is good. */
        char    type;                   /* 'M'=net_malloc, 'F'=net_free */
        short   size;                   /* Size being allocated */
        void    *addr;                  /* Memory address of chunk 'o mem */
        void    *caller1;               /* caller of net_malloc/net_free */
        void    *caller2;               /* caller's caller */
};

struct police_event     *police_events;
int                     police_index;
int                     police_event_size;

simple_lock_data_t              police_lock;
#define POLICE_LOCK(s)          s = disable_lock(INTMAX, &police_lock);
#define POLICE_UNLOCK(s)        unlock_enable(s, &police_lock);

struct police_usage {
        ulong   pu_freebits[4];
};

#define btopup(addr)    \
        (&police_usage[((char *)(addr) - (char *)kmembase) / PAGE_SIZE])

struct police_usage     *police_usage;

/*
 * Assumes the bucket lock is held.
 */
void
log_police_event(type, size, addr, caller1, caller2)
char    type;
short   size;
void    *addr;
void    *caller1;
void    *caller2;
{
        if (type == 'M')
                TRCHKL4T(HKWD_NET_MALLOC | hkwd_net_malloc, size, addr, \
                        caller1, caller2);      
        else
                TRCHKL4T(HKWD_NET_MALLOC | hkwd_net_free, size, addr, \
                        caller1, caller2);      
        police_events[police_index].type = type;
        police_events[police_index].size = size;
        police_events[police_index].addr = addr;
        police_events[police_index].caller1 = caller1;
        police_events[police_index].caller2 = caller2;
        if (++police_index == police_event_size)
                police_index = 0;
}

/*
 * Net_malloc_police_station gets called when a user changes the 
 * net_malloc_police no option.  The user calls the police station
 * to start or stop policing of the malloc users.  Get it?  ar ar...
 */
net_malloc_police_station(start_policing, nop)
int             *start_policing;
struct netopt   *nop;
{
        int size;
        int indx;
        int s;

        if (*start_policing) {

                /*
                 * If this is our first policing, then allocate all the
                 * needed memory.
                 */
                size = round_page(sizeof (struct police_usage) * kmempages);
                if (!police_usage) {
                        if ((police_usage = (struct police_usage *)
                                xmalloc(size, PGSHIFT, pinned_heap)) == NULL)
                                return(ENOBUFS);
                }
                if (!police_events) {

                        /*
                         * The passed in value can affect the event array
                         * size, but keep a minimum of 1024.
                         */
                        police_event_size = MAX(*start_policing, 1024);
                        if ((police_events = (struct police_event *)
                                xmalloc(police_event_size * 
                                        sizeof(struct police_event), PGSHIFT, 
                                        pinned_heap)) == NULL)
                                return(ENOBUFS);
                }
                lock_alloc(&police_lock, LOCK_ALLOC_PIN, KMEMSTAT_LOCK_FAMILY, 
                        M_LAST+2);
                simple_lock_init(&police_lock);

                POLICE_LOCK(s);
                
                /* Clear the event trace array. */
                bzero(police_events, police_event_size * 
                        sizeof(struct police_event));

                /* Mark all buffers as malloced... */
                bzero(police_usage, size);

                police_index = 0;
                POLICE_UNLOCK(s);
        }
        net_malloc_police=*start_policing;
        return(0);
}

/*
 * Police_net_malloc() does some sanity checking and marks the police_usage
 * free bits appropriately.  It also logs the event.
 *
 * Assumes the bucket lock for va is held!.
 */
void
police_net_malloc(va, size, type)
void    *va;
long    size;
int     type;
{
        register struct kmemusage *kup;
        int s;
        

        POLICE_LOCK(s);
        if (kmembase == NULL)
                panic("malloc: not initialized");
        if (type <= 0 || type >= M_LAST)
                panic("malloc: bogus type");

        if (va != NULL) {
                struct police_usage *pup;
                struct kmembuckets *kbp;
                void *addr;

                kup = btokup(va);
                kbp = &bucket[kup->ku_cpu][kup->ku_indx];

                /*
                 * Assert that the next chunk on the free list is reasonable.
                 */
                addr = kbp->kb_next;
                assert ((addr >= kmembase && addr < kmemlimit) || (addr == 0));

                pup = btopup(va);
                size = 1 << kup->ku_indx;
                if (size < PAGESIZE) {
                        caddr_t paddr;
                        int i;
                        int elmnum;
                        ulong bit;

                        paddr = (caddr_t) ((int)va & ~(PAGESIZE-1));
                        elmnum = ((caddr_t)va - paddr) / size;
                        bit = elmnum % (sizeof(ulong)*NBPB);
                        i = elmnum / (sizeof(ulong)*NBPB);
                        pup->pu_freebits[i] &= ~(1<<bit);
                }
                log_police_event('M', size, va, getcaller2(), getcaller3());
        }
        POLICE_UNLOCK(s);
}

static const u_long addrmask[] = { 0x00000000,
        0x00000001, 0x00000003, 0x00000007, 0x0000000f,
        0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
        0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
        0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff
};

/*
 * Police_net_free() does some sanity checking and verifies via the police_usage
 * bit array that the chunk being freed hasn't already been freed.
 * If so, it asserts, else it sets the freebits.
 * Also logs the event.
 *
 * Assumes the bucket lock is held.
 */
void
police_net_free(addr, size, type)
void    *addr;
long    size;
int     type;
{
        register struct kmemusage *kup;
        long alloc;
        int s;

        POLICE_LOCK(s);
        log_police_event('F', size, addr, getcaller2(), getcaller3());

        kup = btokup(addr);

        if (type <= 0 || type >= M_LAST)
                panic("free: bogus type");
        if (kup->ku_indx < MINBUCKET || kup->ku_indx >= MINBUCKET+16)
                panic("free: bad indx");
        if (size >= PAGE_SIZE)
                alloc = addrmask[pagindx];
        else if (kup->ku_indx < sizeof addrmask / sizeof addrmask[0])
                alloc = addrmask[kup->ku_indx];
        else
                panic("free: humungous PAGE_SIZE");
        if (((u_long)addr & alloc) != 0) {
                printf("free: unaligned addr 0x%x, size %d, type %d, mask %x\n",
                        addr, size, type, alloc);
                panic("free: unaligned addr");
        }
        if (size < PAGESIZE) {
                struct police_usage *pup;
                caddr_t paddr;
                u_long bit;
                int elmnum, i;

                pup = btopup(addr);
                paddr = (caddr_t)((int)addr & ~(PAGESIZE-1));
                elmnum = ((caddr_t)addr - paddr) / size;
                bit = elmnum % (sizeof(ulong)*NBPB);
                i = elmnum / (sizeof(ulong)*NBPB);
                assert(!(pup->pu_freebits[i] & (1<<bit)));
                pup->pu_freebits[i] |= (1<<bit);
        }
        POLICE_UNLOCK(s);
}

/*
 * Allocate a block of memory
 */
void *
net_malloc(     u_long  size,
        int     type,
        int     flags)
{
        register struct kmembuckets *kbp;
        register struct kmemusage *kup;
        long indx;
        int s;
        void *va;
        register struct kmemstats *ksp = &kmemstats[type];
        int cpu;

        indx = BUCKETINDX(size);
#ifdef  _POWER_MP
        s = splhigh();
#endif
        cpu = mycpu();
        kbp = &bucket[cpu][indx];

#ifdef  _POWER_MP
        KBP_LOCK(kbp);
#else
        FAST_BUCKET_LOCK(s);
#endif
        if ((va = kbp->kb_next) == NULL) {
#ifdef  _POWER_MP
                KBP_UNLOCK(kbp);
                splx(s);
#else
                FAST_BUCKET_UNLOCK(s);
#endif
                va = empty_bucket(size, indx, flags, ksp, cpu);
                if (net_malloc_police) {
                        if (va) {
#ifdef _POWER_MP
                                s = splhigh();
                                KBP_LOCK(kbp);
#else
                                FAST_BUCKET_LOCK(s);
#endif
                                police_net_malloc(va, size, type);
#ifdef _POWER_MP
                                KBP_UNLOCK(kbp);
                                splx(s);
#else
                                FAST_BUCKET_UNLOCK(s);
#endif
                        }
                }
                return(va);
        }

        kbp->kb_next = *(void **)va;
        kup = btokup(va);
#ifdef	DEBUG
        if (kup->ku_indx != indx)
                panic("net_malloc: wrong bucket");
        if (kup->ku_freecnt <= 0)
                panic("net_malloc: lost data");
        if (kup->ku_cpu != cpu)
                panic("net_malloc: bad usage struct");
#endif	/* DEBUG */

        if (net_malloc_police) {
                police_net_malloc(va, size, type);
        }
        kup->ku_freecnt--;
        kbp->kb_totalfree--;
        kbp->kb_calls++;
        if (indx == MAXINDX && kbp->kb_totalfree < RESERVE)
                et_post(EVENT_NETMALLOC, kmemthread->t_tid);
#ifdef  _POWER_MP
        KBP_UNLOCK(kbp);
#endif
	fetch_and_add(&ksp->ks_memuse, 1 << indx);
out:
	fetch_and_add(&ksp->ks_inuse, 1);
	fetch_and_add(&ksp->ks_calls, 1);
        if (ksp->ks_memuse > ksp->ks_maxused)
                ksp->ks_maxused = ksp->ks_memuse;

#ifdef  _POWER_MP
        splx(s);
#else
        FAST_BUCKET_UNLOCK(s);
#endif
        return va;
}

void *
empty_bucket(size, indx, flags, ksp, cpu) 
u_long size;
long indx;
int flags;
struct kmemstats *ksp;
int cpu;
{
        register struct kmembuckets *kbp;
        register struct kmemusage *kup;
        long allocsize;
        int s, s1;
        void *va, *cp, *savedlist;

        kbp = &bucket[cpu][indx];
#ifdef  _POWER_MP
        s = splhigh();
again:
        KBP_LOCK(kbp);
#else
again:
        BUCKET_LOCK(s);
#endif
        if (kbp->kb_next == NULL) {
#ifdef  _POWER_MP
                KBP_UNLOCK(kbp);
#endif
                if (size > MAXALLOCSAVE)
                        allocsize = round_page(size);
                else
                        allocsize = 1 << indx;
                size = round_page(allocsize);
                if ((va = malloc_loan(size, flags, cpu)) == NULL) {

                        /*
                         * Failed once at least.  Each time we fail, double
                         * kmemreserve until it reaches the highwat mark of 
                         * the MAXALLOCSAVE bucket...
                         *
                         */
                        kmemreserve = MIN(kmemreserve*2, 
                                (bucket[cpu][ maxindx].kb_highwat * 
                                MAXALLOCSAVE / PAGE_SIZE));
                        if (flags & M_NOWAIT) {
                                if (allocsize <= MAXALLOCSAVE)
                                  STATS_ACTION(&kbp->kb_lock, kbp->kb_failed++);
                                STATS_ACTION(&ksp->ks_lock, ksp->ks_failed++);
#ifdef  _POWER_MP
				splx(s);
#else
                                BUCKET_UNLOCK(s);
#endif
                                return (0);
                        }
#ifdef  _POWER_MP
                        splx(s);
#else
                        BUCKET_UNLOCK(s);
#endif
                        va = (void *)kmem_alloc(kmemmap, size, cpu);
#ifdef  _POWER_MP
                        s = splhigh();
#else
                        BUCKET_LOCK(s);
#endif
                        if (va == NULL) {
                                STATS_ACTION(&ksp->ks_lock,ksp->ks_mapblocks++);
                                MALLOCTHREAD_LOCK(s1);
                                wantkmemmap |= (1 << indx);
                                assert_wait_mesg((int)&wantkmemmap, 
                                                FALSE, "memory");
                                MALLOCTHREAD_UNLOCK(s1);
#ifndef _POWER_MP
                                BUCKET_UNLOCK(s);
#endif
                                et_post(EVENT_NETMALLOC, kmemthread->t_tid);
                                thread_block();
                                goto again;
                        }
                }
                kup = btokup(va);
                kup->ku_indx = indx;
                if (allocsize > MAXALLOCSAVE) {
                        if ((size = allocsize / PAGE_SIZE) > USHRT_MAX)
                                panic("malloc: allocation too large");
                        kup->ku_pagecnt = size;
			fetch_and_add(&ksp->ks_memuse, allocsize);
                        goto out;
                }
#ifdef  _POWER_MP
                KBP_LOCK(kbp);
#endif
                kup->ku_freecnt = kbp->kb_elmpercl;
                kbp->kb_total += kbp->kb_elmpercl;
                kbp->kb_totalfree += kbp->kb_elmpercl;
                /*
                 * Just in case we blocked while allocating memory,
                 * and someone else also allocated memory for this
                 * bucket, don't assume the list is still empty.
                 */
                savedlist = kbp->kb_next;
                kbp->kb_next = (char *)va + size - allocsize;
                for (cp = kbp->kb_next; cp > va; cp = (char *)cp - allocsize)
                        *(void **)cp = (char *)cp - allocsize;
                *(void **)cp = savedlist;
                /* Note: two locks held. Safe, and more efficient. */
                MALLOCTHREAD_LOCK(s1);
                if (wantkmemmap & (1 << indx)) {
                        wantkmemmap &= ~(1 << indx);
                        MALLOCTHREAD_UNLOCK(s1);
#ifdef  _POWER_MP
                        KBP_UNLOCK(kbp);
#else
                        BUCKET_UNLOCK(s);
#endif
                        thread_wakeup((int)&wantkmemmap);
                        goto again;
                }
                MALLOCTHREAD_UNLOCK(s1);
        }
        va = kbp->kb_next;

        kbp->kb_next = *(void **)va;
        kup = btokup(va);
#ifdef	DEBUG
        if (kup->ku_indx != indx)
                panic("empty_bucket: wrong bucket");
        if (kup->ku_freecnt <= 0)
                panic("empty_bucket: lost data");
        if (kup->ku_cpu != cpu)
                panic("empty_bucket: bad usage struct");
#endif	/* DEBUG */
        kup->ku_freecnt--;
        kbp->kb_totalfree--;
        kbp->kb_calls++;
        if (indx == maxindx && kbp->kb_totalfree < RESERVE)
                et_post(EVENT_NETMALLOC, kmemthread->t_tid);
#ifdef  _POWER_MP
        KBP_UNLOCK(kbp);
#endif
	fetch_and_add(&ksp->ks_memuse, 1 << indx);
out:
	fetch_and_add(&ksp->ks_inuse, 1);
	fetch_and_add(&ksp->ks_calls, 1);
        if (ksp->ks_memuse > ksp->ks_maxused)
                ksp->ks_maxused = ksp->ks_memuse;
#ifdef  _POWER_MP
        splx(s);
#else
        BUCKET_UNLOCK(s);
#endif
        return va;
}


/*
 * Free a block of memory allocated by malloc.
 */
void
net_free(       void *  addr,
        int     type)
{
        register struct kmembuckets *kbp;
        register struct kmemusage *kup;
        long indx, size;
        int s, wake;
        register struct kmemstats *ksp = &kmemstats[type];

        if (addr < kmembase || addr >= kmemlimit)
                panic("free: bad addr");
        kup = btokup(addr);
        indx = kup->ku_indx;
        size = 1 << indx;

#ifndef _POWER_MP
        FAST_BUCKET_LOCK(s);
#else
        s = splhigh();
#endif
        if (size > MAXALLOCSAVE) {
                large_free(addr);
                goto out;
        }
        kbp = &bucket[kup->ku_cpu][indx];
#ifdef  _POWER_MP
        KBP_LOCK(kbp);
#endif

        if (net_malloc_police) {
                police_net_free(addr, size, type);
        }
        *(void **)addr = kbp->kb_next;
        kbp->kb_next = addr;
        kbp->kb_totalfree++;
        kup->ku_freecnt++;

        /*
         * If we're dealing with a bucket < 4K, and there's at least
         * a page worth of elements free, then try and coalesce...
         */
        if ((kup->ku_freecnt >= kbp->kb_elmpercl) &&
            (kbp->kb_elmpercl > 1))
                coalesce(kup, kbp, indx, addr);
#ifdef  _POWER_MP
        else
                KBP_UNLOCK(kbp);
#endif
out:
	fetch_and_add(&ksp->ks_memuse, -size);
	fetch_and_add(&ksp->ks_inuse, -1);
#ifndef  _POWER_MP
        FAST_BUCKET_UNLOCK(s);
#endif
        if (wantkmemmap & (1 << indx))
                thread_wakeup((int)&wantkmemmap);
#ifdef  _POWER_MP
        splx(s);
#endif
}

/*
 * MP: assume we're at splhigh()...
 */
large_free(addr)
void * addr;
{
        long alloc;
        long size;
        int s1;
        register struct kmemusage *kup;

        kup = btokup(addr);
        size = kup->ku_pagecnt * PAGE_SIZE;
#if     DEBUG
        for (alloc = kup->ku_pagecnt; --alloc >= 0; ) {
                if (alloc && kup[alloc].ku_indx >= 0)
                        panic("free: overlap");
        }
#endif
        /*
         * Cannot call kmem_free in interrupt context,
         * so play it safe and let kmemthread do it.
        kmem_free(kmemmap, (vm_offset_t)addr, size);
         */
	*(int *)((void **)addr + 1) = size;
        MALLOCTHREAD_LOCK(s1);
        *(void **)addr = kmemfreelater;
        kmemfreelater = addr;
        MALLOCTHREAD_UNLOCK(s1);
        if (kmemthread)
                et_post(EVENT_NETMALLOC, kmemthread->t_tid);
}


/*
 * MP assume bucket lock for kbp is held.  Will be released before 
 * returning from coalesce...
 */
coalesce(kup, kbp, indx, addr)
struct kmembuckets *kbp;
struct kmemusage *kup;
long indx;
void * addr;
{

        if (kup->ku_freecnt > kbp->kb_elmpercl)
                panic("free: multiple frees");

        if (kbp->kb_totalfree > kbp->kb_highwat) {

                /*
                 * Bump small, free buckets up to pages.
                 */
                register void *q, *p = kbp->kb_next;
                register void **pp = &kbp->kb_next;
                register struct kmembuckets *pkbp = 
                        &bucket[kup->ku_cpu][pagindx];
                do {
                        q = *(void **)p;
                        if (btokup(p) == kup)
                                *pp = q;
                        else
                                pp = (void **)p;
                } while (p = q);
                kbp->kb_couldfree++;
                kbp->kb_total -= kbp->kb_elmpercl;
                kbp->kb_totalfree -= kbp->kb_elmpercl;
                kup->ku_indx = indx = pagindx;
                kup->ku_freecnt = 1;
#ifdef  _POWER_MP
                KBP_UNLOCK(kbp);
#endif
                /* Add page to page-size bucket */
                addr = (void *) trunc_page(addr);
#ifdef  _POWER_MP
                KBP_LOCK(pkbp);
#endif
                *(void **)addr = pkbp->kb_next;
                pkbp->kb_next = addr;
                pkbp->kb_total++;
                pkbp->kb_totalfree++;
#ifdef  _POWER_MP
                KBP_UNLOCK(pkbp);
#endif
        }
#ifdef  _POWER_MP
        else
                KBP_UNLOCK(kbp);
#endif
}

/*
 * When smaller bucket is empty, borrow from larger.
 * Called at splhigh, with size rounded to page. If
 * possible, don't loan out the reserve to allocations
 * which can wait.
 */
static void *
malloc_loan(
        u_long  size,
        int     flags,
	int	cpu)
{
        register struct kmembuckets *kbp;
        register struct kmemusage *kup;
        register long newsize;
        void *va = 0;

        kbp = &bucket[cpu][ BUCKETINDX(size)];
        for (newsize = size; newsize <= MAXALLOCSAVE; newsize <<= 1) {
#ifdef  _POWER_MP
                KBP_LOCK(kbp);
#endif
                if (!(flags & M_NOWAIT) && newsize >= MAXALLOCSAVE &&
                     kbp->kb_totalfree <= RESERVE) {
#ifdef  _POWER_MP
                        KBP_UNLOCK(kbp);
#endif
                        return 0;
                }
                if (va = kbp->kb_next) {
                        kbp->kb_next = *(void **)va;
                        kbp->kb_total--;
                        kbp->kb_totalfree--;
                }
#ifdef  _POWER_MP
                KBP_UNLOCK(kbp);
#endif
                if (va)
                        break;
                ++kbp;
        }
        if (newsize >= MAXALLOCSAVE && kmemthread)
                et_post(EVENT_NETMALLOC, kmemthread->t_tid);

        if (va && newsize != size) {
                void *nva = (char *)va + size;
                /* For simplicity, toss on as pages */
                kbp = &bucket[cpu][ pagindx];
#ifdef  _POWER_MP
                KBP_LOCK(kbp);
#endif
                do {
                        kup = btokup(nva);
                        kup->ku_indx = pagindx;
                        kup->ku_freecnt = 1;
                        kbp->kb_total++;
                        kbp->kb_totalfree++;
                        *(void **)nva = kbp->kb_next;
                        kbp->kb_next = nva;
                        newsize -= PAGE_SIZE;
                        nva = (char *)nva + PAGE_SIZE;
                } while (newsize != size);
#ifdef  _POWER_MP
                KBP_UNLOCK(kbp);
#endif
        }
        return va;
}

/*
 * Service thread for malloc/free.
 */
static void
malloc_thread(void)
{
        struct kmembuckets *kbp, *mkbp;
        register struct kmemusage *kup;
        struct timeval now, lastgc;
        vm_offset_t addr;
        long indx;
        void *va;
        int s, i, n, tmo = 0;
        int s1;
        int cpu;

        spl0();
        microtime(&lastgc);
        for (;;) {
                if (tmo) {
                        if (tmo > 0) thread_set_timeout(tmo);
                        et_wait(EVENT_NETMALLOC, EVENT_NETMALLOC, EVENT_SHORT);
                }
                /*
                 * Zeroth job: perform delayed frees of large allocations.
                 */
#ifdef  _POWER_MP
                s1 = splhigh();
#endif
                MALLOCTHREAD_LOCK(s1);
                va = kmemfreelater;
                kmemfreelater = 0;
                if (va && (i = wantkmemmap))
                        wantkmemmap = 0;
                else
                        i = 0;
                MALLOCTHREAD_UNLOCK(s1);
#ifdef  _POWER_MP
                splx(s1);
#endif
                while (va) {
                        addr = (vm_offset_t)va;
                        indx = *(int *)((void **)va + 1);       /* size */
                        va = *(void **)va;
                        kmem_free(kmemmap, addr, indx);
                }
                if (i)
                        thread_wakeup((int)&wantkmemmap);
                /*
                 * First job: minimize probability of malloc(M_NOWAIT)
                 * returning NULL by keeping its biggest bucket full.
                 * Note if MAXALLOCSAVE is much greater than PAGE_SIZE,
                 * fragmentation may result, there is a limit enforced
                 * in kmeminit() below which attempts to minimize this.
                 */
        
                for (cpu=0; cpu < _system_configuration.ncpus; cpu++) {
        
                        mkbp = &bucket[cpu][ maxindx];
                        i = RESERVE;            /* target */
#ifdef  _POWER_MP
                        s = splhigh();
                        KBP_LOCK(mkbp);
#else
                        BUCKET_LOCK(s);
#endif
                        i -= mkbp->kb_totalfree;
#ifdef  _POWER_MP
                        KBP_UNLOCK(mkbp);
                        splx(s);
#else
                        BUCKET_UNLOCK(s);
#endif
                        if (i > 0 || (i==0 && wantkmemmap) ) {
                                if ((va = (void *)kmem_alloc(kmemmap, 
                                        MAXALLOCSAVE, cpu)) != NULL) {
                                        kup = btokup(va);
                                        kup->ku_indx = maxindx;
#ifdef  _POWER_MP
                                        s = splhigh();
                                        KBP_LOCK(mkbp);
#else
                                        BUCKET_LOCK(s);
#endif
                                        kup->ku_freecnt = 1;
                                        mkbp->kb_total++;
                                        mkbp->kb_totalfree++;
                                        *(void **)va = mkbp->kb_next;
                                        mkbp->kb_next = va;
#ifdef  _POWER_MP
                                        KBP_UNLOCK(mkbp);
#else
                                        BUCKET_UNLOCK(s);
#endif
                                        MALLOCTHREAD_LOCK(s1);
                                        i = wantkmemmap;
                                        wantkmemmap = 0;
                                        MALLOCTHREAD_UNLOCK(s1);
#ifdef  _POWER_MP
                                        splx(s);
#endif
                                        if (i)
                                                thread_wakeup((int)&wantkmemmap);
                                        tmo = 0;        /* go around now */
                                        continue;
                                }
                                /*
                                 * If alloc fails, force a gc pass and go around
                                 * in 1/2 sec if we fail to free anything.
                                 */
                                tmo = hz / 2;
                        } else
                                tmo = -1;
                }

                /*
                 * If timeout is -1, then check interval since last gc and
                 *   gc or go back to sleep. Check for time warps.
                 * If timeout < hz, then failure dictates some gc is
                 *   necessary before retry.
                 */
                microtime(&now);
                if (tmo < 0) {
                        if (kmemgcintvl <= 0)
                                continue;
                        tmo = kmemgcintvl - (now.tv_sec - lastgc.tv_sec);
                        if (tmo > 0) {
                                if (tmo > kmemgcintvl)  /* timewarp */
                                        tmo = kmemgcintvl;
                                tmo *= hz;
                                continue;
                        }
                        tmo = kmemgcintvl * hz;
                }
                lastgc = now;

                /*
                 * Second job: garbage collect pages scavenged in free().
                 * Scale aggressiveness in freeing memory to the amount
                 * of overage and settable scale. Currently functional
                 * only if KMEMSTATS.
                 */
                for (cpu=0; cpu < _system_configuration.ncpus; cpu++) {
                        indx = maxindx;
                        mkbp = &bucket[cpu][ maxindx];
                        kbp = mkbp;
                        n = 0;
#ifdef  _POWER_MP
                        s = splhigh();
#else
                        BUCKET_LOCK(s);
#endif
                        do {
#ifdef  _POWER_MP
                                KBP_LOCK(kbp);
#endif
                                i = (kbp->kb_totalfree - kbp->kb_highwat) *
                                        (1 << (indx - pagindx));
                                if (i > 0)
                                        n += i;
#ifdef  _POWER_MP
                                KBP_UNLOCK(kbp);
#endif
                                kbp--;
                                indx--;
                        } while (indx >= pagindx);
#ifdef  _POWER_MP
                        splx(s);
#else
                        BUCKET_UNLOCK(s);
#endif
                        /*
                         * Do linear scaling above "scale" extra pages,
                         * constant scaling below "scale", nothing if none.
                         * Note smaller scale = higher aggressiveness.
                         * Typical value == 8 @ 2 sec gcintvl, yielding
                         * recovery from 50 page overage in ~1 minute.
                         * 100 big ethernet packets might cause this, e.g.
                         */
                        if (n > 0 && (n /= kmemgcscale) == 0)
                                n = 1;
                        ++indx;
                        ++kbp;
                        while (n > 0 && indx <= maxindx) {
                                va = 0;
#ifdef  _POWER_MP
                                s1 = splhigh();
                                KBP_LOCK(kbp);
#else
                                BUCKET_LOCK(s1);
#endif
                                if (kbp->kb_totalfree > kbp->kb_highwat) {
                                        va = kbp->kb_next;
                                        kbp->kb_next = *(void **)va;
                                        kbp->kb_couldfree++;
                                        kbp->kb_total--;
                                        kbp->kb_totalfree--;
                                }
                                if (va) {
                                        n -= 1 << (indx - pagindx);     /* page count */
                                        if (tmo < hz)                   /* want retry */
                                                tmo = 0;
                                        i = 1 << indx;
#if     MACH_ASSERT
                                        kup = btokup(va);
                                        s = 1 << (indx - pagindx);      /* page count */
                                        while (--s >= 0) {
                                                if (s && kup[s].ku_indx >= 0)
                                                        panic("free: gc overlap");
                                                kup[s].ku_indx = -1;
                                                kup[s].ku_freecnt = 0;
                                        }
#endif
#ifdef  _POWER_MP
                                        KBP_UNLOCK(kbp);
                                        splx(s1);
#else
                                        BUCKET_UNLOCK(s1);
#endif
                                        kmem_free(kmemmap, (vm_offset_t)va, i);
                                } else {
#ifdef  _POWER_MP
                                        KBP_UNLOCK(kbp);
                                        splx(s1);
#else
                                        BUCKET_UNLOCK(s1);
#endif
                                        kbp++;
                                        indx++;
                                }
                        }
                }
        }
        /*NOTREACHED*/
}

#ifdef	notyet
/*
 * Set limit for type.
 */
int
kmemsetlimit(
        int     type,
        long    limit)
{
        int s;
        register struct kmemstats *ksp = &kmemstats[type];

        if (type > 0 && type < M_LAST) {
                if (limit <= 0)
                        limit = LONG_MAX;
#ifdef  _POWER_MP
                s = splhigh();
                KSP_LOCK(ksp);
#else
                BUCKET_LOCK(s);
#endif
                ksp->ks_limit = limit;
#ifdef  _POWER_MP
                KSP_UNLOCK(ksp);
                splx(s);
#else
                BUCKET_UNLOCK(s);
#endif
                thread_wakeup((int)ksp);
                return 1;
        }
        return 0;
}
#endif	/* notyet */

/*
 * Initialize the kernel memory allocator
 */

void
kmeminit(void)
{
        register long indx;
        vm_offset_t min, max;
        int cpu;

#if     ((MAXALLOCSAVE & (MAXALLOCSAVE - 1)) != 0)
#error  kmeminit: MAXALLOCSAVE not power of 2
#endif
#if     (MAXALLOCSAVE > MINALLOCSIZE * 32768)   /* see BUCKETINDX macro */
#error  kmeminit: MAXALLOCSAVE too big
#endif
#ifdef not
        if (MAXALLOCSAVE > 4 * PAGE_SIZE)
                panic("kmeminit: MAXALLOCSAVE/PAGE_SIZE too big");
        if (MAXALLOCSAVE < PAGE_SIZE)
                panic("kmeminit: MAXALLOCSAVE/PAGE_SIZE too small");
#endif

        nrpages = vmker.nrpages - vmker.badpages;
        kmempages = MIN( (nrpages / 2), (THEWALL_MAX / PAGE_SIZE));

        /* lite */
        if (nrpages <= 2048) /* 8meg */
                kmemreserve = 4;
        else
                if (nrpages <= 4096) /* 16meg */
                        kmemreserve = 16;
                else
                        kmemreserve = 24; /* >16meg */

        lock_alloc(&alloc_lock, LOCK_ALLOC_PIN, KMEMSTAT_LOCK_FAMILY, M_LAST+1);
        simple_lock_init(&alloc_lock);

        /* 
         * Set the wall to be 1/4 of the number of pages we have mapped.
         * Note since thewall is the number of 1K blocks that can be 
         * allocated and kmempages is 4K blocks, a simple assignment
         * achieves the 1/4 goal.  Thus, thewall defaults to 1/8 
         * real memory (since kmempages is 1/2 of real mem, and thewall
         * is 1/4 kmempages) or THEWALL_MAX, whichever is smaller.  
         */
        thewall_dflt = thewall = MIN(kmempages, (THEWALL_MAX / PAGE_SIZE));

        /* 
         * allocated tracks whether we've hit thewall.  It's a running total
         * of memory xmalloced from the kernel heap.
         */
        allocated = 0;  

        if (kmemmap = (vm_map_t) kmem_suballoc(kernel_map, &min, &max,
                                (vm_size_t) (kmempages * PAGE_SIZE), FALSE)) {
                kmembase = (void *)min;
                kmemlimit = (void *)max;
        } else
                panic("kmeminit map");
        indx = round_page(sizeof (struct kmemusage) * kmempages);
        if ((kmemusage = (struct kmemusage *)kup_init(kmemmap, indx)) == NULL)
                panic("kmeminit structs");
        mclxmemd.aspace_id = XMEM_INVAL;
        xmemat(kmembase, CLUSTERSIZE*NBPG, &mclxmemd);
        mbclusters = mbufs = (int)kmembase;
        bzero(kmemusage, indx);

#if     MACH_ASSERT
        for (indx = 0; indx < kmempages; indx++) {
                kmemusage[indx].ku_indx = -1;
                kmemusage[indx].ku_freecnt = -1; /* -1 for aix_kmem_alloc */
                kmemusage[indx].ku_cpu = -1;
        }
#endif

        pagindx = BUCKETINDX(PAGE_SIZE);
        maxindx = BUCKETINDX(MAXALLOCSAVE);
        for (cpu = 0; cpu < _system_configuration.ncpus; cpu++) {
                for (indx = 0; indx < MINBUCKET + 16; indx++) {
                        if ((bucket[cpu][indx].kb_elmpercl = PAGE_SIZE / 
                                (1 << indx)) <= 1) {
                                bucket[cpu][indx].kb_elmpercl = 1;
                                bucket[cpu][indx].kb_highwat = 10;
                                if (indx >= maxindx && RESERVE > 10)
                                        bucket[cpu][indx].kb_highwat = RESERVE;
                        } else
                                bucket[cpu][indx].kb_highwat = 
                                        5 * bucket[cpu][indx].kb_elmpercl;
                        lock_alloc(&(bucket[cpu][indx].kb_lock), 
                                LOCK_ALLOC_PIN, BUCKET_LOCK_FAMILY, 
                                (cpu + 1) * indx);
                        simple_lock_init(&(bucket[cpu][indx].kb_lock));
                }

                /* 
                 * Special case important sizes. Scale to amount of memory.
                 */
                bucket[cpu][pagindx].kb_highwat = 5 * kmemreserve;
                bucket[cpu][BUCKETINDX(256)].kb_highwat = 16 * kmemreserve;
                bucket[cpu][maxindx].kb_highwat = MAX(10, kmemreserve);
        }

        for (indx = 0; indx < M_LAST; indx++) {
                kmemstats[indx].ks_limit = kmempages * PAGE_SIZE * 6 / 10;
                /* Round it off */
                kmemstats[indx].ks_limit += MINALLOCSIZE - 1;
                kmemstats[indx].ks_limit &= ~(MINALLOCSIZE - 1);
                lock_alloc(&kmemstats[indx].ks_lock, LOCK_ALLOC_PIN, 
                        KMEMSTAT_LOCK_FAMILY, indx);
                simple_lock_init(&kmemstats[indx].ks_lock);
        }

	prime_the_buckets();



        /* 
         * if DEBUG, then call the malloc police station and start policing!
         */
#ifdef DEBUG
        {
                int i=1024;

                (void)net_malloc_police_station(&i, 0);
        }
#endif
}

void
prime_the_buckets()
{
	int num_nuggets, i;
	int cpu, s;
        struct kmembuckets *kbp, *mkbp;
        struct kmemusage *kup;
	void *va;

	/*
	 * Fill the biggest bucket of each bucket array with 
	 * MIN(1/64 * real memory, MAX_INITIAL_PIN)
	 * bytes worth of bucket nuggets.  
	 * For example if MAXALLOCSAVE is 16K, 1 cpu, and
	 * real mem is 16Meg, we need to net_malloc 16 nuggets.
	 */

	num_nuggets  = MIN(nrpages * 64 / MAXALLOCSAVE, MAX_INITIAL_PIN);

	for (cpu=0; cpu < _system_configuration.ncpus; cpu++) {
		mkbp = &bucket[cpu][ maxindx];
		for (i=0; i<num_nuggets; i++) {

			if ((va = (void *)kmem_alloc(kmemmap, 
				MAXALLOCSAVE, cpu)) != NULL) {
				kup = btokup(va);
				kup->ku_indx = maxindx;
#ifdef  _POWER_MP
				s = splhigh();
				KBP_LOCK(mkbp);
#else
				BUCKET_LOCK(s);
#endif
				kup->ku_freecnt = 1;
				mkbp->kb_total++;
				mkbp->kb_totalfree++;
				*(void **)va = mkbp->kb_next;
				mkbp->kb_next = va;
#ifdef  _POWER_MP
				KBP_UNLOCK(mkbp);
#else
				BUCKET_UNLOCK(s);
#endif
#ifdef  _POWER_MP
				splx(s);
#endif
			}
		}
	}
}

void
kmeminit_thread(int pri)
{
        extern task_t first_task;
        pid_t           pid;

        /*
         * Start the kmem thread at the specified priority. Should
         * be the same as the netisr threads.
         */

        pid = creatp();
        assert(pid != -1);
        kmemthread = (PROCPTR(pid))->p_threadlist;
        initp(pid, malloc_thread, 0, 0, "netm");
        assert(setpri(pid, MBUF_RTPRI) != -1);
        if (kmemthread == NULL)
                panic("kmeminit_thread");
}


void *
aix_kmem_suballoc(minp, maxp, size)
int     *minp;
int     *maxp;
u_int   size;
{
        caddr_t         area;

        area = (caddr_t)xmalloc(size, PGSHIFT, kernel_heap);
        if (area == (caddr_t)NULL)
                panic("aix_kmem_suballoc: xmalloc");
        *minp = (int)area;
        *maxp = (int)(area + size);
        return(area);
}

kup_init(map, size)
caddr_t         map;
u_int           size;
{
        return(xmalloc(size, PGSHIFT, pinned_heap));
}

struct err_rec0 alloc_log = { ERRID_LOW_MBUFS, "SYSNET" };
unsigned long   tv_sec=0;

void *
aix_kmem_alloc(map, size, cpu)
caddr_t         map;
u_int           size;
int             cpu;
{
        caddr_t                 addr;
        int                     i, j;
        int                     pages;
        int                     gotit;
        struct kmemusage        *kup;
        int                     s;
        struct timestruc_t      ctv;

        assert(csa->prev == NULL);
        if (csa->intpri != INTBASE) {
                assert(size <= MAXALLOCSAVE);
                return((caddr_t)NULL);
        }

        s = disable_lock(INTMAX, &alloc_lock);
        if (allocated + size > thewall*1024) {
                unlock_enable(s, &alloc_lock);
                /*
                 * Don't flood the errlog.  Log message at a
                 * minimum of once every 30 minutes.
                 */
                (void) curtime(&ctv);
                if ( (tv_sec + (60*30)) < ctv.tv_sec) {
                        (void) errsave(&alloc_log,sizeof(alloc_log));
                        tv_sec = ctv.tv_sec;
                }
                return((caddr_t)NULL);
        }

        size = round_page(size);
        pages = size/PAGE_SIZE;
        addr = NULL;
        for (i = 0; i < kmempages - pages ; i++) {
                gotit = 1;
                for (j = 0; j < pages ; j++) {
                        if (kmemusage[i+j].ku_freecnt != (u_short)-1) {
                                gotit = 0;
                                i += j; /* skip over used or too small areas */
                                break;
                        }
                }
                if (gotit)
                        break;
        }
        if (gotit) {
                addr = (caddr_t)((int)kmembase + (i * PAGE_SIZE));
                kup = btokup(addr);
                for (i = 0; i < pages ; i++) {
                        kup->ku_freecnt = 0;

                        /* 
                         * Remember which cpu/bucket array we put this
                         * memory into soos we can free it back to the
                         * same bucket array.  Otherwise we cannot 
                         * Coalesce easily.  
                         */
                        kup->ku_cpu = cpu;
                        kup++;
                }
                allocated += size;
                unlock_enable(s, &alloc_lock);

                if (pin(addr, size)) {
                        s = disable_lock(INTMAX, &alloc_lock);
                        kup = btokup(addr);
                        for (i = 0; i < size/PAGE_SIZE; i++) {
                                kup->ku_freecnt = -1;
                                kup++;
                        }
                        allocated -= size;
                        unlock_enable(s, &alloc_lock);
                        addr = 0;
                }
        } else
                unlock_enable(s, &alloc_lock);
        return(addr);
}

aix_kmem_free(map, addr, size)
caddr_t         map;
caddr_t         addr;
u_int           size;
{
        int                     i;
        struct kmemusage        *kup;
        int                     s;

        s = disable_lock(INTMAX, &alloc_lock);
        size = round_page(size);
        allocated -= size;
        kup = btokup(addr);
        for (i = 0; i < size/PAGE_SIZE; i++) {
                kup->ku_freecnt = -1;
                kup++;
        }
        unlock_enable(s, &alloc_lock);
        unpin(addr, size);
}

static struct trb *threadtime;

void
threadtimer() {
        et_post(EVENT_NETMALLOC, kmemthread->t_tid);
}

malloc_thread_set_timeout(tmo) 
{
        if (threadtime == (struct trb *) NULL) {
                threadtime = talloc();
                threadtime->func = threadtimer; 
                threadtime->func_data = (ulong) 0;
                threadtime->ipri = INTCLASS2;
        }
        threadtime->timeout.it_value.tv_sec = tmo/HZ;
        threadtime->timeout.it_value.tv_nsec = (tmo % HZ) * (1000000000 / HZ);
#ifndef _POWER_MP
        tstop(threadtime);
#else
        while (tstop(threadtime));
#endif
        tstart(threadtime);
}
