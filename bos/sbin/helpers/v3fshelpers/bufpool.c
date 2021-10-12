static char sccsid[] = "@(#)60	1.3.1.7  src/bos/sbin/helpers/v3fshelpers/bufpool.c, cmdfs, bos411, 9430C411a 7/20/94 16:03:13";
/*
 *   COMPONENT_NAME: CMDFS
 *
 *   FUNCTIONS: BHASH
 *		FRAGEQUAL
 *		INSQUE
 *		REMQUE
 *		bpassert
 *		bpclose
 *		bpflush
 *		bpinit
 *		bpread
 *		bprelease
 *		bptouch
 *		buf_stat
 *		bufhashstats
 *		bufstats
 *		bwritebuf
 *		fhdr
 *		findhdr
 *		grow_dynamic
 *		hdr_nmbr
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <libfs/libfs.h>
#include "bufpool.h"

/* Buffer pool for filesystem disk blocks
 * for k > 0 Bufhdr[k] describes contents of Buffer[k-1].
 * Bufhdr[0] is used as anchor for free list. when a
 * Buffer is needed, Bufhdr[0].prev is the Buffer selected.
 */

/*
 * structs
 */

/*
 * header for Buffer pool (note: hnext and hprev MUST be first in order
 * to match HASHHDR...  this lets all the nifty macros work!)
 */
#define	BUFHDR	struct bufhdr
BUFHDR
{
	BUFHDR	*hnext;		/* next on hash chain		*/
	BUFHDR	*hprev;		/* previous on hash chain	*/
	BUFHDR	*fnext;		/* next on free list		*/
	BUFHDR	*fprev;		/* previous on free list	*/

	char	 modify;	/* Buffer modified?		*/
	char	 hashed;	/* on hash list?		*/
	int	 count;		/* reference count		*/
	frag_t	 bn;		/* 4k block number		*/
	char	*buf;		/* actual data			*/
};

/*
 * hash chain hdr
 */
#define	HASHHDR	struct bhash
HASHHDR {
	BUFHDR	*hnext;		/* next on hash chain		*/
	BUFHDR	*hprev;		/* previous on hash chain	*/
	};

/*
 * constants
 */
#define NBUFPOOL	(256)	/* sizeof Buffer pool. (* BLKSIZE)	*/
#define	NHASH		(BLKSIZE / sizeof(HASHHDR))	/* sizeof hash table */

/*
 * hash function (assumes NHASH is a power of 2)
 */
#define	BHASH(bn)		((bn) & (NHASH - 1))

/*
 * static (private) data
 */
static BUFHDR Bufhdr[NBUFPOOL];	/* buffer pool headers		*/
static HASHHDR bhash[NHASH];	/* hash chain headers		*/
static char Buffer[NBUFPOOL - 1][BLKSIZE]; /* actual buffers	*/
static int Bdfd = -1;		/* Buffer device file descriptor	*/
static void (*Rwerr)();		/* read/write error function		*/

/*
 * global data
 */
int	Bp_touched = 0;		/* bptouch called yet?		*/

/*
 * static functions
 */
#ifndef _NO_PROTO

static BUFHDR *findhdr(void *);
static void bwritebuf(BUFHDR *);
static char *hdr_nmbr(BUFHDR *);

#else /* _NO_PROTO */

static BUFHDR *findhdr();
void bwritebuf();
static char *hdr_nmbr();

#endif /* _NO_PROTO */

void bufhashstats();
void grow_dynamic();

/*
 * macros
 */

/*
 *  if frag1 and frag2 exactly the same, return 1 else return 0
 */
#define FRAGEQUAL(frag1, frag2) \
(((frag1).addr == (frag2).addr && (frag1).nfrags == (frag2).nfrags) ? 1 : 0)

/*
 * take node 'node' from the queue defined by 'next' and 'prev'
 */
#define	REMQUE(node, next, prev) \
	(node)->next->prev = (node)->prev, (node)->prev->next = (node)->next

