/* @(#)77       1.6.1.1  src/bos/kernel/io/machdd/pgs_novram.h, machdd, bos41J, 9511A_all 3/14/95 03:30:40 */
/*
 * COMPONENT_NAME:  (MACHDD) Machine Device Driver
 * 
 * FUNCTIONS:  PEGASUS NOVRAM
 * 
 * ORIGINS: 83 
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 */


/****************************************/
/****************************************/
/*					*/
/*	NOVRAM  MEMORY  MAPPING		*/
/*					*/
/****************************************/
/****************************************/



/****************************************/
/****************************************/
/*					*/
/*	BOOT CONTROL STRUCTURE		*/
/*					*/
/****************************************/
/****************************************/

struct	boot_control
	{
	int	res[192] ;
	} ;


/****************************************/
/****************************************/
/*					*/
/*	RISC COMPATIBLE STRUCTURE	*/
/*					*/
/****************************************/
/****************************************/

/********************************/
/*	ASIC READ STATUS	*/
/********************************/

struct	CPU_status
	{
	uchar	res[2] ;		/* reserved		*/
	ushort	CCA2_status ;		/* CCA2 status		*/
	uchar	CCD2_status[4] ;	/* CCD2 #0-3 status	*/
	} ;

struct	ASICS_status
	{
	uint	SMC_status[3] ;		/* SMC general status	*/
			/* SMC simgle and multiple error memory address */
	uint	SMC_res ;		/* SMC reserved	*/
	uchar	DCB_status[4][8] ;	/* DCBx status, syndromes and chip_id */
	struct	CPU_status cpu[4] ;	/* CPU boards status	*/
	uint	res50[4] ;		/* reserved	*/
	} ;

struct	COP_status
	{
	char ionian[2][2];		/* cop IONIAN status	*/
	char cpu[8][2000];		/* cop cpu status	*/
	} ;

struct cks_status
	{
	struct	ASICS_status ASICS_status;
	struct COP_status COP_status;
	};

struct checkstop
	{
	uint	header[4];		/* header		*/
	struct	cks_status status[3];
	uint	res[209];
	char	scan[16384];		/* scan latches state	*/
	};

struct	risc_compatible
	{
	uint	led;			/* LED data Mirrored ( 3 digit ) */
	uint	res1;
	uint	checkstop_count;
	struct	cks_status *pt_checkstop;
	uchar	bump_code_level[4];
	uchar	bump_eprom_level[4];
	uchar	man_control[4];		/* manufacturing Control	*/
	uint	man_data;		/* manufacturing data pointer	*/
	uchar	led_reset[64];		/* LED string Output Area	*/
	uchar	*pt_code;		/* pointer to bump code		*/
	uchar	*pt_work;		/* pointer to bump work area	*/
	uchar	check_save[20];		/* Machine Check Save Area	*/
	uint	bump_cmd;		/* Bump command Interface	*/
	uchar	bump_data[128];		/* Bump Information Area (Sys. VPD) */
	uchar	bump_ipl_rom[7*1024];	/* Bump IPL ROM usage		*/
	uchar	res_bump[9*1024];	/* reserved for Bump		*/
	uchar	soft_area[15*1024];	/* Dynamic. controlled software area*/
	struct	checkstop checkstop ;
	} ;



/****************************************/
/****************************************/
/*					*/
/*	CONFIGURATION TABLE STRUCTURE	*/
/*					*/
/****************************************/
/****************************************/


/********************************/
/*	System Area		*/
/********************************/

struct	sys_param
	{
	ushort	structure_ID ;
	ushort	res1[3] ;		/* reserved	*/
	uint	flags ;

		/* Bit	0 msb	= System configuration changes	(1=changes)
			1	= Remote console en/disabled	(1=enable)
			2	= Auto serv IPL permission
			3	= BUMP local console present	(1=present)
			4	= BUMP remote console present	(1=present)
			5-7	= Remote line speed :
					000 = 300
					001 = 600
					010 = 1200	(default value)
					011 = 2400
					100 = 4800
					101 = 9600
					110 = 19200
					111 = not used
			8	= MFG enviroment
			9	= Customer line Power-on command en/disable
					(1=enable)
			10	= Remote line Power-on command en/disable
					(1=enable)
			11-27	= Not used
			28-31	= Language type :
					0000 = not used
					0001 = English
					0010 = German
					0011 = Spanish
					0100 = French
					0101 = Swedish
					0110 = Norwegian
					0111 = Belgium
					1000 = Italian
					1001 <---> 1111 = reserved */

	uchar	local_pw_on_cmd[16] ;
	uchar	remote_pw_on_cmd[16] ;
	uint	customer_pwd ;
	uint	general_pwd ;
	uint	res2[20] ;		/* reserved	*/
	} ;

