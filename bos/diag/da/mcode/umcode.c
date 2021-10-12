static char sccsid[] = "@(#)55  1.10.2.15  src/bos/diag/da/mcode/umcode.c, damcode, bos41B, 9505A 1/12/95 18:09:32";
/*
 *   COMPONENT_NAME: DAMCODE
 *
 *   FUNCTIONS: build_disk_mcode1493
 *              build_tape_mcode1572
 *              chk
 *              chk_software_error1701
 *              cleanup
 *              config_initial_state1801
 *              copy_text
 *              dasd_drive
 *              device_find
 *              disp_menu
 *              do_scsi
 *              download
 *              get_ascsi_mcode
 *              get_hex_value
 *              get_umcode
 *              hardware_error
 *              hx_to_a
 *              initialize_all
 *              load_diskette
 *              main
 *              mcode_ascsi_name2168
 *              mcode_compatible
 *              mcode_unuseable
 *              tape_get_ready
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <memory.h>
#include <nl_types.h>

#include <sys/buf.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/devinfo.h>
#include <sys/errno.h>
#include <sys/scdisk.h>
#include <sys/scsi.h>
#include <sys/tape.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <diag/diago.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <diag/da.h>
#include <diag/diag_define.h>
#include <diag/diag_exit.h>
#include <diag/diagodm.h>
#include <diag/diag.h>
#include <diag/scsd.h>



#include "umcode_msg.h"

#define TU_READY 0
#define REQUEST_SENSE 0x03
#define START_UNIT 0X1b
#define WRITE_BUFFER 0X3b
#define INQUIRY 0x12
#define DL_LATEST_LEVEL 1
#define DL_PREV_LEVEL 2
#define SLASH_DEV       "/dev/%s"
#define BOOT_MCODE      1

#define SE      5
#define DE      6

int tape_last_buffer=FALSE;
int before_9_92=FALSE;
int odm_flg=-1;
int asl_flg=FALSE;
int testing_tape=0;
int *dev_index;           /* The index into the CuDv objects which are      */
                          /* drives that support download .                 */
int downloading = 0;      /* Indicates download in progress.                */
int exitt = 0;            /* Indicates the Exit key was pressed.            */
int fdes = -1;            /* File discriptor.                               */
int level = 0;            /* Indicates level of mcode to download.          */
int num_of_devices=0;     /* The number of drives that can be download      */
int model_level_start=0;
int rc1=0;
int state1=0;
int state2=0;
int step = -1;            /* Used to indicate a point reached in the        */
                          /*   execution of this code.                      */
short   allow_download=0;
int	size_need=0;

int load_from_diskette=0;
int dev_led=0;
char dname[] = "/dev/";   /* Holds the name of the device to open.          */

char *devname;
char *mcode_data;         /* Pointer to the microcode buffer (32k).         */
int loadid_byte=8;        /* Starting byte of load id in VPD data           */

unsigned char buf_id;            /* Indicates which 32k segment of microcode.*/
                                 /*  is being downloaded(0 - filesize/32k).  */
unsigned char evpd;              /* EVPD value of INQUIRY command.           */
unsigned char inquiry_dat[159];  /* Holds Inquiry (VPD) data.                */
unsigned char mcode_name[128];   /* mcode file name read from diskette.      */
unsigned char pg_code;           /* Page code value of INQUIRY command.      */
unsigned char vpd_len;           /* Indicates amount of expected VPD data.   */

struct CuDv *cudv;
struct listinfo objinfo;
struct mcode
        {
                int     blk_fraction;  /* Remainder of the file (< 32k). */
                int     num_blks;      /* Number of 32k blocks.          */
                long    size;
                long    strip;
        };

nl_catd catd;   /* Catalog file discriptor.         */
FILE *fptr;     /* The microcode diskette file ptr. */


/* Global variables for command time outs */
int		wbuff_to = 180;		/* write buffer comamnd time out      */
int		start_stop_to = 180;	/* start/stop comamnd time out        */

/* Function Declarations */
void    dasd_drive();
void    cleanup();
int     device_find();
int     disp_menu();
int     down_load();
int     do_scsi();
int     get_umcode();
int     mcode_compatible();
void 	tape_get_ready();
long    chk();
char*   hx_to_a();
extern char *diag_cat_gets(nl_catd, int, int);
extern  nl_catd diag_catopen(char *, int);

struct stat dbstat;
int debug = 0;
FILE    *fdebug;

/*
 * NAME: main
 *
 * FUNCTION: This procedure is the entry point for the Micro_code
 *      download Service Aid. This service aid is designed to be used
 *      as a way to correct an error in the drive's microcode.
 *      The microcode is loaded from media and then written to a hard
 *      file where it is saved to be used by the file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *       This routine must execute in the environment set up for diagnostics.
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller after presenting screens to the user
 *      informing them of the error.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOOTES.
 *
 * RETURNS: NONE
 */
main()
{
        int count;
        int rc;
        int bc;
        int     platform=0;
        int mcode_type=0;
        char    ascsi_name[255];
	struct CuAt cuat, *cuat_ptr;
	char criteria[64];
	uchar  operation_flags;
	struct  disk_diag_scsd_cuat *disk_scsd_ptr;

	if ((stat("/tmp/.DIAG_DAMCODE_TRACE",&dbstat)) != -1) {
		debug = 1;
        	fdebug=fopen("/tmp/damcode.trace","w");
	}
        setlocale(LC_ALL,"");

        /* initialize odm, initialize asl and open catalog file */
        initialize_all();

        /* Find all applicable device types in the system */
        count = device_find();

        while(1)                /* Loops until error or Cancel key pressed */
        {
        	if (debug) {
        	fprintf(fdebug,"************** Umcode start **************\n");
        	fflush (fdebug);
        	}

                testing_tape=FALSE;
                /* Display the drive selection screen */
                rc1 = disp_menu(0x802090,SELECT_DEVICE);
                if(strncmp(cudv [ dev_index[rc1]].name,"ascsi",5) != 0)
                {
                        if(!strncmp(cudv [ dev_index[rc1]].name,"rmt",3))
                        {
                                dev_led=(int)cudv[dev_index[rc1]].PdDvLn->led;
				if(dev_led == 0) {
					/* No LED number in PdDv so check for type Z */
					/* attribute                                 */
		

					if ((cuat_ptr = (struct CuAt *)getattr(cudv[dev_index[rc1]].name,
									       "led",0,&bc)) 
					    == (struct CuAt *)NULL) {

						/* Error from CuAt */
						rc = disp_menu(0x802092,ERR_ODM);
						cleanup();
					}	
					dev_led = (int)strtoul(cuat_ptr->value,NULL,0);					
				} 
				
                                testing_tape=TRUE;
                        }

                        if (debug) {
                        fprintf(fdebug,"%d: Device selected to test is %s\n",__LINE__,
                                cudv[dev_index[rc1]].name);
                        fflush(fdebug);
                        }

                        config_initial_state((cudv + dev_index[rc1])->parent,
                                (cudv + dev_index[rc1])->name, NULL);

                        /* Form the /dev name for the selected drive or tape */
                        devname = (char *) calloc(strlen((cudv + dev_index[rc1])->name)
                                + strlen(SLASH_DEV) ,sizeof(char));
                        sprintf(devname,SLASH_DEV,(cudv + dev_index[rc1])->name);
			

			
			
			if (strcmp((cudv + dev_index[rc1])->PdDvLn->type,
				    "scsd")) {
				
				/*
				  This is not an SCSD device
				  */
				
				/*
				  get information from the odm data base
				  */
				rc=get_diag_att( (cudv + dev_index[rc1])->PdDvLn->type,
						"download_ucode",'h', &bc, &allow_download);
				
			}		


                        /* Determine value of loadid_byte                    */

                        rc=get_diag_att( (cudv + dev_index[rc1])->PdDvLn->type,
                                 "loadid_byte", 'i', &bc, &loadid_byte);

			if (rc == -1 ){
                        	rc=get_diag_att( "deflt_diag_att",
                                      "loadid_byte", 'i', &bc, &loadid_byte);
			        if (rc == -1)
					loadid_byte=8;
			}

                        if (debug) {
                        fprintf(fdebug,"%d: devname = %s byte_count = %d,",
                                        __LINE__, devname,bc);
                        fprintf(fdebug," loadid_byte = %d, get_diag_att rc = %d\n"
                                         ,loadid_byte,rc);
                        fflush(fdebug);
                        }
                        /* Display the microcode level selection screen */
                        level = disp_menu(0x802190,SELECT_LEVEL);

                        /* Perform the micro-code download */
                        rc = get_umcode();

                        if(rc == 0) /* Display task completed */
                                rc = disp_menu(0x802099,N_FINISHED);

                        /* Return drive and adapter back to original state. */
                        if(fdes >-1)
                        {
                                if(strlen(mcode_name)&& load_from_diskette)
                                        unlink(mcode_name);

                                if (debug) {
                                fprintf(fdebug,"%d: fdes= %d\n",__LINE__,fdes);
                                fflush(fdebug);
                                }

                                close(fdes);
                                fdes = -1;
                        }
                        config_initial_state((cudv + dev_index[rc1])->parent,
                                (cudv + dev_index[rc1])->name,TRUE);

                }
                else
                {
                        mcode_type=disp_menu(0x802890,NULL);
                        if( mcode_type==BOOT_MCODE &&
                                (rc=disp_menu(0x802891,NULL))==2)
                        {
                                rc=disp_menu(0x802892,NULL);
                                continue;
                        }
                        /* Display the microcode level selection screen */
                        level = disp_menu(0x802190,SELECT_LEVEL);

                        /* get the device microcode name from the VPD   */
                        (void)memset(ascsi_name,0,sizeof(ascsi_name));
                        (void)mcode_ascsi_name(cudv[dev_index[rc1]].name,
                        cudv[dev_index[rc1]].PdDvLn->type,mcode_type,
                                &ascsi_name,&platform);
                        rc = get_ascsi_mcode(ascsi_name,platform,
                                        cudv[dev_index[rc1]].name);
                        if(rc == 2) /* Display task completed */
                                rc = disp_menu(0x802099,N_FINISHED);

                }



        }/*end while*/

        cleanup();

} /* end main() */

