static char sccsid[] = "src/bos/diag/tu/370pca/hardware.c, tu_370pca, bos411, 9428A410j 8/8/91 15:28:52";
#ifdef AIX
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "catuser.h"
#endif

#include "pscatu.h"
#include "psca.h"
#include "extern.h"

#ifdef MTEX
#include "pscadefs.h"
extern STATAREA far		*statarea;
extern BYTE far			*cardarea;
extern unsigned int		numints;	    /* number of interrupts */
extern unsigned int		cardslot;
#endif

read1_sram(addr, val)
SRAM		addr;
BYTE		*val;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = 1;
	cmd.str_ptr = val;
	cmd.rd_wrt = CAT_CHNL_RD;
	cmd.dma_mode = CAT_USE_PIO;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	if (cmd.status) {
		sprintf(MSGBUF, "read_sram error: %d retries at %08lx\n",
			cmd.status, addr);
		return(TU_HARD);
	}
	return(TU_GOOD);
#endif
#ifdef MTEX
	*val = *(cardarea + addr);
	return(TU_GOOD);
#endif
}


read_sram(addr, buf, length)
SRAM		addr;
BYTE		*buf;
int		length;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = length;
	cmd.str_ptr = buf;
	cmd.rd_wrt = CAT_CHNL_RD;
	cmd.dma_mode = CAT_USE_PIO;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	if (cmd.status) {
		sprintf(MSGBUF, "read1_sram error: %d retries at %08lx\n",
			cmd.status, addr);
		return(TU_HARD);
	}
	return(TU_GOOD);
#endif
#ifdef MTEX
	mcopy(cardarea + addr, (BYTE far *) buf, length);
	return(TU_GOOD);
#endif
}

read_sram_dma(addr, buf, length)
SRAM		addr;
BYTE		*buf;
int		length;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = length;
	cmd.str_ptr = buf;
	cmd.rd_wrt = CAT_CHNL_RD;
	cmd.dma_mode = CAT_USE_DMA;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	return(TU_GOOD);
#endif
#ifdef MTEX
	sprintf(MSGBUF,"MTEX defined, called read_sram_dma. ILLEGAL CALL.\n");
	return(TU_HARD);
#endif
}


write1_sram(addr, data)
SRAM		addr;
int		data;
{
#ifdef AIX
	struct cat_rw_sram	cmd;
	BYTE		val;

	cmd.sram_offset = addr;
	cmd.num_bytes = 1;
	cmd.rd_wrt = CAT_CHNL_WR;
	cmd.dma_mode = CAT_USE_PIO;
	val = data;
	cmd.str_ptr = &val;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	if (cmd.status) {
		sprintf(MSGBUF, "write1_sram error: %d retries at %08lx\n",
			cmd.status, addr);
		return(TU_HARD);
	}
	return(TU_GOOD);
#endif
#ifdef MTEX
	*(addr + cardarea) = data;
	return(TU_GOOD);
#endif
}


write_sram(addr, buf, length)
SRAM		addr;
BYTE		*buf;
int		length;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = length;
	cmd.str_ptr = buf;
	cmd.rd_wrt = CAT_CHNL_WR;
	cmd.dma_mode = CAT_USE_PIO;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	if (cmd.status) {
		sprintf(MSGBUF, "write_sram error: %d retries at %08lx\n",
			cmd.status, addr);
		return(TU_HARD);
	}
	return(TU_GOOD);
#endif
#ifdef MTEX
	mcopy((BYTE far *) buf, cardarea + addr, length);
	return(TU_GOOD);
#endif
}

write_sram_dma(addr, buf, length)
SRAM		addr;
BYTE		*buf;
int		length;
{
#ifdef AIX
	struct cat_rw_sram	cmd;

	cmd.sram_offset = addr;
	cmd.num_bytes = length;
	cmd.str_ptr = buf;
	cmd.rd_wrt = CAT_CHNL_WR;
	cmd.dma_mode = CAT_USE_DMA;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	return(TU_GOOD);
#endif
#ifdef MTEX
	sprintf(MSGBUF,"MTEX defined, called write_sram_dma. ILLEGAL CALL.\n");
	return(TU_HARD);
#endif
}


