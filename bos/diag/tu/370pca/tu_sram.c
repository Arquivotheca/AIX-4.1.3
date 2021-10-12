static char sccsid[] = "@(#)tu_sram.c	1.4 2/1/91 14:32:11";
#include <stdio.h>
#include "psca.h"
#include "pscatu.h"
#include "extern.h"

#ifdef MTEX
#include "pscadefs.h"
#endif


/*************************************************************************
* Test the Shared RAM (Static RAM) from the Microchannel host		 *
*									 *
* Fill and verify all of SRAM with 0x55, 0xaa, and a Modified		 *
* Address pattern.							 *
*									 *
* The possible return values are:					 *
*	TU_GOOD		Everything went fine				 *
*	TU_SYS_ERRNO	A system error occured and errno applies	 *
*	TU_SYS		A system error occured and errno does not apply	 *
*	TU_HARD								 *
*	TU_SOFT		A memory failure was detected			 *
* MWW 891017 modified to test with 0's at end to clear SRAM.		 *
* MWW 891131 holds reset on when testing SRAM, releases it when exit	 *
*	     NOTE:  POST will run when reset is released!!		 *
*************************************************************************/
#define BLOCKSIZE	1024
#define NUMBLOCKS	(SRAMSIZE/BLOCKSIZE)
#define NUMLEFT		(SRAMSIZE % BLOCKSIZE)
tu_srammci(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	int		error = 0;
	int		rc;
	int		blocknum, byte;
	BYTE		block[BLOCKSIZE];
	unsigned int	ma_pattern();
	char		epilog[80];

	sprintf(epilog,"tu = %d, loop = %d \n",TU_NUM,cb->header.loop);
	ROF(change_pos_reg(4, 0x04, 0x04));		/* reset OBP */
	ucode = UNKNOWN;

/*************************************************************************
* Fill and verify SRAM with 0x55 pattern				 *
*************************************************************************/
	for (byte=0; byte<BLOCKSIZE; byte++)
		block[byte] = 0x55;

	for (blocknum=0; blocknum<NUMBLOCKS; blocknum++)
		ROF(write_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
	ROF(write_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));

        for (blocknum=0; blocknum<NUMBLOCKS && max_errors>0; blocknum++) {
		ROF(read_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
                for (byte=0; byte<BLOCKSIZE && max_errors>0; byte++)
			error |= mem_check((long) blocknum*BLOCKSIZE+byte,
					0x55, block[byte],epilog);
	}
	ROF(read_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));
        for (byte=0; byte<NUMLEFT && max_errors>0; byte++)
		error |= mem_check((long) NUMBLOCKS*BLOCKSIZE+byte,
			0x55, block[byte],epilog);
	if (VERBOSE) {
		sprintf(MSGBUF,"%ld bytes tested with 0x55\n",
			(long) NUMBLOCKS*BLOCKSIZE+NUMLEFT);
		RPT_VERBOSE(MSGBUF);
	}
	if (error) {
		sprintf(MSGBUF, "SRAM test failed\n");
		ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
		return(TU_SOFT);
	}

/*************************************************************************
* Fill and verify SRAM with 0xAA pattern				 *
*************************************************************************/
	for (byte=0; byte<BLOCKSIZE; byte++)
		block[byte] = 0xaa;

	for (blocknum=0; blocknum<NUMBLOCKS; blocknum++)
		ROF(write_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
	ROF(write_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));

        for (blocknum=0; blocknum<NUMBLOCKS && max_errors>0; blocknum++) {
		ROF(read_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
                for (byte=0; byte<BLOCKSIZE && max_errors>0; byte++)
			error |= mem_check((long) blocknum*BLOCKSIZE+byte,
					0xaa, block[byte],epilog);
	}
	ROF(read_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));
        for (byte=0; byte<NUMLEFT && max_errors>0; byte++)
		error |= mem_check((long) NUMBLOCKS*BLOCKSIZE+byte,
			0xaa, block[byte],epilog);
	if (VERBOSE) {
		sprintf(MSGBUF,"%ld bytes tested with 0xAA\n",
			(long) NUMBLOCKS*BLOCKSIZE+NUMLEFT);
		RPT_VERBOSE(MSGBUF);
	}
	if (error) {
		sprintf(MSGBUF, "SRAM test failed\n");
		ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
		return(TU_SOFT);
	}

/*************************************************************************
* Fill and verify with modified address pattern				 *
*************************************************************************/
	for (blocknum=0; blocknum<NUMBLOCKS; blocknum++) {
		for (byte=0; byte<BLOCKSIZE; byte++)
			block[byte] = ma_pattern((SRAM) blocknum*BLOCKSIZE+byte);
		ROF(write_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
	}
	for (byte=0; byte<NUMLEFT; byte++)
		block[byte] = ma_pattern((SRAM) blocknum*BLOCKSIZE+byte);
	ROF(write_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));

        for (blocknum=0; blocknum<NUMBLOCKS && max_errors>0; blocknum++) {
		ROF(read_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
                for (byte=0; byte<BLOCKSIZE && max_errors>0; byte++)
			error |= mem_check((long) blocknum*BLOCKSIZE+byte,
				ma_pattern((SRAM) blocknum*BLOCKSIZE+byte), block[byte],epilog);
	}
	ROF(read_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));
        for (byte=0; byte<NUMLEFT && max_errors>0; byte++)
		error |= mem_check((long) NUMBLOCKS*BLOCKSIZE+byte,
			ma_pattern((SRAM) NUMBLOCKS*BLOCKSIZE+byte), block[byte],epilog);
	if (VERBOSE) {
		sprintf(MSGBUF,"%ld bytes tested with Modified Address pattern\n",
			(long) NUMBLOCKS*BLOCKSIZE+NUMLEFT);
		RPT_VERBOSE(MSGBUF);
	}
	if (error) {
		sprintf(MSGBUF, "SRAM test failed\n");
		ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
		return(TU_SOFT);
	}

/*************************************************************************
* Fill and verify SRAM with 0x00 pattern - also clear mem at end	 *
*************************************************************************/
	for (byte=0; byte<BLOCKSIZE; byte++)
		block[byte] = 0x00;

	for (blocknum=0; blocknum<NUMBLOCKS; blocknum++)
		ROF(write_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
	ROF(write_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));

        for (blocknum=0; blocknum<NUMBLOCKS && max_errors>0; blocknum++) {
		ROF(read_sram((SRAM) blocknum*BLOCKSIZE, block, BLOCKSIZE));
                for (byte=0; byte<BLOCKSIZE && max_errors>0; byte++)
			error |= mem_check((long) blocknum*BLOCKSIZE+byte,
					0x00, block[byte],epilog);
	}
	ROF(read_sram((SRAM) NUMBLOCKS*BLOCKSIZE, block, NUMLEFT));
        for (byte=0; byte<NUMLEFT && max_errors>0; byte++)
		error |= mem_check((long) NUMBLOCKS*BLOCKSIZE+byte,
			0x00, block[byte],epilog);
	mem_check((long) 0, 0, 0, epilog);		/* finish reporting errors */
	if (VERBOSE) {
		sprintf(MSGBUF,"%ld bytes tested with 0x00\n",
			(long) NUMBLOCKS*BLOCKSIZE+NUMLEFT);
		RPT_VERBOSE(MSGBUF);
	}
	if (error) {
		sprintf(MSGBUF, "SRAM test failed\n");
		ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
		return(TU_SOFT);
	}

	ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
	return(TU_GOOD);
}



