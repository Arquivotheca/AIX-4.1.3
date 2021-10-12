static char sccsid[] = "src/bos/diag/tu/370pca/misc.c, tu_370pca, bos411, 9428A410j 8/8/91 15:29:10";
#include <stdio.h>


#include "pscatu.h"
#include "psca.h"
#include "extern.h"

#ifdef MTEX
#include "pscadefs.h"
#endif

/*************************************************************************
* This file contains various support routines.	They are mostly the	 *
* smaller pieces needed to support the TU's.				 *
*************************************************************************/


/*************************************************************************
* Compare the expected value of a POS register to its actual value. The	 *
* mask specifies which bits HAVE expected values.			 *
*************************************************************************/
valcheck_pos(num, mask, val)
int		num;
unsigned int	mask;
unsigned int	val;
{
	unsigned int	got;

	if (read_pos_reg(num, &got)) {
		sprintf(MSGBUF, "VAL POS: Couldn't read register %d\n", num);
		return(TU_HARD);
	}
	if ((got & mask) != (val & mask)) {
		sprintf(MSGBUF, "VAL POS: register %d value: expected=%02x, got=%02x\n",
			num, val & mask, got & mask);
		return(TU_SOFT);
	}
	return(TU_GOOD);
}


/*************************************************************************
* Test the specified POS register by writing a 0xAA and 0x55 pattern to	 *
* those bits that are expected to be read/writable (without harm).	 *
* Error checking galore ...						 *
*************************************************************************/
rwtest_pos_reg(num, mask)
int		num;				/* register number to test */
unsigned int	mask;				/* mask of bits expected to be R/W-able */
{
	unsigned int	original;		/* original value of the reg */
	unsigned int	unused;			/* value of unused bits in reg */
	unsigned int	got;			/* value read after write */

	if (read_pos_reg(num, &original)) {
		sprintf(MSGBUF, "RW POS: Couldn't read register %d\n", num);
		return(TU_HARD);
	}
	unused = original & ~mask;
	if (write_pos_reg(num, unused | (0xAA & mask))) {	/* half on, half off */
		write_pos_reg(num, original);		/* make best effort to put back */
		sprintf(MSGBUF, "RW POS: Couldn't write register %d\n", num);
		return(TU_HARD);
	}
	if (read_pos_reg(num, &got)) {
		write_pos_reg(num, original);		/* make best effort to put back */
		sprintf(MSGBUF, "RW POS: Couldn't read register %d (after write)\n", num);
		return(TU_HARD);
	}
	if (got != (unused | (0xAA & mask))) {
		write_pos_reg(num, original);		/* make best effort to put back */
		sprintf(MSGBUF, "RW POS: register %d: expected=%02x, got=%02x\n",
			num, unused | (0xAA & mask), got);
		return(TU_SOFT);
	}
	if (write_pos_reg(num, unused | (0x55 & mask))) {	/* half on, half off */
		write_pos_reg(num, original);		/* make best effort to put back */
		sprintf(MSGBUF, "RW POS2: Couldn't write register %d\n", num);
		return(TU_HARD);
	}
	if (read_pos_reg(num, &got)) {
		write_pos_reg(num, original);		/* make best effort to put back */
		sprintf(MSGBUF, "RW POS: Couldn't read register %d (after write)\n", num);
		return(TU_HARD);
	}
	if (got != (unused | (0x55 & mask))) {
		write_pos_reg(num, original);		/* make best effort to put back */
		sprintf(MSGBUF, "RW POS: register %d: expected=%02x, got=%02x\n",
			num, unused | (0x55 & mask), got);
		return(TU_SOFT);
	}
	if (write_pos_reg(num, original)) {
		sprintf(MSGBUF, "RW POS: Couldn't restore register %d\n", num);
		return(TU_HARD);
	}
	return(TU_GOOD);
}