struct	configuration
	{
	uchar	cab_list1 ;	/* Cabinet list for present/absent cabinet */
				/* (1=present) N.B. : Bit 0 = cab.0 */
	uchar	cab_list2 ;	/* Cabinet list for config./deconfig. cabinet */
				/* (1=configured) N.B. : Bit 0 = cab.0 */
	ushort	res ;
	struct
		{
		uchar	rds1 ;
		uchar	rds2 ;	/* Note : for cabinet 0 this field is	*/
				/*	egal to rds1 i/o port 7	*/
		uchar	marg_val ;
		/*
		* For cabinet 0 Main cabinet *
		******************************
		* Bit	0 msb	= +5 Volt Marginate to -3% (bit = 0)
			1	= +5 Volt Marginate to -2%	"
				Note: Bit 0-1 both = 0 (+5 Volt Marg. to -5%)
			2	= +5 Volt Marginate to +2% (bit = 0)
			3	= +5 Volt Marginate to +3%	"
				Note: Bit 2-3 both = 0 (+5 Volt Marg. to +5%)
			4	= Asic Marginate to +4% (bit = 0)
			5	= Asic Marginate to -4%	"
			6	= Cpu Marginate to +4%	"
			7 lsb	= Cpu Marginate to -4%	"

		* For cabinets 1 to 7 *
		***********************
		* Bit	0 msb	= +5 Volt Marginate to -3% (bit = 0)
			1	= +5 Volt Marginate to -2%	"
				Note: Bit 0-1 both = 0 (+5 Volt Marg. to -5%)
			2	= +5 Volt Marginate to +2% (bit = 0)
			3	= +5 Volt Marginate to +3%	"
				Note: Bit 2-3 both = 0 (+5 Volt Marg. to +5%)
		*/

		uchar	res;
		} cab[8] ;
	} ;

struct	system
	{
	uchar	sid[256] ;
	struct	sys_param	sys_par ;
	struct	configuration	config ;
	} ;



/********************************/
/*	Basic Cabinet Area	*/
/********************************/

struct	basic_cabinet
	{
	uchar	sif1[256] ;
	uchar	mvr[256] ;

	uint	ps_status_0 ;	/* Byte 0	= BIST result
					1	= SIF i/o port 0
					2-3	= RFU */
	uint	ps_status_1 ;	/* Byte 0	= RDS1 i/o port 1
					1	= RDS1 i/o port 2
					2	= RDS1 i/o port 3
					3	= RDS1 i/o port 7 */
	uint	ps_status_2 ;	/* Byte 0	= MVR i/o port 4
					1	= MVR i/o port 5
					2	= MVR i/o port 6
					3	= RFU */
	uint	ps_status_3 ;	/* Byte 0	= RDS2 i/o port 1
					1	= RDS2 i/o port 2
					2	= RDS2 i/o port 3
					3	= RFU */

	char	*p_pma_vpd ;
	} ;

/********************************/
/*	Expansion Cabinet Area	*/
/********************************/

struct	expansion_cabinet
	{
	uchar	bp[256] ;
	uchar	sif1[256] ;
	uchar	mvr[256] ;
	uchar	sif2[256] ;
	uchar	res[256] ;		/* Reserved	*/

	uint	ps_status_0 ;	/* Byte 0	= BIST result
					1	= SIF i/o port 0
					2-3	= RFU */
	uint	ps_status_1 ;	/* Byte 0	= RDS1 i/o port 1
					1	= RDS1 i/o port 2
					2	= RDS1 i/o port 3
					3	= RDS1 i/o port 7 */
	uint	ps_status_2 ;	/* Byte 0	= MVR i/o port 4
					1	= MVR i/o port 5
					2	= MVR i/o port 6
					3	= RFU */
	uint	ps_status_3 ;	/* Byte 0	= RDS2 i/o port 1
					1	= RDS2 i/o port 2
					2	= RDS2 i/o port 3
					3	= RFU */

	char	*p_pme_vpd ;
	} ;

