static char sccsid[] = "@(#)81  1.7  src/bos/usr/lib/pios/piomkapqd.c, cmdpios, bos411, 9428A410j 5/26/94 12:48:44";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main, int init_odm, print_msg, getmsg,
 *            build_new_queues, build_existing_queues,
 *            add_additional, build_header, predef_select
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

/**** include files ****/
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/dir.h> 
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <locale.h>
#include <nl_types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/id.h>
#include "piobe_msg.h"
#include "smit_class.h"
#include "common.h"
#define MAXLINE     LINE_MAX
#include "qcadm.h"
#undef  MAXLINE

/********************* default messages ************************************/
#define MF_PIOBESETNO       (8)
#define SETBEGINCHR		'['
#define SETENDCHR		']'
#define MSGSEPCHR		';'
#define FLDSEPCHR		','
#define DEFMC_PREFIXPATH    "/usr/lib/lpd/pio/etc/"
#define DEFAULT_USAGE       "Usage:piomkapqd -A local -p Printer -d DeviceName -h header [-e]\n"\
	"\tpiomkapqd -A local -p Printer -v Device -s Subclass -r Adapter -h header [-e]\n"\
	"\tpiomkapqd -A xstation -p Printer -x XstationName -t XstationType -P Port -h header [-e]\n"\
	"\tpiomkapqd -A ascii -p Printer -T ttyName -h header [-e]\n"\
	"\tpiomkapqd -A file -p Printer -f FileName -h header [-e]\n"\
	"\tpiomkapqd -A Attachment -p Printer [ -d Devicename ] -c CmdExec -i DiscCmd\n"\
	"\t\t -o ObjectId  -h header [-e]\n"
#define DEFAULT_ARG         "%s: Missing argument for the flag -%c\n"
#define DEFAULT_ILLOPT      "%s: Illegal flag '-%c'\n"
#define DEF_SINGLE_QNAME	"Name of new PRINT QUEUE to add"
#define DEF_MC_SINGLE_QNAME	"piosmit.cat"
#define DEF_SN_SINGLE_QNAME	(1)
#define DEF_MN_SINGLE_QNAME	(112)
#define	DEFMSG_LVP_FOPENERR "%s: Error '%s' in opening the file %s\n"
#define DEFMSG_LVP_POPENERR "%s: Error '%s' in opening a pipe to command %s\n"
#define DEF_APQD_HEADER		"Add a Print Queue"
#define DEF_MC_APQD_HEADER	"smit.cat"
#define DEF_SN_APQD_HEADER	(18)
#define DEF_MN_APQD_HEADER	(104)
#define DEF_HEADER_PRINTER  "Add an Additional Printer to an Existing Print Queue"
#define DEF_MC_HEADER_PRINTER	"piosmit.cat"
#define DEF_SN_HEADER_PRINTER	(1)
#define DEF_MN_HEADER_PRINTER	(30)
#define DEF_EXISTING_NAME		"EXISTING print queues for this printer"
#define DEF_MC_EXISTING_NAME	"piosmit.cat"
#define DEF_SN_EXISTING_NAME	(1)
#define DEF_MN_EXISTING_NAME	(50)
#define DEF_APQD_ERRODMINIT "ODM initialization failed (odmerrno is %d).\n"\
	"Use local problem reporting procedures.\n"
#define DEF_APQD_ERRODMSP 	"Setting ODM path to %s failed (odmerrno is %d).\n"\
	"Use local problem reporting procedures.\n"
#define DEF_APQD_ERRODMLOCK	"Locking the ODM path %s failed (odmerrno is %d).\n"\
	"Use local problem reporting procedures.\n"
#define DEF_APQD_ERRODMOPEN "Opening the ODM class \"%s\" failed (odmerrno is %d).\n"\
"Use local problem reporting procedures.\n"
#define DEF_APQD_ERRODMADD 	"Error in adding an object to the ODM class '%s'\n"\
	"	(odmerrno is %d).\n"\
	"	Use local problem reporting procedures.\n"
#define DEF_LVP_DRDERR  	"%s: Error '%s' in reading the directory %s\n"
#define DEFMSG_LVP_MALLOCERR	"%s: Error '%s' in malloc()\n"
#define DEF_APQD_PREDEF  	"Predefined virtual printer does not exist for the \n"\
	"printer type specified with the '-d' command line option. \n"\
	"Use local problem reporting procedures.\n"
#define DEF_DEF_DS_STRING	"(user defined)"
#define CMD_HDR_STR			"sm_cmd_hdr"
#define CMD_OPT_STR			"sm_cmd_opt"

/************************ macros ******************************************/
#define DEFBASEDIR 		"/usr/lib/lpd/pio"      /* default base directory */
#define DEFVARDIR    	"/var/spool/lpd/pio/@local"    /* default base directory */
#define SMITMSGCAT      "piosmit.cat"
#define SMITDIR         "/smit"
#define PREDEFDIR		"/predef"
#define CUSDIR			"/custom"
#define WKBUFLEN        (1024)
#define HDR_ID			"ps_apqd_hdr"
#define Q_OPT_ID 		"ps_apqd_qopt"
#define DESC_ID			"ps_apqd_desc"
#define QNAMES_ID		"ps_apqd_qnames"
#define XNAMES_ID		"ps_apqd_existing"
#define PRINTER_ID		"ps_apqd_printer"
#define COMMON_PRT_ID	"ps_apqd_common_prt"
#define LOCAL_SERIAL	"ps_apqd_serial"
#define LOCAL_PARALLEL	"ps_apqd_parallel"
#define XSTATION_SERIAL	"ps_apqd_xstation"
#define ADDITONAL_ID	"ps_apqd_additional"
#define MODE_ID			"ps_apqd_mode"
#define SPACER_ID		"ps_apqd_spacer"

