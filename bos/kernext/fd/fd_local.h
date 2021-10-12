/* @(#)66 1.20 src/bos/kernext/fd/fd_local.h, sysxfd, bos411, 9428A410j 6/27/94 17:56:03 */
#ifndef _H_FD_LOCAL
#define _H_FD_LOCAL

/*
 * COMPONENT_NAME: (SYSXFD) Diskette Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * Defines for constants 
 */

#define	FDMAXDRIVES	2	/* maximum number of drives allowed */
#define	FDMAX_SIMPLE_RETRIES1	3	/* maximum # of times to just retry the
						command initially */
#define	FDMAX_COMPLEX_RETRIES	5	/* maximum # of times to retry the
						command after doing a reset */
#define	FDMAX_SIMPLE_RETRIES2	6	/* maximum # of times to just retry the
						command after complex retry */
#ifdef _POWER
#define	FDMAX_LOOP_TIME	50000000	/* # of nanoseconds before timeout */
#define FDMAX_CTLR_WAIT  10000		/* # of nanoseconds before timeout */
#endif

#ifdef _IBMRT
#define	FDMAX_LOOP_TIME	50000	/* # of microseconds before timeout */
#endif

#define	FDDISK_CHANGE	0x80	/* value that indicates the door has been 
					opened */ 

#define	FDMT		0x80	/* multi-track r/w (0x00 off, 0x80 on) */
#define	FDMFM		0x40	/* MFM data encoding scheme
					(0x00 off, 0x40 on) */ 
#define	FDDMA		0x00	/* DMA mode (0x0 on, 0x1 off) */

#define FDNORMAL	0xc0	/* mask for determining if a command 
					completed normally */
#define FDCONFIGSET     0x07    /* setting for configuration params */

#define	FDREAD_MASK	0xC0	/* mask indicating a read operation */
#define	FDWRITE_MASK	0x80	/* mask indicating a write operation */
#define	FDRFM_MASK	0x80	/* mask indicating controller ready */
#define	FDINVALID_MASK	0x80	/* mask indicating invalid command */

#ifdef _POWER
#define FDINT_MASK	0x02	/* mask indicating a diskette interrupt */
#endif

#define	FDTYPEMASK	0x3f	/* mask to delete the drive number */
#define	FDSIZEMASK	0xdf	/* mask to delete the diskette diameter flag */
#define	FDCONTROLMASK	0x0c	/* master DMA and interrupt enable,
					controller not reset */ 
#define	FDRESETMASK	0x08	/* enable DMA and interrupts, controller
					reset */
#define	FDNOINTRESETMASK	0x00	/* disable DMA and interrupts,
				 		controller reset */
#define	FDDISABLEMASK	0x04	/* disable DMA and interrupts, controller not
					reset */ 
#define FDDISMTYPE      0x04    /* disable DMA and interrupts, controller not
                                       reset for Salmon */ 
#define FDDISABLE2      0x00    /*disable for Salmon    */
#define	FDDRIVE_0_MASK	0x10	/* mask to enable drive 0 */

#define	FDNO_DRIVE	0xff	/* used to indicate no drive running */
#define FDDRIVE288	0xc0    /* used to indicate 2.88 meg driver installed */

#define	FDSUCCESS 	0	/* return code for successful subroutine
					completion */ 
#define	FDFAILURE	-1	/* return code for unsuccessful subroutine
					completion */ 
#ifdef _POWER
#define FDPOS0REG       0x00400000
#define FDPOS1REG       0x00400001
#define FDENABLEREG     0x00400002
#define FDARBREG        0x00400004
#endif
#ifdef _IBMRT
#define	FDBUS_DELAY	0x80e0
#endif

/*
 * Diskette Controller Register Offsets
 */

#define FDDRIVE_CONTROL_REG     2       /* write only */
#define FDMAIN_STATUS_REG       4       /* read only  */
#define FDDISKETTE_DATA_REG     5       /* read/write */
#define FDDATA_RATE_REG         7       /* write only */
#define FDDISK_CHANGED_REG      7       /* read only  */
#define FDTHRES_FIFO_REG        8       /* read/write */
/*
 * Diskette interrupt handler states
 */