/********************************/
/*	Operator Panel Area	*/
/********************************/

struct	operator_panel
	{
	uchar	op[256] ;
	uint	op_status ;
	} ;



/****************************************/
/*	Mother Board Area		*/
/*	IOD Board Area			*/
/*	CPU Daughter Board Area		*/
/*	Memory Daughter Board Area	*/
/****************************************/

struct	native_agent
	{
	uchar	board_vpd[256] ;
	union	{
		uint	full;
		uchar	octet[4] ;
		}status;

	/* byte 0 = Agent,CPU or MRB bank 0
		1 = CPU or MRB bank 1
		2 =	MRB bank 2
		3 =	MRB bank 3

	Bit	0 msb	= Agent,CPU or MRB bank present (1=present)
		1	= Agent,CPU or MRB bank valid (1=valid; test OK)
		2	= Agent,CPU or MRB bank SW disable
			(1=disable by maint menu or by software)
		3	= Agent,CPU or MRB bank temporary disable
			(1=automaticaly enable at next reset)
		4	= Agent validated (1=agent configuration validated
			into the TBConf by the operator or automaticaly
			if the operator don't answer to the configuration
			validation request)
		5	= Agent changed (1=the agent was changed on this slot
			since the last configuration validation and one agent
			was present on this slot at the last validation)
		6-7	= Reserved	*/

	ushort	bist_post_result ;	/* Agent MRB = SIMM status */
	ushort	res ;
	uint	*p_agent_specific ;
	} ;

/****************************************/
/*	Basic MCA Backpanel VPD		*/
/*	External MCA Backpanel VPD	*/
/****************************************/

struct	mca_backpanel_vpd
	{
	uchar	pm[256] ;
	} ;

/****************************************/
/*	Basic and Extension MCA agent	*/
/****************************************/

struct	mca_agent_conf
	{
	uchar	pos_reg[8] ;
	ushort	addressing ;
	ushort	status ;

		/* Bit	0 msb	= Agent present (1=present)
			1	= Agent valid (1=valid; test OK)
			2	= Agent SW disable
				(1=disable by maint menu or by software)
			3	= Agent temporary disable
				(1=automaticaly enable at next reset)
			4	= Agent validated (1=agent configuration
				validated into the TBConf by the operator or
				automaticaly if the operator don't answer to
				the configuration validation request)
			5	= Agent changed (1=the agent was changed on
				this slot since the last configuration
				validation and one agent was present on
				this slot at the last validation)
			6-15	= Reserved	*/

	ushort	bist_post_result ;
	ushort	res ;			/* reserved	*/
	} ;

/************************/
/*	Adaptor VPD	*/
/************************/

struct	adaptor_vpd
	{
	uchar	adapt[256] ;
	} ;



/********************************/
/********************************/
/*				*/
/*	Configuration Table	*/
/*				*/
/********************************/
/********************************/

struct	config_table
	{
	struct	system			sys ;
	struct	basic_cabinet		basic_cab ;
	struct	expansion_cabinet	expans_cab[7] ;
	struct	operator_panel		op ;
	struct	native_agent		mpb ;
	struct	native_agent		iod ;
	struct	native_agent		cpu[4] ;
	struct	native_agent		mem[4] ;
	struct	mca_backpanel_vpd	bas_pm_vpd ;
	struct	mca_agent_conf		bas_mca_ag[8] ;
	struct	mca_backpanel_vpd	ext_pm_vpd ;
	struct	mca_agent_conf		ext_mca_ag[8] ;
	struct	adaptor_vpd		adpt_vpd[2] ;
	uchar	res[2104] ;		/* reserved	*/
	uint	checksum;		/* checksum	*/
	} ;

/************************************************************************/
/*									*/
/* RACK model :  SMP drawer configuration is contained in basic_cabinet */
/* and 1st expansion_cabinet structures, i.e :				*/
/* basic_cab contains SIB vpd (sif1) and 1st Power Supply vpd (mvr)	*/
/* expans_cab[1] contains 2nd Power Supply vpd (mvr)			*/
/*									*/
/************************************************************************/


/****************************************/
/****************************************/
/*					*/
/*	ERROR LOGGING STRUCTURE		*/
/*					*/
/****************************************/
/****************************************/

