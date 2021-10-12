static char sccsid[] = "@(#)tu_fifo.c	1.2 2/1/91 14:30:13";
#include <stdio.h>
#include "psca.h"
#include "pscatu.h"
#include "hwfifo.h"
#include "extern.h"


/*************************************************************************
* Hardware FIFO test (from MCI)						 *
* Must have diagnostic microcode running				 *
*									 *
* The hardware implements two FIFOs available to the host.  One I call	 *
* the Send Free Buffer FIFO (SFBF), and one I called the Receive Free	 *
* Buffer FIFO (RFBF).  Each FIFO can be full (F), all reserved (R),	 *
* empty (E).  Performing the PART2 diagnostic command causes the 186 to	 *
* report back its status in RECEIVED and copy as much as possible from	 *
* RFBF to SFBF.	 All reserved means empty for SFBF and full for RFBF.	 *
*									 *
* Confirm the correct operation of the Hardware FIFO's by performing	 *
* the following steps:							 *
*	initialize FIFO's						 *
*	confirm my status: SFBF=E | R, RFBF=0   check MCI		 *
*	execute PART2 command (xfer 0)		store OBP, xfer 0	 *
*	confirm 186 status: SFBF=E | R, RFBF=E  check OBP, check MCI	 *
*	reserve 1 to RFBF						 *
*	output 1 to RFBF						 *
*	confirm my status: SFBF=E | R, RFBF=0   check MCI		 *
*	execute PART2 command (xfer 1)					 *
*	confirm 186 status: SFBF=E | R, RFBF=0				 *
*	reserve FIFO_SIZE to RFBF					 *
*	confirm my status: SFBF=0, RFBF=R				 *
*	output FIFO_SIZE to RFBF					 *
*	confirm my status: SFBF=0, RFBF=F | R				 *
*	execute PART2 command (xfer FIFO_SIZE-1)			 *
*	confirm 186 status: SFBF=0, RFBF=F | R				 *
*	execute PART2 command (xfer 0)					 *
*	confirm 186 status: SFBF=F, RFBF=0				 *
*	reserve FIFO_SIZE from SFBF					 *
*	confirm my status: SFBF=R, RFBF=0				 *
*	remove FIFO_SIZE from SFBF					 *
*	reserve and output FIFO_SIZE-1 to RFBF				 *
*	execute PART2 command (xfer FIFO_SIZE)				 *
*	reserve and remove FIFO_SIZE from SFBF				 *
*	confirm my status: SFBF=E | R, RFBF=0				 *
*	execute PART2 command (xfer 0)					 *
*	confirm 186 status: SFBF=E | R, RFBF=E				 *
*************************************************************************/
/*************************************************************************
* I'm gonna repeat some things quite a bit so define them as macros.	 *
*************************************************************************/
/*
 * CAUTION: status from 186 not valid unless status from MCI is read first.
 * Therefore if we haven't done a check_mci_fifo_status we do a read of
 * FIFO_STAT before we run PART2.
 */
#define RUN_FIFO_PART2								\
	{ if (run_cmd(TU_NUM | CMD_PART2, 2)) {					\
		sprintf(MSGBUF, "TU-%02d  %s -- FAILED to copy FIFO data.\n",	\
			TU_NUM, TU_NAME);					\
		ucode = UNKNOWN;						\
		return(TU_HARD); }						\
	ROF(read_sram(RECEIVED, &fifo_status, sizeof(fifo_status))); }

#define CONFIRM_186(val)							\
	{ letni16(&fifo_status);		/* swap bytes if needed ??? */	\
	if ((fifo_status & 0x3f) != (val)) {	  /* compare valid bits */	  \
		sprintf(MSGBUF,							\
			"FIFO Status (186) bad: expected %02x, received %02x\n",\
			(val), fifo_status);					\
		return(TU_SOFT); } }

