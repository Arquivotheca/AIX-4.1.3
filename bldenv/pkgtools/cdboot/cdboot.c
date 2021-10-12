static char sccsid[] = "@(#)86  1.3  src/bldenv/pkgtools/cdboot/cdboot.c, pkgtools, bos41B, 9505A 1/24/95 20:22:59";
/*
* COMPONENT_NAME: pkgtools
* PROGRAM_NAME: cdboot.c
* The purpose of this program is to modify a cdrom boot image
* (specified with the -p option) by including an offset (as calculated
* with the stat system call) which is the size of the cdrom file system
* in bytes.  Adding the offset to the boot record is necessary to
* make the cdrom bootable by placing the actual boot image
* after the filesystem on the cdrom image.  The offset tells the
* ROS startup process where to find the boot image.  The modified boot
* image is appended to the cdrom filesystem image, which is specified with
* the -d option.
* The -s option allows you to specifiy an optional rspc boot image. This
* image will be placed in the output file immediately following the primary
* bootimage (specified with the -p option). When the -p option is specified,
* the rspc portion of the boot record will be initialized to properly
* reflect the location of the rspc boot image on the CD.
*
* FUNCTIONS:
*       main()
*       read_block()
*       write_block()
*       swap_endian()
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business machines Corp. 1989, 1994
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

#include <errno.h>
#include <fcntl.h>
#include <mbrecord.h>
#include <stdio.h>
#include <string.h>
#include <sys/bootrecord.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#define	SECTORSIZE	2048

static	IPL_REC boot_rec;		/* ROS boot record */

int read_block(FILE *stdin_file, long read_size, void *blk_info);
void write_block(FILE *stdout_file, long write_size, void *blk_info);
unsigned int swap_endian(unsigned int num);

