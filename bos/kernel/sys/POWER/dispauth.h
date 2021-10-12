/* @(#)46        1.3  src/bos/kernel/sys/POWER/dispauth.h, sysios, bos411, 9428A410j 5/5/94 19:20:03 */
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef  _H_DISPAUTH
#define  _H_DISPAUTH

#include <sys/types.h>
#include <sys/adspace.h>

#ifdef _KERNSYS
#include <macros.h>
#endif

#ifdef _KERNEL
typedef  struct gruprt  gruprt_t;
typedef  struct busprt	busprt_t;
typedef  union  acws    acws_t; 

typedef union _ioRange {
    ulong       range;      /* both in one word */
    struct {
        ushort   lo;         /* base */
        ushort   hi;         /* limit */
    } lohi;
} ioRange;


union acws
{
	struct
	{	
		ulong	iocc_sr;
		ulong	csr15;
		ioRange	limit;
	}       pwr_iocc_ac;    /* ac words for _BUSPRT_CSR15 type            */

	ulong	mc_sr_ac;       /* ac word for _BUSPRT_SR_MC type             */

	ulong	procbus_sr_ac;   /* ac word for _BUSPRT_PBUS_XXX type         */

	struct
	{	
		ulong	xid_sr; /* must be the first field in structure       */
		ulong	ear;
	}       xid_7f_ac;      /* ac word for _BUSPRT_7F_XID type            */ 

	ulong   sr_ac;          /* generic name segment register field        */

	struct
	{
		ulong   batu;
		ulong   batl;
		ulong   ear;
	}       bat_xid_ac;     /* ac word for _BUSPRT_BAT_XID type           */

	struct
	{	
		ulong	batu; 
		ulong	batl;
	}       bat_ac;         /* ac word for _BUSPRT_BAT type               */
};


struct gruprt
{
	gruprt_t   *gp_next;	/* pointer to the next group 		      */
	busprt_t   *gp_busprt;	/* pointer to the first busprt structure      *
				 * in this group                              */
	void 	   *gp_owner;	/* pointer to owning thread structure         */
	char	    gp_type;	/* access control type                        */
	char        gp_primxid; /* primary xid display                        */
	ushort      gp_own;     /* number of busprt owned in this group       */
	ulong       gp_lock;    /* simple lock, for SMPs                      */
	ulong       gp_resv;	/* reserved for 64 bit processors             */
	ulong       gp_eaddr;   /* 32 bit or 64 bit effective address         */
	uint        gp_nbat;    /* number of BAT type gruprts in list         */
	acws_t      gp_acws;    /* shadow copy of hardware access control reg */
};

#define GRUPRT_NBAT(gruprtp)         ((gruprtp)->gp_nbat)
#define GRUPRT_PRIMXID(gruprtp)	     ((gruprtp)->gp_primxid)
#define GRUPRT_IOCC_SR(gruprtp)	     ((gruprtp)->gp_acws.pwr_iocc_ac.iocc_sr)
#define GRUPRT_CSR15(gruprtp)	     ((gruprtp)->gp_acws.pwr_iocc_ac.csr15)
#define GRUPRT_LIMIT(gruprtp)	     ((gruprtp)->gp_acws.pwr_iocc_ac.limit).range
#define GRUPRT_HILMT(gruprtp)        \
	((gruprtp)->gp_acws.pwr_iocc_ac.limit).lohi.hi
#define GRUPRT_LOLMT(gruprtp)	     \
	((gruprtp)->gp_acws.pwr_iocc_ac.limit).lohi.lo
#define GRUPRT_MC_SR(gruprtp)	     ((gruprtp)->gp_acws.mc_sr_ac)
#define GRUPRT_PBUS_SR(gruprtp)	     ((gruprtp)->gp_acws.procbus_sr_ac)
#define GRUPRT_7F_XID_SR(gruprtp)    ((gruprtp)->gp_acws.xid_7f_ac.xid_sr)
#define GRUPRT_SR(gruprtp)	     ((gruprtp)->gp_acws.sr_ac)
#define GRUPRT_7F_XID_EAR(gruprtp)   ((gruprtp)->gp_acws.xid_7f_ac.ear)
#define GRUPRT_BAT_XID_BATU(gruprtp) ((gruprtp)->gp_acws.bat_xid_ac.batu)
#define GRUPRT_BAT_XID_BATL(gruprtp) ((gruprtp)->gp_acws.bat_xid_ac.batl)
#define GRUPRT_BAT_XID_EAR(gruprtp)  ((gruprtp)->gp_acws.bat_xid_ac.ear)
#define GRUPRT_BAT_BATU(gruprtp)     ((gruprtp)->gp_acws.bat_ac.batu)
#define GRUPRT_BAT_BATL(gruprtp)     ((gruprtp)->gp_acws.bat_ac.batl)

	
struct busprt
{
	gruprt_t   *bp_grp;     /* pointer to the gruprt structure            */ 
	busprt_t   *bp_next;  	/* pointer to next busprt in the same gruprt  */
	char	    bp_type;	/* access control type                        */
	char 	    bp_set;	/* owner of the display iff set == type       */
	ushort 	    bp_pad;	/* padding                                    */
	ulong       bp_resv;	/* reserved for 64 bit processors             */
	ulong       bp_eaddr;   /* 32 bit or 64 bit effective address         */
	acws_t      bp_acws;    /* access control words for this domain       */
};


