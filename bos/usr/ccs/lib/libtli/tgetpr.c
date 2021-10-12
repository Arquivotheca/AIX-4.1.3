static char sccsid[] = "@(#)01  1.3  src/bos/usr/ccs/lib/libtli/tgetpr.c, libtli, bos411, 9428A410j 3/8/94 19:12:36";
/*
 *   COMPONENT_NAME: LIBTLI
 *
 *   FUNCTIONS: t_getprotaddr
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "common.h"
#include <sys/tihdr.h>
#include <sys/timod.h>

int
t_getprotaddr (fd, boundaddr, peeraddr)
	int	fd;
	struct t_bind	* boundaddr;
	struct t_bind	* peeraddr;
{
	char			* buf = 0;
	char			stack_buf[TLI_STACK_BUF_SIZE];
	struct T_addr_req	*taddrr;
	struct T_addr_ack	*taddra;
	int			total_len;
	int			code;
	struct tli_st		* tli;

	code = -1;
	if (!(tli = iostate_lookup(fd, IOSTATE_VERIFY)))
		return code;

	if (tli->tlis_state == T_UNBND) {
		if (boundaddr)
			boundaddr->addr.len = 0;
		if (peeraddr)
			peeraddr->addr.len = 0;
		code = 0;
		goto rtn;
	}

	total_len = sizeof(struct T_addr_ack) + boundaddr->addr.maxlen +
			peeraddr->addr.maxlen;
	if (total_len > sizeof(stack_buf)) {
		if (!(buf = (char *)malloc(total_len))) {
			t_errno = TSYSERR;
			errno = ENOMEM;
			goto rtn;
		}
	} else
		buf = stack_buf;

	taddrr = (struct T_addr_req *)&buf[0];
	taddrr->PRIM_type = T_ADDR_REQ;

	if (tli_ioctl(fd, TI_ADDR, buf, total_len) == -1)
		goto rtn;

	taddra = (struct T_addr_ack *)&buf[0];
	if (boundaddr) {
		if (taddra->LOCADDR_length > 0 && boundaddr->addr.maxlen > 0) {
			if (boundaddr->addr.maxlen < taddra->LOCADDR_length) {
				t_errno = TBUFOVFLW;
				goto rtn;
			}
			boundaddr->addr.len = taddra->LOCADDR_length;
			memcpy(boundaddr->addr.buf,&buf[taddra->LOCADDR_offset],
				boundaddr->addr.len);
		} else 
			boundaddr->addr.len = 0;
	}

	if (peeraddr) {
		if (tli->tlis_state == T_DATAXFER) {
			if (taddra->REMADDR_length > 0 && 
					peeraddr->addr.maxlen > 0) {
				if (peeraddr->addr.maxlen < 
						taddra->REMADDR_length) {
					t_errno = TBUFOVFLW;
					goto rtn;
				}
				peeraddr->addr.len = taddra->REMADDR_length;
				memcpy(peeraddr->addr.buf, 
					&buf[taddra->REMADDR_offset], 
					peeraddr->addr.len);
			} else 
				peeraddr->addr.len = 0;
		} else 
			peeraddr->addr.len = 0;
	}
	code = 0;
rtn:
	if (buf && buf != stack_buf)
		free(buf);
	TLI_UNLOCK(tli);

	return code;
}
