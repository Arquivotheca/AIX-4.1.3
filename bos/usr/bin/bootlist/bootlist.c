static char sccsid[] = "@(#)19 1.13.2.10 src/bos/usr/bin/bootlist/bootlist.c, cmdcfg, bos41J, 9522A_b 95/05/31 14:09:44";
/*
 *   COMPONENT_NAME: CMDCFG
 *
 *   FUNCTIONS: DATA
 *		ERRSTR
 *		MSGSTR
 *		cdrom_device
 *		check_ROS_level
 *		do_error
 *		ent_device
 *		fd_device
 *		get_bus_id
 *		hdisk_device
 *		iplist_parse
 *		main
 *		negate_device
 *		negate_list
 *		net_device
 *		parse_name
 *		pr_iplist
 *		read_list
 *		rmt_device
 *		scdisk_device
 *		scsi_device
 *		serdasd_device
 * @@SSA_BEGIN@@
 *		ssa_device
 * @@SSA_END@@
 *		str_to_hardware
 *		tok_device
 *		write_list
 *		get_sid_lun
 *		bootlist_support
 *		rspc_write_list
 *		rspc_read_list
 *		rspc_device
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <errno.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cfgresid.h>

#define _KERNSYS
#define _RSPC
#include <sys/systemcfg.h>
#undef _KERNSYS


#define TRUE			1
#define FALSE			0

#define FATAL_EXIT		1

#define DATA(a,b,c)		{ if (verbose) printf(a,b,c); }

/*----------------------------- MESSAGES ------------------------------------*/
#define MSG_SET			1
#define BOOTLIST_USEAGE		1
#define NORMAL_TITLE		2
#define SERVICE_TITLE		3
#define PREVBOOT_TITLE		4
#define BOOTLIST_SUCCESS	5

char *msgstr[] =
{	"",
	"Usage:\nbootlist -m mode [-r] [-i]|[[-f file] device1 device2...\
deviceN]\n",
	"\n------------------- NORMAL IPLIST at address 0x%x ---------------\n",
	"\n------------------- SERVICE IPLIST at address 0x%x --------------\n",
	"\n------------------- PREVBOOT LIST at address 0x%x ---------------\n",
	"%d bytes were written to address 0x%x in %s\n"
};


/*----------------------------- ERRORS ------------------------------------*/
#define ERR_SET				2
#define ARGOROPT_CONFLICT		1
#define BAD_NAME			2
#define BAD_MODE			3
#define NO_ROOM				4 
#define IOCTL_ERR			5
#define OPEN_ERR			6
#define CREAT_ERR			7
#define SYNTAX_ERR			8
#define ODM_ACCESS_ERR			9
#define NOT_AVAILABLE			10
#define NO_PARENT			11
#define CONNWHERE_FORMAT		12
#define UNKNOWN_DISK_TYPE		13
#define ILLEGAL_ATTR_COMBO		14
#define ILLEGAL_ATTR			15
#define DUPLICATE_ATTR			16
#define ARGOROPT_MISSING		17
#define BAD_HARDWARE_ADDR		18
#define NOT_BOOTABLE			19
#define NOT_SUPPORTED			20
#define BAD_MODE2			21
#define CANT_MAKE_LIST			22
#define CANT_WRITE_LIST			23

char *errstr[] =
{  "",
	"0514-201 bootlist: Usage error. Conflicting flags (%s)\n",
	"0514-202 bootlist: Invalid device name (%s)\n",
	"0514-203 bootlist: Invalid mode (%s)\n",
	"0514-204 bootlist: There is not enough room in the list for %s\n",
	"0514-205 bootlist: IOCTL system call returned errno %d\n",
	"0514-206 bootlist: Unable to open file '%s'\n",
	"0514-207 bootlist: Unable to create file '%s'\n",
	"0514-208 bootlist: Usage error (%s)\n",
	"0514-209 bootlist: Was not able to access the %s object class\n",
	"0514-210 bootlist: Device %s is not in the AVAILABLE state\n",
	"0514-211 bootlist: No parent found for device %s\n",
	"0514-212 bootlist: The connwhere field of CuDv has changed (it is \
now unrecognizable)\n",
	"0514-213 bootlist: Unknown disk type %s\n",
	"0514-214 bootlist: Illegal combination of attributes for %s\n",
	"0514-215 bootlist: invalid attribute (%s)\n",
	"0514-216 bootlist: attribute specified more than once (%s)\n",
	"0514-200 bootlist: Usage error. Flag or parameter is missing (%s)\n",
	"0514-217 bootlist: hardware address must be 12 hex digits; seperators are \
not allowed\n",
	"0514-218 bootlist: the device %s cannot be used to boot from because\n\t \
the system ROS does not support it\n",
	"0514-219 bootlist: This model does not support boot device lists\n",
	"0514-220 bootlist: Invalid mode (%s) for this model\n",
	"0514-221 bootlist: Unable to make boot device list for %s\n",
	"0514-222 bootlist: Unable to write boot device list to NVRAM\n"
};

#include <nl_types.h>
#include <locale.h>
#define MSG_CATALOG		"bootlist.cat"
#define MSGSTR(num) catgets( catopen(MSG_CATALOG,NL_CAT_LOCALE), MSG_SET, num, msgstr[num] )
#define ERRSTR(num) catgets( catopen(MSG_CATALOG,NL_CAT_LOCALE), ERR_SET, num, errstr[num] )

/*----------------------------- IPLIST SPECIFICS ----------------------------*/
#define IPLIST_LENGTH		84
#define PREVBOOT_LENGTH		18
#define NORMAL_VALIDITY_CODE_1	0x4A
#define NORMAL_VALIDITY_CODE_2	0x4D
#define SERVICE_VALIDITY_CODE_1	0x57
#define SERVICE_VALIDITY_CODE_2	0x52

#define MODE_NORMAL		"normal"
#define MODE_SERVICE		"service"
#define MODE_BOTH		"both"
#define MODE_PREVBOOT		"prevboot"
#define NORMAL_MODE		1
#define SERVICE_MODE		2
#define PREVBOOT_MODE		4

#define MAX_DEVICES		8
#define MAX_ATTRS		9
#define MAX_MODE_CH		8
#define MAX_DEV_NAME		30
#define MAX_PER_LINE		80
#define MAX_ARG_CHARS		80
#define MAX_ARGS		MAX_PER_LINE/2

#define BOOTLIST_LTH		2048
#define MAX_RSPC_DEVICES	4

/*----------------------------- NVRAM ADDRESSES -----------------------------*/
#include <sys/nvdd.h>

/*------------------------------ MACH_DD_IO structure -----------------------*/
#include <sys/mdio.h>
#define NVRAM		"/dev/nvram"

/*------------------------------ module globals vars ------------------------*/
int mode;				/* 0=no mode; 1=normal; 2=service */
int invalidate_list;			/* >0 for invalidating iplists */
int device_specified;			/* >0 if -d entered */
int dev_index;				/* index into the devices array */
int ip_index;				/* index into next location in iplist */
unsigned char iplist[IPLIST_LENGTH];	/* local iplist for non-RSPC models */
int read_iplist;			/* >0 if iplist to be displayed */
int verbose;				/* >0 if verbose output desired */
unsigned char bootlist[BOOTLIST_LTH];	/* Boot list for RSPC models */
int rspc_dev_cnt;			/* Device count for RSPC models */

/*-------------------------------- prefix strings ----------------------------*/
#define FD	"fd"
#define SCDISK	"scdisk"
#define BADISK	"badisk"
#define CDROM	"cd"
#define RMT	"rmt"
#define HDISK	"hdisk"
#define ENT	"ent"
#define TOK	"tok"
#define FDDI	"fddi"

/*------------------------------ attributes ----------------------------------*/
char *netattrs[] =
{  "client",
	"bserver",
	"gateway",
	"hardware",
	NULL
};

/*--------------------------- generic device info ---------------------------*/

struct generic_info 
{	char *name;
	unsigned char length;
	char code1;
	char code2;
};

#define MAX_GENERIC		8

struct generic_info generic[MAX_GENERIC] =	/* device information */
{	
	{FD,2,'G','F'},
	{SCDISK,2,'G','I'},
	{BADISK,2,'G','K'},
	{CDROM,2,'G','C'},
	{RMT,2,'G','T'},
	{ENT,2,'G','D'},
	{TOK,2,'G','O'},
	{FDDI,2,'G','P'}
};

/*--------------------------- specific device info ---------------------------*/

struct specific_info
{	char *name;
	int (*function)();
	char **attrs;
}; 

static int fd_device(),scdisk_device(),hdisk_device();
/* @@SSA_BEGIN@@ */
static void ssa_device();
/* @@SSA_END@@   */
static int cdrom_device(),rmt_device();
static int ent_device();
static int tok_device();
static int fddi_device();
static int rspc_device();

#define MAX_SPECIFIC		8

struct specific_info specific[MAX_SPECIFIC] = 
{
	{ FD, fd_device, NULL },
	{ HDISK, hdisk_device, NULL },
	{ CDROM, cdrom_device, NULL }, 
	{ RMT, rmt_device, NULL },
	{ ENT, ent_device, netattrs },
	{ TOK, tok_device, netattrs },
	{ FDDI, fddi_device, netattrs }
};


/*-------------------------------- device list -------------------------------*/
struct attr_node
{
	char *name;
	char *value;
	struct attr_node *next;
};

struct device_node
{	char 	*name;
	int 	index;
	char 	**valid_attrs;
	struct 	attr_node *attrs;
	int 	(*function)();
	struct 	device_node *next;
};


/*-------------------------------- module globals ----------------------------*/
struct device_node *first_device;
struct device_node *current_device;
struct device_node *next_device;

