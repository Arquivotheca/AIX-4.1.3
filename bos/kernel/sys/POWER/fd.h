/* @(#)82 1.24 src/bos/kernel/sys/POWER/fd.h, sysxfd, bos41J, 9513A_all 3/28/95 13:45:06 */
#ifndef _H_FD
#define _H_FD
/*
 * COMPONENT_NAME: (SYSXFD) Diskette Device Driver Header File
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * The following is the structure passed in to the fd_config routine.
 */

union  	fd_config {
	/* used in CFG_INIT for adapter configuration */
	struct fda_config {
		ulong	slot_num;	/* adapter slot number */
		ulong	bus_id;		/* id of io bus */
		ulong	io_address;	/* offset to first io address */
					/* of the controller 	      */
		int	dma_level;	/* diskette DMA arbitration level */
		int	bus_int_level;	/* bus interrupt level */
		int	device_idle_time;    /* time before entering PM idle mode */
		int	device_standby_time; /* time before entering PM idle mode */
		int	pm_attribute; 	/* PM attribute for the device */
		ushort	bus_type;	/* bus type for interrupt handler */
		uchar	int_class;	/* interrupt class */
		uchar   adapter_type;
	#define FD_ADAPTER0     0x00	/* default adapter type */
	#define FD_ADAPTER1     0x01	/* SIO devid fef6 - RS/6000 2xx type */
	#define FD_ADAPTER2     0x02	/* MCA SMP */
	#define FD_ADAPTER3     0x03	/* ISA Super SIO */
		char	adap_name[8];	/* string for error logging */
	} fda;