#define	FD_NO_STATE		0
#define FD_TYPE1_WAKEUP		1
#define FD_TYPE2_WAKEUP		2
#define FD_FORMAT		3
#define FD_SPEED		4
#define FD_SETTLE		5
#define FD_RW			6
#define FD_INITIAL_INTERRUPT	7
#define FD_IO_RESET		8

/*
 * Diskette interrupt handler sub-states
 */

#define FD_SPEED_READ1		1
#define FD_SPEED_READ2		2
#define FD_SETTLE_READ1		3
#define FD_SETTLE_SEEK		4
#define FD_SETTLE_WRITE		5
#define FD_SETTLE_READ2		6
#define FD_RW_SEEK1		7
#define FD_RW_RECAL1		8
#define FD_RW_RECAL2		9
#define FD_RW_RECAL3		10
#define FD_RW_SEEK2		11
#define FD_RW_DRCHK_RECAL	12
#define FD_RW_DRCHK_SEEK	13
#define FD_RW_SUB		14
#define FD_RW_RESET		15

/*
 * Diskette timer sub-states
 */

#define FD_SETTLE_TIMER		32
#define FD_RW_DELAY_TIMER	33

/*
 * Macros for writing and reading data to and from the diskette controller
 */

#define	FDWRITE(offset, data)  BUSIO_PUTC((caddr_t)(offset), data)
#define	FDREAD(offset) BUSIO_GETC((caddr_t)(offset))

/*
 * Structures to store debug information in memory.
 */

#ifdef DEBUG

#define FDTRACE_SIZE 	0x1000
struct	fdtrace_entry {
	union {
		struct {
			char	tag[4];
			ulong	data;
			caddr_t	address;
			ulong	time;
		} names;
		struct {
			uchar	header[12];
			dev_t	devno;
		} routines;
		char	array[16];
	}un;
}; 

struct fdtrace_struct {
	char	start[16];
	struct	fdtrace_entry	trace_buffer[FDTRACE_SIZE];
	char	end[16];
	long	current;
#ifdef _POWER_MP
        Simple_lock trace_lock;  /* trace lock */
#endif /* POWER_MP */
};


#define FD_TRACE1(s, d)		fd_trace1(s, d)
#define FD_TRACE2(s, d, a, t)	fd_trace2(s, d, a, t)

#define FDA_DEV  fdadapter->drive_list[fdadapter->active_drive]->device_number
#define FDP_DEV  fdp->device_number

#else

#define FD_TRACE1(s, d)
#define FD_TRACE2(s, d, a, t)

#endif

/* these printf's are used to debug fd_config () */
#ifdef  PRTDEBUG

#define DEBUG_0(str)            printf (str)
#define DEBUG_1(str, p1)        printf (str, p1)
#define DEBUG_2(str, p1, p2)    printf (str, p1, p2)

#else

#define DEBUG_0(str)
#define DEBUG_1(str, p1)
#define DEBUG_2(str, p1, p2)

#endif


/* 
 * Structure for storing the physical address of data.
 */

struct phys_add {
	uchar cylinder;
	uchar head;
	uchar sector;
	uint  transfer_length;	/* length (in bytes) of the current transfer */
};

/*
 * Structure to store the result phase data from a diskette controller
 * operation before storing the data in the floppy structure.
 */

struct result_bytes {
	uchar total_bytes;	/* number of bytes in this result */
	union {
		struct
		 {
			union {
				uchar status0;	/* all reads, write, format
							 track, sense int */
				uchar status3;	/* sense drive status */
				uchar value;	/* set track */
			} byte0;
			union {
				uchar status1;	/* all reads, write,
						 	format track */
				uchar present_track;	/* sense interrupt */
			} byte1;
			uchar status2;	/* all reads, write, format track */
			uchar track;	/* all reads, write, format track */
			uchar head;	/* all reads, write, format track */
			uchar sector;	/* all reads, write, format track */
			uchar sector_size;	/* all reads, write,
							 format track */
		} names;
		uchar result_array[7];
	} un1;
};

/* 
 * Structure to build a command phase in before sending it to the
 * diskette controller.
 */

