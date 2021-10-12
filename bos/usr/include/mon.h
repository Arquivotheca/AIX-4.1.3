/* @(#)55       1.11  src/bos/usr/include/mon.h, cmdstat, bos411, 9428A410j 7/28/92 16:38:25 */
#ifndef _H_MON
#define _H_MON
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: mon.h
 *
 * ORIGINS: 10 26 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
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
 *      @(#)gmon.h      5.1 (Berkeley) 5/30/85
 */

/*
 * Definitions for profiling functions -p (prof) and -pg (gprof)
 */

/*
 *  global variable _mondata.prof_type  values
 */
#define _PROF_TYPE_IS_P    -1 /* -p  (prof) */
#define _PROF_TYPE_IS_PG    1 /* -pg (gprof) */
#define _PROF_TYPE_IS_NONE  0 /* no profiling active */

/*
 *  output data file name
 */
#define MON_OUT  "mon.out"    /* -p profiling */
#define GMON_OUT "gmon.out"   /* -pg profiling */

/*
 *  general rounding functions.
 */
#define ROUNDDOWN(x,y)  (((x)/(y))*(y))
#define ROUNDUP(x,y)    ((((x)+(y)-1)/(y))*(y))

/*
 *  from to chain end marker.
 */
#define _MONFromChainEnd 0

/*
 *  Amount of storage to allocate for histogram counters.
 *
 *  The definition of HIST_STG_SIZE is based on:
 *     - TEXT_RANGE - The text storage address range to be analyzed.
 *     - HIST_COUNTER_SIZE - The size of the timer counters.
 *     - INST_MIN_LENGTH -  Minimum processor instruction length.
 *     - INST_PER_CNT - Number of min length instructions to map into
 *                      a single timer counter.
 *
 *   INST_CNT_SIZE = ( INST_MIN_LENGTH * INST_PER_CNT )
 *
 *   HIST_NUM_COUNTERS = ( TEXT_RANGE + INST_CNT_SIZE -1) / INST_CNT_SIZE
 *
 *   HIST_STG_SIZE = HIST_NUM_COUNTERS * HIST_COUNTER_SIZE
 *
 *  Note that this function is valid for all values of HIST_COUNTER_SIZE,
 *  INST_MIN_LENGTH, and INST_PER_CNT.
 *  Alignment requirements may dictate that some values not be used.
 *  HIST_COUNTER_SIZE must agree with that used by system function profil.
 *  Here are the results for some common values:
 *
 *            Min   Inst  Num     Hist    Total  Storage
 *   Storage  Inst  per   Hist   Counter Storage  Range
 *    Range   Len   Cnt  Counters Size    Size   Fraction
 *
 *    1024      1     1   1024      1     1024     1/1
 *    1024      1     1   1024      2     2048     2/1
 *    1024      2     1    512      3     1536     3/2
 *    1024      2     1    512      2     1024     1/1
 *    1024      2     2    256      2      512     1/2
 *    1024      3     1    342      2      684     2/3
 *    1024      3     2    171      2      342     2/6
 *    1024      3     3    114      2      228     2/9
 *    1024      4     1    256      2      512     2/4
 *    1024      4     1    256      4     1024     4/4
 *    1024      4     2    128      2      256     2/8
 *    1024      4     3     86      2      172     2/12
 *    1024      4     4     64      2      128     2/16
 *
 */

/*
 *  histogram counters are unsigned shorts (according to the kernel).
 */

typedef ushort HISTCOUNTER;         /* histogram counter type */
#define HIST_CNT_MAX ((1<<(CHAR_BIT*sizeof(HISTCOUNTER)))-1) /* max value */

#define HIST_COUNTER_SIZE sizeof( HISTCOUNTER )

#define INST_PER_CNT 1      /* num min length inst to count per counter */


/* min addr span between back to back func calls */
/* address units per minimum length instruction */
#ifdef _POWER
#define MIN_SUBR_SPAN 4
#define INST_MIN_LENGTH 4
#else
#ifdef _IBMRT
#define MIN_SUBR_SPAN 2
#define INST_MIN_LENGTH 2
#else
#ifdef _IBMPS
#define MIN_SUBR_SPAN 2
#define INST_MIN_LENGTH 1
#else
#define MIN_SUBR_SPAN 2
#define INST_MIN_LENGTH 1
#endif
#endif
#endif

#define INST_CNT_SIZE ( INST_MIN_LENGTH * INST_PER_CNT ) /* adr span/ctr */
#define HIST_NUM_COUNTERS(range) ( (range + INST_CNT_SIZE -1) / INST_CNT_SIZE)

/* Make sure histogram storage rounded to multiple of counter size */
#define HIST_STG_SIZE(range) ( ROUNDUP( (HIST_NUM_COUNTERS(range) \
                                   * HIST_COUNTER_SIZE ),HIST_COUNTER_SIZE ) )

