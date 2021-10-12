static char sccsid[] = "@(#)castart.c	1.4 6/14/91 14:45:31";
#include <stdio.h>
#include <fcntl.h>
#include "catuser.h"
#include "psca.h"
#include "pscatu.h"
#include "extern.h"


/*
 * Prepare the functional microcode by performing the following commands:
 *	set adapter
 *	Download command decode table number 1
 *	Download command decode table number 2
 */
func_prep(cb)
TUTYPE		*cb;			/* control block for necessary info */
{
	int			rc;	/* used by Return-On-Failure macro */
	ROF(setadap(device,cb->pr.chanspeed,cb->pr.bufsize,
		cb->pr.numxmit,cb->pr.numrec, cb->pr.subchan));
	ROF(dl_table(device, cb->pr.cutbl1, 1));	/* MUST use 1! */
	ROF(dl_table(device, cb->pr.cutbl2, 2));	/* MUST use 2! */
/* moved startsub to beginning of tu_func, because must be done after reopening device in func mode */
	/* ROF(startsub(device, cb->pr.subchan)); */
	return(0);
}


int
setadap(fd, speed, bufsiz, num_xmit, num_rcv, subchan)
int				fd;
int				speed;		/* default channel speed */
int				bufsiz;		/* default x/r buf size */
int				num_xmit;	/* number of xmit buffers */
int				num_rcv;	/* number of recv buffers */
int				subchan;	/* subchannel address to be used */
{
	struct cat_set_adap	cmd;
	struct cat_adapcfg	*cfg;
	int			i;

	i = sizeof(struct cat_adapcfg);
	if ((cfg = (struct cat_adapcfg *) malloc(i)) == NULL) {
		sprintf(MSGBUF,"setadap: out of memory\n");
		return(-1);
	}

	cmd.num_sub	= 1;
	cmd.adap_param	= ( struct cat_adapcfg * ) cfg;

	cfg->speed	= speed;
	cfg->reserve[0]	= cfg->reserve[1] = cfg->reserve[2] = 0;
	cfg->xmitsz	= bufsiz;
	cfg->recvsz	= bufsiz;
	cfg->xmitno	= num_xmit;
	cfg->recvno	= num_rcv;
	cfg->flags	= 0;
	cfg->nosubs	= 1;

		cfg->subid[0].subchan	= subchan;
		cfg->subid[0].cutype[0]	= 1;
		cfg->subid[0].cutype[1]	= 2;

	if (ioctl(fd, CAT_SET_ADAP, &cmd)) {
		sprintf(MSGBUF,"CAT_SET_ADAP: %s\n",sys_errlist[errno]);
		return(1);
	}
	return(0);
}


int
dl_table(fd, fname, type)
int				fd;
char				*fname;
unsigned char			type;
{
	struct cat_cu_load	cmd;
	int			tbl_fd;		/* cu table file descriptor */

	cmd.cu_type = type;
	cmd.overwrite = 1;
	if ((tbl_fd = open(fname, O_RDONLY)) < 0) {
		sprintf(MSGBUF,"%s: %s\n",fname,sys_errlist[errno]);
		return(-1);
	}
	if ((cmd.length = lseek(tbl_fd, 0L, 2)) == -1) {
		sprintf(MSGBUF,"%s: %s\n",fname,sys_errlist[errno]);
		close(tbl_fd);
		return(-1);
	}
	if (lseek(tbl_fd, 0L, 0) == -1) {
		sprintf(MSGBUF,"%s: %s\n",fname,sys_errlist[errno]);
		close(tbl_fd);
		return(-1);
	}
	if ((cmd.cu_addr = malloc(cmd.length)) == NULL) {
		sprintf(MSGBUF,"castart.c: %s\n",sys_errlist[errno]);
		close(tbl_fd);
		return(-1);
	}
	if (read(tbl_fd, cmd.cu_addr, cmd.length) == -1) {
		sprintf(MSGBUF,"%s: %s\n",fname,sys_errlist[errno]);
		free(cmd.cu_addr);
		close(tbl_fd);
		return(-1);
	}
	close(tbl_fd);

	if (ioctl(fd, CAT_CU_LOAD, &cmd) == -1) {
		sprintf(MSGBUF,"CAT_CU_LOAD: %s\n",sys_errlist[errno]);
		free(cmd.cu_addr);
		return(-1);
	}
	free(cmd.cu_addr);
	return(0);
}


startsub(fd, chan)
int				fd;
int				chan;
{
	struct cat_set_sub	startsub;

	startsub.sb.netid	= chan;
	startsub.subset		= 1;
	startsub.set_default	= 0;
	startsub.specmode	= 0;	/* use the "special mode" */
	startsub.shrtbusy	= 0x03;	/* use short busy */
	startsub.startde	= 0;	/* start w/ unsolicited dev end */
	if (ioctl(fd, CIO_START, &startsub) == -1) {
		sprintf(MSGBUF,"CIO_START: %s\n",sys_errlist[errno]);
		return(-1);
	}
	return(0);
}