tu_fifo_mci(cb)
TUTYPE		*cb;
{
#ifdef MTEX
	char		buf[10];
	int		rc;
	int		loop = 1;
	BIT32		fifo_status;
	BIT32		data_out = 1;
	BIT32		data_in = 1;

	while(1) {
		ROF(read_sram(FIFO_STAT, &fifo_status, sizeof(fifo_status)));
		letni32(&fifo_status);
		printf("1 - run part 1,	 2 - run part 2\n");
		printf("R - RFBF reserve, P - RFBF put\n");
		printf("S - SFBF reserve, G - SFBF get\n");
		printf("T - run tu,	 Q - quit\n");
		printf("L - loop next command\n\n");
		printf("status = %02x  ---> ", fifo_status & 0xff);

		locgets(buf, 10);
		switch (buf[0]) {
		case 'L':
			printf("Number of repetitions--> ");
			locgets(buf, 10);
			loop = atoi(buf);
			break;
		case '1':
			if (run_cmd(TU_NUM, 2)) {
				sprintf(MSGBUF, "TU-%02d  %s -- FAILED to initialize FIFOs.\n",
				    TU_NUM, TU_NAME);
				ucode = UNKNOWN;
				return(TU_HARD);
			}
			loop = 1;
			break;
		case '2':
			RUN_FIFO_PART2;
			loop = 1;
			break;
		case 'R':
			while (loop-- > 0 && (rc = rfbf_reserve()) == 0)
				;
			if (rc)
				printf("%d left, returned %d\n",loop,rc);
			loop = 1;
			break;
		case 'P':
			while (loop-- > 0 && (rc = rfbf_out(data_out++)) == 0)
				;
			if (rc)
				printf("%d left, returned %d\n",loop,rc);
			loop = 1;
			break;
		case 'S':
			while (loop-- > 0 && (rc = sfbf_reserve()) == 0)
				;
			if (rc)
				printf("%d left, returned %d\n",loop,rc);
			loop = 1;
			break;
		case 'G':
			while (loop-- > 0 && (rc = sfbf_remove(data_in++)) == 0)
				;
			if (rc)
				printf("%d left, returned %d\n",loop,rc);
			loop = 1;
			break;
		case 'T':
			while (loop-- > 0 && (rc = tu_fifo_mci2(cb)) == 0)
				;
			if (rc) {
				printf("Message: %s\n", MSGBUF);
				printf("%d left, returned %d\n",loop,rc);
			}
			loop = 1;
			break;
		case 'Q':
			return(0);
		default:
			printf("Did nothing\n");
			loop = 1;
		}
	}
}


tu_fifo_mci2(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
#endif
	BIT32		sta;			/* dummy for MCI status */
	BIT16		fifo_status;		/* status from 186 side */
	int		i;
	BIT32		data_out = 1;
	BIT32		data_in = 1;
	int		rc;			/* used by ROF macro */
	extern int	run_cmd();		/* pass cmd to diag ucode */

#ifdef CARD_FIFO
	read_sram(FIFO_STAT, &sta, sizeof(sta));
	if (run_cmd(TU_NUM, 2)) {
		sprintf(MSGBUF, "TU-%02d  %s -- FAILED to initialize FIFOs.\n",
		    TU_NUM, TU_NAME);
		ucode = UNKNOWN;
		return(TU_HARD);
	}
	/*
	 * RFBF contains 0, additional reserved 0
	 * SFBF contains 0, of those reserved 0
	 */
	RPT_VERBOSE("after part 1 initialize\n");
	ROF(check_mci_fifo_status(MCI_EMPTY | MCI_REMPTY));
	RUN_FIFO_PART2;
	RPT_VERBOSE("after first part 2 of ucode\n");
	CONFIRM_186(OBP_SF_EMPTY | OBP_SF_RESV | OBP_RF_EMPTY);
	ROF(check_mci_fifo_status(MCI_EMPTY | MCI_REMPTY));
	RPT_VERBOSE("passed first check mci fifo status after part 2\n");
	ROF(rfbf_reserve());
	/*
	 * RFBF contains 0, additional reserved 1
	 * SFBF contains 0, of those reserved 0
	 */
	ROF(check_mci_fifo_status(MCI_EMPTY | MCI_REMPTY));
	RPT_VERBOSE("after reserve but before out passed check mci fifo stat\n");
	ROF(rfbf_out(data_out++));
	/*
	 * RFBF contains 1, additional reserved 0
	 * SFBF contains 0, of those reserved 0
	 */
	ROF(check_mci_fifo_status(MCI_EMPTY | MCI_REMPTY));
	RUN_FIFO_PART2;
	RPT_VERBOSE("after 2nd part 2 of ucode\n");
	CONFIRM_186(OBP_SF_EMPTY | OBP_SF_RESV);
	/*
	 * RFBF contains 0, additional reserved 0
	 * SFBF contains 1, of those reserved 0
	 */
	for (i=0; i<FIFO_SIZE; i++)
	{
#ifdef REAL_VERBOSE
		if (VERBOSE) {
			sprintf(MSGBUF,"reserving RFBF %d\n",i);
			RPT_VERBOSE(MSGBUF);
		}
#endif
		ROF(rfbf_reserve());
	}
	/*
	 * RFBF contains 0, additional reserved FIFO_SIZE
	 * SFBF contains 1, of those reserved 0
	 */
	ROF(check_mci_fifo_status(MCI_RFULL));
	for (i=0; i<FIFO_SIZE; i++)
		ROF(rfbf_out(data_out++));
	/*
	 * RFBF contains FIFO_SIZE, additional reserved 0
	 * SFBF contains 1, of those reserved 0
	 */
	ROF(check_mci_fifo_status(MCI_RFULL | MCI_FULL));
	RUN_FIFO_PART2;
	RPT_VERBOSE("after 3rd part 2 of ucode\n");
	CONFIRM_186(OBP_RF_FULL | OBP_RF_RESV);
	/*
	 * RFBF contains 1, additional reserved 0
	 * SFBF contains FIFO_SIZE, of those reserved 0
	 */
	/*
	 * Clock those funky latches!!!
	 */
	ROF(read_sram(FIFO_STAT, &sta, sizeof(sta)));
	RUN_FIFO_PART2;
	RPT_VERBOSE("after 4th part 2 of ucode\n");
	CONFIRM_186(OBP_SF_FULL);
	ROF(check_mci_fifo_status(0));
	for (i=0; i<FIFO_SIZE; i++)
	{
#ifdef REAL_VERBOSE
		if (VERBOSE) {
			sprintf(MSGBUF,"reserving SFBF %d\n",i);
			RPT_VERBOSE(MSGBUF);
		}
#endif
		ROF(sfbf_reserve());
	}
	/*
	 * RFBF contains 1, additional reserved 0
	 * SFBF contains FIFO_SIZE, of those reserved FIFO_SIZE
	 */
	RPT_VERBOSE("sfbf reserve loop done\n");
	ROF(check_mci_fifo_status(MCI_REMPTY));
	for (i=0; i<FIFO_SIZE; i++) {
		ROF(sfbf_remove(data_in++));
#ifdef REAL_VERBOSE
		if (VERBOSE) {
			sprintf(MSGBUF,"sfbf data removed was %d\n",(data_in-1));
			RPT_VERBOSE(MSGBUF);
		}
#endif
	}
	/*
	 * RFBF contains 1, additional reserved 0
	 * SFBF contains 0, of those reserved 0
	 */
	ROF(check_mci_fifo_status(MCI_REMPTY|MCI_EMPTY));
	RPT_VERBOSE("status good after removal of all data from sfbf\n");
	for (i=0; i<FIFO_SIZE-1; i++) {
		ROF(rfbf_reserve());
		ROF(rfbf_out(data_out++));
	}
	/*
	 * RFBF contains FIFO_SIZE, additional reserved 0
	 * SFBF contains 0, of those reserved 0
	 */
	RUN_FIFO_PART2;
	RPT_VERBOSE("after 5th part 2 of ucode\n");
	/*
	 * RFBF contains 0, additional reserved 0
	 * SFBF contains FIFO_SIZE, of those reserved 0
	 */
	for (i=0; i<FIFO_SIZE; i++) {
		ROF(sfbf_reserve());
		ROF(sfbf_remove(data_in++));
	}
	/*
	 * RFBF contains 0, additional reserved 0
	 * SFBF contains 0, of those reserved 0
	 */
	ROF(check_mci_fifo_status(MCI_REMPTY | MCI_EMPTY));
	RUN_FIFO_PART2;
	RPT_VERBOSE("after 6th part 2 of ucode\n");
	CONFIRM_186(OBP_SF_EMPTY | OBP_SF_RESV | OBP_RF_EMPTY);
#else
	RPT_VERBOSE("Not compiled to test Hardware FIFO's");
#endif
	return(TU_GOOD);
}


