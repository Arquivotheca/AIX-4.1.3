static char sccsid[] = "@(#)70	1.14  src/bos/usr/bin/mail/send.c, cmdmailx, bos41J, 9523A_all 6/5/95 14:27:42";
/* 
 * COMPONENT_NAME: CMDMAILX send.c
 * 
 * FUNCTIONS: MSGSTR, fixhead, fmt, headerp, infix, mail, mail1, 
 *            puthead, savemail, send, sendmail, statusput 
 *
 * ORIGINS: 10  26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
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
 * static char *sccsid = "send.c       5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

#include "rcv.h"
#include <sys/wait.h>
#include <ctype.h>
#include <sys/stat.h>

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

extern short file_recip;
extern char  *recip_name;

/*
 * Mail -- a mail program
 *
 * Mail to others.
 */

/*
 * Send message described by the passed pointer to the
 * passed output buffer.  Return -1 on error, but normally
 * the number of lines written.  Adjust the status: field
 * if need be.  If doign is set, suppress ignored header fields.
 */
send(mp, obuf, doign, prefix)
	struct message *mp;
	FILE *obuf;
	char *prefix;
{
	long count;
	register FILE *ibuf;
	char line[LINESIZE], field[BUFSIZ];
	int ishead, infld, ignoring, dostat, firstline;
	register char *cp, *cp2;
	register int c;
	int length, linecount, i;
	int prefixlen;
	int counttop;   /* keeps count for displaying top variable */      

	/*
	 * Compute the prefix string, without trailing whitespace.
	 */
	if (prefix != NOSTR) {
		cp2 = 0;
		for (cp = prefix; *cp; cp++)
			if (*cp != ' ' && *cp != '\t')
				cp2 = cp;
		prefixlen = cp2 == 0 ? 0 : cp2 - prefix + 1;
	}
	ibuf = setinput(mp);
	count = mp->m_size;
	ishead = 1;
	dostat = doign == 0 || !isign("status");
	infld = 0;
	firstline = 1;
	linecount = 0;
	counttop = 0;
	while (count > 0L && ishead) {
		if (fgets(line, LINESIZE, ibuf) == NULL)
			break;
		count -= length = (long) strlen(line);
		linecount++;
		if (firstline) {
			/* 
			 * First line is the From line, so no headers
			 * there to worry about
			 */
			firstline = 0;
			ignoring = 0;
		} else if (line[0] == '\n') {
			/*
			 * If line is blank, we've reached end of
			 * headers, so force out status: field
			 * and note that we are no longer in header
			 * fields
			 */
			if (dostat) {
				statusput(mp, obuf, prefix);
				dostat = 0;
				counttop++;
			}
			ishead = 0;
			ignoring = 0;
		} else if (infld && (line[0] == ' ' || line[0] == '\t')) {
			/*
			 * If this line is a continuation (via space or tab)
			 * of a previous header field, just echo it
			 * (unless the field should be ignored).
			 * In other words, nothing to do.
			 */
		} else {
			/*
			 * Pick up the header field if we have one. 
			 */
			for (cp = line;(c = *cp++) && c != ':' && !isspace(c);)
				;
			cp2 = --cp;
			while (isspace(*cp++))
				;
			if (cp[-1] != ':') {
				/*
				 * Not a header line, force out status:
				 * This happens in uucp style mail where
				 * there are no headers at all.
				 */	 
				if (dostat) {
					statusput(mp, obuf, prefix);
					dostat = 0;
					counttop++;
				}
				putc('\n', obuf);
				ishead = 0;
				ignoring = 0;
			} else {
			/*
			 * If it is an ignored field and
			 * we care about such things, skip it.
			 */
				cp = line;
				cp2 = field;
				while (*cp && *cp != ':' && !isspace(*cp))
					*cp2++ = *cp++;
				*cp2 = 0;	/* temporarily null terminate */
				if (doign && isign(field))
					ignoring = 1;
				/*
			 	 * If the field is "status," go compute and
				 * print the real Status: field
			 	 */
				else if (icequal(field, "status")) {
					if (dostat) {
						statusput(mp, obuf, prefix);
						dostat = 0;
						counttop++;
					}	
					ignoring = 1;	
				} else {
					ignoring = 0;
					*cp2 = c;	/* restore */
				}
				infld = 1;
			}
		}
		if (!ignoring) {
			/*
		 	 * Strip trailing whitespace from prefix
		 	 * if line is blank.
		 	 */
			if (prefix != NOSTR)
				if (length > 1)
					fputs(prefix, obuf);
				else
					(void) fwrite(prefix, sizeof *prefix, 
						prefixlen, obuf);
				(void) fwrite(line, sizeof *line, length, obuf);
				if (ferror(obuf)) {
					if (called_from_top)
						called_from_top = FALSE;
					return -1;
				}
			counttop++;
		}
		/* Set called_from_top to false so that we do not
		 * venture down this same path  if we call another
		 * command like "type" after "top".
		 */
		if (called_from_top  && (counttop >= topl)) {
			called_from_top = FALSE;
			return;
		}
	}	
	/* 
	 * Copy out message body
	 */
	if (prefix != NOSTR)
		while (count > 0) {
			if (fgets(line, LINESIZE, ibuf) == NULL) {
				c = 0;
				break;
			}
			count -= c = strlen(line);
			for (i=0; i<strlen(line); i++)
				if (line[i] == '\n')
					linecount++;
			/* 
			 * Strip trailing whitespace from prefix
		 	 * if line is blank.
		 	 */
			if (c > 1)
				fputs(prefix, obuf);
			else
				(void)fwrite(prefix, sizeof *prefix,
						 prefixlen, obuf);
			(void)fwrite(line, sizeof *line, c, obuf);
			if (ferror(obuf))
				return -1;
		}
	else if (called_from_top) {
		/* Set called_from_top to false so that we do not
		 * venture down this same path  if we call another
		 * command like "type" after "top".
		 */
		called_from_top = FALSE;
		while (count > 0 && (counttop < topl)) {
			if (fgets(line, LINESIZE, ibuf) == NULL) {
				c = 0;
				break;
			}
			count -= c = strlen(line);
			for (i=0; i<strlen(line); i++)
				if (line[i] == '\n') {
					linecount++;
					counttop++;
				}
			(void)fwrite(line, sizeof *line, c, obuf);
			if (ferror(obuf))
				return -1;
		}
	}
	else  
		while (count > 0) {
			c = count < LINESIZE ? count : LINESIZE;
			if ((c = fread(line, sizeof *line, c, ibuf)) <= 0)
				break;
			count -= c;
			for (i=0; i<strlen(line); i++)
				if (line[i] == '\n')
					linecount++;
			if (fwrite(line, sizeof *line, c, obuf) != c)
				return -1;
		}
	if (c > 0 && line[c-1] != '\n')
		/* no final blank line */
		if ((c = getc(ibuf)) != EOF && putc(c, obuf) == EOF)
			return -1;
	return(linecount);
}	

