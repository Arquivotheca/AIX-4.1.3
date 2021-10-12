static char sccsid[]="@(#)25   1.8  src/bos/kernext/cie/dmalloc.c, sysxcie, bos411, 9428A410j 4/27/94 15:05:14";

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   rotate
 *   hash
 *   dmallocInit
 *   dmallocTerm
 *   dmallocReport
 *   d_save
 *   d_attach
 *   d_release
 *   dmalloc
 *   dmfree
 *   d_get
 *   d_gethdr
 *   d_getclr
 *   d_clgetm
 *   d_free
 *   d_freem
 *
 * DESCRIPTION:
 *
 *    Debug Version of xmalloc/xmfree
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#if defined(DMALLOC)

#include "ciedd.h"
#include <stddef.h>
#include <unistd.h>
#include <sys/malloc.h>
#include <sys/m_param.h>
#include <sys/mbuf.h>
#include <net/spl.h>
#include "dmalloc.h"

#undef   xmalloc
#undef   xmfree
#undef   m_get
#undef   m_free
#undef   m_freem
#undef   m_gethdr
#undef   m_getclr
#undef   m_clgetm

/*---------------------------------------------------------------------------*/
/*                      Type definition for hash table                       */
/*---------------------------------------------------------------------------*/

typedef struct HTAB_ENTRY    HTAB_ENTRY;

struct HTAB_ENTRY
{
   HTAB_ENTRY              * next;
   MEM_TYPE                  type;
   void                    * addr;
   int                       size;
   int                       clust;
   char                    * file;
   char                    * func;
   int                       line;
};

/*---------------------------------------------------------------------------*/
/*                 Type definition for memory tracking arena                 */
/*---------------------------------------------------------------------------*/

typedef struct ARENA         ARENA;
typedef struct MEM_STATS     MEM_STATS;

struct MEM_STATS
{
   int                       nalloc;
   int                       nfree;
   int                       curmem;
   int                       maxmem;
};

struct ARENA
{
   int                       lock;        // Lock word
   unsigned long             htabSize;    // Size of hash table (entries)
   unsigned long             memSize;     // Size of mem table (entries)
   MEM_STATS                 stats[3];    // Statistics
   HTAB_ENTRY             ** htab;        // Ptr to hash table
   HTAB_ENTRY              * free;        // Htab entry free list head
   HTAB_ENTRY              * mem;         // Ptr to entry table
   HTAB_ENTRY              * memEnd;      // Ptr to end of entry table
};

/*---------------------------------------------------------------------------*/
/*                             Arena Declaration                             */
/*---------------------------------------------------------------------------*/

extern ARENA               * arena = NULL;

/*---------------------------------------------------------------------------*/
/*            Rotate an integer right a specified number of bits             */
/*---------------------------------------------------------------------------*/

/*:::::::::::::::::: Pseudo-function prototype for macro ::::::::::::::::::::

unsigned in
   rotate(
      unsigned int           x           ,// IO-Number to be rotated
      size_t                 n            // IO-Number of bits to rotate
   );

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

#define  rotate(x,n) (((unsigned int)(x) >> (n)) | ((unsigned int)(x) << (32-n)))
#define  hash(x,lim) ((rotate((x),7) ^ rotate((x),20)) % (lim))

/*---------------------------------------------------------------------------*/
/*                      Initialize the hash table arena                      */
/*---------------------------------------------------------------------------*/

void
   dmallocInit(
      unsigned long          htabSize    ,
      unsigned long          memSize
   )
{
   HTAB_ENTRY              * p;
   unsigned long             htabBytes;
   unsigned long             memBytes;
   int                       i;

   /*-----------------------------------------------*/
   /*  Round hash table size up to next power of 2  */
   /*-----------------------------------------------*/

   for (i=0; htabSize!=0; htabSize>>=1,i++);
   htabSize = 1 << (i-1);

   htabBytes = sizeof(HTAB_ENTRY *) * htabSize;
   memBytes  = sizeof(HTAB_ENTRY)   * memSize;

   assert((arena = xmalloc(sizeof(ARENA),2,pinned_heap)) != NULL);

   arena->lock     = 0;
   arena->htabSize = htabSize;
   arena->memSize  = memSize;

   assert((arena->htab = xmalloc(htabBytes,2,pinned_heap)) != NULL);
   assert((arena->mem  = xmalloc(memBytes ,2,pinned_heap)) != NULL);

   arena->memEnd = arena->mem+memSize;

   memset(&arena->stats,0x00,sizeof(arena->stats));
   memset(arena->htab  ,0x00,htabBytes);
   memset(arena->mem   ,0x00,memBytes);

   for (p=arena->mem+1; p < (arena->memEnd); p++) p->next = p-1;

   arena->free = arena->mem+memSize-1;
   dbgout("Arena at %x htab at %x mem at %x free->%x",
          arena,arena->htab,arena->mem,arena->free);
}