#define BUSPRT_TYPE(busprtp)	     ((busprtp)->bp_type)
#define BUSPRT_EADDR(busprtp)        ((busprtp)->bp_eaddr)

#define BUSPRT_IOCC_SR(busprtp)	     ((busprtp)->bp_acws.pwr_iocc_ac.iocc_sr)
#define BUSPRT_CSR15(busprtp)	     ((busprtp)->bp_acws.pwr_iocc_ac.csr15)
#define BUSPRT_LIMIT(busprtp)	     ((busprtp)->bp_acws.pwr_iocc_ac.limit).range
#define BUSPRT_HILMT(busprtp)	     \
	((busprtp)->bp_acws.pwr_iocc_ac.limit).lohi.hi
#define BUSPRT_LOLMT(busprtp)	     \
	((busprtp)->bp_acws.pwr_iocc_ac.limit).lohi.lo
#define BUSPRT_MC_SR(busprtp)	     ((busprtp)->bp_acws.mc_sr_ac)
#define BUSPRT_PBUS_SR(busprtp)	     ((busprtp)->bp_acws.procbus_sr_ac)
#define BUSPRT_7F_XID_SR(busprtp)    ((busprtp)->bp_acws.xid_7f_ac.xid_sr)
#define BUSPRT_SR(busprtp)	     ((busprtp)->bp_acws.sr_ac)
#define BUSPRT_7F_XID_EAR(busprtp)   ((busprtp)->bp_acws.xid_7f_ac.ear)
#define BUSPRT_BAT_XID_BATU(busprtp) ((busprtp)->bp_acws.bat_xid_ac.batu)
#define BUSPRT_BAT_XID_BATL(busprtp) ((busprtp)->bp_acws.bat_xid_ac.batl)
#define BUSPRT_BAT_XID_EAR(busprtp)  ((busprtp)->bp_acws.bat_xid_ac.ear)
#define BUSPRT_BAT_BATU(busprtp)     ((busprtp)->bp_acws.bat_ac.batu)
#define BUSPRT_BAT_BATL(busprtp)     ((busprtp)->bp_acws.bat_ac.batl)


/* possible values for type and set field in busprt and gruprt structure
 */
#define _BUSPRT_NO_ACCESS	     0000  /* no access                       */
#define _BUSPRT_CSR15		     0010  /* Power Micro Channel display     */
#define _BUSPRT_SR_MC		     0020  /* PPC Micro Channel display       */
#define _BUSPRT_PBUS_RSC	     0030  /* processor bus display in RSC    */
#define _BUSPRT_PBUS_RS2G	     0031  /* processor bus display in RS2G   */
#define _BUSPRT_PBUS_601             0032  /* 7F processor bus display in 601 */
#define _BUSPRT_7F_XID		     0033  /* 7F+XID processor bus display in 
					      601                             */
#define _BUSPRT_BAT		     0040  /* BAT only type display           */
#define _BUSPRT_BAT_XID              0041  /* BAT+XID type display            */

#define _BUSPRT_MASK		     0370  /* display type mask               */
#define _BUSPRT_PBUS_TYPE	     0030  /* processor bus type display      */
#define _BUSPRT_BAT_TYPE             0040  /* BAT type display                */

/* The following two defines need to be cleaned up in the future (coreq with
   a RCM defect)
 */  
#define _BUSPRT_PBUS_60X	     _BUSPRT_PBUS_601
#define _BUSPRT_XID                  _BUSPRT_7F_XID


/* Segment register values that ensure exception when accessing display.
 * Note that SID 0x7FFFFE is reserved in vmdefs.h.
 * Any changes here should be reflected in vmdefs.h as well.
 */  