main (int argc, char **argv)
{
	FILE *prim_bi_fp = NULL, *out_file = NULL, *sec_bi_fp = NULL;
	char *prim_boot_file = NULL, *dest_cd_file = NULL;
	char *sec_boot_file = NULL;
	char	buf[BUFSIZ];
	extern char *optarg;
	int offset = 0, flg, read_count, i;
	int cdfs_pad_size, prim_bi_pad_size, prim_bi_size=0;
	boot_partition_table *p_table;
	struct stat stat_buf;
	short *sptr;

	while ((flg = getopt(argc, argv, "d:p:s:")) != EOF)
	    switch(flg) {
		case 'd': /* required - name of output cdrom filesystem file */
			dest_cd_file = optarg;
			break;
		case 'p': /* name of optional primary cdrom boot image file */
			prim_boot_file = optarg;
			break;
		case 's': /* name of optional secondary cdrom boot image file */
			sec_boot_file = optarg;
			break;
	    }

	/*
	 * check for valid syntax
	 */
	if ((prim_boot_file == NULL && sec_boot_file == NULL) ||
		(dest_cd_file == NULL)) {
		fprintf(stderr,"usage: %s -d <output cdrom filesystem image>\n\t\t[ -p <input primary cdrom boot image> |\n\t\t  -s <input secondary cdrom boot image> ]\n", argv[0]);
		exit (1);
	}

	/*
	 * Open primary CDROM boot image input file.
	 */
	if (prim_boot_file) {
		if((prim_bi_fp = fopen(prim_boot_file, "rb")) == NULL) {
			fprintf(stderr,
				"Unable to open file %s\n", prim_boot_file);
			exit(1);
		}
		if(fstat(fileno(prim_bi_fp), &stat_buf)) {
			fprintf(stderr,
				"Unable to stat file %s\n",prim_boot_file);
			exit(1);
		}
		/*
		 * adjust the file size to compensate for the boot record
		 * which is written over the first 512 bytes of the cdrom
		 * filesystem (it is not copied with the primary boot image)
		 * and for the dummy block of 512 bytes that follows the br
		 */
		stat_buf.st_size -= (2 * UBSIZE);
		/*
		 * calculate the amount of padding to achieve sector boundary
		 */
		prim_bi_pad_size = (((stat_buf.st_size + SECTORSIZE - 1) /
			SECTORSIZE) * SECTORSIZE) - stat_buf.st_size;
		/*
		 * Calculate length of primary boot image and any
		 * padding in blocks.
		 */
		prim_bi_size = (stat_buf.st_size + prim_bi_pad_size) / UBSIZE;
	}

	/*
	 * Open secondary CDROM boot image input file.
	 */
	if (sec_boot_file)
		if((sec_bi_fp = fopen(sec_boot_file,"rb")) == NULL) {
			fprintf(stderr,
				"Unable to open file %s\n", sec_boot_file);
			exit(1);
		}

	/*
 	 * Open output cdrom filesystem file.
 	 */
	if((out_file = fopen(dest_cd_file, "r+b")) == NULL) {
  		fprintf(stderr, "Unable to open file %s\n", dest_cd_file);
  		exit(1);
	}

	if(fstat(fileno(out_file), &stat_buf)) {
		fprintf(stderr, "Unable to stat file %s\n",dest_cd_file);
		exit(1);
	}

	/*
	 * calculate the amount of padding to achieve sector boundary
	 */
	cdfs_pad_size = (((stat_buf.st_size + SECTORSIZE - 1) /
		SECTORSIZE) * SECTORSIZE) - stat_buf.st_size;
	/*
	 * Calculate size of the cdrom filesystem and any padding in blocks.
	 */
	offset = (stat_buf.st_size + cdfs_pad_size) / UBSIZE;


	/*
	 * read_block is defined below:
	 *	read boot record from the input cdrom boot image.
	 */
	if (prim_boot_file) {
		read_count = read_block(prim_bi_fp, UBSIZE, &boot_rec);
		if (read_count <= 0)
		{
			fprintf(stderr,"Unable to read file %s\n", prim_boot_file);
			exit(1);
		}

		/*
		 * Point the boot record to the beginning of the boot image.
		 * This will be the end of the cdrom filesystem plus sufficient
		 * padding to establish a 2048 sector boundary.  The boot
		 * record uses 512 byte blocks.
		 */
		boot_rec.boot_lv_start = offset;
		boot_rec.boot_prg_start = offset;
		boot_rec.ser_lv_start = offset;
		boot_rec.ser_prg_start = offset;
	}

	/* optional rspc boot image specified */

	if (sec_boot_file) {
		p_table = (boot_partition_table *)((int)&boot_rec + PART_START);
		/* Build an rspc partition table */
		for(i=0; i<4; i++)
		{
			p_table->partition[i].boot_ind = NO_BOOT;
			p_table->partition[i].begin_h = 0xFF;
			p_table->partition[i].begin_s = 0xFF;
			p_table->partition[i].begin_c = 0xFF;
			p_table->partition[i].syst_ind = NOT_ALLOCATED;
			p_table->partition[i].end_h = 0xFF;
			p_table->partition[i].end_s = 0xFF;
			p_table->partition[i].end_c = 0xFF;
			p_table->partition[i].RBA = 0;
			p_table->partition[i].sectors = 0;
		}
		/* Setup the rspc partition table entry */
		p_table->partition[0].syst_ind = MASTERS;

		/*
		 * the secondary boot image location is calculated by adding
		 * the size of the cdrom filesystem and the size of the primary
		 * boot image.  Both of these sizes have already been rounded
		 * up to the next sector (2048) boundary.  This is for those
		 * cdrom drives that prefer sector boundaries.
		 */
		p_table->partition[0].RBA = swap_endian(offset + prim_bi_size);

		/* Write rspc partition table signature */
		sptr = (short *)&boot_rec;
		sptr[SIG_START/2] = BOOT_SIGNATURE;
	}


	write_block(out_file, UBSIZE, &boot_rec);

	if ( fseek (out_file, 0L, SEEK_END) < 0 )
	{
		printf ("cdboot: fseek on %s failed, errno = %d\n",
			dest_cd_file, errno);
		exit (1);
	}

	memset(buf, '\0', BUFSIZ);
	write_block(out_file, cdfs_pad_size, buf);

	if (prim_boot_file) {
		/* skip over the dummy block */
		fseek(prim_bi_fp, UBSIZE, SEEK_CUR);
		/*
	 	 * Copy primary cdrom boot image with new boot record.
	 	 */
		while (
		  (read_count = read_block(prim_bi_fp, BUFSIZ, buf)) > 0)
			write_block(out_file, read_count, buf);
		fclose(prim_bi_fp);

		if(sec_boot_file) {
			memset(buf, '\0', BUFSIZ);
			write_block(out_file, prim_bi_pad_size, buf);
		}
	}

	/*
	 * If we have an rspc boot image, append it to the output
	 */
	if(sec_boot_file) {
		/* skip over the boot record */
		fseek(sec_bi_fp, 2 * UBSIZE, SEEK_SET);
		while ((read_count =
				read_block(sec_bi_fp, BUFSIZ, buf)) > 0)
			write_block(out_file, read_count, buf);
		fclose(sec_bi_fp);
	}

	fclose(out_file);
}
 /* End of main */

/*****************************************************************************/

int read_block(FILE *stdin_file, long read_size, void *blk_info)
{
	long rcl;

        rcl = fread( blk_info , 1 , read_size , stdin_file );
	if (rcl < 0) fprintf(stderr,"read_block failed\n");
	return(rcl);

} /* End of read_block */


/*****************************************************************************/

void write_block(FILE *stdout_file, long write_size, void *blk_info)
{
	long rcl;

        rcl = fwrite( blk_info , 1 , write_size , stdout_file );
	if (rcl != write_size) fprintf(stderr,"write_block failed\n");

} /* End of write_block */

/*****************************************************************************/

unsigned int swap_endian(unsigned int num)
{
   return(((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +
          ((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24));
} /* End of swap_endian */

/*****************************************************************************/
