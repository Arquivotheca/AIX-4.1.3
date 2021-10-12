static char sccsid[] = "@(#)96 1.2 src/bos/usr/lib/nim/methods/c_nim_mkboot32.c, cmdnim, bos41J, 9516B_all 95/04/21 16:41:41";
/*
 * COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
 *
 * FUNCTIONS: mkboot.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME:	c_nim_mkboot32.c
 *
 * FUNCTION:	This is a NIM-specific version of the 4.1 
 *		/usr/sbin/mkboot/mkboot.c which contains just
 *		the functionality required to support writing
 *		IPL ROM emulation to the boot disk.  It must be
 *		backwards compatible all the way down to 3.2 gold.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	The mkboot program is a command that can be executed from
 *	a shell environment. Several options may be specified to
 *	create particular types of boot image files. The device
 *	flag is required to make the boot image file since the
 *	file is device dependent.
 *
 *	1. A diskette boot image file requires a boot and config
 *	   record at the beginning of the kernel/boot file system
 *	   file with the appropriate boot record information for a
 *	   diskette device.
 *
 *	2. A disk boot image does not require the boot and config
 *	   record to be placed at the beginning of the kernel/boot
 *	   file system. The boot record is instead written to the
 *	   first cylinder-head-sector on the disk. The boot image
 *	   file may then be placed elsewhere on the disk since
 *	   the boot record contains its location. If the boot logical
 *	   volume is mirrored, the boot record is updated on each
 *	   physical disk that is part of the mirror.
 *
 *	3. A tape boot image requires a boot and config record placed
 *	   at the beginning of the image file, however, the config
 *	   information is not interpreted by the ROS IPL tape function.
 *
 *	4. A cdrom boot image requires a boot and config record placed
 *	   at the beginning of the image file, however, the config
 *	   information is not interpreted by the ROS IPL cdrom function.
 *
 *	In all cases, except for the write boot record call, mkboot
 *	writes the boot image file to stdout.
 *
 *	Flags:	-d device	Required - specifies type of device the
 *					boot image file will be placed.
 *		-b		Optional - zero out save base fields.
 *					This flag indicates image with boot
 *					and config record (ie. diskette, tape).
 *		-c		Optional - zero out boot record on device.
 *		-D		Optional - load the low level debugger.
 *		-e expander	Optional - expand code file used to create
 *					a compressed boot image file.
 *		-f filesys	Required - boot file system.
 *		-I		Optional - Invoke the low level debugger.
 *		-i		Optional - write only the boot record on disk.
 *		-k kernel	Required - boot kernel.
 *		-l lvdev	Optional - The lvdev is the logical volume
 *					that contains the loadable boot code.
 *		-s		Optional - updates service portion of boot
 *					record.
 *		-p offset	Optional - indicates address to use as
 *					boot_prg_start (see bootrecord.h).
 *		-w		Optional - indicates mkboot to output the first
 *					two blocks of blv.
 *		-r		Optional - indicates image is ROS Emulation.
 *		-h		Optional - do not update boot header.
 *		-L		Optional - enable MP lock instrumentation
 *
 *	Syntax:	mkboot -d device [[-i -s -l lvdev -p address]] -f filesys
 *		-k kernel | -e expander	[>file]] [-b -c -D -I -h -r -L]
 *
 *
 *		Note:	(1) Although mkboot.c combines a kernel and a
 *			RAM file system to create one boot image file,
 *			it may be run a second time to put expand code
 *			at the beginning of a boot image that is
 *			compressed. Typically this takes the form of:
 *
 *	mkboot -b -d /dev/fd0 -k unix -f ramfs | /bin/compress > /tmp/image
 *	mkboot -b -i -s -d /dev/fd0 -k bootexpand -f /tmp/image > bootfile
 *
 *			for a boot diskette, where "unix" is the kernel,
 *			"ramfs" is the RAM disk file system, "/bin/compress"
 *			is the compression or pact routine and "bootexpand"
 *			is the expansion or kernel uncompact routine.
 *
 *			(2) The -w switch is used to output the first
 *			two blocks of the boot logical volume to stdout. This
 *			is to optimize performance when dd'ing the output to
 *			the boot logical volume. It is valid for hard disk
 *			boot only.
 *
 *	Logic:	Interpret options
 *		Determine boot device with ioctl call
 *		If disk then read boot record from disk
 *		If option to clear boot record then
 *			Write zeroed out boot record
 *		else
 *			Open input files
 *			Calculate boot record info from input files and XTOC
 *			If disk boot record option then
 *				For each mirror copy of the logical volume
 *				  Read boot record from disk
 *				  Update/create boot record - normal fields
 *				  Update/create boot record - service fields
 *				  Write boot record to disk
 *			Else
 *				Update/create boot record - normal fields
 *				Update/create boot record - service fields
 *				Write boot record to stdout
 *				Initialize config record
 *				Write config record to stdout
 *			If -h switch not set then
 *				Update kernel boot header
 *				Write kernel boot header to stdout
 *			Write kernel to stdout
 *			Write RAM fs to stdout
 *			Close files
 *		Exit
 *
 */