#define INV_PB_SEGREG_PWR     0x407FFFFE   /* T = 0, K = 1, S = 0  
					      SID = 0x7FFFFE                  */
#define INV_PB_SEGREG_PPC     0x607FFFFE   /* T = 0, Ks = Kp = 1,
					      SID = 0x7FFFFE 	              */
#define INV_MC_SEGREG         0xE2000000   /* T = 1, Ks = Kp = 1,
					      BUID = 0x20  	              */
#define CSR15_AUTHMASK        0x0000FF00   /* mask for authority bits
					      in Power CSR15                  */ 
#define PPC_SR_AUTHMASK	      0x00007F00   /* mask for authority bits
				       	      in Power PC segment reg	      */
#define EAR_DISABLE_MASK      0x7FFFFFFF   /* enable bit = 0                  */
#define EAR_ENABLE_MASK       0x80000000   /* enable bit = 1                  */

#define BAT_DISABLE_MASK      0xFFFFFFFE   /* Vp = 0                          */

#define INVBUSLIMT	      0xFFFF0000   /* low limit > high limit  	      */
#define INVCSR15              0x000F00FF   /* buffer = 0xF, authmask = 0      */

#define CSR15                 0x004F0060   /* offset of CSR15 register        */
#define LIMIT_REG             0x00400040   /* offset of limit register        */



/* macro to initialize csr15 and limit register
 */
#define INIT_CSR15_LMT(gruprtp)						     \
{									     \
	register ulong   ioccaddr;       /* IO handle            */	     \
        volatile ulong   *csr15p;        /* CSR 15               */          \
        volatile ulong   *limitp;        /* IOCC limits register */          \
		                                                             \
	ioccaddr = (ulong)io_att(GRUPRT_IOCC_SR(gruprtp), 0); 		     \
	csr15p = (ulong *)(ioccaddr + CSR15);                                \
        limitp = (ulong *)(ioccaddr + LIMIT_REG);                            \
	*csr15p = INVCSR15;						     \
	*limitp = INVBUSLIMT;						     \
	io_det(ioccaddr);						     \
}


/* macro to add busio range to the access control words of a _BUSPRT_CSR15 type
 * gruprt structure
 */  
#define ADD_RANGE(gp,bp)                                                     \
{                                                                            \
	GRUPRT_LOLMT(gp) = min(GRUPRT_LOLMT(gp), BUSPRT_LOLMT(bp));          \
                                                                             \
	GRUPRT_HILMT(gp) = max(GRUPRT_HILMT(gp), BUSPRT_HILMT(bp));          \
}


/* macro to subtract busio range from the access control words of a _BUSPRT_CSR15
 * type gruprt structure
 */  
#define SUB_RANGE(gp,bp)                                                     \
{                                                                            \
	if (GRUPRT_HILMT(gp) == BUSPRT_HILMT(bp))                            \
	{                                                                    \
            GRUPRT_HILMT(gp) =  BUSPRT_LOLMT(bp) - 1;                        \
        }                                                                    \
	else {                                                               \
           if (GRUPRT_LOLMT(gp) == BUSPRT_LOLMT(bp))                         \
	   {    	                                                     \
               GRUPRT_LOLMT(gp) = BUSPRT_HILMT(bp) + 1;                      \
	   }                                                                 \
        }                                                                    \
                                                                             \
	if (GRUPRT_HILMT(gp) < 	GRUPRT_LOLMT(gp))                            \
	{                                                                    \
	    GRUPRT_LIMIT(gp) = INVBUSLIMT;                                   \
	}                                                                    \
}		


/* macro to add display access authority to the access control words of
 * a gruprt structure
 */  