static int ip_argc;				/* ptr to argc - for dbg info */
static char **ip_argv;				/* ptr to argv - for dbg info */


int get_sid_lun();

main( argc, argv )
int argc;		/* num args in argv */
char *argv[];		/* args from command line */
{
	int	support;


	/* initialize internal state */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
	privilege(PRIV_LAPSE);
	setlocale( LC_ALL, "" );

	support = bootlist_support();
	if (argc == 1) {
		/* bootlist with no options - indicate if bootlist supported */
		exit(support);
	}
	else if (support==1) {
		/* bootlist not supported on this model....fail */
		do_error(NOT_SUPPORTED,NULL,FALSE);
	}

	/* initialize internal variables */
	mode = read_iplist = verbose = 0;
	invalidate_list = FALSE;
	device_specified = FALSE;
	ip_index = 1; /* reserve room for the iplist header code */
	ip_argc = argc;
	ip_argv = argv;
	first_device = current_device = NULL;
	next_device = (struct device_node *)malloc(sizeof(struct device_node));
	rspc_dev_cnt = 0;

	odm_initialize();

	/* parse user's input */
	iplist_parse( argc, argv );

	odm_terminate();

#ifdef BOOT_DEBUG
/* print out what the iplist would be changed to, but don't change it */
pr_iplist();
exit(0);
#endif

	/* check for user errors */
	if (!mode)
		do_error(ARGOROPT_MISSING,"-m",TRUE);
	if ( (invalidate_list) && (device_specified) )
		do_error(ARGOROPT_CONFLICT,"-i dev_name",TRUE);

	/* process the user's request */

	if (__rspc()) {
		/* RSPC models */
		if (invalidate_list)
			rspc_write_list("fw-boot-path=");
		if (device_specified)
			rspc_write_list(bootlist);
		if (read_iplist)
			rspc_read_list();
	}

	else {
		/* Other models */
		if (invalidate_list) {
			if (mode & NORMAL_MODE) {
				DATA("invalidaing the NORMAL iplist (0x%x)\n",
							IPLIST_NORMAL,NULL)
				negate_list( IPLIST_NORMAL );
			}
			if (mode & SERVICE_MODE) {
				DATA("invalidaing the SERVICE iplist (0x%x)\n",
							IPLIST_SERVICE,NULL)
				negate_list( IPLIST_SERVICE );
			}
			if (mode & PREVBOOT_MODE) {
				DATA("invalidaing the PREVBOOT (0x%x)\n",
							IPLIST_PREVBOOT,NULL)
				negate_device( IPLIST_PREVBOOT );
			}
		}
		if (device_specified) {
			if (mode & NORMAL_MODE) {
				iplist[0] = NORMAL_VALIDITY_CODE_1;
				iplist[1] = NORMAL_VALIDITY_CODE_2;
				DATA("writing to the NORMAL iplist (0x%x)\n",
							IPLIST_NORMAL,NULL)
				write_list( IPLIST_NORMAL );
			}
			if (mode & SERVICE_MODE) {
				iplist[0] = SERVICE_VALIDITY_CODE_1;
				iplist[1] = SERVICE_VALIDITY_CODE_2;
				DATA("writing to the SERVICE iplist (0x%x)\n",
							IPLIST_SERVICE,NULL)
				write_list( IPLIST_SERVICE );
			}
		}

		if (read_iplist) {
			if (mode & NORMAL_MODE)
				read_list( IPLIST_NORMAL, IPLIST_LENGTH,
							NORMAL_TITLE );
			if (mode & SERVICE_MODE)
				read_list( IPLIST_SERVICE, IPLIST_LENGTH,
							SERVICE_TITLE );
			if (mode & PREVBOOT_MODE)
				read_list( IPLIST_PREVBOOT, PREVBOOT_LENGTH,
							PREVBOOT_TITLE );
		}
	}

	exit( 0 );
}


/*-------------------------------  iplist_parse ------------------------------
 * NAME:iplist_parse 
 *																						  
 * FUNCTION: parse the user's input 
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	iplist  - initializes this array with data
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
iplist_parse( argc, argv )
int argc;			/* # of args in argv */
char *argv[];			/* args from command line */
{
	FILE *fp;		/* file ptr if -f specified */
	int c;			/* file ptr if -f specified */
	char tmp[80];		/* device name */
	char *name;		/* ptr to command line arg */
	int names_from_file = FALSE;  /* >0 if -f specified */
	extern int optind;	/* from getopt */
	extern char *optarg;	/* from getopt */

	/* parse the command line arguments */
	while ((c = getopt(argc, argv, "virm:f:")) != EOF) {
		switch (c) {
			case 'i' :	invalidate_list = TRUE;
					break;

			case 'r' :	read_iplist = TRUE;
					break;

			case 'm' :	if (!strcoll( optarg, MODE_NORMAL ))
						mode = (mode | NORMAL_MODE);
					else if (!strcoll(optarg,MODE_SERVICE))
						mode = (mode | SERVICE_MODE);
					else if (!strcoll( optarg, MODE_BOTH ))
						mode = (mode | NORMAL_MODE | SERVICE_MODE);
					else if (!strcoll(optarg,MODE_PREVBOOT))
						mode = (mode | PREVBOOT_MODE);
					else
						do_error(BAD_MODE,optarg,FALSE);

					/* Only normal mode allowed on RSPC */
					if (__rspc() && mode != NORMAL_MODE)
						do_error(BAD_MODE2,optarg,FALSE);
					break;

			case 'f' :	if ((fp = fopen(optarg,"r")) == NULL)
						do_error(OPEN_ERR,optarg,FALSE);
					names_from_file = TRUE;
					break;

			case 'v' :	verbose = TRUE;
					break;
		}/* end switch*/
	}/* end while */

	/* scan the command line args and/or stuff from a file */
	while ( (optind < argc) || (names_from_file) )
	{  if (optind < argc)
			parse_name( argv[optind++] );
		else if (fscanf(fp,"%s",tmp) == EOF)
		{	/* no more to read */
			fclose( fp );
			names_from_file = FALSE;
			break;
		}
		else
		{	/* must copy into malloc'd area because parse_name will point to it */
			name = (char *) malloc( strlen(tmp)+1 );
			strcpy( name, tmp );
			parse_name( name );
		}
	}

	/* put the device information into the temp iplist */
	for (current_device=first_device; current_device != NULL;
		  current_device=current_device->next)
	{	if (current_device->function)
			(*current_device->function)(current_device);
		else
		{	/* treat this as a generic device */
			DATA("generic device = %s\n",current_device->name,NULL)
			dev_index = current_device->index;
			if ((ip_index + 1 + generic[dev_index].length) < IPLIST_LENGTH)
			 {  /* add this device to the list */
				 iplist[++ip_index] = generic[dev_index].length;
				 iplist[++ip_index] = generic[dev_index].code1;
				 iplist[++ip_index] = generic[dev_index].code2;
			 }
			 else
				 do_error(NO_ROOM,generic[dev_index].name,FALSE);
		}
	}/*end for*/
}


/*------------------------------- parse_name  --------------------------------
 * NAME:parse_name
 *																						  
 * FUNCTION: parses a string looking for a valid prefix and/or device name
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	generic		- array of generic names
 *	specific	- array of device prefixes
 *
 * RETURNS: 
 *	i		- index into one of the above arrays
 *	seqno		- >0 if name found & it has a number appended to it
 *	-1		- name not found
 *		
 *---------------------------------------------------------------------------*/
parse_name( name )
char *name;		/* name to validate */
{ char *value;
  char str[80];		/* tmp str */
  int i;		/* loop indexing */
  int max;		/* name strlen */
  char **valid;
  struct attr_node *attr;
  struct attr_node *prev;
  int valid_name;

	/* check for an attribute name */
	if ((value = (char *)strchr(name,'=')) != NULL) {
		if (__rspc()) {
			/* Attributes not allowed on RSPC models */
			do_error(ILLEGAL_ATTR,name,TRUE);
		}

		*value = '\0';
		value++;
 
		/* was there a device already specified ? */
		if (current_device)
		{	/* is this attribute valid for this device ? */
			for (valid=current_device->valid_attrs; *valid != NULL; valid++)
				if (!strcmp(name,*valid))
				{	/* this attr is valid for the current device */
					/* make sure it hasn't already been specified */
					for (attr=current_device->attrs,prev=NULL; attr != NULL; 
						  prev=attr,attr=attr->next)
					{	if (!strcmp(name,attr->name))
							do_error(DUPLICATE_ATTR,name,FALSE);
					}

					/* not already there - add it */
					attr = (struct attr_node *) malloc( sizeof(struct attr_node) );
					attr->name = name;
					attr->value = value;
					attr->next = NULL;
					if (prev)
						prev->next = attr;
					else
						current_device->attrs = attr;

					break;
				} 

			if (!*valid)
				do_error(ILLEGAL_ATTR,name,FALSE);
		}
		else
			do_error(ILLEGAL_ATTR,name,FALSE);
	}/* end if attr */
	else
	{	/* check for a valid device name */

		/* cop to tmp str to work on it */
		max = strlen( name );
		strncpy( str, name, 80 );

		/* start at the end & find the first non-numeric char */
		max--;
		for (i=max; i >= 0; i--)
			if ((str[i] < '0') || (str[i] > '9'))
				break;

		/* is there a seqno at the end??? */
		if (i == max)
		{  /* no - treat it as a generic device name */

			/* check the generic device list */
			for (i=0; i < MAX_GENERIC; i++)
				if (!strcoll( generic[i].name, str ))
				{  /* valid name - add it to the device list */
					if (__rspc()) {
						/* Not valid for RSPC models */
				  		do_error(BAD_NAME,name,FALSE);
					}
					next_device->name = name;
					next_device->index = i;
					next_device->valid_attrs = NULL;
					next_device->attrs = NULL;
					next_device->function = NULL;
					next_device->next = NULL;
					if (first_device)
						current_device->next = next_device;
					else
						first_device = next_device;

					current_device = next_device;
					next_device = NULL;

					break;
				}
			if (i == MAX_GENERIC)
				  do_error(BAD_NAME,name,FALSE);
		}
		else if (i > -1)
		{  /* seqno present - treat as a specific device */

			/* remove the seqno from the end - just want to compare the prefix*/
			str[i+1] = '\0';
			valid_name = FALSE;

			/* check the specific device list */
			for (i=0; i < MAX_SPECIFIC; i++)
				if (!strcoll( specific[i].name, str ))
				{  /* valid name - return this index */
					valid_name = TRUE;
					next_device->name = name;
					next_device->index = i;
					next_device->valid_attrs = specific[i].attrs;
					next_device->attrs = NULL;
					if (__rspc()) {
						next_device->function = rspc_device;
					}

					else {
						next_device->function = specific[i].function;
					}
					next_device->next = NULL;
					if (first_device)
						current_device->next = next_device;
					else
						first_device = next_device;

					current_device = next_device;
					next_device = NULL;

					break;
				}

				if (! valid_name)
			  		do_error(BAD_NAME,name,FALSE);
		}
		else
		  	do_error(BAD_NAME,name,FALSE);

		device_specified = TRUE;

		if (next_device == NULL)
		{	/* allocate another device node */
			next_device = (struct device_node *)malloc(sizeof(struct device_node));
		}
	}

}


