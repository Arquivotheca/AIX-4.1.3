static char sccsid[] = "@(#)40	1.23  src/bos/usr/ccs/lib/libc/mon.c, libcgen, bos41J, 9516A_all 4/12/95 14:23:16";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: monstartup,moncontrol,monitor,
 *            monldq,monst1,monstn,pmondsiz,pmoninit,pmondwrt
 *            gmondsiz,gmoninit,gmondwrt
 *
 * ORIGINS: 10 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
*/

/*
 * Routines to support both  -p (prof) and -pg (gprof) profiling
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <filehdr.h>    /* text location and size calculation */
#include <aouthdr.h>    /* text location and size calculation */
#include <scnhdr.h>     /* text location and size calculation */
#include <stdarg.h>     /* var args */

#include <sys/ldr.h>    /* loadquery(), struct ld_info, L_GETINFO */
#include <string.h>     /* memcpy */
#include <errno.h>      /* ENOMEM */
#include <sys/param.h>  /* MAX(x,y) */
#include <mon.h>        /* profiling definitions */
#include <nl_types.h>

/*********** local debug vars
#define _MONCTL_DEBUG _MONCTL_DEBUG

#define _MONLDQ_DEBUG _MONLDQ_DEBUG

#define _MONSTU_DEBUG _MONSTU_DEBUG

#define _MONST1_DEBUG _MONST1_DEBUG
#define _MONSTN_DEBUG _MONSTN_DEBUG

#define _MONITOR_DEBUG _MONITOR_DEBUG

#define _PMONDSIZ_DEBUG _PMONDSIZ_DEBUG
#define _GMONDSIZ_DEBUG _GMONDSIZ_DEBUG

#define _PMONINIT_DEBUG _PMONINIT_DEBUG
#define _PMONDWRT_DEBUG _PMONDWRT_DEBUG

#define _GMONINIT_DEBUG _GMONINIT_DEBUG
#define _GMONDWRT_DEBUG _GMONDWRT_DEBUG
************/

#include "libc_msg.h"
#define MSG01 catgets(catd, MS_LIBC, M_MON1, \
                    "%s: No space for profiling buffer(s) (%u bytes)\n")
#define MSG02 catgets(catd, MS_LIBC, M_MON2, \
                    "%s: Error processing output data file: %s\n")
#define MSG03 catgets(MF_LIBC, MS_LIBC, M_MON3, \
                    "%s: Null address range specified Lo: %8.8x  Hi: %8.8x\n")
#define MSG04 catgets(catd, MS_LIBC, M_MON4, \
    "%s: _mondata.prof_type(%d) must be _PROF_TYPE_IS_P or _PROF_TYPE_IS_PG\n")
#define MSG05 catgets(catd, MS_LIBC, M_MON5, \
    "%s: No data area found. (Stopped without starting?)\n")
#define MSG06 catgets(catd, MS_LIBC, M_MON6, \
"%s: Buffer range not contiguous. p[%d]: %8.8x size[%d]: %8.8x p[%d]: %8.8x\n")
#define MSG07 catgets(catd, MS_LIBC, M_MON7, \
"%s: Ranges not in ascending order. hi[%d]: %8.8x lo[%d]: %8.8x\n")

/*
 * internal routine error codes
 */
#define _MON_ERR -1         /* an error occurred - function not completed */
#define _MON_OK 0           /* operation as expected */

#define IGNORED_ARG 0xffffffff /* used for don't care situations */

/* structure for arg for monldq call */
struct mldq {
        uint size;              /* total size */
        uint ldrbuf;            /* offset to loader buffer image from mldq */
        uint ldrsiz;            /* total size of loader buffer image */
        uint numldp;            /* number of loaded program modules */
        struct rng {
                caddr_t lpc;    /* low pc value - textorg */
                caddr_t hpc;    /* high pc value - textorg + textsize */
                } range[2];     /* real array size determined by numldp */
/* following the range array is the loader buffer as determined by ldrbuf */
/* size of the loader buffer given by ldrsiz */
};

#ifdef _THREAD_SAFE
/**********
  get _mondata from libc_t.a
**********/
extern struct monglobal _mondata;

#else
/*
 * Begin Global  variables ---------------------------------------------------
 */
/*  Profiling global data control definitions */
struct monglobal _mondata = {
        /* _mondata.prof_type     */ 0, /* p,pg,none */
        /* _mondata.profiling     */ 3, /* used by mcount */
        /* _mondata.sbuf          */ 0, /* fixed out table base address */
        /* _mondata.ssiz          */ 0, /* fixed out table byte size */
        /* _mondata.hbuf          */ 0, /* time ctr (hist) table base addr */
        /* _mondata.hsiz          */ 0, /* time ctr (hist) table byte size */
        /* _mondata.profbuf       */ 0, /* implicit arg for moncontrol */
        /* _mondata.profbufsiz    */ 0, /* implicit arg for moncontrol */
        /* _mondata.proflopc      */ 0, /* implicit arg for moncontrol */
        /* _mondata.profscale     */ 0, /* implicit arg for moncontrol */
        /* _mondata.cb.pcountbase */ 0, /* func cnt data area control */
        /* _mondata.cl.pcountlimit*/ 0, /* func cnt data area control */
        /* _mondata.pnextcounter  */ 0, /* func cnt data area control */
        /* _mondata.monstubuf     */ 0, /* data area allocated by monstartup */
        };
/*
 * End   Global  variables ---------------------------------------------------
 */
#endif /* THREAD_SAFE */


/*
 * Begin Private variables ---------------------------------------------------
 */

nl_catd catd;		   /* message catalog descriptor */
char *OutFileP=MON_OUT;    /* output data file name for -p (prof) profiling */
char *OutFilePG=GMON_OUT;  /* output data file name for -pg (prof) profiling */

static char msgidmonstartup[]="monstartup"; /* function name for err msg */
static char msgidmonitorstart[]=
                               "monitor: start";/* function name for err msg */
static char msgidmonitorstop[]=
                               "monitor: stop"; /* function name for err msg */

/*
 * End   Private variables ---------------------------------------------------
 */


/*
 * NAME: DUMPHEX
 *
 * FUNCTION: Storage dump in hex and ascii
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called only by debug statements.
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) none
 *
 * RETURNS: none
 */

static char printchar(a)
char *a;
{
char ref='A';

if ( (int)ref == 65 ) { /* ! ascii */
   if ( ( (int)*a > 31 ) && ( (int)*a < 127 ) ) return(*a);
   else return(' ');
   }
else {                /* ebcdic */
   if (     (int)*a == 64
      || (( (int)*a >  73) && ( (int)*a <  81))
      || (( (int)*a >  89) && ( (int)*a <  98))
      || (( (int)*a > 106) && ( (int)*a < 112))
      || (( (int)*a > 121) && ( (int)*a < 128))
      || (( (int)*a > 128) && ( (int)*a < 138))
      || (( (int)*a > 144) && ( (int)*a < 154))
      || (( (int)*a > 161) && ( (int)*a < 170))
      || (( (int)*a > 192) && ( (int)*a < 202))
      || (( (int)*a > 208) && ( (int)*a < 218))
      || (( (int)*a > 225) && ( (int)*a < 234))
      || (( (int)*a > 239) && ( (int)*a < 250)) )  return(*a);
   else return(' ');
   }
} /* end proc printchar; */

static void formatline(p,l)
char *p;
char *l;
{
      int i;
      char *lp;

      lp = l;
      sprintf(lp," %8.8x ",p); /* addr  */
      lp +=10;
      sprintf(lp,"%8.8x ",*(long *)p);/* 0 - 3 */
      lp +=9;
      p +=4;
      sprintf(lp,"%8.8x ",*(long *)p);/* 4 - 7 */
      lp +=9;
      p +=4;
      sprintf(lp,"%8.8x ",*(long *)p);/* 8 - C */
      lp +=9;
      p +=4;
      sprintf(lp,"%8.8x |",*(long *)p);/* C - F */
      lp +=10;
      p -=12;
      for ( i=0; i< 8; i++ ) {
         *(lp++) = printchar( p++ );
         }
      *(lp++) = ' ';
      for ( i=0; i< 8; i++ ) {
         *(lp++) = printchar( p++ );
         }
      *(lp++) = '|';
      *lp = '\0';
} /* end proc formatline;*/

static void displayline( line )
char *line;
{
   printf("%s\n",line);
} /* end proc displayline; */
static void DUMPHEX(
        caddr_t  addr,
        uint     numbytes
        )
{
   extern void formatline();
   extern void displayline();
   extern char printchar();

static
   char           dupline[]="             duplicate line(s)";
static
   char           hdr[64]=
"          0        4        8        C         0   4    8   C";
/*
 ----+----1----+----2----+----3----+----4----+----5----+----6----+----7
  00000000 00000000 00000000 00000000 00000000 |12341234 12341234|
  */
   unsigned long *paddr;
   unsigned long  loadr;
   unsigned long  hiadr;
   unsigned long  s[4];
   int            linelen;
   char           line[78];
   int            zline;
if ( numbytes == 0) return;

loadr=(unsigned long)addr & 0xfffffff0;        /* lower dump limit */
hiadr=((unsigned long)addr + numbytes -1)| 0xf;/* upper dump limit */
paddr=(unsigned long *)loadr;                  /* running address */

zline = 0;                        /* last line not same */
printf("%s\n", hdr  );

s[0]= ~(*(paddr+0));
s[1]= ~(*(paddr+1));
s[2]= ~(*(paddr+2));
s[3]= ~(*(paddr+3));

while((unsigned long)paddr <= hiadr){
   if (  (*(paddr+0) == s[0]) &&  /* if same as last */
         (*(paddr+1) == s[1]) &&
         (*(paddr+2) == s[2]) &&
         (*(paddr+3) == s[3])     )     {
      zline = zline +1; /* same as last */
      if ( zline == 2 ) {
            strcpy(line,dupline); /* many same lines flag */
            displayline( line );
            } /* if */
       else if ( zline > 2 ) {
           zline = 4; /* skip others and prevent overflow */
           } /*else if */
       } /* if */
   else { /* not same as last */
      if (zline != 0 ) {
         if ( zline == 2 ) {
            formatline((char *)(paddr-8),line);
            displayline( line );
            } /*if*/
         formatline((char *)(paddr-4),line);
         displayline( line );
         } /*if*/
      zline = 0; /* reset */
      s[0]= *(paddr);
      s[1]= *(paddr+1);
      s[2]= *(paddr+2);
      s[3]= *(paddr+3);
      formatline((char *)paddr, line);
      displayline( line );
      } /*else*/
   paddr=paddr+4;
   } /* while */
if ( zline > 1) {
   formatline((char *)(paddr-4),line);
   displayline( line );
   }
printf("%s\n",hdr);
return;
} /* end proc dumphex; */