struct command_bytes {
	uchar total_bytes;	/* number of bytes in this command */
	union {
		struct
		 {
			uchar command1;	/* all commands, identifies command */
#define	FDREAD_DATA		0x06
#define	FDREAD_ID		0x0a
#define	FDFORMAT_TRACK		0x0d
#define	FDWRITE_DATA		0x05
#define	FDSEEK			0x0f
#define	FDRECALIBRATE		0x07
#define	FDSENSE_INTERRUPT	0x08
#define	FDSENSE_DRIVE_STATUS	0x04
#define FDSPECIFY               0x03
#define FDPERPENDICULAR         0x12
#define FDCONFIG                0x13

/*
 * The following commands do not use the controller data register.
 */

#define	FDDISABLE_CONTROLLER	0xaf
#define	FDENABLE_CONTROLLER	0xbf
#define	FDDRIVE_SELECT		0xcf
#define	FDSET_DATA_RATE		0xdf
#define	FDSOFT_RESET		0xef

#ifdef	_POWER
#define	FDHARD_RESET		0xff
#endif

			uchar command2;		/* all commands */
			union
			 {
				uchar track;		/* read data, write
								 data, seek */
				uchar sector_size;	/* format track */
				uchar motor_time;	/* specify */
				uchar config_set;       /* config setting */
			} byte2;
			union
			 {
				uchar head;		/* read data,
								 write data */
				uchar sectors_per_track;/* format track */
				uchar config_set;       /* config setting */
			} byte3;
			union
			 {
				uchar sector;		/* read data,
								 write data */
				uchar gap_length;	/* format track */
			} byte4;
			union
			 {
				uchar sector_size;	/* read data,
								 write data */
				uchar data_pattern;	/* format track */
			} byte5;
			uchar eot_sector;	/* read data, write data */
			uchar gap_length;	/* read data, write data */
			uchar data_length;	/* read data, write data */
		} cmds;
		uchar command_array[9];
	} un1;
};


/*
 * The following is the structure that is used to pass information
 * between the different parts of the device driver.
 */

struct floppy {
	dev_t	device_number;	/* save off the 'devno' for later use */
	uchar	initialized;	/* is diskette drive initialized? */
	uchar	drive_state;	/* current state of the drive */
#define	FDOPEN		1
#define	FDOPENING	2
#define	FDCLOSED	4
#define	FDCLOSING	8
	uchar	first_move;	/* is this the first move of the drive
					heads after opening? */
	uchar   drive_type;     /* type of diskette drive, uses same
					defines as fdinfo structure. */
	uchar	diskette_type;	/* type of diskette in the drive */
#define	FDUNKNOWN	0
	uchar	sector_size;	/* encoded number of bytes in a sector */
	uchar	tracks_per_cylinder;	/* number of tracks per cylinder */
	uchar	step_size;	/* 1 or 2 depending on drive/diskette 
					combination */
	uchar	data_rate;	/* data rate setting for diskette type */
	uchar	head_load;	/* head load time for diskette type */
	uchar	head_unload;	/* head unload time for diskette type */
	uchar	step_rate;	/* step rate for diskette type */
	uchar	step_rate_time;	/* step rate for diskette type
					(in milliseconds) */
	uchar	gap;		/* inter-sector gap length */
	uchar	format_gap;	/* format inter-sector gap length */
	uchar	retry_flag;	/* 0 = disabled, anything else = enabled */
	uchar	fill_byte;	/* fill byte for formmating */
	uchar	cylinder_id;	/* cylinder number for SEEK ioctls */
	uchar	head_id;	/* head number for READ_ID ioctls */
	uchar	head_settle_time;	/* head settle time (milliseconds) */
	uchar	data_length;	/* data length controller parameter */
	uchar	last_error1;	/* last error type that failed retries (status
					register 1 value) */
	uchar	last_error2;	/* last error type that failed retries (status
					register 2 value) */
	uchar	motor_off_time;	/* inactive time before motor turned off 
					(seconds) */
	ushort	format_size;	/* size of format buffer */
	ushort	sectors_per_track;	/* number of sectors per track */
	ushort	sectors_per_cylinder;	/* number of sectors per cylinder */
	ushort	cylinders_per_disk;	/* number of cylinders on the disk */
	ushort	bytes_per_sector;	/* raw sector size */
	ushort	number_of_blocks;	/* total number of blocks on the
						disk */
	ushort	motor_start;	/* motor start time (in milliseconds) */
	ushort	motor_ticks;	/* motor start time (in timer ticks) */
	ushort	start_block;	/* starting block # for read or write */
	int	dma_flags;	/* flags passed to the DMA services */
	uint	total_bcount;	/* total number of bytes transfered so far */
	uint	modified_bcount;	/* modified byte count for transfers */
	uint	simple_count1;	/* first simple retry count for the i/o
					operation */
	uint	complex_count;	/* complex retry count for the i/o operation */
	uint	simple_count2;	/* second simple retry count for the i/o
					operation */
	uint	buf_offset;	/* the offset in the data buffer for 
					this io */
	uint	motor_speed;	/* motor speed (rpm) */
	uint	motor_speed_time;	/* motor speed (microseconds) */
	ulong	settle_delay;	/* delay used during settle test
					(nanoseconds) */
	ulong	rcount_bytes;	/* the next four variables keep */
	ulong	rcount_megabytes;	/* track of the number of bytes */
	ulong	wcount_bytes;	/* transfered since the drive was */
	ulong	wcount_megabytes;	/* last configured */
	struct	buf *headptr;	/* pointer to first queued buffer header */
	struct	buf *tailptr;	/* pointer to last queued buffer header */
	struct	xmem xmem;	/* cross memory descriptor */
	struct	phys_add start;	/* starting address for read/write for
					the requested block */
	struct	phys_add current;/* current starting address for the io
					this is used to keep track of where
				 	an intermediate io operation started
					during large read or write requests */
	char	resource_name[8];	/* name for error logging */
#ifdef _POWER_MP
        Simple_lock intr_lock;  /* interrupt lock */
#endif /* POWER_MP */
};