/*************************************************************************
* Read the VPD.	 Compare the Description to what is expected and check	 *
* the CRC on all VPD data.						 *
*************************************************************************/
static char	*desc_table[] = {
	" PSCA. S/370 Parallel Channel adapter.",	/* temporary ???? */
	"PSCA. S/370 Parallel Channel adapter.",
	"PCA. S/370 Parallel Channel adapter.",
        "SYSTEM/370 HOST INTERFACE ADAPTER",
        " SYSTEM/370 HOST INTERFACE ADAPTER",
	NULL
};

test_vpd()
{
	BYTE		vpd[VPD_SIZE];
	BYTE		*desc;
	int		rc;			/* used by Return On Failure macro */
	BYTE		*get_from_vpd();

	ROF(read_vpd(vpd));			/* read vpd and check CRC */
	if ((desc = get_from_vpd(vpd, "DS")) == NULL) {
		sprintf(MSGBUF, "VPD: Description String not found\n");
		return(TU_SOFT);
	}
	if (table_cmp(desc_table, desc) < 0) {
		sprintf(MSGBUF, "VPD: Description String wrong: \"%s\"\n", desc);
		return(TU_SOFT);
	}
	if (VERBOSE) {
		sprintf(MSGBUF, "VPD: Description String = \"%s\"\n", desc);
		RPT_VERBOSE(MSGBUF);
	}
	return(TU_GOOD);
}


/*************************************************************************
* Read the VPD by twiddling POS registers 3 and 6 then check the CRC	 *
* Vital Product Data is formatted as follows (see the Specification for	 *
* Microchannel Adapter VPD, Appendix B):				 *
*	Byte 0		inaccessible (this byte counts in VPD_SIZE)	 *
*	Byte 1-3	"VPD"						 *
*	Byte 4, 5	length of VPD after 8 byte header in 2 byte	 *
*			words (MSB, LSB)				 *
*	Byte 6, 7	CRC of remaining bytes (MSB, LSB)		 *
*	Byte n		Delimiter ("*")					 *
*	Byte n+1, n+2	Keyword, 2 characters				 *
*	Byte n+3	L, length of data field in 2 byte words		 *
*	Byte n+4 ...	data field					 *
*	Byte n+2*L	Delimiter ("*")					 *
*	...								 *
*************************************************************************/
read_vpd(buf)
BYTE		*buf;
{
	int		i;
	unsigned int	tmp;			/* safer to put it here */
	unsigned short	crc_gen();

	for (i=1; i<VPD_SIZE; i++) {
		if (write_pos_reg(6, i)) {	/* set addr to map reg 3 to */
			sprintf(MSGBUF, "VPD: Couldn't write POS register 6\n");
			return(TU_HARD);
		}
		if (read_pos_reg(3, &tmp)) {	/* read one byte of VPD */
			sprintf(MSGBUF, "VPD: Couldn't read POS register 3\n");
			return(TU_HARD);
		}
		buf[i] = tmp;
	}

	if (VERBOSE)
		dump_vpd(buf);

	if (strncmp(buf+1, "VPD", 3) != MATCH) {
		sprintf(MSGBUF, "VPD: Expected \"VPD\"\tRead 0x%02x 0x%02x 0x%02x\n",
			buf[1], buf[2], buf[3]);
		return(TU_SOFT);
	}
/*************************************************************************
* Generate a crc of the VPD data (start at byte8, length of 2 * byte5)	 *
* Compare to the crc stored at bytes 6-7 (MSB, LSB)			 *
*************************************************************************/
	if ((tmp = crc_gen(buf+8, buf[5]*2)) != combine(buf[6], buf[7])) {
		sprintf(MSGBUF, "VPD: Bad CRC: Calculated %04x\tRead %04x\n",
			tmp, combine(buf[6], buf[7]));
		return(TU_SOFT);
	}
	return(TU_GOOD);
}


