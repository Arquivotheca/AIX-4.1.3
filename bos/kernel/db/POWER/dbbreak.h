/* @(#)49	1.17	src/bos/kernel/db/POWER/dbbreak.h, sysdb, bos411, 9428A410j 12/13/93 11:54:25 */
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#define LASTSTEP	8		/* max step points */
#define LASTSTOP        32		/* max break points */
#define BRKPT		1
#define TRACEPT		2
#define LOOP		3
#define CLEARB		'b'
#define CLEART		't'
#define QUIT_PARM	"dump"
#define TLB_PARM	"all"
#define ASTRISK		'*'
#define SEGNUM(x)	(0xf0000000 & x)
#define FROMDEBVARS -1       /* srval is to get in debvars for is_break and is_step */

/* flag values */
#define PAGED_OUT	1
#define CANNOT_CLEAR	2

#ifdef  _POWER

/* watchpoint flags */
#define LOAD            0
#define STORE           1
#define BOTH            2
#define WP_ON           1
#define WP_OFF          0
#define BP_ON           1
#define BP_OFF          0

#define BC		"BC"
#define BREAKTRAP	 0x7c800008
#define STATIC_BREAK_TRAP 0x7c810808
#define NULLC		0x00000000
#define segid0		0x00000000		/* for xlate off */
#define segid_mask	0x00ffffff		/* mask for seg id's */
#endif /* _POWER */


#define BAL		"BAL"


struct steptab {
	ulong	stepaddr1[LASTSTEP];	/*step addr 1	*/
	ulong	stepaddr2[LASTSTEP];	/*step addr 2	*/
	ulong	stepsreg1[LASTSTEP];	/* seg reg 1	*/
	ulong 	stepsreg2[LASTSTEP];	/* seg reg 2	*/
#ifdef _POWER
	ulong	stepsave1[LASTSTEP];	/* save area	*/
	ulong	stepsave2[LASTSTEP];	/* save area 2	*/
#endif /* _POWER */
#ifdef _POWER_MP
	ulong  step_local_mst_addr[LASTSTEP];     /* local mst associated to this breakpoint */
	ulong  step_local_mst_sregval[LASTSTEP];  /* segment register to access this mst */
#endif /* POWER_MP */
	int	nnbreaks;	/* # of breakpts	*/
	ulong	sstop_addr[LASTSTOP];	/* slot 0 nevere used	*/
	ulong	sstop_segid[LASTSTOP];  /* segement ids	*/
#ifdef _POWER
	ulong	sstop_save[LASTSTOP];	/* breal locn contents	*/
#endif /* _POWER */
	uchar	sstop_type[LASTSTOP];	/* type: break or trace	*/
	uchar	stepinuse[LASTSTEP];
	uchar	stepgo[LASTSTEP];
	uchar	flags[LASTSTOP];	/* pending flags */
        uchar   watch_brat_step[LASTSTEP];   /* step for watchpoint */
#ifdef _POWER_MP
	ulong  stop_local_mst_addr[LASTSTOP];     /* local mst associated to this breakpoint */
					     /* 0 if breakpoint is not local */
	ulong  stop_local_mst_sregval[LASTSTOP];  /* segment register to access this mst */
#endif /* POWER_MP */
	} ;

struct watch_data {
        caddr_t addr;                   /*watchpoint address            */
        ulong   segid;                  /*watchpoint segid              */
        uchar   wtype;                  /*watchpoint type,load,store,both*/
        uchar   active;                 /*watchpoint on/off flag        */
        };

struct brat_data {
        caddr_t addr;                   /*bratpoint address             */
        ulong   segid;                  /*bratpoint segid               */
        uchar   active;                 /*brathpoint on/off flag        */
        };

extern int loop_count;
char 	errst[80];
#define step1(x)	stepstoptable.stepaddr1[(x)]
#define step2(x)	stepstoptable.stepaddr2[(x)]
#define step_seg1(x)	stepstoptable.stepsreg1[(x)]
#define step_seg2(x)	stepstoptable.stepsreg2[(x)]
#define step_save1(x)	stepstoptable.stepsave1[(x)]
#define step_save2(x)	stepstoptable.stepsave2[(x)]
#define nbreaks		stepstoptable.nnbreaks
#define stop_addr(x)	stepstoptable.sstop_addr[(x)]
#define stop_segid(x) 	stepstoptable.sstop_segid[(x)]
#define stop_save(x) 	stepstoptable.sstop_save[(x)]
#define stop_type(x)	stepstoptable.sstop_type[(x)]
#define step_i(x)	stepstoptable.stepinuse[(x)]
#define step_wb(x)       stepstoptable.watch_brat_step[(x)]
#define step_continue(x) stepstoptable.stepgo[(x)]
#define setstep_continue(x)  stepstoptable.stepgo |= (0x80 >> ((x) - 1)) 
#define unsetstep_continue(x)  stepstoptable.stepgo &= ~(0x80 >> ((x) - 1))
#define setstep_i(x)	stepstoptable.stepinuse |= (0x80 >> ((x) - 1))
#define unsetstep_i(x)	stepstoptable.stepinuse &= ~(0x80 >> ((x) - 1))
#define stop_flags(x)	stepstoptable.flags[(x)]
#define pending(x)	stepstoptable.flags[(x)] & PENDING_REMOVAL
#ifdef _POWER_MP
#define step_local_mst_addr(x)    stepstoptable.step_local_mst_addr[(x)]
#define step_local_mst_sregval(x) stepstoptable.step_local_mst_sregval[(x)]
#define stop_local_mst_addr(x)    stepstoptable.stop_local_mst_addr[(x)]
#define stop_local_mst_sregval(x) stepstoptable.stop_local_mst_sregval[(x)]
#define is_local_break(x) (stepstoptable.stop_local_mst_addr[(x)] != 0)
#endif /* POWER_MP */