/*************************************************************************
* Exercise each possible address range for the SRAM			 *
*									 *
* Write a modified address pattern to the first block of SRAM, then move *
* the SRAM to each of the possible base addresses by modifying POS	 *
* register 2.  Confirm that the memory is addressable at each base	 *
* address.								 *
*									 *
* Some of the ugliness is due to a desire to print the range of		 *
* addresses that are bad rather than stopping at the first error.	 *
* MWW 891017 modified to use only Base Addresses #0 - e, f is reserved.	 *
* MWW 891017 use memory block ABOVE the status area, starting at offset	 *
*	     1000h from base address					 *
* MWW 891017 modified to test with 0's at end to clear SRAM.		 *
* MWW 891131 holds reset on when testing SRAM, releases it when exit	 *
*	     NOTE:  POST will run when reset is released!!		 *
*************************************************************************/
tu_baseaddr(cb)
TUTYPE		*cb;				/* control block for necessary info */
{
	int		rc;
	int		byte;
	int		addr_range;
	BYTE		block[BLOCKSIZE];
	char		*str = MSGBUF;
	int		old_range;
	int		error = 0;
	int		error_in_range;
	unsigned int	ma_pattern();
	char		epilog[80];

	sprintf(epilog,"tu = %d, loop = %d\n",TU_NUM,cb->header.loop);

	ROF(change_pos_reg(4, 0x04, 0x04));		/* reset OBP */
	ucode = UNKNOWN;

	ROF(read_pos_reg(2, &old_range));	/* save old range to put back */
#ifdef CARD_OLD_BASE
	old_range = (old_range & 0xe0) >> 5;
#else
	old_range = (old_range & 0xf0) >> 4;
#endif
/*************************************************************************
* Fill with modified address pattern starting at offset 0x1000, so we	 *
* don't hose the status area						 *
*************************************************************************/
	for (byte=0; byte<BLOCKSIZE; byte++)
		block[byte] = ma_pattern((SRAM) byte);
	ROF(write_sram((SRAM) 0x1000, block, BLOCKSIZE));

#ifdef CARD_OLD_BASE
	for (addr_range = 0x00; addr_range <= 0x07; addr_range++)
#else
	for (addr_range = 0x00; addr_range <= 0x0E; addr_range++)
#endif
	{
#ifdef CARD_BAD_BASE
		if (addr_range == 0x04) continue;
		if (addr_range == 0x07) continue;
		if (addr_range == 0x09) continue;
		if (addr_range == 0x0a) continue;
		if (addr_range == 0x0c) continue;
#endif
		change_base_addr(addr_range);
		error_in_range = 0;

		if (read_sram((SRAM) 0x1000, block, BLOCKSIZE) != TU_GOOD) {
			change_base_addr(old_range);
			ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
			return(TU_HARD);
		}
		for (byte=0; byte<BLOCKSIZE; byte++)
			if (!error_in_range && mem_check((long) byte,
				ma_pattern((SRAM) byte), block[byte],epilog) ) {
				error_in_range++;
				sprintf(MSGBUF, "Range %d: ",
					addr_range);
				RPT_SOFT(0, MSGBUF);
			}
		error |= error_in_range;
		mem_check((long) 0, 0, 0, epilog);    /* finish reporting errors */
	}
	change_base_addr(old_range);
	if (error) {
		sprintf(MSGBUF, "base address test failed\n");
		ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
		return(TU_SOFT);
	}

	ROF(change_pos_reg(4, 0x04, 0x00));	 /* let 186 run again */
	return(TU_GOOD);
}