check_mci_fifo_status(val)
int		val;
{
	BIT32		status;			/* only need LSB */
	int		rc;			/* used by ROF macro */

	ROF(read_sram(FIFO_STAT, &status, sizeof(status)));
	letni32(&status);
	if ((status & 0x55) == val)		  /* only compare the real bits */
		return(TU_GOOD);
	sprintf(MSGBUF, "FIFO Status (MCI) bad: expected %02x, received %02x\n",
		val, status & 0xff);
	return(TU_SOFT);
}


rfbf_reserve()
{
	BIT32		status;			/* only need LSB */
	int		rc;			/* used by ROF macro */

	ROF(read_sram(RFBF_STAT, &status, sizeof(status)));
	letni32(&status);
	if (status & MCI_RFULL) {
		sprintf(MSGBUF, "RFBF Status (MCI) has all reserved\n");
		return(TU_SOFT);
	} else
		return(TU_GOOD);
}


sfbf_reserve()
{
	BIT32		status;			/* only need LSB */
	int		rc;			/* used by ROF macro */

	ROF(read_sram(SFBF_STAT, &status, sizeof(status)));
	letni32(&status);
	if (status & MCI_REMPTY) {
		sprintf(MSGBUF, "SFBF Status (MCI) has all reserved\n");
		return(TU_SOFT);
	} else
		return(TU_GOOD);
}


sfbf_remove(data)
BIT32		data;
{
	BIT32		data_in;
	int		rc;			/* used by ROF macro */

	ROF(read_sram(SFBF_DATA, (char *) &data_in, sizeof(data_in)));
	if (data == data_in)
		return(TU_GOOD);
	sprintf(MSGBUF, "Bad data from FIFO: expected %02lx, received %02lx\n",
		data, data_in);
	return(TU_SOFT);
}


rfbf_out(data)
BIT32		data;
{
	int		rc;			/* used by ROF macro */

	ROF(write_sram(RFBF_DATA, (char *) &data, sizeof(data)));
	return(TU_GOOD);
}