/*
 * NAME: cmp64
 *
 * FUNCTION: Compares 64 bit addresses
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by qsort call
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: -1 arg1 < arg2, +1 arg1 > arg2, 0 arg1 == arg2
 */
static int cmp64(
        caddr_t *a1,       /* first address */
        caddr_t *a2        /* second address */
        )
{
        caddr_t *p1;
        caddr_t *p2;
        p1=a1;
        p2=a2;
        if ( *p1 > *p2 ) return ( +1 );
        if ( *p1 < *p2 ) return ( -1 );
        if ( *(++p1) > *(++p2) ) return ( +1 );
        if ( *p1 < *p2 ) return ( -1 );
        return( 0 );
} /* end function cmp64 */

/*
 * NAME: gmondsiz
 *
 * FUNCTION: Computes size of function call count data area needed for
 *            -pg (gprof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by monst1 and monstn
 *
 * (NOTES:)
 *      Based on BSD4.3 generalized for multiple profiling ranges.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and Global definitions.
 *
 * RETURNS: Total size in bytes needed for the range(s) specified.
 */
static uint gmondsiz(
        uint nrng,       /* number of ranges */
        struct frag *tf, /* range table */
        uint *totalsize  /* total size required  - bytes */
        )
{

        uint fromsize;  /* size for the froms - bytes */
        uint tosize;    /* size for the tos - bytes */
        uint tolimit;   /* max num to block spaces */
        uint range;     /* profiling pc range */
        int   i;         /* index */

#ifdef _GMONDSIZ_DEBUG
char mycall[]="gmondsiz (pg)";
printf("%s: in  nrng %8.8x tf %8.8x\n",mycall,nrng,tf);
for (i=0; i<nrng; i++){
   printf("%s: i %u lo %8.8x hi %8.8x\n",mycall,i,tf[i].p_low,tf[i].p_high);
   } /* endfor */
#endif

        *totalsize = 0;
        for(i=0; i<nrng; i++ ) { /* for all ranges defined */
                range = tf[i].p_high - tf[i].p_low;
                fromsize = (long)FROM_STG_SIZE(range );
                tolimit = (long)TO_NUM_ELEMENTS( range );
                if ( tolimit < MINARCS ) {
                    tolimit = MINARCS;
                    }
                else if ( tolimit > (TO_MAX-1) ) {
                    tolimit = (TO_MAX-1);
                    }
                tosize =  tolimit * sizeof( struct tostruct ) ;
                *totalsize += fromsize + tosize;

#ifdef _GMONDSIZ_DEBUG
printf("%s: i %u range %8.8x fromsize %8.8x tosize %8.8x totalsize %8.8x\n",
mycall,i+1, range,fromsize,tosize,*totalsize);
#endif

                } /* end for all ranges defined */

        *totalsize += nrng * sizeof(struct gfctl); /* + ctl struct */

#ifdef _GMONDSIZ_DEBUG
printf("%s: + sizeof(struct gfctl) %8.8x totalsize %8.8x\n",
mycall,sizeof(struct gfctl),*totalsize);
#endif

        return( *totalsize );

} /* end function gmondsiz */

/*
 * NAME: gmoninit
 *
 * FUNCTION: Initializes the data required for function call counts for
 *           -pg (gprof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by monitor.
 *
 * (NOTES:)
 *      Based on BSD4.3 generalized for multiple profiling ranges.
 *      The link field of the first tos element is used as an allocation
 *      counter for the tos space.  The last element of the tos space is
 *      unused due to the manner of element allocation.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and Global definitions.
 *
 * RETURNS: _MON_OK if all went well, else _MON_ERR
 */

static int gmoninit(
        caddr_t buf,      /* address of the buffer to init space */
        uint bufsiz,     /* buffer size in bytes */
        uint cntsize,     /* number of bytes to use for func call cnt space */
        uint nrng,        /* number of ranges */
        struct outrng *tg,/* range table */
        uint *totalsize  /* size of buf used for func call counters */
        )                 /* returns 0 if ok */
{

        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */

        uint buflimit = (uint)buf+(MIN(cntsize,bufsiz)); /* upper buf limit*/
        struct gfctl *tb;/* from/to control structure array */
        caddr_t ftp;     /* running pointer into from/to data area */
        uint fromsize;  /* size for the froms - bytes */
        uint tosize;    /* size for the tos - bytes */
        uint range;     /* profiling pc range */
        uint rngidx;     /* range index */
        uint fromsidx;   /* froms index */
	uint tolimit;	/* max num to block spaces */

#ifdef _GMONINIT_DEBUG
        char mycall[]="gmoninit";
        uint k;          /* index */
#endif

        /* build the data control structures in the buffer */
        tb = (struct gfctl *)buf; /* base address struct ctl array */
        z->cb.pgcountbase = tb; /* init external data anchor */
        z->cl.pgcountlimit = &(tb[nrng]); /* init range limit test value*/
        if ( ((uint)(&(tb[nrng]))) >= buflimit ) { /* buffer too small */
            goto error; /* go print msg and return err flag */
            }
        ftp = (caddr_t)(&(tb[nrng])); /* base addr of froms and tos space */
        *totalsize=0;                  /* init for accum */
        for(rngidx=0; rngidx<nrng; rngidx++ ) { /* for all ranges defined */
            range = tg[rngidx].hpc - tg[rngidx].lpc; /* pc range for range */
            /* set up froms control */
            fromsize = (long)FROM_STG_SIZE(range ); /* bytes */
            tb[rngidx].froms = (HASH_LINK *)ftp;    /* hash table(s) space  */
            tb[rngidx].fromlowpc = tg[rngidx].lpc;  /* low pc for hash */
            tb[rngidx].fromlimit =                  /* hash offset limit */
                         (uint)(FROM_INDEX(tg[rngidx].hpc,tg[rngidx].lpc));

            /* init all the hash chain heads to empty */
            /* for all heads this range */
            for (fromsidx=0; fromsidx<tb[rngidx].fromlimit; fromsidx++ ){
                tb[rngidx].froms[fromsidx] = _MONFromChainEnd; /* mark empty */
                } /* endfor */

            /* set up tos control */
            tb[rngidx].tos = (struct tostruct *)
                                    ((uint)ftp + fromsize);/* chain area */
            tolimit = (uint)TO_NUM_ELEMENTS( range ); /* # elems */
            if ( tolimit < MINARCS ) {
                tolimit = MINARCS;
                }
            else if ( tolimit > (TO_MAX-1) ) {
                tolimit = (TO_MAX-1);
                }
            tb[rngidx].tolimit = (HASH_LINK)tolimit;
            tosize = sizeof(struct tostruct) * tb[rngidx].tolimit; /* bytes */

            if ( ((uint)(&tb[rngidx].tos[0].link)
                                         + sizeof(HASH_LINK)) >= (buflimit) ){
                /* error - not enough space */
                goto error; /* go print msg and return err flag */
                }
            else { /* ok - still in buffer */
                /* first element used as allocation counter */
                tb[rngidx].tos[0].link = 0; /* init as none allocated */
                } /* endif */
            ftp =(caddr_t)((uint)ftp + fromsize + tosize);/* next range adr */
            *totalsize += fromsize + tosize; /* accum the size */

#ifdef _GMONINIT_DEBUG
printf(
"%s: rngidx %u range %8.8x fromsize %8.8x tosize %8.8x totalsize %8.8x\n",
mycall,rngidx+1,range,fromsize,tosize,*totalsize);
#endif

            } /* end for all ranges */

            *totalsize += nrng * sizeof(struct gfctl); /* grand total */
            /* make sure it will fit */
            if ( (uint)(ftp) <= buflimit ){ /* fits in buffer */

#ifdef _GMONINIT_DEBUG
printf("%s: + sizeof(struct gfctl) %8.8x totalsize %8.8x\n",
mycall,sizeof(struct gfctl),*totalsize);
#endif

#ifdef _GMONINIT_DEBUG
printf(
    "%s: returning OK - z %8.8x\n",mycall,z);
printf(
    "%s: z->cb.pgcountbase %8.8x z->cl.pgcountlimit %8.8x totalsize %8.8x\n",
    mycall,z->cb.pgcountbase,z->cl.pgcountlimit,*totalsize);
for (tb=z->cb.pgcountbase, rngidx=0; rngidx<nrng ; rngidx++ ){
printf("%s: rngidx %d froms %8.8x fromlowpc %8.8x fromlimit %8.8x\n",
     mycall,rngidx,tb[rngidx].froms,tb[rngidx].fromlowpc,tb[rngidx].fromlimit);
printf("%s:     tos   %8.8x tolimit  %8.8x tos.link  %8.8x\n",
      mycall,tb[rngidx].tos,tb[rngidx].tolimit,tb[rngidx].tos[0].link);
} /* endfor */
#endif

                return( _MON_OK ); /* good return - data init ok */
                }

error:

#ifdef _GMONINIT_DEBUG
printf("%s: returning ERR - tb %8.8x ftp %8.8x buflimit %8.8x\n",
mycall,tb,ftp,buflimit);
#endif
	catd = catopen(MF_LIBC,NL_CAT_LOCALE);
        fprintf(stderr,MSG01,msgidmonitorstart,*totalsize-MIN(cntsize,bufsiz));
	catclose(catd);
        return(_MON_ERR); /* no room for all parts */
} /* end function gmoninit */

