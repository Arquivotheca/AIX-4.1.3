static char sccsid[] = "@(#)88	1.8  src/bos/usr/ccs/lib/libc/malloc_y.c, libcmem, bos411, 9428A410j 2/2/94 14:20:59";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: free, mallinfo, malloc, mallopt, realloc
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Redefine the entry points for this code.  This was designed to be
 * the sole malloc in the system, but it currently is not, so we 
 * rename the entry points to avoid conflict with the existing malloc.
 * Note that the mallopt and mallinfo entry points are ifdef'd out.
 */
#define malloc   malloc_y
#define mallinfo mallinfo_y
#define free     free_y
#define realloc  realloc_y


   #pragma pagesize (85)
     /*         _________________________________________
               |                                         |
               |       DEBUGGING PREAMBLE FOR THE        |
               |      AIX DYNAMIC STORAGE ALLOCATOR      |
               |_________________________________________|


          The first few pages of this source file comprise a
          preamble which is compiled only when the DEBUG flag
          is defined. When the preamble is compiled, the fol-
          lowing things happen:

            Overriding functions named malloc, free and realloc
            are defined in the preamble, and form the externally
            visible entry points for the allocator.  These are
            wrappings for the 'primitive' malloc, free and re-
            alloc (in the main part of the program). The wrap-
            pings call their primitive cousins, and also pro-
            vide debugging services (see below).

            In order to avoid a clash of names, the preamble
            contains #define statements which rename the pri-
            mitive routines '_malloc', '_free' and '_realloc'
            (suggestion due to Marc Auslander).

            The structures 'adlen' and 'node' are needed in
            the preamble as well as in the main program, and
            are therefore defined in both places.  To avoid
            a clash of names, the preamble contains #define
            statements that rename the structures '_adlen'
            and '_node' in the remainder of the program.

          The idea is that it should be possible to read and
          understand the main program without reference to the
          preamble; and it should be possible to delete the pre-
          amble without changing a single word in the main pro-
          gram.


          EFFECTS OF DEBUGGING WRAPPINGS

          When the DEBUG flag is defined, the following special
          actions are taken:

          1.   The malloc and realloc wrappings install a
               short hidden guard rail at the head of each
               piece of allocated storage.

               The HEAD character may be defined at compilation
               time, and should be chosen so that it is unlikely
               to match values that might be stored by the pro-
               gram being debugged.  If HEAD is not defined, a
               value of AF hex is used.

          2.   The malloc wrapping fills the body of each new
               piece of allocated storage with a specific char-
               acter (or it fills the first 4096 bytes if the
               piece has a length exceeding 4096).

               Also the realloc wrapping fills an extension in
               the same way (or it fills the first 4096 bytes
               if the extension has a length exceeding 4096).

               The BODY character may be defined at compilation
               time, and should be chosen so that it is likely to
               yield obviously wrong results if it is used inad-
               vertently by the program being debugged. If BODY
               is not defined, a value of BF hex is used.

          3.   The malloc and realloc wrappings install a short
               hidden guard at the tail of each piece of alloc-
               ated storage.                                    */

   #pragma page ()

     /*        The TAIL character may be defined at compilation
               time, and should be chosen so that it is unlikely
               to match values that might be stored by the pro-
               gram being debugged.  If TAIL is not defined, a
               value of CF hex is used.

          4.   The free and realloc wrappings check the guard
               rails at the head and tail of each piece of stor-
               age being deallocated or reallocated, and report
               a catastrophe (trampled storage) if they are not
               both intact.

               If the guard rails are both intact, they are re-
               moved by the free or realloc wrapping before the
               primitive free or realloc is called (thus reduc-
               ing the risk of subsequently stumbling on obsol-
               ete guard rails).

          5.   The realloc wrapping detects when the obsolete
               protocol is used, and emits a warning if desired.

          Notes:

          1.   The hidden head guard is 4 bytes long and occupies
               the word preceding the length field. The length field
               contains the length requested+8 (instead of the length
               requested). The hidden tail guard occupies 8 bytes and
               is placed immediately following the last byte of the
               requested piece.

          2.   It is generally desirable to use different characters
               for the head guard, the body fill and the tail guard.

          3.   In the free and realloc wrappings, there is sometimes
               no sure way of distinguishing between an invalid stor-
               age pointer and trampled storage.  (If storage is suf-
               ficiently trampled, all bets are of course off.) Here
               is a summary of the checks that are actually made by
               the free and realloc wrappings:

               a.   If the given storage pointer has a syntact-
                    ically invalid value, or points outside the
                    region from which storage has been allocated,
                    or points within an existing free node, goto
                    step f below.

               b.   If the hidden head guard is not intact,
                    report that either the storage pointer is
                    invalid or storage has been trampled (can-
                    not tell which) and terminate the program.

               c.   Pick up the allocated length from the word
                    preceding the given pointer, subtract 8 to
                    get the original requested length, and so
                    derive the endad of the piece that was
                    supplied to the caller.

               d.   If the endad lies outside the region from
                    which storage has been allocated, or the al-
                    located piece overlaps one or more existing
                    free nodes, or the hidden tail guard is dam-
                    aged, report that storage has been trampled
                    and terminate the program.  (The damage may
                    be in the length field or in the tail guard
                    -- cannot tell which.)

               e.   Remove the old head and tail guards and call
                    the primitive free or realloc. If this is free,
                    the job is now complete, so return.  Otherwise
                    install a new head guard, initialise the new
                    part of the body (if any), and install a new
                    tail guard.                                  */

   #pragma page ()

     /*           (Come here if given pointer seems to be invalid.)

               f.   Call the primitive free or realloc. If this
                    is free, the primitive free will report an
                    error and terminate the program.  Otherwise
                    it may either report an error and terminate
                    the program, or (possibly) execute the obsol-
                    ete realloc protocol and return here. In the
                    latter case, emit a warning if desired, and
                    return to the caller.

          4.   The body is filled to a maximum length of 4096 bytes
               so that the program under test is not prevented from
               acquiring large areas of storage which are to be used
               sparsely (and for which backing store may not exist).

          5.   My apologies for not including the name of the dying
               program and other variable information in the messages
               that report catastrophes. I am told it is difficult to
               obtain the name of the program; and (separate point) it
               is inadvisable to use the usual conversion services (to
               obtain the failing address in hex, for example), since
               these routines are liable to call malloc, etc.

          CJS, Yorktown Heights, July 1990.                           */


     /*           _______________________________________
                 |                                       |
                 |  PREPROCESSOR VARIABLES FOR PREAMBLE  |
                 |_______________________________________|


        ____________________________________________________________
       |          |        |                                        |
       | Variable | Value  | Effect                                 |
       |__________|________|________________________________________|
       |          |        |                                        |
       | DEBUG    | (def)  | Compile this debugging preamble,       |
       |          |        | and also define EXPOSE (see below)     |
       |          |        |                                        |
       |          | (ndef) | Do not compile the debugging pre-      |
       |          |        | amble (and do not define EXPOSE)       |
       |__________|________|________________________________________|
       |          |        |                                        |
       | HEAD     |  char  | Use given char for head    :           |
       |          |        | guard (instead of AF hex)  : Take      |
       |          |        |                            : effect    |
       | BODY     |  char  | Use given char for body    : only if   |
       |          |        | fill (instead of BF hex)   : DEBUG     |
       |          |        |                            : is also   |
       | TAIL     |  char  | Use given char for tail    : defined   |
       |          |        | guard (instead of CF hex)  :           |
       |__________|________|________________________________________|
       |          |        |                                        |
       | WARN     |  >= 0  | Maximum number of times    : This      |
       |          |        | user will be warned about  : takes     |
       |          |        | use of obsolete realloc    : effect    |
       |          |        | protocol                   : only if   |
       |          |        |                            : DEBUG     |
       |          | (ndef) | Equivalent to WARN being   : is also   |
       |          |        | defined with a value of 1  : defined   |
       |__________|________|________________________________________|
       |          |        |                                        |
       | EXPOSE   |        | Expose certain data: see main program  |
       |__________|________|________________________________________|


       When DEBUG is defined, the primitive allocator funtions are
       exposed under the names _malloc, _free and _realloc.  These
       should not normally be called by the application program.   */



   #pragma page ()

   #ifdef DEBUG                         /* If debugging aids wanted   */

     #pragma comment (user,Compiled with DEBUG defined)   /* Obj note */


     #define NULL 0                     /* Usual convention for ptrs  */
     #define GRAIN 16                   /* Granularity of allocation  */
     #define PREFIX 8                   /* Size of the malloc prefix  */


     #include <string.h>                /* Needed for memset, memcmp  */
     #include <stdio.h>                 /* Needed to display warnings */


     struct adlen {                     /* Adlen means addr-length    */
          struct node *p;               /* Contains addr of a node    */
          unsigned     q; };            /* And length of same node    */

     struct node  {                     /* Node contains two adlens   */
          struct adlen a;               /* One describes the left son */
          struct adlen b; };            /* (And the other, the right) */

     typedef struct adlen     adlen;    /* Addr-length pair (2 words) */
     typedef struct node      node;     /* A node header (two adlens) */
     typedef        unsigned  uns;      /* Handy abbrev for unsigned  */


     #ifdef HEAD                        /* Prepare leading guard rail */
       static char const head[4] = {HEAD,HEAD,HEAD,HEAD};
     #else
       static char const head[4] = {0xaf,0xaf,0xaf,0xaf};
     #endif


     #ifdef BODY                        /* Prepare the body fill char */
       static char const body[1] = {BODY};
     #else
       static char const body[1] = {0xbf};
     #endif


     #ifdef TAIL                        /* Prepare the trailing guard */
       static char const tail[8] = {TAIL,TAIL,TAIL,TAIL,TAIL,TAIL,TAIL,TAIL};
     #else
       static char const tail[8] = {0xcf,0xcf,0xcf,0xcf,0xcf,0xcf,0xcf,0xcf};
     #endif


     #ifdef WARN
       static uns   warn    =  WARN;    /* Count for antique warnings */
     #else
       static uns   warn    =   1;      /* Default warning count is 1 */
     #endif


     extern int    *madanch;            /* Provide access to anchor */


     static uns     premin = (0-1),     /* Min addr seen by preamble */
                    premax =   1;       /* Max endad seen by preamble */


     extern void   *_malloc    (uns requested),           /* Primitive */
                    _free      (char *ptr),               /* allocator */
                   *_realloc   (char *ptr,uns requested); /* functions */


     static uns     overlap    (char *u,char *v);     /* Internal subr */

     #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |           MALLOC WRAPPING           |
                  |_____________________________________|         */



     extern void *
     malloc (uns requested) {

       char         *ptr;               /* Addr of allocated piece */

       if ((ptr = _malloc(requested+8)) == NULL)
         return (NULL);                 /* Return if insuf available */

       if ((uns)ptr < premin)           /* Maintain min and max addr */
         premin = (uns)ptr;

       if ((uns)(ptr+requested) > premax)
         premax = (uns)(ptr+requested);

       memset (ptr-PREFIX,head[0],4);      /* Install guard at the head */
       memset (ptr,body[0],requested<4096?requested:4096); /* Fill body */
       memset (ptr+requested,tail[0],8);   /* Install guard at the tail */

       return (ptr);

     };

     #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |            FREE WRAPPING            |
                  |_____________________________________|         */


     extern void
     free (char *ptr) {

       char         *prefix;            /* Addr of malloc's prefix */
       uns           length;            /* The length of the piece */

       if (((uns)ptr < premin) || ((uns)ptr > premax))
         goto proceed;                  /* Pass this on if bad ptr */

       prefix = ptr - PREFIX;           /* Tent malloc prefix addr */

       if ((uns)prefix & (GRAIN-1))     /* Check the ptr alignment */
         goto proceed;

       if (overlap(prefix,prefix))      /* If inside an existing node */
         goto proceed;                  /* let primitive free complain */

       if (memcmp(prefix,head,4))       /* If the head guard is broken */
         goto confusion;                /* report a general confusion */

       length = (*(uns*)(prefix+4)) - 8;  /* Tent allocated length */

       if ((uns)(ptr+length) < (uns)ptr)  /* Beware addr space wrap */
         goto trampled;

       if ((uns)(ptr+length) > premax)  /* If endad has hopeless value */
         goto trampled;                 /* assume the length is damaged */

       if (overlap(prefix,ptr+length+8))  /* If invalid overlap... */
         goto trampled;                   /* report trampled storage */

       if (memcmp(ptr+length,tail,8))   /* Finally check tail guard */
         goto trampled;

       memset (prefix,0,4);             /* Remove both guard rails */
       memset (ptr+length,0,8);

     proceed:

       _free(ptr);                      /* Let primitive free handle */
       return;                          /* And return to my caller */

     confusion:

       write (2,
         "\nCatastrophe in free: trampled storage or invalid ptr\n",
         54);

       abort ();                        /* Terminate the program */

     trampled:

       write (2,"\nCatastrophe in free: trampled storage\n",39);
       abort ();                        /* Terminate the program */

     };

     #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |          REALLOC WRAPPING           |
                  |_____________________________________|         */

     extern void *
     realloc (char *ptr,uns requested) {

       char         *prefix,            /* Addr of malloc's prefix */
                    *new;               /* The addr of the new piece */

       uns           prev,              /* Previous length of piece */
                     have,              /* The length after realloc */
                     incr;              /* The increase in length */

       uns           lap;               /* Zero unless node overlap */


       lap = 0;                         /* Nothing gone wrong yet */

       if ((uns)ptr < premin)           /* If addr precedes my range */
         if (ptr == NULL)               /* check for a null ptr which */
           return (malloc(requested));  /* means 'behave like malloc' */
         else
           goto proceed;                /* Otherwise pass this on */

       if ((uns)ptr > premax)           /* Prevent addressing excps */
         goto proceed;

       prefix = ptr - PREFIX;           /* Tent malloc prefix addr */

       if ((uns)prefix & (GRAIN-1))     /* Check the ptr alignment */
         goto proceed;

       if (lap = overlap(prefix,prefix))  /* If inside existing node */
         goto proceed;                    /* pass to primitive realloc */

       if (memcmp(prefix,head,4))       /* If the head guard is broken */
         goto confusion;                /* report a general confusion */

       prev = (*(uns*)(prefix+4)) - 8;  /* Tent allocated length */

       if ((uns)(ptr+prev) < (uns)ptr)  /* Beware addr space wrap */
         goto trampled;

       if ((uns)(ptr+prev) > premax)    /* If endad has hopeless value */
         goto trampled;                 /* assume the length is damaged */

       if (overlap(prefix,ptr+prev+8))  /* If invalid overlap... */
         goto trampled;                 /* report trampled storage */

       if (memcmp(ptr+prev,tail,8))     /* Finally check tail guard */
         goto trampled;

       memset (prefix,0,4);             /* Remove both guard rails */
       memset (ptr+prev,0,8);

   #pragma page ()

     proceed:

       if (new = _realloc(ptr,requested+8)) {   /* Unless insuf avail */

         ptr = new;                             /* Set ptr to new addr */

         if ((uns)ptr < premin)                 /* (Adjust premin etc) */
           premin = (uns)ptr;

         if ((uns)(ptr+requested) > premax)
           premax = (uns)(ptr+requested);

       };

     /*  At this point, ptr = addr of new piece (if reallocation has
     been successful), or addr of old piece (if insufficient storage
     available)...                                                */

       prefix = ptr - PREFIX;             /* The actual malloc prefix */
       prev = (*(uns*)(prefix)) - 8;      /* The old allocated length */
       have = (*(uns*)(prefix+4)) - 8;    /* (And actual new length) */

       memset (prefix,head[0],4);         /* Install new head guard */

       if (have > prev) {                 /* If increase in length */
         incr = have - prev;              /* Fill bodily extension */
         memset (ptr+prev,body[0],incr<4096?incr:4096);
       };

       memset (ptr+have,tail[0],8);       /* Install new tail guard */

       if (!(lap && warn))                /* Unless he wants warning */
         return (new);                    /* return new or null ptr */

       fprintf (stderr,
         "Warning: at ???????? - obsolete realloc() protocol used.\n"
         );

       if (--warn)                        /* Unless count has expired */
         return (new);                    /* Return a new or null ptr */

       fprintf (stderr,"There will be no more of these warnings.\n");
       return (new);                      /* Return new or null ptr */

     confusion:

       write (2,
         "\nCatastrophe in realloc: trampled storage or invalid ptr\n",
         57);

       abort ();                        /* Terminate the program */

     trampled:

       write (2,"\nCatastrophe in realloc: trampled storage\n",42);
       abort ();                        /* Terminate the program */

     };

   #pragma page ()

     /*        ______________________________________
              |                                      |
              |  OVERLAP -- internal debugging subr  |
              |______________________________________|

          See if the given piece of storage overlaps any
          existing free nodes in the tree.

          Definition is:
               static uns overlap (char*u,char*v)

          where:
               u,v = begend of piece to be checked

          Also:
               madanch = addr of the malloc anchor

          On return:
               ret value = 0 iff there is no overlap

          Note:

               This subr performs a passive search,
               without any restructuring of the tree.  */

     static uns
     overlap (char*u,char*v) {              /* Beg and end of piece */

       adlen *y = (adlen*)madanch;          /* Ptr to adlen of root */

       while (y->p)                         /* Until I fall from tree */

         if ((char*)(y->p) <= u)            /* If begad is to the left */
           if (((char*)(y->p) + y->q) > u)  /* if endad is not to left */
             return (1);                    /* report there is overlap */
           else                             /* Else descend to right */
             y = & y->p->b;
         else                               /* Otherwise (node on right) */
           if ((char*)(y->p) < v)           /* if node inside his piece */
             return (1);                    /* report there is overlap */
           else                             /* (Else descend to left) */
             y = & y->p->a;

       return (0);                          /* Report no overlap */

     };                                 /* (End of subroutine) */


     /*            _____________________________________
                  |                                     |
                  |       REDEFINE CLASHING NAMES       |
                  |_____________________________________|          */


     #define malloc   _malloc           /* Rename primitive malloc */
     #define free     _free
     #define realloc  _realloc

     #define adlen    _adlen            /* Rename datum structures */
     #define node     _node
     #define uns      _uns

     #undef  EXPOSE                     /* Expose ptr to anchor... */
     #define EXPOSE


   #endif


     /*          _________________________________________
                |                                         |
                |        END OF DEBUGGING PREAMBLE        |
                |_________________________________________|       */

   #pragma page ()

   #pragma pagesize (85)

     /*          _________________________________________
                |                                         |
                |    DYNAMIC STORAGE ALLOCATOR FOR AIX    |
                |_________________________________________|
                |                                         |
                |       MALLOC, FREE, REALLOC, ETC.       |
                |_________________________________________|       */


     /*            _____________________________________
                  |                                     |
                  |   DATUM STRUCTURES AND ALGORITHMS   |
                  |_____________________________________|


          The idle pieces of storage are maintained as nodes
          in a 'cartesian' binary search tree in which the nodes
          are ordered left-to-right by address (increasing to the
          right) and top-to-bottom by length (such that no son is
          longer than its father). A node begins with a four-word
          header, thus:


            <---- node header ----> <-- remainder of node -->

            a.          b.
            _________________________________________________
           |     |     |     |     |                         |
           |  p  |  q  |  p  |  q  |  undefined ...          |
           |_____|_____|_____|_____|_________________________|
              /           \
             /             \
            /               \
          left             right
          son               son


          where  a  stands for left hand side
                 b  stands for right hand side
                 p  points to son (null if none)
                 q  is length of son (0 if none)

          (The combination 'p,q' is referred to as an 'adlen'.)

          So if for example n is a ptr containing the address
          of a node, then n->a contains the adlen of this node's
          left son, and also (provided that n->a.p is not null)
          n->a.p->b.q equals the length of the left son's right
          son.

          Nodes are aligned on (0,4w), where w = bytes per word.
          They have a length that is a multiple of 4w -- which
          is also the min supportable granularity of allocation.

          The tree is anchored from 'manchor', which is laid out
          thus:

              manchor....
              ___________
             |     |     |      where p,q = adlen of root,
             |  p  |  q  |      or (null,0) if nonexistent
             |_____|_____|


          The addresses supplied to the caller are aligned on
          (2w,4w), and are preceded by a private prefix, thus:     */

   #pragma page ()

     /*                     ptr
                             |
                 prefix..... |
                 ____________|________________________
                |     |     |                         |
                |  u  |  v  |  allocated storage      |
                |_____|_____|_________________________|


          where u is undefined (malloc) or contains the previous
          length (realloc); and where v is the (unrounded) length
          of the newly allocated storage (malloc and realloc). The
          total length involved, including the prefix and the nec-
          essary allowance for rounding, is:

                      (prefix+v+grain-1) & (0-grain),

          where  prefix = length of the prefix (equals 2w),
                 grain  = granularity of alloc (min of 4w).

          The previous length is supplied by realloc for possible
          use by a realloc wrapping.  Such a wrapping -- which may
          be interposed for tracing or debugging -- may require the
          old length in order to record the event, or to initialise
          the extension when the new length exceeds the old. (Norm-
          ally the wrapping could obtain the old length by loading
          v before calling realloc, but this does not work if the
          obsolete protocol is used -- see notes below.)

          The 'u' field may be used by the wrapping for its own
          purposes; but the 'v' field must not be tampered with.
          (The application program should not of course examine
          or change either field.)

          In the program, the fields that are here called 'u,v'
          are referred to as 'a.p,a.q', thus avoiding the need
          for a union of structures.

          When a piece of storage is released (e.g. by calling
          'free'), the tree is searched for neighbouring nodes;
          and if neighbours are found, they are immediately co-
          alesced with the piece being released.

          The tree is maintained by root insertion. In addition,
          as a side effect of (almost) every descent, the tree is
          splayed or spliced, to the extent possible, in order to
          reduce the expected depth of the nodes visited. Splaying
          and splicing are possible when passing through a 'layer'
          of equal-length nodes. These techniques dramatically im-
          prove the performance of certain pathological scenarios.

          This program was written by C.J.Stephenson, Yorktown
          Heights, in June 1990. It derives from an assembly-
          language program that was written in the summer of
          1989 for YMS (experimental S/370 operating system).   */

   #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |             REFERENCES              |
                  |_____________________________________|


            For a description of root insertion, see:

            [1]  C.J.Stephenson, A method for constructing
                 binary search trees by making insertions
                 at the root, Int J Comp and Inf Sci, Vol
                 9 (1980).

            For a description of the cartesian tree applied
            to storage allocation, see:

            [2]  C.J.Stephenson, Fast Fits: New methods for
                 dynamic Storage allocation, Operating Sys
                 Review (extended abstract), Vol 17 (1983).

            For a description of the splaying algorithm, see:

            [3]  D.D.Sleator and R.E.Tarjan, Self-adjusting
                 binary search trees, JACM, Vol 32 (1985).

            For a description of the splicing algorithm, see:

            [4]  C.J.Stephenson, Private communication with
                 Messrs Sleator and Tarjan (1983 and 1984).     */



     /*            _____________________________________
                  |                                     |
                  |       PREPROCESSOR VARIABLES        |
                  |_____________________________________|


        ____________________________________________________________
       |          |        |                                        |
       | Variable | Value  | Effect                                 |
       |__________|________|________________________________________|
       |          |        |                                        |
       | CHUNK    |  > 0   | Set preferred malloc growth incr;      |
       |          |        | must be a power of 2 and >= GRAIN      |
       |          |        |                                        |
       |          | (ndef) | Use preferred malloc growth incr       |
       |          |        | of 64K bytes.                          |
       |__________|________|________________________________________|
       |          |        |                                        |
       | EXPOSE   | (def)  | Expose following run-time variables:   |
       |          |        |                                        |
       |          |        |   madanch   addr of malloc anchor      |
       |          |        |                                        |
       |          |        |   acquired  accumulated length of      |
       |          |        |             storage acquired from      |
       |          |        |             sbrk() -- normally a       |
       |          |        |             multiple of CHUNK          |
       |          |        |                                        |
       |          |        |   netalloc  net length allocated       |
       |          |        |             -- incl prefixes and       |
       |          |        |             rounding for alignment     |
       |          |        |                                        |
       |          | (ndef) | Conceal all the run-time variables     |
       |__________|________|________________________________________|
       |          |        |                                        |
       | RECALL   |  > 0   | Set max size of free log (see below)   |
       |          |        |                                        |
       |          | (ndef) | Use max free log size of 64 entries    |
       |__________|________|________________________________________| */


   #pragma page ()


     /*            _____________________________________
                  |                                     |
                  |            THE FREE LOG             |
                  |_____________________________________|


          This progam maintains a journal of the last few
          pieces of storage released via 'free', in order to
          provide partial support for the obsolete 'realloc'
          protocol in which the caller passes the address of
          a piece of storage that has already been released.

          The free log (freelog) is an array of 4-word log
          entries. A log entry contains the address of the pre-
          fix (preceding the deallocated storage), the previous
          requested length, and the first two words of the cal-
          ler's data from the storage.  The log is maintained
          forwards and circularly, and the index 'freeze' id-
          entifies the last-used entry. The end of the log is
          marked by an entry in which word 0 is null. (The ar-
          ray is static, so initially all the entries contain
          null.)

          The function malloc simply stores null in word 0 of
          the last-used log entry, thereby purging the entire
          log.  (The obsolete protocol does not permit an old
          piece to be resurrected if the program has called
          malloc or realloc since it was released.)

          A successful call to  'free' results in incrementing
          (or wrapping) the index 'freeze' and constructing a
          log entry at freelog[freeze].

          The function 'realloc' increments (or wraps) the
          index 'freeze' and stores null in the first word of
          the log entry at freelog[freeze]. In one fell swoop
          it thereby marks the end of the log (for this call
          to realloc), and purges the log (for the next call).
          Then, if the piece of storage to be reallocated ap-
          pears to have been released already, realloc searches
          the free log, backwards, for an entry that identifies
          the piece. If a matching entry is found, the realloca-
          tion is performed; otherwise, if the log was not full,
          the call is invalid (complain and terminate); else the
          call is possibly valid, but cannot be handled (apolog-
          ise and terminate).

          The log entries are referred to as if they were node
          headers.  The fields are used thus:

               a.p = the addr of the prefix
               a.q = original requested length
               b.p = 1st word of caller's data
               b.q = 2nd word of caller's data               */

   #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |           ERROR HANDLING            |
                  |_____________________________________|

          This program cannot report errors in the usual way,
          since the runtime services that handle error messages
          are themselves liable to call malloc (or free, or re-
          alloc). Even if fprint() could manage with fixed re-
          sources, I doubt if esperanto could do so -- and it
          would certainly be imprudent to assume it could be
          restrained for ever, release after release.  This
          program therefore bypasses the normal error mess-
          age services, and issues brief catastrophe reports,
          in English, via write().  After a message has been
          written in this way, the process must be terminated
          promptly; for were it to continue, it might write
          pending error message buffers, and the error stream
          would then contain messages out of order. Most of the
          catastrophes preclude continued execution anyway, and
          for them the obligatory termination presents no great
          disadvantage. There is however one situation for which
          process termination is arguably too severe, viz. the
          passing of an invalid pointer to free(). If it turns
          out that termination in this case is a nuisance, free
          could be changed so that it makes an attempt to write
          this particular message through the normal channels.
          (It could guard against recursion by maintaining a
          static flag.)  Whatever happens, I am firmly of the
          opinion that a 'free' error should never be ignored.
          To continue executing the program after such a gross
          mistake, without at least showing a conspicuous warn-
          ing, would be negligent. We must all take some resp-
          onsibility in protecting the user from programs that
          appear to run correctly but are in fact quite broken.

          The catastrophe reports are as follows:

            Catastrophe in malloc: sbrk() error 1a

               System error: malloc is unable to obtain a
               suitably aligned extension from sbrk(), even
               after explicitly aligning the process break
               address with brk().

            Catastrophe in malloc: sbrk() error 1b

               System error: malloc obtained extension from
               sbrk() at unexpected address (preceding that
               of previous extension).

            Catastrophe in malloc: sbrk() error 2

               System error: malloc received consecutive
               but discontiguous extensions from sbrk().    */

   #pragma page ()

     /*     Catastrophe in free: invalid storage ptr

               Programming error: usually the result of
               passing a pointer to free() which is not null,
               and does not address a piece of storage that
               was supplied by malloc() or realloc() and is
               still in the possession of the caller; this
               report may also be elicited as a delayed ef-
               fect of having previously passed an invalid
               pointer to free or realloc which appeared to
               be valid at the time, or following corruption
               of storage.

            Catastrophe in realloc: stale storage ptr

               Implementation restriction: usually the result
               of passing a pointer to realloc() which refers
               to a piece of storage that was freed too long
               ago to be resurrected (excessive number of in-
               tervening free operations when using the obsol-
               ete realloc protocol); it is also possible for
               this report to be elicited if an invalid pointer
               is passed to realloc, or as a delayed effect of
               having previously passed an invalid pointer to
               free or realloc which appeared to be valid at
               the time, or following corruption of storage.

            Catastrophe in realloc: invalid storage ptr

               Programming error: usually the result of
               passing a pointer to realloc() which is not
               null and does not address a piece of storage
               that was supplied by malloc() or realloc() and
               is still in the possession of the caller; this
               report may also be elicited as a delayed effect
               of having previously passed an invalid pointer
               to free or realloc which appeared to be valid
               at the time, or following corruption of stor-
               age.

            Catastrophe in mallinfo: function not supported

               Implementation restriction: the internal data
               that are supposed to be supplied by mallinfo()
               refer to structures that do not exist in this
               implementation of malloc(), etc.           */

   #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |        MISCELLANEOUS NOTES          |
                  |_____________________________________|


          FORMAL DESCRIPTIONS

          Owing to copyright problems, I have not been able to
          include the formal descriptions of the functions im-
          plemented in this program!


          LANGUAGE VIOLATION

          The ANSI draft standard for the C Programming Language
          states in Appendix A (section A.6.2) that the behaviour
          is undefined if:

            An arithmetic operation is invalid (such as division
            or modulus by 0) or produces a result that cannot be
            represented in the space provided (such as overflow
            or underflow)

          This makes it difficult to check whether an arithmetic
          result is too big for the variable to which it is as-
          signed -- since the resulting value is then undefined.

          This program assumes (contrary to the above) that if x,
          y and z are unsigned integers, then after executing the
          statement:

               x = y + z;

          the value of x will be less than y (and also less than z)
          if and only if the true value of y+z exceeds the greatest
          unsigned integer than can be stored in x.


          NON-USE OF CONST ATTRIBUTE

          I do not use the 'const' attribute in this program, owing
          to seemingly inappropriate restrictions in the compiler I
          am using. If for example 'z' and 'adam' are defined thus:

            static adlen       *z;
            static adlen const  adam = {NULL,0-GRAIN};

          then the statement:

            y = & adam;

          elicits the compiler diagnostic:

            EDC0127  If the operands are pointers, they must point
                     to compatible types.


          POINTER ARITHMETIC

          Some compilers use signed arithmetic for manipulating
          pointers.  Therefore I cast pointers to unsigned when
          checking bounds.  This also accounts for why min and
          max are defined as unsigned integers, not pointers.   */

   #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |     DEFINITIONS AND INCLUSIONS      |
                  |_____________________________________|             */

     #define NULL 0                     /* Usual convention for ptrs  */
     #define GRAIN 16                   /* Granularity of allocation  */
     #define PREFIX 8                   /* Size of the malloc prefix  */


     #ifdef RECALL                      /* If free log size is given  */
       #if (RECALL <= 0)                /* Ensure value is acceptable */
         #error Value of RECALL must exceed zero
       #endif
     #else
       #define RECALL 64                /* Else set the default size  */
     #endif


     #ifdef CHUNK                       /* If a growth incr is given  */
       #if ((CHUNK<GRAIN) || (CHUNK&(CHUNK-1)))   /* Check the value  */
         #error Value of CHUNK must be an adequate power of 2
       #endif
     #else
       #define CHUNK 0x10000            /* Else set the default value */
     #endif


     #ifdef EXPOSE                      /* If he wants to see inside  */
       #define CLASS extern             /* expose addr of anchor etc  */
     #else
       #define CLASS static             /* Otherwise make them static */
     #endif


     #define ENDAD(x)   ((char*)x.p+x.q)      /* Endad of the adlen x */
     #define RNDUP(x,y) (((uns)x+y-1)&(0-y))  /* Round x to mult of y */

     #include <sys/errno.h>       
     #include <malloc.h>                /* For mallopt and mallinfo   */
     #include <string.h>                /* (Needed for memmove subr)  */


     extern void  *sbrk (unsigned length),  /* Extend the addr space  */
                   brk  (void *trunc);      /* Or truncate addr space */

     #define BUST ((void*)(0-1))               /* Failure from sbrk() */

     extern int    write (int stream,char *msg,     /* For reporting  */
                          unsigned length);         /* a catastrophe  */

     /*            _____________________________________
                  |                                     |
                  |         TEMPLATES AND TYPES         |
                  |_____________________________________|             */

     struct adlen {                     /* Adlen means addr-length    */
          struct node *p;               /* Contains addr of a node    */
          unsigned     q; };            /* And length of same node    */

     struct node  {                     /* Node contains two adlens   */
          struct adlen a;               /* One describes the left son */
          struct adlen b; };            /* (And the other, the right) */

     typedef struct adlen     adlen;    /* Addr-length pair (2 words) */
     typedef struct node      node;     /* A node header (two adlens) */
     typedef        unsigned  uns;      /* Handy abbrev for unsigned  */

   #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |         PERSISTENT STATIC           |
                  |_____________________________________|             */

     static  adlen     manchor = {NULL,0};   /* Adlen of the root     */

     CLASS   int      *madanch = (int*)&manchor;   /* Addr of anchor  */

     CLASS   uns       acquired = 0,         /* Total bytes acquired  */
                       netalloc = 0;         /* Net length allocated  */

     static  uns       min = (0-1),          /* Min addr seen so far  */
                       max =   1,            /* Max addr seen so far  */

                       chunk = CHUNK;        /* Preferred growth incr */

     static  node      freelog [RECALL];     /* Log of the recent...  */
     static  uns       freeze = 0;           /* ...calls to 'free'    */

     static  uns       info_ordblks=0;	     /* number of blocks      */
					     /* allocated.            */