/*
 * insert 'second' behind 'first' in the queue defined by 'next' and 'prev'
 */
#define	INSQUE(first, second, next, prev, type) \
	(second)->next = (first)->next,		\
	(second)->prev = (type) (first),	\
	(first)->next->prev = (type) (second),	\
	(first)->next = (type) (second)

/*
 * init Buffer pool
 */
void
#ifndef _NO_PROTO
bpinit(int fd, void (*rwerr)())
#else /* _NO_PROTO */
bpinit(fd, rwerr)
int fd;
void (*rwerr)();
#endif /* _NO_PROTO */
{
	register BUFHDR *hdr;
	register HASHHDR *hash;

	BPPRINTF(("bpinit()...\n"));

	/*
	 * initialize free list
	 */
	for (hdr = &Bufhdr[0]; hdr < &Bufhdr[NBUFPOOL]; hdr++) {
		if (hdr > Bufhdr) {
			hdr->buf = Buffer[hdr - Bufhdr - 1];
			hdr->fprev = hdr - 1;
			}
		hdr->fnext = hdr + 1;
		hdr->count = 0;
		hdr->modify = 0;
		hdr->hashed = 0;
		}
	Bufhdr[0].fprev = &Bufhdr[NBUFPOOL - 1];
	Bufhdr[NBUFPOOL - 1].fnext = &Bufhdr[0];

	/*
	 * initialize hash list
	 */
	for (hash = &bhash[0]; hash < &bhash[NHASH]; hash++)
		hash->hprev = hash->hnext = (BUFHDR *) hash;

	/*
	 * save off file descriptor & read/write error function
	 */
	Bdfd = fd;
	Rwerr = rwerr;

	Bp_touched = 0;

#ifdef BPDEBUG
	bufhashstats();
#endif
}

/*
 *  special purpose hack for expanddir in op_make.  when expanding
 *  lost+found, we must hack the bufhdr struct
 */
void *
#ifndef _NO_PROTO
bpexpand(char *addr, frag_t fragment)
#else
bpexpand(addr, fragment)
	char *addr;
	frag_t fragment;
#endif
{
	fdaddr_t	frag;
	HASHHDR		*hash;
	BUFHDR 		*hdr;

	/*
	 *  find a bufhdr and make sure it's hashed and referenced
	 */
	if ( ! ((hdr = findhdr(addr)) && hdr->hashed && hdr->count))
		return (void *)NULL;
	/*
	 *  remove bufhdr from hash list
	 *  zero out part of buf not referenced by hdr->bn
	 *  put new blkno in bufhdr and set modify and Bp_touched
	 */
	REMQUE(hdr, hnext, hprev);
	memset((void *)(hdr->buf + FRAGLEN(hdr->bn)), 0,
	       (size_t)(BLKSIZE - FRAGLEN(hdr->bn)));
	hdr->bn = fragment;
	hdr->modify = 1;
	Bp_touched = 1;
	/*
	 *  hash to new home
	 */
	frag.f = fragment;
	hash = &(bhash[BHASH(frag.d)]);
	INSQUE(hash, hdr, hnext, hprev, BUFHDR *);
	return (void *)hdr->buf;
}
/*
 * bpread (bn)
 * return pointer of page in Buffer pool containing disk block specified
 */