	/* used in CFG_INIT for drive configuration */
	struct fdd_config {
		int	pm_device_id;	/* drive's planar device id for pm */
		int	device_idle_time;    /* time before entering PM idle mode */
		int	device_standby_time; /* time before entering PM idle mode */
		int	pm_attribute; 	/* PM attribute for the device */
		ushort	motor_start;	/* motor start time in milliseconds */
		uchar 	type;		/* drive type */
	#define	D_96	2		/* 96 tpi, (1.2M, 5.25" drive) */
	#define D_135H  8               /* 135 tpi, high density,
						(1.44M, 3.5" drive) */
	#define D_1354H 4               /* 135 tpi, high density,
						(2.88M, 3.5" drive) */
		uchar	head_settle;	/* head settle time in milliseconds */
		uchar	step_rate;	/* step rate time in milliseconds */
		char	resource_name[8];	/* string for error logging */
	} fdd;

	/* used in CFG_QVPD to detect presence of drives */
	uint	drive_info[2];	/* information about the drive uses D_96, */
			/* D_135H, and D_1354H to identify drive type.    */
#define FD_PRESENT      0x100
#define FD_CONFIGURED   0x200
};


/*
 * Diskette minor numbers use the following convention:
 *   
 *
 *                       Diskette Diameter
 *                              |
 *                    Drive     |     
 * Configure          Number    |        Diskette Type
 * Adapter            ______    |    _____________________
 *  |                |      |  | |  |                     |
 *  0    000 0000     0    0    0    0    0    0    0    0
 *
 */

/*
 * Defines for each of the diskette minor device types.            
 */

#define	FDGENERIC	0	/* generic minor device number */
#define	FD1440_3	0x01	/* 3.5 inch, 1.44M diskette */
#define	FD720_3		0x02	/* 3.5 inch, 720K diskette */
#define FD2880_3        0x03    /* 3.5 inch, 2.88M diskette */
#define	FD1200_5	0x21	/* 5.25 inch, 1.2M diskette */
#define	FD360_5		0x22	/* 5.25 inch, 360K diskette */
#define	FD360_5		0x22	/* 5.25 inch, 360K diskette */

#define FD_ADAP_MINOR_NUM	0x8000	/* minor number for diskette adapter */
					/* a flag so config knows it is      */
					/* configuring the adapter and not   */
					/* the drive 			     */
/*
 * Diskette device driver ioctl operations.
 */

#define	FDIOC	('F'<<8)
#define	FDIOCDSELDRV	(FDIOC|1)	/* de-select the drive */
#define	FDIOCFORMAT	(FDIOC|2)	/* format track */
#define	FDIOCGETPARMS	(FDIOC|3)	/* get diskette & drive
						characteristics */
#define	FDIOCGINFO	(FDIOC|4)	/* get drive info */
#define	FDIOCNORETRY	(FDIOC|5)	/* disable retries on errors */
#define	FDIOCREADID	(FDIOC|6)	/* read first address field found */
#define	FDIOCRECAL	(FDIOC|7)	/* recalibrate the drive */
#define	FDIOCRESET	(FDIOC|8)	/* reset diskette controller */
#define	FDIOCRETRY	(FDIOC|9)	/* enable retries on errors */
#define	FDIOCSEEK	(FDIOC|10)	/* seek to designated cylinder */
#define	FDIOCSELDRV	(FDIOC|11)	/* select the drive */
#define	FDIOCSETPARMS	(FDIOC|12)	/* set diskette & drive
						characteristics */
#define	FDIOCSETTLE	(FDIOC|13)	/* check the head settle time */
#define	FDIOCSINFO	(FDIOC|14)	/* set drive info */
#define	FDIOCSPEED	(FDIOC|15)	/* check the diskette rotation speed */
#define	FDIOCSTATUS	(FDIOC|16)	/* get the drive status */

/*
 * The following is the structure used by the FDIOCGINFO and FDIOCSINFO
 * ioctl operations.
 */

struct fdinfo {
	short	type;	/* drive type */
#define	D_96	2	/* 96 tpi, (1.2M, 5.25" drive) */
#define	D_135H	8	/* 135 tpi, high density, (1.44M, 3.5" drive) */
#define D_1354H 4       /* 135 tpi, high density, (2.88M, 3.5" drive) */
	short	info_reserved;
	int	nsects;	/* number of sectors/track */
	int	sides;	/* number of sides */
	int	ncyls;	/* number of cylinders */
};

/*
 * The following is the structure used by the FDIOCSTATUS ioctl
 * system call.                                               
 */

struct fd_status {
	uchar	status1;	/* status byte 1 */
#define	FDNODRIVE	1	/* no drive selected */
#define	FDDRIVE0	2	/* drive 0 selected */
#define	FDDRIVE1	4	/* drive 1 selected */
#define	FD250KBPS	8	/* 250 kbps data transfer rate */
#define	FD300KBPS	16	/* 300 kbps data transfer rate */
#define	FD500KBPS	32	/* 500 kbps data transfer rare */
#define FD1MBPS         64      /* 1000 Mbps data transfer rare */
	uchar	status2;	/* status byte 2 */
#define	FD3INCHHIGH	2	/* 1.44 Megabyte drive */
#define FD3INCHHIGH4M   4       /* 2.88 Megabyte drive */
#define	FD5INCHHIGH	8	/* 1.2 Megabyte drive */
#define	FDRETRY		16	/* retries are enabled */
#define	FDTIMEOUT	32	/* device driver timer expired */
	uchar	status3;	/* status byte 3 */
#define	FDDOUBLE	2	/* doubled-sided diskette */
#define FD36PRTRCK      4       /* 36 sectors per track */
#define	FD9PRTRCK	8	/* 9 sectors per track */
#define	FD15PRTRCK	16	/* 15 sectors per track */
#define	FD18PRTRCK	32	/* 18 sectors per track */
#define	FD40CYLS	64	/* 40 cylinders */
#define	FD80CYLS	128	/* 80 cylinders */
	uchar	command0;	/* command phase byte 0 */
	uchar	command1;	/* command phase byte 1 */
	uchar	mainstat;	/* controller main status register */
	uchar	dsktchng;	/* controller disk changed register */
	uchar	result0;	/* result phase status register 0 */
	uchar	result1;	/* result phase status register 1 */
	uchar	result2;	/* result phase status register 2 */
	uchar	result3;	/* result phase status register 3 */
	uchar	cylinder_num; 	/* cylinder number */
	uchar	head_num;	/* head number */
	uchar	sector_num;	/* sector number */
	uchar	bytes_num;	/* number of bytes per sector */
	uchar	head_settle_time;	/* time in milliseconds */
	uint	motor_speed;	/* time in milliseconds */
	ulong	Mbytes_read;	/* number of Mbytes read since last config */
	ulong	Mbytes_written;	/* number of Mbytes written since last
					config */
	ulong	status_reserved0;
	ulong	status_reserved1;
};

/*
 * The following is the structure used by the FDIOCSETPARMS and the
 * FDIOCGETPARMS ioctl system calls.
 */

struct fdparms {
	uchar	diskette_type;	/* ORed together flags describing type */
	uchar	sector_size;	/* encoded sector size */
	uchar	tracks_per_cylinder;	/* decimal value */
	uchar	data_rate;	/* encoded data rate */
	uchar	head_settle_time;	/* head settle time in milliseconds */
	uchar	head_load;	/* encoded head settle time */
	uchar	fill_byte;	/* hex data pattern */
	uchar	step_rate;	/* encoded step rate time */
	uchar	step_rate_time;	/* step rate in milliseconds */
	uchar	gap;		/* r/w gap */
	uchar	format_gap;	/* format gap */
	uchar	data_length;	/* see data sheet for usage info */
	uchar	motor_off_time;	/* time before motor is turned off (secs) */
	ushort	sectors_per_track;	/* decimal value */
	ushort	sectors_per_cylinder;	/* decimal value */
	ushort	cylinders_per_disk;	/* decimal value */
	ushort	bytes_per_sector;	/* decimal value */
	ushort	number_of_blocks;	/* total number of sectors on the 
						diskette */
	ushort	motor_start;	/* motor start time in milliseconds */
	ushort	motor_ticks;	/* motor start time in timer ticks */
};

#endif /* _H_FD */