#ifdef SUNINFO
     static  uns       info_netalloc=0;     /* total requested       */
					     /* memory staisfied.     */
#endif
     /*            _____________________________________
                  |                                     |
                  |          PUBLIC VARIABLES           |
                  |_____________________________________|             */

     static  adlen     x,               /* Adlen of current node      */
                      *y,               /* Addr of attachment point   */
                      *z;               /* Addr of attachment's pa    */

     static  node      stomp;           /* Working scratch node hdr   */

     /*            _____________________________________
                  |                                     |
                  |          PUBLIC CONSTANTS           |
                  |_____________________________________|             */

     static  adlen adam = {NULL,0-GRAIN},  /* Imag ult father (const) */
                   mule = {NULL,0};        /* Node sans issue (const) */

     /*            _____________________________________
                  |                                     |
                  |         EXTERNAL FUNCTIONS          |
                  |_____________________________________|             */

     extern  void             *malloc    (uns requested),
                               free      (char *ptr),
                              *realloc   (char *ptr,uns requested);

     extern  int               mallopt   (int cmd,int value);
     extern  struct mallinfo   mallinfo  ();

     /*            _____________________________________
                  |                                     |
                  |         INTERNAL FUNCTIONS          |
                  |_____________________________________|             */

     static  void             *extend    (uns qq);

     static  uns               leftmost  (uns needed),
                               rightmost (uns needed),
                               splay     (node *u,node *v);

     static  void              splint    (adlen *splice);

     static  adlen            *demote    (adlen x,adlen *y,
                                         adlen aa,adlen bb);

   #pragma page ()

     /*        _____________________________________
              |                                     |
              |               MALLOC                |
              |_____________________________________|


          Allocate a piece of storage.  This is the main
          storage allocator for the C runtime environment.

          This implementation supplies the leftmost avail-
          able piece.

          Definition is:
               extern void *malloc (uns requested)

          where:
               requested = requested length in bytes

          On completion:
               If execution is successful,
                 ret value = storage ptr;
               else if insuf storage avail,
                 return value = null;
               else (system error)
                 report a catastrophe,
                 terminate the process.

          1.   The initial contents of the allocated
               storage are undefined.

          2.   According to the draft ANSI standards for
               C, a request for zero bytes may be handled
               in either of the following two ways:

               (a)  it returns a null pointer (as if zero
                    represented 2#b bytes (b=bits per word);
                    or

               (b)  it returns a (unique) pointer to a piece
                    of storage of zero length.

               This program implements alternative (a).  The
               pointer supplied may be passed to free() or re-
               alloc().  A null pointer is supplied by malloc
               (or realloc) to report an insufficiency or an
 	       error.

          3.   Malloc storage is obtained by extending the
               address space via sbrk(). A new extension is
               obtained when the storage in hand is insuf-
               ficient to satisfy an allocation request.  */


   extern void *
   malloc (uns requested) {

     extern int    errno;

     adlen         ext,                 /* 1st or only new extension  */
                   ext2,                /* 2nd new extension, if any  */
                   runt;                /* Adlen of runt (mule if none) */

     uns           needed,              /* Bytes reqd incl prefix,round */
                   brick;               /* Minimum malloc building block */


     if (requested == 0) {		/* System V behaviour on */
	 errno = EINVAL;		/* ...malloc of zero bytes. */
	 return NULL;	
     }


     needed = RNDUP(requested+PREFIX,GRAIN);  /* Gross length required */

     if (needed < requested)            /* If asking for the world... */
       return (NULL);                   /* ...deliver nothing at all */

     if (needed <= manchor.q)           /* If I already have enough */
       goto join;                       /* allocate it from the tree */

   #pragma page ()

     /*   Arrive here if malloc does not have enough
          storage in hand.

          Find the rightmost node, and see if it is at
          the extreme edge of what I have seen.

          Then obtain an extension to the addr space. If
          the rightmost node reaches the very edge of what
          I have seen, assume (optimistically) that the ex-
          tension will abut this node -- which may allow me
          to get by with a smaller extension. At this point:

               needed = the total length required
               chunk = tentative malloc growth incr

          There are several interesting complications here:

          1.   If the required extension size, rounded up to
               an integral number of chunks (my preferred unit
               of growth), exceeds what is available, I reduce
               the size of my building block, so to speak, and
               ask for a smaller extension (usually), i.e. with
               less rounding up. The program thereby automatic-
               ally adjusts to shrinking available space, and
               (by the same means) it can run in small addr
               spaces, the total size of which may be less
               than my preferred chunk size.

          2.   If I receive an extension that touches the
               very end of the addr range, I must avoid using
               the last GRAIN bytes. (Otherwise I will become
               confused when handling the node having an endad
               of 2#b, where b = bits per word -- and so will
               the caller.)

          3.   At the time of writing (1990/06), the sbrk()
               function in AIX 3 supplies an address which
               is aligned only to a word boundary.  This is
               not acceptable for malloc, since the machinery
               supports double-word objects which perform best
               when fully aligned.  In addition, for this im-
               plementation of malloc, I  want all the nodes
               to be similarly aligned within the quad-word
               -- though I do not care how. (In fact, they
               are aligned at (0,4w).)  The private subr
               extend() handles the alignment mismatch,
               and supplies an aligned extension of the
               required length.                            */


     x = manchor;                       /* (Climb down from root) */
     y = & manchor;

     if (x.q)                           /* Unless the tree is empty */
       while (rightmost(1));            /* find the rightmost node */

     if ((uns)ENDAD(x) != max)          /* If this is not at edge */
       x = mule;                        /* forget rightmost node */

     brick = chunk;                         /* Tentative growth incr */

     do {                                   /* Be prepared to repeat */

       ext.q = RNDUP(needed-x.q,brick);     /* Desired extension size */
       if ((ext.p = extend(ext.q)) != BUST) /* Ask for extension and */
         goto extended;                     /* ...jump if successful */

     } while ((brick >>= 2) >= GRAIN);      /* Try with smaller round */

     return (NULL);                         /* (Report insufficiency) */

   #pragma page ()

     /*   Come here after obtaining an initial extension.
          Check that everything is as I expect. On arrival:

               needed = total length required

               x = adlen of rightmost node, or
                (null,0) if rightmost node does
                not exist or does not abut the
                new extension
               y = addr of righmost node's attach-
                ment pt, or undef if x = (null,0)

               brick = (new) preferred growth incr
                (may possibly be less than chunk)

               ext = adlen of my new extension        */


   extended:

     if ((uns)ext.p & (GRAIN-1))        /* Check the alignment and */
       goto unexpected1a;               /* jump if something wrong */

     if ((uns)ext.p < max)              /* Ensure addrs are climbing */
       goto unexpected1b;

     if ((char*)ext.p != ENDAD(x))      /* If new extension does not */
       x = mule;                        /* touch old edge, forget edge */

     if ((uns)ENDAD(ext) == 0) {        /* If at end of address range */
       ext.q -= GRAIN;                  /* forget the last few bytes */
       if (needed > (x.q+ext.q))        /* If that leaves too little */
         goto insuf;                    /* ...give back and give up */
     };

     if ((x.q+ext.q) >= needed)         /* If I obtained sufficient */
       goto hoard;                      /* then join happily below */

   #pragma page ()

     /*   Arrive here after getting a (first) extension
          if it turns out to be valid but insufficient.

          (This is the penalty for being an optimist. I
          thought that perhaps the extension might touch
          the rightmost node, so I asked for less than I
          really need, and now I have to get some more.)

          On arrival:

               needed = total length required

               x = (null,0), and y is undefined

               brick = (new) preferred growth incr
                (may possibly be less than chunk)

               ext = adlen of my first extension      */

     do {                                     /* Be prepared to repeat */

       ext2.q = RNDUP(needed-ext.q,brick);    /* This is the shortfall */
       if ((ext2.p = extend(ext2.q)) != BUST) /* Get 2nd extension and */
         goto extended2;                      /* ...jump if successful */

     } while ((brick >>= 2) >= GRAIN);        /* Try with smaller round */

     goto insuf;                              /* Clean up, report insuf */


     /*   Come here after obtaining a second extension.
          Check that all is as I expect. At this point:

               needed = total length required

               x = (null,0), and y is undefined

               brick = (new) preferred growth incr
                (may possibly be less than chunk)

               ext = adlen of the first extension
               ext2 = adlen of the 2nd extension    */


   extended2:

     if ((char*)ext2.p != ENDAD(ext))   /* If 2nd extension does not */
       goto unexpected2;                /* touch 1st, report and abort */

     ext.q += ext2.q;                   /* Combine the two extensions */

     if ((uns)ENDAD(ext) == 0) {        /* If at end of address range */
       ext.q -= GRAIN;                  /* forget the last few bytes */
       if (needed > ext.q)              /* If that leaves too little */
         goto insuf;                    /* ...give back and give up */
     };

   #pragma page ()

     /*   Arrive here when I have a sufficient extension.
          Record the new acquisition, and insert it as the
          root of the tree. (I know it must be longer than
          the prev longest node, or I would not be here in
          the first place.)  At this point:

               needed = the total length required

               x = adlen of old rightmost node, or
                (null,0) if rightmost node does not
                exist or it does not abut the new
                extension
               y = addr of righmost node's attach-
                ment pt, or undef if x = (null,0)

               brick = (new) preferred growth incr
                (may possibly be less than chunk)

               ext = adlen of (combined) extension     */

   hoard:

     chunk = brick;                     /* New growth incr, maybe */
     acquired += ext.q;                 /* Accum storage acquired */
     max = (uns)ENDAD(ext);             /* My new maximum address */

     if ((uns)ext.p < min)              /* New min addr also, maybe */
       min = (uns)ext.p;

     if (x.q) {                         /* If abuts old rightmost... */
       *y = x.p->a;                     /* Detach old rightmost node */
       ext.p = x.p;                     /* Coalesce it with extension */
       ext.q += x.q;
     };

     ext.p->a = manchor;                /* Old root becomes left son */
     ext.p->b = mule;                   /* New node has no right son */
     manchor = ext;                     /* New node becomes new root */

   #pragma page ()

     /*   Arrive here if (or when) I have sufficient
          storage in hand to satisfy his request. At
          this point:

               requested = requested length
               needed = gross length needed       */

   join:

     freelog[freeze].a.p = NULL;        /* Purge the free log */

     netalloc += needed;                /* Anticipate success */

     info_ordblks ++;		        /* increment the count of */
					/* ordinary blocks allocated */
					/* to date. */

     stomp.a.p = NULL;                  /* Clobber scratch node */
     stomp.b.p = NULL;

     x = manchor;                       /* Descend from the root */
     y = & manchor;

     while (leftmost(needed));          /* Find leftmost adequate */

     if (x.q > needed) {                /* If something remains... */

       runt.p = (node*)((char*)x.p+needed);  /* Adlen of the runt */
       runt.q = x.q - needed;

       while (1)                        /* Do forever ... ... ... */

         if (x.p->a.q >= x.p->b.q)      /* If left son is longer... */

           if (runt.q >= x.p->a.q)      /* ...if demoted enough... */
             break;                     /* ...then leave this loop */
           else
             splint(&x.p->a);           /* Demote the poor runt */

         else                           /* Right son longer... */

           if (runt.q >= x.p->b.q)
             break;
           else
             splint(&x.p->b);

         *runt.p = *x.p;                /* Slide node header to right */
         *y = runt;                     /* (Attach the runt properly) */

     } else {                           /* He took the entire node... */

       while (x.p->a.q && x.p->b.q)     /* Until only one leg remains */

         if (x.p->a.q >= x.p->b.q)      /* If left leg is the heavier */
           splint(&x.p->a);             /* Promote nodes on left side */
         else
           splint(&x.p->b);

       if (x.p->a.q)                    /* Attach remaining subtree */
         *y = x.p->a;
       else
         *y = x.p->b;

     };

#ifdef SUNINFO
     info_netalloc += requested;	/* update net memory satisfied*/
#endif
     x.p->a.q = requested;              /* Requested length to prefix */
     return ((char*)x.p+PREFIX);        /* Supply node addr + prefix */

   #pragma page ()

     /*   Come here if the inability to supply is dis-
          covered after obtaining an extension to the
          addr space.  Give back the extension, and
          report an insufficiency.  At this point:

               ext = adlen of insufficient extension  */

   insuf:

     brk(ext.p);                        /* Give back the extension */
     return (NULL);                     /* Report an insufficiency */


     /*   Come here if the addrs of space extensions
          are repeatedly unaligned -- even after issuing
          brk() at an aligned addr.  Something is wrong
          with sbrk().  At this point:

               ext = adlen of the funny extension

          I do not return the funny extension.       */

   unexpected1a:

     write (2,"\nCatastrophe in malloc: sbrk() error 1a\n",40);
     abort ();                          /* Terminate the program */


     /*   Come here if the addr of a space extension
          does not exceed the maximum address I have
          seen previously.  Something is wrong with
          sbrk(). At this point:

               ext = adlen of the funny extension

          I do not return the funny extension.    */

   unexpected1b:

     write (2,"\nCatastrophe in malloc: sbrk() error 1b\n",40);
     abort ();                          /* Terminate the program */


     /*   Come here if two consecutive space extensions do
          not have the expected relationship. The beginning
          of the second should touch the end of the first --
          and I depend upon this.  Report the problem, and
          terminate his program.  At this point:

               ext = adlen of the 1st extension
               ext2 = adlen of the 2nd extension

          I do not return the two extensions.             */

   unexpected2:

     write (2,"\nCatastrophe in malloc: sbrk() error 2\n",39);
     abort ();                          /* Terminate the program */

   };                                   /* (The end of malloc) */

   #pragma page ()

    /*            ________________________________
                 |                                |
                 |             FREE               |
                 |________________________________|


          Release a piece of storage that was supplied
          by malloc or realloc.

          Definition is:
               extern void free (char *ptr)

          where:
               ptr = addr of a piece of storage
                previously supplied by malloc or
                realloc (or null if nothing to
                free)

          On completion:
               If execution is successful,
                 return to calling program;
               else (invalid storage ptr)
                 report a catastrophe,
                 terminate the process.

          Note:

               The term 'rel piece' stands
               for 'piece to be released'.           */


   extern void
   free (char *ptr) {

     adlen         rel,                 /* Adlen of the rel piece */
                  *y0;                  /* Addr of attachment point */

     node         *oldend,              /* Rounded end of old piece */
                  *source,              /* Source for new node hdr */
                  *endad;               /* End of the current node */

     adlen        *ahook,               /* Hooks for root insertion */
                  *bhook;

     uns           j,k;                 /* General-purpose integers */

   #pragma page ()

     /*   Check validity of pointer, and maintain the
          free log ...                             */

     if ((uns)ptr <= min)               /* If precedes min addr... */
       if (ptr == NULL)                 /* if nothing to release... */
         return;                        /* return pronto (do nought) */
       else
         goto wrong;                    /* Else report catastrophe  */

     if ((uns)ptr >= max)               /* And beware of addr excps */
       goto wrong;

     rel.p = (node*)(ptr-PREFIX);       /* Addr of malloc's prefix */

     if ((uns)rel.p & (GRAIN-1))        /* Check proper alignment */
       goto wrong;

     rel.q = RNDUP(rel.p->a.q+PREFIX,GRAIN);  /* The gross length */
     oldend = (node*)ENDAD(rel);              /* End of old piece */

     if ((uns)rel.p >= (uns)oldend)     /* Beware of addr space wrap */
       goto wrong;

     if ((uns)oldend > max)             /* Make final address check */
       goto wrong;

     if (++freeze == RECALL)            /* Incr the free log index */
       freeze = 0;                      /* (Or wrap if end of log) */

     freelog[freeze].a.p = rel.p;       /* Log the 'node' address    */
     freelog[freeze].a.q = rel.p->a.q;  /* Log his original length   */
     freelog[freeze].b = rel.p->b;;     /* Log 1st two words of data */

   #pragma page ()

     /*   Start the real work.

          Perform a splayed descent until (a) I encounter a
          neighbour of the rel piece, (b) I see a descendant
          that is no longer than the rel piece, or (c) I drop
          out of the tree (broken arm).  On arrival here:

               rel = the adlen of the rel piece
               (and oldend = endad of old piece)        */


     netalloc -= rel.q;                 /* Be optimistic about outcome */
#ifdef SUNINFO
     info_netalloc -= rel.p->a.q;	/* update net memory alloc'ed */
#endif
     info_ordblks --;			/* decrement allocated block */
					/* count */
     if (info_ordblks < 0)
	 info_ordblks = 0;		/* you never know ... */

     stomp.a.p = NULL;                  /* Clobber working scratch node */
     stomp.b.p = NULL;

     z = & adam;                        /* Imaginary ultimate father */
     y = & manchor;                     /* The actual paternal master */
     x = manchor;                       /* (And the adlen of the root) */

     while (x.q > rel.q)                /* While among the big boys... */
       if ((k = splay(rel.p,oldend)))   /* ...descend another layer... */
         goto render;                   /* ...or jump if see neighbour */




     /*   Arrive here if I see a descendant that is no longer
          than the rel piece (before encountering a neighbour).
          Perform a splayed root insertion -- and continue to
          look for neighbours.  On arrival:

               rel = the adlen of the rel piece
               (and oldend = endad of old piece)

               x = the adlen of the descendant node
               y = the addr of the attachment point
               z = addr of attachment's attachment

          Notes:

          1.   It is appropriate to use splayed root inser-
               tion here (rather than spliced root insertion),
               since I want the inserted node to be placed as
               high as possible in the tree, so as to minimise
               the chance of having to promote it later (if it
               ends up being coalesced with neighbour(s)).

          2.   In the following loop, I add the nodes one-at-
               a-time (if a splay step is not desired, or is
               not possible), or two-at-a-time (otherwise). A
               splay step is desired if two consecutive nodes
               belong on the same side; it is possible if the
               two nodes have the same length.

          3.   Be careful not to modify the storage being
               released until the descent is complete (lest
               there be invalid overlap.                   */

   #pragma page ()

     y0 = y;                            /* Attachment for new node */

     ahook = & stomp.a;                 /* Initiate a new insertion */
     bhook = & stomp.b;

     while (x.q) {                      /* Until I fall from tree... */

       if (x.p < oldend) {              /* If node belongs on left */

         endad = (node*)ENDAD(x);       /* The endad of this node */

         if (endad >= rel.p)            /* Leave loop if left neigh- */
           goto leftout;                /* bour or invalid overlap */

         if ((x.q > x.p->b.q) || ((node*)ENDAD(x.p->b) >= rel.p)) {
           *ahook = x;                  /* (Normal root insertion) */
           ahook = & x.p->b;
           x = *ahook;
         } else {
           *ahook = x.p->b;             /* (Splayed root insertion) */
           x.p->b = ahook->p->a;
           ahook->p->a = x;
           ahook = & ahook->p->b;
           x = *ahook;
         }

       } else {                         /* Node belongs on the right */

         if (x.p == oldend)
           goto rightout;

         if ((x.q > x.p->a.q) || (x.p->a.p <= oldend)) {
           *bhook = x;
           bhook = & x.p->a;
           x = *bhook;
         } else {
           *bhook = x.p->a;
           x.p->a = bhook->p->b;
           bhook->p->b = x;
           bhook = & bhook->p->a;
           x = *bhook;
         }

       }

     };

     /*   Arrive here after inserting the rel piece if I
          fall out of the tree without having found any
          neighbours (or overlapping nodes).  Complete
          the insertion and return happily. At this pt:

               rel = the adlen of the rel piece
               (and oldend = endad of old piece)

               y0 = attachment addr for new node

               stomp = node header for rel piece
               ahook,bhook are the final hooks       */

     *ahook = mule;                     /* Complete the insertion */
     *bhook = mule;

     *rel.p = stomp;                    /* Rel piece gets node header */
     *y0 = rel;                         /* Attachment gets its adlen */

     return;                            /* Return from free */

   #pragma page ()

     /*   Come here if while inserting the rel piece
          I encounter a node with a begad preceding the
          endad of the rel piece and an endad not preced-
          ing the begad of the rel piece. Either the node
          is a left neighbour or there is invalid overlap.
          At this point:

               rel = the adlen of the rel piece
               (and oldend = endad of old piece)

               x = adlen of the interesting node
               (and endad = endad of this node)
               y0 = attachment addr for new node
               z->q = length of attachment node

               stomp = node header for rel piece
               ahook,bhook are final hook addrs
                from the splayed root insertion        */

   leftout:

     *ahook = x.p->a;                   /* Complete the insertion */
     *bhook = x.p->b;

     if (endad != rel.p)                /* Jump if invalid overlap */
       goto restore;                    /* to restore the poor tree */

     rel.p = x.p;                       /* Expand rel piece to left */
     rel.q += x.q;                      /* (And increase its length) */

     y = bhook;                         /* (This is next attachment pt) */
     x = *y;                            /* Adlen of the next node down */

     source = & stomp;                  /* Where I built node header */

     if (x.q == 0)                      /* Jump if fallen out of tree */
       goto descant;

     while (leftmost(1));               /* Find leftmost descendant */

     if (x.p > oldend)                  /* Jump if does not touch */
       goto descant;

     if (x.p < oldend)                  /* Jump if invalid overlap */
       goto repeal;

     *y = x.p->b;                       /* Detach the 2nd neighbour */
     rel.q += x.q;                      /* Expand rel piece to right */
     goto descant;                      /* (And join happily below) */

   #pragma page ()

     /*   Come here if while inserting the rel piece
          I encounter a right neighbour.  At this pt:

               rel = the adlen of the rel piece
               (and oldend = endad of old piece)

               x = adlen of the interesting node
               (and endad = endad of this node)
               y0 = attachment addr for new node
               z->q = length of attachment node

               stomp = node header for rel piece
               ahook,bhook are final hook addrs
                from the splayed root insertion      */

   rightout:

     *ahook = x.p->a;                   /* Complete the insertion */
     *bhook = x.p->b;

     rel.q += x.q;                      /* Expand rel piece to right */

     y = ahook;                         /* (This is next attachment pt) */
     x = *y;                            /* Adlen of the next node down */

     source = & stomp;                  /* Origin of new node header */

     if (x.q == 0)                      /* Jump if fallen out of tree */
       goto descant;

     while (rightmost(1));              /* Find rightmost descendant */
     endad = (node*)ENDAD(x);           /* And the endad of this node */

     if (endad < rel.p)                 /* Jump if it does not touch */
       goto descant;

     if (endad > rel.p)                 /* Jump if invalid overlap */
       goto repair;

     *y = x.p->a;                       /* Detach the 2nd neighbour */
     rel.p = x.p;                       /* Expand rel piece to left */
     rel.q += x.q;
     goto descant;                      /* (And join happily below) */

   #pragma page ()

     /*   Come here if I encounter a neighbour on either
          side (or detect invalid overlap) before seeing a
          descendant that is no longer than the rel piece.

          Coalesce the rel piece with this first neighbour.
          Then look for a possible second neighbour further
          down; and, if found, coalesce the second neighbour
          too, and remove it from the tree.

          (The search for, and removal of, the 2nd neighbour
          is easy.  If for example the 1st neighbour is on the
          left, then it follows from the laws of the tree that
          the 2nd neighbour (if any) will be the leftmost node
          in the 1st neighbour's right subtree, and that it
          cannot have a left son.)

          On arrival:

               rel = the adlen of the rel piece
               (and oldend = endad of old piece)

               x = adlen of the interesting node
               (and endad = endad of this node)
               y = addr of its attachment point
               z->q = length of attachment node

               k = return code from splay subr:

                 1  =>  left neighbour
                 2  =>  right neighbour
                 3  =>  invalid overlap

          Note:

               In what follows, I write 'if (k < 2)' --
               rather than the more obvious 'if (k == 1)'
               -- so as to help the compiler combine the
               test with the 'else if (k == 2)' that ap-
               pears further down.                    */

   #pragma page ()

   render:

     y0 = y;                            /* Remember this attachment pt */
     source = x.p;                      /* Where to get node hdr from */

     if (k < 2) {                       /* If seen my left neighbour */

       rel.p = x.p;                     /* Expand rel piece to left */
       rel.q += x.q;                    /* (And increase its length) */

       y = & x.p->b;                    /* Attachment for descendants */
       x = *y;                          /* Adlen of neigh's right son */

       if (x.q == 0)                    /* Jump if fallen out of tree */
         goto descant;

       while (leftmost(1));             /* Find leftmost descendant */

       if (x.p < oldend)                /* (Jump if invalid overlap) */
         goto rundo;

       if (x.p == oldend) {             /* If this is right neighbour */
         rel.q += x.q;                  /* Expand rel piece to right */
         *y = x.p->b;                   /* And fix it to the ceiling */
       };

     } else if (k == 2) {               /* If seen right neighbour etc */

       rel.q += x.q;
       y = & x.p->a;
       x = *y;

       if (x.q == 0)
         goto descant;

       while (rightmost(1));
       endad = (node*)ENDAD(x);

       if (endad > rel.p)
         goto rundo;

       if (endad == rel.p) {
         rel.p = x.p;
         rel.q += x.q;
         *y = x.p->a;
       };

     } else                             /* Invalid overlap detected */

     goto rundo;                        /* Repair tree, report error */

   #pragma page ()

     /*   Join here for all successful free operations
          in which neighbour(s) have been encountered.

          Hang the coalesced node from the appropriate
          attachment point, and then promote it, if this
          is necessary.

          The attachment point is either the father of the
          first node I saw having a length not exceeding that
          of the rel piece (if I saw such a node before I en-
          countered the first neighbour), or it is the father
          of the first neighbour (if I encountered the first
          neighbour before I saw a descendant that was no
          longer than the rel piece).

          Promotion is necessary if the coalesced node is
          longer than the node which contains its point of
          attachment.

          On arrival:

               rel = the adlen of coalesced node
               (and oldend = endad of old piece)

               source = addr of node header that
               belongs in the new coalesced node

               y0 = addr of the attachment point
               (for the new coalesced node)

               z->q = length of attachment node

          Note:

               If a second descent of the tree is necessary,
               I perform simple root insertion, without splay-
               ing or splicing the tree, since splaying has al-
               ready been done once, during the first descent,
               and to do it again for the same node during the
               same operation would not be algorithmically
               virtuous.                                   */

   descant:

     *rel.p = *source;                  /* Header to coalesced node */
     *y0 = rel;                         /* Hang from the attachment */

     if (rel.q <= z->q)                 /* Return if OK where it is */
       return;

     y = & manchor;                     /* Descend again from root */
     while (y->q > rel.q) {             /* While nodes are too long */
       y = & y->p->a;                   /* Step down to the left son */
       if (y <= (adlen*)rel.p)          /* Or (if I guessed wrong)... */
         y++;                           /* ...step down to right son */
     };

   #pragma page ()

     /*   Perform a root insertion of the coalesced node
          (as described by rel) in the subtree whose root
          is described by *y, and terminate the insertion
          when the old instance of the coalesced node is
          encountered.                                 */

     x = *y;                            /* Adlen of arrogated node */
     *y = rel;                          /* Attach rel piece instead */

     ahook = & rel.p->a;                /* Prepare hooks for insertion */
     bhook = & rel.p->b;
     stomp = *rel.p;                    /* (Simplify the bookkeeping) */

     while (x.p != rel.p)               /* Until I reach coalesced node */

       if (x.p < rel.p) {               /* If this node belongs on left */
         *ahook = x;                    /* Attach node to the left hook */
         ahook = & x.p->b;              /* Move the hook into this node */
         x = *ahook;                    /* And step down to right son */
       } else {                         /* But if on the other hand... */
         *bhook = x;
         bhook = & x.p->a;
         x = *bhook;
       };

     *ahook = stomp.a;                  /* Complete the promotion */
     *bhook = stomp.b;

     return;                            /* And return exhausted */

   #pragma page ()

     /*   Come here if invalid overlap is detected after
          starting to insert the rel piece. It is neces-
          sary to remove the piece, and repair the tree.

          (In the comments here, the term 'false neighbour'
          means the 1st neighbour, if a seemingly valid neigh-
          bour was encountered before the conflict was detect-
          ed; or the conflicting node itself, if the conflict
          was detected before any neighbours were encountered.)

          Replace the rel piece, wherever it is, by the false
          neighbour, and then demote the false neighbour as
          needed.  The tree will not necesssarily have the
          same shape as before, but it will be valid.

          I did not make any preparations for this unfortunate
          situation, so it is necessary to recover the adlen of
          the false neighbour and the length of the rel piece by
          roundabout means. I avoid refetching the length of the
          rel piece from the malloc prefix, since I do not want
          to guarantee that in the course of descending the tree
          I will not have trampled on the invalidly placed pre-
          fix (though in fact I do not believe I have done so).

          On arrival hereabouts:

               rel = the adlen of the coalesced node
               (and oldend = endad of the old piece)

               y0 = the addr of the attachment point
               stomp = node header for rel piece           */


   repeal:                              /* Disengage from left neigh */

     x.p = rel.p;                       /* Adlen of false left neigh */
     x.q = (ptr-PREFIX) - (char*)x.p;
     goto rejoin;

   repair:                              /* Disengage from right neigh */

     x.p = oldend;                      /* Adlen of false right neigh */
     x.q = ENDAD(rel) - (char*)x.p;

   rejoin:                              /* (Repeal,repair join here) */

     rel.q -= x.q;                      /* Length of orig rel piece */

   #pragma page ()

     /*   Join here if invalid overlap is detected before
          coalescing any nodes. Restore the damaged tree.
          (Would that in real life etc ...)  On arrival
          here:

               rel.p is undefined
               rel.q = length of the original rel
                piece (including prefix and round)

               x = the adlen of the false neighbour
               y0 = addr of false neigh's attachment
               stomp = node header for the insertion    */

   restore:

     demote(x,y0,stomp.a,stomp.b);      /* Demote false neighbour */

     /*   Join here if invalid overlap is detected
          before making any changes to the tree.    */

   rundo:

     netalloc += rel.q;                 /* Realism always prevails */
#ifdef SUNINFO
     info_netalloc += rel.p->a.q;	/* update net allocated */
#endif
     freelog[freeze].a.p = NULL;        /* Cancel the free log entry */

     if (freeze-- == 0)                 /* Reset the free log index */
       freeze = RECALL - 1;

     /*   Join here if the given storage ptr is outside
          the valid addr range, or has incorrect alignment.
          Report a catastrophe, and terminate the process
          (see notes at head of program).               */

   wrong:

   /* The following code cannot be executed at this time because we
      are glomming this code onto a production system.  There are 
      places where failures would result, and customers would not
      accept the failure of an application that has been running for
      a long time.

     write (2,"\nCatastrophe in free: invalid storage ptr\n",42);
     abort ();                          /* Terminate the program */
     ;

   };                                   /* (The end of free) */

   #pragma page ()

     /*            _____________________________________
                  |                                     |
                  |              REALLOC                |
                  |_____________________________________|


          Reallocate a piece of storage, and optionally increase
          or decrease the length.

          This function can (for example) be used to deallocate an
          unwanted tail (partial release), or to implement double-
          and-copy (new length greater than old).

          In this implementation, a call to realloc with no change
          in length causes storage compaction (new allocation done
          by leftmost fit).

          Definition is:
               extrn void *realloc (char *ptr,uns requested)

          where:
               ptr = addr of a piece of storage that was
                previously supplied by malloc or realloc,
                or null if a new piece of storage is reqd
               requested = new desired length (in bytes)

          or (obsolete protocol):
               ptr = addr of a piece of storage that was
                supplied by malloc or realloc and then
                released (by calling free)
               requested = new desired length (in bytes)

          On completion:

               If given ptr is null:

                 return (malloc(requested)).

               Else if requested < previous:

                 if execution is successful,
                  unwanted tail is deallocated,
                  return value = ptr as given;
                 else (invalid storage pointer)
                  the situation is handled as
                  described below.

               Else (requested >= previous):

                 if execution is successful,
                   the leftmost piece of storage
                    is allocated (from the union
                    of the existing pool of idle
                    pieces, the given old piece,
                    and a new chunk if required),
                   contents of old piece are cop-
                    ied to beginning of new piece,
                   ret value = addr of new piece;
                 else if insuf storage available,
                   a new piece of storage is not
                    allocated,
                   the old storage is resurrected,
                    (and may continue to be used),
                   return value = null;
                 else (invalid storage pointer)
                   the situation is handled as
                   described below.

               Also:

                 If the new piece is longer than
                 the old, the new tail has unde-
                 fined contents.                        */

   #pragma page ()


     /*   If the given storage pointer is correctly aligned,
          and within bounds, but the storage it addresses is
          not owned by the caller, I search the free log for
          a matching entry, to see if the piece was dealloc-
          ated recently.  If there is an entry, I proceed
          more or less as if the piece had never been deal-
          located -- and, if there is insufficient storage
          available to satisfy the request, I resurrect the
          deallocated piece.  This provides partial support
          for the obsolete protocol.

          If the free log does not contain an entry for the
          given address, I report (a) that the piece is in-
          validly situated (if the log is not full), or (b)
          that it was deallocated too long ago to be recov-
          ered (implementation restriction). In either case,
          the program is terminated (there is nothing else
          I can do).

          When a piece is reallocated after being deallocat-
          ed, the old contents of the first 2 words must be
          obtained from the free log and not from the piece
          itself, since this part of the piece may have been
          overlaid with a node header.

          Notes:

          1.   The draft ANSI standard for C is vague on the
               effects of supplying a requested length of zero.
               It states that 'If size is zero and ptr is not a
               null pointer, the object it points to is freed'.
               It does not define the return value for this
               case.  In this program, I assume that the
               intended effect of the statement:

                    new = realloc(old,0);

               is identical to the sequence:

                    free (old);
                    new = malloc(0);

               so that the final value of 'new' will be null, or
               a unique address, depending upon how malloc behaves.
               (In this implementation, 'new' will contain a unique
               address -- which can be passed to free or realloc --
               unless there is insufficient storage available for
               malloc to supply a piece of zero length, in which
               case 'new' will be null.)

          2.   Most Unix C manuals state that the obsolete protocol
               permits the reallocation of one previously deallocated
               piece provided there have been no intervening calls to
               calloc, malloc or realloc. I have been unable to find
               a nice way of supporting this perfectly, since it re-
               quires the preservation of the entire contents of all
               pieces that are deallocated (until the next call to
               calloc, malloc or realloc).  Walter Daniels observed,
               however, that Berkeley Unix, which claims to support
               the obsolete protocol (and employs a bucket scheme)
               can reliably reallocate a deallocated piece only if
               the piece is among the last four to have been deal-
               located within its bucket. Berkeley Unix has been in
               widespread use for several years, and Daniels has not
               heard of problems arising in this area. This program
               therefore adopts the same approach, except that there
               is a single log of recent calls to free (rather than
               one per size class).  My thanks to to Daniels for
               bringing this to my attention.                      */

   #pragma page ()

     /*   3.   This implementation favours the new protocol. The
               support for the obsolete protocol has no impact on
               performance (beyond the maintenance of the free log),
               provided that the obsolete protocol is not actually
               used.  It could easily be removed from the program
               altogether.

          4.   When the new length equals or exceeds the old, the
               leftmost available piece is allocated (and the old
               data are copied to it), and no attempt is made to
               extend the old piece in situ (and avoid the copy
               operation).  In my opinion, attempts to avoid the
               copy operation have dubious merits, since the cal-
               ling program is not (or should not be) in a posi-
               tion to predict when they are likely to succeed;
               so they may give rise to mysterious variations
               in performance. Repeated reallocations of a large
               piece with small increments in size will perform
               badly; but there are other good reasons for avoid-
               ing this anyway.

          5.   In order to implement the 'realloc' function
               as described above, it is necessary to retain
               the actual requested length in the malloc prefix
               (and not just the actual length of the block after
               rounding up), so that it is possible to distingu-
               ish between a small decrease in length and no
               change (or a small increase).

          6.   Whatever happens, I always perform a thorough
               validity check on the given storage addr.        */


   extern void *
   realloc (char *ptr,uns requested) {

     adlen         old,                 /* Adlen of the old node */
                   rel,                 /* Adlen of the rel piece */
                  *y0;                  /* Addr of attachment point */

     node         *oldend,              /* Endad of the old node */
                  *source,              /* Source for new node hdr */
                  *endad;               /* End of the current node */

     uns           prev,                /* Length of the old piece */
                   needed,              /* New incl prefix + round */
                   lesser;              /* Lesser of prev,requested */

     adlen        *ahook,               /* Hooks for root insertion */
                  *bhook;

     adlen         aside,               /* Adlens of remnants when */
                   bside;               /* resurrecting old piece */

     char         *new;                 /* The addr of the new piece */

     adlen         hdsave;              /* Save area for beg of data */
     node          midsave;             /* Save area for middle part */

     uns           midoff,              /* Offset of mid save data */
                   j,k;                 /* General-purpose integers */

   #pragma page ()

     /*   Terminate the free call log; make a preliminary
          check on the given old address; obtain the apparent
          old length from the malloc prefix; and hence derive
          the old endad.  At this point:

               ptr = the given old address                  */

     if (++freeze == RECALL)            /* Incr the free log index */
       freeze = 0;                      /* (Or wrap if end of log) */

     freelog[freeze].a.p = NULL;        /* End of the free call log */

     if ((uns)ptr <= min)               /* If addr precedes my range */
       if (ptr == NULL)                 /* check for a null ptr which */
         return (malloc(requested));    /* means 'behave like malloc' */
       else                             /* Otherwise this is invalid */
         goto wrong;

     if ((ptr != NULL) && (requested == 0)) {
       free(ptr);
       return malloc(0);
     }

     if ((uns)ptr >= max)               /* Prevent addressing excps */
       goto wrong;

     old.p = (node*)(ptr-PREFIX);       /* Addr of the malloc prefix */

     if ((uns)old.p & (GRAIN-1))        /* (Check proper alignment) */
       goto wrong;

     prev = old.p->a.q;                 /* Apparent original length */
     old.q = RNDUP(prev+PREFIX,GRAIN);  /* (Incl prefix and round) */
     oldend = (node*)ENDAD(old);        /* The apparent old endad */

     needed = RNDUP(requested+PREFIX,GRAIN);  /* Gross length reqd */

     if ((uns)old.p >= (uns)oldend)     /* Beware of addr space wrap */
       goto antique;

     if ((uns)oldend > max)             /* Make final address check */
       goto antique;


     /*   Prepare for the real work.

          If requested length >= apparent previous, prepare
          to release the entire old piece; otherwise prepare
          to release the unwanted tail (or to release nothing
          at all, if after rounding there is no change in the
          actual length).  At this point:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length reqd (or
                indicates impossibly much reqd if
                needed < requested)                        */

     if (requested >= prev) {           /* If bigger piece required */
       rel = old;                       /* Release entire old piece */
       hdsave = old.p->b;               /* Save 1st two words of data */
       midoff = 0;                      /* There are no mid save data */
     } else {                           /* (Releasing unwanted tail) */
       rel.p = (node*)((char*)old.p+needed); /* The addr of the tail */
       rel.q = old.q - needed;          /* Length of tail (may be 0) */
     };

   #pragma page ()

     /*   Perform a splayed descent until (a) I encounter a
          neighbour of the old piece, (b) I see a descendant
          that is no longer than the rel part, or (c) I drop
          out of the tree.  At this point:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

          Also, if requested >= prev:

               hdsave = 1st 2 words of old data
               midoff = 0 (no mid save data yet)

          The term 'rel part' means the part to be re-
          leased, i.e. the part of the old piece to be
          inserted into the tree.                        */

     stomp.a.p = NULL;                  /* Clean working scratch node */
     stomp.b.p = NULL;

     z = & adam;                        /* Imaginary ultimate father */
     y = & manchor;                     /* The actual paternal master */
     x = manchor;                       /* (And the adlen of the root) */

     while (x.q > rel.q)                /* While among the big boys... */
       if ((k = splay(old.p,oldend)))   /* ...descend another layer... */
         goto render;                   /* ...or jump if see neighbour */



     /*   If there is nothing at all to be released (because
          requested < prev, but by such a small amount that
          there is no difference in storage required), then
          I have almost finished.                         */


     if (rel.q == 0)                    /* If nothing to release */
       goto assess;                     /* Join far, far below */

   #pragma page ()

     /*   Arrive here if I see a descendant that is no
          longer than the rel part (before encountering
          any neighbours).  Start a splayed root inser-
          tion -- and continue to look for neighbours.
          On arrival:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the descendant node
               y = addr of its attachment point
               z->q = length of attachment node

          See note in 'free'.                       */

     y0 = y;                            /* Attachment for new node */

     ahook = & stomp.a;                 /* Initiate a new insertion */
     bhook = & stomp.b;

     while (x.q) {                      /* Until I fall from tree... */

       if (x.p < oldend) {              /* If node belongs on left */

         endad = (node*)ENDAD(x);       /* The endad of this node */

         if (endad >= old.p)            /* Leave loop if left neigh- */
           goto leftout;                /* bour (or invalid overlap) */

         if ((x.q > x.p->b.q) || ((node*)ENDAD(x.p->b) >= old.p)) {
           *ahook = x;                  /* Do simple root insertion */
           ahook = & x.p->b;
           x = *ahook;
         } else {
           *ahook = x.p->b;             /* Do splayed root insertion */
           x.p->b = ahook->p->a;
           ahook->p->a = x;
           ahook = & ahook->p->b;
           x = *ahook;
         }

       } else {                         /* Node belongs on the right */

         if (x.p == oldend)
           goto rightout;

         if ((x.q > x.p->a.q) || (x.p->a.p <= oldend)) {
           *bhook = x;
           bhook = & x.p->a;
           x = *bhook;
         } else {
           *bhook = x.p->a;
           x.p->a = bhook->p->b;
           bhook->p->b = x;
           bhook = & bhook->p->a;
           x = *bhook;
         }

       }

     };

   #pragma page ()

     /*   Arrive here after inserting the rel part if I
          fall from the tree without finding neighbours
          (or overlapping nodes). Complete the insertion
          and join far below.  At this point:

               ptr = the given old address
               prev = old length of the piece

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               y0 = the addr of the attachment

               stomp = node header for rel part
               ahook,bhook are the final hooks      */

     *ahook = mule;                     /* Complete the insertion */
     *bhook = mule;

     *rel.p = stomp;                    /* Rel part gets node header */
     *y0 = rel;                         /* Attachment gets its adlen */

     goto assess;                       /* And join happily below */

   #pragma page ()

     /*   Come here if while inserting the rel part I
          encounter a node with a begad preceding the
          endad of the rel part and an endad not pre-
          ceding the begad of the old piece. Either the
          node is a left neighbour to the old piece or
          there is invalid overlap.  At this point:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the interesting node
               (and endad = endad of this node)
               y0 = attachment addr for new node
               z->q = length of attachment node

               stomp = node header (for rel part)

               ahook,bhook are final hook addrs
                from the splayed root insertion    */

   leftout:

     *bhook = x.p->b;                   /* Promote node's right son */

     if (requested < prev) {            /* If releasing unwanted tail */
       *ahook = x;                      /* Attach possible neighbour */
       x.p->b = mule;                   /* (And remove his right son) */
     } else {                           /* If bigger piece is required */
       *ahook = x.p->a;                 /* Promote the node's left son */
       rel.p = x.p;                     /* Expand rel part to the left */
       rel.q += x.q;                    /* (And increase the length) */
     };

     if (endad != old.p)                /* Check for invalid overlap */
       goto repeal;

     y = bhook;                         /* This is next attachment pt */
     x = *y;                            /* Adlen of the next node down */

     source = & stomp;                  /* Origin of new node header */

     if (x.q == 0)                      /* Jump if fallen out of tree */
       goto descant;

     while (leftmost(1));               /* Find leftmost descendant */

     if (x.p > oldend)                  /* Jump if well to the right */
       goto descant;

     if (x.p < oldend)                  /* Check for invalid overlap */
       goto repeal;

     *y = x.p->b;                       /* Detach the 2nd neighbour */
     rel.q += x.q;                      /* Expand rel part to right */
     goto descant;                      /* (And join happily below) */

   #pragma page ()

     /*   Come here if while inserting the rel part I
          encounter a right neighbour.  At this point:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the interesting node
               (and endad = endad of this node)
               y0 = attachment addr for new node
               z->q = length of attachment node

               stomp = node header (for rel part)

               ahook,bhook are final hook addrs
                from the splayed root insertion      */

   rightout:

     *ahook = x.p->a;                   /* Complete the insertion */
     *bhook = x.p->b;

     rel.q += x.q;                      /* Expand rel part to right */

     y = ahook;                         /* This is next attachment pt */
     x = *y;                            /* (Adlen of next node down) */

     source = & stomp;                  /* Origin of new node header */

     if (x.q == 0)                      /* Jump if fallen out of tree */
       goto descant;

     while (rightmost(1));              /* Find rightmost descendant */
     endad = (node*)ENDAD(x);           /* And get endad of this node */

     if (endad < old.p)                 /* Jump if it does not touch */
       goto descant;

     if (endad > old.p)                 /* Check for invalid overlap */
       goto repair;

     if (requested < prev)              /* Jump if releasing the tail */
       goto descant;

     *y = x.p->a;                       /* Detach the second neighbour */
     rel.p = x.p;                       /* Expand rel part to the left */
     rel.q += x.q;
     goto descant;                      /* (And join happily below) */

   #pragma page ()

     /*   Come here if I encounter a neighbour on either
          side (or detect invalid overlap) before seeing a
          descendant that is no longer than the part to be
          released (and therefore before starting the in-
          sertion).  On arrival:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the interesting node
               (and endad = endad of this node)
               y = addr of its attachment point
               z->q = length of attachment node

               k = return code from splay subr:

                 1  =>  left neighbour
                 2  =>  right neighbour
                 3  =>  invalid overlap

          See note in 'free'.                       */

   render:

     y0 = y;                            /* Remember the attachment pt */
     source = x.p;                      /* Tent source of new node hdr */

     if (k < 2)                         /* If node is a left neighbour */
       if (requested < prev)            /* And releasing unwanted tail */
         goto leftin;
       else                             /* Or if bigger piece required */
         goto lefter;
     else if (k == 2)                   /* If node is right neighbour */
       if (rel.q == 0)                  /* If the rel part is empty */
         goto rightin;
       else                             /* Or if something to release */
         goto righter;
     else                               /* Or if invalid overlap ... */
       goto antique;

   #pragma page ()

     /*   Come here if I encounter a left neighbour
          before making a start on the insertion, for
          the case in which the caller is releasing an
          unwanted tail (decrease in length).

          The left neighbour is not directly involved
          (since he abuts the old piece, and not the rel
          part), but I can take advantage of meeting him
          to simplify the remaining traversal; for I know
          that all remaining nodes of interest lie on the
          inner edge of the neighbour's right subtree, and
          I can find (and inspect) them without performing
          any address comparisons. (The following could be
          simplified if I did not attempt to take advant-
          age of this.)

          On arrival:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the interesting node
               (and endad = endad of this node)
               y0 = addr of the attachment point
               z->q = length of attachment node

               source = addr of interesting node

          Note:

               I postpone inserting the rel part until I
               have finished checking its validity, since
               the cost of postponement is negligible (in
               these cases), and error recovery is simpli-
               fied.                                    */

   leftin:

     y = & x.p->b;                      /* Examine the right subtree */
     x = *y;                            /* Adlen of neigh's right son */

     if (rel.q == 0) {                  /* If nothing to be released */
       if (x.q) {                       /* If neighbour has right son */
         while (leftmost(1));           /* Search for right neighbour */
         if (x.p < oldend)              /* Check for invalid overlap */
           goto antique;
       }
       goto assess;                     /* And join happily below */
     };

     if (y0->q < rel.q) {               /* If neigh shorter than rel */
       if (x.q) {                       /* If neighbour has right son */
         while (leftmost(1));           /* Search for right neighbour */
         if (x.p < oldend)              /* Check for invalid overlap */
           goto antique;
         if (x.p == oldend) {           /* If found right neighbour */
           *y = x.p->b;                 /* Unhook him from the tree */
           rel.q += x.q;                /* Expand rel part to right */
         }
       }
       rel.p->a = *y0;                  /* Insert the rel part here */
       rel.p->b = y0->p->b;             /* Take left neigh's rt son */
       y0->p->b = mule;
       goto descent;                    /* And join gladly below */
     };

   #pragma page ()

     /*   (Arrive here if the rel part seems to belong
          below the left neighbour -- easy if neighbour
          has no right son.)                        */

     z = y0;                            /* z->q = length of left neigh */
     y0 = y;                            /* Tentative attachment point */

     if (x.q == 0) {                    /* If neigh has no right son */
       rel.p->a = mule;                 /* Construct new node header */
       rel.p->b = mule;
       goto descent;                    /* And join happily below */
     };

     /*   (Arrive here if the left neighbour is longer than
          the rel part, and has a right son. This gets a bit
          complicated if the neighbour's son is also longer
          than the rel part.  In this case I scramble part
          way down the left edge of the right subtree and
          find the first node whose left son is no longer
          than the rel part, so I know where to insert the
          rel part, assuming all goes well.  (Afterwards I
          will slither down the rest of the way, to see if
          the rel part has a right neighbour, and to check
          for invalid overlap.))                         */

     if (x.q > rel.q) {                 /* If son longer than rel part */

       while (leftmost(rel.q+1)) {      /* Find needed insertion level */
         z = y0;                        /* Keep z->q = paternal length */
         y0 = y;                        /* And y0 = attachment address */
       }

       if (y != y0) {                   /* If layer > one node thick */
         z = y0;                        /* Keep z->q = paternal length */
         y0 = y;                        /* And y0 = attachment address */
       }

       if (x.p != oldend) {             /* If this is not right neigh */
         z = y0;                        /* Keep z->q = paternal length */
         y0 = & x.p->a;                 /* Define new attachment point */
       }

       if (x.p->a.q) {                  /* If more node(s) below here */
         y = & x.p->a;                  /* Prepare appropriate rappel */
         x = *y;                        /* And fall down to the left */
       }

     };

     /*   (Arrive here after finding the highest allowable
          attachment point for the rel part.  Slither down
          to the leftmost node, and thus determine whether
          the rel part has a right neighbour, and check
          for invalid overlap.)                         */

     while (leftmost(1));               /* Find extreme leftmost node */

     if (x.p < oldend)                  /* (Check for invalid overlap) */
       goto antique;

     if (x.p == oldend) {               /* If this is right neighbour */
       *y = x.p->b;                     /* Unhook the right neighbour */
       rel.q += x.q;                    /* Expand rel part to right */
      };

     rel.p->a = mule;                   /* Insert the rel part here */
     rel.p->b = *y0;
     goto descent;                      /* And join exhausted below */

   #pragma page ()

     /*   Come here if I encounter a right neighbour,
          before seeing a descendant that is no longer
          than the part to be released, when requested
          length < previous, but by such a tiny amount
          that there is actually nothing to release.
          Find the rightmost node in the neighbour's
          left subtree (in order to make one final
          check for invalid overlap).  On arrival:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the right neighbour
               y0 = addr of its attachment point
               z->q = length of attachment node    */

   rightin:

     y = & x.p->a;                      /* Descend the right edge of */
     x = *y;                            /* neighbour's left subtree */

     if (x.q == 0)                      /* Jump if neighbour does */
       goto assess;                     /* not have a left subtree */

     while (rightmost(1));              /* Find rightmost descendant */
     endad = (node*)ENDAD(x);           /* And also derive its endad */

     if (endad > old.p)                 /* Check for invalid overlap */
       goto antique;

     goto assess;                       /* Join contentedly below */

   #pragma page ()

     /*   Come here if I encounter a left neighbour
          before seeing a descendant that is no longer
          than the rel part, for the case when the new
          requested length >= previous. Tentatively co-
          alesce the rel part with the left neighbour,
          and proceed down to look for a right neigh-
          bour also (and to check for invalid overlap).
          This is just like 'free'.  At this point:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the right neighbour
               y0 = addr of its attachment point
               z->q = length of attachment node    */

   lefter:

     rel.p = x.p;                       /* Expand rel part to left */
     rel.q += x.q;                      /* (And increase its length) */

     y = & x.p->b;                      /* Examine neigh's rt subtree */
     x = *y;

     if (x.q == 0)                      /* Jump if fallen out of tree */
       goto descant;

     while (leftmost(1));               /* Find the leftmost descendant */

     if (x.p < oldend)                  /* (Check for invalid overlap) */
       goto antique;

     if (x.p == oldend) {               /* If this is right neighbour */
       *y = x.p->b;                     /* Unhook the right neighbour */
       rel.q += x.q;                    /* Expand rel part to right */
     }

     goto descant;                      /* Join happily below */

   #pragma page ()

     /*   Come here if I encounter a right neighbour
          before seeing a descendant that is no longer
          than the rel part, for the case when the rel
          part is not empty.  Tentatively coalesce the
          rel part with the right neighbour, and pro-
          ceed down to look for a left neighbour also
          (and to check for invalid overlap). At this
          point:

               ptr = the given old address
               prev = the apparent old length

               old.p = address of the old prefix
               old.q = apparent old gross length
               (and oldend = the apparent endad)

               requested = new length requested
               needed = new total length needed
               rel = adlen of part to release

               x = adlen of the right neighbour
               y0 = addr of its attachment point
               z->q = length of attachment node    */

   righter:

     rel.q += x.q;                      /* Expand rel part to right */

     y = & x.p->a;                      /* Visit neigh's left subtree */
     x = *y;

     if (x.q == 0)                      /* Jump if fallen out of tree */
       goto descant;

     while (rightmost(1));              /* Find the leftmost descendant */
     endad = (node*)ENDAD(x);           /* And derive its ending address */

     if (endad > old.p)                 /* (Check for invalid overlap) */
       goto antique;

     if (requested >= prev)             /* If requesting enlargement */
       if (endad == old.p) {            /* If a left neighbour exists */
         *y = x.p->a;                   /* Unhook the left neighbour */
         rel.p = x.p;                   /* Expand rel part to left */
         rel.q += x.q;                  /* And increase the length */
       };

   #pragma page ()

     /*   Join here for all successful 'realloc' operations
          in which the rel part has been coalesced with one
          or two neighbours.

          (Come to 'descant' if the new node header has been
          constructed in some other node, or in the working
          scratch node; or come to 'descent' if the new node
          has been constructed in situ.)

          Hang the coalesced node from the given attachment
          point, and then promote it, if this turns out to
          be necessary.

          The attachment point is the father of the first
          node I saw having a length not exceeding that of
          the rel part, if I saw such a node before encount-
          ering a neighbour with which the rel part could be
          coalesced; or it is the father of the first neigh-
          bour, otherwise.

          Promotion is necessary if the coalesced node is
          longer than the node which contains its point of
          attachment.

          On arrival:

               ptr = the given old address
               prev = old length of the piece
               old.p = address of the old prefix
               old.q = gross length of old piece

               requested = new length requested
               needed = new total length needed
               rel = adlen of the coalesced node

               source = addr of the header that
                belongs in the coalesced node

               y0 = addr of the attachment point
               z->q = length of attachment node    */

   descant:

     *rel.p = *source;                  /* Header to coalesced node */

   descent:

     *y0 = rel;                         /* Hang from the attachment */

     if (rel.q <= z->q)                 /* Jump if OK where it is */
       goto assess;

     y = & manchor;                     /* Descend again from root */
     while (y->q > rel.q) {             /* While nodes are too long */
       y = & y->p->a;                   /* Step down to the left son */
       if (y <= (adlen*)rel.p)          /* Or (if I guessed wrong)... */
         y++;                           /* ...step down to right son */
     };

   #pragma page ()

     /*   Perform a root insertion of the coalesced node
          (as described by rel) in the subtree whose root
          is described by *y, and terminate the insertion
          when the old instance of the coalesced node is
          encountered.                                 */

     x = *y;                            /* Adlen of arrogated node */
     *y = rel;                          /* Attach rel part instead */

     ahook = & rel.p->a;                /* Prepare hooks for insertion */
     bhook = & rel.p->b;
     stomp = *rel.p;                    /* (Simplify the bookkeeping) */

     while (x.p != rel.p)               /* Until I reach coalesced node */

       if (x.p < rel.p) {               /* If this node belongs on left */
         *ahook = x;                    /* Attach node to the left hook */
         ahook = & x.p->b;              /* Move the hook into this node */
         x = *ahook;                    /* And step down to right son */
       } else {                         /* But if on the other hand... */
         *bhook = x;
         bhook = & x.p->a;
         x = *bhook;
       };

     *ahook = stomp.a;                  /* Complete the promotion */
     *bhook = stomp.b;

   #pragma page ()

     /*   Come here after successfully completing the
          deallocation, in order to decide what to do
          next.

          If an unwanted tail is being released, all
          that remains is to store the new requested
          length in the malloc prefix, and return the
          same old storage pointer. (Otherwise I have
          to allocate a new piece of storage and copy
          the old to the new ... see below.)

          At this point:

               ptr = the given old address
               prev = old length of the piece

               old.p = address of the old prefix
               old.q = gross length of old piece

               requested = new length requested

               needed = new total length reqd (or
                indicates impossibly much reqd if
                needed < requested)

               rel = adlen of rel part (if rel
                part is not coalesced with any
                neighbour(s)), or adlen of the
                combined node (otherwise)

          Also, if requested >= prev:

               hdsave = 1st 2 words of old data
               midoff = zero (no mid save data)    */

   assess:

     if (requested >= prev)           /* If new length >= old */
       goto newalloc;                 /* handle situation below */

     netalloc -= old.q - needed;      /* Decrease in net allocated */
#ifdef SUNINFO
     info_netalloc += requested - prev;
#endif
     old.p->a.p = (node*)prev;        /* Supply old length in prefix */
     old.p->a.q = requested;          /* New length in standard place */
     return (ptr);                    /* And return the same old ptr */

   #pragma page ()

     /*   Come here if invalid overlap is detected after
          starting to insert of the rel part. It is neces-
          sary to remove the rel part, and repair the tree.

          On arrival:

               ptr = the given old address
               prev = the apparent old length
               old.p = addr of the old prefix
               (oldend = apparent end of old)

               requested = new length requested
               needed = new total length required
               rel = adlen of the coalesced part
                (or undef if false neighbour is
                on left and requested < prev)

               y0 = addr of the attachment point
               stomp = node header from insertion  */

   repeal:                              /* Disengage from left neigh */

     x = mule;                          /* Prepare to delete the node */

     if (requested >= prev) {           /* If planned to release all */
       x.p = rel.p;                     /* Adlen of false left neigh */
       x.q = rel.q - old.q;
     }

     goto restore;

   repair:                              /* Disengage from right neigh */

     x.p = oldend;                      /* Adlen of false right neigh */
     x.q = ENDAD(rel) - (char*)x.p;


     /*   Restore the damaged tree. On arrival here:

               x = adlen of the false neighbour
                or (null,0) if false neighbour is
                on the left and requested < prev

               y0 = addr of the attachment point
               stomp = node header from insertion

          Also:
               ptr = the given old address
               old.p = addr of the old prefix

               requested = new length requested
               needed = new total length needed   */

   restore:

     demote(x,y0,stomp.a,stomp.b);      /* Demote or delete */

   #pragma page ()

     /*   Come here if the piece to be reallocated is not
          owned by the caller, but the given addr is properly
          aligned, and is not outside the range of addresses
          I have seen.

          This may be a genuine mistake; or (just possibly)
          the caller may be using the obsolete protocol which
          allows him to resurrect (and reallocate) a piece of
          storage that he has already released -- provided he
          has performed no subsequent calloc, malloc or re-
          alloc operations.

          Climb passively down from the root, and search for
          the node containing the beginning of the given piece.
          If such a node does not exist, the request is defin-
          itely invalid; otherwise there is still hope ...

          (Depending upon how I got here, it may be that I
          have already visited the node I need; but I am not
          aiming for the ultimate in performance here, and it
          is easier always to search anew than to keep track
          of whether I already have the information.)

          On arrival:
               ptr = the given old address
               old.p = the addr of the prefix

               requested = new length requested
               needed = new total length needed          */

   antique:

     y = & manchor;                     /* Descend from the root */
     while (y->q)                       /* Unless I fall from tree */
       if (y->p <= old.p) {             /* If node is here or to left */
         if ((node*)ENDAD((*y)) > old.p)  /* If node embraces his piece */
           goto foundnode;              /* Jump happily out of the loop */
         y = & y->p->b;                 /* Addr of adlen of left son */
       } else                           /* Or if node is to the right */
         y = & y->p->a;                 /* Addr of adlen of right son */

     goto wrong;                        /* (The request is invalid) */

   #pragma page ()

     /*   Come here after finding the node containing the
          first byte of the given piece. Search the free
          log (backwards) for a recent entry describing
          this piece.  At this point:

               ptr = the given old address
               old.p = the addr of the prefix

               requested = new length requested
               needed = new total length needed

               y = addr of containing node's pa       */

   foundnode:

     if ((j = freeze) == 0)
       j = RECALL;

     while (freelog[--j].a.p) {
       if (freelog[j].a.p == old.p)
         goto foundlog;
       if (j == 0)
         j = RECALL;
     };

     if (j == freeze)                   /* If the free log is full */
       goto unable;                     /* piece is beyond the pale */

     goto wrong;                        /* It is his mistake (whew) */

   #pragma page ()

     /*   Come here after finding the node containing the
          first byte of the given piece, and the log entry
          describing it.  I can now proceed to honour his
          disgusting obsolete request.  At this point:

               ptr = the given old address
               old.p = the addr of the prefix

               requested = new length requested
               needed = new total length needed

               j = index of matching log entry
               y = addr of containing node's pa

          Note:

               You might think I ought to check whether the
               piece to be reallocated is properly contained
               within the node that contains its first byte;
               but the fact that I have found a matching entry
               in the free log guarantees that the piece does
               not cross a node boundary, since the free log
               is purged whenever an allocation is performed. */


   foundlog:

     prev = freelog[j].a.q;             /* Recover previous length */
     old.q = RNDUP(prev+PREFIX,GRAIN);  /* Gross length of old piece */
     hdsave = freelog[j].b;             /* 1st two words of his data */
     netalloc += old.q;                 /* (Make this come out right) */
#ifdef SUNINFO
     info_netalloc += requested - prev;
#endif     
     midoff = 0;                        /* Initialise this belatedly */
     rel = *y;                          /* Adlen of pretend rel part */

   #pragma page ()

     /*   Arrive here if a new piece of storage is
          required. This happens if requested length
          >= previous, or if the obsolete protocol
          is used (irrespective of lengths).

          I obtain the new piece of storage simply
          by calling 'malloc' and then copying the
          old data to the new piece. There is how-
          ever an interesting complication.

          It may happen (for one reason or another)
          that the new piece of storage is allocated
          from the node that contains the old piece;
          and furthermore the new piece may start to
          the left of the old piece, include the beg-
          inning of the old piece, and end within it.
          If this happens, malloc will trample on the
          old piece before I get a chance to copy the
          data to the new piece, since it will create
          a node header just following the new alloca-
          tion.  So if there is a risk of this happen-
          ing, I save these precious few bytes before
          calling malloc, and fill them in by hand
          afterwards.

          At this point:

               ptr = the given old address
               prev = old length of the piece

               old.p = address of the old prefix
               old.q = gross length of old piece

               requested = new length requested

               needed = new total length reqd (or
                indicates impossibly much reqd if
                needed < requested)

               rel = adlen of rel part (if rel
                part is not coalesced with any
                neighbour(s)), or adlen of the
                combined node (otherwise)

               hdsave = 1st 2 words of old data
               midoff = zero (no mid save data)

          Note:
               The phrase 'j &&' in the 'if' statement
               below can be deleted if support for the
               obsolete protocol is removed from this
               program; since, when using the new pro-
               tocol, control reaches this point only
               if requested length >= previous.     */

   newalloc:

     if (needed < requested)            /* If impossibly much wanted */
       goto insuf;                      /* take insufficient short cut */

     j = (char*)old.p - (char*)rel.p;   /* Beginning and ending offset */
     k = j + old.q;                     /* of the old piece in its node */

     if (j && (needed > j) && (needed < k)) {  /* If risk of damage... */
       midoff = needed - j;                    /* this is danger offset */
       midsave = *(node*)(ptr-PREFIX+midoff);  /* Save the old contents */
     };

   #pragma page ()

     /*   At this point, ptr etc are unchanged from
          above; and also:

               hdsave = 1st 2 words of old data

               midoff = offset (within the old
                piece) of additional saved data,
                or 0 if no additional data saved

               midsave = contents of old piece
                at midoff, or undef if midoff=0

          The offset (within the old piece) is measured
          from the beginning of the malloc prefix preced-
          ing the address supplied.

          Note:
               At the time of writing (1990/06), the
               memmove() subr in Rios has an internally
               documented restriction of handling at most
               2#311 bytes.  Since the restriction is not
               documented externally, it is deemed a bug,
               and I am assured the subr will have been
               repaired by the time there are machines
               having sufficient backing store to sup-
               port very long moves.                    */


     if ((new = malloc(requested)) == NULL)  /* Allocate new piece */
       goto insuf;                           /* Jump if insuf avail */

     ((node*)(new-PREFIX))->a.p = (node*)prev;  /* Old length to prefix */

     lesser = (prev<requested) ? prev : requested;  /* The lesser length */
     if (new != ptr)
     	memmove(new,ptr,lesser);             /* Copy from old piece to new */

     *(adlen*)new = hdsave;               /* The original 1st two words */

     if (midoff)                          /* If additional data are saved */
       *(node*)(new-PREFIX+midoff) = midsave;  /* repair possible damage */

     netalloc -= old.q;                   /* Maintain the net allocated */
     info_ordblks--;                      /* Maintain the ordblocks */
     return (new);                        /* Supply addr of new piece */

   #pragma page ()

     /*   Come here if there is insufficient storage
          available to supply a piece of the requested
          length (possible only when requested > prev).

          Resurrect the old piece, and supply a null
          pointer (as an indication of insufficiency).

          Resurrecting the old piece generally requires
          demoting the left remnant of the node containing
          the old piece (was its left neighbour) and also
          the right remnant (was right neighbour); though
          either or both remnants may be missing, or may
          already be at a suitable level.

          At this point:

               ptr = the given old address
               prev = old length of the piece

               old.p = address of the old prefix
               old.q = gross length of old piece

               requested = new length requested
               needed = new total length needed

               rel = adlen of old piece (if
                not been coalesced with any
                neighbour(s)), or adlen of
                combined node (otherwise)

               hdsave = 1st 2 words of data

          Notes:

          1.   I rely upon the fact that malloc does
               not damage the body of the old piece
               if it fails to allocate a new piece.

          2.   In demoting the remnants, I do not splay
               or splice the tree, since the path to the
               old piece (and beyond) has already been
               splayed once, and to do it again (during
               the same operation) would not be algorith-
               mically virtuous.                      */

   insuf:

     y = & manchor;                     /* Descend again from root */

     while (y->p != rel.p) {            /* Until I find needed node */
       y = & y->p->a;                   /* Step down to the left son */
       if (y <= (adlen*)rel.p)          /* Or (if I guessed wrong)... */
         y++;                           /* ...step down to the right */
     };

   #pragma page ()

     /*   Now y = addr of attachment point
          for node containing the rel part    */

     aside.p = NULL;                    /* Adlen of the left remnant */
     if ((aside.q = (char*)old.p - (char*)rel.p))
       aside.p = rel.p;

     bside.p = NULL;                    /* Adlen of the right remnant */
     if (bside.q = ENDAD(rel) - ENDAD(old))
       bside.p = (node*)ENDAD(old);

     if (aside.q >= bside.q) {          /* If left remnant bigger... */
       y = demote(aside,y,y->p->a,y->p->b);  /* ...or if no remnants */
       if (bside.q) {                   /* If rt remnant exists also */
         y = &y->p->b;                  /* Descend to right subtree */
         while (y->q > bside.q)         /* Until reach needed level */
           y = &y->p->a;                /* Descend to the left son */
         aside.p->b = mule;             /* (Terminate on the left) */
         bside.p->b = *y;               /* Insert the 2nd remnant */
         *y = bside;                    /* (Attach to new father) */
       };
     } else {                           /* (Right remnant is bigger) */
       y = demote(bside,y,y->p->a,y->p->b);  /* Demote right remnant */
       if (aside.q) {                   /* If left remnant also, etc */
         y = &y->p->a;
         while (y->q > aside.q)
           y = &y->p->b;
         aside.p->a = *y;
         aside.p->b = mule;
         *y = aside;
       };
     };

     old.p->a.p = (node*)prev;          /* Insert previous length */
     old.p->a.q = prev;                 /* Reinsert original length */
     old.p->b = hdsave;                 /* Original first two words */
     return (NULL);                     /* Report an insufficiency */

   #pragma page ()

     /*   Come here if he seems to be using the obsolete
          protocol, but my free log is not long enough to
          handle the required resurrection. Admit sadly to
          an implementation restriction, and terminate his
          program.  (There is nothing else I can do.  Were
          I to supply a null ptr, he would think the old
          piece had been resurrected; and if I supplied
          a non-null ptr, he would think he had a new
          piece, properly initialised.) At this point:

               ptr = given old storage address       */

   unable:

     write (2,"\nCatastrophe in realloc: stale storage ptr\n",43);
     abort ();                          /* Terminate his program */


     /*   Come here if he passes a storage pointer which I
          can be sure is invalid (and not just stale). Emit
          an abusive message, and terminate his program. (As
          in the previous case, there is nothing else I can
          do; but here I have the satisfaction of knowing
          it is definitely his fault.)  At this point:

               ptr = given old storage address       */

   wrong:

     write (2,"\nCatastrophe in realloc: invalid storage ptr\n",45);
     abort ();                          /* Terminate his program */

   };                                   /* Bitter end of realloc */

   #pragma page ()

     /*          _________________________________
                |                                 |
                |      MALLOPT and MALLINFO       |
                |_________________________________|

     */