/*-------------------------------  do_error	--------------------------------
 * NAME:do_error
 *																						  
 * FUNCTION: writes out the specified error message and exits
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *---------------------------------------------------------------------------*/
do_error( code, str1, usage )
int code;		/* error code - index into errmsg array */
char *str1;		/* str to print */
char *usage;		/* >0 if usage message needed */
{ char temp[200];	/* tmp string */

	sprintf( temp, ERRSTR(code), str1 );

	/* auditing */
	privilege(PRIV_ACQUIRE);
	auditlog("NVRAM_Config",-1,temp,strlen(temp)+1);
	privilege(PRIV_LAPSE);

	fprintf( stderr, temp );
	if(usage)
		fprintf(stderr,MSGSTR(BOOTLIST_USEAGE));

	exit( FATAL_EXIT );
}


/*------------------------------- negate_list --------------------------------
 * NAME:negate_list
 *																						  
 * FUNCTION: writes zeros into the validity code of the specified iplist
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
negate_list( address )
int address;			/* address of list */
{ MACH_DD_IO nv;		/* struct for ioctl */
  unsigned char value[4];	/* tmp var */
  int fd;			/* nvram file descriptor */
  char temp[100];		/* tmp var */

	privilege(PRIV_ACQUIRE);
	if ((fd = open(NVRAM,2)) < 0)
		do_error(OPEN_ERR,NVRAM,FALSE);
	privilege(PRIV_LAPSE);

	/* initialize the machine device driver record */
	value[0] = value[1] = value[2] = value[3] = 0;
	nv.md_incr = MV_BYTE;
	nv.md_data = value;
	nv.md_size = 4;
	nv.md_addr = address;

	/* write the data */
	if (ioctl(fd,MIONVPUT,&nv) < 0)
		do_error(IOCTL_ERR,errno,FALSE);

	close( fd );

	/* auditing */
	sprintf(temp,MSGSTR(BOOTLIST_SUCCESS), 4, address, NVRAM );
	privilege(PRIV_ACQUIRE);
	auditlog("NVRAM_Config",0,temp,strlen(temp)+1);
	privilege(PRIV_LAPSE);
}


/*------------------------------- negate_device ------------------------------
 * NAME:negate_device 
 *																						  
 * FUNCTION: writes zeros into the device description for the specified mode
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
negate_device( address )
int address;				/* address of list */
{ MACH_DD_IO nv;			/* ioctl struct */
  unsigned char value[PREVBOOT_LENGTH]; /* data */
  int fd;				/* nvram file descriptor */
  int i;				/* loop indexing */
  char temp[100];			/* tmp str */
 
	privilege(PRIV_ACQUIRE);
	if ((fd = open(NVRAM,2)) < 0)
		do_error(OPEN_ERR,NVRAM,FALSE);
	privilege(PRIV_LAPSE);

	/* initialize the machine device driver record */
	for (i=0; i < PREVBOOT_LENGTH; value[i++] = 0);
	nv.md_incr = MV_BYTE;
	nv.md_data = value;
	nv.md_size = PREVBOOT_LENGTH;
	nv.md_addr = address;

	/* write the data */
	if (ioctl(fd,MIONVPUT,&nv) < 0)
		do_error(IOCTL_ERR,errno,FALSE);

	close( fd );

	/* auditing */
	sprintf(temp,MSGSTR(BOOTLIST_SUCCESS),
		  PREVBOOT_LENGTH, address, NVRAM );
	privilege(PRIV_ACQUIRE);
	auditlog("NVRAM_Config",0,temp,strlen(temp)+1);
	privilege(PRIV_LAPSE);
}


/*------------------------------- write_list  --------------------------------
 * NAME:write_list
 *																						  
 * FUNCTION: writes the local iplist into the specified NVRAM iplist
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	iplist - reads information from this local array
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
write_list( address )
int address;
{ MACH_DD_IO nv;	/* ioctl struct */
  int fd;		/* NVRAM fd */
  char temp[100];	/* temp var */
	
	privilege(PRIV_ACQUIRE);
	if ((fd = open(NVRAM,2)) < 0)
		do_error(OPEN_ERR,NVRAM,FALSE);
	privilege(PRIV_LAPSE);

	/* initialize the machine device driver record */
	nv.md_incr = MV_BYTE;
	nv.md_data = iplist;
	nv.md_size = IPLIST_LENGTH;
	nv.md_addr = address;

	/* write the data */
	if (ioctl(fd,MIONVPUT,&nv) < 0)
		do_error(IOCTL_ERR,errno,FALSE);

	close( fd );
	/* auditing */
	sprintf(temp,MSGSTR(BOOTLIST_SUCCESS),
		  IPLIST_LENGTH, address, NVRAM );
	privilege(PRIV_ACQUIRE);
	auditlog("NVRAM_Config",0,temp,strlen(temp)+1);
	privilege(PRIV_LAPSE);
}


/*------------------------------- get_bus_id    --------------------------------
 * NAME: get_bus_id
 *																						  
 * FUNCTION: returns the bus id that the given device is attached to
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * RETURNS: 
 *		bus id for the bus which the given device is attached to
 *		
 *---------------------------------------------------------------------------*/

#define SCSI_BUS_ID		0x82000000
#define BUS_UNIQUETYPE		"bus/sys/mca"

unsigned int get_bus_id( cudv, parent )
struct CuDv *cudv;
struct CuDv **parent;
{ char criteria[MAX_CRITELEM_LEN];	/* ODM search str */
  struct listinfo parent_info;		/* odm info */
  struct CuDv *cu;						/* tmp CuDv ptr */
  struct listinfo cu_info;				/* odm info */
  struct PdAt *pdat;						/* PdAt ptr */
  struct listinfo pdat_info;			/* odm info */
  struct CuAt *cuat;						/* CuAt ptr */
  struct listinfo cuat_info;			/* odm info */
  unsigned int bus_id;					/* bus id for this device */

	DATA("-------------------- function get_bus_id --------------------------\n",
			NULL,NULL)
	DATA("   device = %s\n", cudv->name,NULL)