struct	err_log_header
	{
	uint	size_offset ;	/* bits 0-7 = current table size in Kbytes */
				/* bits 8-31 = next table offset (0 = last) */
	uint	storage_size ;	/* messages storage area size in bytes	*/
	uint	begin_ptr ;	/* pointer on first valid error message	*/
	uint	end_ptr ;	/* pointer on last valid error message	*/
	uint	current_ptr ;	/* pointer for next error message	*/
	uint	checksum ;	/* checksum on valid messages area	*/
	uint	res[2] ;
	} ;

struct	error_logging
	{
	struct	err_log_header	header ;	/* error logging header	*/
	uchar	err_messages[16352] ;		/* error messages	*/
	} ;



/****************************************/
/****************************************/
/*					*/
/*	CPU/BUMP INTERFACE STRUCTURE	*/
/*					*/
/****************************************/
/****************************************/


/********************************/
/*	Interface header	*/
/********************************/

struct	interf_header
	{
	uint	tp_header_adr ;		/* Transport header rel. addr.	*/
	uint	tp_fifo_adr ;		/* Transport FIFOs rel. addr.	*/
	uint	tp_buffer_adr ;		/* Transport buffer rel. addr.	*/
	uint	tp_buffer_lg ;		/* Transport buffer length	*/
	uint	op_interf_adr ;		/* Operator panel interf. addr	*/
	uint	op_interf_lg ;		/* Operator panel interf. length*/
	uint	err_log_adr ;		/* Error log. Table rel. addr.	*/
	uint	err_log_lg ;		/* Error log. Table length	*/
	uint	VPD_conf_adr ;		/* VPD/Config Table rel. addr.	*/
	uint	int_test_adr ;		/* Interface Test rel. addr.	*/
	uint	res[5] ;		/* reserved	*/
	uint	version ;		/* CPU/BUMP interface version	*/
	} ;

/********************************/
/*	Transport header	*/
/********************************/

struct	tp_fifo_header
	{
	uint	command_in ;		/* Command fifo in	*/
	uint	command_out ;		/* Command fifo out	*/
	uint	status_in ;		/* Status fifo in	*/
	uint	status_out ;		/* Status fifo out	*/
	uint	length ;		/* FIFO length		*/
	uint	res14[3] ;		/* reserved	*/
	uint	rsr ;			/* Reset status register	*/
	uint	pksr ;			/* Power/keylock status register*/
	uint	prcr ;			/* Power reset control register	*/
	uint	src ;			/* System reset count	*/
	uint	res30[4] ;		/* reserved	*/
	} ;



/********************************/
/*	Operator Panel interf	*/
/********************************/

#define LCD_SZ 32                     /* LCD string size */

struct	op_interf
	{
	uint	physic_key ;		/* Physical key state	*/
	uint	electro_key ;		/* Electronic key state	*/
	uint	res8 ;			/* reserved		*/
	uint	power_switch ;		/* Power Switch state	*/
	uint	op_panel_state ;	/* Operator Panel state	*/
	uint	remote_val ;		/* Remote Console flag	*/
	uint	res18[9] ;		/* reserved		*/
	uint	man_start ;		/* "manufacturing" start flag	*/
	uchar	LCD_str_new[LCD_SZ] ;	/* Op. LDC string new	*/
	uchar	LCD_str_old[LCD_SZ] ;	/* Op. LDC string old	*/
	} ;

/********************************/
/*	CPUs start area		*/
/********************************/

struct	cpu_start
	{
	uchar	mode_flag ;		/* 00 - Start in EPROM or FLASH	*/
	uchar	cpu_number ;		/* 01 - Started CPU number	*/
	ushort	memory_size ;		/* 02 - Memory size in Mega	*/
	uint	firm_version ;		/* 04 - Firmware Version	*/
	uint	toc_bss_area ;		/* 08 - TOC and BSS area	*/
	uint	res0C ;			/* 0C - reserved	*/
	uint	start_address ;		/* 10 - CPU start address	*/
	uint	iplcb_ptr ;		/* 14 - IPLCB pointer		*/
	uint	res18 ;			/* 18 - reserved	*/
	uint	HID0_reg ;		/* 1C - HID0 forced bits	*/
	uchar	start_flag[8] ;		/* 20 - CPUs start flags	*/
	uint	res28[6] ;		/* 28 - reserved	*/
	uint	res40[8] ;		/* 40 - reserved	*/
	} ;

/********************************/
/*	Config/VPD ptrs Table	*/
/********************************/