#include <aouthdr.h>
#include <errno.h>
#include <fcntl.h>
#include <filehdr.h>
#include <locale.h>
#include <lvmrec.h>
#include <nl_types.h>
#include <scnhdr.h>
#include <stdio.h>
#include <string.h>
#include <sys/bootrecord.h>
#include <sys/cfgodm.h>
#include <sys/configrec.h>
#include <sys/devinfo.h>
#include <sys/hd_psn.h>
#include <sys/ioctl.h>
#include <sys/lldebug.h>
#include <sys/lvdd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mkboot.h"

/* Dummy extern int to satisfy reference for sub-3.2.5 systems */
int _system_configuration = 0;

/*
 * functions
 */
int id_device(char *in_dev, char *comp_str);
int mkboot_error();			/* Write error messages */
unsigned int write_disk_record();	/* Write boot record to device */
unsigned long disk_file_size();		/* Calculate file size */
void init_boot_rec();			/* Set boot record normal fields */
void init_serv_rec();			/* Set boot record service fields */
void query_lvs();			/* Get logical volume info */
void read_block();			/* Read block(s) from input file */
void read_boot_rec();			/* Read boot record */
void write_block();			/* Write block(s) to boot file */
void write_kernel();			/* Write kernel to boot file */
void write_ramfs();			/* Write RAM fs to boot file */
unsigned int swap_endian(unsigned int num);
void init_boot_partition_table(boot_partition_table *p_table_ptr);
void init_hints_table(hints *hints_ptr);

/*
 * options
 */
int	no_savebase = 0;		/* clear savebase fields on diskette */
int	rspc = FALSE;
int	write_boot_rec = 0;		/* write boot record for boot lv */
int	write_serv_rec = 0;		/* write boot record for service lv */
int	write_cfig_rec = 0;		/* write config record on boot image */
int	clear_boot = 0;			/* clear boot record */
int	sector_size = 0;		/* sector size in bytes for config */
int	track_size = 0;			/* track size for diskette config */
int	cyl_size = 0;			/* cylinder size for diskette config */
int	capacity = 0;			/* number of blocks for config */
int	head_size = 0;			/* number of disk heads */
int	fs_roundoff = 0;		/* Round off RAM fs to 4K boundary */
int	boot_start_loc = 0;		/* Override boot program start location */
int	write_2blk = 0;			/* Output the first 2 blocks of blv */
int	no_boot_hdr = 0;		/* 1 => do not update boot header */
int	ros_emulation = 0;		/* 1 => image is ROS Emulation */

/*
 * definitions:
 */
#define NUM_MIRRORS 3		   /* Max number of mirrors */
#define DEV_MAX_NM_SZ   _D_NAME_MAX+1+4 /* Max size of pathname in /dev */
#define XFER_BUF_SIZE   46080
struct  pv_info {		       /* Physical volume info */
	int     pbn;		    /* Physical block number */
	char    pv_name[DEV_MAX_NM_SZ]; /* Physical volume name */
};

/*
 * External variables
 */
char	*msgfmt;			/* Error message string pointer */
char	*bdevice;			/* boot device */
char	*lv_dev;			/* logical volume device */
char	*execname;			/* name of file exec'ed */
char	*kernel;			/* kernel load module file name */
char	*filesys;			/* file system image file name */
int	floppy = FALSE;
int	file_size;			/* Boot file size (bytes) */
int	kernel_ep;			/* Kernel entry point */
int	aixmon_ep;			/* aixmon entry point */
int	number_of_sectors_calculated;	/* Boot file size (512b blks) */
int     mode_control;			/* Aixmon boot mode control */
long	text_start;			/* Start of kernel text */
long	disk_size;			/* Ram disk fs size */
static	IPL_REC boot_rec;		/* ROS boot record */
static	IPL_REC clear_rec;		/* ROS boot record */
unique_id_t	pv_id_save;		/* Pvid holder */
struct  pv_info lv_info[NUM_MIRRORS];   /* Logical volume info */
boot_partition_table p_table;	   /* Allocate a partition table */
hints   hints_data;			  /* Boot hints cheat */
struct stat sbuf1;			/* stat file info */

/*
 *  main entry point.
 */