dump_vpd(buf)
BYTE		buf[];
{
	int		i;
	int		tmp;
	int		out;
	char		*s;

	RPT_VERBOSE("Vital Product Data Dump\n");
	sprintf(MSGBUF,"CRC: Calculated = %04x, Retrieved = %04x\n",
		crc_gen(buf+8, buf[5]*2), combine(buf[6], buf[7]));
	RPT_VERBOSE(MSGBUF);
	s = MSGBUF;
	ADD_SPRINTF(s, sprintf(s, "   "));
	for (i=1; i<VPD_SIZE; i++) {
		ADD_SPRINTF(s,sprintf(s,"%02x ", buf[i]));
		if ((i & 0x0f) == 0xf) {
			ADD_SPRINTF(s,sprintf(s,"\n"));
			RPT_VERBOSE(MSGBUF);
			s = MSGBUF;
		}
	}
	return(TU_GOOD);
}

/*************************************************************************
* Search the vpd for a specified keyword.  Copy the data field to a	 *
* static buffer and return the address of the buffer.  Return NULL if	 *
* keyword not found.							 *
*************************************************************************/
BYTE *
get_from_vpd(vpd, kw)
BYTE		*vpd;		/* VPD in above described format */
BYTE		*kw;		/* two character keyword to find */
{
	static BYTE	buf[512];	/* since length is 2*L must be < 508 */
	int		length;			/* length of the current data field */
	int		vpd_len;		/* remaining length of vpd */

	vpd_len = 2 * combine(vpd[4], vpd[5]);
	vpd += 8;					/* skip the header */
	while (vpd_len > 0) {
		if (*vpd != '*')			/* check for delimiter */
			return((BYTE *) NULL);	/* really an error */
		length = 2 * vpd[3];			/* get data field length */
		if (strncmp(vpd+1, kw, 2) == MATCH) {	/* does keyword match? */
			strncpy(buf, vpd+4, length-4);	/* BUG?: will stop on a null */
			buf[length-4]=NULL;              /* make sure string has null terminator */
			return(buf);
		}
		vpd += length;				/* skip field and keep looking */
		vpd_len -= length;
	}
	return((BYTE *) NULL);
}


start_timeout()
{
	int		tmp;

	read1_sram(READY,&tmp);
	(void) sprintf(MSGBUF, "Microcode in wrong state (%s): Diagnostic Microcode not ready to run test\n", ready_string(tmp));
	RPT_ERR(0, MSGBUF);
}


end_timeout(additional)
int		additional;		/* retrieve additional info */
{
	int		tmp;
	struct errdbgblk results;

	read1_sram(READY,&tmp);
	switch(tmp) {
	case DIAG_RUNNING:
		strcpy(MSGBUF, "Mircocode in wrong state (DIAG_RUNNING):\nTest Unit started but never completed\n");
		if (additional) {
			read_sram(ERRDBGBLK, &results, sizeof(results));
			mem_results(&results, MSGBUF+strlen(MSGBUF));
		}
		break;
	default:
		(void) sprintf(MSGBUF, "Mircocode in wrong state (%s):\nTest Unit never completed\n", ready_string(tmp));
	}
}


char *
operlvl_string(x)
int		x;
{
	switch(x) {
	case DL_WAITING:	return("DL_WAITING");
	case DL_CMD:		return("DL_CMD");
	case DL_CONFIRM:	return("DL_CONFIRM");
	default:		return("Unknown state!");
	}
}


char *
ready_string(x)
int		x;
{
	switch(x) {
	case DIAG_WAITING:	return("DIAG_WAITING");
	case DIAG_CMD:		return("DIAG_CMD");
	case DIAG_RUNNING:	return("DIAG_RUNNING");
	default:		return("Unknown state!");
	}
}


mem_results(res, str)
struct errdbgblk *res;
char		*str;
{
	ADD_SPRINTF(str, sprintf(str,"Error Block:\n"));
	ADD_SPRINTF(str, sprintf(str,"Errorcode = %04x\n",res->errcode));
	ADD_SPRINTF(str, sprintf(str, "Testing location 0x%04x:%04x (0x%08x)\n",
		res->badsegm, res->badoffs, res->badsegm*16 + res->badoffs));
	ADD_SPRINTF(str, sprintf(str, "Expected 0x%04x	  Received 0x%04x\n",
		res->expected, res->received));
}


