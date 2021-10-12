static char sccsid[] = "@(#)80	1.28  src/bos/kernel/db/POWER/vdbmemop.c, sysdb, bos411, 9439C411a 9/29/94 17:45:15";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:	get_from_memory, write_to_memory, get_put_aligned, rw_virt,
 *		get_put_data, pre_get_put_data
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/param.h>
#include <sys/seg.h>
#include <sys/systemcfg.h>
#include <sys/machine.h>
#include <sys/inline.h>
#include "debvars.h"			/* Debugger variables		*/

#define SREG(x)  (((ulong)x)>>SEGSHIFT)	/* Segment reg. #		*/
#define SEG(x)   (SRTOSID(debvars[IDSEGS+SREG(x)].hv)) /* seg. id.	*/


#define IS_IOSPACE(x) ((x) & 0x80000000)
#define KEY_VALUE	0x60000000	/* shift for segment access key */

#define SCRATCH_SEG 7


/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern	int debug_xlate();		/* Check address validity	*/
extern  int lqra();			/* Query real address		*/
extern	int in_real_mem();		/* Ckeck real addr validity	*/


/*
 * NAME: get_from_memory
 *
 * FUNCTION:
 *	Fetch data from system memory.
 *	Fetch the number of bytes asked for.
 *	Return FALSE if the data isn't present.
 *
 * EXECUTION ENVIRONMENT:
 *      This executes in the debugger environment.
 *
 * RETURN VALUE DESCRIPTION: TRUE - The operation worked.
 *			     FALSE-   it failed.
 */
int get_from_memory(adr,virt,area,len)
ulong adr;
int virt,len;
caddr_t area;
{
	return( get_put_data(adr,virt,area,len,FALSE) );
}


/*
 * NAME: write_to_memory
 *
 * FUNCTION:
 *	Write the data in area to system memory.
 *	Write the number of bytes indicated.
 *	Return FALSE if the memory isn't mapped to real.
 *
 * EXECUTION ENVIRONMENT:
 *      This executes in the debugger environment.
 *
 * RETURN VALUE DESCRIPTION: TRUE - The operation worked.
 *			     FALSE-   it failed.
 */
int write_to_memory(adr,virt,area,len)
ulong adr;
int virt,len;
caddr_t area;
{
	return( get_put_data(adr,virt,area,len,TRUE) );
}


/*
 * NAME: get_put_aligned
 *
 * FUNCTION:
 *	Perform the memory transfer operations on data for
 *	which we want to do the appropriate memory operation,
 *	1-byte-lc, 2-bytes-lh, 4-bytes-l.
 *
 * EXECUTION ENVIRONMENT:
 *      This executes in the debugger environment.
 *
 * RETURN VALUE DESCRIPTION: TRUE - The operation worked.
 *			     FALSE-   it failed.
 */
