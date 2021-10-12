static char sccsid[] = "@(#)21  1.3  src/bos/kernel/io/ramio.c, sysio, bos411, 9428A410j 4/5/93 17:17:24";
/*
 * COMPONENT_NAME: SYSIO
 *
 * FUNCTIONS: Read and write blocks to compressed ram image
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/param.h>
#include <sys/ramdd.h>
#ifdef _KERNEL
#include <sys/low.h>
#else
#include <assert.h>
#define halt_display(a,b,c) assert(0)
#endif


BUGXDEF(ramdd_dbg);

#define HALT_NOMEM 0x55800000

/*
 * NAME: pack_ram
 *
 * FUNCTION: pack ram image, adjusting offsets
 *
 * EXECUTION ENVIORNMENT:
 *	called from cram_write when high water mark of ram disk
 *	segment is reached.
 *	This routine is NOT reentrant
 *
 * RETURNS: NONE
 */
void
ram_pack(rd)
struct ramimage *rd;
{
	int dcount;			/* debug count of blocks moved	*/
	int block;			/* block to relocate		*/
	int offset;			/* current offset in segment	*/
	struct ramblock *rb;		/* block struct for block being	*/
					/* relocated			*/

	BUGLPR(ramdd_dbg, 10, ("pack ram\n"));

	dcount = 0;
	offset = DATA_START(rd);	/* start of packed data		*/
	block = rd->r_first;

	/*
	 * loop till reaching the last block in the disk.  Copy data to
	 * the lowest available address and adjust the data offsets for
	 * each block
	 */
	while (block != LAST_BLOCK) {
		rb = &rd->r_block[block];
		bcopy((char *)rd + rb->rb_addr, (char *)rd + offset,
							 rb->rb_size);
		rb->rb_addr = offset;
		offset += rb->rb_size;
		block = rb->rb_next;
		dcount++;
		ASSERT(dcount <= rd->r_blocks);
	}

	ASSERT(dcount == rd->r_blocks);

	/*
	 * set the allocation off set to next available byte
	 */
	rd->r_alloc = offset;
	ASSERT(rd->r_high >= offset);

}

void
ram_alloc(rd)
struct ramimage *rd;
{
	if (rd->r_high + RAM_ALLOC > rd->r_segsize)
		rd->r_high = rd->r_segsize;
	else
		rd->r_high += RAM_ALLOC;
	
	BUGLPR(ramdd_dbg, 10,("alloc %d\n", rd->r_high));
	ASSERT(rd->r_high <= rd->r_blocks * PSIZE + DATA_START(rd));
}

/*
 * NAME: read_ram
 *
 * FUNCTION: read block from ram disk
 * 
 * EXECUTION ENVIRONMENT:
 *	called from the ram disk strategy routine
 *	this routine is NOT reentrant
 *
 * RETURNS: NONE
 *
 */
void
cram_read(rd, block, ibuf)
struct ramimage *rd;
int block;
char *ibuf;
{
	struct ramblock *rb;

	BUGLPR(ramdd_dbg, 10, ("cram_read: %x %x %x\n", rd, block, ibuf));
	ASSERT(rd->r_blocks >= block);
	ASSERT(block >= 0);

	rb = &rd->r_block[block];
	ram_decompress((char *)rd + rb->rb_addr, ibuf, rb->rb_size);
}

/*
 * NAME: cram_write:
 *
 * FUNCTION: pack and write a block of data to the ram disk
 *
 * EXECUTION ENVIRONMENT:
 *	called from ram disk strategy routine to write a block of data
 *	to compressed ram disk
 *
 * RETURNS: NONE
 */
