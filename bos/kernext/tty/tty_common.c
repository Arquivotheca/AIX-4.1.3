#ifndef lint
static char sccsid[] = "@(#)62 1.13 src/bos/kernext/tty/tty_common.c, sysxldterm, bos41J, 9508A 12/16/94 11:35:34";
#endif
/*
 * COMPONENT_NAME: (sysxtty)
 *
 * FUNCTIONS: ldtty_speedtab, ldtty_compatgetflags, ldtty_compatsetflags,
 *            ldtty_compatsetlflags, termios_to_sgttyb,
 *            sgttyb_to_termios, termios_to_tchars, tchars_to_termios,
 *            termios_to_ltchars, ltchars_to_termios, flags_to_termios
 *
 * ORIGINS: 71, 83
 *
 */
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
#include <unistd.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/str_tty.h>

#ifndef	VDISCARD
#define	VDISCARD	VDISCRD
#endif	/* VDISCARD */

/* symbolic sleep message strings */
const char ttyin[] = "ttyin";
const char ttyout[] = "ttyout";
const char ttopen[] = "ttyopn";
const char ttclos[] = "ttycls";
const char ttybg[] = "ttybg";
const char ttybuf[] = "ttybuf";

/*
 * Table giving parity for characters and indicating
 * character classes to tty driver. The 8th bit
 * indicates parity, the 7th bit indicates the character
 * is an alphameric or underscore (for ALTWERASE), and the
 * low 6 bits indicate delay type.  If the low 6 bits are 0
 * then the character needs no special processing on output.
 */

const char partab[] = {
	0001,0201,0201,0001,0201,0001,0001,0201,	/* nul - bel */
	0202,0004,0003,0205,0007,0206,0201,0001,	/* bs - si */
	0201,0001,0001,0201,0001,0201,0201,0001,	/* dle - etb */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* can - us */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* sp - ' */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* ( - / */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* 0 - 7 */
	0300,0100,0000,0200,0000,0200,0200,0000,	/* 8 - ? */
	0200,0100,0100,0300,0100,0300,0300,0100,	/* @ - G */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* H - O */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* P - W */
	0300,0100,0100,0200,0000,0200,0200,0300,	/* X - _ */
	0000,0300,0300,0100,0300,0100,0100,0300,	/* ` - g */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* h - o */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* p - w */
	0100,0300,0300,0000,0200,0000,0000,0201,	/* x - del */
	/*
	 * meta chars
	 */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* nul - bel */
	0202,0004,0003,0201,0005,0206,0201,0001,	/* bs - si */
	0201,0001,0001,0201,0001,0201,0201,0001,	/* dle - etb */
	0001,0201,0201,0001,0201,0001,0001,0201,	/* can - us */
	0200,0000,0000,0200,0000,0200,0200,0000,	/* sp - ' */
	0000,0200,0200,0000,0200,0000,0000,0200,	/* ( - / */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* 0 - 7 */
	0300,0100,0000,0200,0000,0200,0200,0000,	/* 8 - ? */
	0200,0100,0100,0300,0100,0300,0300,0100,	/* @ - G */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* H - O */
	0100,0300,0300,0100,0300,0100,0100,0300,	/* P - W */
	0300,0100,0100,0200,0000,0200,0200,0300,	/* X - _ */
	0000,0300,0300,0100,0300,0100,0100,0300,	/* ` - g */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* h - o */
	0300,0100,0100,0300,0100,0300,0300,0100,	/* p - w */
	0100,0300,0300,0000,0200,0000,0000,0200 	/* x - del */
};

/*
 * return the code for the speed
 */
int
ldtty_speedtab(speed, table)
	struct speedtab table[];
{
	register int i;

	for (i = 0; table[i].sp_speed != -1; i++)
		if (table[i].sp_speed == speed) {
			return(table[i].sp_code);
		}
	return(-1);
}