/*
 * Diskette error logging structure.
 */

struct fd_err_rec {
	struct err_rec0 header;
	struct fd_status data;
};

/* 
 * Structure used for passing parameters to pio_assist routines.
 */

struct pio_parms {
	volatile uchar	data;		/* data byte */
	char		read;		/* is this a read? */
	ulong		reg;		/* i/o register offset */
	caddr_t		bus_val;	/* bus id value */
};

/* 
 * structure to keep config parameters around to reset to if needed.
 */

struct fdconfig_parameters {
	uchar  head_settle;
	uchar  step_rate;
	ushort motor_start;
	ushort type;
};

/* 
 * Structure describing controller (global) status.
 */

struct adapter_structure {
	uchar	adapter_busy;	/* is the interrupt handler busy? */
	uchar	active_drive;	/* valid values are 0 or 1 */
	uchar	motor_on;	/* which drive motor is on? */
	uchar	data_rate;	/* encoded data rate in use */
	uchar	int_class;	/* diskette interrupt class */
	uchar	slot_num;	/* diskette adapter slot number */
	uchar	initialized;	/* are adapter structures initialized? */
	uchar	pinned;		/* is the adapter structure pinned? */
	uchar	int_init;	/* is the interrupt handler defined? */
	uchar	dma_init;	/* is the dma channel defined? */
	uchar	reset_needed;	/* a reset is needed */
	uchar   reset_active;   /* a reset is active */
	uchar	reset_performed;	/* has a reset been done */
	uchar	d_comp_needed;	/* is a d_complete needed? */
	uchar	first_open;	/* is this the first open after config? */
	uchar  	adapter_type;   /* defines adapter type         */
	ushort	bus_type;	/* type of i/o bus */
	ushort	state;		/* interrupt state */
	ushort	sub_state;	/* interrupt sub-state */
	int	sleep_anchor;	/* event word for waiting on interrupts */
	int	adapter_sleep_anchor;	/* event word for adapter lock */
	int	dma_level;	/* DMA arbitration level */
	int	bus_int_level;	/* diskette bus interrupt level */
	int	dma_id;		/* DMA channel id */
	int	error_value;	/* error value set at interrupt level */
	uint	adapter_id;	/* adapter type id (used by RAS) */
	ulong	bus_id;		/* io bus id */
	ulong	actual_delay;	/* actual time delayed for settle ioctl */
	ulong	fudge_factor;	/* compensation value for settle ioctl */
	ulong	io_address;	/* start of diskette io addresses */
	ulong	data_reg_excess;	/* number of times data register delay
						exceeded 50 microseconds */
	char	*format_buffer;	/* pointer to buffer with format info */
	char	*speed_buffer;	/* pointer to buffer for speed and settle
					ioctls */
	struct	intr *fdhandler;	/* pointer to interrupt handler
					structure */
	struct	trb *fdstart_timer;	/* timer structure for interrupt level 
						delays */
	struct	trb *fdsettle_timer;	/* timer structure for head settle
						test */
	struct	fd_err_rec *fderrptr;	/* pointer to error log structure */
	struct	xmem xdp;		/* xmem descriptor for speed and settle
						ioctls */
	struct	timestruc_t start_time;
	struct	timestruc_t end_time;
	struct	watchdog inttimer;	/* interrupt watchdog timer
					 	structure */
	struct	watchdog mottimer;	/* motor watchdog timer structure */
	struct	command_bytes command;	/* last command executed */
	struct	result_bytes results;	/* results of the last executed
						command */
	struct	floppy *drive_list[FDMAXDRIVES];	/* list of pointers to
								device
								structures */
	struct fdconfig_parameters *fdconfparms[FDMAXDRIVES];
#ifdef DEBUG
	struct fdtrace_struct	*trace_table;
	struct fdtrace_entry	trace;
#endif
};


