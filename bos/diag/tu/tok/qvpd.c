static char sccsid[] = "src/bos/diag/tu/tok/qvpd.c, tu_tok, bos411, 9428A410j 6/20/91 12:41:24";
/*
 * COMPONENT_NAME: (TU_TOK) Token Test Unit
 *
 * FUNCTIONS: qvpd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Query Vital Product Data

Module Name :  qvpd.c
SCCS ID     :  1.5

Current Date:  6/20/91, 10:14:45
Newest Delta:  1/19/90, 16:49:25

Function(s) attempts to read vital product data (VPD) off the Token Card
esp. to grab the network address to use for open options to the adapter.

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/tokuser.h>
#include <memory.h>

#include "toktst.h"

#ifdef debugg
extern void detrace();
#endif

int qvpd (fdes, tucb_ptr, net_add)
   int fdes;
   TUTYPE *tucb_ptr;
   unsigned char *net_add;
   {
	tok_vpd_t vpd_s;
	unsigned char *ucp;
	int cc;
#ifdef debugg
	int i;
#endif

	struct htx_data *htx_sp;
	extern int errno;
	extern int mktu_rc();

#ifdef debugg
	detrace(0,"\n\nqvpd:  Begins...\n");
#endif
	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->token_s.htx_sp;

	cc = ioctl(fdes, TOK_QVPD, &vpd_s);
	if (cc)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
	
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, QVPD_ERR));
	   }
#ifdef debugg
	detrace(0,"qvpd:  This is what I got in the VPD:\n");
	for (i = 0; i < vpd_s.l_vpd; i++)
	   {
		if (i % 16)
		detrace(0,"qvpd:  locat:  %02X, hex:  %02X, ascii:  %c\n", 
			i + 1, vpd_s.vpd[i], vpd_s.vpd[i]);
		else
		detrace(1,"qvpd:  locat:  %02X, hex:  %02X, ascii:  %c\n", 
			i + 1, vpd_s.vpd[i], vpd_s.vpd[i]);
	   }
#endif
	
	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	
	ucp = vpd_s.vpd;
	memcpy(net_add, ucp + NET_OFFSET, NETADD_LEN);

	return(PASSED);
   }