int
ldtty_compatgetflags(iflag, lflag, oflag, cflag, pflag)
	register tcflag_t iflag;
	register tcflag_t lflag;
	register tcflag_t oflag;
	register tcflag_t cflag;
	register int *pflag;
{
	register flags = 0;

	if (iflag&IXOFF)
		flags |= TANDEM;
	if (iflag&ICRNL || oflag&ONLCR)
		flags |= CRMOD;
	/* use pflag instead of flags to prevent clobbering NOFLUSH */
	if (cflag&PARENB) {
		if (iflag&INPCK) {
			if (cflag&PARODD) {
				if (*pflag < 0)
					flags |= ODDP;
				else *pflag |= ODDP;
			      }
			else {
				if (*pflag < 0) 
					flags |= EVENP;
				else *pflag |= EVENP;
			      }
		} else {
		  if (*pflag < 0)
			flags |= EVENP | ODDP;
		  else *pflag |= EVENP | ODDP;
		}
	} else {
		if (!(oflag&OPOST))
			flags |= LITOUT;
		
	}
	/* capture CSIZE info */
	if (*pflag < 0) {
	  if ((cflag&CSIZE) == CS8)
			flags |= PASS8;
	}
	else {
	  *pflag |= (cflag&CSIZE);
	}
              
		
	if ((lflag&ICANON) == 0) {
		/* fudge */
		if (iflag&IXON || lflag&ISIG || lflag&IEXTEN || cflag&PARENB)
			flags |= CBREAK;
		else
			flags |= RAW;
	}
	if (oflag&OXTABS)
		flags |= XTABS;
	if (lflag&ECHOE)
		flags |= CRTERA|CRTBS;
	if (lflag&ECHOKE)
		flags |= CRTKIL|CRTBS;
	if (lflag&ECHOPRT)
		flags |= PRTERA;
	if (lflag&ECHOCTL)
		flags |= CTLECH;
	if ((iflag&IXANY) == 0)
		flags |= DECCTQ;
	flags |= lflag&(ECHO|MDMBUF|TOSTOP|FLUSHO|NOHANG|PENDIN|NOFLSH);
	return(flags);
}

void
ldtty_compatsetflags(flags, iflagp, oflagp, lflagp, cflagp, minp, timep,
		     pflag)
	register	int	flags;
	tcflag_t	*iflagp;
	tcflag_t	*oflagp;
	tcflag_t	*lflagp;
	tcflag_t	*cflagp;
	cc_t	*minp;
	cc_t	*timep;
	register int pflag;
{
	register	tcflag_t	iflag = *iflagp;
	register	tcflag_t	oflag = *oflagp;
	register	tcflag_t	lflag = *lflagp;
	register	tcflag_t	cflag = *cflagp;

	if (flags & RAW) {
		iflag &= IXOFF;
		oflag &= ~OPOST;
		lflag &= ~(ECHOCTL|ISIG|ICANON|IEXTEN);
		*minp = 1;
		*timep = 0;
	} else {
		iflag |= BRKINT|IMAXBEL;
		oflag |= OPOST;
		lflag |= ISIG|IEXTEN;
		if (flags & XTABS)
			oflag |= OXTABS;
		else
                        oflag &= ~OXTABS;
                if (flags & CBREAK) {
                        lflag &= ~ICANON;
			/*
			 * Defect number 127761. In CBREAK mode only one
			 * character should be read at atime.
			 */
			*minp = 1;
			*timep = 0;
                } else {
                        lflag |= ICANON;

			/* NOTE:
			 * The following is VERY dependent on the fact
			 * that VMIN==VEOF and VTIME=VEOL.  If this
			 * ever changes, you'll have to change this
			 * code.
			 */
			*minp = CEOF;
			*timep = _POSIX_VDISABLE;
		}
                if (flags&CRMOD) {
                        iflag |= ICRNL;
                        oflag |= ONLCR;
                } else {
                        iflag &= ~ICRNL;
                        oflag &= ~ONLCR;
                }
        }
        if (flags&ECHO)
                lflag |= ECHO;
        else
                lflag &= ~ECHO;

        if (flags&(RAW|LITOUT)) {
                cflag &= ~(CSIZE|PARENB);
                cflag |= CS8;
                if ((flags&(RAW)) == 0)
                        iflag |= ISTRIP;
        } else {
                cflag &= ~CSIZE;
		cflag |= (pflag&CSIZE);
		if (pflag&(EVENP|ODDP))
			cflag |= PARENB;
        }
	
        if ((pflag&(EVENP|ODDP)) == EVENP) {
                iflag |= INPCK;
                cflag &= ~PARODD;
        } else if ((pflag&(EVENP|ODDP)) == ODDP) {
                iflag |= INPCK;
                cflag |= PARODD;
        } else
                iflag &= ~INPCK;
        if ((flags&LITOUT) && (cflag&PARENB))
                oflag &= ~OPOST;        /* move earlier ? */
        if (flags&TANDEM)
                iflag |= IXOFF;
        else
                iflag &= ~IXOFF;
	*iflagp = iflag;
	*oflagp = oflag;
	*lflagp = lflag;
	*cflagp = cflag;
}

