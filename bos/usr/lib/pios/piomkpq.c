static char sccsid[] = "@(#)82  1.6  src/bos/usr/lib/pios/piomkpq.c, cmdpios, bos411, 9428A410j 3/31/94 11:28:29";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, getmsg, print_msg, make_device,
 *            multi_byte_check, valid_chars, quedev_exist,
 *            queue_exist, make_queues, error_exit
 *            
 *            
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Note:	"mkp_system" function uses code borrowed from "system" function
 *		in Standard C Library.  The main difference between these two
 *		functions is, the former directly executes a given
 *		command, whereas the latter uses "/usr/bin/sh" to execute the
 #		command..
 */

/*********************   include files  ************************************/
#define _ILS_MACROS
#include <sys/limits.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <nl_types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <sys/id.h>
#include <sys/wait.h>
#include <ctype.h>
#include "piobe_msg.h"

/********************* default messages ************************************/
#define SMITMSGCAT      "piosmit.cat"
#define MF_PIOBESETNO	(8)
#define DEFAULT_ARG 	"%s: Missing argument for the flag -%c\n"
#define DEFAULT_ILLOPT	"%s: Illegal flag -%c\n"
#define DEFMSG_LVP_MALLOCERR	"%s: Error '%s' in malloc()\n"
#define DEFMSG_LVP_FOPENERR		"%s: Error '%s' in opening the file %s\n"
#define DEF_MKPQ_BAD_QNAME  	"Invalid print queue name '%s'.\n"\
	"\tName must not exceed 20 characters and may only\n"\
	"\tcontain the characters [a-z], [A-Z], [0-9], @, -, or _.\n"
#define DEF_NO_CREATE 		"Unable to create print queue.\n"
#define DEF_DEVICE_DONE		"Added printer '%s'.\n"
#define DEF_QUEUE_DONE		"Added print queue '%s'.\n"
#define DEF_QDEV_DONE		"Added printer '%s' to print queue '%s'.\n"
#define DEF_QD_EXISTS 		"Queue device '%s' already exists on print "\
	"queue '%s'.\nUnable to add queue device.\n"
#define DEF_Q_NOT_EXIST		"Queue '%s' does not exist. \n"
#define DEF_Q_MISSING		"At least one print queue name must be specified.\n"
#define DEF_DUP_Q			"Print queue '%s' specified more than once.  \n"\
	"\tDuplicate names not allowed.\n"  
#define DEF_DUP_PRT			"Print queue '%s' already contains printer '%s'.\n"
#define DEF_MKPQ_USAGE 		"Usage:\n"\
	"piomkpq -A local -p PrinterType -d DeviceName { -D DataStream -q QueueName }...\n"\
	"piomkpq -A local -p PrinterType -d DeviceName -Q QueueName -D DataStream\n"\
	"piomkpq -A local -p PrinterType { -D DataStream -q QueueName }...\n"\
	"\t-s SubClass -r ParentAdapter -w PortNo -v DeviceType\n"\
	"\t[ -a {interface|ptop|autoconfig|speed|parity|bpc|stops|xon|dtr|tbc}=DescVal]...\n"\
	"piomkpq -A local -p PrinterType -Q QueueName -D DataStream -v DeviceType\n"\
	"\t-s SubClass -r ParentAdapter -w PortNo\n"\
	"\t[ -a {interface|ptop|autoconfig|speed|parity|bpc|stops|xon|dtr|tbc}=DescVal]...\n"\
	"piomkpq -A xstation -p PrinterType -d PseudoDeviceName\n"\
	"\t{ -D DataStream -q QueueName }...\n"\
	"piomkpq -A xstation -p PrinterType -d PseudoDeviceName\n"\
	"\t-Q QueueName -D DataStream\n"\
	"piomkpq -A xstation -p PrinterType { -D DataStream -q QueueName }...\n"\
	"\t-x XstationName -t XstationType -P XstationPort\n"\
	"\t[ -a {speed|parity|bpc|stops}=DescVal ]...\n"\
	"piomkpq -A xstation -p PrinterType -Q QueueName -D DataStream\n"\
	"\t-x XstationName -t XstationType -P XstationPort\n"\
	"\t[ -a {speed|parity|bpc|stops}=DescVal ]...\n"\
	"piomkpq -A ascii -p PrinterType -d TtyName { -D DataStream -q QueueName }...\n"\
	"piomkpq -A ascii -p PrinterType -d TtyName -Q QueueName -D DataStream\n"\
	"piomkpq -A file -p PrinterType -d FileName { -D DataStream -q QueueName }...\n"\
	"piomkpq -A file -p PrinterType -d FileName -Q QueueName -D DataStream\n"\
	"piomkpq -A OtherAttachmentType -p PrinterType -d DeviceName\n"\
	"\t{ -D DataStream -q QueueName }... -b 'QueueAttribute=Value'...\n"\
	"\t-c 'QueueDeviceAttribute=Value'...\n"