void
cram_write(rd, block, obuf)
struct ramimage *rd;		/* ram disk image		*/
int block;			/* disk block to write		*/
char *obuf;			/* data to write		*/
{
	struct ramblock *rb;	/* ram disk block info of block to write */
	char *addr;		/* ram disk address to write to		*/
	int size;		/* size of packed block			*/

	ASSERT(rd->r_blocks > block);
	ASSERT(block >= 0);

	/*
	 * at least PSIZE bytes are needed to write block to ram disk.  If
	 * we are within PSIZE bytes of high water mark then pack the
	 * segment
	 */
	if (rd->r_alloc + PSIZE > rd->r_high) {
		ram_pack(rd);

		/*
		 * if the allocation offset is within RAM_ALLOC of the high
		 * water mark after packing then raise the high water mark
		 */
		if (rd->r_alloc + RAM_ALLOC  > rd->r_high) {
			ram_alloc(rd);
			BUGLPR(ramdd_dbg, 10, 
				("%d %d\n", rd->r_alloc, rd->r_segsize));

			/*
			 * if we still don't have enough memory then give up
			 */
			if (rd->r_alloc + PSIZE > rd->r_high)
				halt_display(csa, HALT_NOMEM, 0);
		}
	}

	/*
	 * compress the block
	 */
	addr = (char *)rd + rd->r_alloc;
	if (rd->r_flag == RAM_WPACK) {
		ram_compress(obuf, addr, &size);
	} else {
		ASSERT(rd->r_flag == RAM_WCOPY);
		bcopy(obuf, addr, PSIZE);
		size = PSIZE;
	}
	rb = &rd->r_block[block];

	/*
	 * check if this is the last block (highest address) in the ram
	 * ram disk segment.  If it is the last block then we don't
	 * need to adust block list
	 */
	if (rb->rb_next != LAST_BLOCK) {

		/*
		 * remove block form ordered list, and place
		 * it at the end.
		 */
		rd->r_block[rb->rb_next].rb_prev = rb->rb_prev;
		rd->r_block[rd->r_last].rb_next = block;

		/*
		 * check if this is the first block on the list. If it
		 * is not the first block then adjust the previous
		 * blocks next pointer
		 */
		if (rb->rb_prev != LAST_BLOCK) {
			rd->r_block[rb->rb_prev].rb_next = rb->rb_next;
		} else {

			/*
			 * make the next block the first one on list
			 */
			ASSERT(rd->r_first == block);
			rd->r_first = rb->rb_next;
			rd->r_block[rb->rb_next].rb_prev = LAST_BLOCK;
		}

		/*
		 * newly written block is always place at the end of list
		 */
		rb->rb_prev = rd->r_last;
		rb->rb_next = LAST_BLOCK;
		rd->r_last = block;

	} else {
		/*
		 * nothing to do if block is the last one on the list
		 */
		ASSERT(rd->r_last == block);
	}

	rb->rb_addr = rd->r_alloc;
	rb->rb_size = size;
	rd->r_alloc += size;

}

/*
 * NAME: update_rmap
 *
 * FUNCTION: Update the map in a compressed ram disk when new ramblocks
 *		have been added.
 *
 * EXECUTION ENVIRONMENT: 
 *	called from the ram disk ioctl IOCCONFIG
 *	this routine is not reentrant
 *
 * LOGIC:
 *	If there is enough space between the end of map and first data
 *	block, new ramblocks are allocated there. Otherwise
 *	enough blocks are moved to the end to make room for new ramblocks.
 *
 * RETURNS: NONE
 *
 */
void
update_rmap(rd, nblocks)
struct ramimage *rd;
int nblocks;	/* number of new ramblocks */
{
	int blk, prev_blk, tot_blocks, last_block;
	size_t	map_offset;	/* offset to end of new map */
	struct ramblock *rb;


	/* update segsize here so ram_alloc will work */
	if( ram_low_mem() ) 
		rd->r_segsize = (rd->r_blocks + nblocks + 1 )/2*PSIZE 
					+ DATA_START(rd) ;	
	else 
		rd->r_segsize += nblocks*PSIZE;
		
	map_offset = sizeof( struct ramblock ) * nblocks + DATA_START(rd);		
	blk = rd->r_first;
	rb = &rd->r_block[blk];
	prev_blk = rd->r_last;	
	
	if( rb->rb_addr < map_offset ) {
		/*
		 * There are data blocks where the new map needs to be, so
	 	 * move data blocks out of area where new ramblocks need to be
	 	 */
		ram_pack(rd); /* ensure enough space at the end */

		while( rb->rb_addr < map_offset ) {
		   /* copy data block to next available offset */
		   bcopy((char *)rd + rb->rb_addr, (char *)rd + rd->r_alloc,
						rb->rb_size);
		   rb->rb_addr = rd->r_alloc;	/* new offset of data block */
		   rd->r_alloc += rb->rb_size;  /* update next avail offset */
		   /* link block moved after previous last block */
		   rb->rb_prev = prev_blk;
		   rd->r_block[prev_blk].rb_next = blk;
		   /* get set for next block */
		   prev_blk = blk;
		   blk = rb->rb_next;
		   rb = &rd->r_block[blk];
		}
		/*
		 * Now that we have made room for the new map, update fields
		 * to make blk the first (lowest address) block and
		 * prev_blk the last (highest address) block.
		 */
		rd->r_first = blk;
		rd->r_last = prev_blk;
		rd->r_block[blk].rb_prev = LAST_BLOCK;	
		rd->r_block[ prev_blk ].rb_next = LAST_BLOCK;
	}
	
	last_block = rd->r_last;	
	tot_blocks = rd->r_blocks + nblocks;
	rd->r_block[ last_block ].rb_next = rd->r_blocks;

	/*
	 * Link the new blocks together, and make each start at the 
	 * end of the data area and have a length of zero.
	 */
	for( blk = rd->r_blocks; blk < tot_blocks; blk++) {
		rd->r_block[blk].rb_next = blk + 1;
		rd->r_block[blk].rb_prev = last_block;
		rd->r_block[blk].rb_addr = rd->r_alloc;
		rd->r_block[blk].rb_size = 0;
		last_block = blk;
	}

	/* set up last block */
	rd->r_last = tot_blocks - 1 ;
	rd->r_block[rd->r_last].rb_next = LAST_BLOCK;

	rd->r_high = rd->r_alloc;
	rd->r_blocks += nblocks;   /* total number of blocks in ram disk */
}