void
ldtty_compatsetlflags(flags, iflagp, oflagp, lflagp, cflagp, pcsflag)
	register	flags;
	tcflag_t	*iflagp;
	tcflag_t	*oflagp;
	tcflag_t	*lflagp;
	tcflag_t	*cflagp;
	register        pcsflag;
{
        register tcflag_t	iflag = *iflagp;
        register tcflag_t	oflag = *oflagp;
        register tcflag_t	lflag = *lflagp;
        register tcflag_t	cflag = *cflagp;

        if (flags&CRTERA)
                lflag |= ECHOE;
        else
                lflag &= ~ECHOE;
        if (flags&CRTKIL)
                lflag |= ECHOKE;
        else
                lflag &= ~ECHOKE;
        if (flags&PRTERA)
                lflag |= ECHOPRT;
        else
                lflag &= ~ECHOPRT;
        if (flags&CTLECH)
                lflag |= ECHOCTL;
        else
                lflag &= ~ECHOCTL;
        if ((flags&DECCTQ) == 0)
                lflag |= IXANY;
        else
                lflag &= ~IXANY;
        lflag &= ~(ECHO|MDMBUF|TOSTOP|FLUSHO|NOHANG|PENDIN|NOFLSH);
        lflag |= flags&(ECHO|MDMBUF|TOSTOP|FLUSHO|NOHANG|PENDIN|NOFLSH);
        if (flags&(LITOUT)) {
                iflag &= ~ISTRIP;
                cflag &= ~(CSIZE|PARENB);
                cflag |= CS8;
                if (flags&LITOUT)
                        oflag &= ~OPOST;
                if ((flags&(RAW)) == 0)
                        iflag |= ISTRIP;
        } else if ((flags&RAW) == 0) {
                cflag &= ~CSIZE;
		cflag |= (pcsflag&CSIZE);
		if (pcsflag&(EVENP|ODDP))
			cflag |= PARENB;
                oflag |= OPOST;
        }
        *iflagp = iflag;
        *oflagp = oflag;
        *lflagp = lflag;
        *cflagp = cflag;
}

void
termios_to_sgttyb(termp, sg)
	struct	termios	*termp;
	struct	sgttyb	*sg;
{
        
	register u_char	*cc = termp->c_cc;
	register int	speed;
        int    pflag;  

	speed = cfgetospeed(termp);
	sg->sg_ospeed = (speed == -1) ? 15 : speed;
	if ((speed_t)cfgetispeed(termp) == 0)
		sg->sg_ispeed = sg->sg_ospeed;
	else {
		speed = cfgetispeed(termp);

		sg->sg_ispeed = (speed == -1) ? 15 : speed;
	}
	sg->sg_erase = cc[VERASE];
	sg->sg_kill = cc[VKILL];
	pflag = -1;  /* turn off new pflag handling */
	sg->sg_flags = ldtty_compatgetflags(termp->c_iflag, 
					    termp->c_lflag,
					    termp->c_oflag, 
					    termp->c_cflag, &pflag);
}