#define ODM_HDR_CRIT  	"id = 'ps_apqd_hdr'"
#define ODM_OPT_CRIT  	"id = 'ps_apqd_qopt'"
#define CMD_TO_EXEC		"/usr/lib/lpd/pio/etc/piomkpq"
#define LOCAL_DISC_CMD	"lsattr -c printer -D -O -s %s -t %s|{ read hdr;read data;ports=$(/usr/sbin/lsconn -p %s -k %s);if [[ $(print \"$ports\"|wc -l) -eq 1 ]];then hdr=$hdr:portno;data=$data:$ports;fi;print \"$hdr\n$data\";}"
#define LOCAL_ATTACH	"local"
#define XSTA_ATTACH		"xstation"
#define	FILE_ATTACH 	"file"
#define ASCII_ATTACH	"ascii"
#define PARALLEL_CLASS	"parallel"
#define RS232_CLASS		"rs232"
#define RS422_CLASS		"rs422"
#define XTA_PARALLEL_PORT	"p"
#define ODM_LOCKTIME	(5)
#define EXISTING_SEQ_NUM	(2000)
#define NEW_SEQ_NUM			(3000)
#define MA_ATTR			":mA:"
#define LSALLQ          "/usr/bin/lsallq"
#define LSALLQDEV       "/usr/bin/lsallqdev"
#define LSQUEDEV		"/usr/bin/lsquedev"
#define ADD_PRINT_QUEUE_HMI	"1800226"
#define ADD_PRINTER_QUEUE_HMI	"1810031"
#define NEW_QUEUE_ADD_HMI	"1810193"
#define NEW_QUEUES_ADD_HMI	"1810128"
#define EXISTING_QUEUES_HMI	"1810129"
#define EXISTING_QUEUE_HMI	"1810194"

/****  global variables  ***/
const char 				*progname;				/* used for error message */
static char             basedir[PATH_MAX+1];	/* printer backend base dir */
static char             vardir[PATH_MAX+1]; 	/* printer backend var dir */
CLASS_SYMBOL			cmd_hdr;				/* sm_cmd_hdr class */
CLASS_SYMBOL			cmd_opt;				/* sm_cmd_opt class */
char					*ptype;          		/* used by predef_select() */
char					option_id[WKBUFLEN];/* the option id for the sm_cmd_hdr */
char					cmd_exec[2*PATH_MAX+1]; /* options for the cmd_to_exec */
int						predef_num;			/* the # of predefs for printer type */
int						odmld;				/* lock descriptor for ODM */
int						add_printer_to_q = FALSE;
gid_t						real_gid;
gid_t						saved_gid;

typedef struct {
    char					*attach;
    char					*ptype;
    char					*dtype;
    char					*device;
    char					*subclass;
    char					*parent;
    char					*xstation;
    char					*xtype;
    char					*xport;
    char					*tty;
    char					*file;
	char					*exec_cmd;
	char					*disc_cmd;
	char					*prt_comm;
    char					*title;
} flag_args_t;

/**** function prototypes *****/
void init_odm(void);
static void print_msg(FILE *, int , const char *, ...);
static char const *getmsg(const char*, int, int );
void build_new_queues(void);
void build_existing_queues(char *);
void add_additional(void);
void build_header(flag_args_t *);
static int predef_select(struct dirent *);
extern int      scandir(char *, struct dirent *(*[]),
                int (*)(struct dirent *),
                int (*)(struct dirent **, struct dirent **));
                    /* should've been in <sys/dir.h> */