/*
 * NAME: gmondwrt
 *
 * FUNCTION: Writes out the count data accumulated during profiling for
 *           -pg (gprof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by monitor.
 *
 * (NOTES:)
 *      Based on BSD4.3 generalized for multiple profiling ranges.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and Global definitions.
 *
 * RETURNS: _MON_OK if all went well, else _MON_ERR
 */

static int gmondwrt(
        int fd    /* file handle */
        )         /* returns 0 if ok */
{
        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */
        struct gfctl *p;        /* running pointer in control array */
        uint     fromindex;     /* temp from hash index */
        caddr_t  frompc;        /* temp from function address */
        uint     toindex;       /* temp to element index */
        struct goutcnt rawarc;  /* counter for each arcs data for output */
        int rc;                 /* return code from write */

#ifdef _GMONDWRT_DEBUG
char mycall[]="gmondwrt (pg)"; /* funct name for err msg */
fprintf(stderr,
   "%s z->cb.pgcountbase %8.8x z->cl.pgcountlimit %8.8x\n"
   ,mycall,z->cb.pgcountbase,z->cl.pgcountlimit);
#endif

        rc=0; /* init return code in case of no writes */
        /* for all program ranges defined */
        for ( p = z->cb.pgcountbase; p < z->cl.pgcountlimit; p++) {
            /* for all the chain heads in the hash table */
            for ( fromindex = 0 ; fromindex < p->fromlimit ; fromindex++ ) {
                if ( p->froms[fromindex] == _MONFromChainEnd ) {
                    continue; /* this from chain empty */
                    }
                /* from function address */
                frompc = p->fromlowpc + (fromindex * (MIN_SUBR_SPAN));

                /* for all the to function count blocks on this chain */
                for (toindex=p->froms[fromindex]; toindex != _MONFromChainEnd;
                                               toindex=p->tos[toindex].link) {
#ifdef _GMONDWRT_DEBUG
fprintf( stderr , "%s frompc 0x%x selfpc 0x%x count %d\n" , mycall , frompc ,
                              p->tos[toindex].selfpc , p->tos[toindex].count );
#endif
                    /* build the arc count counter and write it out */
                    rawarc.raw_frompc = (unsigned long) frompc;
                    rawarc.raw_selfpc = (unsigned long) p->tos[toindex].selfpc;
                    rawarc.raw_count = p->tos[toindex].count;
                    rc = (write(fd,&rawarc,sizeof(rawarc))-sizeof(rawarc));
                    if ( rc != 0 ) {
                        return(rc); /* write error */
                        }
                    } /* end for all to blocks this chain */
                } /* end for all from chains */
            } /* end for all program ranges being profiled */
        return(rc); /* good return - no error */
} /* end function gmondwrt */




/*
 * NAME: pmondsiz
 *
 * FUNCTION: Computes size of function call count data area needed for
 *           -p (prof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by monst1 and monstn
 *
 * (NOTES:)
 *      Based on BSD4.3 and AIX version 2.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and Global definitions.
 *
 * RETURNS: Total size in bytes needed for the range(s) specified.
 */
static uint pmondsiz(
        uint nrng,       /* number of ranges */
        struct frag *tf, /* range table */
        uint *numctrs    /* number of counters to allocate */
        )                /* returns size required in bytes */
{

        int i;           /* index */
	char *env_max;	 /* Max number of counters from ENV */

        *numctrs = 0;
        for(i=0; i<nrng; i++ ) { /* for all ranges defined */
                *numctrs += TO_NUM_ELEMENTS( tf[i].p_high - tf[i].p_low);
                } /* end for all ranges */
        if (*numctrs < MINARCS)       /* get at least min num of call ctrs */
                *numctrs = MINARCS;
        if (*numctrs > MAXARCS) {     /* no more than maximum */
		if (env_max = getenv("MAXARCS")) {
			if ((*numctrs = atol(env_max)) < MAXARCS)
				*numctrs = MAXARCS;
		}
		else
				*numctrs = MAXARCS;
	}
        return( *numctrs * sizeof(struct poutcnt) ); /* bytes */

} /* end function pmondsiz */



/*
 * NAME: pmoninit
 *
 * FUNCTION: Initializes the data required for function call counts for
 *           -p (prof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by monitor.
 *
 * (NOTES:)
 *      Based on BSD4.3 and AIX version 2.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h and Global definitions.
 *
 * RETURNS: _MON_OK if all went well, else _MON_ERR
 */

static int pmoninit(
        caddr_t buf,      /* address of the buffer to init space */
        uint bufsiz,     /* max buffer size in bytes - includes hist */
        uint numctrs,     /* number of func call counters to allocate */
        uint nrng,        /* number of ranges - unused here */
        struct outrng *tg,/* range table - unused here */
        uint *cntsize    /* num bytes of buffer used for func call cntrs */
        )                 /* returns 0 if ok */
{

        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */
        int i;                              /* index */


#ifdef _PMONINIT_DEBUG
        static char mycall[]="pmoninit (p)"; /* func name for debug msg */
printf(
   "%s: in buf %8.8x bufsize %8.8x numctrs %8.8x nrng %8.8x range tbl %8.8x\n",
mycall,buf,bufsiz,numctrs,nrng,tg);
#endif

        /* compute the size of the func call counter buffer */
        *cntsize = numctrs * sizeof(struct poutcnt); /* bytes */

        /* init control variables */
        z->cb.pcountbase = (struct poutcnt *)buf; /* counters base address */
        z->pnextcounter = z->cb.pcountbase;       /* mcount running pointer */
        z->cl.pcountlimit = &(z->cb.pcountbase[numctrs]); /* mcount limit e */
	/**********
	  The correct fix is setting **cntp=0 in __mcount when
	  *cntp is allocated the first time.  This is not a 
	  possible solution at this time so......
	  buf needs to be cleared out or there is a chance that
	  the count field will not be clear and the call count
	  will be bogus. 
	**********/
	bzero(buf, numctrs * sizeof(*z->cb.pcountbase));

#ifdef _PMONINIT_DEBUG
printf(
    "%s: z %8.8x\n",mycall,z);
printf(
"%s: out z->cb.pcountbase %8.8x z->pnextcounter %8.8x z->cl.pcountlimit %8.8x cntsize %8.8x\n",
mycall,z->cb.pcountbase,z->pnextcounter,z->cl.pcountlimit,*cntsize);
#endif

        /* make sure it will fit */
        if ( *cntsize > bufsiz  ) {
		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                fprintf(stderr,MSG01,msgidmonitorstart,*cntsize-bufsiz);
		catclose (catd);

#ifdef _PMONINIT_DEBUG
printf("%s: returning err\n",mycall);
#endif

                return( _MON_ERR ); /* no room for all parts */
                }


#ifdef _PMONINIT_DEBUG
printf("%s: returning ok\n",mycall);
#endif

        return( _MON_OK );
} /* end function pmoninit */

/*
 * NAME: pmondwrt
 *
 * FUNCTION: Writes out the count data accumulated during profiling for
 *           -p (prof) profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function.  User process.  Called by monitor.
 *
 * (NOTES:)
 *      Based on BSD4.3 and AIX version 2.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h.
 *
 * RETURNS: _MON_OK if all went well, else _MON_ERR
 */

static int pmondwrt(
        int fd  /* file handle */
        )       /* return 0 if ok */
{
        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */
        int size;

#ifdef _PMONDWRT_DEBUG
char mycall[]="pmondwrt (p)";
#endif

        size = sizeof(struct poutcnt) * (MIN((z->pnextcounter-z->cb.pcountbase)
                                      ,(z->cl.pcountlimit-z->cb.pcountbase)) );

#ifdef _PMONDWRT_DEBUG
printf(
  "%s: z->cb.pcountbase %8.8x z->pnextcounter %8.8x z->cl.pcountlimit %8.8x size %8.8x\n",
mycall,z->cb.pcountbase,z->pnextcounter,z->cl.pcountlimit,size);
DUMPHEX((caddr_t)z->cb.pcountbase,(uint)size);
#endif


        return( write(fd, z->cb.pcountbase,size) - size );
} /* end function pmondwrt */

/*
 * NAME: monst1
 *
 * FUNCTION: Allocates data areas for single range profiling call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function. User process.  Called by monstartup.
 *
 * (NOTES:) Uses malloc for allocation.
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) See mon.h
 *
 * RETURNS: _MON_OK if all went well.  _MON_ERR otherwise, and profiling not
 * started.
 */
static int monst1(
        caddr_t lpc, /* low address of range */
        caddr_t hpc  /* high address of range */
        )            /* returns _MON_OK or _MON_ERR */
{


        extern uint pmondsiz( uint, struct frag *, uint *);
        extern uint gmondsiz( uint, struct frag *, uint *);

        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */

        caddr_t buffer;     /* buffer to pass to monitor */
        uint fcnumrsiz;     /* num of call cntrs(p)/byte size of area(pg) */
        uint callcntsize;   /* bytes of storage for funct call cnt data */
        uint histsize;      /* bytes of storage for hist counters to alloc */
        uint monsize;       /* total bytes of buffer storage */
        uint bufsizctrs;    /* buffer size in number of hist counters */
        uint pcrange;       /* range of storage addresses to profile */
        struct frag irng;   /* rounded range limits */
        int rc;             /* monitor return code */

#ifdef _MONST1_DEBUG
        char mycall[]="monst1";
        printf("%s: entry\n",mycall);
#endif


/*
 *      round lowpc and highpc to multiples of the density we're using
 *      so the rest of the scaling (here and in gprof) stays in ints.
 */
        irng.p_high = (caddr_t)
                ROUNDUP((uint)hpc, (uint)INST_CNT_SIZE );
        irng.p_low = (caddr_t)
                ROUNDDOWN((uint)lpc, (uint)INST_CNT_SIZE );
        pcrange = (uint)irng.p_high - (uint)irng.p_low;

        /* hist buffer space */
        histsize = HIST_STG_SIZE( pcrange );

        /* calculate the call count data size required */
        if ( z->prof_type == _PROF_TYPE_IS_P){
                callcntsize = pmondsiz((uint)1,&irng,&fcnumrsiz);
                }
        else { /* z->prof_type == _PROF_TYPE_IS_PG */
                callcntsize = gmondsiz((uint)1,&irng,&fcnumrsiz);
                }

        /* hist and call cnt buffer size  in bytes */
        monsize = histsize + callcntsize;

#ifdef _MONST1_DEBUG
        printf("%s: histsize %8.8x cntsize %8.8x monsize %8.8x\n",
        mycall,histsize,callcntsize,monsize);
#endif

        buffer = (caddr_t)malloc(monsize);/* allocate buffer */
        if (buffer == (caddr_t)NULL) {
                /* "No space for monitor buffer(s)\n" */
		catd = catopen(MF_LIBC,NL_CAT_LOCALE);
                fprintf(stderr, MSG01,msgidmonstartup,monsize);
		catclose(catd);
                return(_MON_ERR);
                } /* endif */
        z->monstubuf = buffer; /* save for monitor free */

#ifdef _MONST1_DEBUG
        printf("%s: Calling monitor with:\n \
     lowpc: %8.8x  highpc: %8.8x  buf: %8.8x  bufsiz: %8.8x  nfunc: %u %8.8x\n",
           mycall,irng.p_low,irng.p_high,buffer,monsize,fcnumrsiz,fcnumrsiz);
        printf("%s: exit\n",mycall);
#endif
        /* buffer size as number of histcounters */
        bufsizctrs = monsize/HIST_COUNTER_SIZE;
        /* start profiling */
        rc = monitor(irng.p_low, irng.p_high, buffer, bufsizctrs, fcnumrsiz);

        if ( rc != _MON_OK ) { /* in case of err release buffer and set flag */
           free( z->monstubuf );
           z->monstubuf = NULL;
           }

        return( rc );

} /* end function monst1 */