void
sgttyb_to_termios(sg, termp, flagp, pflag)
	struct	sgttyb	*sg;
	struct	termios	*termp;
	int	*flagp;
	int     pflag;
{
	int	speed;
	int	ask_for_speed;
	
	speed = sg->sg_ispeed;

	if ((speed <= 15) && (speed >= 0)) {
		cfsetispeed(termp, speed);
	} else {
		ask_for_speed = 0;
		cfsetispeed(termp, ask_for_speed);
	}
	speed = sg->sg_ospeed;
	termp->c_cc[VERASE] = sg->sg_erase;
	termp->c_cc[VKILL] = sg->sg_kill;
	*flagp = (*flagp & 0xffff0000) | sg->sg_flags;
	/*
	 * 128266 patch part 13 BEGIN
	 */
	ldtty_compatsetflags(*flagp, &termp->c_iflag, &termp->c_oflag,
				&termp->c_lflag, &termp->c_cflag,
				&termp->c_cc[VMIN], &termp->c_cc[VTIME],
			        pflag);
	/*
	 * 128266 patch part 13 END
	 */
	if ((speed <= 15) && (speed >= 0)) {
		cfsetospeed(termp, speed);
	} else {
		ask_for_speed = 0;
		cfsetospeed(termp, ask_for_speed);
	}
}

void
termios_to_tchars(termp, tc)
	struct	termios	*termp;
	struct	tchars	*tc;
{
	tc->t_intrc = termp->c_cc[VINTR];
	tc->t_quitc = termp->c_cc[VQUIT];
        tc->t_startc = termp->c_cc[VSTART];
        tc->t_stopc = termp->c_cc[VSTOP];
        tc->t_eofc = termp->c_cc[VEOF];
        tc->t_brkc = termp->c_cc[VEOL];
}

void
tchars_to_termios(tc, termp)
	struct	tchars	*tc;
	struct	termios	*termp;
{
	termp->c_cc[VINTR] = tc->t_intrc;
        termp->c_cc[VQUIT] = tc->t_quitc;
        termp->c_cc[VSTART] = tc->t_startc;
        termp->c_cc[VSTOP] = tc->t_stopc;
        termp->c_cc[VEOF] = tc->t_eofc;
        termp->c_cc[VEOL] = tc->t_brkc;
        if (tc->t_brkc == -1)
                termp->c_cc[VEOL2] = ((cc_t)_POSIX_VDISABLE);
}

void
termios_to_ltchars(termp, ltc)
	struct	termios	*termp;
	struct	ltchars	*ltc;
{
        ltc->t_suspc = termp->c_cc[VSUSP];
        ltc->t_dsuspc = termp->c_cc[VDSUSP];
        ltc->t_rprntc = termp->c_cc[VREPRINT];
        ltc->t_flushc = termp->c_cc[VDISCARD];
        ltc->t_werasc = termp->c_cc[VWERSE];
        ltc->t_lnextc = termp->c_cc[VLNEXT];
}

void
ltchars_to_termios(ltc, termp)
	struct	ltchars	*ltc;
	struct	termios	*termp;
{
        termp->c_cc[VSUSP] = ltc->t_suspc;
        termp->c_cc[VDSUSP] = ltc->t_dsuspc;
        termp->c_cc[VREPRINT] = ltc->t_rprntc;
        termp->c_cc[VDISCARD] = ltc->t_flushc;
        termp->c_cc[VWERSE] = ltc->t_werasc;
        termp->c_cc[VLNEXT] = ltc->t_lnextc;
}

void
flags_to_termios(cmd, newflags, termp, flagp)
	int	cmd;
	int	newflags;
	struct	termios	*termp;
	int	*flagp;
{
  	int     pcsflag = 0;
        if (cmd == TIOCLSET) {
                *flagp = (*flagp & 0xffff) | newflags << 16;
	}
        else {
	  int ret = ldtty_compatgetflags(termp->c_iflag, termp->c_lflag,
					 termp->c_oflag, termp->c_cflag,
					 &pcsflag);

	  /* *flagp = (ret & 0xffff0000) | (*flagp & 0xffff); */
	  *flagp = (ret) ;
	  if (cmd == TIOCLBIS)
		  *flagp |= newflags << 16;
	  else
		  *flagp &= ~(newflags << 16);
        }
        ldtty_compatsetlflags(*flagp, &termp->c_iflag, &termp->c_oflag,
                          &termp->c_lflag, &termp->c_cflag, pcsflag);
}