/*************************************************************************
* In most cases the TU routines are actually what I call "handles" to a	 *
* Test Unit which resides in microcode and gets executed on the I/O	 *
* board itself.	 In this case, the "handle" must request that the board	 *
* perform the test, wait for it to complete, fetch the results and	 *
* report them back to the main application (i.e. HTX).	This operation	 *
* is done in conformance to the document entitled "Protocol for		 *
* Communicating with the PSCA Diagnostic Microcode" written by Paul	 *
* Chamberlain.	This routine handles that task.				 *
*									 *
* Return values:							 *
*	0	The test was performed					 *
*	1	For some reason the test was not performed		 *
*		or did not complete					 *
*************************************************************************/
run_cmd(cmd, secs)
BIT16		cmd;			/* Diagnostic cmd, i.e.	 TU010 */
int		secs;			/* max # of secs test can run */
{
	int		rc;			/* used by ROF macro */
/*************************************************************************
* Wait for the diagnostic microcode to say "I'm here and waiting"	 *
*************************************************************************/
	ROF(wait_sram(READY, DIAG_WAITING, 2));
/*************************************************************************
* Give the board a command to execute a Test Unit			 *
*************************************************************************/
	letni16(&cmd);
	ROF(write_sram(ERRLOG, &cmd, sizeof(cmd)));
	ROF(write1_sram(READY, DIAG_CMD));
/*************************************************************************
* Wait for the board to complete the Test Unit				 *
*************************************************************************/
	ROF(wait_sram(READY, DIAG_WAITING, secs));
	return(TU_GOOD);
}
/************************************************************************
* run a TU without checking for DL_WAITING first.			*
*************************************************************************/
force_cmd(cmd, secs)
BIT16		cmd;			/* Diagnostic cmd, i.e.	 TU010 */
int		secs;			/* max # of secs test can run */
{
	int		rc;			/* used by ROF macro */
/*************************************************************************
* Give the board a command to execute a Test Unit			 *
*************************************************************************/
	ROF(write_sram(ERRLOG, &cmd, sizeof(cmd)));
	ROF(write1_sram(READY, DIAG_CMD));
/*************************************************************************
* Wait for the board to complete the Test Unit				 *
*************************************************************************/
	ROF(wait_sram(READY, DIAG_WAITING, secs));
	return(TU_GOOD);
}


/*************************************************************************
* Swap bytes (in a word) for that pesky intel processor			 *
*************************************************************************/
letni16(data_in)
BIT16		*data_in;
{
#ifndef NOSWAP
/*************************************************************************
* I hope this is more efficient than it looks...			 *
* I could have done it like this:					 *
*	*data_in = ((*data_in) >> 8) | ((*data_in &0xff) << 8);		 *
*************************************************************************/
	BYTE		char_tmp;
	union {
		struct {
			BYTE		byte1;
			BYTE		byte2;
		}		bytes;
		BIT16		data;
	}		tmp;

	tmp.data = *data_in;
	char_tmp = tmp.bytes.byte1;
	tmp.bytes.byte1 = tmp.bytes.byte2;
	tmp.bytes.byte2 = char_tmp;
	*data_in = tmp.data;
#endif
}


/*************************************************************************
* Swap bytes in both words then swap words in the double word for that	 *
* pesky intel processor							 *
*************************************************************************/
letni32(data_in)
BIT32		*data_in;
{
#ifndef NOSWAP
	BIT16	word_tmp;
	union {
		struct {
			BIT16		word1;
			BIT16		word2;
		}		words;
		BIT32		data;
	}		tmp;

	tmp.data = *data_in;
	letni16(&tmp.words.word1);
	letni16(&tmp.words.word2);
/*************************************************************************
* I hope this is more efficient than it looks...			 *
* I could have done it like this:					 *
*	*data_in = (word2 << 16) | word1;				 *
*************************************************************************/
	word_tmp = tmp.words.word1;
	tmp.words.word1 = tmp.words.word2;
	tmp.words.word2 = word_tmp;
	*data_in = tmp.data;
#endif
}