struct	config_VPD_pt
	{
	uint	system_config ;		/* System Configuration ptr	*/
	uint	basic_config ;		/* Basic Cabinet Config. ptr	*/
	uint	expansion_config ;	/* Expans. Cabinet Config. ptr	*/
	uint	panel_config ;		/* Operator Panel Config. ptr	*/
	uint	mother_config ;		/* Mother Board Config. ptr	*/
	uint	native_io_config ;	/* Native IO Config. ptr	*/
	uint	cpu_config ;		/* CPU Daughter Boards Conf. pt	*/
	uint	memory_config ;		/* Memory Daughter Brds Conf. pt*/
	uint	basic_MCA_backp ;	/* Basic MCA Backpanel Cf. Ptr	*/
	uint	basic_MCA_agents ;	/* Basic MCA Agents Conf. Ptr	*/
	uint	extern_MCA_backp ;	/* Extern MCA Backpanel Cf. Ptr	*/
	uint	extern_MCA_agents ;	/* Extern MCA Agents Conf. Ptr	*/
	uint	adaptor_config ;	/* adaptor Conf. Ptr		*/
	uint	res34[3] ;		/* reserved	*/
	} ;



/********************************/
/*	Debug interface area	*/
/********************************/

struct	dbg_interf
	{
	uchar	command ;		/* command byte	*/
	uchar	control ;		/* control byte	*/
	uchar	type ;			/* type byte	*/
	uchar	mode ;			/* mode byte	*/
	uint	param1 ;		/* param 1 word	*/
	uint	param2 ;		/* param 2 word	*/
	uint	param3 ;		/* param 3 word	*/
	} ;

/********************************/
/*	Transport Messages	*/
/********************************/

#define TP_FIFO_LG	32		/* transport fifo length	*/
#define TP_BUF_LG	4096		/* transport buffer length	*/

struct	tp_messages
	{
	uchar	bump_pid;		/* bump port ID			*/
	uchar	cpu_pid;		/* cpu port ID			*/
	uchar	tran_id;		/* transaction ID		*/
	uchar	res;			/* reserved transport		*/
	uchar	cmd_stat;		/* command/status		*/
	uchar	cmd_ext;		/* command extension		*/
	uchar	user6;			/* command dependent		*/
	uchar	user7;			/* command dependent		*/
	uchar	*address;		/* command dependent		*/
	ushort	user12;			/* command dependent		*/
	ushort	user14;			/* command dependent		*/
	} ;

struct	tp_fifo
	{
	struct	tp_messages	tp_command[TP_FIFO_LG] ; /* Transport Commands	*/
	struct	tp_messages	tp_status[TP_FIFO_LG] ;	/* Transport Status	*/
	} ;

/********************************/
/*				*/
/*	CPU/BUMP INTERFACE	*/
/*				*/
/********************************/

struct	cpu_bump
	{
	struct	interf_header	interf_header ;	/* Interface header	*/
	struct	tp_fifo_header	tp_fifo_header ; /* Transport header	*/
	struct	op_interf	op_interf ;	/* Operator Panel interf*/
	struct	cpu_start	cpu_start ;	/* CPUs start area	*/
	struct	config_VPD_pt	config_VPD_pt ;	/* Config/VPD ptrs Table*/
	struct	ASICS_status	ASICS_status ;	/* ASICS read status	*/
	uint	res200[608] ;			/* reserved	*/
	struct	dbg_interf	dbg_interf[8] ;	/* Debug interface area	*/
	struct	tp_fifo		tp_fifo ;	/* Transport Messages	*/
	uchar	tp_buffer[TP_BUF_LG] ;		/* Transport Buffer	*/
	} ;



/****************************************/
/****************************************/
/*					*/
/*	BUMP DRIVERS AREA STRUCTURE	*/
/*					*/
/****************************************/
/****************************************/


/****************************************/
/*	I2C BUFFERS AREA STRUCTURE	*/
/****************************************/

/* I2C driver Command buffer: used from SPART/SSF to Bump */

struct	I2C_cmnd
	{
	uchar	sa ;			/* slave address	*/
	uchar	ll ;			/* length		*/
	uchar	cab ;			/* cabinet number	*/
	uchar	com ;			/* command		*/
	uchar	sel ;			/* selection		*/
	uchar	wa ;			/* word address		*/
	uchar	dt[32] ;		/* command data		*/
	uchar	res[10] ;		/* reserved		*/
	} ;