static void 	apqd_parsemsg(char *,char **,long *,long *,char **);



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           main                                                         *
*                                                                              *
* DESCRIPTION:    Perform main logic.                                          *
*                                                                              *
*                 Parse command line.                                          *
*                 Initialize the ODM.                                          *
*                 Call build_new_queues to build new queue options.            *
*                 Call build_existing_queues to build existing queue options.  *
*                 Call build_additional to build options if this is            *
*                    add a printer to an existin queue.                        *
*                 Call build_header to build the sm_cmd_hdr for the dialog.    *
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
int						dflag = FALSE;
flag_args_t				flag_args;

	/* Query real and saved group ids and set effective group to real id. */
	real_gid 	= getgidx(ID_REAL);
	saved_gid 	= getgidx(ID_SAVED);
	(void) setgidx(ID_EFFECTIVE,real_gid);

	(void) setlocale(LC_ALL, "");

	progname = *argv; 

	(void)memset((void *)&flag_args,0,sizeof(flag_args));

	if (argc == 1)  /* if no arguments then error */
	{
		print_msg(stderr,APQD_USAGE,DEFAULT_USAGE);
		exit(EXIT_FAILURE);
	}

	opterr = 0;
	while ((c = getopt(argc,argv,":A:p:v:d:s:r:x:t:P:T:f:c:i:o:h:e")) !=
	       EOF)
	{
		switch(c)
		{
			case 'A':   /* attachment type */
			 	flag_args.attach = optarg;	
				break;

			case 'p':  /* predef file type */
				flag_args.ptype = optarg;
				ptype = flag_args.ptype;  /* set global var */
				break;

			case 'v':  /* device type - for new devices */
				flag_args.dtype = optarg;
				break;

			case 'd':  /* existing device name */
				flag_args.device = optarg;
				dflag = TRUE;
				break;

			case 's':  /* device subclass - for new devices */
				flag_args.subclass = optarg;
				break;

			case 'r':  /* parent adapter - for new devices */
				flag_args.parent = optarg;
				break;

			case 'x':  /* xstation name */
				flag_args.xstation = optarg;
				break;

			case 't':  /* xstation type */
				flag_args.xtype = optarg;
				break;

			case 'P':  /* xstation port */
				flag_args.xport = optarg;
				break;

			case 'T':  /* tty name */
				flag_args.tty = optarg;
				break;

			case 'f':  /* file name */
				flag_args.file = optarg;
				break;

			case 'c':  /* cmd_to_exec */
				flag_args.exec_cmd = optarg;
				break;

			case 'i':  /* cmd_to_discover */
				flag_args.disc_cmd = optarg;
				break;

			case 'o':  /* communication parameters object name  */
				flag_args.prt_comm = optarg;
				break;

			case 'h':  /* dialog header (title) */
				flag_args.title = optarg;
				break;

			case 'e':  /* if this is add printer to existing queue */
				add_printer_to_q++;
				break;

			case ':':
				print_msg(stderr,MSG_LVP_MISSARG,DEFAULT_ARG,progname,
						(char)optopt);
				print_msg(stderr,APQD_USAGE,DEFAULT_USAGE);
				exit(EXIT_FAILURE);

			case '?':
				print_msg(stderr,MSG_LVP_ILLOPT,DEFAULT_ILLOPT,progname,
						(char)optopt);
                print_msg(stderr,APQD_USAGE,DEFAULT_USAGE);
				exit(EXIT_FAILURE);
		}
	}

	/* begin setting up the cmd_to_exec */
	if ((flag_args.exec_cmd != NULL) && (flag_args.exec_cmd[0] != '\0'))
	{
		(void)strncpy(cmd_exec,flag_args.exec_cmd,sizeof(cmd_exec));
		(void)strncat(cmd_exec," -p '",sizeof(cmd_exec)-strlen(cmd_exec)-1);
		(void)strncat(cmd_exec,flag_args.ptype,sizeof(cmd_exec)-strlen(cmd_exec)-2);
		(void)strncat(cmd_exec,"'",sizeof(cmd_exec)-strlen(cmd_exec)-1);
	}
	else
		(void)sprintf(cmd_exec,"%s -A '%s' -p '%s' ",CMD_TO_EXEC,flag_args.attach,
				flag_args.ptype);

    /* initialize the odm and cleanup any old objects */
    (void) init_odm();

    if (!add_printer_to_q) /* NOT add an additonal printer */
	{
		build_new_queues();
		if ((dflag == TRUE) && (flag_args.device[0] != '\0'))
			/* if the device already exists find existing Qs */
			build_existing_queues(flag_args.device);
	}
	else /* add an additonal printer to a queue */
		add_additional();

    build_header(&flag_args);

    /* clean up */
   (void) setgidx(ID_EFFECTIVE,saved_gid);
   (void)odm_close_class(cmd_hdr);
   (void)odm_close_class(cmd_opt);
   (void)odm_unlock(odmld);
   (void)odm_terminate();
   (void) setgidx(ID_EFFECTIVE,real_gid);

	return(0);

}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           init_odm                                                     *
*                                                                              *
* DESCRIPTION:    Initialize the ODM and clean up any old 'Add a Print         *
*                 Queue' dialogs that might be lying around.                   *
*                                                                              *
* PARAMETERS:     None                                                         *
*                                                                              *
* RETURN VALUES:  None                                                         *
*                                                                              *
*******************************************************************************/
void init_odm(void)
{
const char			     	*cp; 	 			/* tmp pointer */
char             			sdir[PATH_MAX+1];   /* SMIT ODM dir */

	/* Perform odm initialization with printq group authority. */
	(void) setgidx(ID_EFFECTIVE,saved_gid);

    /* initialize the odm */
	if (odm_initialize() == -1)
	{ 
		print_msg(stderr,MSG_APQD_ERRODMINIT,DEF_APQD_ERRODMINIT,odmerrno);
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
   (void)strncpy(sdir,vardir,sizeof(sdir));
   (void)strncat(sdir,SMITDIR,sizeof(sdir)-strlen(sdir)-1);

    /* set the odm path */
	if ((odm_set_path(sdir)) == (char *)-1)
	{ 
		print_msg(stderr,MSG_APQD_ERRODMSP,DEF_APQD_ERRODMSP,sdir,odmerrno);
		exit(EXIT_FAILURE);
	}

	if ((odmld = odm_lock(sdir,ODM_LOCKTIME)) == -1)
	{ 
		print_msg(stderr,MSG_APQD_ERRODMLOCK,DEF_APQD_ERRODMLOCK,sdir,odmerrno);
		exit(EXIT_FAILURE);
	} 

	if ((int)(cmd_hdr = odm_open_class(sm_cmd_hdr_CLASS)) == -1)
	{ 
		print_msg(stderr,MSG_APQD_ERRODMOPEN,DEF_APQD_ERRODMOPEN,CMD_HDR_STR,odmerrno);
		exit(EXIT_FAILURE);
	}

	if ((int)(cmd_opt = odm_open_class(sm_cmd_opt_CLASS)) == -1)
	{ 
		print_msg(stderr,MSG_APQD_ERRODMOPEN,DEF_APQD_ERRODMOPEN,CMD_OPT_STR,odmerrno);
		exit(EXIT_FAILURE);
	}

    /* remove old objects from ODM */
    (void)odm_rm_obj(cmd_hdr,ODM_HDR_CRIT);
    (void)odm_rm_obj(cmd_opt,ODM_OPT_CRIT);

	/* Restore the group to real group id. */
	(void) setgidx(ID_EFFECTIVE,real_gid);

	return;
}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           build_new_queues                                             *
*                                                                              *
* DESCRIPTION:    Build sm_cmd_opt objects for the SMIT prompts for the        *
*                 new queue names.                                             *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:   None.                                                       *
*                                                                              *
*                                                                              *
*******************************************************************************/
void build_new_queues(void)
{
char                    predefdir[PATH_MAX+1];   
char                    predefnm[PATH_MAX+1];
int                     num_entries;		
struct dirent           **dir_ptr;
int                     i;					/* counter */
struct dirent           **tmp_ptr;
struct sm_cmd_opt       option;				
char                    *mAptr;              /* pointer to the mA description  */
char                    *dsptr;              /* pointer to printer's dastastream */
char                    prefixbuf[WKBUFLEN+1];  /* work buffer */
char                    namebuf[WKBUFLEN+1];  /* work buffer for option name */
FILE                    *fp;
char                    line[LINE_MAX+1];    
int                     found;
char					*tmpptr;

    /* scan the predef directory for predef files of the selected printer type */
	(void)strcpy(predefdir,basedir);
	(void)strncat(predefdir,PREDEFDIR,sizeof(predefdir)-strlen(predefdir)-1);
	if ((num_entries = scandir(predefdir,&dir_ptr,predef_select,NULL)) == -1)
	{
		print_msg(stderr, MSG_LVP_DRDERR, DEF_LVP_DRDERR, progname,
				strerror(errno), predefdir);
		exit(EXIT_FAILURE);
	}

	if (num_entries == 0)
	{
		print_msg(stderr, MSG_APQD_PREDEF, DEF_APQD_PREDEF);
		exit(EXIT_FAILURE);
	}

    	/* loop once for each predef file */
	for (i = 0, tmp_ptr = dir_ptr; i < num_entries; i++, tmp_ptr++)
	{
		(void)memset((void *)&option,0,sizeof(option));
		(void)strcpy(option.id,Q_OPT_ID);
		(void)sprintf(option.id_seq_num,"%d",NEW_SEQ_NUM+i+1);

		predef_num = num_entries;  /* set global variable */

		if (num_entries > 1)
		{
			(void) strncpy(predefnm, predefdir, sizeof(predefnm)-2);
			*(predefnm+sizeof(predefnm)-2) = 0;
			(void) strncat(predefnm, "/",sizeof(predefnm)-strlen(predefnm)-1);
			(void) strncat(predefnm, (*tmp_ptr)->d_name,
					sizeof(predefnm)-strlen(predefnm)-1);
			*(predefnm+sizeof(predefnm)-1) = 0;
			if (!(fp = fopen(predefnm, "r")))
			{
				print_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
				strerror(errno), predefnm);
				exit(EXIT_FAILURE);
			}

			/* scan the predef file for mA */
			for (found = FALSE; !found  &&  fgets(line, sizeof(line)-1, fp); )
			{
				if ((mAptr = strstr(line,MA_ATTR)) != NULL)
				{
					found = TRUE;  /* we found the mA attribute */
					mAptr++;
					mAptr = strchr(mAptr,':'); 	  	/* advance 2 colons */
					mAptr++;   
					mAptr = strchr(mAptr,':');
					mAptr++;   					/* point to the value */
					tmpptr = strchr(mAptr,'\n');					
					*tmpptr = '\0';
				}
			}
			(void)fclose(fp);

			/* get the datastream extension of the predef file */
			dsptr = strrchr((*tmp_ptr)->d_name,'.'); dsptr++;

			if (found == TRUE)  /* if mA was found */
				(void)sprintf(namebuf,"   %s",mAptr);
			else
				(void)sprintf(namebuf,"   %s",dsptr);
			option.name = namebuf;   /* name is the value of mA */

			(void)strcpy(option.required,"n");

			/* set up the prefix field */
			(void)sprintf(prefixbuf,"-D %s -q ",dsptr);

			/* set up the help field */
			(void)strncpy(option.help_msg_id,NEW_QUEUES_ADD_HMI,
				      sizeof(option.help_msg_id)-1),
			*(option.help_msg_id+sizeof(option.help_msg_id)-1) = 0;
	 	}	
		else /* only one predef file */
		{
			option.name		= DEF_SINGLE_QNAME;
			option.name_msg_file	= DEF_MC_SINGLE_QNAME;
			option.name_msg_set	= DEF_SN_SINGLE_QNAME;
			option.name_msg_id	= DEF_MN_SINGLE_QNAME;
			(void)strcpy(option.required,"+");

			/* find the printers datastream from the predef extension */
			dsptr = strrchr((*tmp_ptr)->d_name,'.'); dsptr++;
			(void)sprintf(prefixbuf,"-D %s -q ",dsptr);  /* for the prefix field */

			/* set up the help field */
			(void)strncpy(option.help_msg_id,NEW_QUEUE_ADD_HMI,
				      sizeof(option.help_msg_id)-1),
			*(option.help_msg_id+sizeof(option.help_msg_id)-1) = 0;
		}

		(void)strcpy(option.op_type,"n");
		(void)strcpy(option.entry_type,"t");
		option.entry_size = 41;
		option.prefix = prefixbuf;

		(void) setgidx(ID_EFFECTIVE,saved_gid);
		if (odm_add_obj(cmd_opt,&option) == -1)
		{
		    print_msg(stderr,MSG_APQD_ERRODMADD,DEF_APQD_ERRODMADD,
					CMD_OPT_STR,odmerrno);
		    exit(EXIT_FAILURE);
		}
		(void) setgidx(ID_EFFECTIVE,real_gid);

	}

    /* set up the option_id string */
    if (num_entries > 1)  /* if there > 1 predef, include 'Names' descriptor */
		(void) sprintf(option_id,"%s,#%s%s#%s",QNAMES_ID,vardir,SMITDIR,Q_OPT_ID);
	else
		(void) sprintf(option_id,"#%s%s#%s",vardir,SMITDIR,Q_OPT_ID);

    /* free the memory allocated by scandir */
   for (i = num_entries,tmp_ptr = dir_ptr; i-- > 0; tmp_ptr++)
      free((void *)*tmp_ptr);
   free((void *)dir_ptr);

	return;

}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           predef_select                                                *
*                                                                              *
* DESCRIPTION:    Select predef files (printer_type.*)                         *
*                                                                              *
* PARAMETERS:     dp            directory entry                                *
*                                                                              *
* RETURN VALUES:  0             to be selected                                 *
*                 !0            not to be selected                             *
*                                                                              *
*******************************************************************************/
static int predef_select(struct dirent *dp)
{
	size_t				len;
	char				wkbuf[WKBUFLEN+1];  /* work buffer */

	(void)strcpy(wkbuf,ptype);
	(void)strncat(wkbuf,".",sizeof(wkbuf)-strlen(wkbuf)-1);
	return (strlen(dp->d_name) > (len = strlen(wkbuf))) ?
           !strncmp(dp->d_name, wkbuf, len) : FALSE;
}    


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           build_existing_queues                                        *
*                                                                              *
* DESCRIPTION:    Build sm_cmd_opt objects for the options on the              *
*                 add print queue dialog to display existing queues            *
*                 for an existing printer device.                              *
*                                                                              *
* PARAMETERS:     device      The printer device name.                         *
*                                                                              *
* RETURN VALUES:  none                                                         *
*                                                                              *
*                                                                              *
*******************************************************************************/
void build_existing_queues(char *device)
{

FILE			*fp;							/* the custom file pointer */
char            line[LINE_MAX+1]; 				/* line from custom file */
char			namebuf[WKBUFLEN+1];			/* work buffer for option names */
struct sm_cmd_opt		option;			
int				count = 0;
char			cusdir[PATH_MAX+1];
char			custom_nm[PATH_MAX+1];
int				found = FALSE;					/* set if mA is found in custom */
char			*ptr;							/* tmp pointer */
char			*nameptr;						/* pointer to option name */
struct stat		statbuf;						/* not used - required for stat */
int				qcfgfd;
struct flock			lck;
struct quelist			*qlistp	= NULL;
register struct quelist	*thisqd;
register char     *queue;
register const char     *qdevice;
int						type;
char					thisline[LINE_MAX];
char					tmpstr[LINE_MAX];
FILE					*qcfd;
char					*tmpptr;
char					*msgptr;


   /* Build a list of queues and their devices by parsing the /etc/qconfig */
   if ((qcfgfd = open(QCONFIG, O_RDONLY)) == -1)
   {
      print_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR, progname,
	      strerror(errno), QCONFIG);
      exit(EXIT_FAILURE);
   }
   lck.l_whence = lck.l_start = lck.l_len = 0;
   lck.l_type = F_RDLCK;
   (void) fcntl(qcfgfd, F_SETLKW, &lck);
   read_qconfig(&qlistp); /* call spooler routine to read qconfig */
   (void) close(qcfgfd);  /* close and unlock the file */
   if (!qlistp)
      return;

	/* open qconfig */
	qcfd = (FILE *)open_stream(QCONFIG,"r");

	thisqd = qlistp;  /* initialize pointer to beginning of list */
	while(readln(qcfd,thisline) != NOTOK)
	{
		type = parseline(thisline,tmpstr);
		if (type == TYPNAME)
		{
			queue = thisqd->qname;
			qdevice = thisqd->dname;
			thisqd = thisqd->next;
		}
		else if (type == TYPASSG)
		{
			if (strcmp(tmpstr,"file") == 0)
			{
				(void)strtok(thisline,"\n =");
				ptr = strtok(NULL,"\n =");
				if ((ptr = strrchr(ptr,'/')) != NULL)
						ptr++;

	            /* if this queue device contains 'file = /dev/device' */
				if (strcmp(ptr,device) == 0)   /* create an object for this queue */
				{
					/* initialize the sm_cmd_opt structure */
			        (void)memset((void *)&option,0,sizeof(option));
	
					(void)strcpy(option.id,Q_OPT_ID);
					(void)sprintf(option.id_seq_num,"%d",EXISTING_SEQ_NUM+count+1); 
					count++;
	
					/* if there is more than one predef for this printer type
					   then get mA */
					if (predef_num > 1) 
					{
			            /*  build the custom dir name */
						(void)strcpy(cusdir,vardir);
						(void)strncat(cusdir,CUSDIR,sizeof(cusdir)-strlen(cusdir)-1);
		
						/* build the full path to the custom file */
			            (void) strncpy(custom_nm, cusdir, sizeof(custom_nm)-2);
			            *(custom_nm+sizeof(custom_nm)-2) = 0;
			            (void) strncat(custom_nm, "/",sizeof(custom_nm)-
                                       strlen(custom_nm)-1);
						(void) strncat(custom_nm,queue,sizeof(custom_nm)-
                                       strlen(custom_nm)-1);
						(void) strncat(custom_nm,":",sizeof(custom_nm)-
                                       strlen(custom_nm)-1);
						(void) strncat(custom_nm,qdevice,sizeof(custom_nm)-
                                       strlen(custom_nm)-1);
			
						if (stat(custom_nm,&statbuf) == 0)  /* there is a custom file */
						{
							/* open the custom file */
				            if (!(fp = fopen(custom_nm, "r")))
				            {
				                print_msg(stderr, MSG_LVP_FOPENERR, DEFMSG_LVP_FOPENERR,
			                              progname, strerror(errno), custom_nm);
				                exit(EXIT_FAILURE);
				            }
				
				            /* scan the custom file for mA */
				            for (found = FALSE; !found  &&  fgets(line,
		                         sizeof(line)-1, fp); )
				            {
				                if ((nameptr = strstr(line,MA_ATTR)) != NULL)
				                {
				                    found = TRUE;  /* we found the mA attribute */
				                    nameptr++;
				                    nameptr = strchr(nameptr,':'); /* advance 2 colons */
				                    nameptr++;
				                    nameptr = strchr(nameptr,':');
				                    nameptr++;                   /* point to the value */
									tmpptr = strchr(nameptr,'\n');					
									*tmpptr = '\0';
									(void)sprintf(namebuf,"   %s",nameptr);
									option.name = namebuf;	
				                }
				            }
					    (void)fclose(fp);
							if (found == FALSE)  /* set name = (user defined) */
							{
								msgptr = (char *)getmsg(MF_PIOBE, MF_PIOBESETNO,
                                            DEF_DS_STRING);
                                if (!msgptr)
                                	msgptr = DEF_DEF_DS_STRING;

								(void)sprintf(namebuf,"   %s",msgptr);
								option.name = namebuf;
							}
						}
						else
						{	 /* set name = (user defined) */
							msgptr = (char *)getmsg(MF_PIOBE, MF_PIOBESETNO,
								DEF_DS_STRING);
							if (!msgptr)
								msgptr = DEF_DEF_DS_STRING;

							(void)sprintf(namebuf,"   %s",msgptr);
							option.name = namebuf;
						}		
	
						if (count == 1)  /* the 1st time set the option id */
						{
							(void) strncat(option_id,",",
									sizeof(option_id) - strlen(option_id)-1);
							(void) strncat(option_id,XNAMES_ID,
									sizeof(option_id) - strlen(option_id)-1);
						}

						/* set the help msg id */
      						(void)strncpy(option.help_msg_id,EXISTING_QUEUE_HMI,
		    						sizeof(option.help_msg_id)-1),
      						*(option.help_msg_id+sizeof(option.help_msg_id)-1) = 0;
					}
					else   /* there is only one predef/datastream */
					{
						if ( count == 1) /* only get descriptor for the 1st option */
						{
							option.name = DEF_EXISTING_NAME;
							option.name_msg_file	= DEF_MC_EXISTING_NAME;
							option.name_msg_set	= DEF_SN_EXISTING_NAME;
							option.name_msg_id	= DEF_MN_EXISTING_NAME;
	                       /* include dummy option to space things correctly */
							(void) strncat(option_id,",",
									sizeof(option_id) - strlen(option_id)-1);
							(void) strncat(option_id,SPACER_ID,
	                                       sizeof(option_id) - strlen(option_id)-1);
						}

						/* set the help msg id */
      						(void)strncpy(option.help_msg_id,EXISTING_QUEUES_HMI,
		    						sizeof(option.help_msg_id)-1),
      						*(option.help_msg_id+sizeof(option.help_msg_id)-1) = 0;
					}
	
					(void)strcpy(option.entry_type,"n");
					option.disp_values = queue;
	
					(void) setgidx(ID_EFFECTIVE,saved_gid);
					if (odm_add_obj(cmd_opt,&option) == -1)
					{
					    print_msg(stderr,MSG_APQD_ERRODMADD,DEF_APQD_ERRODMADD,
								CMD_OPT_STR,odmerrno);
					    exit(EXIT_FAILURE);
					}
					(void) setgidx(ID_EFFECTIVE,real_gid);
				}
			}
		}
	}

   /* close file descriptors */
	(void)fclose(qcfd);  /* close the qconfig */

	return;

}