/*
 * Test if the passed line is a header line, RFC 733 style.
 */
headerp(line)
	register char *line;
{
	register char *cp = line;

	while (*cp && !isspace(*cp) && *cp != ':')
		cp++;
	while (*cp && isspace(*cp))
		cp++;
	return(*cp == ':');
}

/*
 * Output a reasonable looking status field.
 * But if "status" is ignored and doign, forget it.
 */
statusput(mp, obuf, prefix)
	register struct message *mp;
	register FILE *obuf;
	char *prefix;
{
	char statout[3];

	if ((mp->m_flag & (MNEW|MREAD)) == MNEW)
		return;
	if (mp->m_flag & MREAD)
		strcpy(statout, "R");
	else
		strcpy(statout, "");
	if ((mp->m_flag & MNEW) == 0)
		strcat(statout, "O");
	fprintf(obuf, MSGSTR(STATUS, "%sStatus: %s\n"), prefix == NOSTR ? "" : prefix, statout); /*MSG*/
}


/*
 * Interface between the argument list and the mail1 routine
 * which does all the dirty work.
 */

mail(people)
	char **people;
{
	register char *cp2;
	register int s;
	char *buf, **ap;
	struct header head;

	for (s = 0, ap = people; *ap != (char *) 0; ap++)
		s += strlen(*ap) + 1;
	buf = salloc(s+1);
	cp2 = buf;
	for (ap = people; *ap != (char *) 0; ap++) {
		cp2 = copy(*ap, cp2);
		*cp2++ = ' ';
	}
	if (cp2 != buf)
		cp2--;
	*cp2 = '\0';
	head.h_to = buf;
	head.h_subject = NOSTR;
	head.h_cc = NOSTR;
	head.h_bcc = NOSTR;
	head.h_seq = 0;
	mail1(&head, 0, 0);
	return(0);
}