#ifdef NOTDEF 

   void disclaim_free_y(void);

   extern int
   mallopt (int cmd,int value) {        /* Set parameters */

     if (acquired > 0)                  /* If calling too late */
       return (1);                      /* return with non-zero */

     switch (cmd) {                     /* Select the given code */

     case M_DISCLAIM:
       disclaim_free_y();               /* disclaim any free space */
       return(0);

     case M_MXFAST:                     /* Small-block threshold */
       if (value < 0)                   /* Check the given value */
         return (1);
       return (0);

     case M_NLBLKS:                     /* Capacity of buckets */
     case M_GRAIN:                      /* Granularity of alloc */
       if (value <= 0)                  /* Check the given value */
         return (1);
       return (0);

     case M_KEEP:                       /* Preserve data after free */
       return (0);                      /* (No value in this case) */

     default:                           /* An invalid command code */
       return (1);

     };                                 /* (The end of the switch) */

   };                                   /* (End of the function) */

static int add_fordblks(int);

#endif /* NOTDEF */

extern struct mallinfo mallinfo () 
{
    static struct mallinfo info;

    info.arena = acquired;	/* acquired storage includes actual */
				/* data and overhead. */

    /*
      I have defined ordblks as the total number of ordinary blocks
      managed by malloc.  This is calculated as:

         no. of blocks currently allocated + 1;
	 
      The additional 1 is for the free tree, which is considered one
      logical block.
    */
    info.ordblks   = info_ordblks + 1;

    /* 
      Small block allocation is NOT supported by this implementation,
      therefore, all of the numbers relavant to small blocks are 0.
    */
    info.smblks    = 0;
    info.hblks     = 0;
    info.hblkhd    = 0;
    info.usmblks   = 0;
    info.fsmblks   = 0;
    info.keepcost  = 0;
#ifdef SUNINFO
    info.mxfast    = 0;
    info.nblks     = 0;
    info.grain     = 0;
#endif

    /*
      This is the "space in ordinary blocks in use".  This value is
      interestesting because it does not include the prefix bytes, but
      it does include the memory resulting from the rounding up to
      GRAIN.  Why? I don't know, but that's what SUN puts here.
    */
    info.uordblks  = netalloc - (info_ordblks * PREFIX);

    /*
      The number of bytes in free'd blocks (i.e. the free tree in this
      case).
    */
    info.fordblks  = add_fordblks(&manchor);

#ifdef SUNINFO
    /* 
      This is the "space allocated in ordinary blocks". This number
      includes the PREFIX.
    */
    info.uordbytes = netalloc;

    /*
      This is the number of blocks actually allocated.
    */
    info.allocated = info_ordblks;

    /* 
      Other implementations result in tree overhead numbers
      here.  Our answer should always be zero.  This is the memory
      that is neither allocated nor in the free list ready for 
      allocation.
    */
    info.treeoverhead = acquired - manchor.q - netalloc;
#endif

    return info;
}

