static char sccsid[] = "@(#)65  1.12  src/bos/usr/bin/mail/quit.c, cmdmailx, bos41J, 9516B_all 4/19/95 15:34:43";
/* 
 * COMPONENT_NAME: CMDMAILX quit.c
 * 
 * FUNCTIONS: MSGSTR, quit, writeback 
 *
 * ORIGINS: 10  26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * #ifndef lint
 * static char *sccsid = "quit.c       5.3 (Berkeley) 3/6/86";
 * #endif not lint
 */

#include <sys/stat.h>
#include <sys/file.h>
#include <sys/syspest.h>
#include <sys/errno.h>
#include "rcv.h"

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/* declare BUGLPR variable as extern */

BUGXDEF(mailbug)


/*
 * Rcv -- receive mail rationally.
 *
 * Termination processing.
 */

/*
 * Save all of the undetermined messages at the top of "mbox"
 * Save all untouched messages back in the system mailbox.
 * Remove the system mailbox, if none saved there.
 */

quit()
{
	int mcount, p, modify, autohold, anystat, holdbit, nohold;
	FILE *ibuf, *obuf, *fbuf, *rbuf, *readstat, *abuf;
	register struct message *mp;
	register int c, ck;
	extern char tempQuit[], tempResid[];
	struct stat minfo;
	char *id;
	char *cp, *cp2;

	/*
	 * If we are read only, we can't do anything,
	 * so just return quickly.
	 */

	if (readonly)
		return;
	/*
	 * See if there any messages to save in mbox.  If no, we
	 * can save copying mbox to /tmp and back.
	 *
	 * Check also to see if any files need to be preserved.
	 * Delete all untouched messages to keep them out of mbox.
	 * If all the messages are to be preserved, just exit with
	 * a message.
	 *
	 * If the luser has sent mail to himself, refuse to do
	 * anything with the mailbox, unless mail locking works.
	 */

	fbuf = Fopen(mailname, "r");
	if (fbuf == NULL)
		goto newmail;
#ifdef AIX
	lockf (fileno(fbuf), F_LOCK, 0);
#else
	flock(fileno(fbuf), LOCK_EX);
#endif
#ifndef CANLOCK
	if (selfsent) {
		printf(MSGSTR(NEWMAIL, "You have new mail.\n")); /*MSG*/
		Fclose(fbuf);
		return;
	}
#endif
	rbuf = NULL;
	if (fstat(fileno(fbuf), &minfo) >= 0 && minfo.st_size > mailsize) {
		printf(MSGSTR(MAILTIME, "New mail has arrived.\n")); /*MSG*/
		rbuf = Fopen(tempResid, "w");
		if (rbuf == NULL || fbuf == NULL)
			goto newmail;
#ifdef APPEND
		fseek(fbuf, mailsize, 0);
		while ((c = getc(fbuf)) != EOF) {
			ck = putc(c, rbuf);
			if (ck == EOF) {
			perror(tempResid);
			Fclose(rbuf);
			Fclose(fbuf);
			return;
			}
		}
#else
		p = minfo.st_size - mailsize;
		while (p-- > 0) {
			c = getc(fbuf);
			if (c == EOF)
				goto newmail;
			ck = putc(c, rbuf);
			if (ck == EOF) {
			perror(tempResid);
			Fclose(rbuf);
			Fclose(fbuf);
			return;
			}
		}
#endif
		Fclose(rbuf);
		if ((rbuf = Fopen(tempResid, "r")) == NULL)
			goto newmail;
		remove_file(tempResid);
	}

	/*
	 * Adjust the message flags in each message.
	 */

	anystat = 0;
	autohold = value("hold") != NOSTR;
	holdbit = autohold ? MPRESERVE : MBOX;
	nohold = MBOX|MSAVED|MDELETED|MPRESERVE;
	if (value("keepsave") != NOSTR) {
		nohold &= ~MSAVED;
		holdbit = MPRESERVE;
	}
	for (mp = &message[0]; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MNEW) {
			mp->m_flag &= ~MNEW;
			mp->m_flag |= MSTATUS;
		}
		if (mp->m_flag & MSTATUS)
			anystat++;
		if ((mp->m_flag & MTOUCH) == 0)
			mp->m_flag |= MPRESERVE;
		if ((mp->m_flag & nohold) == 0)
			mp->m_flag |= holdbit;
	}
	modify = 0;
	if (Tflag != NOSTR) {
		if ((readstat = Fopen(Tflag, "w")) == NULL)
			Tflag = NOSTR;
	}
	for (c = 0, p = 0, mp = &message[0]; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MBOX)
			c++;
		if (mp->m_flag & MPRESERVE)
			p++;
		if (mp->m_flag & MODIFY)
			modify++;
		if (Tflag != NOSTR && (mp->m_flag & (MREAD|MDELETED)) != 0) {
			id = hfield("article-id", mp);
			if (id != NOSTR)
				fprintf(readstat, "%s\n", id);
		}
	}
	if (Tflag != NOSTR)
		Fclose(readstat);
	if (p == msgCount && !modify && !anystat) {
		if (p == 1)
			printf(MSGSTR(MSGHELD, "Held 1 message in %s\n"), mailname); /*MSG*/
		else
			printf(MSGSTR(MSGSHELD, "Held %2d messages in %s\n"), p, mailname); /*MSG*/
		Fclose(fbuf);
		return;
	}
	if (c == 0) {
		if (p != 0) {
			writeback(rbuf);
			Fclose(fbuf);
			return;
		}
		goto cream;
	}

	/*
	 * Create another temporary file and copy user's mbox file
	 * darin.  If there is no mbox, copy nothing.
	 * If he has specified "append" don't copy his mailbox,
	 * just copy saveable entries at the end.
	 */

	mcount = c;
	if (value("append") == NOSTR) {
		if ((obuf = Fopen(tempQuit, "w")) == NULL) {
			perror(tempQuit);
			Fclose(fbuf);
			return;
		}
		if ((ibuf = Fopen(tempQuit, "r")) == NULL) {
			perror(tempQuit);
			remove_file(tempQuit);
			Fclose(obuf);
			Fclose(fbuf);
			return;
		}
		remove_file(tempQuit);
		if ((abuf = Fopen(mbox, "r")) != NULL) {
			while ((c = getc(abuf)) != EOF) {
				ck = putc(c, obuf);
				if (ck == EOF) {
				perror(tempQuit);
				Fclose(ibuf);
				Fclose(obuf);
				Fclose(fbuf);
				return;
				}
			}
			Fclose(abuf);
		}
		if (ferror(obuf)) {
			perror(tempQuit);
			Fclose(ibuf);
			Fclose(obuf);
			Fclose(fbuf);
			return;
		}
		Fclose(obuf);
		close(creat(mbox, 0600));
		if ((obuf = Fopen(mbox, "r+")) == NULL) {
			perror(mbox);
			Fclose(ibuf);
			Fclose(fbuf);
			return;
		}
	}
	if (value("append") != NOSTR) {

		if ((obuf = Fopen(mbox, "a")) == NULL) {
			perror(mbox);
			Fclose(fbuf);
			return;
		}

		/* this is to compensate for the open() behavior that
		   causes the file ptr to == 0 even on open for append,
		   as per POSIX specs (?) */
		
		fseek(obuf, 0L, SEEK_END);

		BUGLPR(mailbug, BUGACT,
		    ("quit: opened '%s' for append, offset=%ld\n",
		    mbox, ftell(obuf)));

#ifdef AIX
		chmod (mbox, 0600);
#else
		fchmod(fileno(obuf), 0600);
#endif
	}
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MBOX) {

			BUGLPR(mailbug, BUGACT,
			    ("quit: sending message %d to %s, offset=%ld\n",
			    ((mp - &message[0]) + sizeof(struct message))
			    / sizeof(struct message), mbox, ftell(obuf)));

			if (send(mp, obuf, 0, NOSTR) < 0) {
				perror(mbox);
				Fclose(ibuf);
				Fclose(obuf);
				Fclose(fbuf);
				return;
			}

			BUGLPR(mailbug, BUGACT,
			    ("quit: after send, offset=%ld\n", ftell(obuf)));

		}

	/*
	 * Copy the user's old mbox contents back
	 * to the end of the stuff we just saved.
	 * If we are appending, this is unnecessary.
	 */

	if (value("append") == NOSTR) {
		rewind(ibuf);
		c = getc(ibuf);
		while (c != EOF) {
			putc(c, obuf);
			if (ferror(obuf))
				break;
			c = getc(ibuf);
		}
		Fclose(ibuf);
		fflush(obuf);
	}
	trunc(obuf);
	if (ferror(obuf)) {
		perror(mbox);
		Fclose(obuf);
		Fclose(fbuf);
		return;
	}

	BUGLPR(mailbug, BUGACT, ("quit: after trunc: offset=%ld\n",
	    ftell(obuf)));

	if (Fclose(obuf) == EOF) {
		perror(mbox);
		return;
	}
	if (mcount == 1)
		printf(MSGSTR(MSGSAVED, "Saved 1 message in %s\n"), mbox); /*MSG*/
	else
		printf(MSGSTR(MSGSSAVED, "Saved %d messages in %s\n"), mcount, mbox); /*MSG*/

	/*
	 * Now we are ready to copy back preserved files to
	 * the system mailbox, if any were requested.
	 */

	if (p != 0) {
		writeback(rbuf);
		Fclose(fbuf);
		return;
	}

	/*
	 * Finally, remove his /usr/mail file.
	 * If new mail has arrived, copy it back.
	 */