/*
 * Top half device driver entry points.
 */

#ifndef _NO_PROTO
int	fd_config( 
	dev_t devno,
	int cmd,
	register struct uio *uiop
);

int	fd_open(
	dev_t devno,
	ulong devflag
);

int	fd_close(
	dev_t devno
);

int	fd_read(
	dev_t devno,
	register struct uio *uiop
);

int	fd_write(
	dev_t devno,
	register struct uio *uiop
);

int	fd_ioctl(
	dev_t devno,
	int op,
	register long arg,
	ulong devflag
);

#else

int	fd_config();

int	fd_open();

int	fd_close();

int	fd_read();

int	fd_write();

int	fd_ioctl();
#endif

/*
 * Additional top half internal device driver routines.
 */

#ifndef _NO_PROTO
int	fddoor_check();

int	fdtype();

int	nodev();

int	fdconfig_vpd(
	register struct floppy *fdp,
	register union  fd_config *vpdptr,
	uchar drive_number
);

int	fdconfig_reset(
	register struct pio_parms *piop
);

int	fdconfig_soft_reset(
	register struct pio_parms *piop
);

int	fdconfig_specify(
	register struct floppy *fdp,
	register struct pio_parms *piop
);

int	fdconfig_motor_start(
	register struct floppy *fdp,
	register struct pio_parms *piop,
	uchar drive_number
);

int     fdconfig_read_threshold(
	register struct pio_parms *piop
);

int     fdconfig_motor_stop(
	register struct pio_parms *piop
);

int	fdconfig_recalibrate(
	register struct floppy *fdp,
	register struct pio_parms *piop,
	uchar drive_number
);

int	fdconfig_sense_interrupt(
	register struct floppy *fdp,
	register struct pio_parms *piop
);

int	fdconfig_seek(
	register struct floppy *fdp,
	register struct pio_parms *piop,
	uchar drive_number
);

void	fd_qvpd_exit(
	register union  fd_config *vpdptr,
	register struct iovec *localuio_iovec
);
#else

int	fddoor_check();

int	fdtype();

int	nodev();

int	fdconfig_vpd();

int	fdconfig_reset();

int	fdconfig_soft_reset();

int	fdconfig_specify();

int	fdconfig_motor_start();

int	fdconfig_motor_stop();

int     fdconfig_read_threshold();

int	fdconfig_recalibrate();

int	fdconfig_sense_interrupt();

int	fdconfig_seek();

void	fd_qvpd_exit();
#endif

/*
 * Bottom half device driver entry points.
 */

#ifndef _NO_PROTO
int	fd_strategy(
	register struct buf *bp
);

int	fd_intr();

#else

int	fd_strategy();

int	fd_intr();
#endif

/*
 * Additional bottom half internal device driver routines.
 */

#ifndef _NO_PROTO
uchar	fdread_change_line();

uchar	fdread_main_status();

int     fdload_floppy(
	int value, 
	register struct floppy *fdp
);

int	fdrw_exit(
	register struct floppy *fdp,
	int errno_value
);