/*************************************************************************
* Change some bits of a POS register.  Change the bits indicated in mask *
* to the values indicated in val.  Note that (val & ~mask) is assumed to *
* be 0.	 i.e. There are no bits set in val that are not set in mask.	 *
*************************************************************************/
change_pos_reg(num, mask, val)
int		num;
int		mask;
int		val;
{
	int		original;	/* original value of the reg */
	int		unused;		/* value of unused bits in reg */
	int		got;		/* value read after write */

	if (read_pos_reg(num, &original)) {
		sprintf(MSGBUF, "Couldn't read POS register %d\n", num);
		return(TU_HARD);
	}
	unused = original & ~mask;			/* keep bits not in mask */
	if (write_pos_reg(num, unused | val)) {		/* or in requested val */
		sprintf(MSGBUF, "Couldn't write POS register %d\n", num);
		return(TU_HARD);
	}
	return(TU_GOOD);
}


post_results(res, str)
struct errdbgblk *res;
char		*str;
{
	switch(res->errcode & 0xff00) {
	case 0x0000:
		ADD_SPRINTF(str, sprintf(str, "POST errcode = 0x00nn?\n"));
		break;
	case 0x0100:
		ADD_SPRINTF(str, sprintf(str, "POST Processor test FAILED\n"));
		break;
	case 0x0200:
		ADD_SPRINTF(str, sprintf(str, "POST Checksum test FAILED\n"));
		break;
	case 0x0300:
		ADD_SPRINTF(str, sprintf(str, "POST SRAM info area test FAILED\n"));
		break;
	case 0x0400:
		ADD_SPRINTF(str, sprintf(str, "POST SRAM test FAILED\n"));
		break;
	case 0x0500:
		ADD_SPRINTF(str, sprintf(str, "POST DRAM test FAILED\n"));
		break;
	case 0x0600:
		ADD_SPRINTF(str, sprintf(str, "POST DRAM<=>SRAM test FAILED\n"));
		break;
	case 0x0700:
		ADD_SPRINTF(str, sprintf(str, "POST Hardware FIFO test FAILED\n"));
		break;
	case 0x0800:
		ADD_SPRINTF(str, sprintf(str, "POST Internal Hardware Register test FAILED\n"));
		break;
	case 0x0900:
		ADD_SPRINTF(str, sprintf(str, "POST R/W from State Machine test FAILED\n"));
		break;
	case 0x0a00:
		ADD_SPRINTF(str, sprintf(str, "POST Parity Detection test FAILED\n"));
		break;
	case 0x0b00:
		ADD_SPRINTF(str, sprintf(str, "POST DMA Allocation register test FAILED\n"));
		break;
	case 0x0c00:
		ADD_SPRINTF(str, sprintf(str, "POST Internal Channel Wrap test FAILED\n"));
		break;
	case 0xfe00:
		ADD_SPRINTF(str, sprintf(str, "POST Illegal Interrupt\n"));
		break;
	default:
		ADD_SPRINTF(str, sprintf(str, "POST ** UNKNOWN TEST ** FAILED\n"));
	}
	ADD_SPRINTF(str, sprintf(str, "Complete Error Code: 0x%04x\n", res->errcode));
	ADD_SPRINTF(str, sprintf(str, "Testing location	    0x%04x:%04x (0x%08x)\n",
		res->badsegm, res->badoffs, res->badsegm*16 + res->badoffs));
	ADD_SPRINTF(str, sprintf(str, "Expected: 0x%04x	    Received: 0x%04x\n",
		res->expected, res->received));
}