/*
 * NAME: monstn
 *
 * FUNCTION: Allocates data areas for multiple range profiling call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function. User process.  Called by monstartup.
 *
 * (NOTES:) Uses malloc for allocation.
 *
 * (RECOVERY OPERATION:) Returns _MON_ERR on any failure.
 *
 * (DATA STRUCTURES:) See mon.h
 *
 * RETURNS: _MON_OK if all went well.  _MON_ERR otherwise, and profiling not
 * started.
 */
static int monstn(
        struct frag *pfrag, /* multi range input array - last elem hi=0 */
        uint numrngs        /* number of actual ranges in input *pfrag */
        )                   /* returns _MON_OK or _MON_ERR */
{
        extern uint pmondsiz( uint, struct frag *, uint *);
        extern uint gmondsiz( uint, struct frag *, uint *);

        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */

        struct prof *pprof; /* non-contig range def array out to monitor */
        int i;              /* index */

        caddr_t buffer;     /* buffer/prof struct to pass to monitor */
        caddr_t bufp;       /* buffer for non-contig case */
        uint fcnumrsiz;     /* num of call cntrs(p)/byte size of area(pg) */
        uint callcntsize;   /* bytes of storage for funct call cnt data */
        uint histsize;      /* bytes of storage for hist counters to alloc */
        uint monsize;       /* total bytes of buffer storage */
        uint numhistctrs;   /* buffer size in number of hist counters */
        uint profsize;      /* total bytes of prof struct storage */
        uint pcrange;       /* range of storage addresses to profile */
        struct frag *rndfrag; /* multi range input array - last elem hi=0 */
        int rfragsize;       /* size in bytes to allocate */
        int rc;             /* return code from monitor */

#ifdef _MONSTN_DEBUG
        char mycall[]="monstn";
        uint dcount;
        printf("%s: entry\n",mycall);
#endif

	catd = catopen(MF_LIBC,NL_CAT_LOCALE);

        /* allocate the prof buffer for the call to monitor */
        profsize = (numrngs+1)*sizeof(struct prof);
        buffer = (caddr_t)malloc( profsize );
        if (buffer == (caddr_t)NULL) { /* if not available */
             /* "No space for monitor buffer(s)\n" */
             fprintf(stderr, MSG01,msgidmonstartup,profsize);
	     catclose(catd);
             return(_MON_ERR);
             } /* endif */

        /* compute the space required for HIST buffers */
        histsize = 0;       /* init HIST buffer space needed */
        /* Fill in the prof structure limit and range info for monitor call */
        pprof = (struct prof *)buffer;              /* prof structure */

        /* allocate frag struct for rounded values for gmondsiz call */
        rfragsize = numrngs*sizeof(struct frag);
        rndfrag = (struct frag *)malloc( rfragsize );
        if ( rndfrag == NULL ) { /* no space */
             /* "No space for monitor buffer(s)\n" */
             fprintf(stderr, MSG01,msgidmonstartup,rfragsize);
	     catclose(catd);
             free( buffer ); /* return prof arg space */
             return(_MON_ERR);
             }

        /* Do some sanity checking and fill in the range data */
        for (i=0; i<numrngs; i++,pprof++) {         /* for each range */
             /* make sure range is at least a range */
             if ( pfrag[i].p_high <= pfrag[i].p_low ){ /* range not a range */
                /* "%s: Null address range specified Lo: %8.8x  Hi: %8.8x\n" */
                fprintf(stderr, MSG03,msgidmonstartup,
                                               pfrag[i].p_low,pfrag[i].p_high);
	        catclose(catd);
                free(rndfrag); /* free the rounded frag struct */
                free( buffer ); /* return prof arg space */
                return(_MON_ERR);
                }
             /* verify ascending order */
             if ( i != 0 ){ /* not first range */
                if ( pfrag[i].p_low < pfrag[i-1].p_high ){
                   /* "%s: Ranges not in ascending order Lo: %8.8x  Hi: %8.8x\n" */
                   fprintf(stderr, MSG07,msgidmonstartup,
                                       pfrag[i-1].p_high,i-1,pfrag[i].p_low,i);
	           catclose(catd);
                   free(rndfrag); /* free the rounded frag struct */
                   free( buffer ); /* return prof arg space */
                   return(_MON_ERR);
                   }
                }

             /*
              * round lowpc and highpc to multiples of the density we're using
              * so the rest of the scaling (here and in gprof) stays in ints.
              */
             pprof->p_high = (caddr_t)              /* range highpc */
                      ROUNDUP((uint)pfrag[i].p_high, (uint)INST_CNT_SIZE );
             rndfrag[i].p_high = pprof->p_high;
             pprof->p_low = (caddr_t)               /* range lowpc */
                      ROUNDDOWN((uint)pfrag[i].p_low, (uint)INST_CNT_SIZE );
             rndfrag[i].p_low = pprof->p_low;
             pcrange = pprof->p_high - pprof->p_low;/* this pc range */
             histsize += HIST_STG_SIZE( pcrange );  /* accum hist buffers */
             } /* endfor */

        /*
         * Note:  System function profil requires that the buffers for
         * non-contiguous profiling ranges be contiguous.  Hence the hist
         * buffer is allocated in one chunk.
         *
         * An undocumented AIX version 2 convention put the call count
         * buffer in with the first range histogram buffer for the call
         * to monitor.  We continue that convention but document it.
         *
         * With -pg (gprof) profiling in BSD4.3, the function call count data
         * area was not passed to monitor.  monitor served only as an intrface
         * to profil.  Here we pass this data to monitor where the data space
         * is initialized as is/was done for -p (prof) profiling in AIX
         * version 2.  This makes makes monstartup and monitor both full
         * function user interfaces.
         */

        /* fcnumnsiz is num ctrs for p (prof) and bytes for pg (gprof) */
        /* calculate the call count data size required in bytes */
        if ( z->prof_type == _PROF_TYPE_IS_P){
                /* fcnumrsiz is an output */
                callcntsize = pmondsiz(numrngs,rndfrag,&fcnumrsiz);
                }
        else { /* z->prof_type == _PROF_TYPE_IS_PG */
                /* fcnumrsiz is an output */
                callcntsize = gmondsiz(numrngs,rndfrag,&fcnumrsiz);
                }

        monsize = histsize + callcntsize; /* total bytes for all ranges */
        free(rndfrag); /* free the rounded frag struct - done with it */

#ifdef _MONSTN_DEBUG
        printf("%s: histsize %8.8x cntsize %8.8x monsize %8.8x\n",
        mycall,histsize,callcntsize,monsize);
#endif

        /* allocate the buffer for the hist and funct call counts */
        bufp = (caddr_t)malloc(monsize);
        if (bufp == (caddr_t)NULL) { /* if not available */
             free( buffer ); /* free prof storage */
             /* "No space for monitor buffer(s)\n" */
             fprintf(stderr, MSG01,msgidmonstartup,monsize);
	     catclose(catd);
             return(_MON_ERR);
             } /* endif */
        z->monstubuf = bufp; /* save for monitor free */


        /* Fill in the prof structure info for monitor call */
        pprof = (struct prof *)buffer;              /* prof structure */

        /* fill in hist range data and funct call count data*/
        for (         i=0;                          /* index */
                      i < numrngs;                  /* for hist ranges */
                      bufp += pprof->p_bufsize      /* next range buffer */
                                * HIST_COUNTER_SIZE,
                      pprof++,i++)                  /* bump ptrs,index */
           {
           pcrange = pprof->p_high - pprof->p_low;/* this pc range */
           pprof->p_buff = (HISTCOUNTER *)bufp;   /* range buffer addr */
           if ( i==0 ){ /* put call count buffer in first range buffer */
                /* number of hist counters for time sampling + call cnt space*/
                pprof->p_bufsize = HIST_NUM_COUNTERS( pcrange ) +
                     ( (callcntsize + HIST_COUNTER_SIZE-1)/HIST_COUNTER_SIZE );
           }
           else { /* not first one */
                /* number of hist counters for time sampling */
                pprof->p_bufsize = HIST_NUM_COUNTERS( pcrange );
           } /* endif */
           pprof->p_scale = HIST_SCALE_1_TO_1;    /* range scale */

#ifdef _MONSTN_DEBUG  /* each element in loop */
           printf(
"%s: i: %d  low: %8.8x  high: %8.8x  buff: %8.8x  size: %8.8x  scale: %8.8x\n"
                ,mycall,i,pprof->p_low,pprof->p_high,pprof->p_buff,
                                         pprof->p_bufsize, pprof->p_scale);
#endif

           } /* endfor - fill in prof ranges */

        pprof->p_high = (caddr_t)0;       /* last element flag */

        /* set parms for monitor call */
        /* lowpc is don't care but non-zero */
        /* highpc is don't care */
        /* buffer is set */
        monsize = (int)-1; /* multi-range (non-contig) flag */
        /* fcnumrsiz is func count buffer size - num functions(p)/bytes(pg) */

#ifdef _MONSTN_DEBUG
        printf("%s: Prof at %8.8x\n",mycall,buffer);
        dcount = (numrngs+1)*sizeof(struct prof);
        DUMPHEX((caddr_t)buffer,(uint)dcount);

        printf("%s: Calling monitor with:\n \
     lowpc: %8.8x  highpc: %8.8x  buf: %8.8x  bufsiz: %8.8x  nfunc: %d\n",
           mycall,1,IGNORED_ARG,buffer,monsize,fcnumrsiz);
        printf("%s: exit\n",mycall);
#endif

        /* start profiling */
        rc = monitor( (caddr_t)1, (caddr_t)IGNORED_ARG,
                                                   buffer, monsize, fcnumrsiz);
        free( buffer ); /* return prof arg space */

        if ( rc != _MON_OK ) { /* in case of err release buffer and set flag */
           free( z->monstubuf );
           z->monstubuf = NULL;
           }

        return ( rc );

} /* end function monstn */