int	fdio_exit(
	register struct floppy *fdp,
	int errno_value
);

int	fdiocformat(
	long arg
);

void	fdformat_track();

int	fddata_reg_pio(
	caddr_t parms
);

int	fdcfg_data_pio(
	caddr_t parms
);

int	fdlock_adapter(
	int drive_number
);

void	fdunlock_adapter(
	int drive_number
);

void	fdtop_unlock_adapter(
	int drive_number
);

int	fddata_reg_pio_recovery(
	caddr_t parms,
	int action
);

int	fdcfg_data_pio_recovery(
	caddr_t parms,
	int action
);

int	fdcntl_reg_pio(
	caddr_t parms
);

int	fdcfg_cntl_pio(
	caddr_t parms
);

int	fdcntl_reg_pio_recovery(
	caddr_t parms,
	int action
);

int	fdcfg_cntl_pio_recovery(
	caddr_t parms,
	int action
);

void	fdmotor_start();

void	fdreset_check();

void	fdio(
	register struct buf *bp
);

void	fdsettle_timer_handler(
	register struct trb *settle_timer
);

void	fdlog_error(
	register struct floppy *fdp,
	uchar value
);

void	fdtimeout(
	register struct trb *fdstart_timer_ptr
);

void	fdwatchdog(
	register struct watchdog *timer
);

void	fdmotor_timer(
	register struct watchdog *timer
);

int	fdiocrecal();

void	fdrecalibrate();

int	fdiocseek();

void	fdseek();

int	fdiocspeed();

int	fdiocsettle(
	long arg
);

void	fdrw();

void	fdrw_ioctl();

int	fdiocreset();

void	fdreset();

void	fdset_data_rate();

void	fdselect_drive();

void	fdtop_select_drive();

void	fdenable_controller();

void	fdtop_enable_controller();

void	fddisable_controller();

void	fdsoft_reset();

void	fdsense_interrupt();

void	fdsense_drive_status();

void	fdspecify();

void    fdset_perp();

void    fdset_config();

void	fdreadid();

void	fdassign_error(
	register struct floppy *fdp
);

int	fdexecute_int_cmd(
	void (*func)()
);

void	fdqueue_check(
	register struct floppy *fdp, 
	int drive_number
);

#ifdef DEBUG
void	fd_trace1(
	char *string,
	dev_t devno
);

void	fd_trace2(
	char *string,
	ulong data,
	caddr_t address,
	ulong time
);

void	fdmove_trace();
#endif

#else

int	fdlock_adapter();

void	fdunlock_adapter();

void	fdtop_unlock_adapter();

uchar	fdread_change_line();

uchar	fdread_main_status();

int	fdiocformat();

void	fdformat_track();

int	fdload_floppy();

int	fdrw_exit();

int	fdio_exit();

int	fddata_reg_pio();

int	fdcfg_data_pio();

int	fddata_reg_pio_recovery();

int	fdcfg_data_pio_recovery();

int	fdcntl_reg_pio();

int	fdcfg_cntl_pio();

int	fdcntl_reg_pio_recovery();

int	fdcfg_cntl_pio_recovery();

void	fdmotor_start();

void	fdreset_check();

void 	fdio();

void	fdsettle_timer_handler();

void	fdlog_error();

void	fdtimeout();

void	fdwatchdog();

void	fdmotor_timer();

int	fdiocrecal();

void	fdrecalibrate();

int	fdiocseek();

void	fdseek();

int	fdiocspeed();

int	fdiocsettle();

int	fdiocreset();

void	fdreset();

void	fdset_data_rate();

void	fdselect_drive();

void	fdtop_select_drive();

void	fdenable_controller();

void	fdtop_enable_controller();

void	fddisable_controller();

void	fdsoft_reset();

void	fdsense_interrupt();

void	fdsense_drive_status();

void	fdspecify();

void    fdset_perp();

void    fdset_config();

void	fdreadid();

void	fdrw();

void	fdrw_ioctl();

void	fdassign_error();

int	fdexecute_int_cmd();

void	fdqueue_check();

#ifdef DEBUG
void	fd_trace1();

void	fd_trace2();

void	fdmove_trace();
#endif
#endif

#endif /* _H_FD_LOCAL */