/* scale value to get 1 instruction per counter */
#define HIST_SCALE_1_TO_1 ((0x10000*(HIST_COUNTER_SIZE))/((INST_MIN_LENGTH) \
                                                             *(INST_PER_CNT)))

/*
 *  Amount of storage to allocate for 'from' hash buckets for -pg profiling.
 *
 *  The definition of FROM_STG_SIZE is based on:
 *     - TEXT_RANGE - The text storage address range to be analyzed
 *     - FromHashBucket_SIZE - The size of the hash buckets
 *     - MIN_SUBR_SPAN - Minimum address span between subroutine calls
 *
 *   NUM_FromHashBucketS = ( TEXT_RANGE + MIN_SUBR_SPAN-1) / MIN_SUBR_SPAN
 *
 *   FROM_STG_SIZE = NUM_FromHashBucketS * FromHashBucket_SIZE
 *
 *  Note that this function is valid for all values of FromHashBucket_SIZE
 *  and MIN_SUBR_SEPARATION.  It yields the amount of storage required
 *  as if all the storage range were filled with minimum sized calls.
 *  This is required to descriminate between all possible calls.
 *  Alignment requirements may dictate that some values not be used.
 *  Here are the results for some common values:
 *
 *            Min    Num     Hash    Total
 *   Storage  Subr   Hash   Bucket  Storage
 *    Range   Span  Buckets  Size    Size
 *
 *    1024      1    1024      1     1024
 *    1024      1    1024      2     2048
 *    1024      2     512      2     1024
 *    1024      3     342      2      684
 *    1024      4     256      2      512
 *    1024      1    1024      3     3072
 *    1024      2     512      3     1536
 *    1024      3     342      3     1026
 *    1024      4     256      3      768
 *    1024      1    1024      4     4096
 *    1024      2     512      4     2048
 *    1024      3     342      4     1368
 *    1024      4     256      4     1024
 *
 */

typedef ushort HASH_LINK; /* 'from' hash buckets and 'to' links */

#define HASH_LINK_SIZE sizeof( HASH_LINK )

#define NUM_HASH_LINKS(range) ( ( (range)+((MIN_SUBR_SPAN)-(1)) ) \
                                                           / (MIN_SUBR_SPAN) )
#define FROM_STG_SIZE(range) (NUM_HASH_LINKS(range) * (HASH_LINK_SIZE))

#define ARCDENSITY 2    /* constant defined by BSD 4.3 */
#define WHATSIT    100  /* constant defined by BSD 4.3 */
#define MINARCS   50 /* mininum number of function call counters to allocate */
#define MAXARCS  600 /* max number for func call counters -p profiling */

#define TO_NUM_ELEMENTS(range) ( ((range)*(ARCDENSITY))/(WHATSIT) )