cream:
	if (rbuf != NULL) {
		abuf = Fopen(mailname, "r+");
		if (abuf == NULL)
			goto newmail;
		while ((c = getc(rbuf)) != EOF)
			putc(c, abuf);
		Fclose(rbuf);
		trunc(abuf);
		Fclose(abuf);
		alter(mailname);
		Fclose(fbuf);
		return;
	}
	demail(mailname);
	Fclose(fbuf);
	return;

newmail:
	printf(MSGSTR(THOU, "Thou hast new mail.\n")); /*MSG*/
	if (fbuf != NULL)
		Fclose(fbuf);
}

/*
 * Preserve all the appropriate messages back in the system
 * mailbox, and print a nice message indicated how many were
 * saved.  On any error, just return -1.  Else return 0.
 * Incorporate the any new mail that we found.
 */
writeback(res)
	register FILE *res;
{
	register struct message *mp;
	register int p, c, ck;
	FILE *obuf;

	p = 0;
	if ((obuf = Fopen(mailname, "r+")) == NULL) {
		perror(mailname);
		return(-1);
	}
#ifndef APPEND
	if (res != NULL)
		while ((c = getc(res)) != EOF) {
			ck = putc(c, obuf);
			if (ck == EOF) {
			perror(mailname);
			Fclose(obuf);
			return(-1);
			}
 		}
#endif
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if ((mp->m_flag&MPRESERVE)||(mp->m_flag&MTOUCH)==0) {
			p++;
			if (send(mp, obuf, 0, NOSTR) < 0) {
				perror(mailname);
				Fclose(obuf);
				return(-1);
			}
		}
#ifdef APPEND
	if (res != NULL)
		while ((c = getc(res)) != EOF) {
			ck = putc(c, obuf);
			if (ck == EOF) {
			perror(mailname);
			Fclose(obuf);
			return(-1);
			}
 		}
#endif
	fflush(obuf);
	trunc(obuf);
	if (ferror(obuf)) {
		perror(mailname);
		Fclose(obuf);
		return(-1);
	}
	if (res != NULL)
		Fclose(res);
	if (Fclose(obuf) == EOF) {
		perror(mailname);
		return(-1);
	}
	alter(mailname);
	if (p == 1)
		printf(MSGSTR(MSGHELD, "Held 1 message in %s\n"), mailname); /*MSG*/
	else
		printf(MSGSTR(MSGSKEPT, "Held %d messages in %s\n"), p, mailname); /*MSG*/
	return(0);
}