main (int argc, char **argv)
{
	FILE *aixmon_fd;
	FILE *in_file;				/* -k tag file (kernel) */
	FILE *out_file;				/* Standard out file */
	FILE *disk_image;			/* -f tag file (RAM fs) */
	FILE *blv_file;				/* blv device */
	struct devinfo devinfo;			/* ioctl device info */
	struct stat sbuf;			/* stat file info */
	static struct config_rec config_rec;	/* ROS config record */
	char xfer_buf[XFER_BUF_SIZE];
	char *mode_string;
	char *aixmon_name;
	char *masters_pimage;		/* Partition image pointer */
	unsigned long mk_debug = NO_LLDB;	/* No ll debugger default */
	boolean_t lock_instrumentation = FALSE; /* no lock instrumentation default */
	int flg, kflag, eflag, errflg, devcnt;  /* Option flags */
	int rc;					/* Function call return code */
	int fd;				/* File descriptor for physical vol */
	int cntr;			/* For loop counter */
	int b_index;			/* Array index */
	int num_blks = 0;		/* Number of XTOC blocks to skip */
	int mode_override = FALSE;
	int tot_bytes;
	extern char *optarg;
	unsigned long end_disk;		/* end of file system(ram disk) */
	unsigned int IPL_record_id_save;
	partition_header p_head;	/* Masters partition header */


	/*
	 * Internationalize!
	 */
	char *p;
	p = setlocale(LC_ALL, "");

	/*
	 *
	 * Interpret options and input files.
	 *
	 */
	mode_control = 0;
	execname = argv[0];
	flg = kflag = eflag = errflg = devcnt = 0;
	while ((flg = getopt(argc, argv, "A:bcDd:e:Ff:hIik:Ll:m:p:rsw"))
				!= EOF)
	{
	switch(flg) {
		case 'd':
			bdevice = optarg;	/* boot device */
			devcnt++;
			break;
		case 'f':
			filesys = optarg;	/* boot RAM file system */
			break;
		case 'h':
			no_boot_hdr = 1;	/* do not update boot header */
			break;
		case 'i':
			write_boot_rec = 1;	/* create disk boot record */
			break;
		case 'k':
			kernel = optarg;	/* boot kernel */
			if (eflag) errflg++; else kflag++;
			break;
		case 'l':
			lv_dev = optarg;	/* boot physical partition */
			break;
		case 'r':
			ros_emulation = 1;	/* image is ROS Emulation */
			break;
		case 'w':
			write_2blk = 1;		/* output first 2 blocks of blv */
			break;
		default:
			errflg++;
			break;
		}
	} /* while */
	if (errflg || !devcnt) mkboot_error(1, execname, NULL, 0);

	/*
	 *
	 * Determine device characteristics
	 *
	 */
	rc = stat(bdevice, &sbuf);

	if (rc < 0)
		if (errno == ENOENT) {
			/* the file does not exist */
			devinfo.devtype = 0;
		}
		else mkboot_error(4, execname, bdevice, 0);

	else if ((S_ISBLK(sbuf.st_mode) || S_ISCHR(sbuf.st_mode))) {
		/* the file exists and it is block or char device */
		/*
		 * now determine if it is CDROM.  if true, do NOT try
		 * to open it because that will fail if there is not a
		 * disk in the drive and we do not need to open RO devices
		 */
		if (id_device(bdevice, "cdrom/"))	/* is it a CDROM? */
			devinfo.devtype=0;		/* YES, it's a CDROM */
		else {		/* NO, it is NOT a CDROM */
			fd = open(bdevice, O_RDONLY);
			if (fd < 0) mkboot_error(2, execname, bdevice, 0);
			rc = ioctl(fd, IOCINFO, &devinfo);
			close(fd);	/* Close boot device */
			if (rc < 0) {
				perror("ioctl");
				mkboot_error(3, execname, NULL, rc);
			}
		}
	}
	/* the file exists and it is NOT a block or char special file */
	else devinfo.devtype = 0;

	switch(devinfo.devtype) {
		case DD_SCDISK:		/* SCSI disk */
			/* Fall through to Disk device */
		case DD_DISK:		/* Disk - Diskette */
			capacity = devinfo.un.dk.numblks;
			if (capacity < DISKETTE_SIZE) {
				sector_size = devinfo.un.dk.bytpsec;
				track_size = devinfo.un.dk.secptrk;
				cyl_size = devinfo.un.dk.trkpcyl;
				head_size = 0x01;
				write_cfig_rec = fs_roundoff = 1;
				devinfo.devtype = DD_DSKT;
			}
			break;
		case DD_SCTAPE:	/* SCSI Tape */
			capacity = devinfo.un.scmt.blksize;
			write_cfig_rec = fs_roundoff = 1;
			break;
		default:
			write_cfig_rec = fs_roundoff = 1;
			break;
	}

	/*
	 *  Read boot record directly from disk.
	 */
	if ((write_boot_rec || write_serv_rec || clear_boot) &&
	    (devinfo.devtype == DD_DISK || devinfo.devtype == DD_SCDISK)) {
		fd = open(bdevice, O_RDONLY);
		read_boot_rec(fd);
		close(fd);	/* Close boot device */
	}

	/*
	 * Open input files
	 *
	 */
	/* open kernel load module file */
	in_file = fopen( kernel, "rb" );
	/* open file system image file */
	disk_image = fopen( filesys, "rb" );
	if (in_file == NULL || disk_image == NULL) {
		mkboot_error(2, execname, (in_file? filesys: kernel), 0);
	}

	/* Write to standard out */
	out_file = stdout;

	/* Output first 2 blocks of blv (only for disk). */
	if( write_2blk && ((devinfo.devtype == DD_DISK) ||
			 (devinfo.devtype == DD_SCDISK)) ) {
		if( ( blv_file = fopen(lv_dev, "rb")) == NULL )
			mkboot_error(2,execname,lv_dev,0);
		for ( rc = 0;  rc < RESBLK; rc++) {
			read_block( blv_file,UBSIZE, &kernel_read_buffer);
			write_block( out_file,UBSIZE, &kernel_read_buffer);
		}
		fclose(blv_file);
	}

	/*
	 *
	 * Calculate boot record information from input files.
	 *
	 */
	/* Calculate kernel size */
	file_pos.end_of_text = disk_file_size(in_file);
	/* Calculate RAM fs size */
	disk_size = disk_file_size(disk_image);
	/* Read XTOC from kernel */
	read_block(in_file, UBSIZE, &kernel_read_buffer);
	if (K_XTOC.filehdr.f_magic == RS_TOC_NUM ||
		K_XTOC.filehdr.f_magic == OLD_TOC_NUM ) {
		/* Find kernel entry point */
		text_start =
			K_XTOC.scnhdr[K_XTOC.aouthdr.o_sntext-1].s_scnptr;
		kernel_ep =
			K_XTOC.aouthdr.o_entry;
	}
	/* not TOC magic number */
	else mkboot_error(8, execname, kernel, 0);

	/* Round off kernel size */
	file_pos.start_disk = file_pos.end_of_text;
	if ((file_pos.start_disk & 0xfff) != 0) {
		file_pos.start_disk = 0x1000 +
		(file_pos.start_disk & 0xfffff000);
	}
	/* Round off RAM fs size */
	end_disk = file_pos.end_disk = file_pos.start_disk + disk_size;
	if ((file_pos.end_disk & 0xfff) != 0 && !fs_roundoff) {
		file_pos.end_disk = 0x1000 +
		(file_pos.end_disk & 0xfffff000);
	}


	if (write_cfig_rec)
		file_size = file_pos.end_disk;
	else
		/* Add savebase length for disk */
		file_size = file_pos.end_disk + SBSIZE;

	/*
	 * Determine bootfile size in 512 byte blocks for ROS
	 * and round it up to the next whole sector
	 */
	number_of_sectors_calculated = (file_size + UBSIZE - 1)/UBSIZE;

	/*
	 * rspc is false, and hints_data.header_block_size
	 * needs to be initialized
	 */
	hints_data.header_block_size=0;

	/*
	 *
	 * Initialize boot record
	 *
	 */
	boot_rec.IPL_record_id = IPLRECID;

	/*
	 *
	 * Write boot record
	 *
	 */
	if (write_boot_rec || write_serv_rec) {
		/* Write disk boot record directly to device */
		if (devinfo.devtype == DD_DISK ||
			devinfo.devtype == DD_SCDISK) {
			query_lvs(lv_dev);
			/* loop here for mirrors */
			for (cntr=0; cntr < NUM_MIRRORS; cntr++) {
				if (!strlen(lv_info[cntr].pv_name))
					continue;
				fd = open(lv_info[cntr].pv_name, O_RDONLY);
				if (fd < 0)
					mkboot_error(2, execname,
						lv_info[cntr].pv_name, 0);
				read_boot_rec(fd);
				close(fd);
				if (write_boot_rec)
					init_boot_rec(devinfo.devtype,
						lv_info[cntr].pbn);
				if (write_serv_rec)
					init_serv_rec(devinfo.devtype,
						lv_info[cntr].pbn);
				/* calculate which disk to write to */
				write_disk_record(lv_info[cntr].pv_name);
			}
		}
		else {
			if (write_boot_rec) init_boot_rec(devinfo.devtype, 0);
			if (write_serv_rec) init_serv_rec(devinfo.devtype, 0);
			/* Write boot record to boot image file (non disk) */
			write_block(out_file, UBSIZE, &boot_rec);
			/*
			 *
			 * Initialize config record (non disk)
			 *
			 */
			config_rec.config_rec_id = CONFIGID;
			config_rec.sector_size = 0x02;
			config_rec.data_capacity = capacity;
			config_rec.last_cyl = 0x004f;
			config_rec.last_head = head_size;
			config_rec.last_sector = track_size;
			strcpy(config_rec.reserved3,COPYRIGHT);
			/*
			 *
			 * Write config record to boot image file (non disk)
			 *
			 */
			write_block(out_file, UBSIZE, &config_rec);
		}
	}

	/*
	 *
	 * Write boot image file.
	 *
	 */
	write_block(out_file, UBSIZE, &kernel_read_buffer);
	write_kernel(in_file, out_file, num_blks + 1);
	write_ramfs(disk_image, out_file, disk_size);

	/* zero the start of savebase area (which has value of total
	   count of customized objects) if disk device */
	if ( ( devinfo.devtype == DD_SCDISK ) ||
		( devinfo.devtype == DD_DISK )) {
		while( (end_disk & 0xfff) != 0 ) {
			fputc(0, out_file);
			end_disk++;
		}
		for( rc = 0; rc < sizeof(unsigned long); rc++ )
			fputc(0, out_file);
	}

	fclose( in_file );
	fclose( disk_image );
	fclose( out_file );
	exit(0);

} /* End of main */