/*
 *  Hash function to be applied to 'from' subroutine address.
 *
 *  The definition of FROM_OFFSET IS  based on:
 *     - frompc - The address under consideration
 *     - lopc - The beginning address of the range to be analyzed
 *     - HASH_LINK_SIZE - The size of the hash buckets
 *     - MIN_SUBR_SPAN - Minimum address span between subroutine calls
 *
 *    FROM_OFFSET = (frompc-lopc)* HASH_LINK_SIZE / MIN_SUBR_SPAN
 *
 *  Note that this function is valid for all values of HASH_LINK_SIZE
 *  and MIN_SUBR_SEPARATION.  It is exact and yields the storage address
 *  offset into the 'from' hash bucket array.  To be valid frompc must
 *  be within the address range used to calculate the size of the hash
 *  bucket storage.   lowpc <= frompc < hipc
 *  Here are the results for some common values:
 *
 *                     Min   Num   Hash    Valid
 *               Stg  Subr  Hash  Bucket   Offset                  Hash
 *  lopc  hipc  Range Span Buckets Size    Range         frompc   Offset
 *
 *  1024  2048   1024   1   1024     1   0-1023 by 1      1024       0
 *                                                        1025       1
 *                                                        2047    1023
 *                                                        2048 invalid
 *
 *  1024  2048   1024   2    512     2   0-1022 by 2      1024       0
 *                                                        1025 Cant happen
 *                                                        1026       1
 *                                                        2046    1022
 *
 *  1024  2048   1024   3    342     2    0-682 by 2      1024       0
 *                                                        1025 Cant happen
 *                                                        1026 Cant happen
 *                                                        1027       1
 *                                                        1030       2
 *                                                        2047     682
 *
 *  1024  2048   1024   4    256     3    0-765 by 3      1024       0
 *                                                        1025 Cant Happen
 *                                                        1026 Cant Happen
 *                                                        1027 Cant Happen
 *                                                        1028       3
 *                                                        2044     765
 *
 *  1024  2048   1024   4    256     2    0-510 by 2      1024       0
 *                                                        1025 Cant Happen
 *                                                        1026 Cant Happen
 *                                                        1027 Cant Happen
 *                                                        1028       3
 *                                                        2044     510
 */
/* storage offset - address units */
#define FROM_OFFSET(frompc,lopc) ( ( ((frompc)-(lopc)) * (HASH_LINK_SIZE) ) \
                                                           / (MIN_SUBR_SPAN) )

/* array index - array element units */
#define FROM_INDEX(frompc,lopc) (((frompc)-(lopc)) / (MIN_SUBR_SPAN))

/*
 * data structures
 */

/* keeps track of pinned user profiling buffer - since profiling is preserved
   across forks, we have to keep track of what has been pinned */
struct pinprof {
        long count;             /* num processes using this buffer address */
        short *pin_buf;         /* address of user buffer */
        int pin_siz;            /* size of user buffer */
};

/* structure for non-contiguous program args for monitor and profil calls */
struct prof {
        caddr_t p_low;          /* low sampling address */
        caddr_t p_high;         /* high sampling address */
        HISTCOUNTER *p_buff;    /* address of sampling buffer */
        int     p_bufsize;/* buffer size - monitor/HISTCOUNTERs,profil/bytes */
        uint    p_scale;        /* scale factor */
};

/* structure for non-contiguous program args for monstartup calls */
struct frag {
        caddr_t p_low;          /* low sampling address */
        caddr_t p_high;         /* high sampling address */
};

/*  output data file header */
struct outhdr {
        uint    lbs;            /* loader buffer size */
        int   nrngs;            /* number of program ranges */
        int   nhcnt;            /* Number of entries in hist buffer */
};

/* output data file program loader buffer */
struct pldr {
        uint next;              /* offset to next element */
        caddr_t textaddr;       /* loaded text address */
        char names[2];          /* null terminated prog name path & */
                                /* null terminated member name */
/* actual name strings are in here at variable length. use terminating NULLs */
/* determine actual length from strings. rounded up space indicated by next. */
};

/* output data file range buffer */
struct outrng {
        caddr_t lpc;            /* low address of profile range */
        caddr_t hpc;            /* high address +1 of profile range */
        int   nhcnt;            /* Number of hist counters this range */
};

/* output data file histogram buffer array element */
struct outhist {
        HISTCOUNTER hc;         /* timer counter for iar(pc) histogram */
};

/* output data file function call count buffer array element -p profiling */
struct poutcnt {
        caddr_t  fnpc;          /* function address */
        ulong    mcnt;          /* function call count */
};

/* output data file function call count buffer array element -pg profiling */
struct goutcnt {                    /* tag was rawarc in BSD43 */
    unsigned long       raw_frompc; /* from function address */
    unsigned long       raw_selfpc; /* to function address */
    long                raw_count;  /* to - from arc count */
};