/*
 * Send mail to a bunch of user names.  The interface is through
 * the mail routine below.
 */

sendmail(str)
	char *str;
{
	register char **ap;
	char *bufp;
	register int t;
	struct header head;

	if (blankline(str))
		head.h_to = NOSTR;
	else
		head.h_to = str;
	head.h_subject = NOSTR;
	head.h_cc = NOSTR;
	head.h_bcc = NOSTR;
	head.h_seq = 0;
	mail1(&head, 0, 0);
	return(0);
}

/*
 * Mail a message on standard input to the people indicated
 * in the passed header.  (Internal interface).
 */

mail1(hp, savemsg, filename)
	struct header *hp;
	int savemsg;
	char *filename;
{
	register char *cp;
	int pid, i, s, p, gotcha;
	char **namelist, *deliver, *ccp;
	char ccpbuf[sizeof(myname)];
	struct name *to, *np;
	struct stat sbuf;
	FILE *mtf, *postage;
	int remote = rflag != NOSTR || rmail;
	char **t;
	char buf[BUFSIZ];
	char fold_dir[BUFSIZ];


	/*
	 * Collect user's mail from standard input.
	 * Get the result as mtf.
	 */

	pid = -1;
	if ((mtf = collect(hp)) == NULL)
		return(-1);
	hp->h_seq = 1;

	/*
	 * Now, take the user names from the combined
	 * to and cc lists and do all the alias
	 * processing.
	 */

	senderr = 0;
	to = usermap(cat(extract(hp->h_bcc, GBCC),
	    cat(extract(hp->h_to, GTO), extract(hp->h_cc, GCC))));
	if (to == NIL) {
		printf(MSGSTR(NORECIPS, "No recipients specified\n")); /*MSG*/
		goto topdog;
	}

	/*
	 * Look through the recipient list for names with /'s
	 * in them which we write to as files directly.
	 */

	to = outof(to, mtf, hp);
	rewind(mtf);
	to = verify(to);
	if (senderr && !remote) {
topdog:

		if (fsize(mtf) != 0) {
			remove_file(deadletter);
			exwrite(deadletter, mtf, 1);
			rewind(mtf);
		}
	}
	for (gotcha = 0, np = to; np != NIL; np = np->n_flink)
		if ((np->n_type & GDEL) == 0) {
			gotcha++;
			break;
		}
	if (!gotcha)
		goto out;
	to = elide(to);
	mechk(to);
	if (count(to) > 1)
		hp->h_seq++;
	if (hp->h_seq > 0 && !remote) {
		fixhead(hp, to);
		if (fsize(mtf) == 0)
		    if (hp->h_subject == NOSTR)
			printf(MSGSTR(NOINFO, "No message, no subject; hope that's ok\n")); /*MSG*/
		    else
			printf(MSGSTR(NOTEXT, "Null message body; hope that's ok\n")); /*MSG*/
		if ((mtf = infix(hp, mtf)) == NULL) {
			fprintf(stderr, MSGSTR(LOST, ". . . message lost, sorry.\n")); /*MSG*/
			return(-1);
		}
	}
	namelist = unpack(to);
	if (debug) {
		fprintf(stderr, "Recipients of message:\n");
		for (t = namelist; *t != NOSTR; t++)
			fprintf(stderr, " \"%s\"", *t);
		fprintf(stderr, "\n");
		fflush(stderr);
		return;
	}
	
	/*
	 * If followup was the original caller then save a copy of the reply
	 * message in a file named after the person being replied to.
	 */
	
	if (savemsg == TRUE) {
		sprintf(buf, "%s/%s", homedir, filename);
		cp = buf;
		savemail(expand(cp), hp, mtf);
	}
	 else 
		if ((ccp = value("record")) != NOSTR || file_recip == TRUE) {
			if (file_recip == TRUE)
				ccp = recip_name;

			/*
			 * Try to-list first for recipient name, then cc
			 * list. If all else fails, use myname.
			 */
			if (ccp == NULL) {
			    if (hp->h_to && *(hp->h_to)) {
				strcpy(ccpbuf, hp->h_to);
			    } else if (hp->h_cc && *(hp->h_cc)) {
				strcpy(ccpbuf, hp->h_cc);
			    } else {
				strcpy(ccpbuf, myname);
			    }
		/* not sure if all these delimeters are possible but ... */
			    ccp = strtok(ccpbuf, " ,\t\n");
			}
		
		/* 
		 * If the record environment variable contains a full
		 * pathname, then store the msg in there instead of
		 * in the user's home directory.
		 */

			if (ccp[0] == '/') 
				cp = ccp;
			else {
				if ((value("outfolder") != NOSTR) &&
				    (getfold(fold_dir) == 0))
					sprintf(buf, "%s/%s", fold_dir,ccp);
				else
					sprintf(buf, "%s/%s", homedir, ccp);
				cp = buf;
			}
			savemail(expand(cp), hp, mtf);
		}	
		
	/*
	 * Wait, to absorb a potential zombie, then
	 * fork, set up the temporary mail file as standard
	 * input for "mail" and exec with the user list we generated
	 * far above. Return the process id to caller in case he
	 * wants to await the completion of mail.
	 */

#ifdef	pdp11
	while (wait2(&s, WNOHANG) > 0)
		;
#endif
#ifdef AIX
	wait ((int *)0);
#else
	while (wait3(&s, WNOHANG, 0) > 0)
		;
#endif AIX
	rewind(mtf);
	pid = fork();
	if (pid == -1) {
		perror("fork");
		remove_file(deadletter);
		exwrite(deadletter, mtf, 1);
		goto out;
	}
	if (pid == 0) {
		sigchild();
		if (remote == 0) {
			signal(SIGTSTP, SIG_IGN);
			signal(SIGTTIN, SIG_IGN);
			signal(SIGTTOU, SIG_IGN);
		}
		for (i = SIGHUP; i <= SIGQUIT; i++)
			signal(i, SIG_IGN);
		if (!stat(POSTAGE, &sbuf))
			if ((postage = Fopen(POSTAGE, "a")) != NULL) {
				fprintf(postage, "%s %d %d\n", myname,
				    count(to), fsize(mtf));
				Fclose(postage);
			}
		s = fileno(mtf);
		for (i = 3; i < 15; i++)
			if (i != s)
				close(i);
		close(0);
		dup(s);
		close(s);
#ifdef CC
		submit(getpid());
#endif CC
#ifdef SENDMAIL
		deliver = getenv ("sendmail");
		if (deliver == NOSTR)
			deliver = getenv ("SENDMAIL");
		if (deliver == NOSTR)
			deliver = value("sendmail");
		if (deliver == NOSTR)
			deliver = SENDMAIL;
		execv (deliver, namelist);
		perror(deliver);
#endif SENDMAIL
		execv(MAIL, namelist);
		perror(MAIL);
		exit(1);
	}

out:
	if (remote || (value("verbose") != NOSTR) ||
	    (value("sendwait") != NOSTR)) {
		p = waitpid(pid, &s, 0);
		if (s != 0)
			senderr++;
		pid = 0;
	}
	Fclose(mtf);
	return(pid);
}