void *
#ifndef _NO_PROTO
bpread(register frag_t bn)	/* 'bn' = physical block number to read */
#else /* _NO_PROTO */
bpread(bn)
register frag_t bn;	/* 'bn' = physical block number to read */
#endif /* _NO_PROTO */
{
	register BUFHDR *hdr;
	register HASHHDR *hash;
	fdaddr_t 	frag;
	char	*cp;
	int	flen, i;

	BPPRINTF(("bpread(nfrags=%d  addr=%d): ", bn.nfrags, bn.addr));
	frag.f = bn;
	/*
	 * search hash chain for the block
	 */
	hash = &bhash[BHASH(frag.d)];
	for (hdr = hash->hnext;
	     hdr != (BUFHDR *)hash && !FRAGEQUAL(hdr->bn, bn);
	     hdr = hdr->hnext)
		BPPRINTF(("hash: looking at %s\n", hdr_nmbr(hdr)));

	/*
	 * found?
	 */
	if (hdr != (BUFHDR *) hash)
	{
		/*
		 * yep
		 */

		/*
		 * increment reference count, if ref count was 0, remove from
		 * the free list
		 */
		if (hdr->count++ == 0)
			REMQUE(hdr, fnext, fprev);

		BPPRINTF(("increment...\n"));

		return (hdr->buf);
	}

	/*
	 * if not found, use the free list (Bufhdr[0].fprev)
	 */
	if ((hdr = Bufhdr[0].fprev) == &Bufhdr[0]) {
		/*
		 * try to get some dynamic space...
		 */
		grow_dynamic();
		/*
		 * try it again
		 */
		if ((hdr = Bufhdr[0].fprev) == &Bufhdr[0]) {
			BPPRINTF(("bpread: no free blocks left.  help!\n"));
			abort();
			return (NULL);	/* shouldn't get this far, but... */
		}
	}

	BPPRINTF(("bpread: next on free is k = %s\n", hdr_nmbr(hdr)));

	/*
	 * remove hdr from free list
	 */
	REMQUE(hdr, fnext, fprev);

	/*
	 * write it out if necessary
	 */
	if (hdr->modify)
		bwritebuf(hdr);

	/*
	 * remove it from its old position on the hash chain
	 */
	if (hdr->hashed)
		REMQUE(hdr, hnext, hprev);

	/*
	 * insert in new hash position
	 */
	INSQUE(hash, hdr, hnext, hprev, BUFHDR *);

	/*
	 * fill in bufhdr
	 */
	hdr->modify = 0;	/* data has not been touched yet */
	hdr->bn.addr = bn.addr;	/* fill in block number		*/
	hdr->bn.nfrags = bn.nfrags;
	hdr->bn.new = 0;
	hdr->count = 1;		/* set reference count		*/
	hdr->hashed = 1;	/* now it's on the hash list!	*/
	flen = FRAGLEN(bn);

	/* Read in data.  If an error occurs attempting the read the 
	 * complete block, then read each sector individually.  The 
	 * offending sector will be zero'd. 
	 */
	if (bread(Bdfd, hdr->buf, bn) != flen)
	{
#ifdef _BLD
		if (lseek(Bdfd, (off_t)FRAG2BYTE(bn.addr), SEEK_SET) < 0)
#else
		if (llseek(Bdfd, (offset_t)FRAG2BYTE(bn.addr), SEEK_SET) < 0)
#endif
		{
			(*Rwerr)(2, bn);
			return NULL;
		}
		bzero(hdr->buf, BLKSIZE);

		for (cp = hdr->buf, i = 0; i < flen; i += DEV_BSIZE, 
						     cp += DEV_BSIZE) 
		{
                	if (read(Bdfd, cp, DEV_BSIZE) != DEV_BSIZE) 
			{
				/* Tell the user we cleared this block */
				(*Rwerr)(5, (int)BYTE2DEVBLK
						(FRAG2BYTE(bn.addr) + i));

#ifdef _BLD
                        	if (lseek(Bdfd, (off_t)
				    FRAG2BYTE(bn.addr) + i + DEV_BSIZE, 
				    SEEK_SET) < 0)
#else
                        	if (llseek(Bdfd, (offset_t)
				    FRAG2BYTE(bn.addr) + i + DEV_BSIZE, 
				    SEEK_SET) < 0)
#endif
				{
					(*Rwerr)(2, bn);
					return NULL;
				}
				/* Data has been zero'd and therefore modified.
				 * Bad block relocation will occur.
				 */
				hdr->modify = 1;
				Bp_touched = 1;
			}
                }
        }

	BPPRINTF(("read...\n"));

#ifdef BPDEBUG
	bufhashstats();
#endif

	return (hdr->buf);
}