/*
 * NAME: device_find()
 *
 * FUNCTION: This procedure is used to find all drive that support the download
 *      installed in the system. A list of the device names and locations
 *      is prepanum_of_drives so that a selection menu can be displayed.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller.
 *
 * RETURNS: NONE
 */

device_find()
{
        int count;
        int rc, bc;
        char criteria[80];
	uchar operation_flags;

        sprintf(criteria, "parent LIKE 'scsi*' or name like ascsi*");

        criteria[0]='\0';
        cudv = get_CuDv_list(CuDv_CLASS,criteria,&objinfo,16,2);
        if (cudv == (struct CuDv *)-1)
        {
                /* Error from CuDv */
                rc = disp_menu(0x802092,ERR_ODM);
                cleanup();
        }

        dev_index=calloc(objinfo.num+1,sizeof(int));
        for (count = 0; count <= (objinfo.num -1); count++)
        {
                allow_download=0;

		if (!strcmp((cudv + count)->PdDvLn->type,"scsd")) {

			/* This is an SCSD device */

			rc = diag_get_scsd_opflag((cudv + count)->name, 
						  &operation_flags);
			
			if (rc == -1)
				return (-1);
			
			if (operation_flags & SCSD_UCODE_DWNLD_FLG) {
				allow_download = 1;
			}
			else {
				allow_download = 0;
			}			

		}
		else {
			rc=get_diag_att( (cudv + count)->PdDvLn->type, "download_ucode",
                                'h', &bc, &allow_download);
		}
                if(( allow_download > 0) && ((cudv + count)->chgstatus
                                        != MISSING) )
                {
                        if (debug) {
                        fprintf(fdebug,"%d: Device allowed to download %s\n",
                        __LINE__,(cudv+count)->name);
                        fflush(fdebug);
                        }

                        num_of_devices++;
                        dev_index[num_of_devices] = count;
                }
        }
        if (num_of_devices == 0 || cudv == (struct CuDv *) NULL)
        {

                if (debug) {
                fprintf(fdebug,"%d: No SCSI device allowed to download/n",
                        __LINE__);
                fflush(fdebug);
                }

                /* No SCSI device found */
                rc = disp_menu(0x802091,ERR_NO_DEVICES);
                cleanup();
        }

        return (num_of_devices);

} /* end device_find() */

/*
 * NAME: get_umcode()
 *
 * FUNCTION: This procedure is responsible for reading the microcode
 *      diskette, temporarily copying it to a hardfile or RAM disk.
 *      When the data has been read all the MIF records are stripped
 *      away to get to the microcode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (RECOVERY OPERATION:) If the diskette can not be read or any other
 *      errors occur the user is displayed an error message prior
 *      to returning to the Diagnostic controller.
 *
 * RETURNS: NONE
 */

int get_umcode()
{
        char    *a;
        unsigned long   level1=0;
        long    int     level2=0;


        int curr_mcode_found = 0;
        int env_flg = 77;
        int i = 0;
        int numfiles = 0;
        int rc;
        char command[128];
        char curr_level[64];
        char prev_level[64];
        char latest_level[64];
        char fname[128];
        char tempstr[128];
        unsigned char curr_mcode_name[128];
        struct mcode mdata;
        struct sc_iocmd iocmd;
        struct stat file_status;
        FILE    *popen();
	struct  disk_scsd_inqry_data *scsd_vpd;
        char    ucode_size[10];

        (void)bzero(&iocmd,sizeof(iocmd));

        /* Open the special file associated with the selected drive */

        fdes = openx(devname, O_RDWR, NULL, SC_DIAGNOSTIC);
        rc = errno;

        if (debug) {
        fprintf(fdebug,"%d: Openx returned (fdes=%d), errno= %d\n",
                __LINE__,fdes,rc);
        fflush(fdebug);

        }

        if (fdes < 0)   /* Open should normally succeed */
        {
                ipl_mode(&env_flg); /*Determine if we are executing diskette */
                                    /* or fixed disk diagnostic package */
                if (rc == EACCES && env_flg == 0)
                        disp_menu(0x802093,ERR_DEVICE_BUSY);
                else
                        hardware_error();
                cleanup();
        }

	if (!strcmp((cudv + dev_index[rc1])->PdDvLn_Lvalue,"disk/scsi/scsd")) {
		/*
		  This is an SCSD disk
		  */
		
		
		/* 
		  Get main SCSD information
		  for this device. 
		  */
		
		evpd = 1;
		pg_code = 0xc7;
		vpd_len = 159;

		/* Do Inquiry command */

		(void)do_scsi(fdes, INQUIRY, &iocmd, inquiry_dat);

		scsd_vpd = (struct disk_scsd_inqry_data *) inquiry_dat;
						
		/*
		  Set SCSD time out values
		*/
		if (scsd_vpd->wbuff_timeout)
			wbuff_to = scsd_vpd->wbuff_timeout;
		if (scsd_vpd->start_timeout)
			start_stop_to = scsd_vpd->start_timeout;


		/* This device supports microcode download, so      */
		/* let's determine the maximum size of each write   */
		/* buffer 					    */
		
		sprintf(ucode_size,"0x%02x%02x%02x",
			scsd_vpd->ucode_size[0],
			scsd_vpd->ucode_size[1],
			scsd_vpd->ucode_size[2]);
		
                
		ucode_size[8] = '\0';
		
		size_need = strtoul(ucode_size,NULL,16);


	}
        if(testing_tape)
        {
                model_level_start = 33;
                if (debug) {
                fprintf(fdebug,"%d: Testing a tape drive\n",__LINE__);
                fflush(fdebug);
                }

                /* Make the tape ready to accept download microcode */
                tape_get_ready();
                build_tape_mcode(curr_mcode_name,curr_level);
        }
        else
        {
                model_level_start = 32;
                if (debug) {
                fprintf(fdebug,"%d: Testing a disk drive\n",__LINE__);
                fflush(fdebug);
                }

                /* Check whether drive is ready to accept download command */
                dasd_drive();
                build_disk_mcode(curr_mcode_name,curr_level);
        }

        load_diskette();

        /* Initialize prev_level */ 
        strcpy(prev_level,"INIT");
        /* Init latest level to the current level */
        strcpy(latest_level,curr_level);

        if(!load_from_diskette )
        {
		sprintf(command, "%s%s", FIND, 
			" /etc/microcode/* -print 2>/dev/null");
                if((fptr = popen(command, "r") ) == NULL)
                {
                        if (debug) {
                        fprintf(fdebug,"errno = %d\n",errno);
                        fflush(fdebug);
                        }
                        mcode_unuseable();
                }

                if (debug) {
                fprintf(fdebug,"Loading MICROCODE from bootable media.\n");
                fflush(fdebug);
                }
        }
        else
        {
                /* Get a list (toc) of the mcode files on the diskette */
		sprintf(command, "%s%s" , RESTBYNAME,
			" -TqSf/dev/rfd0 2>/dev/null");
                fptr = popen(command, "r");

                if (debug) {
                fprintf(fdebug,"Loading MICROCODE from diskette media.\n");
                fflush(fdebug);
                }
        }

        while(fgets(fname,80,fptr) != NULL)
        {
                /* Get rid of the newline character */
                fname[strlen(fname) - 1] = '\0';
                /* Check to see if the mcode files are compatible */
                if((rc = mcode_compatible(curr_mcode_name,fname)) != -1)
                {
                        if (rc == 0)
                                curr_mcode_found = 1;
                        strcpy(tempstr,&fname[model_level_start]);
                        if ( (rc > 0) && (level == DL_LATEST_LEVEL) )
                        {       /* Later version */
                                if (debug) {
                                fprintf(fdebug,"%d: tempstr = %s,",
                                __LINE__,tempstr);
                                fprintf(fdebug," latest_level =%s\n",
                                latest_level);
                                fflush(fdebug);
                                }

                                if (strcmp(tempstr , latest_level) > 0)
				{
                                        ++numfiles;
                                        strcpy(latest_level,tempstr);
                                }
                        }
                        if ( (rc < 0) && (level == DL_PREV_LEVEL) )
			{       /* Earlier version */
                                if (debug) {
                                fprintf(fdebug,"%d: tempstr = %s,",
                                        __LINE__,tempstr);
                                fprintf(fdebug," prev_level =%s\n",
                                        prev_level);
                                fflush(fdebug);
                                }

                                if ((!strcmp(prev_level,"INIT")) ||
				    (strcmp(tempstr , latest_level) > 0))
				{
                                        ++numfiles;
                                        strcpy(prev_level,tempstr);
                                }
                        }
                }
        }
        pclose(fptr);

        if(numfiles <=0)
                mcode_unuseable();

        if (!curr_mcode_found)
        {       /* Tell them present mcode level not on diskette */
                rc = disp_menu(0x802191,NO_CURR_LEVEL);
                if (rc ==2)
                        return(-1); /* Do not continue with download */
                rc = disp_menu(0x802096,STANDBY_READ);
        }

        if ( ( (level == DL_LATEST_LEVEL)  &&
               (strcmp(latest_level,curr_level) == 0) ) ||
             ( (level == DL_PREV_LEVEL) &&
               (strcmp(prev_level,curr_level) == 0) ) )

        {
                        /* Drive at current level */
                        disp_menu(0x802093,ERR_AT_LEVEL);
                        return(-1);
        }

        /* Build the name of the mcode file to download */
        strncpy(mcode_name,curr_mcode_name,model_level_start);
        mcode_name[model_level_start] = '\0';

        if (level == DL_LATEST_LEVEL)
                strcat(mcode_name,latest_level);
        else
                strcat(mcode_name,prev_level);

        if (debug) {
        fprintf(fdebug,"download file name is %s\n", mcode_name);
        fflush(fdebug);
        } 

        /* cd to the root dir */
        rc = system("cd /");
        if (rc != 0)
        {
                rc = disp_menu(0x802093,ERR_SW);
                cleanup();
        }

        /* Get the microcode file off of the diskette */
        sprintf(command,"%s -xqSf/dev/rfd0 %s 1>/dev/null 2>&1",
                RESTBYNAME, mcode_name);
        if(load_from_diskette)
                rc = system(command);
        if (rc != 0)
        {
                rc = disp_menu(0x802093,ERR_READING_FILE);
                cleanup();
        }

        rc = stat(mcode_name, &file_status);
        if (rc == -1)
                mcode_unuseable();

        /* Open the microcode file */
        if((fptr = fopen(mcode_name, "r")) == NULL)
        {
                rc = disp_menu(0x802093,ERR_READING_FILE);
                cleanup();
        }

	if (size_need == 0) {
		/* size_need has not yet been set. So decide whether */
                /* 32k blocks or 64k blocks are downloaded.          */
                /* allow_download value should be 1 for 32k and 2 for*/
		/* 64k  					     */

		size_need = allow_download * 0x8000;
	}

        /* Allocate storage for the microcode file*/
        mcode_data = (char *) malloc((size_need) * sizeof(char));
        if(mcode_data == (char *) NULL)
                mcode_unuseable();

        /* Get the mcode file size */
        mdata.size = file_status.st_size;
        if (mdata.size < size_need)
                mcode_unuseable();

        /* Determine number of 32k or 64k blocks */
        mdata.num_blks = mdata.size/size_need;
        mdata.blk_fraction = mdata.size % size_need;
        if (mdata.blk_fraction > 0)
                ++mdata.num_blks;

        /* Perform the microcode download */
        rc = download(&mdata);
        close(fptr);

        free(mcode_data);
        if (rc == -1)
                disp_menu(0x802093,ERR_AT_LEVEL);
        if (rc == -2)
                disp_menu(0x802093,N_INCOMPAT);

        return(rc);

} /* end get_umcode() */