static
int
add_fordblks(adlen *adl)
{
	int	fordblks;

	/**********
	  check if there are any children
	**********/
	if (adl->p == NULL)
		return(0);

	/**********
	  count this space
	**********/
	fordblks = adl->q;

	/**********
	  do left child
	**********/
	fordblks += add_fordblks(&(adl->p->a));

	/**********
	  do right child
	**********/
	fordblks += add_fordblks(&(adl->p->b));

	/**********
	  return the number of blocks found
	**********/
	return (fordblks);
}
	

   #pragma page ()

     /*       _____________________________________
             |                                     |
             |       EXTEND -- internal subr       |
             |_____________________________________|

          Obtain an aligned extension to the addr space.

          Definition is:
               void *extend (uns qq);

          where:
               qq = required extension size (zero
               means 2#b, where b = bits per word)

          Also:
               GRAIN = reqd alignment for extension

          On return:
               return value = addr of new extension,
               or BUST if extension is not available

          If an extension has unsuitable alignment,
          almost all of it is thrown back, so as to
          leave the break address aligned, and a new
          (aligned) extension is obtained.  I do not
          expect this to happen very often.

          Note:

               This subr can be scrapped if the addr
               supplied by sbrk() is guaranteed to be
               aligned at (0,GRAIN) or better.     */


   static void *
   extend (uns qq) {

     void         *pp;                  /* Local temporary ptr */

     if ((int)qq <= 0)			/* The max sbrk() can handle */
       return BUST;			/* ...is 2^32-1 */

     pp = sbrk(qq);                     /* Bid for a new extension */

     if (pp == BUST)                    /* If it was not supplied */
       return (pp);                     /* ...yield nothing at all */

     if (((uns)pp & (GRAIN-1)) == 0)    /* If suitably aligned... */
       return (pp);                     /* ...then return happily */

     brk((char*)RNDUP(pp,GRAIN));       /* Align the brk address */
     return (sbrk(qq));                 /* Supply new extension */

   };                                   /* (End of subroutine) */

   #pragma page ()

     /*        _________________________________
              |                                 |
              |    LEFTMOST -- Internal subr    |
              |_________________________________|

          Find the leftmost node in the given layer of
          the given subtree; perform splicing operations
          in the layer; and return information about the
          next layer down.

          Definition is:
               static uns leftmost (uns needed)

          where:
               needed = needed length in
                bytes (1 if nought needed)

          and also:
               x = adlen of 1st node to be
                examined (at top of layer
                to be searched)
               y = addr of attachment point
                (at bottom of prev layer)

          On return:
               If the next layer (below
               the given layer) contains
               nodes of adequate size,
                x = adlen of next node
                 (at top of next layer),
                y = its attachment point
                 (in the leftmost node
                 of the given layer),
                return value = 1 (true);
               else
                x = adlen of the deepest
                 node of adequate length
                 (bottom of given layer),
                y = attachment point for
                 the node described by x,
                return value = 0 (false).

          Note:
               On entry, the node described by
               x must be properly attached to y.       */


   static uns
   leftmost(uns needed) {

     while (x.q == x.p->a.q) {          /* While still in this layer */

       y->p = x.p->a.p;                 /* Promote my left son... */
       x.p->a = y->p->b;
       y->p->b = x;                     /* ...and demote my node  */
       x.p = y->p;

       if (x.q > x.p->a.q)              /* Break if next node... */
         break;                         /* ...starts a new layer */

       y = & x.p->a;                    /* The next attachment pt */
       x.p = y->p;                      /* The next node to visit */

     };                                 /* (The end of the while) */

       if (needed > x.p->a.q)           /* If next layer too short... */
         return (0);                    /* ...supply answer of false */

       y = & x.p->a;                    /* The next attachment point */
       x = *y;                          /* Step down to next layer */
       return (1);                      /* Return answer of true */

   };                                   /* (End of subroutine) */

   #pragma page ()

     /*        _________________________________
              |                                 |
              |    RIGHTMOST -- Internal subr   |
              |_________________________________|

          Find the rightmost node in the given layer of
          the given subtree; perform splicing operations
          in the layer; and return information about the
          next layer down.

          Definition is:
               static uns rightmost (uns needed)

          where:
               needed = needed length in
                bytes (1 if nought needed)

          and also:
               x = adlen of 1st node to be
                examined (at top of layer
                to be searched)
               y = addr of attachment point
                (at bottom of prev layer)

          On return:
               If the next layer (below
               the given layer) contains
               nodes of adequate size,
                x = adlen of next node
                 (at top of next layer),
                y = its attachment point
                 (in the rightmost node
                 of the given layer),
                return value = 1 (true);
               else
                x = adlen of the deepest
                 node of adequate length
                 (bottom of given layer),
                y = attachment point for
                 the node described by x,
                return value = 0 (false).

          Note:
               On entry, the node described by
               x must be properly attached to y.       */


   static uns
   rightmost(uns needed) {

     while (x.q == x.p->b.q) {          /* While still in this layer */

       y->p = x.p->b.p;                 /* Promote my right son... */
       x.p->b = y->p->a;
       y->p->a = x;                     /* ...and demote my node  */
       x.p = y->p;

       if (x.q > x.p->b.q)              /* Break if next node... */
         break;                         /* ...starts a new layer */

       y = & x.p->b;                    /* The next attachment pt */
       x.p = y->p;                      /* The next node to visit */

     };                                 /* (The end of the while) */

     if (needed > x.p->b.q)             /* If next layer too short... */
       return (0);                      /* ...supply answer of false */

     y = & x.p->b;                      /* The next attachment point */
     x = *y;                            /* Step down to next layer */
     return (1);                        /* Return answer of true */

   };                                   /* (End of subroutine) */

   #pragma page ()

     /*          _________________________________
                |                                 |
                |     SPLINT -- Internal subr     |
                |_________________________________|

          Descend through the given layer of the tree and
          perform a splicing operation (within the layer)
          w.r.t. the given address.

          Definition is:
               static void splint (adlen *splice)

          Uses static parameter:
               static adlen *y      (attachment point)

          On entry:
               splice = splice addr, or splice
                addr + 8 (it is undefined which)
               *splice = adlen of 1st node to be
                examined (at top of layer to be
                searched)
               y = addr of the attachment point
                (at bottom of the prev layer)

          On return:
               *splice = adlen of the next node
                (at the top of the next layer)
               y = the next attachment point
                (at bottom of given layer)

          Notes:

          1.   On entry, the pointer fields in the working
               scratch node header (stomp) must be set such
               that there is no risk of their matching the
               address of any nodes in the layer to be oper-
               ated on. On return, these address fields may
               be unchanged, they may contain the addr of
               node(s) in the layer operated on, or they
               may be null -- it is undefined which. (The
               length fields have no requirement on entry,
               and are undefined on return.)

          2.   On entry, the node described by *splice
               need not be connected to y.

          (Uses stomp (working scratch node) -- see above.)  */


   static void
   splint (adlen *splice) {

     adlen        *ahook,               /* Hooks for root insertion */
                  *bhook;

     node         *pp;                  /* Need a scratch ptr here */

   #pragma page ()

     /*   Handle the simple case quickly (layer only one
          node thick); or prepare for spliced descent (if
          layer is more than one node thick).         */

     if (splice > (adlen*)splice->p) {  /* If node belongs on left */

       if (splice->q != splice->p->b.q) {  /* If layer one node thick */
         *y = *splice;                  /* Make my paternal attachment */
         y = & splice->p->b;            /* Addr of the next attachment */
         *splice = *y;                  /* Adlen of the next node down */
         return;                        /* And make fast return (easy) */
       } else {                         /* If more than one node here */
         stomp.a = *splice;             /* Attach this node on left */
         ahook = & splice->p->b;        /* Initialise the tree hooks */
         bhook = & stomp.b;
         splice->p = ahook->p;          /* And descend to the right */
       }

     } else {                           /* Node belongs on the right */

       if (splice->q != splice->p->a.q) {
         *y = *splice;
         y = & splice->p->a;
         *splice = *y;
         return;
       } else {
         stomp.b = *splice;
         ahook = & stomp.a;
         bhook = & splice->p->a;
         splice->p = bhook->p;
       }

     };                                 /* (End of the outer else) */

   #pragma page ()

     /*   Arrive here if this layer in the tree is more
          than one node thick. Perform a spliced descent
          (until I reach the next layer). At this point:

               splice = the given splice address
               *splice = adlen of descendant node
               y = addr of the attachment point
               ahook,bhook are prepared for the
                spliced root insertion                */

     while (1)                          /* Please do forever ... ... */

       if (splice > (adlen*)splice->p) {   /* If node belongs on left */

         if (splice->q != splice->p->b.q)  /* If falling out of layer */
           goto leftout;                   /* Leave loop (see below) */

         if (splice->p == ahook->p) {   /* If splice is needed here... */

           *ahook = splice->p->a;       /* Deposit left son with uncle */
           *y = *splice;                /* And attach me to the ceiling */

           pp = splice->p;              /* (Remember this node addr) */
           splice->p = pp->b.p;         /* Root of the next subtree */

           *pp = stomp;                 /* Promote the lucky node */
           y = bhook;                   /* Tentative attachment addr */

           if (bhook == & stomp.b)      /* If right hook -> temp node */
             y = & pp->b;               /* Revise the attachment point */

           ahook = & stomp.a;           /* Now start a new insertion */
           bhook = & stomp.b;

         } else {                       /* Splice is not needed here */

           *ahook = *splice;            /* Attach node on the left */
           ahook = & splice->p->b;      /* Prepare a new left hook */
           splice->p = ahook->p;        /* Descend to my right son */

         }

       } else {                         /* Node belongs on the right */

         if (splice->q != splice->p->a.q)
           goto rightout;

         if (splice->p == bhook->p) {

           *bhook = splice->p->b;
           *y = *splice;

           pp = splice->p;
           splice->p = pp->a.p;

           *pp = stomp;
           y = ahook;

           if (ahook == & stomp.a)
             y = & pp->a;

           ahook = & stomp.a;
           bhook = & stomp.b;

         } else {

           *bhook = *splice;
           bhook = & splice->p->a;
           splice->p = bhook->p;

         }

       };

   #pragma page ()

     /*   Come here if the current node belongs on the
          left, and its right son is either absent or
          resides in the next layer down. Finish the
          spliced insertion and return. At this point:

               splice = the given splice addr
               *splice = adlen of descendant node
               y = addr of the attachment point
               ahook,bhook are the final hooks        */

   leftout:

     *ahook = splice->p->a;             /* Reattach my two sons */
     *bhook = splice->p->b;

     *(splice->p) = stomp;              /* Promote the lucky node */
     *y = *splice;                      /* Attach me to the ceiling */

     y = & splice->p->b;                /* Tentative attachment addr */
     *splice = *bhook;                  /* Adlen of the next subroot */

     if (bhook == & stomp.b)            /* If right hook -> temp node */
       return;                          /* ...return (guessed right) */

     y = bhook;                         /* Revised attachment point */
     return;

     /*   Arrive here if the current node belongs on
          the right, etc ... (see leftout, above) ...  */

   rightout:

     *ahook = splice->p->a;
     *bhook = splice->p->b;

     *(splice->p) = stomp;
     *y = *splice;

     y = & splice->p->a;
     *splice = *ahook;

     if (ahook == & stomp.a)
      return;

     y = ahook;
     return;

   };                                   /* (End of subroutine) */

   #pragma page ()

     /*          _________________________________
                |                                 |
                |      SPLAY -- Internal subr     |
                |_________________________________|

          Search the given layer of the tree for nodes that
          touch the given rel piece, and (as a side-effect)
          performs a splaying operation within the layer.

          Definition is:
               static uns splay (node*u,node*v)

          where:
               u,v = begend of rel piece

          Also:
               x = adlen of the 1st node
                to be examined (at top of
                the layer to be searched)
               y = addr of attachment pt
                (at bottom of prev layer)
               z = addr of attachment's
                attachment

          On return:
                If the rel piece has no
                neighbours in given layer:
                  x = adlen of next node (at
                   top of the next layer), or
                   (null,0) if fallen from tree,
                  y = the next attachment point
                   (at bottom of this layer),
                  z = the value of y on entry,
                  return value = 0.

                (In this case an undefined node
                is hung from given attachment pt.)

                If a neighbour is encountered:
                  x = adlen of the neighbour,
                  y,z are unchanged from entry,
                  ret value = 1 (if x is left
                   neighbour) or 2 (if right).

                (In this case the neighbour is
                hung from given attachment pt.)

                If rel piece overlaps existing node:
                  x = adlen of the conflicting node,
                  y,z are unchanged from entry,
                  return value = 3.

                (In this case the conflicting node
                is hung from given attachment pt.)

          Notes:

          1.   On entry, the pointer fields in the working
               scratch node header (stomp) must be set such
               that there is no risk of their matching the
               address of any nodes in the layer to be oper-
               ated on. On return, these address fields may
               be unchanged, they may contain the addr of
               node(s) in the layer operated on, or they
               may be null -- it is undefined which. (The
               length fields have no requirement on entry,
               and are undefined on return.)

          2.   On entry, the first node to be examined must
               be properly connected to the given attachment.
               On return, the tree will be rearranged, but
               valid.

          3.   If a neighbour is encountered, the search
               stops immediately, without looking for a
               possible second neighbour.

          (Uses stomp (working scratch node) -- see above.)  */


   static uns
   splay (node *u,node *v) {

     adlen        *ahook,               /* Hooks for root insertion */
                  *bhook,

                  *ahookpa,             /* Addr of left hook's father */
                  *bhookpa;             /* Addr of left hook's father */

     node         *endad;               /* End of the current node */

     /*   Handle the simple case quickly (layer only
          one node thick); or prepare for splayed root
          insertion (if layer more than one node thick).   */

     if (v > x.p) {                     /* If given node is on left */

       endad = (node*)ENDAD(x);         /* The endad of this node */

       if (u == endad)                  /* If this is left neighbour... */
         return (1);                    /* ...yield ret value 1 (good) */

       if (u < endad)                   /* If invalid overlap yield... */
         return (3);                    /* ...return value of 3 (bad) */

       if (x.q != x.p->b.q) {           /* If layer only one node thick */
         z = y;                         /* Set z as required for caller */
         y = & x.p->b;                  /* Addr of the next attachment */
         x = *y;                        /* Adlen of the next node down */
         return (0);                    /* And make fast return (easy) */
       } else {                         /* If more than one node here */
         stomp.a = x;                   /* Attach this node on left */
         ahook = & x.p->b;              /* Initialise the tree hooks */
         ahookpa = & stomp.a;           /* (Left hook's father also) */
         bhook = & stomp.b;
         x.p = ahook->p;                /* And descend to the right */
       }

     } else {                           /* Given node is on the right */

       if (v == x.p)                    /* If this is rt neighbour... */
         return (2);                    /* ...yield ret value 2 (good) */

       if (x.q != x.p->a.q) {           /* If only one node thick etc */
         z = y;
         y = & x.p->a;
         x = *y;
         return (0);
       } else {
         stomp.b = x;
         ahook = & stomp.a;
         bhook = & x.p->a;
         bhookpa = & stomp.b;
         x.p = bhook->p;
       }

     };

   #pragma page ()

     /*    Arrive here if this layer in the tree is more
           than one node thick. Perform a splayed descent,
           until I encounter a neighbour or I see the next
           layer down in the tree.  At this point:

                u,v are unchanged from entry
                x = adlen of the descendant
                y,z are unchanged from entry

                ahook,bhook are ready for
                 the splayed root insertion
                ahookpa = addr of left hook's
                 father (undef is not needed)
                bhookpa = addr of right hook's
                 father (undef is not needed)           */


     while (1)                          /* Do forever ... ... ... */

       if (v > x.p) {                   /* If this node is on left */

         endad = (node*)ENDAD(x);       /* The endad of this node */

         if (u == endad)                /* If this is left neigh... */
           goto leftin;                 /* ...leave loop, see below */

         if (u < endad)                 /* Likewise if invalid overlap */
           goto splat;

         if (x.q != x.p->b.q)           /* Or if falling out of layer */
           goto leftout;

         if (x.p != ahook->p) {         /* If rotation not needed ... */
           *ahook = x;                  /* Attach this node on left */
           ahookpa = ahook;             /* Adjust left hook's father */
           ahook = & x.p->b;            /* This is the new left hook */
           x.p = ahook->p;              /* Descend to my right son */
         } else {                       /* Or if rotation needed ... */
           *ahook = x.p->a;             /* New house for my left son */
           x.p->a.p = (node*)(ahook-1); /* Take care of my father */
           x.p->a.q = x.q;
           ahookpa->p = x.p;            /* My grandpa cares for me */
           ahook = & x.p->b;            /* This is new left hook */
           x.p = ahook->p;              /* Descend to right son */
           ahook->p = NULL;             /* (Clobber hook addr) */
         }

       } else {                         /* Given node is on the right */

         if (v == x.p)                  /* If found right neighbour... */
           goto rightin;                /* ...then deal with it below */

         if (x.q != x.p->a.q)           /* If the layer ends here etc */
           goto rightout;

         if (x.p != bhook->p) {         /* If rotation not needed etc */
           *bhook = x;
           bhookpa = bhook;
           bhook = & x.p->a;
           x.p = bhook->p;
         } else {
           *bhook = x.p->b;
           x.p->b.p = (node*)bhook;
           x.p->b.q = x.q;
           bhookpa->p = x.p;
           bhook = & x.p->a;
           x.p = bhook->p;
           bhook->p = NULL;
         }

       };                               /* (End of the outer else) */

   #pragma page ()

     /*   Come here if I encounter a neighbour before
          I see down into the next layer.  Attach the
          neighbour's sons to the hooks, and hang the
          neighbour from the given attachment.  At
          this point:

               x = adlen of neighbouring node
               y = the original attachment point
               ahook,bhook are the final hooks         */


   leftin:                              /* Handle left neighbour */

     *ahook = x.p->a;                   /* Attach both my sons */
     *bhook = x.p->b;
     *(x.p) = stomp;                    /* Promote to the root */
     *y = x;                            /* Attach to the ceiling */
     return (1);                        /* Report left neighbour */

   rightin:                             /* Handle right neighbour */

     *ahook = x.p->a;
     *bhook = x.p->b;
     *(x.p) = stomp;
     *y = x;
     return (2);


     /*   Come here if I see the next layer without
          encountering a neighbour. Attach the lower
          layer to the hooks, and supply the first
          node in the next layer.  At this point:

               x = adlen of the descendant node
               y = the original attachment point
               ahook,bhook are the final hooks         */


   leftout:                             /* This node is on left */

     *ahook = x.p->a;                   /* Attach both my sons */
     *bhook = x.p->b;
     *(x.p) = stomp;                    /* Promote to the root */
     *y = x;                            /* (Fix to the ceiling) */

     z = y;                             /* The original attachment */

     y = & x.p->b;                      /* Tentative new attachment */
     x = *bhook;                        /* Descend to the next layer */

     if (bhook == & stomp.b)            /* If right hook -> temp node */
      return (0);                       /* Return (I guessed right) */

     y = bhook;                         /* Revised attachment point */
     return (0);

   rightout:                            /* This node is on right */

     *ahook = x.p->a;
     *bhook = x.p->b;
     *(x.p) = stomp;
     *y = x;

     z = y;

     y = & x.p->a;
     x = *ahook;

     if (ahook == & stomp.a)
      return (0);

     y = ahook;
     return (0);

   #pragma page ()

     /*   Come here if I detect invalid overlap.
          Attach the conflicting node's sons to the
          tree hooks, and hang the conflicting node
          from the given attachment point. At this
          point:

               x = adlen of the conflicting node
               y = the original attachment point
               ahook,bhook are the final hooks        */

   splat:                               /* Trouble in paradise */

     *ahook = x.p->a;                   /* Attach both my sons */
     *bhook = x.p->b;
     *(x.p) = stomp;                    /* Promote to the root */
     *y = x;                            /* (Fix to the ceiling) */
     return (3);                        /* And return angrily */

   };                                   /* (End of subroutine) */

   #pragma page ()

     /*      _____________________________________
            |                                     |
            |       DEMOTE -- internal subr       |
            |_____________________________________|

          Demote or delete a node in the cartesian tree.

          Definition is:
               adlen *demote (x,y,aa,bb)

          where:
               x = adlen of node to be demoted,
                or (null,0) if node to be deleted
               y = addr of the attachment point
               aa = adlen of the left subroot, or
                (null,0) if left subtree is empty
               bb = adlen of the right subroot, or
                (null,0) if right subtree is empty

          On return:
               ret value = addr of new attachment
                for demoted node, or null if node
                has been deleted

          Notes:

          1.   This subr is intended for repairing or re-
               storing the tree when something goes wrong.
               It has been written with an eye to simplicity
               and compactness: it does not splay or splice
               the tree, and it does not aim to have the
               shortest possible path length.

          2.   The long fancy while statement below can be
               expressed in words thus::

                 if a node is being demoted and I have not
                 yet descended far enough to reattach it, or
                 if a node is being deleted and I have not yet
                 fallen out of either subtree, then continue
                 looping.                                   */

   static adlen *
   demote (adlen x,adlen *y,adlen aa,adlen bb) {

     while ((x.q && ((aa.q>x.q) || (bb.q>x.q))) || (!x.q && aa.q && bb.q))

       if (aa.q >= bb.q) {              /* If left subroot is heavier */
         *y = aa;                       /* Promote the heavier subroot */
         y = &aa.p->b;                  /* Attachment falls to right */
         aa = *y;                       /* Slither down inside edge */
       } else {
         *y = bb;
         y = &bb.p->a;
         bb = *y;
       };

     if (x.q) {                         /* If this is a demotion... */
       *y = x;                          /* Insert demoted node here */
       x.p->a = aa;                     /* Attach remaining subtrees */
       x.p->b = bb;
       return (y);                      /* Supply the new attachment */
     };

     if (aa.q)                          /* Attach remaining subtree */
       *y = aa;
     else
       *y = bb;

     return (NULL);                     /* And yield null pointer */

   };                                   /* (End of subroutine) */


   #pragma page ()




    /*            __________________________________
                 |                                  |
                 |  __do_disclaim -- Internal subr  |
                 |__________________________________|


	  Releases page space taken by the freed blocks.

          Definition is:
	       void __do_disclaim(adlen *)

          where:
               addlen = pointer to address and len of the
	                root node of the tree to descend.

	  Implementation:

	  Recursively descend down the tree while block
	  size is equal to or greater than PAGESIZE.
	  Disclaim pages that lie in the blocks' address
	  range.

	  For efficiency, we should not call disclaim()
	  on any part of the address space that does
	  not overlap a whole page because the disclaim()
	  system call will just zero out the memory (taking
	  cycles). The formula used to determine start and
	  length of the address range to disclaim:

          ptr  = address of memory block being freed
          size = size of memory block being freed
    
          start  = start of addr to be disclaimed (multiple of 4k)
                 = (ptr + PAGESIZE - 1) & ~(PAGESIZE - 1);
    
          length = length of memory to disclaim (multiple of 4k)
                 = (ptr + size - start) & ~(PAGESIZE - 1);        */


#include <sys/shm.h>
#include <sys/param.h>

static void __do_disclaim(adlen *adl)
{
    node *p = adl->p;
    uint  q = adl->q;

    if (p == NULL)
	return;

    if (p->a.q >= PAGESIZE + sizeof(node))
	__do_disclaim(&(p->a));

    if (q >= PAGESIZE + sizeof(node))
    {
	char *ptr = (char *) p + sizeof(node);
	uint size = q - sizeof(node);
	char *start;
	uint len;

	start = ((uint)ptr + PAGESIZE - 1) & ~(PAGESIZE - 1);
	len = (ptr + size - start) & ~(PAGESIZE - 1);

	if (len >= PAGESIZE)
	    disclaim(start, len, ZERO_MEM);
    }

    if (p->b.q >= PAGESIZE + sizeof(node))
	__do_disclaim(&(p->b));
}


void disclaim_free_y(void)
{
    freelog[freeze].a.p = NULL;	/* Purge the Free Log */
    __do_disclaim(&manchor);	/* disclaim any free space */
}

   #pragma page ()






     /*             _________________________________
                   |                                 |
                   |            THE  END             |
                   |_________________________________|             */