	/* get the parent info */
	DATA("   parent device = %s; getting data for it\n",cudv->parent,NULL)
	sprintf( criteria, "name = '%s'", cudv->parent );
	if ((int)(*parent=odm_get_list(CuDv_CLASS,criteria,&parent_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if (!parent_info.num)
		do_error(NO_PARENT,cudv->parent,FALSE);

	/* in order to find the "bus id" we must traverse backward until we find */
	/*		the bus this device uses, then get the bus id for that bus */
	DATA("   searching for device with bus uniquetype of \"%s\"\n",
			BUS_UNIQUETYPE,NULL)
	cu = *parent;
	bus_id = SCSI_BUS_ID;
	do
	{
		/* is this device a bus ??? */
		if (!strcmp(cu->PdDvLn_Lvalue,BUS_UNIQUETYPE))
		{	/* found the bus - get it's bus id */
			DATA("   bus found : device = %s\n",cu->name,NULL)
			sprintf( criteria, "name = '%s' and attribute = 'bus_id'", cu->name );
			DATA("   looking for bus_id : crit = \"%s\"\n",criteria,NULL)
			if ((int)(cuat = (struct CuAt *) 
							odm_get_list(CuAt_CLASS,criteria,&cuat_info,1,1))<0)
				do_error(ODM_ACCESS_ERR,"CuAt",FALSE);
			if (cuat_info.num)
			{
				DATA("   bus_id found : value = %s\n", cuat->value, NULL)
				bus_id = strtoul( cuat->value, NULL, 16 );
			}
			else
			{	/* use the default value */
				sprintf( criteria, "uniquetype = '%s' and attribute = 'bus_id'", 
							cu->PdDvLn_Lvalue );
				DATA("   no CuAt found - looking for PdAt :\n\t   crit = \"%s\"\n",
						criteria,NULL)
				if ((int)(pdat = (struct PdAt *) 
								odm_get_list(PdAt_CLASS,criteria,&pdat_info,1,1))<0)
					do_error(ODM_ACCESS_ERR,"PdAt",FALSE);
				if (pdat_info.num)
				{
					DATA("   PdAt found; value = %s\n",pdat->deflt,NULL)
					bus_id = strtoul( pdat->deflt, NULL, 16 );
				}
				else
				{	DATA("   no PdAt - using the default of 0x%x\n",SCSI_BUS_ID,NULL)
					bus_id = SCSI_BUS_ID;
				}
			}

			break;
		}
		/* get the next parent cudv */
	 	sprintf( criteria, "name = %s", cu->parent );
		if ((int)(cu=odm_get_list(CuDv_CLASS,criteria,&cu_info,1,1)) < 0)
			do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
		if (!cu_info.num)
			do_error(NO_PARENT,criteria,FALSE);
	}
	while (cu->parent[0] != '\0');

	/* mask off the ROS part of the bus_id && shift */
	bus_id = (bus_id & 0x0FF00000) >> 20;

	DATA("-------------------- exit function get_bus_id ---------------------\n",
			NULL,NULL)

	return( bus_id );
}


/*------------------------------- fd_device  --------------------------------
 * NAME:fd_device
 *																						  
 * FUNCTION: initializes iplist with information for fd devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	ip_index	- index into iplist
 *	iplist		 - byte array which will be written into NVRAM
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
#define FD_LENGTH	2
#define FD_CODE		'N'

fd_device( device )
struct device_node *device;			/* ptr to device info */
{ char criteria[MAX_CRITELEM_LEN];  		/* search criteria */
  struct CuDv *cudv;				/* ptr to CuDv */
  struct listinfo cudv_info;			/* odm info */

	DATA("function fd_device : device name = %s\n",device->name,NULL)

	/* get this device out of CuDv */
	sprintf( criteria, "name = '%s'", device->name );
	if ((int)(cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if ((!cudv_info.num) || (cudv->status != AVAILABLE))
		do_error(NOT_AVAILABLE,device->name,FALSE);

	/* device found - is there enough room for it??? */
	if ((ip_index + 1 + FD_LENGTH) < IPLIST_LENGTH)
	{  /* use the connwhere field to initialize the iplist */
		iplist[++ip_index] = FD_LENGTH;
		iplist[++ip_index] = FD_CODE;
		iplist[++ip_index] = atoi(cudv->connwhere);
	}
	else
		do_error(NO_ROOM,device->name,FALSE);
}


/*------------------------------- check_ROS_level  -----------------------------
 * NAME: check_ROS_level
 *																						  
 * FUNCTION: checks to see whether the system ROS can support a pvid in the
 *					bootlist
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		0	=	ROS does NOT support pvid
 *		1	=	ROS DOES support pvid
 *		
 *---------------------------------------------------------------------------*/
check_ROS_level()
{
  struct CuAt *cuat;				/* CuAt ptr */
  struct listinfo cuat_info;			/* odm info */
  int use_pvid = FALSE;				/* tmp flag */

	DATA("---------------------- function check_ROS_level ------------------\n",
			NULL,NULL)

	/* check the ROS level - anything before 2/22/90 will not accept  */
	/*	  a pvid, so get the "rostime" attribute from CuAt			  */
	DATA("   any ROS before 2/22/90 will not accept pvid - checking for attr\n",
			NULL,NULL)
	if ((int)(cuat = odm_get_list(CuAt_CLASS,"attribute=rostime",
				 &cuat_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuAt",FALSE);
	if (cuat_info.num)
	{  /* check the decade */
		DATA("   rostime attr found : date = %s\n",cuat->value,NULL)
		if (cuat->value[0] == '9')
		{  /* check the year, then the month, then the day */
			if (cuat->value[1] >= '1')
				use_pvid = TRUE;
			else if ((cuat->value[2] == '1') || (cuat->value[3] > '2'))
		 use_pvid = TRUE;
			else if ((cuat->value[3] == '2') && (cuat->value[4] >= '2'))
		 use_pvid = TRUE;
		}
		else if (cuat->value[0] == '0')
			/* assuming first decade of 2000 */
			use_pvid = TRUE;
	} 

	DATA("---------------------- exit function check_ROS_level --------------\n",
			NULL,NULL)

	return( use_pvid );
}


/*------------------------------- scsi_device  --------------------------------
 * NAME:scsi_device
 *																						  
 * FUNCTION: initializes iplist with information for scsi devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	ip_index	- index into iplist
 *	iplist		 - byte array which will be written into NVRAM
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
#define SCSI_DISK		1
#define SCSI_DISKETTE		2
#define SCSI_CDROM		3
#define SCSI_RMT		4

#define NOPVID_LENGTH		17
#define SCSI_LENGTH		8
#define PVID_LENGTH		17
#define PVID_SIZE		PVID_LENGTH-1
#define PVID_CODE		'V'
#define SCSI_CODE		'S'
#define SCSI_INTERNAL		'I'
#define SCSI_EXTERNAL		'E'
#define SCSI_ADPID		7

scsi_device( device, scsi_type )
struct device_node *device;			/* ptr to device info */
int scsi_type;								/* scsi type */
{ char criteria[MAX_CRITELEM_LEN];  /* scsi type */
  struct CuDv *cudv;						/* CuDv ptr */
  struct listinfo cudv_info;			/* odm info */
  struct CuDv *parent;					/* parent CuDv ptr */
  struct listinfo parent_info;		/* odm info */
  struct CuDv *cu;						/* tmp CuDv ptr */
  struct listinfo cu_info;				/* odm info */
  struct PdAt *pdat;						/* PdAt ptr */
  struct listinfo pdat_info;			/* odm info */
  struct CuAt *cuat;						/* CuAt ptr */
  struct listinfo cuat_info;			/* odm info */
  char *ptr;								/* tmp ptr */
  int i,j,len;									/* loop indexing */
  char temp[80];							/* temp str */
  int use_pvid;							/* >0 if ok to use pvid */
  unsigned int bus_id;					/* bus id for this device */
  uchar sid,lun;						/* for getting sid and lun */

	DATA("***************** function scsi_device ****************************\n",
			NULL,NULL)
	DATA("   device name = %s\n",device->name,NULL)

	/* get this device out of CuDv */
	sprintf( criteria, "name = '%s'", device->name );
	if ((int)(cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if ((!cudv_info.num) || (cudv->status != AVAILABLE))
		do_error(NOT_AVAILABLE,device->name,FALSE);

	/* check the ROS level - anything before 2/22/90 will not accept a pvid */
	use_pvid = check_ROS_level();
	if ( use_pvid )
	{  /* check for a PVid for this device */
		DATA("   ok to use pvid - looking for one\n",NULL,NULL)
		sprintf( criteria, "name = '%s' and attribute = 'pvid'", device->name );
		if ((int)(cuat = odm_get_list(CuAt_CLASS,criteria,&cuat_info,1,1))<0)
			do_error(ODM_ACCESS_ERR,"CuAt",FALSE);
		if (cuat_info.num)
		{  /* it has a PVid - is the enough room in the iplist??? */
			DATA("   pvid found\n",NULL,NULL)
			if ((ip_index + 1 + PVID_LENGTH+SCSI_LENGTH) < IPLIST_LENGTH)
			{  /* add the PVid to the iplist */
				/* set the length */
				iplist[++ip_index] = PVID_LENGTH + SCSI_LENGTH;

				iplist[++ip_index] = PVID_CODE;

				/* convert the PVid */
				DATA("   pvid = %s\n", cuat->value, NULL)
				for (i=0,ptr=cuat->value; i < 16; i++)
				{  for (j=0; j < 2; j++,ptr++)
						temp[j] = *ptr;
					temp[j] = '\0';
					iplist[++ip_index] = strtol(temp,NULL,16);
				}
			}
			else /* not enough room */
				do_error(NO_ROOM,device->name,FALSE);
		}
		else /* no pvid */
		{	DATA("   no pvid found for %s\n",device->name,NULL)
			use_pvid = FALSE;
		}
	}

	if (!use_pvid)
	{  /* no PVid - is there enough room for scsi??? */
		DATA("   pvid will NOT be used\n",NULL,NULL)
		if ((ip_index + 1 + NOPVID_LENGTH) < IPLIST_LENGTH)
			iplist[++ip_index] = NOPVID_LENGTH;
		else
			do_error(NO_ROOM,device->name,FALSE);

		/* set the PVID field to zeros */
		for (i=0; i < (NOPVID_LENGTH-SCSI_LENGTH); i++)
			iplist[++ip_index] = 0;
	}

	/* get the "bus id" */
	bus_id = get_bus_id( cudv, &parent );

	/* add the SCSI code next */
	iplist[++ip_index] = SCSI_CODE;

	/* add the bus id next */
	iplist[++ip_index] = bus_id;
	DATA("   setting bus_id = 0x%x\n", iplist[ip_index], NULL)

        /* SCSI bus connection code next */
        /* if location specifies an S or 0, then the bus
           connection is internal */
        if (cudv->location[7] == 'S' || cudv->location[7] == '0' ) {
           iplist[++ip_index] = SCSI_INTERNAL;
        }
        else { /* parent's location specifies a 1, then external */
           iplist[++ip_index] = SCSI_EXTERNAL;
        }


        /* if integrated SCSI, then slot number is 14, subtract
           one to keep same scheme as others */
        /* adapter slot number : 1-8 in database */
        if ( cudv->location[4] == '0')  {
           iplist[++ip_index] = 0xd ;
        }
        else {
           iplist[++ip_index] = cudv->location[4] - '0' - 1 ;
        }
        DATA("   adapter location = %s; using %d in the list\n",
                cudv->location,iplist[ip_index])

	/* get parent's scsid from "id" attr */
	sprintf( criteria, "name = '%s' and attribute = 'id'", cudv->parent );
	DATA("   looking for id : crit = \"%s\"\n",criteria,NULL)
	if ((int)(cuat = (struct CuAt *) 
					odm_get_list(CuAt_CLASS,criteria,&cuat_info,1,1))<0)
		do_error(ODM_ACCESS_ERR,"CuAt",FALSE);
	if (cuat_info.num)
	{
		iplist[++ip_index] = cuat->value[0] - '0';
		DATA("   attr found : value = %s; using %d in the list\n",
				cuat->value,iplist[ip_index])
	}
	else
	{	/* use the default value */
		sprintf( criteria, "uniquetype = '%s' and attribute = 'id'", 
					parent->PdDvLn_Lvalue );
		DATA("   no CuAt found - looking for PdAt :\n\t   crit = \"%s\"\n",
				criteria,NULL)
		if ((int)(pdat = (struct PdAt *) 
						odm_get_list(PdAt_CLASS,criteria,&pdat_info,1,1))<0)
			do_error(ODM_ACCESS_ERR,"PdAt",FALSE);
		if (pdat_info.num)
		{
			iplist[++ip_index] = pdat->deflt[0] - '0';
			DATA("   PdAt found; value = %s; using %d in the list\n",
					pdat->deflt,iplist[ip_index])
		}
		else
		{	DATA("   no PdAt - using the default of %d\n",SCSI_ADPID,NULL)
			iplist[++ip_index] = SCSI_ADPID;
		}
	}
		
	/* type */
	iplist[++ip_index] = scsi_type;
	DATA("   scsi type = %d\n",iplist[ip_index],NULL)


        /* scsi id - must seperate it from the lun */
        /* scsi id & lun expected in this format : */
        /*              "x,y" where x=scsid & y=lun */

		get_sid_lun(cudv->connwhere, &sid, &lun);
        iplist[++ip_index] = sid;

        DATA("   scsi id = %d\n",iplist[ip_index],NULL)

        /* lun */
        iplist[++ip_index] = lun;
	DATA("   lun = %d\n",iplist[ip_index],NULL)

	DATA("***************** exit function scsi_device ***********************\n",
			NULL,NULL)
}


/*------------------------------- serdasd_device  ------------------------------
 * NAME: serdasd_device
 *																						  
 * FUNCTION: initializes iplist with information for scsi devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	ip_index	- index into iplist
 *	iplist		 - byte array which will be written into NVRAM
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/

serdasd_device( device )
struct device_node *device;		/* ptr to device info */
{ char criteria[MAX_CRITELEM_LEN];  	/* scsi type */
  struct CuDv *cudv;			/* CuDv ptr */
  struct listinfo cudv_info;		/* odm info */
  struct CuDv *parent;			/* parent CuDv ptr */
  struct listinfo parent_info;		/* odm info */
  struct CuDv *cu;			/* tmp CuDv ptr */
  struct listinfo cu_info;		/* odm info */
  struct CuAt *cuat;			/* CuAt ptr */
  struct listinfo cuat_info;		/* odm info */
  char *ptr;				/* tmp ptr */
  int i,j;				/* loop indexing */
  char temp[80];			/* temp str */
  unsigned int bus_id;			/* bus id for this device */
  int error;				/* odm_run_method error */
  char *out_ptr, *err_ptr;		/* buffer ptrs for odm_run_method */
  int use_pvid;				/* >0 if pvid to be used */
  int scsi_id;				/* tmp for scsi id */

	DATA("******************* function serdasd_device **********************\n",
			NULL,NULL)
	DATA("   device name = %s\n", device->name,NULL)

	/* get this device out of CuDv */
	sprintf( criteria, "name = '%s'", device->name );
	if ((int)(cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if ((!cudv_info.num) || (cudv->status != AVAILABLE))
		do_error(NOT_AVAILABLE,device->name,FALSE);

	/* check the ROS - can it boot a serial dasd disk ???? */
	DATA("   checking the ROS : can it boot a serial dasd disk ?\n",NULL,NULL)
	sprintf(temp,"-B %s",device->name);
	DATA("   executing : \"bootinfo %s\"\n",temp,NULL)
	if ((error = 
			odm_run_method("/usr/sbin/bootinfo",temp,&out_ptr,&err_ptr)) == -1)
		do_error(NOT_BOOTABLE,device->name,FALSE);
	else
	{	/* check the output : bootinfo prints a "1" if the device is supported */
		if (out_ptr[0] != '1')
			do_error(NOT_BOOTABLE,device->name,FALSE);
		else
			DATA("   ROS does support this device as a boot device\n",NULL,NULL)
	}

	/* check the ROS level - anything before 2/22/90 will not accept a pvid */
	use_pvid = check_ROS_level();
	if ( use_pvid )
	{  /* check for a PVid for this device */
		DATA("   ok to use pvid - looking for one\n",NULL,NULL)
		sprintf( criteria, "name = '%s' and attribute = 'pvid'", device->name );
		if ((int)(cuat = odm_get_list(CuAt_CLASS,criteria,&cuat_info,1,1))<0)
			do_error(ODM_ACCESS_ERR,"CuAt",FALSE);
		if (cuat_info.num)
		{  /* it has a PVid - is the enough room in the iplist??? */
			DATA("   pvid found\n",NULL,NULL)
			if ((ip_index + 1 + PVID_LENGTH+SCSI_LENGTH) < IPLIST_LENGTH)
			{  /* add the PVid to the iplist */
				/* set the length */
				iplist[++ip_index] = PVID_LENGTH + SCSI_LENGTH;

				iplist[++ip_index] = PVID_CODE;

				/* convert the PVid */
				DATA("   pvid = %s\n", cuat->value, NULL)
				for (i=0,ptr=cuat->value; i < 16; i++)
				{  for (j=0; j < 2; j++,ptr++)
						temp[j] = *ptr;
					temp[j] = '\0';
					iplist[++ip_index] = strtol(temp,NULL,16);
				}
			}
			else /* not enough room */
				do_error(NO_ROOM,device->name,FALSE);
		}
		else /* no pvid */
		{	DATA("   no pvid found for %s\n",device->name,NULL)
			use_pvid = FALSE;
		}
	}

	if (!use_pvid)
	{  /* no PVid - is there enough room for scsi??? */
		DATA("   pvid will NOT be used\n",NULL,NULL)
		if ((ip_index + 1 + NOPVID_LENGTH) < IPLIST_LENGTH)
			iplist[++ip_index] = NOPVID_LENGTH;
		else
			do_error(NO_ROOM,device->name,FALSE);

		/* set the PVID field to zeros */
		for (i=0; i < (NOPVID_LENGTH-SCSI_LENGTH); i++)
			iplist[++ip_index] = 0;
	}

	/* get the "bus id" */
	bus_id = get_bus_id( cudv, &parent );

	/* add the SCSI code next */
	iplist[++ip_index] = SCSI_CODE;

	/* add the bus id next */
	iplist[++ip_index] = bus_id;
	DATA("   setting bus_id = 0x%x\n", iplist[ip_index], NULL)

	/* SCSI internal code next */
	iplist[++ip_index] = SCSI_INTERNAL;

	/* for Harrier, device info is structured like this : */
	/*		cudv.parent.parent.connwhere specifies the slot # */
	/*		the adapter scsi id is 0 */
	/*		cudv.parent.connwhere specifies the scsi id */
	/*		cudv.connwhere specifies the LUN, which can be from "1" to "15" */
	scsi_id = atoi( parent->connwhere );

	/* get the "slot" info (comes from grandparent's info) */
	DATA("   getting grandparent (%s) info to obtain the slot\n",
			parent->parent,NULL)
	sprintf( criteria, "name = '%s'", parent->parent );
	if ((int)(parent = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if ((!cudv_info.num) || (cudv->status != AVAILABLE))
		do_error(NOT_AVAILABLE,parent->name,FALSE);
	iplist[++ip_index] = atoi(parent->connwhere) - 1;
	DATA("   slot = %d\n", iplist[ip_index], NULL)

	/* adapter scsi id is 0 for Harrier */
	iplist[++ip_index] = 0;
		
	/* type */
	iplist[++ip_index] = SCSI_DISK;
	DATA("   scsi type = %d\n",iplist[ip_index],NULL)

	/* scsi id - comes from parent's connwhere */
	iplist[++ip_index] = scsi_id;
	DATA("   scsi id = %d\n",iplist[ip_index],NULL)

	/* lun */
	iplist[++ip_index] = atoi( cudv->connwhere );
	DATA("   lun = %d\n",iplist[ip_index],NULL)

	DATA("******************** exit function serdasd_device *****************\n",
			NULL,NULL)
}

/* @@SSA_BEGIN@@ */
/*------------------------------- ssa_device  ------------------------------
 * NAME: ssa_device
 *
 * FUNCTION: initializes iplist with information for ssa devices
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	ip_index	- index into iplist
 *	iplist		- byte array which will be written into NVRAM
 *
 * RETURNS: None
 *		
 *---------------------------------------------------------------------------*/

#define SSA_LENGTH	16
#define SSA_ADAP_LENGTH	2
#define SSA_MIN_LENGTH	(SSA_LENGTH + SSA_ADAP_LENGTH)
#define SSA_MAX_ADAPS	16
#define SSA_CODE	'T'

#define SSAR_SUBCLASS	"ssar"
#define SSA_SUBCLASS	"ssa"

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct _ssa_adapter_t			/* SSA adapter		*/
{
    unsigned int  bus_id;			/* ID of parent bus	*/
    unsigned int  slot;				/* Slot in this bus	*/
} ssa_adapter_t, * pssa_adapter_t;

void ssa_device( struct device_node * device )
{ 
    char criteria[MAX_CRITELEM_LEN]; 	/* ODM search string		*/
    struct CuDv *cudv;			/* CuDv ptr for hdisk		*/
    struct CuDv *cudv_adapter;		/* CuDv ptr for adapter		*/
    struct CuDv cudv_object;		/* CuDv object retrieved 	*/
    struct CuDv cudv_adapter_object;	/* CuDv object retrieved 	*/
    struct CuDv *parent;		/* parent CuDv ptr 		*/
    struct PdDv *pddv;			/* PdDv ptr for hdisk		*/
    struct PdDv pddv_object;		/* PdDv object retrieved	*/
    struct listinfo cuat_info;		/* odm info 			*/
    struct CuAt * cuat_adapter;		/* CuAt adapter ptr		*/
    struct CuAt * cuat_serialnum;	/* CuAt serial number ptr	*/
    int i;				/* loop indexing 		*/
    int error;				/* odm_run_method error 	*/
    char * out_ptr;			/* odm_run_method stdout	*/
    char * err_ptr;			/* odm_run_method stderr	*/
    ssa_adapter_t   Adapters[SSA_MAX_ADAPS];	/* SSA Adapters		*/
    ssa_adapter_t * pAdapter;		/* Current adapter		*/
    int	 AdaptersFound;			/* How many adapters found 	*/
    int	 MaxSize;			/* Room needed in iplist	*/
    int	 AvailRoom;			/* Room available in iplist	*/
    int	 AdaptersToAdd;			/* Adapters to add to iplist	*/

    DATA( "*********** function ssa_device **************\n", NULL, NULL );
    DATA( "   device name = %s\n", device->name, NULL );

/************************************************************************/
/* Get this device out of CuDv, and its PdDv also			*/
/************************************************************************/

    sprintf( criteria, "name = '%s'", device->name );
    cudv = odm_get_first( CuDv_CLASS, criteria, &cudv_object );

    if( (int)cudv == -1 )
    {
	do_error( ODM_ACCESS_ERR, "CuDv", FALSE );
    }

    if( cudv->status != AVAILABLE )
    {
	do_error( NOT_AVAILABLE, device->name, FALSE );
    }

/************************************************************************/
/* Check to see if there is room for the minimum bootlist		*/
/************************************************************************/

    if( ip_index + 1 + SSA_MIN_LENGTH > IPLIST_LENGTH )
    {
	do_error( NO_ROOM, device->name, FALSE );
    }

/************************************************************************/
/* Find all the adapters which get us to this disk 			*/
/************************************************************************/

    AdaptersFound = 0;
    pAdapter      = Adapters;

    sprintf( criteria, "uniquetype = '%s'", cudv->PdDvLn_Lvalue );

    pddv = odm_get_first( PdDv_CLASS, criteria, &pddv_object );

    if( (int)pddv == -1 )
    {
	do_error( ODM_ACCESS_ERR, "PdDv", FALSE );
    }

    if( !strcoll( pddv->subclass, SSAR_SUBCLASS ) )
    {
	DATA( "Checking all routes to ssar owned %s\n", device->name, NULL );

/************************************************************************/
/* Get all attributes for this device of type 'adapter_?'		*/
/************************************************************************/

	sprintf( criteria, "name = '%s' and attribute like 'adapter_?'",
		 device->name );

	cuat_adapter = odm_get_list( CuAt_CLASS,
				     criteria,
				     &cuat_info,
				     SSA_MAX_ADAPS, 1 );
	    
	if( (int)cuat_adapter == -1 )
	{
	    do_error( ODM_ACCESS_ERR, "CuAt", FALSE );
	}

	if( cuat_adapter == NULL )
	{
	    do_error( NO_PARENT, cudv->name, FALSE );
	}

/************************************************************************/
/* Look for the adapter's CuDv objects (which should be available)	*/
/************************************************************************/

	for( i = 0; i < cuat_info.num; i++ )
	{
	    sprintf( criteria,
		     "name = '%s' and status = 1",
		     cuat_adapter[ i ].value );

	    cudv_adapter = odm_get_first( CuDv_CLASS,
					  criteria,
					  &cudv_adapter_object );

	    if( (int)cudv_adapter == -1 )
	    {
		do_error( ODM_ACCESS_ERR, "CuDv", FALSE );
	    }

	    if( cudv_adapter != NULL )
	    {
/************************************************************************/
/* Get the adapters parent bus ID, and its slot number			*/
/************************************************************************/

		pAdapter->bus_id = get_bus_id( cudv_adapter, &parent );
		pAdapter->slot   = atoi( cudv_adapter->connwhere ) - 1;
		pAdapter++;
		AdaptersFound++;
	    }
	}
    }
    else
    {
/************************************************************************/
/* Its simply owned by one adapter					*/
/************************************************************************/

	DATA( "device %s is simply owned.\n", device->name, NULL );

	pAdapter->bus_id = get_bus_id( cudv, &parent );
	pAdapter->slot   = atoi( parent->connwhere ) - 1;
	AdaptersFound    = 1;
    }

/************************************************************************/
/* Verify that we found at least one adapter okay			*/
/************************************************************************/

    if( AdaptersFound == 0 )
    {
	do_error( NOT_BOOTABLE, device->name, FALSE );
    }

/************************************************************************/
/* Deduce how big the bootlist entry will be				*/
/************************************************************************/

    MaxSize       = (AdaptersFound * SSA_ADAP_LENGTH) + SSA_LENGTH;
    AvailRoom     = min( IPLIST_LENGTH - (ip_index + 1), MaxSize );
    AdaptersToAdd = (AvailRoom - SSA_LENGTH) / SSA_ADAP_LENGTH;

    DATA( "Adding %d adapters for disk %s\n", AdaptersToAdd, device->name );

/************************************************************************/
/* Create the standard bootlist header of length and ID byte		*/
/************************************************************************/

    iplist[ ++ip_index ] = (AdaptersToAdd * SSA_ADAP_LENGTH) + SSA_LENGTH;
    iplist[ ++ip_index ] = SSA_CODE;

/************************************************************************/
/* Store the serial number - from connwhere 				*/
/************************************************************************/

    for( i=0; i<15; i++ )
    {
	iplist[ ++ip_index ] = cudv->connwhere[ i ];
    }

/************************************************************************/
/* Now add each adapter							*/
/************************************************************************/

    pAdapter = Adapters;
    while( AdaptersToAdd-- )
    {
	iplist[ ++ip_index ] = pAdapter->bus_id;
	iplist[ ++ip_index ] = pAdapter->slot;
	pAdapter++;
    }

    DATA( "************ exit function ssa_device *********\n", NULL, NULL );
}
/* @@SSA_END@@   */


/*------------------------------ scdisk_device  --------------------------------
 * NAME:scdisk_device
 *																						  
 * FUNCTION: initializes iplist with information for scdisk devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
scdisk_device( device )
struct device_node *device;			/* ptr to device info */
{
	/* this is a scsi device */
	scsi_device( device, SCSI_DISK );
}

/*------------------------------- hdisk_device  --------------------------------
 * NAME:hdisk_device
 *																						  
 * FUNCTION: initializes iplist with information for hdisk devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
#define SCSI_SUBCLASS			"scsi"
#define BADISK_SUBCLASS			"mca"
#define SERDASD_SUBCLASS		"serdasdc"

#define BADISK_LENGTH			2
#define BADISK_CODE			'K'

hdisk_device( device )
struct device_node *device;		/* ptr to device info */
{ char criteria[MAX_CRITELEM_LEN];  	/* odm search criteria */
  struct CuDv *cudv;			/* CuDv ptr */
  struct listinfo cudv_info;		/* odm info */
  char str[80];				/* temp str */
  int i;										/* loop indexing */
  int max;				/* strlen */

	/* get this device out of CuDv */
	sprintf( criteria, "name = '%s'", device->name );
	if ((int)(cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,2)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if (cudv_info.num > 0)
	{  /* check the subclass for the disk type */
		if (cudv->PdDvLn == NULL)
			do_error(ODM_ACCESS_ERR,"CuDv->PdDv",FALSE);

		if (!strcoll( cudv->PdDvLn->subclass, SCSI_SUBCLASS ))
			scsi_device( device, SCSI_DISK );
		else if (!strcoll( cudv->PdDvLn->subclass, SERDASD_SUBCLASS ))
			serdasd_device( device );
/* @@SSA_BEGIN@@ */
		else if (!strcoll( cudv->PdDvLn->subclass, SSAR_SUBCLASS ))
			ssa_device( device );
/* @@SSA_END@@   */
		else if (!strcoll( cudv->PdDvLn->subclass, BADISK_SUBCLASS ))
		{  /* is there enough room to add this device??? */
			if ((ip_index + 1 + BADISK_LENGTH) < IPLIST_LENGTH)
			{  /* initialize the iplist */
				iplist[++ip_index] = BADISK_LENGTH;
				iplist[++ip_index] = BADISK_CODE;

				/* look at the connwhere field - should be either */
				/* '7' or '8', so subtract a '6' to get a 1 or 2 */
				iplist[++ip_index] = cudv->connwhere[0] - '6';
			}
			else
				do_error(NO_ROOM,device->name,FALSE);
		}
		else
			do_error(UNKNOWN_DISK_TYPE,cudv->PdDvLn->subclass,FALSE);
	}
	else
		do_error(BAD_NAME,device->name,FALSE);
}

/*------------------------------- cdrom_device  --------------------------------
 * NAME:cdrom_device
 *																						  
 * FUNCTION: initializes iplist with information for cdrom devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
cdrom_device( device )
struct device_node *device;			/* ptr to device info */
{
/* @@SSA_BEGIN@@ */
	struct CuDv * cudv;			/* CuDv object pointer	*/
        struct CuDv   cudv_object;		/* CuDv object		*/
	struct PdDv * pddv;			/* PdDv object pointer	*/
        struct PdDv   pddv_object;		/* PdDv object		*/
	char criteria[MAX_CRITELEM_LEN];	/* search criteria	*/

/************************************************************************/
/* Get this device out of CuDv, and get its PdDv                        */
/************************************************************************/

	sprintf( criteria, "name = '%s'", device->name );
	cudv = odm_get_first( CuDv_CLASS, criteria, &cudv_object );

	if( (int)cudv == -1 )
	{
		do_error( ODM_ACCESS_ERR, "CuDv", FALSE );
	}

/************************************************************************/
/* Call the correct function based on the subtype of the device         */
/************************************************************************/

	if( cudv != NULL )
	{
		sprintf( criteria, "uniquetype = '%s'", cudv->PdDvLn_Lvalue );
		pddv = odm_get_first( PdDv_CLASS, criteria, &pddv_object );

		if( (int)pddv == -1 )
		{
			do_error( ODM_ACCESS_ERR, "PdDv", FALSE );
		}

		if( !strcoll( pddv->subclass, SCSI_SUBCLASS ) )
		{
			scsi_device( device, SCSI_CDROM );
		}
		else if( !strcoll( pddv->subclass, SSA_SUBCLASS ) ||
				   !strcoll( pddv->subclass, SSAR_SUBCLASS ) )
		{
			ssa_device( device );
		}
		else
		{
			do_error( UNKNOWN_DISK_TYPE, device->name, FALSE );
		}
	}
	else
	{
		do_error( BAD_NAME, device->name, FALSE );
	}

/* @@SSA_END@@ */
}


/*------------------------------- rmt_device  --------------------------------
 * NAME:rmt_device
 *																						  
 * FUNCTION: initializes iplist with information for rmt devices
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
rmt_device( device )
struct device_node *device;			/* ptr to device info */
{
/* @@SSA_BEGIN@@ */
	struct CuDv * cudv;			/* CuDv object pointer	*/
        struct CuDv   cudv_object;		/* CuDv object		*/
	struct PdDv * pddv;			/* PdDv object pointer	*/
        struct PdDv   pddv_object;		/* PdDv object		*/
	char criteria[MAX_CRITELEM_LEN];	/* search criteria	*/

/************************************************************************/
/* Get this device out of CuDv                                          */
/************************************************************************/

	sprintf( criteria, "name = '%s'", device->name );
	cudv = odm_get_first( CuDv_CLASS, criteria, &cudv_object );

	if( (int)cudv == -1 )
	{
		do_error( ODM_ACCESS_ERR, "CuDv", FALSE );
	}

/************************************************************************/
/* Call the correct function based on the subtype of the device         */
/************************************************************************/

	if( cudv != NULL )
	{
		sprintf( criteria, "uniquetype = '%s'", cudv->PdDvLn_Lvalue );
		pddv = odm_get_first( PdDv_CLASS, criteria, &pddv_object );

		if( (int)pddv == -1 )
		{
			do_error( ODM_ACCESS_ERR, "PdDv", FALSE );
		}

		if( !strcoll( pddv->subclass, SCSI_SUBCLASS ) )
		{
			scsi_device( device, SCSI_RMT );
		}
		else if( !strcoll( pddv->subclass, SSA_SUBCLASS ) ||
				   !strcoll( pddv->subclass, SSAR_SUBCLASS ) )
		{
			ssa_device( device );
		}
		else
		{
			do_error( BAD_NAME, device->name, FALSE );
		}
	}
	else
	{
		do_error( BAD_NAME, device->name, FALSE );
	}

/* @@SSA_END@@ */
}


/*-------------------- str to byte conversion for net addrs ------------------*/
union convert
{
	unsigned long l;
	unsigned char bytes[4];
}

#define MAX_HARDWARE_BYTES		6

str_to_hardware(str,bytes)
char *str;
unsigned char *bytes;
{
	int i;
	int count;
	int len;
   char tmp[3];

	/* initialize bytes */
	for (i=0; i < MAX_HARDWARE_BYTES; bytes[i++] = 0);

	/* the string (str) represents a hardware address, which is 12 hex digits */
	/* these digits need to get converted into 6 bytes, so peel off 2 hex */
	/*		digits at a time */
	len = strlen( str );
	i = count = 0;
	while ((count <= MAX_HARDWARE_BYTES) && (str[i] != '\0'))
	{
		tmp[0] = str[i++];
		if (str[i] != '\0')
		{	tmp[1] = str[i++];
			tmp[2] = '\0';
		}
		else
			tmp[1] = '\0';

		*bytes = (char) strtol(tmp,NULL,16);
		bytes++;
		count++;
	}
}


/*------------------------------- net_device --------------------------------*/
net_device( device, device_code )
struct device_node *device;
char device_code;
{  char criteria[MAX_CRITELEM_LEN];  /* odm search criteria */
	struct 	CuDv *cudv;
	struct 	listinfo cudv_info;
	struct 	attr_node *attr;
	union 	convert bserver;
	union 	convert client;
	union 	convert gateway;
	unsigned char hardware[MAX_HARDWARE_BYTES];
	int 	bserver_present	 = FALSE;
	int 	client_present	 = FALSE;
	int 	gateway_present = FALSE;
	int 	hardware_present = FALSE;
	char 	*tmp;
	uint 	slot;
	int 	length = 0;
	int 	i;
	struct 	CuDv *parent;
	unsigned int bus_id;

	DATA("function net_device : device name = %s; device code = %c\n",
			device->name,device_code)

	/* get the CuDv entry */
	sprintf(criteria, "name = %s", device->name );
	if ((int)(cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,1,1)) < 0)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	if ((!cudv_info.num) || (cudv->status != AVAILABLE))
		do_error(NOT_AVAILABLE,device->name,FALSE);

	/* get the bus id */
	bus_id = get_bus_id( cudv, &parent );

	/* get the slot number */
	slot = (atoi(cudv->connwhere) - 1) & 0xf;

	DATA("   device in slot %d; using \"%d\" in the bootlist\n",slot+1,slot)

	/* convert any attrs to ROS format */
	DATA("   looking for attrs\n",NULL,NULL)
	for (attr=device->attrs; attr != NULL; attr=attr->next)
		if (!strcmp(attr->name,"bserver"))
		{  bserver.l = inet_addr( attr->value );
			bserver_present = TRUE;
			DATA("   bootp server attr : addr = %s\n",attr->value,NULL)
		}
		else if (!strcmp(attr->name,"client"))
		{  client.l = inet_addr( attr->value );
			client_present = TRUE;
			DATA("   client attr : addr = %s\n",attr->value,NULL)
		}
		else if (!strcmp(attr->name,"gateway"))
		{  gateway.l = inet_addr( attr->value );
			gateway_present = TRUE;
			DATA("   gateway attr : addr = %s\n",attr->value,NULL)
		}
		else if (!strcmp(attr->name,"hardware"))
		{	if (	(strchr(attr->value,'.')) || 
					(strchr(attr->value,':')) ||
					(strlen(attr->value) != 12) )
				do_error(BAD_HARDWARE_ADDR,NULL,FALSE);
			DATA("   hardware attr : addr = %s\n",attr->value,NULL)
		   str_to_hardware( attr->value, hardware );
			hardware_present = TRUE;
		}

	/* check for illegal attr combinations */
	if (gateway_present)
	{	if (!(bserver_present && client_present))
			do_error(ILLEGAL_ATTR_COMBO,device->name,FALSE);
		else if (hardware_present)
			length = 25;
		else
			length = 18;
	}
	else if (bserver_present)
	{  if (client_present)
			do_error(ILLEGAL_ATTR_COMBO,device->name,FALSE);
		else if (hardware_present)
			length = 15;
		else
			length = 8;
	}
	else if (client_present || hardware_present)
		do_error(ILLEGAL_ATTR_COMBO,device->name,FALSE);
	else
		length = 3;

	/* is there enough room ? */
	if ((ip_index + 1 + length) >= IPLIST_LENGTH)
		do_error(NO_ROOM,device->name,FALSE);

	/* fill in the temp iplist */
	iplist[++ip_index] = length;
	iplist[++ip_index] = device_code;
	iplist[++ip_index] = bus_id;
	iplist[++ip_index] = slot;

	if (gateway_present)
	{  iplist[++ip_index] = 'W';
		iplist[++ip_index] = gateway.bytes[0];
		iplist[++ip_index] = gateway.bytes[1];
		iplist[++ip_index] = gateway.bytes[2];
		iplist[++ip_index] = gateway.bytes[3];
		if (hardware_present)
		{  iplist[++ip_index] = 'H';
			for (i=0; i < MAX_HARDWARE_BYTES; i++)
				iplist[++ip_index] = hardware[i];
			hardware_present = FALSE;
		}
	}

	if (bserver_present)
	{  iplist[++ip_index] = 'B';
		iplist[++ip_index] = bserver.bytes[0];
		iplist[++ip_index] = bserver.bytes[1];
		iplist[++ip_index] = bserver.bytes[2];
		iplist[++ip_index] = bserver.bytes[3];
		if (hardware_present)
		{  iplist[++ip_index] = 'H';
			for (i=0; i < MAX_HARDWARE_BYTES; i++)
				iplist[++ip_index] = hardware[i];
		}
	}

	if (client_present)
	{  iplist[++ip_index] = 'L';
		iplist[++ip_index] = client.bytes[0];
		iplist[++ip_index] = client.bytes[1];
		iplist[++ip_index] = client.bytes[2];
		iplist[++ip_index] = client.bytes[3];
	}
}


/*----------------------------- ent_device ----------------------------------*/

#define ENT_CODE		'D'
#define TOK_CODE		'O'
#define FDDI_CODE		'P'

ent_device( device )
struct device_node *device;
{
	net_device( device, ENT_CODE );
}

/*----------------------------- tok_device ----------------------------------*/
tok_device( device )
struct device_node *device;
{
	net_device( device, TOK_CODE );
}

/*----------------------------- fddi_device ----------------------------------*/
fddi_device( device )
struct device_node *device;
{
	net_device( device, FDDI_CODE );
}

/*------------------------------- read_list  --------------------------------
 * NAME:read_list
 *																						  
 * FUNCTION: reads the specified IPL list and writes it to stdout
 *																						  
 * EXECUTION ENVIRONMENT:
 *																						 
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *	ip_index	- index into iplist
 *	iplist		 - byte array which will be written into NVRAM
 *
 * RETURNS: 
 *		
 *---------------------------------------------------------------------------*/
read_list( address, length, title )
int address;		/* address to read from */
int length;		/* number of bytes to read */
int title;		/* which message to print as a title */
{ MACH_DD_IO nv;	/* ioctl struct */
  int fd;		/* fd for NVRAM */
  int i,j;		/* loop indexing */
  char ch;		/* tmp char */
  char temp[100];	/* tmp str */
  char str[100];	/* tmp str */

	/* open NVRAM */
	if ((fd = open(NVRAM,2)) < 0)
		do_error(OPEN_ERR,NVRAM,FALSE);

	/* initialize the machine device driver record */
	nv.md_incr = MV_BYTE;
	nv.md_data = iplist;
	nv.md_size = length;
	nv.md_addr = address;

	/* get the data from NVRAM */
	if (ioctl(fd,MIONVGET,&nv) < 0)
		do_error(IOCTL_ERR,errno,FALSE);

	close( fd );

	/* print the info */
	printf(MSGSTR(title),nv.md_addr);

	/* print 10 hex numbers, then there corresponding ASCII equivalents */
	sprintf(str,"\t\t");
	for (i=0,j=0; i < nv.md_size; i++)
	{  ch = nv.md_data[i];
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
			 (ch >= 'a' && ch <= 'z'))
		  sprintf(temp,"%c ", nv.md_data[i]);
		else
		  sprintf(temp,"- ");
		strcat(str,temp);
		printf("%2x ", nv.md_data[i]);
		if (++j == 10)
		{  j = 0;
			printf("%s\n",str);
			sprintf(str,"\t\t");
		}
	}
	if ((j) && (j < 10))
	{  temp[0] = '\0';
		for (; j < 10; j++)
		  strcat(temp,"   ");
		printf("%s",temp);
	}
	printf("%s\n\n",str);
}


#ifdef BOOT_DEBUG
int pr_iplist()
{int i,j;
char str[80];
char temp[80];
char ch;

	if (__rspc()) {
		printf("%s\n\n",bootlist);
		return(0);
	}

	/* print 10 hex numbers, then there corresponding ASCII equivalents */
	sprintf(str,"\t\t");
	for (i=0,j=0; i < IPLIST_LENGTH; i++)
	{  ch = iplist[i];
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
			 (ch >= 'a' && ch <= 'z'))
		  sprintf(temp,"%c ", iplist[i]);
		else
		  sprintf(temp,"- ");
		strcat(str,temp);
		printf("%2x ", iplist[i]);
		if (++j == 10)
		{  j = 0;
			printf("%s\n",str);
			sprintf(str,"\t\t");
		}
	}
	if ((j) && (j < 10))
	{  temp[0] = '\0';
		for (; j < 10; j++)
		  strcat(temp,"   ");
		printf("%s",temp);
	}
	printf("%s\n\n",str);
}
#endif

/*
 * NAME: get_sid_lun
 *
 * FUNCTION: Extracts the sid, and lun from a SCSI address
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *      assumed.
 * 2. The basic function is to convert the input string
 *    from the form sss-lll to uchar values (i.e. the sid, and the lun )
 *    sss  = 1 to 3 decimal characters for the sid
 *    lll  = 1 to 3 decimal characters for the lun
 *
 * RETURNS:
 * 0 for success, -1 for failure.
 *
 */

int get_sid_lun( scsiaddr, sid_addr, lun_addr )
char    *scsiaddr;
uchar   *sid_addr;
uchar   *lun_addr;
{

	if (*scsiaddr == '\0') return -1;
	if ( strchr(scsiaddr,',') == NULL) return -1;

/* We utilize the behavior of strtoul which stops converting characters at
   the first non-base character Thus after the start position is set, the
   conversion stops either at the ',' or at the NULL end-of-string */

	*sid_addr = (uchar)strtoul(scsiaddr,NULL,10);
	*lun_addr = (uchar)strtoul(strchr(scsiaddr,',')+1,NULL,10);

        return 0;
}



/*
 * NAME: bootlist_support
 *
 * FUNCTION: Determines if model supports setting of bootlist.
 *
 * RETURNS:
 * 0 = bootlist supported, 1 = not supported
 *
 */

int
bootlist_support(void)
{
	CFG_VPD 	vpd;


	/* bootlist with no options - indicate if bootlist can be set */
	if (__rspc()) {
		/* For RSPC models, look at what firmware supports */
		if (get_resid_vpd(&vpd) == 0) {
#ifdef BOOT_DEBUG
fprintf(stderr,"FirmwareSupports = %x\n",vpd.FirmwareSupports);
#endif
			if (vpd.FirmwareSupports & 0x800)
				return(0);
		}
		return(1);
	}

	/* All other models support setting bootlist */
	return(0);
}



int
rspc_write_list(list)
char	*list;
{
	if (put_nv_env(list)) {
		do_error(CANT_WRITE_LIST,NULL,FALSE);
	}
	return(0);
}


int
rspc_read_list(void)
{
	char	*p1;
	char	*p2;
	char	odm_name[16];
	int	devcnt = 0;
	int	rc;


	if (!get_nv_env("fw-boot-path=",bootlist)) {
#ifdef BOOT_DEBUG
fprintf(stderr,"%s\n",bootlist);
#endif

		p1 = &bootlist[13];
		while(p1) {
			devcnt++;

			p2 = (char *)strchr(p1,';');
			if (p2) {
				*p2 = '\0';
				p2++;
			}
			DATA("fw-boot-path string for device %d = (%s)\n",
								devcnt,p1)
			rc = get_bootdev_odm_name(p1,odm_name);
			if (rc) {
				printf("-\n");
			}
			else {
				printf("%s\n",odm_name);
			}

			p1 = p2;
		}
	}
	else {
#ifdef BOOT_DEBUG
fprintf(stderr,"No fw-boot-path set\n");
#endif
	}
	return(0);
}



int
rspc_device(dev)
struct device_node *dev;
{
	char	bootstring[512];
	int	rc;
	struct CuDv cudv;
	struct PdDv pddv;
	char	sstr[128];


#ifdef BOOT_DEBUG
fprintf(stderr,"In rspc_device() for device: '%s'\n",dev->name);
#endif

	rspc_dev_cnt++;
	if (rspc_dev_cnt > MAX_RSPC_DEVICES) {
		do_error(NO_ROOM,dev->name,FALSE);
	}

	/* Get CuDv object and check device status */
	sprintf(sstr,"name=%s",dev->name);
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&cudv);
	if (rc == -1)
		do_error(ODM_ACCESS_ERR,"CuDv",FALSE);
	else if ((rc == 0) || (cudv.status != AVAILABLE))
		do_error(NOT_AVAILABLE,dev->name,FALSE);


	/* Get PdDv object */
	sprintf(sstr,"uniquetype=%s",cudv.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstr,&pddv);
	if (rc == -1 || rc == 0)
		do_error(ODM_ACCESS_ERR,"PdDv",FALSE);

	rc = mk_nv_list(&cudv, &pddv, bootstring);
	if (rc) {
		do_error(CANT_MAKE_LIST,dev->name,FALSE);
	}

#ifdef BOOT_DEBUG
fprintf(stderr,"boot list for device: '%s'\n",bootstring);
#endif

	if (ip_index == 1) {
		strcpy(bootlist,"fw-boot-path=");
		ip_index = 13;
	}
	else {
		strcat(bootlist,";");
		ip_index++;
	}

	if ((ip_index + strlen(bootstring)) <= BOOTLIST_LTH) {
		strcat(bootlist,bootstring);
		ip_index += strlen(bootstring);
	}
	else {
		do_error(NO_ROOM,dev->name,FALSE);
	}

	return(0);
}

int
get_nv_env(keyword,data)
char	*keyword;
char	*data;

{
	int	fd;
	MACH_DD_IO      mdd;
	int	rc;


	mdd.md_addr = (int)keyword;
	mdd.md_data = data;
	mdd.md_size = 2048;
	mdd.md_incr = 0;
	mdd.md_sla = 0;
	mdd.md_length = 0;

	fd = open("/dev/nvram",O_RDONLY, 0);
	if (fd == -1)
		return(-1);
	rc = ioctl(fd,MIOGEARD,&mdd);
	close(fd);
	return(rc);
}


int
put_nv_env(data)
char	*data;

{
	int	fd;
	MACH_DD_IO      mdd;
	int	rc;


	mdd.md_addr = (int)data;
	mdd.md_data = 0;
	mdd.md_size = 0;
	mdd.md_incr = 0;
	mdd.md_sla = 0;
	mdd.md_length = 0;

	fd = open("/dev/nvram",O_RDWR, 0);
	if (fd == -1)
		return(-1);
	rc = ioctl(fd,MIOGEAUPD,&mdd);
	close(fd);
	return(rc);
}