/*
 * NAME: mcode_compatible()
 *
 * FUNCTION: This procedure is responsible for determining if the
 *           microcode files passed are of the same type.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (RECOVERY OPERATION:) None
 *
 * RETURNS: -1 if not same type,
 *           0 if the same type and model level,
 *          -2 if the same type but earlier model level,
 *           1 if the same type but later model level.
 */

int mcode_compatible(c_mcode_name,f_name)
        char c_mcode_name[];
        char f_name[];
{
        int i;
        int rc;
        char current_name[128];
        char new_name[128];
	char **ep;

        /* if (debug) {
        fprintf(fdebug,"%d: c_mcode_name = %s, f_name = %s\n",
                __LINE__, c_mcode_name,f_name);
        fflush(fdebug);
        } */

        /* First check for exact match */
        if (strcmp(c_mcode_name,f_name) == 0)
                return(0);

        /* Set up local copies of the file names */
        strcpy(current_name,c_mcode_name);
        strcpy(new_name,f_name);

        /* Clear out the model level field */
        for (i = model_level_start;i < model_level_start+8;i++)
        {
                current_name[i] = '0';
                new_name[i] = '0';
        }
        current_name[i] = '\0';
        new_name[i] = '\0';

        /* Check if the files are the same type */
        if (strcmp(current_name,new_name) != 0)
                return(-1);

        rc = strcmp(f_name,c_mcode_name);

        if (rc > 0)
                rc = 1;
        else
                rc = -2;

        if (debug) {
        fprintf(fdebug,"rc = %d, f_name=%s  \n c_mcode_name=%s\n", 
		rc, f_name, c_mcode_name);
        fflush(fdebug);
        }

        return(rc);

} /* end mcode_compatible() */

/**********************************************************
*  NAME:  char *hx_to_a()
*
*
*  FUNCTION:  Converts one hex byte to ascii (two bytes)
*
*
*  DATA STRUCTURES:
*
*
*  RETURNS:  pointer to two bytes that are the ascii equivalent
*            of the hex value of the byte passed in (hex_byte).
*
*
***********************************************************/

char *hx_to_a(hex_byte)
unsigned char hex_byte;
{
        char *ret_ptr;
        char tbuff[3];

        /* Look at first four bits */
        if ( ((hex_byte >> 4) & 0x0F) <= 9 )
                tbuff[0] = ((hex_byte >> 4) & 0x0F) + 48;
        else
                tbuff[0] = ((hex_byte >> 4) & 0x0F) + 55;

        /* Look at next four bits */
        if ( (hex_byte & 0x0F) <= 9 )
                tbuff[1] = (hex_byte & 0x0F) + 48;
        else
                tbuff[1] = (hex_byte & 0x0F) + 55;

        /* add a srting terminator */
        tbuff[2] = '\0';

        ret_ptr = tbuff;

        if (debug) {
        fprintf(fdebug,"%d: hx_to_a returning %s\n",__LINE__,ret_ptr);
        fflush(fdebug);
        }

        return(ret_ptr);

} /* end hx_to_a */

/*
 * NAME: dasd_drive()
 *
 * FUNCTION: This procedure is responsible for determining if the
 *      drive is ready to be  downloaded to. If the drive motor is
 *      not spinning it will be spun up.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (RECOVERY OPERATION:) The only error recovery from any failed SCSI
 *      commands is when the drive motor is not running. If any other
 *      error occur a message is displayed and execution is passed to D.C..
 *
 * RETURNS: NONE
 */