/*
 * NAME: monldq
 *
 * FUNCTION: Get loaded program info from system and return in mldq struct.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Private function. User process.
 *
 * (NOTES:) Uses system routine loadquery.
 *
 * (RECOVERY OPERATION:) Returns NULL on any failure.
 *
 * (DATA STRUCTURES:) Allocates data the size of the loadquery data to build
 * the return structure.  Uses malloc for allocation.
 *
 * RETURNS: Address of mldq struct/NULL
 */
static struct mldq *monldq(
    char *extfuncname  /* err msg funct name */
    )                  /* returns pointer to mldq struct - NULL if error */
{
    struct mldq *qp;          /* pointer to return data */
    struct rng *tg;           /* pointer to range array in *qp */
    struct pldr *tr;          /* running pointer in loader buffer in *qp */
    caddr_t ctp;              /* to string ptr for copy */
    int nprg;                 /* number of programs loaded */

    uint qbufsiz;             /* size of info buffer for loadquery */
    struct ld_info *qinfo;    /* addr of info buffer */
    struct ld_info *tp;       /* running ptr in *qinfo */
    caddr_t cfp;              /* from string ptr for copy */

    caddr_t  real_textorg;    /* Loaded address of text */
    ulong    real_textlen;    /* Length of just the text */
    FILHDR  *fhp;             /* loaded address of object file header */
    AOUTHDR *fhxp;            /* loaded address of obj file aux header */
    SCNHDR  *shp;             /* loaded address of text section header */

    int i;                    /* index */
    int rc;                   /* return code */
    uint sizreq;              /* size required when error */

#ifdef _MONLDQ_DEBUG
    char mycall[]="monldq";
    caddr_t tfp; /* temp to file   name ptr */
    caddr_t tmp; /* temp to member name ptr */
    printf("%s: entry\n",mycall);
#endif

    #define ESTNAMESIZE 20+20 /* guess at name sizes */
    #define ESTNUMELEM  2     /* guess at number of prog parts */
    qbufsiz = ESTNUMELEM*sizeof(struct ld_info)+ESTNAMESIZE; /* common case */
    qinfo = (struct ld_info *)malloc(qbufsiz); /* get space */
    if ( qinfo == NULL ) { /* no room in the inn */
       sizreq=qbufsiz;   /* set err msg parm */
       goto ErrorReturn; /* error -print MSG01 and return NULL */
       }
    else { /* got the space - now get the info */
       rc=loadquery((int)L_GETINFO, (void *)qinfo, qbufsiz);
       while ((rc == -1) && (errno == ENOMEM)) {
          free( qinfo );                             /* release old space */
          qbufsiz = 2*qbufsiz;                       /* double space */
          qinfo = (struct ld_info *)malloc(qbufsiz); /* get new space */
          if ( qinfo == NULL ) { /* no room in the inn */
             sizreq=qbufsiz;   /* set err msg parm */
             goto ErrorReturn; /* error -print MSG01 and return NULL */
             }
          rc=loadquery((int)L_GETINFO, (void *)qinfo, qbufsiz);
          } /* end while */
       if ( rc != 0 ) {
          sizreq=qbufsiz;   /* set err msg parm */
          goto ErrorReturn; /* error -print MSG01 and return NULL */
          }

       /* compute number of ranges */
       tp=qinfo;
       nprg=1;
       while ( tp->ldinfo_next != 0 ) {
#ifdef _MONLDQ_DEBUG
          printf("%s: ldinfo_next %8.8x  ldinfo_textorg %8.8x  name >>%s<<\n",
             mycall,tp->ldinfo_next,tp->ldinfo_textorg,tp->ldinfo_filename);
#endif
          tp = (struct ld_info *)((uint)tp + tp->ldinfo_next);
          nprg++;
          }

       /* compute space needed for mldq struct - is bigger than needed */
       /* but does not require running the strings to calc exactly. */
       /* use size of data area returned from loadquery */
       qp = (struct mldq *)malloc(qbufsiz  /* loader buffer will fit in here */
                      + sizeof(struct mldq)       /* base + 2 range elements */
                      + (MAX(0,nprg-2))*sizeof(struct rng) ); /* over 2 rngs */
       if ( qp == NULL ){ /* no room */
          free( qinfo );
          sizreq=qbufsiz;   /* set err msg parm */
          goto ErrorReturn; /* error -print MSG01 and return NULL */
          }

       qp->numldp=nprg; /* number loaded programs */

       /* calc size of loader buffer and fill it and qp* as we go */
       tp = qinfo;                        /* init element */
       tg = &(qp->range[0]);              /* range addr in *qp */
       i=0;                               /* range index */
       tr = (struct pldr *)(&(qp->range[nprg]));/* loader buffer addr in qp* */
       qp->ldrbuf = (uint)tr - (uint)qp; /* offset to loader buffer */
       for ( ;; ){ /* for each loaded program entry */

          /* find text loaded address and length from the headers */
          fhp = (FILHDR *)tp->ldinfo_textorg;        /* loaded file header */
          fhxp = (AOUTHDR *)((caddr_t)fhp + FILHSZ); /* aux header */
          shp = (SCNHDR *)( (caddr_t)fhxp + fhp->f_opthdr); /* section hdrs */
          shp = (SCNHDR *)( (caddr_t)shp +
                      ((fhxp->o_sntext)-1)*SCNHSZ );  /* text section header */

          real_textorg = (caddr_t)(fhp) + shp->s_scnptr;/* loaded text addr */
          real_textlen = (ulong)fhxp->o_tsize;        /* text size in bytes */
	  /**********
	    if there is no text segment, then decrement the number of
	    loaded programs and do no increment i
	  ***********/
	  if (real_textlen) {

              /* fill in the range info */
              tg[i].lpc = (caddr_t)real_textorg;          /* range lo end */
              tg[i].hpc = (caddr_t)(real_textorg +
                                 real_textlen);           /* range hi end +1 */

              /* fill in the loader buffer entry */
              tr->textaddr = real_textorg;                /* loaded text address */

              ctp = (caddr_t)(&(tr->names[0]));           /* to string ptr */
              cfp = (caddr_t)(&(tp->ldinfo_filename[0])); /* from str ptr */

#ifdef _MONLDQ_DEBUG
              printf("%s: from file name >>%s<<\n",mycall,cfp);
              tfp=ctp; /* trap to file name address */
#endif

              while( (*(ctp++) = *(cfp++)) != 0 );   /* copy prog name and null */

#ifdef _MONLDQ_DEBUG
              printf("%s: from member name >>%s<<\n",mycall,cfp);
              tmp=ctp; /* trap to member name address */
#endif

              while( (*(ctp++) = *(cfp++)) != 0 ); /* copy member name and null */

#ifdef _MONLDQ_DEBUG
              printf("%s: to   file name >>%s<<\n",mycall,tfp);
              printf("%s: to member name >>%s<<\n",mycall,tmp);
#endif

          	i++;                                           /* next index */
	  }
	  else
                qp->numldp--; /* decrement number loaded programs */

          /* we round up to caddr_t for each entry */
          tr->next = ROUNDUP( (uint)ctp,sizeof(caddr_t) )-(uint)(&(tr->next));
          if ( tp->ldinfo_next == 0 ) { /* is this last element */
             break; /* leave - this is last element */
             }
          tr = (struct pldr *)((uint)tr + tr->next); /* next elmt addr/end  */

          tp = (struct ld_info *)((uint)tp + tp->ldinfo_next); /* next elmt */
          } /* endfor */

       qp->ldrsiz = (uint)tr + tr->next
                          - (uint)&(qp->range[nprg]); /* loader buffer size */
       qp->size = (uint)tr + tr->next
                                             - (uint)qp ;/* total data size */
       tr->next = 0; /* last element marker */

#ifdef _MONLDQ_DEBUG
       printf("%s: loader buffer built\n",mycall);
#endif

       free( qinfo );
       }

#ifdef _MONLDQ_DEBUG
       printf("%s: Good exit\n",mycall);
#endif

CommonReturn:

#ifdef _MONLDQ_DEBUG
       printf("%s: Returning: %8.8x\n",mycall,qp);
#endif

       return( qp ); /* pointer to mldq structure */

ErrorReturn:
       catd = catopen(MF_LIBC,NL_CAT_LOCALE);
       fprintf(stderr,MSG01,extfuncname,sizreq);
       catclose (catd);
       qp = NULL; /* error return value */

#ifdef _MONLDQ_DEBUG
       printf("%s: Error exit\n",mycall);
#endif
       goto CommonReturn;
} /* end function monldq */