/*************************************************************************
* This routine waits for the contents of "addr" to become "data".  The	 *
* ioctls are done inside this routine to prevent repeatedly setting up	 *
* the same rw_cmd structure.						 *
*************************************************************************/
wait_sram(addr, data, secs)
SRAM		addr;
int		data;
int		secs;
{
#ifdef AIX
	struct cat_rw_sram	cmd;
	BYTE		buf;

	cmd.sram_offset = addr;
	cmd.num_bytes = 1;
	cmd.str_ptr = &buf;
	cmd.rd_wrt = CAT_CHNL_RD;
	cmd.dma_mode = CAT_USE_PIO;
	if (ioctl(device, CAT_RW_SRAM, &cmd)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}

	while (secs--) {
		if (buf == data)
			break;
		sleep(1);
		if (ioctl(device, CAT_RW_SRAM, &cmd)) {
			sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
				sys_errlist[errno]);
			return(TU_SYS_ERRNO);
		}
	}

	if (buf == data)
		return(TU_GOOD);
	else
		return(TU_HARD);
#endif
#ifdef MTEX
	extern unsigned short	funcbox;

	while (secs--) {
		if (*(cardarea + addr) == data)
			break;
		mtsleep(100);
		if (mtcmsg(funcbox)) {
			printf("Terminating tests, keyboard request\n");
			break;
		}
	}

	if (*(cardarea + addr) == data)
		return(TU_GOOD);
	else
		return(TU_HARD);
#endif
}
/*************************************************************************
* This routine waits for the contents of "addr" to become "data".	 *
* It is meant to wait for POST specifically, checking the POST status	 *
* every	 10 milliseconds (every 1 second for AIX) and printing out the	 *
* time & status every time it changes.					 *
*************************************************************************/
wait_post(addr, data, secs)
SRAM		addr;
int		data;
int		secs;
{
#ifdef AIX
	struct cat_rw_sram	cmd1;
	struct cat_rw_sram	cmd2;
	BYTE			buf;
	BIT16			stat, savestat;
	int			count;

	cmd1.sram_offset	= addr;
	cmd1.num_bytes		= 1;
	cmd1.str_ptr		= (unsigned char *) &buf;
	cmd1.rd_wrt		= CAT_CHNL_RD;
	cmd1.dma_mode		= CAT_USE_PIO;

	cmd2.sram_offset	= ERRCODE;
	cmd2.num_bytes		= sizeof(stat);
	cmd2.str_ptr		= (unsigned char *) &stat;
	cmd2.rd_wrt		= CAT_CHNL_RD;
	cmd2.dma_mode		= CAT_USE_PIO;

	if (ioctl(device, CAT_RW_SRAM, &cmd2)) {
		sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	letni16(&stat);

	savestat = stat;
	sprintf(MSGBUF, "%04x started at ~time: %d secs\n", savestat, 0 );
	RPT_VERBOSE(MSGBUF);
	for (count = 0; count < secs; count++) {
		if (ioctl(device, CAT_RW_SRAM, &cmd1)) {
			sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
				sys_errlist[errno]);
			return(TU_SYS_ERRNO);
		}
		if (buf == data) {
			sprintf(MSGBUF,"POST finished at ~time: %d secs\n", count);
			RPT_VERBOSE(MSGBUF);
			break;
			}
		if (ioctl(device, CAT_RW_SRAM, &cmd2)) {
			sprintf(MSGBUF, "ioctl CAT_RW_SRAM failed: %s\n",
				sys_errlist[errno]);
			return(TU_SYS_ERRNO);
		}
		letni16(&stat);
		if (stat != savestat) {
			sprintf(MSGBUF,"%04x started at ~time: %d secs\n", stat, count);
			RPT_VERBOSE(MSGBUF);
			savestat = stat;
			}
		sleep(1);

	}

	if (buf == data)
		return(TU_GOOD);
	else
		return(TU_HARD);
#endif
#ifdef MTEX
int millisecs, stat, savestat;


       secs *= 1000;		   /* convert to millisecs */
       stat = savestat = statarea->errdgblk[2];
       sprintf(MSGBUF, "%04x started at ~time: %d ms\n", stat, 0 );
       RPT_VERBOSE(MSGBUF);

       for (millisecs = 0; millisecs < secs; millisecs += 10) {
	 if (*(cardarea + addr) == data) {
	    sprintf(MSGBUF,"POST finished at ~time: %d ms\n", millisecs);
	    RPT_VERBOSE(MSGBUF);
	    break;
	    }
	 if ((stat = statarea->errdgblk[2]) != savestat)
	   {
	    sprintf(MSGBUF,"%04x started at ~time: %d ms\n", stat, millisecs);
	    RPT_VERBOSE(MSGBUF);
	    savestat = stat;
	    }
	 mtsleep(1);		 /* sleep for 10 milliseconds */
	 }

	if (*(cardarea + addr) == data)
		return(TU_GOOD);
	else
		return(TU_HARD);
#endif
}


