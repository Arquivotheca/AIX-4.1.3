static char sccsid[] = "@(#)30	1.23  src/bos/kernel/ldr/ld_lookup.c, sysldr, bos411, 9428A410j 3/31/94 11:47:35";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: ld_hash(), ld_hashfind(), ld_symfind(), ld_svcaddress(),
 *            ld_buildhash(), ld_updatehash()
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

#include <sys/syspest.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/xcoff.h>
#include <sys/malloc.h>
#include <sys/ldr.h>
#include "ld_rbytes.h"
#include "ld_data.h"


	extern void bcopy();
	extern void bzero();

/* type check field - this seems not to be declared elsewhere */
#define	UNIVERSAL  0x20202020  /* four blanks are the universal hash value */
/* these access functions assume machine supports unaligned load */
#define LANG(typchk)  (*(ushort*)(typchk))
#define HASH(typchk)  (*(ulong*)((ulong)(typchk)+2))
#define LANGHASH(typchk)  (*(ulong*)((ulong)(typchk)+6))

/* ld_hash computes a universal hash of the input string
 * by using random values, it avoids being sensitive to
 * patterns in the input.
 * It is called with a string pointer and a MAXIMUM length
 * It returns the hash and the true length of the string NOT including the NULL
 */

unsigned
ld_hash(
caddr_t      sp,
ulong        *sl)
{
    unsigned               hash;
    ulong                  i,len,inlen,byte;
    i = 0;
    hash = 0;
    inlen = len = *sl;
    /* rbytes is RBYTES_SIZE words long */
    for(; len && (byte = (ulong)*sp++); len--)
	hash = hash + rbytes[i = (i + byte)&(RBYTES_SIZE-1)];
    *sl = inlen - len;
    return hash;
    }

/* ld_hashfind will find a symbol in the hash table and return
 * the control block (exp_index) which describes it
 * it is passed the loader exports table which it is to search
 * and a pointer to the ldhdr and ldsym it is to search for
 * it needs ldhdr since ldsym contains offsets relative to the string
 * table and the string table is found relative to ldhdr!
 */

struct exp_index *
ld_hashfind(
struct loader_exports	*lx,
struct ldhdr	*ldhdr,
struct ldsym	*ldsym,
int		*rc,		/* only set when null pointer is returned */
struct loader_entry *le)	/* only used for messages - may be NULL */
{
    caddr_t	symstring;
    ulong 	symlength,hash;
    struct exp_index	*ex;
    if (NULL == lx ) {
    	/* NOT an error - le_exports is null for any module that doesn't have
    	 * any exports
	 */
    	*rc = 0;
    	return NULL;
    }
    	
    if (ldsym->l_zeroes == 0){
       if (ldsym->l_offset > ldhdr->l_stlen) goto enoexec;
       symstring = (caddr_t) ldhdr+ldhdr->l_stoff + ldsym->l_offset;
       /* set max length which prevents running off the end*/
       symlength = ldhdr->l_stlen - ldsym->l_offset;
    }
    else {
       symstring = &(ldsym->l_name[0]);  /* always hash 8 - includes
					    training nulls */
       symlength = 8;
    }
    hash = ld_hash(symstring,&symlength); /* N.B. symlength reset to true
					     length */
    ex = lx->hashes[hash & lx->hash_mask];
    for(; ex != NULL; ex = ex->next_index) {
        if (ex->fullhash != hash) continue;
        if (symlength == ex->syml &&
	    0 == memcmp(ex->sym,symstring,symlength) ){
		void *nt,*ot;
        	if(0 == ldsym->l_parm || NULL == ex->typecheck)
			return ex;
        	if( ldsym->l_parm+10 > ldhdr->l_stlen )
        		goto enoexec;
           	nt=(void*)((ulong)ldhdr+ldhdr->l_stoff+ldsym->l_parm);
		ot=(void*)(ex->typecheck);
		if (
			/* N.B. in the tests we test the common case first */
			/* first check general hash */
			(HASH(nt) == HASH(ot) || HASH(nt)==UNIVERSAL
				|| HASH(ot) == UNIVERSAL)
			/* now check language hash */
		  &&	( LANGHASH(nt) == LANGHASH(ot) || LANG(nt) != LANG(ot)
			 || LANGHASH(nt) == UNIVERSAL ||
				LANGHASH(ot) == UNIVERSAL)
		   )	return ex;
		/* if we fall through, the types mismatch */
		{ /* to declare sym */
		char sym[65];
		symlength=MIN(symlength,64);
		bcopy(symstring,sym,symlength);
		sym[symlength]=0;
		ld_emess(L_ERROR_TYPE,sym,le);
		goto enoexec;
		} /*end of use of sym */
        }
    }
    *rc = 0; /*not found rather than error*/
    return NULL;

enoexec:
    *rc = ENOEXEC;
    return NULL;
}

/* ld_symfind will find a symbol in the hash table and return
 * the control block (exp_index) which describes it
 * it is passed the loader exports table which it is to search
 */