/*
 *
 * NAME: moncontrol
 *
 * FUNCTION: Turns profiling off and on (both call counting and timing).
 *
 * EXECUTION ENVIRONMENT:
 *
 *      System Subroutine. User process.
 *
 * (NOTES:) Uses global variables for control.
 *
 * (RECOVERY OPERATION:) None
 *
 * (DATA STRUCTURES:) Maintains internal static value of mode.
 *
 * RETURNS: Previous value of mode (the input parm)
 */
int moncontrol(
    int mode
    )         /* returns previous value of mode */
{
    extern int errno;
    extern struct monglobal _mondata; /* global control variables */
    static struct monglobal *z=&_mondata; /* pointer to them */

    static struct prof killit = {0, 0, 0, 0, 0}; /* no ranges for profiling */
    static int      oldmode;        /* mode value last entry */

    int             tmode;          /* variable for the switch */
    int             rc;             /* return code */

#ifdef _MONCTL_DEBUG
    char mycall[]="moncontrol";
    printf("%s: entry\n",mycall);
#endif
    errno = 0; /* init for profil call - profil always returns 0 */
    if (mode) {  /* start */
        profil(z->ProfBuf, z->ProfBufSiz, z->ProfLoPC, z->ProfScale);
        z->profiling = 0;
        }
    else { /* stop */
        if ( z->ProfBufSiz == -1 ) { /* multiple range */
                profil( (caddr_t)&killit, (uint)-1,
                                         (uint)IGNORED_ARG, (uint)IGNORED_ARG);
                }
        else {                        /* single range */
                profil((caddr_t)0,0,0,0);
                }
        z->profiling = 3;
        }
    tmode=oldmode;
    oldmode=mode;
    if ( errno != 0 ){ /* if profil returned error */
        perror("profil");
        rc = _MON_ERR; /* return err flag */
        }
    else {           /* profil return ok - return old mode */
        if ( tmode == 0 ){ /* return only 0 or 1 for old mode */
            rc=tmode;      /* old mode = stopped */
            }
        else {
            rc = 1;        /* old mode = started */
            } /* endif */

        } /* endif */
#ifdef _MONCTL_DEBUG
             printf("%s: exit mode %d oldmode %d rc %d\n",mycall,mode,tmode,rc);
#endif
    return( tmode ); /* previous mode value, 0 the first time */

} /* end function moncontrol */


/*
 * NAME: monstartup
 *
 * FUNCTION: Allocates data structures and starts profiling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      System Subroutine.  User process.
 *
 * (NOTES:)
 * This is the default allocation routine for -p (prof) and -pg (gprof)
 * profiling.  It is called with an address range or ranges to be profiled.
 * It allocates and initializes the required data structures using default
 * values and algorithms.  See mon.h for the values and algorithms.  For
 * default profiling of a complete program (ld with -p or -pg) this routine
 * is called by mcrt0.o (-p) or gcrt0.o (-pg) with the parms set to complete
 * program.
 *
 * For the program address range(s) specified in parms, allocate profiling
 * data space(s) and start profiling.  Parm conventions:
 *   - lowpc = 0xffffffff and highpc  = 0x00000000 - Profile complete program
 *   - lowpc = 0xffffffff and highpc != 0x00000000  - More than one program
 *     range defined.  highpc is address of an array of frag structures which
 *     define the ranges.
 *
 * Calls the following routines:
 * monst1 - allocate data space for profiling for single range.
 * monstn - allocate data space for profiling for multiple ranges.
 * monldq - get loaded program(s) info from loader.
 * malloc - allocate data space(s).
 * free   - free data space(s).
 *
 * (RECOVERY OPERATION:) none
 *
 * (DATA STRUCTURES:) Allocates data using malloc for counting and timing.
 *
 * RETURNS: _MON_OK if all went well.  _MON_ERR otherwise, and profiling not
 * started.
 */
int monstartup(
        caddr_t lowpc,  /* lo address of range to profile/non-contig flag */
        caddr_t highpc  /* hi address of range to profile/addr range struct */
        )               /* hi address is upper limit +1 mod min inst size */
                        /* returns _MON_OK, or _MON_ERR if error */
{
        extern struct mldq *monldq( char * );

        extern int monst1( caddr_t, caddr_t );
        extern int monstn( struct frag *, uint );

        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */

        struct mldq *lqinfo; /* monldq data table */
        struct rng *pg;      /* program range in data table */

        struct frag *np;     /* non-contig input */
        int fragsize;        /* size in bytes to allocate */
        int i;               /* index */
        uint nrng;           /* temp num ranges */
        caddr_t tlpc;        /* temp lo pc */
        caddr_t thpc;        /* temp hi pc */
        int monstrc;         /* return code from internal functions */

#ifdef _MONSTU_DEBUG
        char mycall[]="monstartup";
        extern void DUMPHEX(caddr_t,uint);
        printf("%s z->prof_type: %8.8x\n",mycall,z->prof_type);
        printf
          ("%s: Input args: lowpc %8.8x  highpc %8.8x \n",mycall,lowpc,highpc);
#endif
	catd = catopen(MF_LIBC,NL_CAT_LOCALE);

        if ( (z->prof_type != _PROF_TYPE_IS_P) &&
             (z->prof_type != _PROF_TYPE_IS_PG) ){
           fprintf(stderr, MSG04,msgidmonstartup,z->prof_type);
	   catclose(catd);
           return( _MON_ERR );
           }
        /* check if caller wants the whole program profiled */
        if ( (highpc==(caddr_t)0) && (lowpc==(caddr_t)(0xffffffff)) ) {
           /* profile the whole program wherever it's parts are */
           /* if loader info shows non-contig program - gen that */
           if ( (lqinfo=monldq(msgidmonstartup)) == NULL ) { /* ld prog info */
              /* Error Message printed in monldq */
              return(_MON_ERR); /* no storage to free */
              }
           pg=(struct rng *)(&lqinfo->range[0]); /* addr of range table */
           if ( lqinfo->numldp == 1 ){ /* if contig program */
              /* get the actual range limits out of the loader info */
              tlpc = pg->lpc; /* save for call */
              thpc = pg->hpc; /* save for call */
              free( lqinfo ); /* free the loader info storage */
              monstrc = monst1( tlpc, thpc ); /* setup whole contig prog */
              }
           else { /* do non-contig prog - build range table */
              fragsize = (lqinfo->numldp+1)*sizeof(struct frag);
              np = (struct frag *)malloc( fragsize );
              if ( np == NULL ) { /* no space */
                 /* "No space for monitor buffer(s)\n" */
                 fprintf(stderr, MSG01,msgidmonstartup,fragsize);
		 catclose(catd);
                 free( lqinfo ); /* free the loader info storage */
                 return(_MON_ERR);
                 }
              /* this range table is like the input for non-contig ranges */
              for (i=0; i < lqinfo->numldp; i++) { /* for each range */
                 np[i].p_low = pg[i].lpc;
                 np[i].p_high = pg[i].hpc;
                 } /* end for - fill in contig ranges from ld info */
              np[i].p_high = 0; /* end element flag */
              nrng = lqinfo->numldp; /* save num ranges for call */
              free( lqinfo ); /* free the loader info storage */
#ifdef _MONSTU_DEBUG
        printf("%s sort input frag table\n",mycall,z->prof_type);
        for ( i = 0; i < nrng ; i++ ){
              printf("%s: rng %d  lowpc %8.8x  highpc %8.8x \n",
                                        mycall,i,np[i].p_low,np[i].p_high);
              } /* endfor */
#endif
              /* put entries in ascending address order */
              qsort( np, nrng, sizeof( struct frag ), cmp64);

#ifdef _MONSTU_DEBUG
        printf("%s sort output frag table\n",mycall,z->prof_type);
        for ( i = 0; i < nrng ; i++ ){
              printf("%s: rng %d  lowpc %8.8x  highpc %8.8x \n",
                                        mycall,i,np[i].p_low,np[i].p_high);
              } /* endfor */
#endif
              monstrc = monstn( np,nrng ); /* setup whole non-contig prog */
              free( np ); /* free the range table built */
              } /* end else do non-contig prog */
           } /* end  if whole program profile */
        else { /* whole program profile not requested */
           if ( lowpc==(caddr_t)(0xffffffff) ) { /* non-contiguous ranges */
              np = (struct frag *)highpc;    /* address of the frag table */
              for ( i=1; (np+i)->p_high != 0; i++ ){   /* count num of ranges */
                 } /* endfor */
              monstrc = monstn( np,(uint)i ); /* do non-contig prog */
              }
           else { /* contiguous range specified */
              /* make sure range is at least a range */
              if ( highpc <= lowpc ){ /* range not a range */
                 /* "%s: Null address range specified Lo: %8.8x  Hi: %8.8x\n"*/
                 fprintf(stderr, MSG03,msgidmonstartup,lowpc,highpc);
	    	 catclose(catd);
                 return(_MON_ERR);
                 }
              monstrc = monst1( lowpc,highpc ); /* do contig prog */
              }
           }
        return(monstrc);
} /* end function monstartup */

/*
 * NAME: monitor
 *
 * FUNCTION: Initializes data areas for single and multiple range profiling
 * call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      System Subroutine. User process.  Called by monst1 and monstn.
 *
 * (NOTES:)
 * System Subroutine for -p (prof) and -pg (gprof) profiling.
 *   - initializes function call count data structure
 *   - starts profiling
 *   - stops profiling and writes out the mon.out/gmon.out file
 *   - buffers for histogram and call counters are supplied by caller
 *
 * For multi range profiling, the function call count buffer is packaged with
 * the first hist buffer described by the prof structure.  That bufsize parm
 * defines the total of both spaces.  The fcnumrsiz parm defines the portion
 * of the buffer to be used for the function call counts.  For p (prof)
 * profiling it is the number of counters.  For pg (gprof) profiling it is the
 * byte size.  By arbitrary convention the first part is used for the function
 * call counts.
 *
 * Functions called:
 * monldq - get loaded program info
 * pmoninit - func call count data init (-p)
 * gmoninit - func call count data init (-pg)
 * moncontrol - start/stop profiling
 * creat,write,close - create write and close the common parts of the
 *                     output data file
 * pmondwrt - write the func call data counters (-p)
 * gmondwrt - write the func call data counters (-pg)
 *
 * (RECOVERY OPERATION:) Returns _MON_ERR on any failure.
 *
 * (DATA STRUCTURES:) See mon.h
 *
 * RETURNS: _MON_OK if all went well.  _MON_ERR otherwise, and profiling not
 * started.
 */