int get_put_aligned(adr,virt,area,wrt,len)
int virt,len,wrt;
ulong *area, adr;
{
	int rval=TRUE;
	ulong orig_segval;
	caddr_t data;
	int segval;
#ifdef _POWER_PC
	uint segnum, i, bat_flag, bat_num;
	uint orig_dbatu, orig_dbatl;
#endif /* ifdef _POWER_PC */

	/* Return if bad address. */
	if ((adr==-1)) return(FALSE);

	/* if IO OR virtual address use virtual & aligned read/writes */
	if( virt) {
		bat_flag = 0;
#ifdef _POWER_PC
		if (__power_pc() && !(__power_601())) {/* Use BATs */
			/* Scan BATs in debvars for match */
			/* If match found set bat_flag */
			/* Assuming 256M mapped for a BAT since that is how */
			/* the pio services do things. */
			segnum = (uint)(adr >> SEGSHIFT);
			for (i=0; i < NUM_KERNEL_BATS; i++) {
				if (debvars[IDBATU + i].hv != 0) {
					if (BAT_ESEG(debvars[IDBATU + i].hv) == segnum) {
						bat_flag = 1;
						bat_num = i;
						break;
					} /* end if (BAT_ESEG... */
				} /* end if (debvars... */
			} /* end for */
			/*
		 	 * If this is a write to a virtual address and the
			 * matching BAT is not marked for writing return FALSE.
		 	 */
			if (bat_flag) {
				if((wrt) && (virt) && !(debvars[IDBATL + bat_num].hv & BT_WRITE)) {
					return(FALSE);
				}
			}
		} /* end if (__power_pc()... */
		if (bat_flag) {
			/*
			 * We have a BAT match so save off the proper BAT and
			 * replace it with the one from debvars and do I/O.
			 */
			switch(bat_num) {
				case 0:
					/* Setup hardware BATs */
					orig_dbatu = mfdbat0u();
					orig_dbatl = mfdbat0l();
					mtdbat0l((uint)debvars[IDBATL].hv);
					mtdbat0u((uint)debvars[IDBATU].hv);

					/* Do I/O */
					if(wrt) write_align(adr,*area,len);
					else *area = read_align(adr,len);
					/* Restore hardware BATs */
					mtdbat0l(orig_dbatl);
					mtdbat0u(orig_dbatu);
					break;	
				case 1:
					/* Setup hardware BATs */
					orig_dbatu = mfdbat1u();
					orig_dbatl = mfdbat1l();
					mtdbat1l((uint)debvars[IDBATL + 1].hv);
					mtdbat1u((uint)debvars[IDBATU + 1].hv);
					/* Do I/O */
					if(wrt) write_align(adr,*area,len);
					else *area = read_align(adr,len);
					/* Restore hardware BATs */
					mtdbat1l(orig_dbatl);
					mtdbat1u(orig_dbatu);
					break;	
				case 2:
					/* Setup hardware BATs */
					orig_dbatu = mfdbat2u();
					orig_dbatl = mfdbat2l();
					mtdbat2l((uint)debvars[IDBATL + 2].hv);
					mtdbat2u((uint)debvars[IDBATU + 2].hv);
					/* Do I/O */
					if(wrt) write_align(adr,*area,len);
					else *area = read_align(adr,len);
					/* Restore hardware BATs */
					mtdbat2l(orig_dbatl);
					mtdbat2u(orig_dbatu);
					break;	
				default:
					printf("The NUM_KERNEL_BATS #define is now greater than 2\n");
					break;	
			} /* end switch */
			return(TRUE);
		} /* end if (bat_flag) */
#endif /* ifdef _POWER_PC */
		segval=debvars[IDSEGS+SREG(adr)].hv;

		/* If write requested and not to IO space, do not allow writes */
		/* to RDONLY memory */
		if((wrt) && (!IS_IOSPACE(debvars[IDSEGS+SREG(adr)].hv)))
			if(!(check_address(segval, adr)))
				return(FALSE);

		/* if NOT i/o space - Return if nonexistent address. */
		if(!IS_IOSPACE(debvars[IDSEGS+SREG(adr)].hv) ) {
			if(lqra(SEG(adr),adr) == -1)
				return (FALSE);
		}

		/* save old segid with Move From Seg Reg routine */
		orig_segval = mfsr(SCRATCH_SEG);
	
		/* insert new segid */
		segval = debvars[IDSEGS+SREG(adr)].hv;
		/* force segid to be key zero */
		segval &= ~KEY_VALUE;

		mtsr(SCRATCH_SEG, segval);
	
		/* read/write data */
		data = (caddr_t) ((adr&0x0FFFFFFF)|((SCRATCH_SEG)<<SEGSHIFT));
		if (wrt)
			write_align(data,*area,len); /* does flush */
		else 
			*area = read_align(data,len); /* does flush */

		/* replace old segid */
		mtsr(SCRATCH_SEG,orig_segval);

		return (TRUE);
	} /* end if (virt) */
	/* We are working with a real address.  Verify that this real address */
	/* is valid on this system. */
	if (!in_real_mem(adr)) return (FALSE);

	/* Doing aligned read/write to real address */
	if (wrt) 
		writbyt(adr,*area,len);		/* does flush */
                         
	else 
		*area = realbyt(adr,len);	/* does flush */

	return(TRUE);
}



/*
 * NAME: rw_virt
 *
 * FUNCTION:
 *	Read or write to a virtual address that we know is paged in.
 *	adr	address to which to read/write
 *	area	where our data is the we want to store/load
 *	wrt	0 =  read, !0 = write
 *
 * RETURN VALUE DESCRIPTION: none.
 */