/*
 * mark 'addr' as modified
 */
void
#ifndef _NO_PROTO
bptouch(void *addr)
#else /* _NO_PROTO */
bptouch(addr)
void *addr;
#endif /* _NO_PROTO */
{
	register BUFHDR *hdr = findhdr(addr);

#ifdef BPDEBUG
	BPPRINTF(("bptouch(0x%x):", (int) addr));

	if (hdr == NULL)
		BPPRINTF(("bptouch: can't find address 0x%x!", addr));
	else
#endif /* BPDEBUG */
		{
		BPPRINTF(("k = %s", hdr_nmbr(hdr)));

		hdr->modify = 1;
		Bp_touched = 1;
		}
}

/*
 * mark 'addr' as released (stick on free list)
 */
void
#ifndef _NO_PROTO
bprelease(void *addr)
#else /* _NO_PROTO */
bprelease(addr)
void *addr;
#endif /* _NO_PROTO */
{
	register BUFHDR *hdr = findhdr(addr);

#ifdef BPDEBUG
	BPPRINTF(("bprelease(0x%x): ", addr));

	if (hdr == NULL)
		BPPRINTF(("can't find address 0x%x!", addr));
	else if (hdr->count < 1)
		BPPRINTF(("address 0x%x count gone negative", addr));
	else
#endif /* BPDEBUG */

	if (--hdr->count == 0) {
		BPPRINTF(("releasing "));

		/*
		 * reference count went to 0; no one else is using it, so add
		 * it to the free list
		 */
		INSQUE(&Bufhdr[0], hdr, fnext, fprev, BUFHDR *);
		}

	BPPRINTF(("k = %s", hdr_nmbr(hdr)));
}

/*
 * flush 'addr' from the Buffer pool
 */
void
#ifndef _NO_PROTO
bpflush(void *addr)
#else /* _NO_PROTO */
bpflush(addr)
void *addr;
#endif /* _NO_PROTO */
{
	register BUFHDR *hdr = findhdr(addr);

#ifdef BPDEBUG
	BPPRINTF(("bpflush(0x%x): ", addr));

	if (hdr == NULL)
		BPPRINTF(("bpflush: can't find address 0x%x!", addr));

	else if (hdr->count < 1)
		BPPRINTF(("bpflush: address 0x%x count gone negative", addr));

	else if (hdr->count > 1)
		BPPRINTF(("bpflush: address 0x%x still being used", addr));

	else
#endif /* BPDEBUG */
	if (--hdr->count == 0) {
#ifdef BPDEBUG
		BPPRINTF(("k = %s\n", hdr_nmbr(hdr)));
		bufhashstats();
		BPPRINTF(("hdr->hnext = %x, hdr->hprev = %x\n", hdr->hnext, hdr->hprev));
#endif

		/*
		 * write it out if modified
		 */
		if (hdr->modify)
			bwritebuf(hdr);

		/*
		 * remove from the hash list
		 */
		REMQUE(hdr, hnext, hprev);

#ifdef BPDEBUG
		bufhashstats();
#endif

		hdr->hashed = 0;

		/*
		 * add it to the free list
		 */
		INSQUE(&Bufhdr[0], hdr, fnext, fprev, BUFHDR *);
		}
}

/*
 * flush all Buffers & close down Buffer pool
 */