int monitor(
        caddr_t lowpc,  /* low address of contig range/stop profiling flag */
        ...
        )
{

        /* required variable arg list if lowpc != 0 */
        caddr_t highpc; /* high address of contig range/don't care */
        caddr_t buf;    /* buffer/prof struct address */
        int     bufsiz; /* buf size-num hist ctrs/non-contig ranges flag */
        int     bufbytes; /* buf size in bytes */
        int     fcnumrsiz;/* max num of call counters in buffer (p) or */
                          /* max bytes of storage to use for call ctrs (pg) */


        extern int moncontrol(int);       /* starts/stops profiling */
        extern struct mldq *monldq( char * ); /* gets loader data */
        extern int pmondwrt( int );       /* writes -p func call ctrs out */
        extern int gmondwrt( int );       /* writes -pg func call ctrs out */
        extern int pmoninit(              /* -p funct call count data init */
                    caddr_t, uint, uint, uint, struct outrng *, uint *);
        extern int gmoninit(              /* -pg funct call count data init */
                    caddr_t, uint, uint, uint, struct outrng *, uint *);

        extern struct monglobal _mondata; /* global control variables */
        static struct monglobal *z=&_mondata; /* pointer to them */

        extern char *OutFileP;    /* output file name -p */
        extern char *OutFilePG;   /* output file name -pg */
        struct outhdr *php;       /* header in output file data table */
        caddr_t ctp;              /* loader buffer in output file data table */
        struct outrng *tg;        /* range buffer in output file data table */
        static int numrngs;       /* number of program ranges for profiling */
        uint numhist;             /* number of hist counters */
        uint rnghist;             /* hist size bytes this range */
        caddr_t fbuf;             /* function call count data buffer */
        uint  cntsize;            /* func call cnt buffer size in bytes */
        uint maxbuf;              /* first range buffer size - bytes */

        register int i;           /* temp/index */
        int rc;                   /* return code */

        struct mldq *lqinfo;      /* loader info from monldq */
        caddr_t cfp;              /* loader buffer in lqinfo */

        struct prof *pprof;       /* input arg structure if non-contiguous */
        static struct prof        /* profil arg structure if non-contiguous */
                    *spprof=NULL;
        int profsize;             /* size of prof struct to allocate */

        va_list argp;             /* variable arg pointer */

        char *OutFile;            /* output file name */
        uint hist_cntr_avail;     /* temp - num hist counters available */
        uint hist_cntr_reqd;      /* temp - num hist counters required */


#ifdef _MONITOR_DEBUG
        static char mycall[]="monitor";  /* function name for debug */
        uint dcount;
        struct prof *dpprof;
        extern void DUMPHEX(caddr_t,uint);

        printf( "%s: entry  lowpc: %8.8x\n" ,mycall,lowpc);
#endif

	catd = catopen(MF_LIBC,NL_CAT_LOCALE);

        if ( (z->prof_type != _PROF_TYPE_IS_P) &&
             (z->prof_type != _PROF_TYPE_IS_PG) ){
           fprintf(stderr, MSG04,msgidmonitorstart,z->prof_type);
	   catclose(catd);
           return( _MON_ERR );
           }
        if ( lowpc == 0) { /* if done profiling write out table & cleanup */
                register int fd;
                register int rc;

                moncontrol(0);   /* stop timer profiling */

                /* free profil call prof struct if non-contig */
                if ( spprof != NULL ) free( spprof );

                if (z->ssiz == 0){
                        fprintf(stderr,MSG05,msgidmonitorstop);
			catclose(catd);
                        /* free data buffer if allocated by monstartup */
                        if ( z->monstubuf != NULL ) {
                                free( z->monstubuf );
                                z->monstubuf = NULL; /* reset */
                                }
                        return(_MON_ERR); /* no table */
                        }
                /* set the correct output data file name */
                if ( z->prof_type == _PROF_TYPE_IS_P ){
                        OutFile = OutFileP;
                        }
                else { /* z->prof_type == _PROF_TYPE_IS_PG */
                        OutFile = OutFilePG;
                        }
                /* create the output data file */
                fd = creat(OutFile, 0666);
                if (fd < 0) goto DoneErrorExit; /* err msg and cleanup */

                /* write output data file hdr,loader buffer and range buffer */
                rc = write(fd, z->sbuf, z->ssiz);
                free( z->sbuf ); /* release that storage */
                if ( rc  != z->ssiz ) goto DoneErrorExit; /* err msg/cleanup */
                z->ssiz=0;       /* mark no data area allocated */

                /* write output data file histogram buffer(s) */
                rc = write(fd, z->hbuf,z->hsiz);
                if ( rc != z->hsiz )
                                  goto DoneErrorExit; /* err msg and cleanup */
                /* write output data file function(p)/arc(pg) counters */
                if ( z->prof_type == _PROF_TYPE_IS_P ){
                        rc = pmondwrt(fd); /* function call counters */
                        }
                else { /* z->prof_type == _PROF_TYPE_IS_PG */
                        rc = gmondwrt(fd); /* function arc counters */
                        }
                if ( rc != 0 ) goto DoneErrorExit; /* err msg and cleanup */

                /* close output data file */
                if ( close(fd) != 0 )
                                  goto DoneErrorExit; /* err msg and cleanup */

                /* free data buffer if allocated by monstartup */
                if ( z->monstubuf != NULL ) {
                        free( z->monstubuf );
                        z->monstubuf = NULL; /* reset */
                        }

#ifdef _MONITOR_DEBUG

        printf( "%s: done ok exit\n",mycall);
#endif
                return(_MON_OK);

DoneErrorExit:
                fprintf(stderr,MSG02,msgidmonitorstop,OutFile);
		catclose(catd);
                perror(msgidmonitorstop); /* say what errno means */
                /* free data buffer if allocated by monstartup */
                if ( z->monstubuf != NULL ) {
                        free( z->monstubuf );
                        z->monstubuf = NULL; /* reset */
                        }
                z->ssiz = 0; /* mark no data area */
                return(_MON_ERR);

                } /* end if done profiling */

        /* start profiling */
        va_start(argp,lowpc);               /* init var list */
        highpc    = va_arg(argp,caddr_t);   /* get args */
        buf       = va_arg(argp,caddr_t);
        bufsiz    = va_arg(argp,int);
        fcnumrsiz = va_arg(argp,int);
        va_end(argp);                       /* cleanup var list */

#ifdef _MONITOR_DEBUG
        printf("%s: entry\n",mycall);
        printf( \
          "%s: Input: lowpc: %8.8x  highpc %8.8x  buf: %8.8x  bufsiz: %8.8x\n"
                                              ,mycall,lowpc,highpc,buf,bufsiz);
        printf( "%s: Input: fcnumrsiz: %d %8.8x\n",mycall,fcnumrsiz,fcnumrsiz);
#endif


        /* get loaded program info */
        if ( ( lqinfo = monldq(msgidmonitorstart) ) == NULL ) {
                fprintf(stderr,MSG01,msgidmonitorstart,0); /* unk size - ? dup msg*/
                z->ssiz=0;
                goto StartErrorExit;
                } /* endif */

        /* determine number of program ranges to be profiled */
        if ( bufsiz == -1 ) { /* non-contig ranges specified */
                pprof=(struct prof *)(buf); /* non-contig input structure */
                /* compute actual number of program ranges */
                for (numrngs=0; pprof[numrngs].p_high != 0 ;numrngs++ ) {
                        } /* end for all ranges */
                } /* if multi range input */

        else { /* single program range specified */
                numrngs = 1;
                }

        /* create front of output data file data table */
        z->ssiz = sizeof(struct outhdr)         /* output header */
                +lqinfo->ldrsiz                 /* loader buffer */
                +sizeof(struct outrng)*numrngs; /* range buffer */

        /* allocate the table */
        z->sbuf = (caddr_t)malloc(z->ssiz);
        if ( z->sbuf == NULL ){
                fprintf(stderr,MSG01,msgidmonitorstart,z->ssiz);
                z->ssiz=0;
                free( lqinfo ); /* free loader info space */
                goto StartErrorExit;
                } /* endif */

        /* fill in the table */
        /* header */
        php=(struct outhdr *)z->sbuf;       /* base address */
        php->lbs = lqinfo->ldrsiz;          /* loader buffer size */
        php->nrngs = numrngs;               /* number of range elements */

        /* copy loader buffer - size already rounded to caddr_t */
        cfp=(caddr_t)((uint)lqinfo + lqinfo->ldrbuf); /* from address */
        ctp=(caddr_t)&php[1];                         /* to address */
        /* copy the loader buffer */
        memcpy((void *)ctp,(void *)cfp,(size_t)lqinfo->ldrsiz);
        free( lqinfo ); /* free loader info space */

        /* init range buffer - hist buffer info - func call cnt info */
        tg = (struct outrng *)((uint)ctp+php[0].lbs); /* range buffer */

        if ( bufsiz == -1 ) { /* non-contig ranges specified */
                z->hsiz = 0; /* total bytes of hist  counters */

                /* profil prof arg structure */
                /* Must persist throughout profiling for moncontrol calls */
                profsize = sizeof(struct prof)*(numrngs+1);
                spprof = (struct prof *)malloc( profsize );
                if ( spprof == NULL ){
                        fprintf(stderr,MSG01,msgidmonitorstart,profsize);
                        free( z->sbuf ); /* free the table space */
                        z->ssiz=0;       /* set control flag */
                        goto StartErrorExit;
                        } /* endif */
                /*
                 *  Fill in the range table addresses in the output table.
                 */
                for ( i = 0; i < numrngs; i++ ){ /* each inp rng */
                        /* make sure range is at least a range */
                        if ( pprof[i].p_high <= pprof[i].p_low ){/* bad range*/
                                /* "%s: Null address range specified
                                                     Lo: %8.8x  Hi: %8.8x\n" */
                                fprintf(stderr, MSG03,msgidmonitorstart,
                                               pprof[i].p_low,pprof[i].p_high);
                                goto StartErrorExit;
                                }
                        tg[i].lpc = pprof[i].p_low;  /* range low limit */
                        tg[i].hpc = pprof[i].p_high; /* range high limit */

                        } /* end for each range */

                /* func call count buffer is in with first hist buffer */
                fbuf = (caddr_t)pprof[0].p_buff; /* first part for call cnts */

                maxbuf = HIST_COUNTER_SIZE * pprof[0].p_bufsize; /* bytes */

                /* init the call count data and get size used */
                /* cntsize is an output */
                if ( z->prof_type == _PROF_TYPE_IS_P){
                      /* cntsize is an output */
                      rc = pmoninit(fbuf,maxbuf,
                                    (uint)fcnumrsiz, (uint)numrngs,tg,
                                                                    &cntsize);
                        }
                else { /* z->prof_type == _PROF_TYPE_IS_PG */
                      /* cntsize is an output */
                      rc = gmoninit(fbuf,maxbuf,
                                    (uint)fcnumrsiz, (uint)numrngs,tg,
                                                                    &cntsize);
                        }


#ifdef _MONITOR_DEBUG
  printf("%s: moninit return %d\n", mycall,rc);
#endif

                if ( rc != (_MON_OK)) {
                        goto StartErrorExit; /* err msg already printed out */
                        }

#ifdef _MONITOR_DEBUG
  printf("%s: function call count size/num pprof/moninit %8.8x %8.8x \n",
  mycall,pprof[0].p_bufsize,cntsize);
#endif

                z->hbuf = fbuf+cntsize; /* save hist buffer address */

                /*
                 *  Fill in the range table in the output table.  Copy the
                 *  pprof struct to spprof struct for prof call, changing
                 *  num hist cntrs to bytes.
                 */
                for ( i = 0; i < numrngs; i++ ){ /* each inp rng */

                        if (i==0){ /* if first range - take out count space */
                                /* range number of hist counters */
                                tg[i].nhcnt = pprof[i].p_bufsize
                                             - (cntsize/HIST_COUNTER_SIZE);

                                /* accum number of hist bytes */
                                z->hsiz += ( (HIST_COUNTER_SIZE *
                                                pprof[i].p_bufsize) - cntsize);

                                /* copy arg prof struct for profil call */
                                spprof[i].p_buff = pprof[i].p_buff +
                                                   (cntsize/HIST_COUNTER_SIZE);
                                spprof[i].p_bufsize = (HIST_COUNTER_SIZE *
                                                 pprof[i].p_bufsize) - cntsize;

                                } /* end if first range */

                        else { /* not first range */

                                /* range number of hist counters */
                                tg[i].nhcnt = pprof[i].p_bufsize;

                                /* accum number of hist bytes */
                                z->hsiz += HIST_COUNTER_SIZE *
                                                           pprof[i].p_bufsize;

                                /* copy arg prof struct for profil call */
                                spprof[i].p_buff = pprof[i].p_buff;
                                spprof[i].p_bufsize = HIST_COUNTER_SIZE *
                                                            pprof[i].p_bufsize;
                                } /* end else not first range */

                        /* copy rest of arg prof struct for profil call */
                        spprof[i].p_low = pprof[i].p_low;
                        spprof[i].p_high = pprof[i].p_high;

                        /*
                         * Verify that the hist buffer is indeed in
                         * one piece as required by profil.  Constraint
                         * defined here is that addresses and sizes must agree
                         * within eight bytes (to allow for possible
                         * rounding of each buffer piece to pointer size).
                         */
                        if ( i > 0 ) { /* test only after the first one */
                                caddr_t p,lp;           /* temps for ease */
                                uint size;              /* of expression */
                                p = (caddr_t)spprof[i].p_buff;   /* p[i] */
                                lp = (caddr_t)spprof[i-1].p_buff;/* p[i-1] */
                                size = spprof[i-1].p_bufsize; /* size[i-1] */

                                if ( !( (p >= (lp + size    )) &&
                                        (p <= (lp + size + 8))
                                                               )  ) {
                                        fprintf(stderr,MSG06,msgidmonitorstart,
                                                          i-1,lp,i-1,size,i,p);
                                        goto StartErrorExit;
                                        } /* if not one piece buffer */
                                } /* if not range */

                        /* compute scale for profil */
                        /* scale 1 instruction per counter if room */
                        hist_cntr_avail = pprof[i].p_bufsize; /* num cntrs */
                        hist_cntr_reqd = HIST_NUM_COUNTERS(spprof[i].p_high -
                                                             spprof[i].p_low);
                        if( hist_cntr_avail < hist_cntr_reqd ) { /* can't */
                                spprof[i].p_scale =
                                  ((float) hist_cntr_avail / hist_cntr_reqd)
                                                          * HIST_SCALE_1_TO_1;
                                }
                        else { /* can */
                                spprof[i].p_scale = HIST_SCALE_1_TO_1;
                                }

                        } /* end for each range */

                spprof[numrngs].p_high = (caddr_t)0; /* last element flag */

                php->nhcnt = z->hsiz
                                   /HIST_COUNTER_SIZE ; /* num cntrs total */


                /* set up profil call args */
                z->ProfBuf = (caddr_t)spprof;     /* prof address for profil*/
                z->ProfBufSiz = -1;     /* non-contig flag value for profil */
                z->ProfLoPC = IGNORED_ARG;       /* ignored - dontcare */
                z->ProfScale = IGNORED_ARG;       /* ignored but not 0 */

                } /* end non-contig setup for range,hist,funct call cnt */

        else { /* single program range specified */
                bufbytes=bufsiz*2; /* buffer size in bytes */
                /* make sure range is at least a range */
                if ( highpc <= lowpc ){/* bad range*/
                        /* "%s: Null address range specified
                                                     Lo: %8.8x  Hi: %8.8x\n" */
                        fprintf(stderr, MSG03,msgidmonitorstart,lowpc,highpc);
                        goto StartErrorExit;
                        }
                tg[0].lpc = lowpc; /* range low limit */
                tg[0].hpc = highpc; /* range high limit */

                fbuf = buf;
                /* init the call count data and get size used */
                if ( z->prof_type == _PROF_TYPE_IS_P){
                        rc = pmoninit(buf,(uint)bufbytes,(uint)fcnumrsiz,
                                                          (uint)1,tg,&cntsize);
                        }
                else { /* z->prof_type == _PROF_TYPE_IS_PG */
                        rc = gmoninit(buf,(uint)bufbytes,(uint)fcnumrsiz,
                                                          (uint)1,tg,&cntsize);
                        }
                if ( rc != _MON_OK ) {
                        fprintf(stderr,MSG01,msgidmonitorstart,
                                                           cntsize-(bufbytes));
                        /* no room in the buff for all parts */
                        goto StartErrorExit;
                        }


#ifdef _MONITOR_DEBUG
  printf("%s: function call count size moninit/bufsize %8.8x %8.8x \n",
  mycall,cntsize,bufsiz);
#endif

                if (cntsize >= (bufbytes) ) { /* no room for hist buffer */
                        fprintf(stderr,MSG01,msgidmonitorstart,z->hsiz);
                        goto StartErrorExit; /* no buff room for all parts */
                        }

                z->hsiz = bufbytes - cntsize; /* space not used for call ctrs */
                z->hbuf = (caddr_t)((uint)buf + cntsize); /* hist buffer */

                /* compute number of histogram counters and update headers */
                numhist = z->hsiz/HIST_COUNTER_SIZE;/* number hist ctrs */
                tg[0].nhcnt = numhist;/* range table number of hist counters */

                php->nhcnt = numhist; /* num cntrs total */

                /* set up profil call args */
                z->ProfBuf = z->hbuf;             /* save address for profil */
                z->ProfBufSiz = z->hsiz;          /* buffer size for profil */
                z->ProfLoPC = (uint)lowpc;        /* lowpc for profil */
                /* scale for profil */
                hist_cntr_avail = numhist;
                hist_cntr_reqd = HIST_NUM_COUNTERS(highpc - lowpc);
                if( hist_cntr_avail < hist_cntr_reqd ) {
                        z->ProfScale =
                                  ((float) hist_cntr_avail / hist_cntr_reqd)
                                                          * HIST_SCALE_1_TO_1;
                        }
                else {
                        z->ProfScale = HIST_SCALE_1_TO_1;
                        }

                } /* end  contig setup for range,hist,funct call cnts */

#ifdef _MONITOR_DEBUG
   printf("%s: args to profil:\n",mycall);
   printf("  Buf %8.8x Siz %8.8x LPC %8.8xs Sca %8.8x\n",
      z->ProfBuf,z->ProfBufSiz,z->ProfLoPC,z->ProfScale);
   if (z->ProfBufSiz == -1){ /* non-contig */
         printf("  Buffer   Bufsize  low pc   high pc  scale\n");
      for( dpprof=(struct prof *)z->ProfBuf; dpprof->p_high; dpprof++ ) {
         printf("  %8.8x %8.8x %8.8xs %8.8x %8.8x\n",
            dpprof->p_buff,dpprof->p_bufsize,dpprof->p_low,dpprof->p_high,
            dpprof->p_scale);
         }
      }

   printf("%s: static part of output table at: %8.8x size: %8.8x\n",
                                                       mycall,z->sbuf,z->ssiz);
   DUMPHEX((caddr_t)z->sbuf,(uint)z->ssiz);
   printf("%s: hist part of output table at: %8.8x size %8.8x\n",
                                                      mycall,z->hbuf,z->hsiz);
   printf("%s: func part of output table at: %8.8x size %8.8x\n",
                                                          mycall,fbuf,cntsize);
#endif

        rc = moncontrol(1);                     /* start profiling */


#ifdef _MONITOR_DEBUG
        printf("%s: exit\n",mycall);
#endif

        return( rc );
/*
 * Set values so output routines will not run amuck
 */
StartErrorExit:
z->hsiz = 0; /* protect output writer */

#ifdef _MONITOR_DEBUG
        printf("%s: exit through ErrorExit:\n",mycall);
#endif

catclose(catd);
return(_MON_ERR);

} /* end function monitor */