/************************ macros ******************************************/
#define DEFBASEDIR 		"/usr/lib/lpd/pio"      /* default base directory */
#define DEFVARDIR    	"/var/spool/lpd/pio/@local"    /* default base directory */
#define DEFMC_PREFIXPATH	"/usr/lib/lpd/pio/etc"
#define DEVDIR			"/dev/"
#define WKBUFLEN        (1024)
#define MAXQATSSIZ	(4*WKBUFLEN)
#define MAXQDATSSIZ	(4*WKBUFLEN)
#define DSFLAG			"-D"
#define QFLAG			"-q"
#define LOCAL_ATTACH    "local"
#define XSTA_ATTACH     "xstation"
#define FILE_ATTACH     "file"
#define ASCII_ATTACH    "ascii"
#define MKDEV			"/usr/sbin/mkdev"
#define RMDEV			"/usr/sbin/rmdev"
#define MGPDEV			"/usr/lib/lpd/pio/etc/piomgpdev"
#define DEVNULL         ">/dev/null 2>&1"
#define LSQUE			"/usr/bin/lsque"
#define LSQUEDEV		"/usr/bin/lsquedev"
#define MKQUE			"/usr/bin/mkque"
#define MKQUEDEV		"/usr/bin/mkquedev"
#define RMQUE			"/usr/bin/rmque"
#define RMQUEDEV		"/usr/bin/rmquedev"
#define PIOLPX			"/usr/lib/lpd/pio/etc/piolpx"
#define PIOBE			"/usr/lib/lpd/piobe"
#define MKVIRPRT		"/usr/sbin/mkvirprt"
#define RMVIRPRT		"/usr/sbin/rmvirprt"
#define CUSTP			"/usr/lib/lpd/pio/etc/piocustp"
#define CATCMD			"/usr/bin/cat"
#define XSTPDATSEP		"#"
#define MKP_ARGMAX		(127)
#define ARRAYSIZ(a)		(sizeof(a)/(sizeof(*(a))))

/************************ typedefs ******************************************/
typedef struct {
	char					*datastream;
	char					*queue;
} qlistm_t;

/************************ global variables ********************************/
static const char 		*pgm_name;				/* used for error message */
static char             basedir[PATH_MAX+1];	/* printer backend base dir */
static char             vardir[PATH_MAX+1]; 	/* printer backend var dir */
struct {
	char					*attach;
	char					*ptype;
	char					*device;
	char					*datastream;
	char					*existqueue;
	char					*subclass;
	char					*parent;
	char					*port;
	char					*dtype;
	char					attrs[WKBUFLEN];
	char					*xstation;
	char					*xtype;
	char					*xport;
	char					*qatsp;
	char					*qdatsp;
} flags;

char						*rmlist[200]; /* allocate space for 200 remove commands */
int							rmnum = 0;
qlistm_t 					*qlist; 		/* array of new queue to create */
int							qnum = 0;		/* number of elements in the array */
FILE						*msgfp;

uid_t						real_uid;
uid_t						saved_uid;
int						rmdev_flg = -1;
	
/************************ function prototypes ********************************/