/* I2C driver Expected status buffer: used from Bump to SPART/SSF */
/* I2C driver Unexpected status buffer is the same as the Expected buffer */

struct	I2C_reply
	{
	uchar	ll ;			/* length	*/
	uchar	sc ;			/* source	*/
	uchar	qf ;			/* qualifier	*/
	uchar	dt[32] ;		/* reply data	*/
	uchar	res[13] ;		/* reserved	*/
	} ;

/************************/
/*	Auto_dial	*/
/************************/

struct	auto_dial
	{
	uchar	phone[144] ;		/* Phone number	*/
	uchar	site_param[160] ;	/* Site parameters	*/
	uchar	modem_param[360] ;	/* Modem parameters	*/
	uchar	res[360] ;		/* reserved	*/
	} ;



/****************************************/
/****************************************/
/*					*/
/*	BUMP DRIVERS AREA STRUCTURE	*/
/*					*/
/****************************************/
/****************************************/

struct	bump_drivers
	{
	struct	I2C_reply	unexp ;	/* I2C unexpected buffer	*/
	struct	I2C_cmnd	cmnd ;	/* I2C command buffer		*/
	struct	I2C_reply	reply ;	/* I2C expected buffer		*/
	uint	cmd_task_id ;		/* command task id		*/
	uint	unex_task_id ;		/* unexpacted message task id	*/
	uchar	I2C_BUMP_ENV ;		/* I2C BUMP environment flag	*/
	uchar	I2C_TO_EXPIR;		/* i2c_driver TO expired	*/
	uchar	I2C_UNEXP_BB;		/* Unexpected Buffer Busy	*/
	uchar	I2C_DAT_NC;		/* Uncomplete Data received
							from I2C Bus	*/
	uchar	res2[4] ;
	uint	FLASH_PR_FLG ;		/* Flash_prom call flag	*/
	uchar	boot_flag ;		/* Boot flag
					0x00 = Normal or Service boot list
					0x01 = Chosen Boot Device
					0xff = Network	*/
	uchar	device_A ; /* (Not used to boot) */
	uchar	device_B ; /* Drawer ID of SCSI Controller (Not used to boot) */
	uchar	device_C ; /* Channel/Bus# (BUID = Channel/Bus# + Base_BUID) */
	uchar	device_D ; /* Slot# */
	uchar	device_E ; /* (Not used to boot) */
	uchar	device_F ; /* Connector ID (Not used to boot) */
	uchar	device_G ; /* SCSI ID */
	uchar	device_H1 ; /* H1 LUN ID (for 8 bits SCSI BUS,
				high digit for 16 bits SCSI bus) */
	uchar	device_H2 ; /* H2 LUN ID (0x00 for 8 bits SCSI BUS,
				low digit for 16 bits SCSI bus) */
	uchar	res3 ;
	uchar	base_BUID ;		/* Base IOCC BUID	*/
	uint	run_time_flag ;		/* Run time flag	*/
	uint	init_flag ;		/* Power / restart flag	*/
	uint	fast_ipl ;		/* Reduce tests		*/
	uint	clear_E_Key ;		/* E_Key cleared on Boot	*/
	uint	autodial ;		/* Autodial permission	*/
	uint	serv_multi ;		/* Boot multi in Service mode	*/
	uint	extended_tests ;	/* Extended Tests enabled	*/
	uint	trace_tests ;		/* Tests in Trace mode	*/
	uint	endurance_tests ;	/* Tests in Endurance mode	*/
	uint	msg_mode_tests ;	/* Tests message mode	*/
	uint	def_param_tests ;	/* Tests default parameters	*/
	ushort	machine_type ;		/* Machine type		*/
	uchar	conf_table_id ;		/* Config Table used (0 = Work)	*/
	uchar	res31;
	struct	tp_messages *mes_flash_update;	/* mesage update flash	*/
	uint	size_flash;		/* flash eprom size		*/
	uint	l_debug;		/* debug line	*/
	struct	auto_dial	dial ;	/* Auto_dial parameters	*/
	uchar	res4[2836] ;		/* reserved	*/
	uchar	STACK[1024] ;		/* Bump stand-by stack	*/
	uchar	BUMP_STAT[32] ;		/* Bump status table image */
	uchar	res5[992] ;
	uchar	screen[2048] ;
	} ;



/****************************************/
/****************************************/
/*					*/
/*	BUMP DATA AREA STRUCTURE	*/
/*					*/
/****************************************/
/****************************************/