/*************************************************************************
* Call the device driver to write val to POS register num.		 *
* Return values are							 *
*	1	Any type of error (including system type errors)	 *
*	0	No error occurred					 *
* Dependencies:	 cardslot is assumed to be set to a valid card on entry *
*************************************************************************/
write_pos_reg(num, val)
int		num;			/* The POS register number (0 - 7) */
int		val;			/* The value to write (0x00 - 0xff) */
{
#ifdef AIX
	struct cat_pos_acc	cmd;	/* The structure for the ioctl cmd */

	cmd.opcode = CAT_CHNL_WR;
	cmd.pos_reg = num;
	cmd.pos_val = val;
	if (ioctl(device, CAT_POS_ACC, &cmd) != 0) {
		sprintf(MSGBUF, "ioctl CAT_POS_ACC failed: %s\n",
			sys_errlist[errno]);
		return(1);
	}
	if (cmd.status != CIO_OK) {
		return(1);
	}

	return(0);
#endif
#ifdef MTEX
	mtlock();
	outportb(0x96, cardslot | 8);
	outportb(num + 0x100, val);
	outportb(0x96, 0x00);
	mtunlock();
#endif
}


/*************************************************************************
* Call the device driver to read from POS register num.			 *
* Return values are							 *
*	1	Any type of error (including system type errors)	 *
*	0	No error occurred					 *
*************************************************************************/
read_pos_reg(num, val)
int		num;			/* The POS register number (0 - 7) */
int		*val;			/* The value read (0x00 - 0xff) */
{
#ifdef AIX
	struct cat_pos_acc	cmd;	/* The structure for the ioctl cmd */

	cmd.opcode = CAT_CHNL_RD;
	cmd.pos_reg = num;
	if (ioctl(device, CAT_POS_ACC, &cmd) != 0) {
		sprintf(MSGBUF, "ioctl CAT_POS_ACC failed: %s\n",
			sys_errlist[errno]);
		return(1);
	}
	if (cmd.status != CIO_OK) {
		return(1);
	}

	*val = cmd.pos_val;
	return(0);
#endif
#ifdef MTEX
	mtlock();			/* prevent others from running */
	outportb(0x96, cardslot | 8);	/* select adapter */
	*val = inportb(num + 0x100);	/* get value from POS register */
	outportb(0x96, 0x00);		/* de-select slot */
	mtunlock();
#endif
}