void dasd_drive()
{
        int rc;
        unsigned char sense_dat[255];
        struct sc_iocmd iocmd;

        (void)bzero(&iocmd,sizeof(iocmd));

        /* Send A TU Ready command to the drive*/
        rc = do_scsi(fdes, TU_READY, &iocmd, NULL);
        if (rc != SC_GOOD_STATUS )
        {

                if (rc != SC_CHECK_CONDITION)
                        hardware_error();
                else            /* May indicate drive's motor is not started*/
                {               /*  or a UNIT ATTENTION                    */
                                /* Set up to do Request Sense command */

                        rc = do_scsi(fdes, REQUEST_SENSE, &iocmd, sense_dat);

                        if (rc != SC_GOOD_STATUS) /* Can't handle this */
                                hardware_error();
                        if (sense_dat[2] ==0x06) /* UNIT ATTENTION */
                        {

                                rc = do_scsi(fdes,TU_READY,&iocmd,NULL);
                                if (rc != SC_GOOD_STATUS)
                                {
                                        rc = do_scsi(fdes,REQUEST_SENSE,
                                                        &iocmd,sense_dat);
                                        if(rc != SC_GOOD_STATUS)
                                                hardware_error();
                                }
                        }
                        if((sense_dat[2] == 0x02 && sense_dat[12] == 0x4) &&
                                              (sense_dat[13] <= 0x2))
                        {
                                /*Send a Start Unit command */
                                rc = disp_menu(0x802098,MOTORUP);
                                rc = do_scsi(fdes, START_UNIT, &iocmd,NULL);
                                sleep(10);
                                if (rc != SC_GOOD_STATUS)
                                        hardware_error();
                                /* Send a TU Ready cmd */
                                rc = do_scsi(fdes, TU_READY, &iocmd, NULL);
                                if (rc != SC_GOOD_STATUS)
                                        hardware_error();
                        }
                        else
                        {
                                if(sense_dat[2] !=0)
                                        hardware_error();
                        }
                }
        }
} /* end dasd_drive() */

/*
 * NAME: download()
 *
 * FUNCTION: This procedure is responsible for reading the microcode
 *      file that will be downloaded, querying the drives VPD to find
 *      the microcode download ID and finally write the microcode to
 *      the selected drive.
 * Note: The minimum size of the microcode file must be 32k + strip else
 *       this routine will fail.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (RECOVERY OPERATION:) The only error recovery from any failed SCSI
 *      commands is when the drive motor is not running. If any other
 *      error occur a message is displayed and execution is passed to D.C..
 *
 * RETURNS: NONE
 */

download(mcode_da)
        struct mcode *mcode_da;
{
        int     count;
        int     offset;
        int     rc;
        long    itemsize;
        long    int     buffer_count=0;
        unsigned char sense_dat[255];
        struct  sc_iocmd iocmd;

        (void)bzero(&iocmd,sizeof(iocmd));

        tape_last_buffer=FALSE;
        rc = disp_menu(0x802097,STANDBY_DL);
        for (count = 1; count <= mcode_da->num_blks ; count++)
        {
                if (count == mcode_da->num_blks && mcode_da->blk_fraction > 0)
                        itemsize = mcode_da->blk_fraction;
                else
                {
                        if (dev_led == 0x915)
                        {
                                itemsize = mcode_da->size;
                                count = mcode_da->num_blks;
                                mcode_data = (char*)malloc((itemsize)*sizeof(char));
                        }
                        else
                                itemsize = size_need;
                }
                if (count ==  mcode_da->num_blks)
                        tape_last_buffer=TRUE;
                if (testing_tape)
                {
                        if (debug) {
                        fprintf(fdebug,"%d: buffer offset [3] = %x, ",
                                __LINE__,buffer_count >> 16);
                        fprintf(fdebug,"buffer offset [4] = %x, ",
                                (buffer_count >> 8 )& 0x0ff);
                        fprintf(fdebug,"buffer offset [5] = %x\n",
                                buffer_count & 0x0ff);
                        fflush(fdebug);
                        }
                        if(dev_led == 0x915)
                        {
                                iocmd.scsi_cdb[3] = 0x00; /* Buffer offset */
                                iocmd.scsi_cdb[4] = 0x00; /* Buffer offset */
                                iocmd.scsi_cdb[5] = 0x00; /* Buffer offset */
                        }
                        else
                        {
                                iocmd.scsi_cdb[3] = buffer_count>>16;
                                iocmd.scsi_cdb[4] = (buffer_count>>8)&0xff;
                                iocmd.scsi_cdb[5] = buffer_count&0xff;
                        }

                        iocmd.scsi_cdb[6] = itemsize>>16;
                        iocmd.scsi_cdb[7] = (itemsize>>8)&0xff;
                        iocmd.scsi_cdb[8] = itemsize & 0x0ff;
                        iocmd.data_length = itemsize;
                        buffer_count+=itemsize;
                        if ((count == mcode_da->num_blks || before_9_92)  &&
                            dev_led != 0x915)
                                iocmd.scsi_cdb[9] = 0x0;
                        else
                                iocmd.scsi_cdb[9] = 0x80;
                }
                /* Read the microcode file */
                rc = fread(mcode_data, itemsize,1, fptr);
                if (rc <= 0)
                        mcode_unuseable();

                /* Do Write Buffer and Save command */
                buf_id = count-1;
                rc = do_scsi(fdes, WRITE_BUFFER, &iocmd, (mcode_data));
                if (rc != SC_GOOD_STATUS)
                {
                        if(rc == SC_CHECK_CONDITION) {
                                rc = do_scsi(fdes,REQUEST_SENSE,
                                             &iocmd,sense_dat);
                                rc = disp_menu(0x802093,ERR_CHECKSUM);
                        } else {
                                disp_menu(0x802093,ERR_HW);
			}
                        /* Forced open to recover from error */
                        if (fdes >= 0)
                        {
                                close(fdes);
                                fdes=-1;
                        }
                        fdes = openx(devname,O_RDWR,NULL,SC_FORCED_OPEN);
                        if (debug) {
                        fprintf(fdebug,"openx with force, fdes = %d\n",fdes);
                        fflush (fdebug);
                        }
                        cleanup();
                }

        }
        return(0);
} /* end download() */

/* NAME: do_scsi()
 *
 * FUNCTION: Executes a given SCSI command.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: A value is returned which indicates the pass/fail status of
 *              the SCSI command.
 */