void
rw_virt(adr,wrt,area)
ulong adr,wrt;
caddr_t area;
{
	ulong orig_segval,new_segval;
	caddr_t data;
	ulong tmpseg;
#ifdef _POWER_PC
	uint bat_flag=0,i,bat_num, segnum;
	uint orig_dbatu, orig_dbatl;

	if (__power_pc() && !(__power_601())) {/* Use BATs */
		/*
		 * Scan BATs in debvars for match
		 * If match found set bat_num and bat_flag.
		 * Assuming 256M mapped for a BAT since that is how
		 * the pio services does things.
	 	 */
		segnum = (uint)(adr >> SEGSHIFT);
		bat_flag = 0;
		for (i=0; i < NUM_KERNEL_BATS; i++) {
			if (debvars[IDBATU + i].hv != 0) {
        			if (BAT_ESEG(debvars[IDBATU + i].hv) == segnum) {
					bat_num = i;
					bat_flag = 1;
					break;
				}
			}
		}
		/*
		 * We have a BAT match so save off the proper BAT and replace
		 * it with the one from debvars and do I/O.
		 */
		if (bat_flag) {
			switch(bat_num) {
				case 0:
					/* Setup hardware BATs */
					orig_dbatu = mfdbat0u();
					orig_dbatl = mfdbat0l();
					mtdbat0l((uint)debvars[IDBATL].hv);
					mtdbat0u((uint)debvars[IDBATU].hv);
					/* Do I/O */
					if(wrt) write_align(adr,*area,1);
					else *area = read_align(adr,1);
					/* Restore hardware BATs */
					mtdbat0l(orig_dbatl);
					mtdbat0u(orig_dbatu);
					break;	
				case 1:
					/* Setup hardware BATs */
					orig_dbatu = mfdbat1u();
					orig_dbatl = mfdbat1l();
					mtdbat1l((uint)debvars[IDBATL + 1].hv);
					mtdbat1u((uint)debvars[IDBATU + 1].hv);
					/* Do I/O */
					if(wrt) write_align(adr,*area,1);
					else *area = read_align(adr,1);
					/* Restore hardware BATs */
					mtdbat1l(orig_dbatl);
					mtdbat1u(orig_dbatu);
					break;	
				case 2:
					/* Setup hardware BATs */
					orig_dbatu = mfdbat2u();
					orig_dbatl = mfdbat2l();
					mtdbat2l((uint)debvars[IDBATL + 2].hv);
					mtdbat2u((uint)debvars[IDBATU + 2].hv);
					/* Do I/O */
					if(wrt) write_align(adr,*area,1);
					else *area = read_align(adr,1);
					/* Restore hardware BATs */
					mtdbat2l(orig_dbatl);
					mtdbat2u(orig_dbatu);
					break;	
				default:
					printf("The NUM_KERNEL_BATS #define is now greater than 2\n");
					break;	
			}
		}
	} 
	if (!bat_flag) {
#endif /* ifdef _POWER_PC */
		if(SREG(adr))		/* adr is some seg other than 0 */
			tmpseg = (SREG(adr));
		else			/* use scratch seg */
			tmpseg = SCRATCH_SEG ;


		/* save old segid with Move From Seg Reg routine */
		orig_segval = mfsr(tmpseg);

		/* insert new segid */
		new_segval=debvars[IDSEGS+SREG(adr)].hv;
		/* force segid to be key zero */
		new_segval &= ~KEY_VALUE;
	
	
		mtsr(tmpseg,new_segval);
	
		/* read/write data */
		data = (caddr_t) ((adr&0x0FFFFFFF)|((tmpseg)<<SEGSHIFT));
	
		if(wrt)
			write_align(data,*area,1);
		else
			*area = read_align(data,1);
	
		/* replace old segid */
		mtsr(tmpseg,orig_segval);
#ifdef _POWER_PC
	}
#endif /* ifdef _POWER_PC */
}


/*
 * NAME: get_put_data
 *
 * FUNCTION:
 *	Perform the memory transfer operations for
 *	get_from_memory and write_to_memory above.
 *
 * EXECUTION ENVIRONMENT:
 *      This executes in the debugger environment.
 *
 * RETURN VALUE DESCRIPTION: TRUE - The operation worked.
 *			     FALSE-   it failed.
 */
