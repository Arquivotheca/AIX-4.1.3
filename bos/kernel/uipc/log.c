static char sccsid[] = "@(#)42	1.4.1.4  src/bos/kernel/uipc/log.c, sysuipc, bos411, 9428A410j 6/7/94 13:27:22";
/*
 *   COMPONENT_NAME: SYSUIPC
 *
 *   FUNCTIONS: bsdlog
 *		islogfile
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdarg.h>
#include <sys/var.h>
#include <sys/mbuf.h>
#include <sys/un.h>
#include <sys/unpcb.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/syslog.h>
#include <sys/syspest.h>
#include <sys/file.h>
#include <sys/fp_io.h>

#define islogfile(so) \
			(sotounpcb(so)->unp_addr->m_len == sizeof(log_addr) + \
			sizeof(((struct sockaddr_un *)log_addr)->sun_len) + \
			sizeof(((struct sockaddr_un *)log_addr)->sun_family) &&\
			!bcmp(log_addr, mtod(sotounpcb(so)->unp_addr, \
			struct sockaddr_un *)->sun_path, sizeof(log_addr)))

static char log_addr[] = "/dev/log";  /* XXX changeable by syslogd's -d flag */
struct socket *log_so = NULL;  /* points to syslogd's log socket */


/*
 * NAME: bsdlog
 *                                                                    
 * FUNCTION: Formats a message and sends it to syslogd.
 *                                                                    
 * NOTES:
 *   Similar to kernel printf() but we send the formatted message to
 *   syslogd's socket, if it exists and is valid.
 *
 *   NB that we simply give up if any problems occur (although the
 *   message is still logged to the printbuf circular buffer).
 *
 * DATA STRUCTURES:  Updates the global circular buffer in printbuf.
 *
 * RETURN VALUE: None.
 */
void 
bsdlog(int pri, char *fmt, ...)
/* VARARGS */
{
	char buf[256];
	va_list vap;
	register char *cp;
	register int len;
	static char pfx[] = "<2>unix: ";
#define pfx_siz (sizeof(pfx) - 1)

	if (getpid() == -1)
		return;

	if (csa->intpri != INTBASE)
		return;

	assert(strlen(fmt) <= (MHLEN - pfx_siz));

	/* format the message into the buffer */
	va_start(vap, fmt);
	len = _doprnt(fmt, vap, buf);
	va_end(vap);

	assert(len <= (MHLEN - pfx_siz));

	/* log data to circular buffer */
	logbuf(len, buf);

	UNPMISC_LOCK();
	if (log_so) {
		/* prefix of syslog message: we set priority below */
		static struct sockaddr_un from = { sizeof(from), AF_UNIX };
		register struct mbuf *m;

		/* get an mbuf and insert the syslog prefix */
		m = m_gethdr(M_DONTWAIT, MT_DATA);
		if (!m)  /* just forget it */
			goto out;
		cp = mtod(m, char *);
		bcopy(pfx, cp, pfx_siz);

		/* set message priority; use default if none specified */
		if (pri == -1)
			pri = LOG_CRIT;
		else
			pri &= LOG_PRIMASK;
		*(cp+1) = '0' + pri;  /* ascii-centric, but why be different? */

		/*
		 * copy message after the prefix, truncating if necessary
		 * to make it fit in one mbuf
		 */
		if ((m->m_len = len + pfx_siz) > MHLEN) {
			m->m_len = MHLEN;
			len = MHLEN - pfx_siz;
		}
		m->m_pkthdr.len = m->m_len;
		bcopy(buf, cp + pfx_siz, len);

		SOCKET_LOCK(log_so);
		if (! (log_so->so_snd.sb_flags & SB_LOCK) &&
		    sbappendaddr(&log_so->so_rcv, &from, m, NULL)) {
			sorwakeup(log_so);

		} else {  /* buffer is locked or we can't append to it */
			m_free(m);
		}
		SOCKET_UNLOCK(log_so);
	} else if (pri & LOG_CONS) {
		struct file *fd;
		int count;

		UNPMISC_UNLOCK();
		if (fp_open("/dev/console", O_WRONLY|O_NOCTTY|O_NDELAY, 
		      0777,0, FP_SYS, &fd))
			return;

		fp_write(fd, buf, len, 0, UIO_SYSSPACE, &count);
		fp_close(fd);
		return;
	}
out:
	UNPMISC_UNLOCK();
}

bsdlog_reg(struct socket *so) {
	UNPMISC_LOCK();
	if (islogfile(so))
		log_so = so;
	UNPMISC_UNLOCK();
}

bsdlog_unreg(struct socket *so) {
	UNPMISC_LOCK();
	if (islogfile(so))
		log_so = NULL;
	UNPMISC_UNLOCK();
}