struct	bump_data
	{
	int	res[28672] ;
	} ;

/************************************************/
/************************************************/
/*						*/
/*	GLOBALE NOVRAM STRUCTURE		*/
/*						*/
/*	This structure MUST ONLY be used	*/
/*		by the BUMP Firmware :		*/
/*	Ths AIX system MUST ONLY use the	*/
/*		'interf_header' structure.	*/
/*						*/
/************************************************/
/************************************************/

struct	novram
	{
	struct	boot_control	boot_control ;
	struct	risc_compatible	risc_compatible ;
	struct	config_table	config_table ;
	struct	error_logging	error_logging ;
	struct	cpu_bump	cpu_bump ;
	struct	bump_drivers	bump_drivers ;
	struct	bump_data	bump_data ;
	} ;



/****************************************/
/****************************************/
/*					*/
/*	DEFINES FOR NOVRAM POINTERS	*/
/*					*/
/****************************************/
/****************************************/

#define	PT_NOVRAM	( (struct novram *)NOVRAM )
#define	WORK_CT		( (struct config_table *)WCF_TABLE )

#define BOOT_CONTROL	PT_NOVRAM->boot_control
#define RISC_COMPATIBLE	PT_NOVRAM->risc_compatible
#define	CONFIG_TABLE	PT_NOVRAM->config_table
#define ERROR_LOGGING	PT_NOVRAM->error_logging
#define CPU_BUMP	PT_NOVRAM->cpu_bump
#define BUMP_DRIVERS	PT_NOVRAM->bump_drivers
#define PT_BUMP_DATA	PT_NOVRAM->bump_data

#define BUMP_DRV	BUMP_DRIVERS
#define BDRV_TO		BUMP_DRIVERS.I2C_TO_EXPIR
#define BDRV_BB		BUMP_DRIVERS.I2C_UNEXP_BB
#define BDRV_ENV	BUMP_DRIVERS.I2C_BUMP_ENV
#define BDRV_RUN	BUMP_DRIVERS.run_time_flag

/*************************************************************************/
/******************* VPD definitions *************************************/
/*************************************************************************/
/*
 * Each component (IOD, Memory board, Mother board) has one VPD area.
 * A VPD area contains one vpd_head, followed by a number of VPD fields.
 * A field starts with a '*', and 2 ascii characters, depending on the field
 * type.
 */ 

/* VPD header description */
struct vpd_head {
	uchar res;		/* NULL 	*/
	char ident[3];		/* "VPD" 	*/
	ushort length;		/* VPD length w/o header, divided by 2 */
	ushort crc;		/* VPD CRC w/o the header */
};
/* Field header description */
struct vpd_field_head {
	uchar star;		/* '*' */
	char ident[2];		/* name of this field */
	uchar length;		/* Total length of this field, divided by 2 */
};

/* Field decription, including data */
struct vpd_field {
	struct vpd_field_head h;
	char data[512];
};

/* Name of VPD fields */
#define VPD_IDENT		"VPD"
#define ALTERABLE_ROM_ID	"RM"
#define ROM_ID			"RL"
#define PROCESSOR_COMPONENT	"PC"
#define FRU_NUMBER		"FN"
#define JUNIOR_PREFIX		"PJR"	/* Junior VPD start with that */
#define JUNIOR_2_PREFIX		"JR2"	/* Junior II                  */

/* Offsets into the VPD fields */
#define Flash_lvl_o	25	/* Flash_base version number	*/
#define Flash_lvl_l	4	/* coded on 4 ascii characters	*/
#define IPL_lvl_o	29	/* IPL version number		*/
#define IPL_lvl_l	4	/* coded on 4 ascii characters	*/
#define SSGA_lvl_o	4	/* SSGA version number		*/
#define SSGA_lvl_l	2	/* coded on a short integer	*/
#define IONIAN0_lvl_o	8	/* IONIAN version number	*/
#define IONIAN0_lvl_l	2	/* coded on a short integer	*/
#define CCA2_lvl_o	12	/* CCA2 version number		*/
#define CCA2_lvl_l	2	/* coded on a short integer	*/
#define CPU_lvl_o	4	/* 601 version number		*/
#define CPU_lvl_l	2	/* coded on a short integer	*/
#define DCB_lvl_o	8	/* DCB version number		*/
#define DCB_lvl_l	2	/* coded on a short integer	*/