/*****************************************************************************/

void init_boot_rec(int bootdevice, int pbn)

/* pbn is used only for disk devices */

{
	/*
	 *
	 * Initialize normal boot record - common fields.
	 *
	 */
	boot_rec.boot_code_length = number_of_sectors_calculated;
	boot_rec.boot_lv_length = number_of_sectors_calculated +
				hints_data.header_block_size;
	boot_rec.boot_load_add = 0;
	boot_rec.boot_frag = FRAG_FLAG;
	boot_rec.boot_code_offset = kernel_ep + text_start;
	if (ros_emulation)
		/* image is ROS emulation code */
		boot_rec.boot_emulation = 1;
	else
		/* image is AIX code supporting ROS emulation */
		boot_rec.boot_emulation = 2;
	/*
	 *
	 * Initialize normal boot record - device dependent fields.
	 *
	 */
	switch(bootdevice) {
		case DD_SCDISK:
			/* Fall through to Disk device */
		case DD_DISK:		/* Disk device */
			/*
			 * Find PSN (Physical Sector Number) of boot logical
			 * volume and reserve the second block for BOS install
			 * system settings.
			 */
			boot_rec.boot_lv_start = pbn;
			boot_rec.boot_prg_start =
				boot_rec.boot_lv_start + RESBLK;
			boot_rec.formatted_cap = 0;
			boot_rec.last_head = 0;
			boot_rec.last_sector = 0;
			boot_rec.basecn_length = SBSIZE;
			boot_rec.basecn_start = boot_rec.boot_prg_start +
				(number_of_sectors_calculated -
				(SBSIZE / UBSIZE)) +
				hints_data.header_block_size;
			break;
		case DD_DSKT:		/* Diskette device */
			boot_rec.boot_prg_start = C0_H0_S2;
			boot_rec.formatted_cap = capacity;
			boot_rec.last_head = head_size;
			boot_rec.last_sector = track_size;
			boot_rec.basecn_length = 0;
			boot_rec.basecn_start = 0;
			break;
		default:		/* anything else */
			if ( boot_start_loc ) {
				boot_rec.boot_lv_start = boot_start_loc;
				boot_rec.boot_prg_start = boot_start_loc;
			}
			else {
				boot_rec.boot_lv_start = C0_H0_S2;
				boot_rec.boot_prg_start = C0_H0_S2;
			}
			boot_rec.formatted_cap = 0;
			boot_rec.last_head = 0;
			boot_rec.last_sector = 0;
			boot_rec.basecn_length = 0;
			boot_rec.basecn_start = 0;
			break;
	}

} /* End of init_boot_rec */