/*
 * Fix the header by glopping all of the expanded names from
 * the distribution list into the appropriate fields.
 * If there are any ARPA net recipients in the message,
 * we must insert commas, alas.
 */

fixhead(hp, tolist)
	struct header *hp;
	struct name *tolist;
{
	register struct name *nlist;
	register int f;
	register struct name *np;

	for (f = 0, np = tolist; np != NIL; np = np->n_flink)
		if (any('@', np->n_name)) {
			f |= GCOMMA;
			break;
		}

	if (debug && f & GCOMMA)
		fprintf(stderr, "Should be inserting commas in recip lists\n");
	hp->h_to = detract(tolist, GTO|f);
	hp->h_cc = detract(tolist, GCC|f);
}

/*
 * Prepend a header in front of the collected stuff
 * and return the new file.
 */

FILE *
infix(hp, fi)
	struct header *hp;
	FILE *fi;
{
	extern char tempMail[];
	register FILE *nfo, *nfi;
	register int c, ck;

	rewind(fi);
	if ((nfo = Fopen(tempMail, "w")) == NULL) {
		perror(tempMail);
		return(fi);
	}
	if ((nfi = Fopen(tempMail, "r")) == NULL) {
		perror(tempMail);
		Fclose(nfo);
		return(fi);
	}
	remove_file(tempMail);
	puthead(hp, nfo, GTO|GSUBJECT|GCC|GNL);
	c = getc(fi);
	while (c != EOF) {
		ck = putc(c, nfo);
		if (ck == EOF) {
		perror(tempMail);
		Fclose(nfo);
		Fclose(nfi);
		return(fi);
		}
		c = getc(fi);
	}
	if (ferror(fi)) {
		perror("read");
		return(fi);
	}
	fflush(nfo);
	if (ferror(nfo)) {
		perror(tempMail);
		Fclose(nfo);
		Fclose(nfi);
		return(fi);
	}
	Fclose(nfo);
	Fclose(fi);
	rewind(nfi);
	return(nfi);
}