do_scsi(fdes, scsi_cmdx, iocmd_blk, buffert)
int     fdes;
int     scsi_cmdx;
struct  sc_iocmd *iocmd_blk;
char    *buffert;
{
        int     rc;
        int     i=0;

        if (debug) {
        fprintf(fdebug,"%d: In do_scsi(), scsi_cmdx = 0x%x\n",
		__LINE__,scsi_cmdx);
        fflush(fdebug);
        }

        iocmd_blk->timeout_value = 180;
        switch (scsi_cmdx)
        {
                case    TU_READY:
                        /* Set up to send TU Ready cmd. */
                        iocmd_blk->command_length = 6;
                        iocmd_blk->scsi_cdb[0] = 0x00;/*TU Ready opcode */
                        iocmd_blk->scsi_cdb[1] = 0x00;
                        iocmd_blk->scsi_cdb[2] = 0x00;
                        iocmd_blk->scsi_cdb[3] = 0x0;
                        iocmd_blk->scsi_cdb[4] = 0x0;
                        iocmd_blk->scsi_cdb[5] = 0x0;
                        iocmd_blk->data_length = 0x0;
                        iocmd_blk->buffer = buffert;
                        iocmd_blk->flags = B_READ;
                        /* Send TU Ready command */
                        break;
                case    REQUEST_SENSE:
                        iocmd_blk->command_length = 6;
                        iocmd_blk->scsi_cdb[0] = 0x03;/*Request sense opcode */
                        iocmd_blk->scsi_cdb[1] = 0x00;
                        iocmd_blk->scsi_cdb[2] = 0x00;
                        iocmd_blk->scsi_cdb[3] = 0x0;
                        iocmd_blk->scsi_cdb[4] = 0xff; /* Expected sense data*/
                        iocmd_blk->scsi_cdb[5] = 0x0;
                        iocmd_blk->data_length = 0xff;
                        iocmd_blk->buffer = buffert;
                        iocmd_blk->flags = B_READ;
                        /* Send the Request Sense cmd. */
                        break;
                case    START_UNIT:
			iocmd_blk->timeout_value = start_stop_to;
                        iocmd_blk->command_length = 6;
                        iocmd_blk->scsi_cdb[0] = 0x1b; /*Start Unit opcode*/
                        iocmd_blk->scsi_cdb[1] = 0x00;
                        iocmd_blk->scsi_cdb[2] = 0x00;
                        iocmd_blk->scsi_cdb[3] = 0x0;
                        iocmd_blk->scsi_cdb[4] = 0x1; /* Start motor */
                        iocmd_blk->scsi_cdb[5] = 0x0;
                        iocmd_blk->data_length = 0x0;
                        iocmd_blk->buffer = buffert;
                        iocmd_blk->flags = B_READ;
                        /* Send the Start Unit cmd. */
                        break;
                case    INQUIRY:
                        iocmd_blk->command_length = 6;
                        iocmd_blk->scsi_cdb[0] = 0x12; /*Inquiry opcode */
                        if(before_9_92)
                        {
                                iocmd_blk->scsi_cdb[1] = 0; /* Logical Unit */
                                                            /* NUmber */
                                iocmd_blk->scsi_cdb[2] =  evpd;
                                iocmd_blk->scsi_cdb[3] = pg_code; /* Page code */
                                iocmd_blk->scsi_cdb[4] = vpd_len; /* Expected amount*/
                                                                  /* of VPD data */
                                iocmd_blk->scsi_cdb[5] = 0x0;
                        }
                        else
                        {
                                iocmd_blk->scsi_cdb[1] = evpd; /* EVPD bit  */
                                iocmd_blk->scsi_cdb[2] = pg_code; /* Page code */
                                iocmd_blk->scsi_cdb[3] = 0x0;
                                iocmd_blk->scsi_cdb[4] = vpd_len; /* Expected amount*/
                                                                  /* of VPD data */
                                iocmd_blk->scsi_cdb[5] = 0x0;
                        }
                        iocmd_blk->scsi_cdb[6] = 0x0;
                        iocmd_blk->scsi_cdb[7] = 0x0;
                        iocmd_blk->scsi_cdb[8] = 0x0;
                        iocmd_blk->scsi_cdb[9] = 0x0;
                        iocmd_blk->data_length = vpd_len;
                        iocmd_blk->buffer = buffert;
                        iocmd_blk->flags = B_READ;
                        /* Send the Inquiry command. */
                        break;
                case    WRITE_BUFFER:
			iocmd_blk->timeout_value = wbuff_to;
                        iocmd_blk->command_length = 0x0a;
                        iocmd_blk->scsi_cdb[0] = 0x3b; /*Write Buffer opcode*/
                        if(before_9_92 && testing_tape && !tape_last_buffer)

                                iocmd_blk->scsi_cdb[1] = 0x04; /* Mode 100 */
                        else
                                iocmd_blk->scsi_cdb[1] = 0x05; /* Mode 101 */
                        if(dev_led == 0x915)
                                iocmd_blk->scsi_cdb[2] = 0x20;
                        else
                                iocmd_blk->scsi_cdb[2] = buf_id;

                        if(!testing_tape )
                        {

                                iocmd_blk->scsi_cdb[3] = 0x0;
                                iocmd_blk->scsi_cdb[4] = 0x0;
                                iocmd_blk->scsi_cdb[5] = 0x0;
                                if (allow_download) {

                                        iocmd_blk->scsi_cdb[6] = 
						(size_need >> 16) & 0xff;
                                        iocmd_blk->scsi_cdb[7] = 
						(size_need >> 8 ) & 0xff;
                                        iocmd_blk->scsi_cdb[8] = 
						size_need & 0xff;
                                        iocmd_blk->data_length = size_need;

                                }
                                iocmd_blk->scsi_cdb[9] = 0x0;
                        }
                        iocmd_blk->buffer = buffert;
                        iocmd_blk->flags = B_WRITE;
                        /* Send the Write Buffer command. */
                        break;
        }

        if (debug) {
        fprintf(fdebug,"%d: iocmd_blk-> command_length = %x\n",__LINE__,
                iocmd_blk->command_length);
        for (i=0;i<10;i++)
                fprintf(fdebug,"%d: iocmd_blk-> scsi_cdb[%d] = %x \n",
                __LINE__,i,iocmd_blk->scsi_cdb[i]);
        fprintf(fdebug,"%d: iocmd_blk-> data_length = %x\n",__LINE__,
                iocmd_blk->data_length);

        fprintf(fdebug,"%d: iocmd_blk-> flags = %x\n",__LINE__,
                iocmd_blk->flags);
        fflush(fdebug);
        }

        if(testing_tape)
                rc = ioctl(fdes, STIOCMD, iocmd_blk);
        else
                rc = ioctl(fdes, DKIOCMD, iocmd_blk);

        if (debug) {
	fprintf(fdebug,"%d: ioctl returned %d\n" , __LINE__,rc);
	fprintf(fdebug,"%d: iocmd_blk->status_validity %d\n" ,
		__LINE__,iocmd_blk->status_validity);
        fprintf(fdebug,"%d: iocmd_blk-> adapter_status = %x\n",__LINE__,
		iocmd_blk->adapter_status);
        fprintf(fdebug,"%d: iocmd_blk-> scsi_bus_status = %x\n",__LINE__,
		iocmd_blk->scsi_bus_status);
        fflush(fdebug);
	}

        if (scsi_cmdx== REQUEST_SENSE) {
                buffert[2] = buffert[2] & 0xF ;
        	if (debug) {
		fprintf(fdebug,"sense key = %x\n", buffert[2]);
		fprintf(fdebug,"ASC = %x, ASCQ = %x\n",
			buffert[12], buffert[13]);
        	fflush(fdebug);
		}
	}
        if (scsi_cmdx == INQUIRY && rc != SC_GOOD_STATUS)
                hardware_error();
        if (rc == -1)
        {
                if (iocmd_blk->status_validity & 0x1)
                {
                        if (iocmd_blk->scsi_bus_status & SC_CHECK_CONDITION) {
                                return(SC_CHECK_CONDITION);
                        } else {
                                return(rc);
			}
                }
        }
        return(rc);

} /* end do_scsi() */

/*
 * NAME: disp_menu()
 *
 * FUNCTION: This procedure is used to perform the user interface. All
 *      prompts, selections and error messages are performed here.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOOTES.
 *
 * RETURNS: menutypes.cur_index (The selection made by the user)
 */