int get_put_data(adr,virt,area,len,wrt)
ulong adr;
int virt,len,wrt;
caddr_t area;
{
	int segval;
#ifdef _POWER_PC
	uint segnum, i, bat_flag=0, bat_num;
#endif /* ifdef _POWER_PC */

	/* Return if bad address or no data to handle. */
	if (((int)adr==-1) || (len<=0)) return(FALSE);

#ifdef _POWER_PC
	if (__power_pc() && !(__power_601()) && (virt)) {/* Use BATs */
		/* Scan BATs in debvars for match */
		/* If match found set bat_flag */
		/* Assuming 256M mapped for a BAT since that is how */
		/* the pio services do things. */
		segnum = (uint)(adr >> SEGSHIFT);
		bat_flag = 0;
		for (i=0; i < NUM_KERNEL_BATS; i++) {
			if (debvars[IDBATU + i].hv != 0) {
				if (BAT_ESEG(debvars[IDBATU + i].hv) == segnum) {
					bat_flag = 1;
					bat_num = i;
					break;
				}
			}
		}
		/*
		 * If this is a write to a virtual address and the matching BAT
		 * is not marked for writing return FALSE.
		 */
		if (bat_flag) {
			if((wrt) && (virt) && !(debvars[IDBATL + bat_num].hv & BT_WRITE)) {
				return(FALSE);
			}
		}
	} 
	if (!bat_flag) { /* Use Seg Regs */
#endif /* ifdef _POWER_PC */
		segval=debvars[IDSEGS+SREG(adr)].hv;

		/* 
		 * If write requested and not to IO space, do not allow writes
		 * to RDONLY memory
		 */
		if((wrt) && (virt) && (!IS_IOSPACE(debvars[IDSEGS+SREG(adr)].hv)))
			if(!(check_address(segval, adr)))
				return(FALSE);
#ifdef _POWER_PC
	}
#endif /* ifdef _POWER_PC */

	while(len) {
		if(!debug_xlate(adr,virt))	/* adr is in memory? */
			return(FALSE);
		if(virt) {
			rw_virt(adr,wrt,area);
		}
		else {	/* real address */
			if (wrt) 
				writbyt(adr,*area,1);
			else 
				*area = realbyt(adr,1);
		}
		len--; adr++; area++;
	}
	return(TRUE);
}



/*
 *  Function:	this routine is a cover for get_put_data(), which calls
 *		debug_xlate(), which uses the current value in debvars[]
 *		for the seg id that goes with the address to read/write to.
 *		For example, we have to set up the appropriate seg id in
 *		debvars, which has come out of the stepstoptable,
 *		before get_put_data() is called to re/place a breakpoint.
 *
 *		No BAT manipulations are needed here because the BATs are
 *		used for I/O space addressing and this routine is involved
 *		in setting/clearing breakpoints which always involve
 *		instructions.  The watch point code does not use this
 *		routine.
 *
 *  Returns:	true or false from get_put_data().
 */

pre_get_put_data(addr,write,data,seg_id,virt,len)
ulong addr,write,seg_id,len;
caddr_t data;
{
	ulong seg, old_segid, index;
	int rc;

	index  = IDSEGS+(addr>>SEGSHIFT) ;

	/* save old debvars[sreg] */
	old_segid = debvars[index].hv;

	/* instert new sid to use */
	debvars[index].hv = seg_id;

	rc = get_put_data(addr,virt,data,len,write);

	/* replace old debvars[sreg] */
	debvars[index].hv = old_segid;

	return rc;
}

/*
 *  Function:	this routine is a cover for get_put_aligned(), which calls
 *		debug_xlate(), which uses the current value in debvars[]
 *		for the seg id that goes with the address to read/write to.
 *		For example, we have to set up the appropriate seg id in
 *		debvars, which has come out of the stepstoptable,
 *		before get_put_aligned() is called to re/place a breakpoint.
 *
 *		No BAT manipulations are needed here because the BATs are
 *		used for I/O space addressing and this routine is involved
 *		in setting/clearing breakpoints which always involve
 *		instructions.  The watch point code does not use this
 *		routine.
 *
 *  Returns:	true or false from get_put_data().
 */

pre_get_put_aligned(addr,write,data,seg_id,virt,len)
ulong addr,write,seg_id,len;
caddr_t data;
{
	ulong seg, old_segid, index;
	int rc;

	index  = IDSEGS+(addr>>SEGSHIFT) ;

	/* save old debvars[sreg] */
	old_segid = debvars[index].hv;

	/* instert new sid to use */
	debvars[index].hv = seg_id;

	rc = get_put_aligned(addr,virt,data,write,len);

	/* replace old debvars[sreg] */
	debvars[index].hv = old_segid;

	return rc;
}