/*
 * Dump the to, subject, cc header on the
 * passed file buffer.
 */

puthead(hp, fo, w)
	struct header *hp;
	FILE *fo;
{
	register int gotcha;

	gotcha = 0;
	if (hp->h_to != NOSTR && w & GTO)
		fmt("To: ", hp->h_to, fo), gotcha++;
	if (hp->h_subject != NOSTR && w & GSUBJECT)
		fprintf(fo, "Subject: %s\n", hp->h_subject), gotcha++;
	if (hp->h_cc != NOSTR && w & GCC)
		fmt("Cc: ", hp->h_cc, fo), gotcha++;
	if (hp->h_bcc != NOSTR && w & GBCC)
		fmt("Bcc: ", hp->h_bcc, fo), gotcha++;
	if (gotcha && w & GNL)
		putc('\n', fo);
	return(0);
}

/*
 * Format the given text to not exceed 72 characters.
 */

fmt(str, txt, fo)
	register char *str, *txt;
	register FILE *fo;
{
	register int col;
	register char *bg, *bl, *pt, ch;

	col = strlen(str);
	if (col)
		fprintf(fo, "%s", str);
	pt = bg = txt;
	bl = 0;
	while (*bg) {
		pt++;
		if (++col >72) {
			if (!bl) {
				bl = bg;
				while (*bl && !isspace(*bl))
					bl++;
			}
			if (!*bl)
				goto finish;
			ch = *bl;
			*bl = '\0';
			fprintf(fo, "%s\n    ", bg);
			col = 4;
			*bl = ch;
			pt = bg = ++bl;
			bl = 0;
		}
		if (!*pt) {
finish:
			fprintf(fo, "%s\n", bg);
			return;
		}
		if (isspace(*pt))
			bl = pt;
	}
}

/*
 * Save the outgoing mail on the passed file.
 */

savemail(name, hp, fi)
	char name[];
	struct header *hp;
	FILE *fi;
{
	register FILE *fo;
	register int i;
	time_t now;
	char buf[BUFSIZ];
	char *n;

	if ((fo = Fopen(name, "a")) == NULL) {
		perror(name);
		return(-1);
	}
	time(&now);
	n = rflag;
	if (n == NOSTR)
		n = myname;
	fprintf(fo, "From %s %s", n, ctime(&now));
	rewind(fi);
	while ((i = fread(buf, 1, sizeof buf, fi)) > 0)
		(void)fwrite(buf, 1, i, fo);
	(void)putc('\n', fo);
	fflush(fo);
	if (ferror(fo))
		perror(name);
	Fclose(fo);
	return(0);
}
