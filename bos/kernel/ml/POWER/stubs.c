static char sccsid[] = "@(#)95	1.1  src/bos/kernel/ml/POWER/stubs.c, sysml, bos411, 9428A410j 7/27/93 19:31:51";
/*
 * COMPONENT_NAME: (SYSML) System Initialization
 *
 * FUNCTIONS: sr_slih
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#define PSIZE 4096


void
v_copypage_com(
	int tsid,
	int tpno,
	int ssid,
	int spno)
{
	int taddr;
	int saddr;

	saddr = vm_att(ssid, spno*PSIZE);
	taddr = vm_att(tsid, tpno*PSIZE);
	
	bcopy(saddr, taddr, PSIZE);
	vm_cflush(taddr, PSIZE);

	vm_det(saddr);
	vm_det(taddr);
}

v_zpage_com(
	int sid,
	int pno)
{
	int addr;

	addr = vm_att(sid, pno*PSIZE);

	bzero(addr, PSIZE);
	vm_cflush(addr, PSIZE);

	vm_det(addr);
}