struct exp_index *
ld_symfind(
struct loader_exports	*lx,
caddr_t	symstring)
{
    ulong	symlength,hash;
    struct exp_index	*ex;
    if (NULL == lx)
    	return NULL;
    symlength = 1<<30;             /* hash will compute true length */
    hash = ld_hash(symstring,&symlength);
    ex=lx->hashes[hash & lx->hash_mask];
    for(; ex != NULL; ex = ex->next_index) {
        if (ex->fullhash != hash) continue;
        if ( symlength == ex->syml && 0==memcmp(ex->sym,symstring,symlength) )
	       return ex;
    }
    return NULL;
}

/* ld_svcaddress will return the address that a symbol resolves to in
 * user space.  used by aixinit to find execve
 */

char *
ld_svcaddress(
char	*symstring)
{
	struct exp_index	*ex;
	ex = ld_symfind(syscall_exports->le_exports,symstring);
	return ex ? ex -> exp_location : NULL ;
}

/*
 * builds the hash table data structure for an already loaded routine
 * passed the address of the loader section for the routine, the number
 * of exports, and a table which contains the relocation of each symbol.
 * This must be passed because the loader does not modify the loader
 * section in place while resolving symbols. After buildhash, this
 * address vector is no longer needed.  However, the hash table continues
 * to point to and thus need the loader section, including string storage
 */

/*
 * at least temporary kludge for svc - we change __ names to name without __
 * in hash table.
 */