#define SET_ACWS(gp,bp)							     \
{									     \
	switch(bp->bp_type)                                                  \
	{                                                                    \
		case _BUSPRT_CSR15:                                          \
		GRUPRT_CSR15(gp) |= BUSPRT_CSR15(bp);                        \
		ADD_RANGE(gp,bp);                                            \
		break;                                                       \
			                                                     \
	        case _BUSPRT_SR_MC:                                          \
		if (GRUPRT_MC_SR(gp) == INV_MC_SEGREG)                       \
		{                                                            \
	            GRUPRT_MC_SR(gp) = BUSPRT_MC_SR(bp);                     \
	        }                                                            \
		else {                                                       \
		    GRUPRT_MC_SR(gp) |= BUSPRT_MC_SR(bp);		     \
		}	                                                     \
		break;                                                       \
		                                                             \
                case _BUSPRT_PBUS_RSC:                                       \
		case _BUSPRT_PBUS_601:                                       \
		GRUPRT_PBUS_SR(gp) = BUSPRT_PBUS_SR(bp);                     \
		break;                                                       \
			                                                     \
		case _BUSPRT_7F_XID:                                         \
		GRUPRT_7F_XID_SR(gp) = BUSPRT_7F_XID_SR(bp);                 \
		GRUPRT_7F_XID_EAR(gp) = BUSPRT_7F_XID_EAR(bp);               \
		break;                                                       \
									     \
		case _BUSPRT_BAT:                                            \
		GRUPRT_BAT_BATU(gp) = BUSPRT_BAT_BATU(bp);                   \
		GRUPRT_BAT_BATL(gp) = BUSPRT_BAT_BATL(bp);                   \
		break;                                                       \
									     \
		case _BUSPRT_BAT_XID:                                        \
		GRUPRT_BAT_XID_BATU(gp) = BUSPRT_BAT_XID_BATU(bp);           \
		GRUPRT_BAT_XID_BATL(gp) = BUSPRT_BAT_XID_BATL(bp);           \
		GRUPRT_BAT_XID_EAR(gp)  = BUSPRT_BAT_XID_EAR(bp);            \
		break;                                                       \
									     \
                default: assert(0);                                          \
	}                                                                    \
}


/* macro to remove display access authority from the access control words of
 * a gruprt structure
 */  
#define RESET_ACWS(gp,bp)                                                    \
{                                                                            \
	switch(bp->bp_type)                                                  \
	{                                                                    \
		case _BUSPRT_CSR15:                                          \
		GRUPRT_CSR15(gp) &= ~(BUSPRT_CSR15(bp) & CSR15_AUTHMASK);    \
		SUB_RANGE(gp,bp);                                            \
		break;                                                       \
			                                                     \
	        case _BUSPRT_SR_MC:                                          \
		if (GRUPRT_MC_SR(gp) == BUSPRT_MC_SR(bp))                    \
		{                                                            \
	            GRUPRT_MC_SR(gp) =  INV_MC_SEGREG;                       \
	        }                                                            \
		else {                                                       \
		    GRUPRT_MC_SR(gp) &=                                      \
			 ~(BUSPRT_MC_SR(bp) & PPC_SR_AUTHMASK);              \
		}	                                                     \
		break;                                                       \
		                                                             \
                case _BUSPRT_PBUS_RSC:                                       \
		GRUPRT_PBUS_SR(gp) = INV_PB_SEGREG_PWR;                      \
		break;                                                       \
                                                                             \
		case _BUSPRT_PBUS_601:                                       \
		GRUPRT_PBUS_SR(gp) = INV_PB_SEGREG_PPC;                      \
		break;                                                       \
			                                                     \
		case _BUSPRT_7F_XID:                                         \
		GRUPRT_7F_XID_SR(gp) = INV_PB_SEGREG_PPC;                    \
		GRUPRT_7F_XID_EAR(gp) &= EAR_DISABLE_MASK;                   \
		break;                                                       \
			                                                     \
		case _BUSPRT_BAT:                                            \
                GRUPRT_BAT_BATU(gp) &= BAT_DISABLE_MASK;                     \
                break;                                                       \
									     \
		case _BUSPRT_BAT_XID:                                        \
		GRUPRT_BAT_XID_BATU(gp) &= BAT_DISABLE_MASK;		     \
		GRUPRT_BAT_XID_EAR(gp) &= EAR_DISABLE_MASK;                  \
		break;                                                       \
			                                                     \
		default: assert(0);                                          \
	}                                                                    \
}	


#ifdef _NO_PROTO
extern int reg_display_acc();
extern void unreg_display_acc();
extern void grant_display_owner();
extern int revoke_display_owner();
extern void setup_display_acc();
extern void *u_iomem_att();
extern void u_iomem_det();
#else /* _NO_PROTO */
extern int reg_display_acc(struct busprt *busprtp);
extern void unreg_display_acc(struct busprt *busprtp);
extern void grant_display_owner(struct busprt *busprtp);
extern int revoke_display_owner(struct busprt *busprtp);
extern void setup_display_acc(struct gruprt *gp);
extern void *u_iomem_att(struct io_map *iop);
extern void u_iomem_det(void *ioaddr);
#endif /* _NO_PROTO */
#else  /* _KERNEL */
#ifdef _NO_PROTO
extern int _set_primxid();
#else /* _NO_PROTO */
extern int _set_primxid(uint ear);
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif /* _H_DISPAUTH */