void
bpclose()
{
	register BUFHDR *hdr;
	register HASHHDR *hash;

	BPPRINTF(("bpclose()\n"));

	for (hash = &bhash[0]; hash < &bhash[NHASH]; hash++)
		for (hdr = hash->hnext; hdr != (BUFHDR *) hash;
		     hdr = hdr->hnext) {
#ifdef BPDEBUG
			if (hdr->count > 0) {
				BPPRINTF(("\tbpclose: block (%d,%d) has ref count of %d\n",
				hdr->bn.nfrags, hdr->bn.addr, hdr->count));
				}
#endif /* BPDEBUG */
			if (hdr->modify)
				bwritebuf(hdr);
			}

	Bp_touched = 0;
}

/*
 * write out Buffer 'hdr'
 */
static void
#ifndef _NO_PROTO
bwritebuf(register BUFHDR *hdr)
#else /* _NO_PROTO */
bwritebuf(hdr)
register BUFHDR *hdr;
#endif /* _NO_PROTO */
{
	BPPRINTF(("bwrite(%s)\n", hdr_nmbr(hdr)));

	/*
	 * write it out
	 */
	if (bwrite(Bdfd, hdr->buf, hdr->bn) != FRAGLEN(hdr->bn))
		(*Rwerr)(4, hdr->bn);

	hdr->modify = 0;
}

/*
 * assert that 'addr' is still be referenced
 */
void
#ifndef _NO_PROTO
bpassert(char *string, void *addr, int line)
#else /* _NO_PROTO */
bpassert(string, addr, line)
char	*string;
void	*addr;
int	 line;
#endif /* _NO_PROTO */
{
	register BUFHDR *hdr;

	if ((hdr = findhdr(addr)) == NULL)
		BPPRINTF(("bpassert: can't find address 0x%x!", addr));

	else if (hdr->count <= 0)
		BPPRINTF(("bpassert: %s on line %d (k = %s)", string, line,
			hdr_nmbr(hdr)));
}

/*
 * print out hash chains and free list
 */
void
bufstats()
{
	register BUFHDR *hdr;
	register HASHHDR *hash;
	register FILE *fp;

	if ((fp = popen("pg", "w")) == NULL)
		printf("can't pipe to pg\n");
	else {
		fprintf(fp,"Hash chains:\n");
		for (hash = bhash; hash < &bhash[NHASH]; hash++)
			if ((hdr = hash->hnext) != (BUFHDR *) hash) {
				fprintf(fp,"\tchain %d: ", hash - bhash);
				for ( ; hdr != (BUFHDR *) hash;
				     hdr = hdr->hnext)
					fprintf(fp,"%s (%d), ",
						hdr_nmbr(hdr), hdr->bn);
				putchar('\n');
				}

		fprintf(fp,"Free list:\n");
		for (hdr = Bufhdr[0].fnext; hdr != Bufhdr; hdr = hdr->fnext)
			fprintf(fp,"\t%s (%d)\n", hdr_nmbr(hdr), hdr->bn);

		pclose(fp);
		}
}

/*
 * print out head of hash chains
 */
void
bufhashstats()
{
	int i;

	for (i = 0; i < NHASH; i++)
		BPPRINTF(("hash %d: %x, prev = %x, next = %x\n", i, &bhash[i],
			bhash[i].hprev, bhash[i].hnext));
}

/*
 * stat 'addr'
 */
buf_stat(addr)
void *addr;
{
	register BUFHDR *hdr = findhdr(addr);

	printf("0x%x (%s):\n", addr, hdr_nmbr(hdr));

#define	PRADDR(addr) \
	printf("\t addr = 0x%0x (%s)\n", hdr->addr, hdr_nmbr(hdr->addr))

	PRADDR(fnext);
	PRADDR(fprev);
	PRADDR(hnext);
	PRADDR(hprev);

	printf("\t modify = %d\n", hdr->modify);
	printf("\t count = %d\n", hdr->count);
	printf("\t bn = %d\n", hdr->bn);
	printf("\t buf = 0x%x\n", hdr->buf);
}

#ifndef _NO_PROTO

static BUFHDR *fhdr(void *, int *);

#else /* _NO_PROTO */

static BUFHDR *fhdr();

#endif /* _NO_PROTO */