/*************************************************************************
* dnld_code() - load 186 program to shared RAM				 *
*************************************************************************/
dnld_code(filename)
char		*filename;
{
	int		tmp;
	void		*voidptr;
#ifdef AIX
	int		fildes;
	struct stat	stat_buf;
	struct cat_dnld	cmd;

/*************************************************************************
* Reset the adapter to prepare for downloading microcode.            	 *
*************************************************************************/
	if (tmp=ioctl(device, CAT_RESET_ADAP, voidptr )) {
		sprintf(MSGBUF,
			"Error resetting adapter: RC=%d ERRNO=%d %s\n",tmp,errno,sys_errlist[errno]);
		return(TU_SYS);
	}
/*************************************************************************
* Open the file containing the microcode.				 *
*************************************************************************/
	if ((fildes = open(filename, O_RDONLY)) < 0) {
		sprintf(MSGBUF, "Could not open %s: %s\n", filename, sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
/*************************************************************************
* Do a stat to get the length of the file.				 *
*************************************************************************/
	if (fstat(fildes, &stat_buf)) {
		sprintf(MSGBUF, "Could not fstat %s: %s\n", filename, sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
	cmd.mcode_len = stat_buf.st_size;
/*************************************************************************
* Get enough memory to read the whole file in.				 *
*************************************************************************/
	cmd.mcode_ptr = (BYTE *) malloc(cmd.mcode_len);
	if (cmd.mcode_ptr == NULL) {
		sprintf(MSGBUF, "Could not malloc %ld bytes\n", (long) cmd.mcode_len);
		return(TU_SYS);
	}
/*************************************************************************
* Read the file into the new memory area.				 *
*************************************************************************/
	if (read(fildes, cmd.mcode_ptr, cmd.mcode_len) != cmd.mcode_len) {
		(void) sprintf(MSGBUF, "Could not read %s\n", filename);
		return(TU_SYS);
	}
	CHECK(close(fildes));
	if (VERBOSE) {
		sprintf(MSGBUF, "Downloading %s (%ld bytes)\n", filename,
			(long) cmd.mcode_len);
		RPT_VERBOSE(MSGBUF);
	}
/*************************************************************************
* Download the code to the board using the CAT_DNLD command of ioctl.	 *
*************************************************************************/
	ucode = UNKNOWN;
	if (tmp=ioctl(device, CAT_DNLD, &cmd)) {
		sprintf(MSGBUF,
			"Error downloading code: RC=%d ERRNO=%d %s\n",tmp,errno,sys_errlist[errno]);
		return(TU_SYS);
	}
	free(cmd.mcode_ptr);			/* return memory to system */
#endif /* AIX */
#ifdef MTEX
	int file, len, rc;

	ROF(reset_board());
	ucode = UNKNOWN;
	file = open(filename);
	if (file < 0) {
		sprintf(MSGBUF, "%s: could not open, code = %x\n", filename, file);
		return(TU_SYS_ERRNO);
	}
	if (wait_sram(OPERLVL, DL_WAITING, 30)) {
		sprintf(MSGBUF, "Timed out(30 sec) waiting to begin download\n");
		return(TU_HARD);
	}
	len = read(file, (BYTE far *) &cardarea[DATABUFS], DATASZ);
	close(file);

	if (VERBOSE) {
		sprintf(MSGBUF, "Downloading %s (%ld bytes)\n", filename, (long) len);
		RPT_VERBOSE(MSGBUF);
	}
	ROF(write1_sram(READY, 0x00));
	ROF(write1_sram(OPERLVL, DL_CMD));
#ifdef POST_WITH_CRC
	if (wait_sram(OPERLVL, DL_CONFIRM, 10)) {
		sprintf(MSGBUF, "Timed out waiting for DL_CONFIRM\n");
		return(TU_HARD);
	}

#endif
#endif
/*************************************************************************
* Wait for the diagnostic microcode to say "I'm here and waiting"	 *
*************************************************************************/
	if (wait_sram(READY, DIAG_WAITING, 10)) {
		read1_sram(READY,&tmp);
		sprintf(MSGBUF, "Microcode in wrong state (%s)\n"
		"Diagnostic Microcode never started running\n",
			ready_string(tmp));
		return(TU_HARD);
	}
	return(TU_GOOD);
}


set_debug(new_level)
int		new_level;
{
#ifdef	AIX
	if (ioctl(device, CAT_DEBUG, &new_level)) {
		sprintf(MSGBUF, "ioctl CAT_DEBUG failed: %s\n",
			sys_errlist[errno]);
		return(TU_SYS_ERRNO);
	}
#endif
}