mem_err(str, addr, expected, received)
char		*str;
SRAM		addr;
BIT16		expected;
BIT16		received;
{
	ADD_SPRINTF(str, sprintf(str, "Testing location	    (0x%08x)\n",addr));
	ADD_SPRINTF(str, sprintf(str, "Expected:   0x%02x     Received:	  0x%02x\n",
		expected, received));
}


unsigned int
ma_pattern(addr)
SRAM		addr;
{
	union {
		SRAM		long_val;
		struct {
			BYTE		byte1;
			BYTE		byte2;
			BYTE		byte3;
			BYTE		byte4;
		}		bytes;
	}		data;

	data.long_val = addr;
	return(data.bytes.byte1 ^ data.bytes.byte2 ^
		data.bytes.byte3 ^ data.bytes.byte4);
}


read_dbgblk(res)
struct errdbgblk	*res;
{
	int		rc;			/* used by ROF macro */

	ROF(read_sram(ERRDBGBLK, res, sizeof(*res)));	/* get results of POST */
	letni16(&res->testnumber);
	letni16(&res->errlog);
	letni16(&res->errcode);
	letni16(&res->badoffs);
	letni16(&res->badsegm);
	letni16(&res->expected);
	letni16(&res->received);
	return(TU_GOOD);
}


reset_board()
{
	int		rc;			/* used by ROF macro */

	if (VERBOSE)
		RPT_VERBOSE("Resetting board\n");
	ROF(change_pos_reg(4, 0x04, 0x04));		/* reset OBP */
	ROF(write1_sram(OPERLVL, 0));
	ROF(write1_sram(READY, 0));
	ROF(change_pos_reg(4, 0x04, 0x00));		/* let it run again */
	return(TU_GOOD);
}


/*************************************************************************
* Change the base address of the card to the range-th address range. In	 *
* AIX all you do is write the POS register and the device driver notices *
* the change and updates the base address.  Under MTEX we change the	 *
* base address manually by calling base_addr().				 *
*************************************************************************/
change_base_addr(range)
int		range;
{
#ifdef CARD_OLD_BASE				/* old used bits 5-7 */
		change_pos_reg(2, 0xe0, range << 5);
#else						/* new uses bits 4-7 */
		change_pos_reg(2, 0xf0, range << 4);
#endif
#ifdef MTEX
		base_addr(range);		/* manually update base addr */
#endif
}


#define MAXERRORS	20
mem_check(addr, expect, receive, epilog)
long		addr;
BYTE		expect;
BYTE		receive;
char		*epilog;
{
	int		this_time;
	static int	last_time;
	static long	bad_addr;
	static BYTE	first_expect;
	static BYTE	first_receive;
	char		*s = MSGBUF;
	int		num_error = 0;

	if (addr == 0)	{			 /* assume the memory test restarted */
		num_error = 0;
		if (last_time) {
			ADD_SPRINTF(s, sprintf(s,
				"Errors: 0x%08x to end.\n", bad_addr));
			ADD_SPRINTF(s, sprintf(s,
				"Expected: 0x%02x Received: 0x%02x\n",
				first_expect, first_receive));
			ADD_SPRINTF(s, sprintf("%s\n",epilog));
			RPT_SOFT(0, MSGBUF);
			last_time = 0;
		}
	}
	this_time = (expect != receive);
	if (this_time != last_time)
		if (last_time) {
			if (num_error++ < MAXERRORS)  {
				ADD_SPRINTF(s, sprintf(s,
					"Errors: 0x%08x to 0x%08x. \n",
					bad_addr, addr-1));
				ADD_SPRINTF(s, sprintf(s,
					"Expected: 0x%02x Received: 0x%02x\n",
					first_expect, first_receive));
				ADD_SPRINTF(s, sprintf("%s\n",epilog));
				RPT_SOFT(0, MSGBUF);
			}
			last_time = this_time;
		} else {
			bad_addr = addr;
			first_expect = expect;
			first_receive = receive;
			last_time = this_time;
		}
	return(this_time);
}