/*---------------------------------------------------------------------------*/
/*                      Release Memory tracking storage                      */
/*---------------------------------------------------------------------------*/

void
   dmallocTerm(
      void
   )
{
   FUNC_NAME(dmallocTerm);

   xmfree(arena->htab,pinned_heap);
   xmfree(arena->mem ,pinned_heap);
   xmfree(arena      ,pinned_heap);
}

/*---------------------------------------------------------------------------*/
/*                       Display storage usage report                        */
/*---------------------------------------------------------------------------*/

void
   dmallocReport(
      void
   )
{
   FUNC_NAME(dreport);

   register int              type;
   register int              i;
   register HTAB_ENTRY     * e;
   register int              n;

   static char             * heading[] =
      {
         "xmalloc",
         "mbuf",
         "mbuf Cluster"
      };

   for (type = 0; type<3; type++)
   {
      n = 0;

      dbgout("***** %s statistics:",heading[type]);
      dbgout("   alloc calls: %d",arena->stats[type].nalloc);
      dbgout("   free  calls: %d",arena->stats[type].nfree);
      dbgout("   current mem: %d",arena->stats[type].curmem);
      dbgout("   maximum mem: %d",arena->stats[type].maxmem);

      for (i=0; i<arena->htabSize; i++)
      {
         for (e=arena->htab[i]; e; e=e->next)
         {
            if (e->type == type)
               dbgout("Addr %x Size %d was allocated at %s %s %d",
                      e->addr,e->size,e->file,e->func,e->line);
            n++;
         }
      }

      if (!n) dbgout("No memory left allocated");
   }
}

/*---------------------------------------------------------------------------*/
/*                Save a memory allocation in the hash table                 */
/*---------------------------------------------------------------------------*/

void
   d_save(
      void                 * p           ,
      unsigned long          size        ,
      MEM_TYPE               type        ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   register int              hval = hash(p,arena->htabSize);
   register HTAB_ENTRY     * e;
   register HTAB_ENTRY    ** link = &(arena->htab[hval]);
   int                       lockedHere = 0;

   if (arena == NULL) return;

   assert(hval >= 0 && hval < arena->htabSize);

   if (csa->intpri > PL_IMP) lockedHere = i_disable(PL_IMP);

   for (e=arena->htab[hval]; e && e->addr < p; e=e->next) link = &(e->next);

   if (e != NULL) assert(e->addr != p);

   assert(arena->free != NULL);

   e           = arena->free;

   assert(e >= arena->mem);
   assert(e <  arena->memEnd);

   arena->free = e->next;
   e->next     = *link;
   *link       = e;

   e->type  = type;
   e->addr  = p;
   e->size  = size;
   e->clust = 0;
   e->file  = file;
   e->func  = func;
   e->line  = line;

   arena->stats[type].nalloc++;
   arena->stats[type].curmem += size;

   if (arena->stats[type].maxmem < arena->stats[type].curmem)
      arena->stats[type].maxmem = arena->stats[type].curmem;

   if (type == MBUF && M_HASCL((struct mbuf *)p))
   {
      e->clust    = MCLBYTES;

      arena->stats[MCLUST].nalloc++;
      arena->stats[MCLUST].curmem += MCLBYTES;

      if (arena->stats[MCLUST].maxmem < arena->stats[MCLUST].curmem)
         arena->stats[MCLUST].maxmem = arena->stats[MCLUST].curmem;
   }

   if (lockedHere) i_enable(lockedHere);
}

/*---------------------------------------------------------------------------*/
/*                Save a memory allocation in the hash table                 */
/*---------------------------------------------------------------------------*/

void
   d_attach(
      void                 * p           ,
      unsigned long          size        ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   register int              hval = hash(p,arena->htabSize);
   register HTAB_ENTRY     * e;
   register HTAB_ENTRY    ** link = &(arena->htab[hval]);
   int                       lockedHere = 0;

   if (arena == NULL) return;

   assert(hval >= 0 && hval < arena->htabSize);

   if (csa->intpri > PL_IMP) lockedHere = i_disable(PL_IMP);

   for (e=arena->htab[hval]; e && e->addr < p; e=e->next) link = &(e->next);

   assert(e >= arena->mem);
   assert(e <  arena->memEnd);

   assert(e->addr  == p);
   assert(e->type  == MBUF);
   assert(e->clust == 0);

   e->clust    = MCLBYTES;

   arena->stats[MCLUST].nalloc++;
   arena->stats[MCLUST].curmem += MCLBYTES;

   if (arena->stats[MCLUST].maxmem < arena->stats[MCLUST].curmem)
      arena->stats[MCLUST].maxmem = arena->stats[MCLUST].curmem;

   if (lockedHere) i_enable(lockedHere);
}