/* -pg profiling func count from/to datacontrol array element */
struct gfctl {
    HASH_LINK        *froms;     /* address of hash array */
    caddr_t           fromlowpc; /* beginning pc addr this hash array */
    uint              fromlimit; /* max num elements this hash array */
    struct tostruct  *tos;       /* 'to' funct count element space */
    HASH_LINK         tolimit;   /* max number elements in *tos */
    };

/* -pg profiling func count to data count element */
struct tostruct {
    caddr_t             selfpc;  /* to subroutine address */
    long                count;   /* arc count */
    HASH_LINK           link;    /* next element in chain index */
    };
/* Max value of link field  as in 255, 65535 ... */
#define TO_MAX ( ( 1 << ( (CHAR_BIT) * (sizeof( HASH_LINK )) ) )-1 )

/* Profiling Global data control structure */
struct monglobal {
    /* Global 'which type profiling are we doing' flag - tri-state */
    int              prof_type;      /* -p,-pg,none */

    /* variable for moncontrol and mcount control of profiling */
    int              profiling;

    /* monitor allocated tables addresses and sizes */
    caddr_t          sbuf;           /* fixed output table base address */
    int              ssiz;           /* fixed output table byte size */
    caddr_t          hbuf;           /* histogram table base address */
    int              hsiz;           /* histogram table byte size */

    /* last profil call args - implicit arguments to moncontrol*/
    caddr_t          ProfBuf;        /* buffer address */
    int              ProfBufSiz;     /* buffer size/multi range flag */
    uint             ProfLoPC;       /* pc offset for hist buffer - lo limit */
    uint             ProfScale;      /* pc scale/compute scale flag */

    /* function call counter data area control variables */
    union countbase {
        struct poutcnt *pcountbase;  /* base address func call count space */
        struct gfctl  *pgcountbase;  /* base address func call count space */
        } cb;
    union countlimit {
        struct poutcnt *pcountlimit; /* max counter address + 1 */
        struct gfctl  *pgcountlimit; /* &(_pcountbase[numrngs]) upper limit */
        } cl;
    struct poutcnt *pnextcounter;/* next available counter address */
    /* address of data area allocated by monstartup */
    caddr_t monstubuf; /* used by monitor to free if not NULL */
    } ;

/*
 * external function prototypes
 */

#ifndef _NO_PROTO
int moncontrol( int mode ); /* returns previous value of mode */
/*
 * if mode = 0 then stop profiling
 * else             start profiling - use as profil call args the values of
 *                  the global variables:
 *                  _mondata.ProfBuf
 *                  _mondata.ProfBufSiz
 *                  _mondata.ProfLoPC
 *                  _mondata.ProfBufScale
 */
#else
int moncontrol();
#endif

#ifndef _NO_PROTO
int monstartup(
        caddr_t lowpc,  /* low address of single range to profile */
        caddr_t highpc  /* high address of single range to profile */
        ) ;             /* hi address is upper limit +1 mod min inst size */
                        /* returns 0 if profiling started, -1 if not - err */
/* Note:
 * if     lowpc == 0xffffffff and highpc == 0 then profile whole program
 * elseif lowpc == 0xffffffff and highpc != 0 then highpc is frag struct addr
 * else   lowpc is address to begin profiling, highpc is 1 beyond end
 */
#else
int monstartup();
#endif

#ifndef _NO_PROTO
int monitor(
        caddr_t lowpc,  /* low address of single range to profile */
        ...
        );              /* returns 0 if profiling started, -1 if not - err */
/* Note:
 * if     lowpc == 0 then stop profiling and write out data output file
 * elseif bufsiz == -1 then buf is prof struct address for multiple ranges
 * else   as defined here
 *
 * when lowpc is not zero the remaining argument definitions are:
 *      caddr_t highpc,    high address +1(mod min inst len) of single range
 *      caddr_t buf,       buffer address
 *      int     bufsiz,    buffer size - bytes
 *      int     fcnumrsiz  max num of call counters in buffer (p) or
 *                           max bytes of storage to use for call ctrs (pg)
 */
#else
int monitor();
#endif

#endif /* _H_MON */