/*****************************************************************************/

void init_serv_rec( int bootdevice, int pbn )

/* pbn is used only for disk devices */

{
	/*
	 *
	 * Initialize service boot record - common fields
	 *
	 */
	boot_rec.ser_code_length = number_of_sectors_calculated;
	boot_rec.ser_lv_length = number_of_sectors_calculated +
				hints_data.header_block_size;
	boot_rec.ser_load_add = 0;
	boot_rec.ser_frag = FRAG_FLAG;
	boot_rec.ser_code_offset = kernel_ep + text_start;
	if (ros_emulation)
		/* image is ROS emulation code */
		boot_rec.ser_emulation = 1;
	else
		/* image is AIX code supporting ROS emulation */
		boot_rec.ser_emulation = 2;
	/*
	 *
	 * Initialize service boot record - device dependent fields.
	 *
	 */
	switch(bootdevice) {
		case DD_SCDISK:
			/* Fall through to Disk device */
		case DD_DISK:		/* Disk device */
			/*
			 * Find PSN (Physical Sector Number) of service
			 * logical volume and reserve the first block.
			 */
			boot_rec.ser_lv_start = pbn;
			boot_rec.ser_prg_start =
				boot_rec.ser_lv_start + RESBLK;
			boot_rec.basecs_length = SBSIZE;
			boot_rec.basecs_start = boot_rec.ser_prg_start +
				(file_pos.end_disk / UBSIZE) +
				hints_data.header_block_size;
			break;
		case DD_DSKT:		/* Diskette device */
			if ( boot_start_loc )
				boot_rec.ser_prg_start = boot_start_loc;
			else
				boot_rec.ser_prg_start = C0_H0_S2;
			boot_rec.basecs_length = 0;
			boot_rec.basecs_start = 0;
			break;
		default:		/* anything else */
			if ( boot_start_loc ) {
				boot_rec.ser_lv_start = boot_start_loc;
				boot_rec.ser_prg_start = boot_start_loc;
			}
			else {
				boot_rec.ser_lv_start = C0_H0_S2;
				boot_rec.ser_prg_start = C0_H0_S2;
			}
			boot_rec.basecs_length = 0;
			boot_rec.basecs_start = 0;
			break;
	}

} /* End of init_serv_rec */

