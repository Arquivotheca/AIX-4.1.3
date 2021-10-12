/* @(#)65	1.41  src/bos/kernel/vmm/POWER/vmscb.h, sysvmm, bos412, 9445C412a 11/4/94 12:18:11 */
#ifndef _h_VMSCB
#define _h_VMSCB

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "mplock.h"

/*
 * Segment Control Block (SCB) declaration. scbs reside in vmmdseg.
 */

struct scb
{
        union
        {
                uint    _sibits;        /* segment info bits */
                struct
                {
                        unsigned  _defkey :2;   /* default storage key */
                        unsigned  _valid :1;    /* segment is valid */
			unsigned  _intrseg :1;  /* interruptable segment */
                        unsigned  _delpend  :1; /* delete pending  */
                        unsigned  _delete :1;   /* delete in progress */
                        unsigned  _iodelete :1; /* delete when i/o done */
			unsigned  _privseg  :1; /* process private segment */
                        unsigned  _wseg  :1;    /* working storage */
                        unsigned  _clseg :1;    /* client segment */
                        unsigned  _pseg  :1;    /* persistent storage */
                        unsigned  _jseg  :1;    /* journalled */
                        unsigned  _logseg:1;    /* used as log */
                        unsigned  _defseg :1;   /* deferred update */
                        unsigned  _system :1;   /* system segment  */
			unsigned  _ptaseg :1;	/* pta segment   */
                        unsigned  _combit  :1;  /* commit in progress */
                        unsigned  _chgbit  :1;  /* segment modified */
                        unsigned  _inoseg  :1;  /* .inodes segment */
                        unsigned  _compseg :1;  /* computational - ws + text */
                        unsigned  _mseg    :1;  /* mapping segment */
                        unsigned  _cow     :1;  /* mmap copy-on-write */
                        unsigned  _eio     :1;  /* I/O write error occurred */
                        unsigned  _compress :1; /* data compression */
			unsigned  _shrlib :1;   /* shared library segment */
			unsigned  _psearlyalloc :1; /* paging space reserved */
			unsigned  _sparse :1;   /* sparse segment */   
			unsigned  _indseg   :1;	 /* .indirect segment */
			unsigned  _dmapseg   :1; /* diskmap segment */
			unsigned  _inactive  :1; /* segment is inactive */
			unsigned  _unused :2;
                } s1;
        } u1;
	union
	{
		int	_free;		/* next free/delete pending */
		int	_refcnt;	/* mmap reference count */
	} uf;
	short		_npopages;	/* non-fblru pageout count	*/
	short		_xmemcnt;	/* xmem attach count            */
       	uint    	_vxpto;         /* address of XPT root          */
        int             _npages;        /* number of pages in real memory   */
        int             _sidlist;       /* page frame at head of list       */
        int             _maxvpn;        /* max page <= MAXVPN/2 or max page */
        union
        {
            struct working_info
            {
                int             _parent;        /* parent sid     */
                int             _left;          /* left child sid */
                int             _right;         /* right child sid */
                int             _minvpn;        /* smallest pno > MAXVPN/2 */
                int             _sysbr;         /* last page user region    */
                int             _uplim;         /* [0,uplim] legal */
                int             _downlim;       /* [downlim,MAXVPN] legal  */
		int		_npsblks;	/* number of disk blocks w.s.*/
            } ws;

            struct persistent_info
            {
                uint            _devid;         /* index in pdt        */
                int             _iolev;         /* I/O level            */
		struct thread * _iowait;	/* I/O level wait list anch */
		void	      * _gnptr;		/* if not log, gnode ptr   */	
                int             _newdisk;       /* # of new disk blocks     */
                int             _logsidx;       /* if not log, log's sidx */
		short		_agsize;	/* disk block alloc grp size */
		short		_iagsize;	/* disk inode alloc grp size */
                int             _serial;        /* segment serial number */
            } ps;

            struct client_info
            {
                uint            _devid;         /* index in PDT		*/
                int             _iolev;         /* I/O level            */
		struct thread * _iowait;	/* I/O level wait list anch */
		void	      * _gnptr;		/* pointer to gnode      */	
		struct thread * _delwait;	/* iodelete waitor       */
		struct thread * _waitlist;	/* wait for change key */
                int             _spare;         /* spare		*/
                int             _serial;        /* segment serial number */
            } cl;