struct loader_exports *
ld_buildhash(
struct ldhdr *ldhdr,
int          numexports,
struct locs  *locs,       /* vector parallel to syms with relocation for
			     each symbol */
char         type,        /* which class of symbols to include - usually
			     export or svc */
int          kernel,      /* when hashing kernel or extensions, do __ kludge */
struct loader_entry	*le, 	/* when kernel, this is the le of the
				 * previous kernel or svc exports - we
				 * must append the new names to
				 * this namespace */
heapaddr_t   heap,
int	*redefine)	  /* flag set if we redefine a previously defined */
			  /* symbol,  can be used to optimize loads of    */
			  /* modules that depend on the kernel */
{
    int                    i,j,k,size,size2,l_nsyms,fi;
    unsigned               hash_mask;
    caddr_t                location;
    caddr_t                symstring;
    struct loader_exports  *lx;
    struct exp_index       *e_i,
                           **hashes,
                           **h,
                           *firstnewexp; /* when appending to kernel
					    names space */
    struct ldsym           *ldsym;
    unsigned               hash;
    ulong                  symlength;
    char		   kludge;
    void		   *typchk;
    struct                 sharedstuff *sspointer,sharedstuff;


    if (redefine)
    	*redefine = 0;

    if ( le )
	    numexports += le->le_exports->numexports;
    for(size2=2; size2 < numexports; size2 *= 2);
    hash_mask = (size2 - 1);
    size = sizeof(struct loader_exports) +
           size2*sizeof(struct exp_index*) +
           numexports * sizeof(struct exp_index);
    /* Check and see if this specified heap is in the process private
     * segment.  If it is,  then we know that this is the per-process
     * kernel heap.  ld_ualloc should be called to allocate space from
     * this heap and use the overflow heap if necessary.  Before calling
     * ld_ualloc,  a dummy sharedstuff structure must be built.  
     * NOTE:	This code assumes addressability to the overflow segment
     * 		if present in the calling process.  If the ld_ualloc
     *		call must create an overflow segment,  the segment will
     *		remain addressable(attached) upon exit from this function.
     *		Code in this path of execution(ld_cleanup) will be
     *		responsible detaching from the overflow segment.
     */
    if (IS_PRIVSEG(heap)) {
	sspointer = &sharedstuff;
	ss.la = (struct loader_anchor *)(u.u_loader);
	lx = ld_ualloc(sspointer, size);
    }
    else
	lx=xmalloc(size,2,heap);

    if ( lx == NULL ) return NULL;
    lx->hash_mask = hash_mask;
    lx->hashes = hashes = (void *)(lx + 1); /* N.B. this really adds
						    sizeof(exp_header) !*/
    lx->indexes = e_i = (void *)(hashes + size2); /* N.B. really
						    size2*sizeof(*exp_index) */

    for(i = 0; i< size2; i++)
	  hashes[i] = NULL;

    /* if kernel and a previous table exists, initialize current with contents
     * of previous
     */

    if ( le ) {
        fi = le->le_exports->numexports;
        bcopy(le->le_exports->indexes,e_i,sizeof(struct exp_index)*fi);
        for(i = 0; i< fi; i++){
          hash = e_i[i].fullhash & hash_mask;
          e_i[i].next_index = hashes[hash];
          hashes[hash] = &e_i[i];
        }
    } /* end of merge previous  */
    else
	 fi = 0;                      /*next free exp_index*/
    firstnewexp = &e_i[fi];

    l_nsyms = ldhdr->l_nsyms;    /* compiler may not be smart enough to
				    know this is fixed*/
    ldsym = (struct ldsym *) (ldhdr+1);
    for(i=0;i<l_nsyms;i++)
       /* for each exported symbol, make a hash table entry */
       if ((ldsym[i].l_smtype & type)) {
          ASSERT(fi < numexports);
          if (ldsym[i].l_zeroes == 0){
             symstring=(caddr_t)ldhdr+ldhdr->l_stoff+ldsym[i].l_offset;
             /* set max length which prevents running off the end*/
             symlength = ldhdr->l_stlen - ldsym[i].l_offset;
          }
          else {
             symstring = &(ldsym[i].l_name[0]);  /* always hash 8 - includes
						    training nulls*/
             symlength = 8;
          }
          if ( kernel ) {
              char	temp[9];
              char	*symp;
              struct exp_index	*exp;
              if ( *symstring == '_' && *(symstring+1) == '_' ) {
                  kludge = 1;
                  symstring += 2;
                  symlength -= 2;
              }
              else
		  kludge = 0;
              /* ugh - in 8 char case, string may not be null terminated
               * so move it to a temp
               */
              if (symlength == 8) {
		   bcopy(symstring,temp,8);
		   temp[8]=0;
		   symp=temp;
              }
              else
		   symp=symstring;
              if (exp = ld_symfind(lx,symp)){
                 /* so there already is a symbol with this name - because it is
                  * around with and without __
                  * we use the __ version
                  * Other possibility is that this symbol redefines one in the
		  * previous name space which is being extended - in that case
		  * always use new symbol.  firstnewexp is used for this test.
                  */
                 if ((ulong)exp < (ulong)firstnewexp || kludge) {
			exp->fullhash += 1;        /* ruin existing entry */
			if (redefine)		   /* set redefine flag */
				*redefine = 1;
		 }
                 else {                            /* dont insert this entry */
			numexports-- ;
			continue;
		 }
              }
          } /* (kernel) */
          hash = ld_hash(symstring,&symlength);  /* also computes correct i
						    symlength */
          e_i[fi].sym = symstring;
          e_i[fi].syml = symlength;
          if (ldsym[i].l_parm){
          	typchk = (void*)(e_i[fi].typecheck =
		     (caddr_t)ldhdr + ldhdr->l_stoff+ldsym[i].l_parm) ;
          	if( ldsym[i].l_parm+10 > ldhdr->l_stlen ){
			if (IS_PRIVSEG(heap))
				ld_ufree(sspointer, lx);
			else
          			xmfree(lx,heap);
          		u.u_error = ENOEXEC;
          		return NULL;
          	}	
          	if (HASH(typchk) == UNIVERSAL && LANGHASH(typchk)==UNIVERSAL)
			e_i[fi].typecheck = NULL;
	  }
	  else
		e_i[fi].typecheck = NULL;
          if (!(locs[i + 3].v))
		e_i[fi].exp_location = (void *)-1;
          else  {
		e_i[fi].exp_location = 		/* compute relocated value*/
		   (char *)locs[i+3].d + ldsym[i].l_value;
		
		/* type propogation - if the exported symbol being defined is
		 * of universal type use the type of the defining symbol.
		 * I believe that the check for NULL is redundant since
		 * when non null the two must match.  However, I think
		 * the cost of checking is small and may be free or a
		 * win (avoid the store) - and I feel better doing it!
		 * locs[].v==1 when this is not an imported symbol.
		 * type propogation only occurs for exports of imports!
		 * Note we do not do propogate into a null general or lang
		 * specific hash, only propogate if the whole think is
		 * universal!
		 */
		if (1 != locs[i + 3].v && NULL == e_i[fi].typecheck )
			e_i[fi].typecheck =
			       ((struct exp_index*)locs[i+3].v)->typecheck;
          }
          e_i[fi].fullhash = hash;
          hash = hash & hash_mask;
          e_i[fi].next_index = hashes[hash];
          hashes[hash] = &e_i[fi];
          fi++;
          }
    ASSERT(fi == numexports);
    lx->numexports = fi;
    return lx;
}

/* finds an existing entry in the hash table and resets its exp_location value.
 * return 1 if value changed, zero otherwise.
 * this is needed since relocation is an iterative computation
 */
void
ld_updatehash(
	struct loader_exports	*lx,
	struct ldhdr	*ldhdr,
	struct ldsym	*ldsym,
	struct locs	*locs)	/*relocation value for this symbol*/
{
	struct exp_index       *e_i;
	int	rc;

	e_i = ld_hashfind(lx,ldhdr,ldsym,&rc,NULL) ;
	assert(e_i);
	e_i->exp_location = (char *)ldsym->l_value + locs->d;
	/* see comments in buildhash on type propogation */
	if (1 != locs->v && NULL == e_i->typecheck )
		e_i->typecheck = ((struct exp_index*) locs->v)->typecheck;
}