/*****************************************************************************/

void read_block(	FILE *stdin_file,
			long read_size,
			unsigned char *blk_info	)
{
	long rcl;

	rcl = fread( blk_info , 1 , read_size , stdin_file );
	if (rcl != read_size) mkboot_error(9, execname, NULL, rcl);

} /* End of read_block */


/*****************************************************************************/

void write_block(	FILE *stdout_file,
			long write_size,
			unsigned char *blk_info	)
{
	long rcl;

	rcl = fwrite( blk_info , 1 , write_size , stdout_file );
	if (rcl != write_size) mkboot_error(10, execname, NULL, rcl);

} /* End of write_block */

/****************************************************************************/

void write_kernel(	FILE *kern_file,
			FILE *newk_file,
			int num_blks	)
{
	unsigned char *buffer;	/* malloc buffer */
	long	buff_size;	/* Read-write buffer size */

	buff_size = file_pos.end_of_text - (UBSIZE * num_blks);
	buffer = (unsigned char *)malloc( buff_size );
	if (buffer == NULL) mkboot_error(7, execname, NULL, buff_size);
	read_block(kern_file, buff_size, buffer);
	write_block(newk_file, buff_size, buffer);
	free( buffer );

} /* End of write_kernel */

/*****************************************************************************/

void write_ramfs(	FILE *disk_file,
			FILE *newd_file,
			long ramfs_size	)
{
	unsigned char *buffer;	/* malloc buffer */
	unsigned long text_end;

	text_end = file_pos.end_of_text;

	while ((text_end & 0xfff) != 0) {
		fputc( 0 , newd_file );
		text_end++;
	} /* while */

	/* File may legitimately be zero length. */
	if (ramfs_size > 0) {
		buffer = (unsigned char *)malloc( ramfs_size );
		if (buffer == NULL) mkboot_error(7,execname,NULL,ramfs_size);
		read_block(disk_file, ramfs_size, buffer);
		write_block(newd_file, ramfs_size, buffer);
		free( buffer );
	}

} /* End of write_ramfs */

/*****************************************************************************/

unsigned long disk_file_size( FILE *test_file )
{
	unsigned long length;
	int rc;

	rc = fseek( test_file , 0L , 2 );
	if (rc != 0) mkboot_error(5, execname, NULL, 0);
	length = ftell( test_file );
	rc = fseek( test_file , 0L , 0 );
	if (rc != 0) mkboot_error(5, execname, NULL, 0);
	return( length );

} /* End of disk_file_size */

/*****************************************************************************/

unsigned int write_disk_record(char *bdevice)
{
	int fd, rc, offset;

	fd = open(bdevice, O_RDWR);
	if (fd < 0) mkboot_error(2, execname, bdevice, 0);
	offset = lseek(fd, PSN_IPL_REC, 0);
	if (offset < 0) mkboot_error(5, execname, NULL, 0);
	rc = write(fd, (char *) &boot_rec, sizeof(boot_rec));
	if (rc < 0) mkboot_error(10, execname, NULL, rc);
	close(fd);

} /* End of write_disk_record */

/*****************************************************************************/

int id_device(char *in_dev, char *comp_str)
{
	/*
	 * this function returns 0 or 1 to indicate whether or not a
	 * device specified by in_dev matches a device type, which
	 * is specified by comp_str.
	 */

	char sstr[256];
	char *strptr;
	int rc;
	struct  CuDv cudv_info;

	/* strip away anything before the last slash */
	strptr=strrchr(in_dev, '/');
	/* then increment the ptr and put it in a search string */
	sprintf(sstr, "name='%s'", ++strptr);
	if (odm_initialize() == -1)
		mkboot_error(11, execname, "odm_initialize", 0);
	rc=(int)odm_get_first(CuDv_CLASS, sstr, &cudv_info);
	if (rc==0 || rc==-1)
		/*
		 * this is an error because the stat call told us that
		 * the device exists and it is block or char
		 */
		mkboot_error(11, execname, "odm_get_first", 0);
	odm_terminate();
	if (strncmp(cudv_info.PdDvLn_Lvalue, comp_str, strlen(comp_str)))
		return(0);	/* does NOT match */
	else return(1);		/* does match */
} /* End of id_device */