/*
 * return a 'header number' for this BUFHDR
 */
static char *
hdr_nmbr(hdr)
register BUFHDR *hdr;
{
	int nmbr;
	static char buf[20];

	if ((nmbr = hdr - Bufhdr) >= 0 && nmbr < NBUFPOOL)
		sprintf(buf, "%d", nmbr);
	else if (fhdr(hdr->buf, &nmbr) != NULL)
		sprintf(buf, "%d+", nmbr);
	else
		strcpy(buf, "?");

	return (buf);
}

/*
 * take an address and return the bufhdr
 */
static BUFHDR *
#ifndef _NO_PROTO
findhdr(void *addr)
{
	char *charaddr = (char *) addr;
#else /* _NO_PROTO */
findhdr(charaddr)
char *charaddr;
{
#endif /* _NO_PROTO */

	int ct, i;
	register int bufno = (charaddr - (char *) Buffer) / BLKSIZE + 1;
	BUFHDR *bh;

	/*
	 * static buffer pool?
	 */
	if (bufno > 0 && bufno < NBUFPOOL)
		return (&Bufhdr[bufno]);

	/*
	 * nope - try dynamic
	 */
	return (fhdr(charaddr, &ct));
}

/*
 * 'dynamic' buffer pool
 */
#define	DYNAMIC struct dynamic
DYNAMIC {
	BUFHDR	 bufhdr;
	DYNAMIC	*next;
	};

static DYNAMIC *Dynamic = NULL;	/* Dynamic buffer pool itself */

static void
grow_dynamic()
{
	register char *bufs;
	register BUFHDR *hdr;
	register int i;
	extern void *malloc();
	register DYNAMIC *dyn, *block;

#define	DYN_BLOCKS	32	/* # of dynamic blocks to grow at a time */

	/*
	 * alloc DYN_BLOCKS buffers at a time...
	 */
	if ((block=(DYNAMIC *) malloc(DYN_BLOCKS * sizeof(*block))) != NULL &&
	    (bufs = (char *) malloc(DYN_BLOCKS * BLKSIZE)) != NULL)
		/*
		 * walk thru & initialize the new ones
		 */
		for (i = 0; i < DYN_BLOCKS; i++) {
			dyn = block + i;
			hdr = &dyn->bufhdr;

			/*
			 * init new bufhdr
			 */
			hdr->buf = bufs + (BLKSIZE * i);
			hdr->modify = 0;
			hdr->hashed = 0;
			hdr->count = 0;

			/*
			 * stick it onto the free list
			 */
			INSQUE(&Bufhdr[0], hdr, fnext, fprev, BUFHDR *);

			/*
			 * link it into the Dynamic list
			 */
			dyn->next = Dynamic;
			Dynamic = dyn;
			}

	/*
	 * we do no error recovery here since we know the calling
	 * function will note that the free list never grew - they
	 * will bomb out somehow
	 */
}

/*
 *	fhdr
 *
 *	- look for 'addr' in the dynamic list.  return the bufhdr for
 *	  it.  as a side effect, stick into 'indx' the index of the
 *	  bufhdr in the dynamic list...
 */
static BUFHDR *
#ifndef _NO_PROTO
fhdr(void *voidaddr, int *indx)
{
	char *addr = (char *) voidaddr;
#else /* _NO_PROTO */
fhdr(addr, indx)
char *addr;
int *indx;
{
#endif /* _NO_PROTO */
	register int ct;
	register BUFHDR *hdr;
	register DYNAMIC *dyn;

	for (ct = 0, dyn = Dynamic; dyn != NULL; dyn = dyn->next, ct++) {
		hdr = &dyn->bufhdr;
		if (hdr->buf <= addr && addr < hdr->buf + BLKSIZE)
			break;
		}

	if (dyn == NULL) {
		*indx = -1;
		return (NULL);
		}

	*indx = ct;

	return (hdr);
}