            struct log_info
            {
                uint            _devid;         /* index in pdt         */
                int             _iolev;         /* io level             */
		struct thread * _iowait;	/* I/O level wait list anch */
                int             _logsize;       /* size in bytes of log */
                int             _logcur;        /* smallest log diff so far */
                int             _logsync;       /* oldest logval         */
                int             _loglast;       /* last logval v_setlog */
            } log;

            struct mapping_info
            {
                uint            _start;         /* start addr of 1st ame */
                void          * _ame;           /* pointer to 1st ame    */
            } map;

        } u2;

        union
        {
	    int     _npseablks;                 /* allocated unassigned ps blks*/ 
            uint    _pageahead;        		/* pageahead info */
	    uint    _sibling;			/* sibling mmap fork segment */
            struct pgahead_info
            {
		unsigned	_npgahead  :8;	/* number of pages to prepage */
		unsigned	_lstpagex  :24; /* last hidden or faulted on  */
             } pa;

        } u3;

#ifdef _VMM_MP_EFF
	/* MUST BE THE LAST FIELD OF THE STRUCTURE! (Cf bzero in v_create)
	 */
	Simple_lock	_lock;	/* per scb lock, the workhorse of lccks */
#endif /* _VMM_MP_EFF */
};

/* macros for accessing variables in scb.           */
#define scb_sibits(x)   vmmdseg.scb[(x)].u1._sibits
#define scb_valid(x)    vmmdseg.scb[(x)].u1.s1._valid
#define scb_delpend(x)  vmmdseg.scb[(x)].u1.s1._delpend
#define scb_intrseg(x)  vmmdseg.scb[(x)].u1.s1._intrseg
#define scb_iodelete(x) vmmdseg.scb[(x)].u1.s1._iodelete
#define scb_defkey(x)   vmmdseg.scb[(x)].u1.s1._defkey
#define scb_privseg(x)  vmmdseg.scb[(x)].u1.s1._privseg
#define scb_wseg(x)     vmmdseg.scb[(x)].u1.s1._wseg
#define scb_clseg(x)    vmmdseg.scb[(x)].u1.s1._clseg
#define scb_pseg(x)     vmmdseg.scb[(x)].u1.s1._pseg
#define scb_jseg(x)     vmmdseg.scb[(x)].u1.s1._jseg
#define scb_logseg(x)   vmmdseg.scb[(x)].u1.s1._logseg
#define scb_defseg(x)   vmmdseg.scb[(x)].u1.s1._defseg
#define scb_system(x)   vmmdseg.scb[(x)].u1.s1._system
#define scb_ptaseg(x)   vmmdseg.scb[(x)].u1.s1._ptaseg
#define scb_combit(x)   vmmdseg.scb[(x)].u1.s1._combit
#define scb_chgbit(x)   vmmdseg.scb[(x)].u1.s1._chgbit
#define scb_inoseg(x)   vmmdseg.scb[(x)].u1.s1._inoseg
#define scb_compseg(x)  vmmdseg.scb[(x)].u1.s1._compseg
#define scb_delete(x)   vmmdseg.scb[(x)].u1.s1._delete
#define scb_mseg(x)     vmmdseg.scb[(x)].u1.s1._mseg
#define scb_cow(x)      vmmdseg.scb[(x)].u1.s1._cow
#define scb_eio(x)      vmmdseg.scb[(x)].u1.s1._eio
#define scb_compress(x) vmmdseg.scb[(x)].u1.s1._compress
#define scb_shrlib(x)	vmmdseg.scb[(x)].u1.s1._shrlib
#define scb_psearlyalloc(x) vmmdseg.scb[(x)].u1.s1._psearlyalloc
#define scb_indseg(x)	vmmdseg.scb[(x)].u1.s1._indseg
#define scb_dmapseg(x)	vmmdseg.scb[(x)].u1.s1._dmapseg
#define scb_sparse(x)	vmmdseg.scb[(x)].u1.s1._sparse
#define scb_inactive(x)	vmmdseg.scb[(x)].u1.s1._inactive
#define scb_free(x)     vmmdseg.scb[(x)].uf._free
#define scb_refcnt(x)   vmmdseg.scb[(x)].uf._refcnt
#define scb_npopages(x) vmmdseg.scb[(x)]._npopages
#define scb_xmemcnt(x)  vmmdseg.scb[(x)]._xmemcnt
#define scb_vxpto(x)    vmmdseg.scb[(x)]._vxpto
#define scb_npages(x)   vmmdseg.scb[(x)]._npages
#define scb_sidlist(x)  vmmdseg.scb[(x)]._sidlist
#define scb_maxvpn(x)   vmmdseg.scb[(x)]._maxvpn
#define scb_devid(x)    vmmdseg.scb[(x)].u2.ps._devid
#define scb_iolev(x)    vmmdseg.scb[(x)].u2.ps._iolev
#define scb_iowait(x)   vmmdseg.scb[(x)].u2.ps._iowait
#define scb_gnptr(x)    vmmdseg.scb[(x)].u2.ps._gnptr
#define scb_logsidx(x)  vmmdseg.scb[(x)].u2.ps._logsidx
#define scb_agsize(x)   vmmdseg.scb[(x)].u2.ps._agsize
#define scb_iagsize(x)  vmmdseg.scb[(x)].u2.ps._iagsize
#define scb_newdisk(x)  vmmdseg.scb[(x)].u2.ps._newdisk
#define scb_serial(x)   vmmdseg.scb[(x)].u2.ps._serial
#define scb_parent(x)   vmmdseg.scb[(x)].u2.ws._parent
#define scb_left(x)     vmmdseg.scb[(x)].u2.ws._left
#define scb_right(x)    vmmdseg.scb[(x)].u2.ws._right
#define scb_minvpn(x)   vmmdseg.scb[(x)].u2.ws._minvpn
#define scb_uplim(x)    vmmdseg.scb[(x)].u2.ws._uplim
#define scb_downlim(x)  vmmdseg.scb[(x)].u2.ws._downlim
#define scb_sysbr(x)    vmmdseg.scb[(x)].u2.ws._sysbr
#define scb_npsblks(x)  vmmdseg.scb[(x)].u2.ws._npsblks
#define scb_delwait(x)  vmmdseg.scb[(x)].u2.cl._delwait
#define scb_waitlist(x) vmmdseg.scb[(x)].u2.cl._waitlist
#define scb_logsize(x)  vmmdseg.scb[(x)].u2.log._logsize
#define scb_logcur(x)   vmmdseg.scb[(x)].u2.log._logcur
#define scb_logsync(x)  vmmdseg.scb[(x)].u2.log._logsync
#define scb_loglast(x)  vmmdseg.scb[(x)].u2.log._loglast
#define scb_start(x)    vmmdseg.scb[(x)].u2.map._start
#define scb_ame(x)      vmmdseg.scb[(x)].u2.map._ame
#define scb_npseablks(x) vmmdseg.scb[(x)].u3._npseablks
#define scb_sibling(x)  vmmdseg.scb[(x)].u3._sibling
#define scb_npgahead(x) vmmdseg.scb[(x)].u3.pa._npgahead
#define scb_lstpagex(x) vmmdseg.scb[(x)].u3.pa._lstpagex
#ifdef _VMM_MP_EFF
#define scb_lock(x)	vmmdseg.scb[(x)]._lock
#endif /* _VMM_MP_EFF */

/*
 * Address Map Entry (AME) declaration. ames reside in vmmdseg.
 *
 * NOTE: this definition merely reserves enough space for
 *	 each ame in vmmdseg. the real definition of an
 *	 address map and map entry is in vm_map.h --
 *	 the size of these structures cannot exceed that defined here.
 */

struct ame
{
	int _word[16];	/* define space for 64-bytes */
};

/* macros for accessing variables in ame.           */
/* ame free list chained through 1st word in ame    */

#define ame_free(x)     *( (int *)&vmmdseg.ame[(x)] )

#endif /* _h_VMSCB */