/*****************************************************************************/

mkboot_error(	int errnum,
		char *progname,
		char *strngarg,
		int numbarg	)

{
	nl_catd catd;		/* NLS catalogue */
	char	*msgfmt;	/* Error message string pointer */


	/*
	 * Open message catalogue for NLS support.
	 */
	catd = catopen(MSGCAT, NL_CAT_LOCALE);

	switch(errnum) {
	case   1:
		msgfmt = catgets(catd, 100, 1,
		"\n usage: %s  -d device [[-i -s -l lvdev -p address]]\n");
		fprintf(stderr, msgfmt, progname);
		msgfmt = catgets(catd, 100, 2,
		"\t-f filesys {-k kernel | -e expander} [-D |-I] [-b -c -h -L -r -w]\n");
		fprintf(stderr, msgfmt);
		break;
	case   2:
		msgfmt = catgets(catd, 100, 3,
		"\n0301-101 %s: Device open failure: %s\n");
		fprintf(stderr, msgfmt, progname, strngarg);
		break;
	case   3:
		msgfmt = catgets(catd, 100, 4,
		"\n0301-102 %s: The ioctl call failed with return: %d\n");
		fprintf(stderr, msgfmt, progname, numbarg);
		break;
	case   4:
		msgfmt = catgets(catd, 100, 5,
		"\n0301-103 %s: error opening \"%s\"\n");
		fprintf(stderr, msgfmt, progname, strngarg);
		break;
	case   5:
		msgfmt = catgets(catd, 100, 6,
		"\n0301-104 %s: An error occurred on a seek operation!\n");
		fprintf(stderr, msgfmt, progname);
		break;
	case   7:
		msgfmt = catgets(catd, 100, 8,
		"\n0301-106 %s: The malloc call failed for size: %d\n");
		fprintf(stderr, msgfmt, progname, numbarg);
		break;
	case   8:
		msgfmt = catgets(catd, 100, 9,
		"\n0301-107 %s: \"%s\" is NOT a properly formatted load module.\n");
		fprintf(stderr, msgfmt, progname, strngarg);
		break;
	case   9:
		msgfmt = catgets(catd, 100, 10,
		"\n0301-108 %s: Unable to read file blocks. Return code: %d\n");
		fprintf(stderr, msgfmt, progname, numbarg);
		break;
	case  10:
		msgfmt = catgets(catd, 100, 11,
		"\n0301-109 %s: Unable to write file blocks. Return code: %d\n");
		fprintf(stderr, msgfmt, progname, numbarg);
		break;
	case  11:
		msgfmt = catgets(catd, 100, 12,
		"\n0301-111 %s: error on Object Data Manager routine \"%s\"\n");
		fprintf(stderr, msgfmt, progname, strngarg);
		break;
	default:
		msgfmt = catgets(catd, 100, 99,
		"\n0301-110 %s: Unknown failure!\n");
		fprintf(stderr, msgfmt, progname);
		errnum = 99;
		break;
	}
	exit(errnum);
} /* End of mkboot_error */

/*****************************************************************************/

void query_lvs(char *lvdev)
{
	int m, fd;
	struct xlate_arg xlate_arg[NUM_MIRRORS];
	char newname[DEV_MAX_NM_SZ];
	char sstr[256];
	int rc;
	struct  CuDvDr cudvdr_info;

	fd = open(lvdev, O_RDONLY);
	if (fd < 0) mkboot_error(2, execname, lvdev, 0);

	if (odm_initialize() == -1)
		mkboot_error(11, execname, "odm_initialize", 0);

	for (m=0; m < NUM_MIRRORS; m++) {
		xlate_arg[m].lbn = 0;
		xlate_arg[m].mirror = m + 1;
		rc = ioctl(fd, XLATE, &xlate_arg[m]);
		if (rc < 0) {
			lv_info[m].pv_name[0] = '\0'; /* clear phys vol name */
			continue;
		}
		/* Convert physical device number to physical volume name */
		sprintf(sstr, "resource='devno' and value1=%d and value2=%d",
			major(xlate_arg[m].p_devt),
			minor(xlate_arg[m].p_devt));
		rc=(int)odm_get_first(CuDvDr_CLASS, sstr, &cudvdr_info);
		if (rc==0 || rc==-1)
			/*
			 * this is an error because the XLATE ioctl told us
			 * that the device exists
			 */
			mkboot_error(11, execname, "odm_get_first", 0);
		sprintf(newname, "%s/%s", "/dev", cudvdr_info.value3);
		strcpy(lv_info[m].pv_name, newname);
		/* Get physical block number */
		lv_info[m].pbn = xlate_arg[m].pbn;
	}

	odm_terminate();
	close(fd);

} /* End of query_lvs */