/*---------------------------------------------------------------------------*/
/*                        Release a memory allocation                        */
/*---------------------------------------------------------------------------*/

void
   d_release(
      void                 * p           ,
      MEM_TYPE               type
   )
{
   register int              hval = hash(p,arena->htabSize);
   register HTAB_ENTRY     * e;
   register HTAB_ENTRY    ** link = &(arena->htab[hval]);
   int                       lockedHere = 0;

   if (arena == NULL) return;

   assert(hval >= 0 && hval < arena->htabSize);

   if (csa->intpri > PL_IMP) lockedHere = i_disable(PL_IMP);

   for (e=arena->htab[hval]; e && e->addr < p; e=e->next) link = &(e->next);

   assert(e >= arena->mem);
   assert(e <  arena->memEnd);

   assert(e->addr == p);
   assert(e->type == type);

   *link       = e->next;
   e->next     = arena->free;
   arena->free = e;

   arena->stats[type].nfree++;
   arena->stats[type].curmem -= e->size;

   if (e->clust)
   {
      arena->stats[MCLUST].nfree++;
      arena->stats[MCLUST].curmem -= e->clust;
   }

   if (lockedHere) i_enable(lockedHere);
}

/*---------------------------------------------------------------------------*/
/*                         Debug version of xmalloc                          */
/*---------------------------------------------------------------------------*/

void *
   dmalloc(
      register int           size        ,
      register int           align       ,
      register void        * heap        ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   FUNC_NAME(dmalloc);

   register int              hval;
   register void           * p;

   p = xmalloc(size,align,heap);

   if (p) d_save(p,size,XMALLOC,file,func,line);

   return p;
}

/*---------------------------------------------------------------------------*/
/*                          Debug version of xmfree                          */
/*---------------------------------------------------------------------------*/

int
   dmfree(
      register void        * p           ,
      register void        * heap
   )
{
   FUNC_NAME(dmfree);

   if (p)
   {
      d_release(p,XMALLOC);

      assert(xmfree(p,heap) == 0);
   }

   return 0;
}

/*---------------------------------------------------------------------------*/
/*                          Debug version of m_get                           */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_get(
      int                    wait        ,// I -Wait Flag
      int                    type        ,// I -Block Type
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   struct mbuf             * m;

   m = m_get(wait,type);

   TRC_OTHER(mget,m,func,line);

   if (m) d_save(m,256,MBUF,file,func,line);

   return m;
}

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_gethdr                         */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_gethdr(
      int                    wait        ,// I -Wait Flag
      int                    type        ,// I -Block Type
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   struct mbuf             * m;

   m = m_gethdr(wait,type);

   TRC_OTHER(mghd,m,func,line);

   if (m) d_save(m,256,MBUF,file,func,line);

   return m;
}

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_getclr                         */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_getclr(
      int                    wait        ,// I -Wait Flag
      int                    type        ,// I -Block Type
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   struct mbuf             * m;

   m = m_getclr(wait,type);

   TRC_OTHER(mgec,m,func,line);

   if (m) d_save(m,256,MBUF,file,func,line);

   return m;
}

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_getclr                         */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_clgetm(
      struct mbuf          * m           ,// IO-mbuf to attach to
      int                    wait        ,// I -Wait Flag
      int                    size        ,// I -Cluster Size
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   int                       rc;

   if ((rc = m_clgetm(m,wait,size)) == 1)
      d_attach(m,size,file,func,line);

   TRC_OTHER(mgcl,m,func,line);

   return rc;
}

/*---------------------------------------------------------------------------*/
/*                          Debug version of m_free                          */
/*---------------------------------------------------------------------------*/

struct mbuf *
   d_free(
      struct mbuf          * m           ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   TRC_OTHER(mfre,m,func,line);

   if (m)
   {
      d_release(m,MBUF);
      return m_free(m);
   }
   else
      return NULL;
}

/*---------------------------------------------------------------------------*/
/*                         Debug version of m_freem                          */
/*---------------------------------------------------------------------------*/

void
   d_freem(
      struct mbuf          * m           ,
      register char        * file        ,
      register char        * func        ,
      register int           line
   )
{
   struct mbuf             * p;

   TRC_OTHER(mfrm,m,func,line);

   for (p = m ; p; p = p->m_next)
      d_release(p,MBUF);

   m_freem(m);
}

#endif