/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           add_additional
*                                                                              *
* DESCRIPTION:    Generates sm_cmd_opt objects for the 'Add a printer to       *
*                 an existing queue' menu option.                              *
*                                                                              *
* PARAMETERS:     None.                                                        *
*                                                                              *
* RETURN VALUES:   None.                                                       *
*                                                                              *
*                                                                              *
*******************************************************************************/
void add_additional(void)
{
char                   	predefdir[PATH_MAX+1];   
int						num_entries;
struct dirent			**dir_ptr;
struct dirent			**tmp_ptr;
int						i;
char					*ptr;

  /* scan the predef directory for predef files of the selected printer type */
	(void)strcpy(predefdir,basedir);
	(void)strncat(predefdir,PREDEFDIR,sizeof(predefdir)-strlen(predefdir)-1);
	if ((num_entries = scandir(predefdir,&dir_ptr,predef_select,NULL)) == -1)
	{
		print_msg(stderr, MSG_LVP_DRDERR, DEF_LVP_DRDERR, progname,
				strerror(errno), predefdir);
		exit(EXIT_FAILURE);
	}

	if (num_entries == 0)
    {
		print_msg(stderr, MSG_APQD_PREDEF, DEF_APQD_PREDEF);
		exit(EXIT_FAILURE);
    }


	if (num_entries > 1)  /* there is > 1 datastream for this printer */
		(void) sprintf(option_id,"%s,%s",ADDITONAL_ID,MODE_ID);
	else  /* num_entries = 1 */
	{
		(void) sprintf(option_id,"%s",ADDITONAL_ID);
		ptr = strrchr((*dir_ptr)->d_name,'.'); ptr++;
		(void)strncat(cmd_exec," -D '",sizeof(cmd_exec)-strlen(cmd_exec)-1);
		(void)strncat(cmd_exec,ptr,sizeof(cmd_exec)-strlen(cmd_exec)-1);
		(void)strncat(cmd_exec,"'",sizeof(cmd_exec)-strlen(cmd_exec)-1);
	}

	/* free space allocated by scandir */
	for (i = num_entries,tmp_ptr = dir_ptr; i-- > 0; tmp_ptr++)
		free((void *)*tmp_ptr);
	free((void *)dir_ptr);

	return;
}