/*****************************************************************************/

void read_boot_rec(int fd)
{
	int rc;
	long offset;

	offset = lseek(fd, PSN_IPL_REC, 0);
	if (offset < 0) mkboot_error(5, execname, NULL, 0);
	rc = read(fd, (char *) &boot_rec, sizeof(boot_rec));
	if (rc < 0) mkboot_error(9, execname, NULL, rc);

} /* End of read_boot_rec */

/*****************************************************************************/

/*
 * swap_endian
 * This function converts the endian-ness of a number.  If the input is big
 * endian, then the output is little endian, and vice-versa.
 *
 * input via globals: none
 * input via parameters: num, which is unsigned integer
 * output via globals: none
 * output via parameters: none
 * output via return value: the converted value, in the form of unsigned integer
 * locals: none
 */
unsigned int swap_endian(unsigned int num) {
	return(((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +
		((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24));
}

/*****************************************************************************/

/*
 * init_boot_partition_table
 * This function initializes the boot partition table, which is defined in
 * mkboot.h.
 *
 * input via globals: boot_rec, which is a structure of type IPL_REC as
 *	defined in bootrecord.h
 * input via parameters: p_table_ptr, which is a pointer to a structure of
 *	type boot_partition_table as defined in mkboot.h
 * output via globals: none
 * output via parameters: p_table_ptr, same as parameter input
 * output via return value: none
 * locals: i, tmp_p_table
 */
void init_boot_partition_table(boot_partition_table *p_table_ptr) {

	int i;

	for(i=0;i<4;i++) {
		p_table_ptr->partition[i].boot_ind = NO_BOOT;
		p_table_ptr->partition[i].begin_h = 0xFF;
		p_table_ptr->partition[i].begin_s = 0xFF;
		p_table_ptr->partition[i].begin_c = 0xFF;
		p_table_ptr->partition[i].syst_ind = NOT_ALLOCATED;
		p_table_ptr->partition[i].end_h = 0xFF;
		p_table_ptr->partition[i].end_s = 0xFF;
		p_table_ptr->partition[i].end_c = 0xFF;
		p_table_ptr->partition[i].RBA = 0;
		p_table_ptr->partition[i].sectors = 0;
	}
	/* Setup the Masters Partition */
	p_table_ptr->partition[0].syst_ind = MASTERS;
	p_table_ptr->partition[1].syst_ind = MASTERS;

	/* check to determine if we need to save existing offsets */
	if ((((char *)&boot_rec)[SIG_START] == 0x55) &&
			(((char *)&boot_rec)[SIG_START + 1] == 0xAA)) {
		boot_partition_table *tmp_p_table;

		tmp_p_table = (boot_partition_table *)((char *)(&boot_rec) + PART_START);
		p_table_ptr->partition[0].RBA = tmp_p_table->partition[0].RBA;
		p_table_ptr->partition[0].sectors =
			tmp_p_table->partition[0].sectors;
		p_table_ptr->partition[1].RBA = tmp_p_table->partition[1].RBA;
		p_table_ptr->partition[1].sectors =
			tmp_p_table->partition[1].sectors;
	}
}

/*****************************************************************************/

/*
 * init_hints_table
 * This function initializes the hints table, which is defined in mkboot.h.
 *
 * input via globals:	text_start, a long
 *			kernel_ep, an integer
 *			sbuf1, a structure of type stat as defined in stat.h
 *			aixmon_ep, an integer
 *			A_XTOC, a structure of type xtoc as defined in mkboot.h
 *			mode_control, an integer
 * input via parameters: hints_ptr, which is a pointer to a structure of type
 *	hints as defined in mkboot.h
 * output via globals: none
 * output via parameters: hints_ptr, same as parameter input
 * output via return value: none
 * locals: none
 */
void init_hints_table(hints *hints_ptr) {

	hints_ptr->signature = AIXMON_SIG;
	if (floppy) {
		hints_ptr->jump_offset = 0;
		hints_ptr->header_size = 4 * UBSIZE;
	}
	else {
		hints_ptr->jump_offset = text_start + kernel_ep;
		hints_ptr->header_size = 2 * UBSIZE;
	}
	hints_ptr->image_length = (sbuf1.st_size + sizeof(hints)) - aixmon_ep;
	hints_ptr->header_block_size = ((hints_ptr->header_size +
		(hints_ptr->image_length + UBSIZE - 1))/UBSIZE);
	hints_ptr->bss_offset = A_XTOC.scnhdr[A_XTOC.aouthdr.o_snbss-1].s_paddr;
	hints_ptr->bss_length = A_XTOC.aouthdr.bsize;
	hints_ptr->mode_control = 0xDEAD0000 | mode_control;
}

/*****************************************************************************/

/*
 * End of mkboot.c
 */