static void print_msg(FILE *, int , const char *, ...);
static char const *getmsg(const char*, int, int );
void get_queues(int, char **);
char *make_device(void);
int multi_byte_check(char *);
int valid_chars(char *);
int quedev_exist(char *, char *);
int queue_exist(char *);
void make_queues(void);
void error_exit(void);
static int mkp_system(char const *);

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           main                                                         *
*                                                                              *
* DESCRIPTION:    Performs main logic.                                         *
*                                                                              *
*                 First parse command line for '-D -q' pairs.                  *
*                 Parse the rest of the command line.                          *
*                 If the '-d' flag was not specified then create               *
*                    a device for local and xstation attachments.              *
*                 Make all the queues specified.                               *
*                                                                              *
*                                                                              *
*                                                                              *
*                                                                              *
* PARAMETERS:     ac            arg count                                      *
*                 av            arg vector                                     *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
int main(int argc, char **argv)
{

int 					c;
extern int 				optind;
extern int				opterr;
extern int				optopt;
extern char				*optarg;
const char				*cp;                /* tmp pointer */
char					msgfilename[L_tmpnam];  /* file used for saving status msgs */
char					tcmd[WKBUFLEN]; 	/* used to print out messages */


	/* Query real and saved user ids and set effective user to real id. */
	real_uid 	= getuidx(ID_REAL);
	saved_uid 	= getuidx(ID_SAVED);
	(void) seteuid(real_uid);

	(void) setlocale(LC_ALL, "");

	pgm_name = *argv;   /* used by error routines */

	(void)tmpnam(msgfilename);   /* get tmp file name */
	if (!(msgfp = fopen(msgfilename,"w+")))  
        {
            print_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, pgm_name,
            strerror(errno), msgfilename);
            exit(EXIT_FAILURE);
        }

	if (argc == 1)  /* if no arguments then error */
	{
		print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
		exit(EXIT_FAILURE);
	}

    /* set the paths to our backend stuff */
    if (cp = getenv("PIOBASEDIR"))
    {
        (void)strncpy(basedir,cp,sizeof(basedir)-1);
        *(basedir+sizeof(basedir)-1) = 0;
    }
    else
    {
        (void)strncpy(basedir,DEFBASEDIR,sizeof(basedir)-1);
        *(basedir+sizeof(basedir)-1) = 0;
    }

    if (cp = getenv("PIOVARDIR"))
    {
        (void)strncpy(vardir,cp,sizeof(vardir)-1);
        *(vardir+sizeof(vardir)-1) = 0;
    }
    else
    {
        (void)strncpy(vardir,DEFVARDIR,sizeof(vardir)-1);
        *(vardir+sizeof(vardir)-1) = 0;
    }

	get_queues(argc,argv);  /* build 'qlist' with '-D -q' pairs from command line */

	opterr = 0;
	while ((c = getopt(argc,argv,":A:p:d:D:q:Q:s:r:w:v:a:x:t:P:b:c:")) !=
	       EOF)
	{
		switch(c)
		{
			case 'A':   /* attachment type */
			 	flags.attach = optarg;	
				break;

			case 'p' :
				flags.ptype = optarg;
				break;

			case 'd' :
				flags.device = optarg;
				break;

			case 'D' :
				flags.datastream = optarg;
				break;

			case 'q' :  /* do nothing since we've already processed these */
				break;

			case 'Q' :
				flags.existqueue = optarg;
				break;

			case 's' :
				flags.subclass = optarg;
				break;

			case 'r' :
				flags.parent = optarg;
				break;

			case 'w' :
				flags.port = optarg;
				break;

			case 'v' :
				flags.dtype = optarg;
				break;

			case 'a' : /* cat the -a flags together to use in making the device */
				(void)strncat(flags.attrs," -a ",sizeof(flags.attrs)-5);
				(void)strncat(flags.attrs,optarg,sizeof(flags.attrs)-
							strlen(flags.attrs)-1);
				break;

			case 'x' :
				flags.xstation = optarg;
				break;

			case 't' :
				flags.xtype = optarg;
				break;

			case 'P' :
				flags.xport = optarg;
				break;

			case 'b':
				if (!flags.qatsp)
				if (!(flags.qatsp = malloc((size_t)MAXQATSSIZ)))
				{
		    			print_msg(stderr, MSG_LVP_MALLOCERR,
						DEFMSG_LVP_MALLOCERR,
		             			pgm_name, strerror(errno));
		    			exit(EXIT_FAILURE);
				}
				else
				   *flags.qatsp = 0;
				(void)strncat(flags.qatsp," -a '",
					MAXQATSSIZ-strlen(flags.qatsp));
				(void)strncat(flags.qatsp,optarg,
					MAXQATSSIZ-strlen(flags.qatsp));
				(void)strncat(flags.qatsp,"'",
					MAXQATSSIZ-strlen(flags.qatsp));
				break;
				
			case 'c':
				if (!flags.qdatsp)
				if (!(flags.qdatsp = 
					malloc((size_t)MAXQATSSIZ)))
				{
		    			print_msg(stderr, MSG_LVP_MALLOCERR,
						DEFMSG_LVP_MALLOCERR,
		             			pgm_name, strerror(errno));
		    			exit(EXIT_FAILURE);
				}
				else
				   *flags.qdatsp = 0;
				(void)strncat(flags.qdatsp," -a '",
					MAXQATSSIZ-strlen(flags.qdatsp));
				(void)strncat(flags.qdatsp,optarg,
					MAXQATSSIZ-strlen(flags.qdatsp));
				(void)strncat(flags.qdatsp,"'",
					MAXQATSSIZ-strlen(flags.qdatsp));
				break;
				
			case ':':
				print_msg(stderr,MSG_LVP_MISSARG,DEFAULT_ARG,pgm_name,
						(char)optopt);
				print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
				exit(EXIT_FAILURE);

			case '?':
				print_msg(stderr,MSG_LVP_ILLOPT,DEFAULT_ILLOPT,pgm_name,
						(char)optopt);
                print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
				exit(EXIT_FAILURE);
		}
	}

	if ((qnum == 0) && (flags.existqueue == NULL))
	{
		print_msg(stderr,MKPQ_Q_MISSING,DEF_Q_MISSING);
		exit(EXIT_FAILURE);
	}

	/* If queue or queue device stanza flags were specified, make sure that
	   a non-supported attachment type was selected. */
	if ((flags.qatsp || flags.qdatsp) &&
	    (!strcmp(flags.attach,LOCAL_ATTACH) ||
	     !strcmp(flags.attach,XSTA_ATTACH) ||
	     !strcmp(flags.attach,ASCII_ATTACH) ||
	     !strcmp(flags.attach,FILE_ATTACH)))
                print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE),
		exit(EXIT_FAILURE);

    /* make the device if necessary */
	if ((strcmp(flags.attach,LOCAL_ATTACH) == 0) ||
		(strcmp(flags.attach,XSTA_ATTACH) == 0))
	{
		if (flags.device == NULL) 
			flags.device = make_device();   
	}

	make_queues();   /* create all the necessary print queues */

    /* since we are here everything was successful, print out status */
	(void)fclose(msgfp);
	(void)sprintf(tcmd,"%s %s",CATCMD,msgfilename);
	(void)system(tcmd);   /* cat the tmp file containg status to stdout */
	(void)unlink(msgfilename);

	return(0);
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           make_device                                                  *
*                                                                              *
* DESCRIPTION:    Creates an xstation or local device.                         *
*                                                                              *
*                 Make the device with stdout pointing to a tmp file.          *
*                 Read the tfile for the device name and return it.          *
*                                                                              *
*                                                                              *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:  dname        pointer to new device name                      *
*                                                                              *
*                                                                              *
*******************************************************************************/
char *make_device(void)
{
	static char dname[WKBUFLEN];
	char		cmdstr[PATH_MAX+1];
	char		line[WKBUFLEN];
	char		tfile[L_tmpnam];
	FILE		*fp;
	int		savfd;


	if (strcmp(flags.attach,LOCAL_ATTACH) == 0)
	{
		(void)tmpnam(tfile);   /* get tmp file name */

		(void)sprintf(cmdstr,"%s -c printer -t %s -s %s \
			 	-p %s -w %s %s", MKDEV,flags.dtype,flags.subclass,
				flags.parent,flags.port,flags.attrs);

		/* Execute 'mkdev' command with root authority. */
		if (!(fp = fopen(tfile,"w")))	/* to store the device name */
		{
			print_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, pgm_name,
			strerror(errno), tfile);
			exit(EXIT_FAILURE);
		}
		(void) fflush(stdout);
		savfd = dup(fileno(stdout));
		(void) fflush(fp);
		(void) dup2(fileno(fp),fileno(stdout));
		(void) seteuid(saved_uid);
		if (mkp_system(cmdstr) != 0)  /* execute the mkdev command */
			exit(EXIT_FAILURE);
		(void) seteuid(real_uid);
		(void) fflush(stdout);
		(void) dup2(savfd,fileno(stdout));
		(void) fclose(fp);
		(void) close(savfd);

		if (fp = fopen(tfile,"r"))  /* get the device name from the file */
		{
			(void)fgets(line,sizeof(line),fp);
			if (strlen(line) > 0)
				(void)strcpy(dname,strtok(line," "));
		}
		else
		{
			print_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, pgm_name,
			strerror(errno), tfile);
			exit(EXIT_FAILURE);
		}

		/* set up the remove command */
		rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
		if (rmlist == NULL)
		{
		    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
		             pgm_name, strerror(errno));
		    exit(EXIT_FAILURE);
		}
		(void)sprintf(rmlist[rmnum],"%s -l %s -d -q",RMDEV,dname);
		rmdev_flg = rmnum++;

		(void)fclose(fp);
		(void)unlink(tfile);
	}
	else if (strcmp(flags.attach,XSTA_ATTACH) == 0)
	{
		(void)strcpy(dname,flags.xport);
		(void)strcat(dname,"@");
		(void)strcat(dname,flags.xstation);
		(void)sprintf(cmdstr,"%s -p '%s' -A -t '%s' -a 'xstation=%s'\
					-a 'interface=%s' -a 'port=%s' %s",MGPDEV,dname,
					flags.attach,flags.xstation,
					(flags.xport[0]=='p')?"parallel":"serial",
					flags.xport,flags.attrs);

		if (system(cmdstr) != 0)  /* execute the piomgpdev command */
			exit(EXIT_FAILURE);

		rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
		if (rmlist == NULL)
		{
		    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
		             pgm_name, strerror(errno));
		    exit(EXIT_FAILURE);
		}
		(void)sprintf(rmlist[rmnum],"%s -R -p '%s' -t '%s'",MGPDEV,dname,flags.attach);
		rmnum++;
	}

	print_msg(msgfp, MKPQ_DEVICE_DONE, DEF_DEVICE_DONE , dname);
	(void)fputc('\n',msgfp);

	return(dname);

}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           make_queues                                                  *
*                                                                              *
* DESCRIPTION:    Make all the queues specified on the command line.           *
*                                                                              *
*                 Loop once for each queue:                                    *
*                     Get the queue and queue device names.                    *
*                     Validate the queue and queue device names.               *
*                     Build the 'file=' and 'backend=' values.                 *
*                     If the queue does not already exist:                     *
*                          Make the queue.                                     *
*                          Make the queue device.                              *
*                          Make the virtual printer.                           *
*                     Else if the queue does exist:                            *
*                          If the queue device already exists, error.          *
*                          Make the queue device.                              *
*                          Make the virtual printer.                           *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:  None.                                                        *
*                                                                              *
*                                                                              *
*******************************************************************************/
void make_queues(void)
{

int 					i;
char					q[WKBUFLEN];
char					qd[WKBUFLEN];
char					*ptr = NULL;
int						they_typed;
char					wkbuf[WKBUFLEN];
char					filestr[WKBUFLEN];
char					backend[WKBUFLEN];
char					cmd[MAXQATSSIZ+WKBUFLEN];

 /* if this is add an additional printer to an existing queue then
    do the main loop once */
	if (flags.existqueue != NULL)  
		qnum = 1;

    /* loop once for each queue/datastream pair */
	for (i = 0; i < qnum; i++)
	{
        /* get the queue and queue device name */
		they_typed = 0;
		if (flags.existqueue == NULL)
		{
			(void)strcpy(q,qlist[i].queue);  /* queue name came from '-q' flag */
			flags.datastream = qlist[i].datastream; 
		}
		else
			(void)strcpy(q,flags.existqueue); /* queue name came from '-Q' flag */

		ptr = strchr(q, ':');
		if (ptr != NULL)
		{
	   		*ptr = '\0';
			ptr++;  /* ptr is now pointing to the queue device */
			(void)strcpy(qd,ptr);
			(void)strtok(qd, "\n");
		 	they_typed++;	 /* they maually entered the queue-device name */
		}
		else
		{
			(void)strncpy(qd,flags.device,20); 
			qd[20] = '\0';
			/* for xstations queue device is only to first '.' */
			ptr = strchr(qd,'.');
			if (ptr != NULL) 
				*ptr = '\0';
		}

		/* validate the queue name */
		if ((q[0] == '\0') || (multi_byte_check(q) == 1) ||
			(valid_chars(q) == 1) || (strlen(q) > 20))
		{
			print_msg(stderr, MKPQ_BAD_QNAME, DEF_MKPQ_BAD_QNAME,q);
			print_msg(stderr, MKPQ_NO_CREATE, DEF_NO_CREATE);
			error_exit();   /* exit and cleanup anthing already created */
		}

        /* validate the queue-device name */
		if ((qd[0] == '\0') || (multi_byte_check(qd) == 1) ||
			(valid_chars(qd) == 1) || (strlen(qd) > 20))
		{
			(void)strcat(q,":");
			(void)strcat(q,qd);
			print_msg(stderr, MKPQ_BAD_QNAME, DEF_MKPQ_BAD_QNAME,q);
			print_msg(stderr, MKPQ_NO_CREATE, DEF_NO_CREATE);
			error_exit();   /* exit and cleanup anthing already created */
		}

		/* set the 'file =' value and the 'backend =' value */
		if (strcmp(flags.attach,XSTA_ATTACH) == 0)
		{
			(void)strncpy(filestr,vardir,sizeof(filestr)-1);
			filestr[sizeof(filestr)-1] = '\0';
			(void)strncat(filestr,DEVDIR,sizeof(filestr) - strlen(filestr));
			(void)strncat(filestr,flags.device,sizeof(filestr) - strlen(filestr));
			(void)strncat(filestr,XSTPDATSEP XSTA_ATTACH,sizeof(filestr) - strlen(filestr));

			(void)strncpy(backend,PIOLPX,sizeof(backend)-1);
			backend[sizeof(backend)-1] = '\0';
			(void)strncat(backend," ",sizeof(backend)-strlen(backend));
			(void)strncat(backend,flags.device,sizeof(backend)-strlen(backend)); 
		}
		else if (!flags.qdatsp)
		{
			(void)strncpy(filestr,DEVDIR,sizeof(filestr)-1);
			filestr[sizeof(filestr)-1] = '\0';
			(void)strncat(filestr,flags.device,sizeof(filestr) - strlen(filestr));

			(void)strncpy(backend,PIOBE,sizeof(backend) - strlen(backend) - 1);
			backend[sizeof(backend)-1] = '\0';
		}


 		if (!queue_exist(q))  /* queue does not exist */
		{
			if (flags.existqueue != NULL) /* error: the queue should have existed */
			{
				print_msg(stderr,MKPQ_Q_NOT_EXIST,DEF_Q_NOT_EXIST,q);
				print_msg(stderr, MKPQ_NO_CREATE, DEF_NO_CREATE);
				error_exit();	/* exit and clean up */
			}

 			/* make the queue */
 			(void)sprintf(cmd,"%s -q %s%s",MKQUE,q,flags.qatsp?
				flags.qatsp:"");
			if (system(cmd) != 0) 
				error_exit();

			rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
			if (rmlist == NULL)
			{
			    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
			             pgm_name, strerror(errno));
			    error_exit();  /* exit and clean up */
			}
			(void)sprintf(rmlist[rmnum],"%s -q %s",RMQUE,q);  /* generate remove command */
			rmnum++;

			/* make the queue device */
			if (flags.qdatsp)
				(void)sprintf(cmd,"%s -q %s -d %s%s",MKQUEDEV,q,
					qd,flags.qdatsp);
			else
			(void)sprintf(cmd,"%s -q %s -d %s -a 'file = %s' -a %s -a %s %s \
				 -a 'backend = %s'", MKQUEDEV, q, qd, filestr,
				"'header = never'", "'trailer = never'",
				 "-a 'access = both'", backend);
			if (system(cmd) != 0)  /* execute the mkquedev command */
				error_exit();

			rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
			if (rmlist == NULL)
			{
			    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
			             pgm_name, strerror(errno));
			    error_exit();  /* exit and clean up */
			}
			(void)sprintf(rmlist[rmnum],"%s -q %s -d %s", RMQUEDEV, q, qd);
			rmnum++;

			/* make the virtual printer */ 
			if (strcmp(flags.attach,ASCII_ATTACH) == 0) /* if ascii, set PIOprgname */
				(void)sprintf(cmd,"PIOprgname=%s %s -q %s -d %s -n %s -s %s -t %s -A %s",
						CUSTP,MKVIRPRT,q,qd,flags.device,flags.datastream,flags.ptype,
						flags.attach);
			else
				(void)sprintf(cmd,"%s -q %s -d %s -n %s -s %s -t %s -A %s",
						MKVIRPRT,q,qd,flags.device,flags.datastream,flags.ptype,
						flags.attach);

			if (system(cmd) != 0)  /* execute the mkvirprt command */
				error_exit();

		    /* generate remove command in case we need to clean up later */
			rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
			if (rmlist == NULL)
			{
			    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
			             pgm_name, strerror(errno));
			    error_exit();  /* exit and clean up */
			}
			(void)sprintf(rmlist[rmnum],"%s -q %s -d %s",RMVIRPRT,q,qd);
			rmnum++;

			print_msg(msgfp, MKPQ_QUEUE_DONE, DEF_QUEUE_DONE ,q);
		}
		else  /* queue already exists */
		{
			if (quedev_exist(q,qd))
				if (they_typed) /* if queue-device exists & they typed it, error */
				{
					print_msg(stderr, MKPQ_QD_EXISTS,DEF_QD_EXISTS,qd,q);
					print_msg(stderr, MKPQ_NO_CREATE, DEF_NO_CREATE);
					error_exit();
				}
				else  /* the printer already exists on the queue, error */
				{
					print_msg(stderr, MKPQ_DUP_PRT,DEF_DUP_PRT,q,qd);
					print_msg(stderr, MKPQ_NO_CREATE, DEF_NO_CREATE);
					error_exit();
				}

			/* we now have a unique queue-device name so make it */
			if (flags.qdatsp)
				(void)sprintf(cmd,"%s -q %s -d %s%s",MKQUEDEV,q,
					qd,flags.qdatsp);
			else
			(void)sprintf(cmd,"%s -q %s -d %s -a 'file = %s' -a %s -a %s %s \
				 -a 'backend = %s'", MKQUEDEV, q, qd, filestr,
				"'header = never'", "'trailer = never'",
				 "-a 'access = both'", backend);

			if (system(cmd) != 0)  /* execute the mkquedev command */
				error_exit();

			rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
			if (rmlist == NULL)
			{
			    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
			             pgm_name, strerror(errno));
			    error_exit();  /* exit and clean up */
			}
			(void)sprintf(rmlist[rmnum],"%s -q %s -d %s", RMQUEDEV, q, qd);
			rmnum++;

			/* queue device is made so now make the virtual printer */
			if (strcmp(flags.attach,ASCII_ATTACH) == 0) /* if ascii, set PIOprgname */
				(void)sprintf(cmd,"PIOprgname=%s %s -q %s -d %s -n %s -s %s -t %s -A %s",
						CUSTP,MKVIRPRT,q,qd,flags.device,flags.datastream,flags.ptype,
						flags.attach);
			else
				(void)sprintf(cmd,"%s -q %s -d %s -n %s -s %s -t %s -A %s",
						MKVIRPRT,q,qd,flags.device,flags.datastream,flags.ptype,
						flags.attach);

			if (system(cmd) != 0)  /* execute the mkvirprt command */
				error_exit();

		    /* generate remove command */
			rmlist[rmnum] = malloc((size_t)(PATH_MAX*(rmnum+1)));
			if (rmlist == NULL)
			{
			    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
			             pgm_name, strerror(errno));
			    error_exit();  /* exit and clean up */
			}
			(void)sprintf(rmlist[rmnum],"%s -q %s -d %s",RMVIRPRT,q,qd);
			rmnum++;

			print_msg(msgfp, MKPQ_QDEV_DONE, DEF_QDEV_DONE ,qd,q);
		}

	}
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           multi_byte_check                                             *
*                                                                              *
* DESCRIPTION:    Checks for multi-byte characters in a string.                *
*                                                                              *
* PARAMETERS:     name      The name of the queue/queue-device to check.       *
*                                                                              *
* RETURN VALUES:  1         Multi-byte characters were found.                  *
*                 0         No multi-byte characters were found.               *
*                                                                              *
*******************************************************************************/
int multi_byte_check(char *name)
{
int length;

      length = 1;
      if (MB_CUR_MAX > 1)    /* then possibility of multi-byte characters  */
         {
         while (*name != '\0')
            {
            length = mblen(name,MB_CUR_MAX);
            if (length > 1)
               return(1);
            name++;
            }
         }

      return(0);   /* no multi-byte chars found */
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           valid_chars                                                  *
*                                                                              *
* DESCRIPTION:    Checks for valid characters in a queue or queue device name. *
*                 The only valid characters are [0-9], [a-z],                  *
*                 [A-Z], '_', '-', '+' and '@'                                 *
*                                                                              *
* PARAMETERS:     name        The queue name.                                  *
*                                                                              *
* RETURN VALUES:  1           The name was bad.                                *
*                 0           The name was ok.                                 *
*                                                                              *
*******************************************************************************/
int valid_chars(char *name)
{
   char *ptr;

   for(ptr = name; *ptr != '\0'; ptr++)
      if ((*ptr<'+') || (*ptr>'+' && *ptr<'-') || (*ptr>'-' && *ptr<'0') ||
          (*ptr>'9' && *ptr<'@') || (*ptr>'Z' && *ptr<'_') ||
          (*ptr>'_' && *ptr<'a') || (*ptr>'z'))
             return(1);   /* bad character found */
   return(0);
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           queue_exist                                                  *
*                                                                              *
* DESCRIPTION:    Checks if a queue exists or not.                             *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:  1          If queue does exist.                              *
*                 0          If queue does not exist.                          *
*                                                                              *
*******************************************************************************/
int queue_exist(char *qname)
{
char		cmd[PATH_MAX+1];

  (void)sprintf(cmd,"%s -q%s %s", LSQUE, qname, DEVNULL);
  return !system(cmd);
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           quedev_exist                                                 *
*                                                                              *
* DESCRIPTION:    Checks if a queue device exists or not.                      *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:  1       If queue device does exist.                          *
*                 0       If queue device does not exist.                      *
*                                                                              *
*******************************************************************************/
int quedev_exist(char *qname,char *qdname)
{
char		cmd[PATH_MAX+1];

  (void)sprintf(cmd,"%s -q%s -d%s %s", LSQUEDEV, qname, qdname, DEVNULL);
  return !system(cmd);
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           getmsg                                                       *
*                                                                              *
* DESCRIPTION:    Fetch an error message and put it in a msg buffer.           *
*                                                                              *
* PARAMETERS:     catnm        catalog name                                    *
*                 setno        set no.                                         *
*                 msgno        message no.                                     *
*                                                                              *
* RETURN VALUES:  message    success                                           *
*                 NULL        failure                                          *
*                                                                              *
*******************************************************************************/
static char const *
getmsg(const char *catnm, int setno, int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

   /* Fetch the specified message. */
   if (strcmp(catnm, prevcatnm))
   {
      (void) strncpy(prevcatnm, catnm, sizeof(prevcatnm)-1),
      *(prevcatnm+sizeof(prevcatnm)-1) = 0;
      if (md != (nl_catd)-1)
         md = ((void) catclose(md), (nl_catd)-1);
   }
   if (md == (nl_catd)-1  &&
       (md = catopen((char *)catnm, NL_CAT_LOCALE)) == (nl_catd)-1)
   {
      (void) strncpy(defmcpath, DEFMC_PREFIXPATH, sizeof(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      (void) strncat(defmcpath, catnm, sizeof(defmcpath)-strlen(defmcpath)-1);
      if ((md = catopen(defmcpath, NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md, setno, msgno, (char *)dmsg),
		 (char const *)dmsg) ? mp : NULL;
}	/* end - getmsg() */


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           print_msg                                                    *
*                                                                              *
* DESCRIPTION:    Fetch a message and put it in a given file.                  *
*                                                                              *
* PARAMETERS:     outfp		output file pointer 	                           *
*                 msgno		message no.                                   	*
*                 defmsg	default error message                          *
*                 ...		(stdarg)                                       *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static void print_msg(FILE *outfp, int msgno, const char *defmsg, ...)
{
   va_list			vap;
   const char		*msgp = getmsg(MF_PIOBE, MF_PIOBESETNO, msgno);

   /* Parse the message with the specified args. */
   if (!msgp)
      msgp = defmsg;
   va_start(vap, defmsg);
   (void) vfprintf(outfp, msgp, vap);
   va_end(vap);

   return;
}	/* end - print_msg() */


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:         get_queues                                                     *
*                                                                              *
* DESCRIPTION:  Parses the command line for the -D -q pairs that               *
*               specify the queue names and data stream type to                *
*               create.                                                        *
*                                                                              *
* PARAMETERS:   argc         Number of arguments on the command line           *
*               argv         Array of the arguments                            *
*                                                                              *
* RETURN VALUES:   None.                                                       *
*                                                                              *
*******************************************************************************/
void get_queues(int argc, char **argv)
{

int 					i;
int						x;
int						y;
char					*ptr;

	/* parse the command line and pick off the -D, -q combinations */
	for (i = 1; i < argc; i++)
	{
		if (strncmp(argv[i],QFLAG,2) == 0)  /* we found an '-q' on the cmd line */
		{
			/* get the queue name and add it to the list */
			if (strcmp(argv[i],QFLAG) != 0)  /* no space betwee -q and arg */
			{
				ptr = argv[i]+2; /* ptr is now pointing at the queue name */

				qlist = (qlistm_t *)realloc((void *)qlist,
				(size_t)(sizeof(qlistm_t)*(qnum+1)));
				if (qlist == NULL)
				{
				    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
				             pgm_name, strerror(errno));
				    exit(EXIT_FAILURE);
				}
				qlist[qnum].queue = ptr;  /* add queue to list */
			}
			else /* the queue is the next element of argv */
			{
				if (i < (argc - 1))  /* there is still at least 1 more option */
				{
	                qlist = (qlistm_t *)realloc((void *)qlist,
	                                    (size_t)(sizeof(qlistm_t)*(qnum+1)));
	                if (qlist == NULL)
	                {
	                    print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR,
	                             pgm_name, strerror(errno));
	                    exit(EXIT_FAILURE);
	                }

					qlist[qnum].queue = argv[i+1];  /* add queue to list */
				}
				else  /* there is not an argument for '-q' */
				{		
					print_msg(stderr,MSG_LVP_MISSARG,DEFAULT_ARG,pgm_name,'q');
					print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
					exit(EXIT_FAILURE);
				}
			}

			/* now get the datastream for this queue */
			if (strncmp(argv[i-1],DSFLAG,2) == 0)
			{
				ptr = argv[i-1]+2;  /* ptr is now pointing at the datastream */
				if (*ptr != '\0')  /* make sure there is an argument */
					qlist[qnum].datastream = ptr;
				else
				{
					print_msg(stderr,MSG_LVP_MISSARG,DEFAULT_ARG,pgm_name,'D');
					print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
					exit(EXIT_FAILURE);
				}
			}		
			else if (i > 1)
			{
					if (argv[i-1][0] != '-')  /* make sure previous option was
                                                 an argument not a flag */
					{
				 		if (strcmp(argv[i-2],DSFLAG) == 0)
							qlist[qnum].datastream = argv[i-1];
						else
						{
							print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
							exit(EXIT_FAILURE);
						}
					}
					else 
					{
						print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
						exit(EXIT_FAILURE);
					}
		 	}
			else /* not enough arguments */
			{	
				print_msg(stderr,MKPQ_USAGE,DEF_MKPQ_USAGE);
				exit(EXIT_FAILURE);
			}

			qnum++;  /* we successfully added one queue to the list */
		}
	}

	/* now check to make sure duplicate queue names were not passed in */
	for (x = 0; x < qnum; x++)
	{
		for (y = 0; y < qnum; y++)
		{

            if ((x != y) && (strcmp(qlist[x].queue,qlist[y].queue) == 0))
			{
		 	   print_msg(stderr,MKPQ_DUP_Q,DEF_DUP_Q,qlist[x].queue);
			   exit(EXIT_FAILURE);
			}
		}

	}


}


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:         error_exit                                                     *
*                                                                              *
* DESCRIPTION:  Exit routine that cleans up already made device and            *
*               queues.                                                        *
*                                                                              *
*                                                                              *
* PARAMETERS:  None.                                                           *
*                                                                              *
* RETURN VALUES:   None.                                                       *
*                                                                              *
*******************************************************************************/
void error_exit(void)
{
int 			i;

	if (rmdev_flg == -1)
	   for (i = rmnum-1; i >= 0; i--)
		(void)system(rmlist[i]);
	else
	   for (i = rmnum-1; i >= 0; i--)
	   {
		/* Execute 'rmdev' command with root authority. */
		if (i == rmdev_flg)
		   (void) seteuid(saved_uid),
		   (void) mkp_system(rmlist[i]),
		   (void) seteuid(real_uid);
		else
		   (void)system(rmlist[i]);
	   }

	exit(EXIT_FAILURE);
}


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           mkp_system                                                   *
*                                                                              *
* DESCRIPTION:    Execute a specified command.                                 *
*		  Parse the specified command into an argument vector and      *
*		  invoke execv() to execute the command.                       *
*                                                                              *
* PARAMETERS:     cmd		command string   	                       *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static int
mkp_system(const char *cmd)
{
	int	status;
	pid_t	pid;
	struct sigaction ointact,oquitact,ochldact,ignact;
	sigset_t savemask;
	char			*av[MKP_ARGMAX+1];
	register int		ac		= 0;
	register const char	*cp;
	register const char	*cp1;
	size_t			tlen;
	char			*cmdpath;

	if (cmd)
	{
		for (cp = cmd; *cp && isspace((int)*cp); cp++)
		   ;
		if (!*cp)
		   return(1);	/* Error, return zero */
		for (cp1 = cp; *cp1 && !isspace((int)*cp1); cp1++)
		   ;
		if (!(cmdpath = malloc((tlen = cp1-cp)+1)))
		{
		   print_msg(stderr, MSG_LVP_MALLOCERR,
			     DEFMSG_LVP_MALLOCERR,
	       		     pgm_name, strerror(errno));
	    	   exit(EXIT_FAILURE);
		}
		(void)memcpy((void *)cmdpath,(void *)cp,tlen),
		*(cmdpath+tlen) = 0;
		cp = (cp = strrchr(cmdpath,'/')) ? ++cp: cmdpath;
		if (!(av[ac] = malloc((tlen = strlen(cmdpath)-(cp-cmdpath))+1)))
		{
		   print_msg(stderr, MSG_LVP_MALLOCERR,
			     DEFMSG_LVP_MALLOCERR,
	       		     pgm_name, strerror(errno));
	    	   exit(EXIT_FAILURE);
		}
		(void)strcpy(av[ac],cp), ac++;
		for ( ; *cp1 && ac < ARRAYSIZ(av)-1; )
		{
		   for (cp = cp1; *cp && isspace((int)*cp); cp++)
		      ;
		   if (!*cp)
		      break;
		   for (cp1 = cp; *cp1 && !isspace((int)*cp1); cp1++)
		      ;
		   if (!(av[ac] = malloc((tlen = cp1-cp)+1)))
		   {
		      print_msg(stderr, MSG_LVP_MALLOCERR,
			        DEFMSG_LVP_MALLOCERR,
	       		        pgm_name, strerror(errno));
	    	      exit(EXIT_FAILURE);
		   }
		   (void)memcpy((void *)av[ac],(void *)cp,tlen),
		   *(av[ac]+tlen) = 0, ac++;
		}
		av[ac++] = NULL;
	}
	else			/* Error!!! */
		return(1);      /* return non zero */

	ignact.sa_handler = SIG_IGN;
	sigemptyset(&(ignact.sa_mask));
	ignact.sa_flags = 0 ;
	sigaction(SIGINT,&ignact,&ointact);
	sigaction(SIGQUIT,&ignact,&oquitact);

	sigaddset(&ignact.sa_mask,SIGCHLD);
	sigprocmask(SIG_BLOCK,&ignact.sa_mask,&savemask);

	ignact.sa_handler = SIG_DFL;
	sigemptyset(&(ignact.sa_mask));
	ignact.sa_flags = 0;
	sigaction(SIGCHLD,&ignact,&ochldact);

	switch(pid = kfork())
	{
	   case 0:			/* child process */
		sigaction(SIGINT,&ointact,NULL);
		sigaction(SIGQUIT,&oquitact,NULL);
		sigaction(SIGCHLD,&ochldact,NULL);
		sigprocmask(SIG_SETMASK,&savemask,NULL);

		(void) execv(cmdpath,av);
		exit(127);

	   case -1:			/* fork failed */
		status = -1;
		break;

	   default:			/* parent process */
		while (waitpid(pid,&status,0) == -1)
			if (errno != EINTR) {
				status = -1;
				break;
			}
		break;
	}

	/* Restore original signal states */
	sigaction(SIGINT,&ointact,NULL);
	sigaction(SIGQUIT,&oquitact,NULL);
	sigaction(SIGCHLD,&ochldact,NULL);
	sigprocmask(SIG_SETMASK,&savemask,NULL);

	return(status);
}	/* end - mkp_system() */