int
disp_menu(menu_num,msg)
        long menu_num;
        int msg;
{
        struct  msglist remove_tape[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        REMOVE_TAPE},
                                        { 0,            0}
                                      };

        struct  msglist tape_in_drive[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        TAPE_IN_DRIVE},
                                        { 0,            0}
                                      };

        struct  msglist mcode_on_diskette[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        YES_OPTION},
                                        { MCODE,        NO_OPTION},
                                        { MCODE,        MICRO_DISKETTE},
                                        { 0,            0}
                                      };
        struct  msglist mcode_type[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        MCODE_BOOT},
                                        { MCODE,        MCODE_BLOCK},
                                        { MCODE,        Select},
                                        { 0,            0}
                                      };

        struct  msglist mcode_jumper[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        YES_OPTION},
                                        { MCODE,        NO_OPTION},
                                        { MCODE,        MCODE_JUMPER},
                                        { 0,            0}
                                      };

        struct  msglist mcode_jumper_req[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        MCODE_JUMPER_REQ},
                                        { 0,            0}
                                      };
        struct  msglist mcode_nodos[]={
                                        { MCODE,        TITLE   },
                                        { MCODE,        NO_DOSDIR},
                                        { 0,            0}
                                      };
        int count;
        int rc;
        int device_type=0;
        char string[1024];
        char    *devname;
        ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
        ASL_SCR_INFO *uinfo;

        switch (menu_num)
        {
        case 0x802890:
                rc=chk(diag_display(menu_num,catd,mcode_type,DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
                rc = DIAG_ITEM_SELECTED(menutypes);
                break;
        case 0x802891:
                rc=chk(diag_display(menu_num,catd,mcode_jumper,DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
                rc = DIAG_ITEM_SELECTED(menutypes);
                break;

        case    0x802892:
                rc=chk(diag_display(menu_num,catd,mcode_jumper_req,DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC,&menutypes,NULL));
                rc=DIAG_ITEM_SELECTED(menutypes);
                break;

        case    0x802893:
                rc=chk(diag_display(menu_num,catd,mcode_nodos,DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC,&menutypes,NULL));
                rc=DIAG_ITEM_SELECTED(menutypes);
                break;

        case 0x802090:
                /* Device selection screen */
                count = num_of_devices + 2;
                uinfo = (ASL_SCR_INFO *)calloc(count,sizeof(ASL_SCR_INFO));
                /* First line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,SELECT_DEVICE));
                uinfo[0].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[0].text,string);
                /* Next line(s) */
                for (count = 1; count <= num_of_devices; count++)
                {
                        devname = get_dev_desc(cudv[dev_index[count]].name);
                        (void)memset(string,0,sizeof(string));
                        sprintf(string, diag_cat_gets(catd,MCODE,DEVICE),
                                (cudv +dev_index[count])->location);
                        copy_text(strlen(string),&string[strlen(string)],devname);
                        uinfo[count].text = (char *)malloc(strlen(string)+1);
                        strcpy(uinfo[count].text,string);
                }
                /* Last line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,Select));
                uinfo[count].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[count].text,string);

                menutypes.max_index = count;
                menutypes.cur_index = 1;
                rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
                                      ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                      &menutypes,uinfo));
                rc = DIAG_ITEM_SELECTED(menutypes);
                break;
        case 0x802190:
                /* Mcode level selection screen*/
                uinfo = (ASL_SCR_INFO *)calloc(4,sizeof(ASL_SCR_INFO));
                /* First line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,SELECT_LEVEL));
                uinfo[0].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[0].text,string);
                /* Next line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,LEVEL1));
                uinfo[1].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[1].text,string);
                /* Next line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,LEVEL2));
                uinfo[2].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[2].text,string);
                /* Last line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,Select));
                uinfo[3].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[3].text,string);

                menutypes.max_index = 3;
                menutypes.cur_index = 1;
                rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
                                      ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                      &menutypes,uinfo));
                rc = DIAG_ITEM_SELECTED(menutypes);
                break;
        case 0x802191:
                /* Error 10, current level not present */
                uinfo = (ASL_SCR_INFO *)calloc(4,sizeof(ASL_SCR_INFO));
                /* First line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,NO_CURR_LEVEL));
                uinfo[0].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[0].text,string);
                /* Next line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,YES_OPTION));
                uinfo[1].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[1].text,string);
                /* Next line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,NO_OPTION));
                uinfo[2].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[2].text,string);
                /* Last line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,Select));
                uinfo[3].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[3].text,string);

                menutypes.max_index = 3;
                menutypes.cur_index = 1;
                rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
                                      ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                      &menutypes,uinfo));
                rc = DIAG_ITEM_SELECTED(menutypes);
                break;
        case 0x802091:
        case 0x802092:
        case 0x802093:
        case 0x802094:
        case 0x802099:
        case 0x802894:
        case 0x802895:
        case 0x802896:
                uinfo = (ASL_SCR_INFO *)calloc(2,sizeof(ASL_SCR_INFO));
                /* First line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,TITLE));
                uinfo[0].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[0].text,string);
                /* Last line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,msg));
                uinfo[1].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[1].text,string);

                menutypes.max_index = 1;
                rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
                                      ASL_DIAG_KEYS_ENTER_SC,
                                      &menutypes,uinfo));
                break;
        case 0x802096:
        case 0x802097:
        case 0x802098:
                uinfo = (ASL_SCR_INFO *)calloc(2,sizeof(ASL_SCR_INFO));
                /* First line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,TITLE));
                uinfo[0].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[0].text,string);
                /* Last line */
                sprintf(string,
                          diag_cat_gets(catd,MCODE,msg));
                uinfo[1].text = (char *)malloc(strlen(string)+1);
                strcpy(uinfo[1].text,string);

                menutypes.max_index = 1;
                rc = chk(diag_display(menu_num,catd,NULL,DIAG_IO,
                                      ASL_DIAG_OUTPUT_LEAVE_SC,
                                      &menutypes,uinfo));
                break;
        case 0x802390:
                rc=chk(diag_display(menu_num,catd,remove_tape,DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));
                break;

        case 0x802391:
                rc=chk(diag_display(menu_num,catd,tape_in_drive,DIAG_IO,
                        ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));
                cleanup();
                break;
        case 0x802399:
                rc=chk(diag_display(menu_num,catd,mcode_on_diskette,DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
                rc = DIAG_ITEM_SELECTED(menutypes);
                break;
        default:
                break;
        }


        free(uinfo);
        return(rc);

} /* end disp_menu() */

/* NAME: chk()
 *
 * FUNCTION: Designed to check ASL return code and take an appropriate action.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

long chk(asl_stat)
        long asl_stat;
{
        switch(asl_stat)
        {
                case DIAG_ASL_OK:
                        break;
                case DIAG_MALLOCFAILED:
                case DIAG_ASL_ERR_NO_SUCH_TERM:
                case DIAG_ASL_ERR_NO_TERM:
                case DIAG_ASL_ERR_INITSCR:
                case DIAG_ASL_ERR_SCREEN_SIZE:
                case DIAG_ASL_FAIL:
                case DIAG_ASL_EXIT:
                case DIAG_ASL_CANCEL:
                        cleanup();
                        break;
                default:
                        break;
        }
}  /* end chk() */

/* NAME: cleanup()
 *
 * FUNCTION: Closes all open files, terminates ASL and ODM before
 * returning to the Diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: The an indication of what point in the code execution had
 *              reached. The Diagnostic controller does not use this
 *              returned value.
 */

void cleanup()
{

        if(odm_flg==0)
        {
                term_dgodm();
                odm_flg=-1;
        }
        if (fdes != -1)
        {
                close(fdes);
                fdes=-1;
        }

        if (catd != CATD_ERR)
        {
                catclose( catd );
                catd=CATD_ERR;
        }
        if ( asl_flg)
        {
                diag_asl_quit(NULL);
                asl_flg=FALSE;
        }

        config_initial_state((cudv + dev_index[rc1])->parent,
                (cudv + dev_index[rc1])->name,TRUE);
        if(strlen(mcode_name)&& load_from_diskette)
                unlink(mcode_name);

        if (debug) {
        fflush(fdebug);
        fclose(fdebug);
        }

        exit(0);

} /* end clean_up() */

/* NAME: build_disk_mcode()
 *
 * FUNCTION: Buid the microcode file name from the current vpd.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: The an indication of what point in the code execution had
 *              reached. The Diagnostic controller does not use this
 *              returned value.
 */


build_disk_mcode(name,level)
unsigned        char    *name;
char    *level;
{

        struct sc_iocmd iocmd;
        int     i=0;

        (void)bzero(&iocmd,sizeof(iocmd));

        /* Get drive's Product Type and Model Number */
        evpd = 0;
        pg_code = 0;
        vpd_len = 159;

        /* Do Inquiry command */

        (void) do_scsi(fdes, INQUIRY, &iocmd, inquiry_dat);

        /* Build the current mcode file name. */

        /* Microcode path name */
        strcpy(name,"/etc/microcode/");

        /* Product Type (bytes 16 - 19, ASCII) */
        strncat(name,(inquiry_dat + 16),4);

        /* Model Number (bytes 20 - 22, ASCII) */
        strncat(name,(inquiry_dat + 20),3);

        strcat(name,".");

        /* Get drive's Model Level and Load ID */

        evpd = 1;
        pg_code = 3;
        vpd_len = 159;

        /* Do Inquiry command */

        (void)do_scsi(fdes, INQUIRY, &iocmd, inquiry_dat);

        /* Load ID (bytes 4 - 7, HEX) or (bytes 8 - 11) */
        for (i=loadid_byte;i<=(loadid_byte+3);i++)
        {
                strcat(name,(char *)hx_to_a(*(inquiry_dat + i)));
        }
        strcat(name,".");

        /* Model Level (bytes 8 - 11, HEX) or (byte 12 - 15) */
        level[0] = '\0';
        for (i=(loadid_byte+4);i<=(loadid_byte+7);i++)
        {
                strcat(level,(char *)hx_to_a(*(inquiry_dat + i)));
        }
        strcat(name,level);

        if (debug) {
        fprintf(fdebug,"current file name is %s\n", name);
        fflush(fdebug);
        }
}

/* NAME: build_tape_mcode()
 *
 * FUNCTION: Buid the microcode file name from the current vpd.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: The an indication of what point in the code execution had
 *              reached. The Diagnostic controller does not use this
 *              returned value.
 */


build_tape_mcode(name,level)
unsigned        char    name[];
char    level[];
{
        struct sc_iocmd iocmd;
        int     i=0;

        (void)bzero(&iocmd,sizeof(iocmd));

        /* Get drive's Product Type and Model Number */
        evpd = 0;
        pg_code = 0;
        vpd_len = 159;

        /* Do Inquiry command */

        (void) do_scsi(fdes, INQUIRY, &iocmd, inquiry_dat);

        before_9_92=FALSE;
        /* Microcode path name */
        strcpy(name,"/etc/microcode/");

        if(dev_led == 0x995 && inquiry_dat[16]==' ')
        {
                        /* Tape 4100's */
                /* Product Type (bytes 17 - 19, ASCII) */
                strncat(name,(inquiry_dat + 17),3);
                strcat(name,"-");
                /* Model Number (bytes 21 - 24, ASCII) */
                strncat(name,(inquiry_dat + 21),4);
                before_9_92=TRUE;
        }
        else
        {
                /* Product Type (bytes 16 - 19, ASCII) */
                strncat(name,(inquiry_dat + 16),4);

                /* Model Number (bytes 20 - 23, ASCII) */
                strncat(name,(inquiry_dat + 20),4);
        }

        strcat(name,".");
        if(dev_led == 0x995 && inquiry_dat[16]==' ')
        {
                /* Get drive's Model Level and Load ID */

                evpd = 0x80;
                pg_code = 3;
                vpd_len = 159;

                /* Do Inquiry command */

                (void)do_scsi(fdes, INQUIRY, &iocmd, inquiry_dat);

                /* Load ID (bytes 4 - 7, HEX) or (bytes 8 - 7) */
                for (i=loadid_byte;i<=(loadid_byte+3);i++)
                {
                        strcat(name,(char *)hx_to_a(*(inquiry_dat + i)));
                }
                strcat(name,".");

                /* Model Level (bytes 8 - 11, HEX) or (byte 12 - 15) */
                level[0] = '\0';
                strncat(level,inquiry_dat+loadid_byte+4,4 );
                strcat(name,level);
        }
        else
        {
                /* Load ID (bytes 44 - 47, HEX) */

                for (i=loadid_byte;i<=(loadid_byte+3);i++)
                        strcat(name,(char *)hx_to_a(*(inquiry_dat + i)));
                strcat(name,".");

                /* Revision Level (bytes 32 - 35, ASCII)  */

                (void)memset(level,0,sizeof(level));
                strncat(level,inquiry_dat+32,4);
                if(dev_led == 0x915 && level[1] == '.')
                {
                        level[1] = level[0];
                        level[0] = '0';
                }
                strcat(name,level);
        }

        if (debug) {
        fprintf(fdebug,"current file name is %s\n", name);
        fflush(fdebug);
        }
}

/*
 * NAME: initialize_all()
 *
 * FUNCTION: Designed to initialize the odm, get TM input and to
 *           initialize ASL.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

initialize_all()
{

        odm_flg=init_dgodm() ;                 /* Initialize odm.        */
        if(odm_flg<0)
                cleanup();

        chk( diag_asl_init("NO_TYPE_AHEAD") );/* initialize ASL   */
        asl_flg=TRUE;
        /* Open message catalog file */
        catd = diag_catopen(MF_UMCODE, 0);
}

/*
 * NAME: chk_software_error()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_software_error(error)
int     error;
{
        if(error == -1)
        {
                disp_menu(0x802093,ERR_SW);
                cleanup();
        }

}

/*
 * NAME: tape_get_ready()
 *
 * FUNCTION: This procedure is responsible for determining if the
 *      drive is ready to be  downloaded to. If the drive motor is
 *      not spinning it will be spun up.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (RECOVERY OPERATION:) The only error recovery from any failed SCSI
 *      commands is when the drive motor is not running. If any other
 *      error occur a message is displayed and execution is passed to D.C..
 *
 * RETURNS: NONE
 */

void tape_get_ready() {
        int     rc;
        int     i;
        unsigned char sense_dat[255];
        struct sc_iocmd iocmd;

        disp_menu(0x802390,REMOVE_TAPE);

        for (i = 0; i < 3; i++)
        {
                (void)memset(sense_dat,0,sizeof(sense_dat));
                (void)memset(&iocmd,0,sizeof(iocmd));

                /* Send A TU Ready command to the drive*/
                rc = do_scsi(fdes, TU_READY, &iocmd, NULL);

		switch(rc) {
		case SC_CHECK_CONDITION:
                        rc = do_scsi(fdes,REQUEST_SENSE,&iocmd,sense_dat);

			if (rc != SC_GOOD_STATUS) {
                                hardware_error(); /* Does not return! */
			}

			if (sense_dat[2] == 0x2) {
				/* Not ready, no tape in drive. */
				/* This is what we expect.      */
				return;
			} else if (i < 3) {
				/* Retry */
				break;
			} else {	
                                hardware_error(); /* Does not return! */
			}
			break;
		case SC_GOOD_STATUS: 
			/* Display "tape in drive" msg. Does not return! */
                        disp_menu(0x802391,TAPE_IN_DRIVE); 
			break;
		default:
			if (i == 0) {
				/* Give device time to recover if */
				/* a reset occurred. Then retry.  */
				sleep(20); 
				break;
			}
                        hardware_error(); /* Does not return! */
			break;
		}
        }

	return;

} /* end tape_get_ready() */

/*
 * NAME: config_initial_state()
 *
 * FUNCTION: Designed to put the device in the available state and
 *           return the device to the original state when we're done
 *           testing. If the device would not change to the available
 *           state software error will be created.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

config_initial_state(parent_name,child_name,clean_flg)
char    *parent_name;
char    *child_name;
int     clean_flg;
{
        static  int     parent_state=-1;
        static  int     child_state=-1;
        static  short   config_flg=0;
        int     rc=0;

        if (debug) {
        fprintf(fdebug,"%d: parent = %s, child = %s, clean_up flag = %d\n",
                __LINE__,parent_name,child_name,clean_flg);
        fflush(fdebug);
        }

        if (!config_flg && !clean_flg)
        {
                /* Make sure both SCSI adapter and device are configured */
                parent_state = configure_device(parent_name);

                if (debug) {
                fprintf(fdebug,"%d: configure_device(%s) returned = %d\n",
                        __LINE__,parent_name,parent_state);
                fflush(fdebug);
                }

                chk_software_error (parent_state);

                if(!testing_tape)
                        rc = disp_menu(0x802098,MOTORUP);
                child_state = configure_device(child_name);

                if (debug) {
                fprintf(fdebug,"%d: configure_device(%s) returned = %d\n",
                        __LINE__,child_name,child_state);
                fflush(fdebug);
                }
                chk_software_error (child_state);
        }
        else
        {
                if(child_state >=0)
                        (void)initial_state(child_state,child_name);
                if(parent_state >=0)
                        (void)initial_state(parent_state,parent_name);
        }
        config_flg ^= 1;
}

/*
 * NAME: mcode_unuseable()
 *
 * FUNCTION: Designed to display error message when the microcode is
 *           unuseable.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

mcode_unuseable()
{
        disp_menu(0x802093,ERR_NO_FILE);
        cleanup();
}

/*
 * NAME: hardware_error()
 *
 * FUNCTION: Designed to display error message when there is a hardware
 *           failure.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

hardware_error()
{
        disp_menu(0x802093,ERR_HW);
        cleanup();
}
/*
 * NAME: copy_text
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: 0
 */

copy_text( string_length, buffer, text )
int     string_length;            /* current length of text already in buffer */
char    *buffer;                  /* buffer to copy text into     */
char    *text;                    /* text to be copied            */
{

        int     space_count;
        int     char_positions;
        char    *tmp;
        char    *next_text;

        /*
          determine if length of text string will fit on one line
        */
        char_positions = LINE_LENGTH - string_length;
        if ( char_positions < strlen(text))  {
                /*
                  dont break the line in the middle of a word
                */
                if(text[char_positions] != ' ' && text[char_positions+1] != ' ')
                        while(--char_positions)
                                if(text[char_positions] == ' ')
                                        break;
                if ( char_positions == 0 )
                        char_positions = LINE_LENGTH - string_length;

                strncpy(buffer, text, char_positions+1);
                tmp = buffer + char_positions + 1;
                *tmp++ = '\n';
                next_text = text + char_positions+1;
                while ( *next_text == ' ' )      /* remove any leading blanks */
                        next_text++;
                space_count = string_length;
                while ( space_count-- )
                        *tmp++ = ' ';
                copy_text( string_length, tmp, next_text);
        }
        else
                sprintf(buffer, "%s", text);
}
/*
 * NAME: get_ascsi_mcode()
 *
 * FUNCTION: This procedure is responsible to determine the microcode
 *           name and check is compatible with the adapter. Invoke the
 *           diagnostics application, to download micrcode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 * (RECOVERY OPERATION:) If the diskette can not be read or any other
 *      errors occur the user is displayed an error message prior
 *      to returning to the Diagnostic controller.
 *
 * RETURNS: 0 or 1
 */

int get_ascsi_mcode(vpd_name,adp_type,devname)
char    *vpd_name;
int     adp_type;
char    *devname;
{
        struct  cfg_load diagex_cfg_load =
        {
                "/usr/lib/drivers/diagex",      /* ptr to object module path */
                "",                             /* ptr to substitute libpath */
                0 };                            /* kernel module id          */

        extern  int     sysconfig(int, void *, int);
        char    *path_name;
        int     diagex_rc=0;

	int 	cfg_state = -1;
        int     rc;
        int     exit_status=0;
        struct  stat    file_status;
        int     mcode_found=0;
        char    command[128];
        char    new_name[128];
        char    fname[128];
        long    int     soft_ll=0;
        long    int     vpd_hardware_zb=0;
        long    int     vpd_hardware_ll=0;
        char    pipe_cmd[128];
        char    *end;
        char    *diagbin;
        char    *option[6];
        FILE    *popen();

        load_diskette();

        if(!load_from_diskette )
        {
                memset(pipe_cmd,0,sizeof(pipe_cmd));
                sprintf(pipe_cmd,
			"cd /etc/microcode; %s %c%c%c%c* -print 2>/dev/null",
			FIND, vpd_name[0],vpd_name[1],vpd_name[2],vpd_name[3]);
                if((fptr = popen(pipe_cmd,"r") ) == NULL)
                {
                        if (debug) {
                        fprintf(fdebug,"errno = %d\n",errno);
                        fflush(fdebug);
                        }
                        mcode_unuseable();
                }

                if (debug) {
                fprintf(fdebug,"Loading MICROCODE from bootable media.\n");
                fflush(fdebug);
                }
        }
        else
        {
                if((stat(DOSDIR, &file_status)) == -1 ||
                        (stat(DOSREAD, &file_status)) == -1 )
                {
                        disp_menu(0x802893,NULL);
                        return(-1);
                }
                /* Get a list of the mcode files on the diskette */
		sprintf(command, "%s%s", DOSDIR, " 2>/dev/null");
                fptr = popen(command, "r");
                if (debug) {
                fprintf(fdebug,"Loading MICROCODE from diskette media.\n");
                fflush(fdebug);
                }
        }

        vpd_hardware_ll=get_hex_value(&vpd_name[4],4);
        vpd_hardware_zb=get_hex_value(&vpd_name[9],2);

        if(adp_type==SE || adp_type==DE)
                vpd_hardware_ll |= 0x1000;
        else
                /* integrated SCSI-2 hardware                   */
                vpd_hardware_ll |= 0x2000;

        strcpy(new_name,vpd_name);
        while(fgets(fname,80,fptr) != NULL)
        {
                /* Get rid of the newline character */
                fname[strlen(fname) - 1] = '\0';

                /* Check to see if the mcode files are compatible */
                if((rc = strncmp(vpd_name,fname,4)) == 0 &&
                        fname[11] == vpd_name[11])
                {
                        soft_ll=get_hex_value(&fname[4],4);

                        /* check for compatible hardware and software   */
                        /* if the user selected latest level of software*/
                        /* then pick the latest level of soft. available*/

                        if((vpd_hardware_ll & soft_ll)== vpd_hardware_ll
                                && level == DL_LATEST_LEVEL &&
                                get_hex_value(&fname[9],2) >
                                get_hex_value(&new_name[9],2) )
                        {

                                /* copy Later version file name         */
                                strcpy(new_name,fname);
                                mcode_found=TRUE;
                        }
                        if((vpd_hardware_ll & soft_ll)== vpd_hardware_ll
                                && level == DL_PREV_LEVEL &&
                                get_hex_value(&fname[9],2) <
                                get_hex_value(&vpd_name[9],2) )
                        {
                                mcode_found=TRUE;
                                if(strcmp(new_name,vpd_name))
                                {
                                        if(get_hex_value(&new_name[9],2)
                                           < get_hex_value(&fname[9],2))
                                                strcpy(new_name,fname);
                                }
                                else
                                        strcpy(new_name,fname);
                        }
                }
        }
        pclose(fptr);

        if(mcode_found <=0)
                mcode_unuseable();

        /* cd to the root dir */
        if((rc = system("cd /")) !=0)
        {
                rc = disp_menu(0x802093,ERR_SW);
                cleanup();
        }

        if(load_from_diskette)
        {

                path_name=calloc(strlen("/tmp/")+strlen(new_name)
                        +2,sizeof(char));
                sprintf(path_name,"/tmp/%s",new_name);
                /* Get the microcode file off of the diskette */
                sprintf(command,"%s %s %s 1>/dev/null 2>&1",
                        DOSREAD, new_name,path_name);
                if (debug) {
                fprintf(fdebug,"%d: command = %s, pathname=%s\n",__LINE__,
                        command,path_name);
                fflush(fdebug);
                }

                if((rc = system(command)) !=0)
                {
                        rc = disp_menu(0x802093,ERR_READING_FILE);
                        cleanup();
                }
        }
        else
        {
                path_name=calloc(strlen("/etc/microcode/")+strlen(new_name)
                        +2,sizeof(char));
                sprintf(path_name,"/etc/microcode/%s",new_name);
        }

        if ( (diagbin = (char *)getenv("DIAG_UTILDIR")) == NULL )
                diagbin = DEFAULT_UTILDIR;
        memset(command,0,sizeof(command));
        sprintf(command,"%s/mcorv",diagbin);

        /* set all the arguments to be passed to the DA         */
        option[0]=command;
        option[1]=calloc(strlen(devname)+2,sizeof(char));
        sprintf(option[1],"%s",devname);
        option[2]=calloc(strlen(path_name)+2,sizeof(char));
        sprintf(option[2],"%s",path_name);
        option[3]=(char *) NULL;

        /* Microcode download requires loading diagnostics kernel ext.  */

        if((rc=sysconfig(SYS_SINGLELOAD,(void *)&diagex_cfg_load,
                        (int)sizeof(diagex_cfg_load))))
        {
                disp_menu(0x802894,ERR_SW);
                return(-1);
        }

        rc=diag_asl_execute(command,option,&exit_status);
        if( (exit_status & 0xff) || rc < 0)
                disp_menu(0x802894,ERR_SW);
        else
        if((exit_status=exit_status>>8) !=2)
                disp_menu(0x802895,ERR_ADAPTER_BUSY);

        if(sysconfig(SYS_KULOAD,(void *)&diagex_cfg_load,
                        (int)sizeof(diagex_cfg_load)))
                disp_menu(0x802896,UNLOAD_FAILED);

        if(load_from_diskette)
                unlink(path_name);

	/* Configure and unconfigure the adapter to update CuVPD. */
	cfg_state = configure_device(devname);
	rc = initial_state(cfg_state, devname);

        return(exit_status);

} /* end get_ascsi_umcode() */

/*
 * NAME: mcode_ascsi_name()
 *
 * FUNCTION: This procedure is responsible to construct the microcode
 *           name from the vpd data in the database.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: 0 or 1
 */


mcode_ascsi_name(devname,posid,mcodetype,mcode_name,adap_type)
char    *devname;
char    *posid;
int     mcodetype;
char    *mcode_name;
int     *adap_type;
{
        char                    odm_search_crit[40];   /* odm search criteria */
        char                    *ptr;
        struct CuVPD            *cuvpd;      /* ODM Customized VPD struct */
        struct listinfo         obj_info;
        int                     length;
        int                     rc=0;
        char    *dg="";
        char    dg_field[4]={""};

        (void)memset(odm_search_crit,0,sizeof(odm_search_crit));
        sprintf( odm_search_crit, "name = %s",devname );
        cuvpd = get_CuVPD_list( CuVPD_CLASS, odm_search_crit,&obj_info, 1, 2 );
        if ( (cuvpd == ( struct CuVPD * ) -1 ) ||
                (cuvpd == ( struct CuVPD * ) NULL ) )
        {
                rc = disp_menu(0x802092,ERR_ODM);
                cleanup();
        }

        dg=strstr(cuvpd->vpd,"DG");
        sprintf(dg_field,"%c%c",dg[3],dg[4]);
        *adap_type= atoi(dg_field);

        /* search for LL field then copy the next 4 characters after    */

        memset(mcode_name,0,sizeof(mcode_name));

        sprintf(mcode_name,"%s",posid);
        length=strlen(mcode_name);
        for (rc=0;rc<length;rc++)
                mcode_name[rc]=toupper(mcode_name[rc]);

        ptr=strstr(cuvpd->vpd,"LL");
        strncpy(&mcode_name[length],&ptr[3],4);

        strcat(mcode_name,".");
        if(mcodetype == BOOT_MCODE)
                ptr=strstr(ptr,"ZB");
        else
                ptr=strstr(ptr,"RL");

        strncpy(dg_field,&ptr[3],2);

        sprintf(dg_field,"%02X",get_hex_value(dg_field,2));
        strcat(mcode_name,dg_field);
        length=strlen(mcode_name);

        mcode_name[length] = mcodetype == BOOT_MCODE ? 'B':'M';
        if (debug) {
        fprintf(fdebug,"%d: CORV mcodename = %s,",__LINE__,mcode_name);
        }
}

/*
 * NAME: load_diskette()
 *
 * FUNCTION: This procedure is responsible to ask the user where the
 *           microcode is located (i.e. Hardfile of diskette ).
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 *
 * RETURNS: 0, 1 or 2
 */


load_diskette()
{
        int     diskette_base;
        int     rc;

        (void) ipl_mode(&diskette_base);
        load_from_diskette=TRUE;

        if(!diskette_base)
        {
                rc=disp_menu(0x802399,NULL);
                if(rc== TRUE)
                {
                        /* Prompt for the mcode diskette */
                        rc = disp_menu(0x802094,INSERT);
                }
                else
                        load_from_diskette=FALSE;
        }
        else
                /* Prompt for the mcode diskette */
                rc = disp_menu(0x802094,INSERT);

        /* Display the diskette read standby msg. */
        rc = disp_menu(0x802096,STANDBY_READ);

}

/*
 * NAME: get_hex_value()
 *
 * FUNCTION: This procedure is responsible to convert a strings into a
 *           hex values.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is forked and execed by the diagnostic controller.
 *
 *
 * RETURNS: INT
 */

get_hex_value(string,how_many)
char    *string;
int     how_many;
{
        char    *end;
        char    *tmp;

        tmp=calloc(how_many+1,sizeof(char));

        strncpy(tmp,string,how_many);

        return(strtol(tmp,&end,16));
}
