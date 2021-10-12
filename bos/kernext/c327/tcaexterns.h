/* @(#)36	1.12  src/bos/kernext/c327/tcaexterns.h, sysxc327, bos411, 9430C411a 7/27/94 09:33:31 */
#ifndef _H_TCAEXTERNS
#define	_H_TCAEXTERNS

/*
 * COMPONENT_NAME: (SYSXC327) c327 tca external declarations
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "tcadecls.h"

/*
**	tcaexterns.h
*/

extern cardData	tca_data[MAX_MINORS]; 

extern char	tca_sess_type;
extern char	printerAddr; 
extern int	c327_dev_major;

/* tcaclose.c */
int tcaclose (dev_t, int);
short mscan_link_ptrs(int);
void mfree_la_struct (int, linkAddr *, int);

/* tcaintr.c */
void mupdate_link_fields(int, linkAddr *, int, int);
void clearIoInProg (linkAddr *);
void tcaintr (NETWORK_ID, INTR_TYPE, int, int);
void aixSendStat (linkAddr *);
int isApiCmdWSF(char *);

/* tcaioctl.c */
int tcaioctl (dev_t, int, int, int, int);
void mtca_inquire_link(linkAddr *, iotca *, int, int);
void wakeup_mwait_for_io(linkAddr *);
int mwait_for_io(linkAddr *, int, int, int);
void wakeup_mwait_for_io(linkAddr *);

/* tcampx.c */
int tcampx(dev_t, int *, char *);

/* tcaopen.c */
int tcaopen(dev_t, int, int, DDS_DATA *);

/* tcaread.c */
int tcaread(dev_t, struct uio *, int, int);

/* tcaselect.c */
int tcaselect (dev_t, ushort, ushort *, int);

/* tcasubnpn.c */
void mdepHaltLA (int, linkAddr *, int);
int mdepClosePort (int, int);
int mdepStartLA (linkAddr *, int, int, int, void *);
int mdepOpenPort (int);
Data_Buffer *bufferAlloc (int, Data_Buffer *, struct uio *);
void bufferFree (Data_Buffer *);
int waitForLinkState (linkAddr *, state);

/* tcasubpn.c */
int mdepSendStatus (int, int, int);
int mdepWriteBuffer (int, int, Data_Buffer *);
int bufferEmpty(Data_Buffer *);
void mlogerr (int, int, int, int);
void setLinkState (linkAddr *, state);

/* tcawrite.c */
int tcawrite(dev_t, struct uio *, int, int);

#endif	/* _H_TCAEXTERNS */