/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           build_header                                                 *
*                                                                              *
* DESCRIPTION:    Generates one sm_cmd_hdr object for the 'Add a print         *
*                 queue dialog'.                                               *
*                                                                              *
* PARAMETERS:     flag_args    - a structure containing the cmd line flags     *
*                                                                              *
* RETURN VALUES:  none                                                         *
*                                                                              *
*                                                                              *
*******************************************************************************/
void build_header(flag_args_t *flag_args)
{

struct sm_cmd_hdr 		header;

	/* null out header structure */
	(void)memset((void *)&header,0,sizeof(header));

	(void)strcpy(header.id,HDR_ID);

	(void)strncat(option_id,",",sizeof(option_id)-strlen(option_id)-1);
	(void)strncat(option_id,DESC_ID,sizeof(option_id)-strlen(option_id)-1);
	header.option_id = option_id;  

	(void)strcpy(header.has_name_select,"y");

	if (!flag_args->title || !*flag_args->title)	/* title not specfd */
	   if (add_printer_to_q == 0) /* add a print queue */
	   {
		header.name		= DEF_APQD_HEADER;
		header.name_msg_file	= DEF_MC_APQD_HEADER;
		header.name_msg_set	= DEF_SN_APQD_HEADER;
		header.name_msg_id	= DEF_MN_APQD_HEADER;
	   }
	   else	/* add additional printer to existing print queue */
	   {
		header.name		= DEF_HEADER_PRINTER;
		header.name_msg_file	= DEF_MC_HEADER_PRINTER;
		header.name_msg_set	= DEF_SN_HEADER_PRINTER;
		header.name_msg_id	= DEF_MN_HEADER_PRINTER;
	   }
	else	/* title specified */
	   apqd_parsemsg(flag_args->title,&header.name_msg_file,
			 &header.name_msg_set,&header.name_msg_id,&header.name);

	/* set the help msg id */
      	(void)strncpy(header.help_msg_id,add_printer_to_q?ADD_PRINTER_QUEUE_HMI:
		      ADD_PRINT_QUEUE_HMI,sizeof(header.help_msg_id)-1),
      	*(header.help_msg_id+sizeof(header.help_msg_id)-1) = 0;

	header.cmd_to_exec = cmd_exec;

	if (!strcmp(flag_args->attach,LOCAL_ATTACH))
	{
  			/* if the device exists */
		if ((flag_args->device != NULL) && (flag_args->device[0] != '\0'))
		{
			(void)strncat(header.cmd_to_exec," -d '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->device,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
			(void)strncat(header.option_id,PRINTER_ID,
                          WKBUFLEN-strlen(header.option_id)-1);
		}
		else /* device = "", so we want a new device */
		{

			(void)strncat(header.cmd_to_exec," -v '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->dtype,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.cmd_to_exec," -s '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->subclass,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.cmd_to_exec," -r '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->parent,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
			(void)strncat(header.option_id,COMMON_PRT_ID,
                          WKBUFLEN-strlen(header.option_id)-1); /* get common options */

			/* set the cmd_to_discover */
			if ((header.cmd_to_discover = malloc(WKBUFLEN)) == NULL)
			{
				print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR, progname, 
							strerror(errno));
				exit(EXIT_FAILURE);
			}
			(void)sprintf(header.cmd_to_discover,LOCAL_DISC_CMD,
                    flag_args->subclass,flag_args->dtype,flag_args->parent,
		    flag_args->subclass);

			/* pick up the correct set of communication parameters */
			if (!strcmp(flag_args->subclass,PARALLEL_CLASS))
			{
				(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
				(void)strncat(header.option_id,LOCAL_PARALLEL,
                          WKBUFLEN-strlen(header.option_id)-1);
			}
			else if (!strcmp(flag_args->subclass,RS232_CLASS) ||
			         !strcmp(flag_args->subclass,RS422_CLASS))
			{
				(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
				(void)strncat(header.option_id,LOCAL_SERIAL,
                          WKBUFLEN-strlen(header.option_id)-1);
			}

		}
	}
	else if (!strcmp(flag_args->attach,XSTA_ATTACH))
	{
		if ((flag_args->device != NULL) && (flag_args->device[0] != '\0'))
		{
			(void)strncat(header.cmd_to_exec," -d '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->device,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
			(void)strncat(header.option_id,PRINTER_ID,
                          WKBUFLEN-strlen(header.option_id)-1);
		}
		else /* device = "", so we want a new device */
		{
			(void)strncat(header.cmd_to_exec," -x '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->xstation,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.cmd_to_exec," -t '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->xtype,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.cmd_to_exec," -P '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->xport,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			/* if the port type is NOT parallel */
			if (strcmp(flag_args->xport,XTA_PARALLEL_PORT))
			{
				(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
				(void)strncat(header.option_id,XSTATION_SERIAL,
                          WKBUFLEN-strlen(header.option_id)-1);
			}
		}
	}
	else if (!strcmp(flag_args->attach,ASCII_ATTACH))
	{
			(void)strncat(header.cmd_to_exec," -d '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->tty,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
			(void)strncat(header.option_id,PRINTER_ID,
                          WKBUFLEN-strlen(header.option_id)-1);
	}
	else if (!strcmp(flag_args->attach,FILE_ATTACH))
	{
			(void)strncat(header.cmd_to_exec," -d '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->file,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
			(void)strncat(header.option_id,PRINTER_ID,
                          WKBUFLEN-strlen(header.option_id)-1);
	}
	else  /* attachment is of an unknown type */
	{
		/* if a device was specified */
		if ((flag_args->device != NULL) && (flag_args->device[0] != '\0'))
		{
			(void)strncat(header.cmd_to_exec," -d '",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,flag_args->device,
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));
			(void)strncat(header.cmd_to_exec,"'",
                          (2*PATH_MAX)-strlen(header.cmd_to_exec));

			(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
			(void)strncat(header.option_id,PRINTER_ID,
                          WKBUFLEN-strlen(header.option_id)-1);
		}
		else  /* we want a new device */
		{
			/* set the cmd_to_discover if -i was passed */
			if ((flag_args->disc_cmd != NULL) && (flag_args->disc_cmd[0] != '\0'))
			{
				if ((header.cmd_to_discover = malloc(WKBUFLEN)) == NULL)
				{
					print_msg(stderr, MSG_LVP_MALLOCERR, DEFMSG_LVP_MALLOCERR, progname, 
								strerror(errno));
					exit(EXIT_FAILURE);
				}
				(void)strncpy(header.cmd_to_discover,flag_args->disc_cmd, WKBUFLEN-1);
			}
	
			/* set option id for communication parameters if -o was passed */
			if ((flag_args->prt_comm != NULL) && (flag_args->prt_comm[0] != '\0'))
			{
					(void)strncat(header.option_id,",",
                          WKBUFLEN-strlen(header.option_id)-1);
					(void)strncat(header.option_id,flag_args->prt_comm,
                          WKBUFLEN-strlen(header.option_id)-1);
			}

		}
	}


    /* add the cmd_hdr object to the ODM */
	(void) setgidx(ID_EFFECTIVE,saved_gid);
	if (odm_add_obj(cmd_hdr,&header) == -1)
	{
	    print_msg(stderr,MSG_APQD_ERRODMADD,DEF_APQD_ERRODMADD,
				CMD_HDR_STR,odmerrno);
	    exit(EXIT_FAILURE);
	}
	(void) setgidx(ID_EFFECTIVE,real_gid);

    /* free malloc'd memory */
	if (header.cmd_to_discover != NULL)
		free(header.cmd_to_discover);
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
* NAME:           apqd_parsemsg                                                *
*                                                                              *
* DESCRIPTION:    Parse a given message and fetch it, if necessary.            *
*                                                                              *
* PARAMETERS:     mp		msg template ptr [mc,sn,mn;dt]                 *
*                                                                              *
* RETURN VALUES:  mcnm 		msg catalog name                               *
*		  sno		set no                                         *
*		  mno		msg no                                         *
*		  dfmsg		default msg                                    *
*                                                                              *
*******************************************************************************/
static void
apqd_parsemsg(char *mp,char **mcnm,long *sno,long *mno,char **dfmsg)
{
   register char		*cp;
   register char		*sp;
   char				*tp;
   char				*tmp;

   /* Get default message text. */
   if (!(*dfmsg = mp) || !*mp || *mp != SETBEGINCHR ||
       *(cp = mp+strlen(mp)-1) != SETENDCHR ||
       !(sp = strchr(tmp = mp+1,MSGSEPCHR)) ||
       (*dfmsg = sp+1, *cp = 0, sp == tmp))
      return;

   do
   {
      /* Get msg no. */
      for (--sp; *sp != FLDSEPCHR; )
	 if (sp == tmp)
	    break;
	 else
	    sp--;
      if ((*mno = strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,(char **)&tp,10),
	   *tp != MSGSEPCHR) || sp == tmp)
	 break;

      /* Get set no. */
      for (--sp; *sp != FLDSEPCHR; )
	 if (sp == tmp)
	    break;
	 else
	    sp--;
      if ((*sno = strtol(tp = *sp == FLDSEPCHR ? sp+1 : sp,(char **)&tp,10),
	   *tp != FLDSEPCHR) || sp == tmp)
	 break;

      /* Get message catalog name. */
      if (tp = malloc((size_t)((sp-tmp)+1)))
         (void)memcpy((void *)tp,(void *)tmp,(size_t)(sp-tmp)),
         *(tp+(sp-tmp)) = 0,
	 *mcnm = tp;
   } while (0);

   return;
}	/* end - apqd_parsemsg() */


/**************************************************************************
 	The following code defines functions and variables that are
	used in in read_qconfig() function.  This is a kludge to get
	'qccom.c' file to compile and link properly.
***************************************************************************/
#undef va_start
#undef va_end
#undef va_arg
#include <varargs.h>	/* for functions that are used in read_qconfig() */

int			sysmsg(char *);
int			syserr();
int			syswarn();
int			systell();
int			sysraw();
void			*Qalloc(size_t);

/*==== Send a message to the correct place */
sysmsg(char *message /* the message to print */)
{
	/*----Combine the error message and the perror messages */
	if (errno)
        	fprintf(stderr, "%s%s: errno = %d: %s\n",message,progname,
			errno,strerror(errno));
	else
		fprintf(stderr, "%s\n", message);

	/*----Reset errno and return */
	errno = 0;
	return(0);
}


/*==== Fatal error message, and die                                          */
/*     Call:  syserr(<exitcode>, <message_format>, <thing1>, <thing2>, ...); */
syserr(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
	char	message[256];
        char    *fmt;
	int	exitcode;

        va_start(args);
	exitcode = va_arg(args,int);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n",progname,buf);
        va_end(args);
	sysmsg(message);
	exit(exitcode);
}


/*==== Warning error message                                      */
/*     Call:  syswarn(<message_format>, <thing1>, <thing2>, ...); */
syswarn(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
	char	message[256];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n", progname, buf);
        va_end(args);
	sysmsg(message);
	return(0);
}



/*==== Information message                                        */
/*     Call:  systell(<message_format>, <thing1>, <thing2>, ...); */
systell(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
	char	message[256];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
	sprintf(message,"%s: %s\n", progname, buf);
        va_end(args);
	sysmsg(message);
	return(0);
}



/*==== Raw, unformatted information message (for debugging statements) */
/*     Call:  sysraw(<message_format>, <thing1>, <thing2>, ...);       */
/*     NOTE: no line feed added                                        */
sysraw(va_alist)
va_dcl
{
        va_list args;
        char    buf[256];
        char    *fmt;

        va_start(args);
        fmt = va_arg(args,char *);
        vsprintf(buf,fmt,args);
        va_end(args);
	errno = 0;
	sysmsg(buf);
	return(0);
}



void *Qalloc(size_t size)
{       
	register void *ans;

	ans = malloc(size);
	bzero(ans,size);
	return(ans);
}


