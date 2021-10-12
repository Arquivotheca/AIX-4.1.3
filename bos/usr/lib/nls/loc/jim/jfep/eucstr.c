static char sccsid[] = "@(#)66	1.5  src/bos/usr/lib/nls/loc/jim/jfep/eucstr.c, libKJI, bos411, 9428A410j 9/29/93 21:44:24";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : SjisToEuc
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <iconv.h>
#include <sys/errno.h>

#define	MAX_932_LEN	2
#define	MAX_EUC_LEN	3

static int	conv_one(iconv_t cd,
	unsigned char *srcb, unsigned char *dstb, int *dstl)
{
	int	srcl, ret;
	unsigned char	*srcbuf, *dstbuf;
	int	srclen, dstlen;

	for (srcl = 1; srcl <= MAX_932_LEN; srcl++) {
		srcbuf = srcb;
		dstbuf = dstb;
		srclen = srcl;
		dstlen = MAX_EUC_LEN;
		ret    = iconv(cd, &srcbuf, (size_t *)&srclen,
			&dstbuf, (size_t *)&dstlen); 
		if ( ret >= 0 ) {
			*dstl = MAX_EUC_LEN - dstlen;
			return srcl;
		}
		else {
		    	if( errno == EINVAL )
			        continue;
			else
				break;
		}
	}
	/*
	 *	ICONV_OVER or ICONV_INVAL or exceed MAX_932_LEN
	 *	These should never happen.
	 *	Accordingly, the following code will never be executed.
	 */
	if (srcl == 1) {
		dstb[0] = 0x20;
		*dstl = 1;
	}
	else {
		dstb[0] = 0xa1;
		dstb[1] = 0xa1;
		*dstl = 2;
	}
	return srcl;
}

/*
 *	SjisToEuc()
 *	Convert IBM-932 string to IBM-eucJP
 *	p1, p2, p3 contain positions in the source string if they are not
 *	NULL and contain corresponding positions in the destination string
 *	on return.   Used for cursor position translation.
 *	SjisToEucAtt()
 *	This function also copy the attribute string appropreately.
 */
int	SjisToEuc(iconv_t cd,
	unsigned char *sjis_buf, unsigned char *euc_buf, int sjis_len,
	unsigned int *p1, unsigned int *p2, unsigned int *p3)
{
	int	slen, elen, euc_len;
	int	i;
	unsigned int	q1, q2, q3;

	q1 = q2 = q3 = 0;
	for (i = euc_len = 0; i < sjis_len; ) {
		slen = conv_one(cd, sjis_buf, euc_buf, &elen);
		if (p1 && i <= *p1 && *p1 < i + slen)
			q1 = euc_len;
		if (p2 && i <= *p2 && *p2 < i + slen)
			q2 = euc_len;
		if (p3 && i <= *p3 && *p3 < i + slen)
			q3 = euc_len;
		i += slen;
		sjis_buf += slen;
		euc_len += elen;
		euc_buf += elen;
	}
	if (p1)
		if (i <= *p1)
			*p1 = euc_len;
		else
			*p1 = q1;
	if (p2)
		if (i <= *p2)
			*p2 = euc_len;
		else
			*p2 = q2;
	if (p3)
		if (i <= *p3)
			*p3 = euc_len;
		else
			*p3 = q3;
	return euc_len;
}

int	SjisToEucAtt(iconv_t cd,
	unsigned char *sjis_buf, unsigned char *sjis_att,
	unsigned char *euc_buf, unsigned char *euc_att, int sjis_len,
	unsigned int *p1, unsigned int *p2, unsigned int *p3)
{
	int	slen, elen, euc_len;
	unsigned int	i;
	unsigned int	q1, q2, q3;

	q1 = q2 = q3 = 0;
	for (i = euc_len = 0; i < sjis_len; ) {
		slen = conv_one(cd, sjis_buf, euc_buf, &elen);
		(void)memset(euc_att, sjis_att[0], elen);
		if (p1 && i <= *p1 && *p1 < i + slen)
			q1 = euc_len;
		if (p2 && i <= *p2 && *p2 < i + slen)
			q2 = euc_len;
		if (p3 && i <= *p3 && *p3 < i + slen)
			q3 = euc_len;
		i += slen;
		sjis_buf += slen;
		euc_len += elen;
		euc_buf += elen;
		sjis_att += slen;
		euc_att += elen;
	}
	if (p1)
		if (i <= *p1)
			*p1 = euc_len;
		else
			*p1 = q1;
	if (p2)
		if (i <= *p2)
			*p2 = euc_len;
		else
			*p2 = q2;
	if (p3)
		if (i <= *p3)
			*p3 = euc_len;
		else
			*p3 = q3;
	return euc_len;
}
