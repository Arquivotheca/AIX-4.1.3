static char sccsid[] = "@(#)22	1.24  src/bos/diag/lscfg/lscfg.c, lscfg, bos411, 9434B411a 8/23/94 15:56:16";
/*
 * COMPONENT_NAME: DUTIL - Diagnostic Utility
 *
 * FUNCTIONS:   main
 *              build_devices
 *              get_device_list
 *              build_vpd
 *              copy_vpd
 *              mergetext
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <limits.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include "diag/class_def.h"
#include "diag/diag.h"
#include "lscfg_msg.h"
#include "diag/diagvpd.h"
#include <locale.h>

#define DELETED_DEVICE '-'
#define NOT_SUPPORTED  '*'
#define TESTABLE_DEV   '+'
#define MAN_ENTERED     DIAG_TRUE
#define NOT_MAN_ENTERED DIAG_FALSE
#define ODM_FAILURE -1
#define MAX_DEV_TEXT 256
#define DEV_TEXT_FMT "%c %-16.16s  %-16.16s  "
#define VPD_HDR_FMT  "  %-16.16s  %-16.16s  %s\n\t\n"
#define MSGSTR(num,str) catgets(fdes,MS_LSCFG,num,str)  /*MSG*/

/* Globally Defined Variables */
nl_catd         fdes;
typedef struct dev_info_s {
        char        *dev_text;
        char        *vpd_text;
        struct diag_dev_info_s  *device;
} dev_info_t, *dev_info_ptr_t;

int     vpd_flag;
char    devicename[NAMESIZE];

/* Externally Defined Variables */
extern char *optarg;

/* Called Functions */
extern char     *malloc();
extern nl_catd catopen( char *, int );
extern int catclose( nl_catd );
extern int odm_initialize();
extern int odm_terminate();
diag_dev_info_ptr_t     *get_device_list();
diag_dev_info_ptr_t     *init_db();
dev_info_t              *build_devices();

/*  */
/*
 * NAME: main
 *
 * FUNCTION: Display VPD data for installed resource(s).
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This program executes as an application of the kernal. It
 *      executes under a process, and can page fault.
 *
 * NOTES:
 *
 *      lsvpd [-l logical device ] [-v]
 *
 *      -l logical device    Display device information for the logical device
 *      -v                   Display VPD information for all logical devices
 *
 *      If no options are given, then a list of all devices is generated.
 *
 * RETURNS:
 *      0 : No errors
 *      1 : Error occurred
 */

main (argc, argv)
int argc;
char *argv[];
{
        int c, num_devices, i, rc;
        dev_info_t *dev_info;

        setlocale(LC_ALL, "");

        /* Allow options to be specified w/o the '-'. (ala Berkeley) */
        if (argv[1][0] != '-') {
                char *bcp;

                argv[argc] = 0;
                argv++;
                for (bcp = *argv++; *bcp; bcp++)
                        switch(*bcp) {
                                case 'v':
                                        vpd_flag++;
                                        break;
                                case 'l':
                                        if (*argv == 0) {
                                                fprintf(stderr, MSGSTR( LFLAG,
                                                        "lscfg: devicename must be specified with 'l' option\n"));
                                                usage();
                                        }
                                        strcpy( devicename, *argv);
                                        argv++;
                                        break;
                                case 'h':
                                        usage();
                                        exit(1);
                                default :
                                        usage();
                                        exit(1);
                        }
        }
        else
        while((c = getopt(argc,argv,"vl:h")) != EOF) {
                switch(c) {
                        case 'v':
                                vpd_flag++;
                                break;
                        case 'l':
                                if (*optarg == 0) {
                                        fprintf(stderr, MSGSTR( LFLAG,
                                                "lscfg: devicename must be specified with 'l' option\n"));
                                        usage();
                                }
                                strcpy(devicename,optarg);
                                break;
                        case 'h':
                                usage();
                                exit(1);
                        default :
                                usage();
                                exit(1);
                }
        }

        fdes = catopen( MF_LSCFG,NL_CAT_LOCALE );

        /* get device's data from the customized device object class */
        if ((dev_info = build_devices(&num_devices)) == (dev_info_t *) NULL)
                exit(1);

        /* write out the headings */
        if ( strlen(devicename) )
                fprintf( stdout, VPD_HDR_FMT,
                        catgets( fdes, VPD_SET, VPD_DEVICE, "DEVICE" ),
                        catgets( fdes, VPD_SET, VPD_LOCATION, "LOCATION" ),
                        catgets( fdes, VPD_SET, VPD_DESC, "DESCRIPTION" ));
        else if ( vpd_flag )
                fprintf( stdout, "%s\n\t\n",
                        catgets( fdes, VPD_SET, VPD_TITLE,
           "INSTALLED RESOURCE LIST WITH VPD\n\n\
The following resources are installed on your machine." ));
        else
                fprintf( stdout, "%s\n\t\n",
                        catgets( fdes, INST_SET, INST_TITLE,
          "INSTALLED RESOURCE LIST\n\n\
The following resources are installed on the machine.\n\
+/- = Added or deleted from Diagnostic Test List.\n\
*   = Diagnostic support not available." ));

        /* now fill in the data part */
        for (i = 0; i < num_devices; i++, dev_info++ ){
                fprintf( stdout, "%s\n", dev_info->dev_text );
                if ( strlen(dev_info->vpd_text) )
                        fprintf( stdout, "\n%s\n", dev_info->vpd_text );

        }

        catclose(fdes);

        exit(0);

}

/*  */
/*
 * NAME: build_devices
 *
 * FUNCTION: build list of device names, descriptions, vpd data
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      dev_info_t * : pointer to array of installed devices
 *      dev_info_t * -1: error
 */

dev_info_t *
build_devices( num_devices )
int        *num_devices;
{
        int                     rc, i;
        diag_dev_info_ptr_t     *All;
        dev_info_t              *dev_info;
        char                    flag;

        /* initialize ODM */
        rc = odm_initialize();
        if(!getenv("ODMDIR"))
                odm_set_path(DEFAULTOBJPATH);

        /* get a list of devices */
        All = get_device_list( num_devices );

        /* allocate master array of devices */
        dev_info = (struct dev_info_s *)
                        calloc(*num_devices, sizeof(struct dev_info_s));

        for ( i = 0; i < *num_devices; i++ ){

                dev_info[i].device = All[i];

                /* alloc space for device name, location and description */
                dev_info[i].dev_text = malloc( MAX_DEV_TEXT );

                /* determine flag value */
                if ( vpd_flag  || strlen(devicename) )
                        flag = ' ';
                else if ( !dev_info[i].device->T_PDiagDev )
                        flag = NOT_SUPPORTED;
                else if ( dev_info[i].device->T_CDiagDev->RtMenu & RTMENU_DDTL )
                        flag = DELETED_DEVICE;
                else
                        flag = TESTABLE_DEV;
                sprintf( dev_info[i].dev_text, DEV_TEXT_FMT, flag,
                    dev_info[i].device->T_CuDv->name,
                    dev_info[i].device->T_CuDv->location, "%s" );

                mergetext( strlen(dev_info[i].dev_text),
                           &dev_info[i].dev_text[strlen(dev_info[i].dev_text)],
                           dev_info[i].device->Text );

                /* generate VPD text */
                if ( vpd_flag )
                        if ((rc = build_vpd( &dev_info[i] )) != 0)
                                return((dev_info_t*)rc);
        }

        rc = odm_terminate();
        return(dev_info);
}

/*  */
/*
 * NAME: get_device_list
 *
 * FUNCTION: Gets list of all devices in system
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *      diag_dev_info_ptr_t * : pointer to list of devices
 */

diag_dev_info_ptr_t *
get_device_list( num_devices )
int        *num_devices;
{
        int                     i, index, length;
        diag_dev_info_ptr_t     *device_list;
        diag_dev_info_ptr_t     *ALL;

        /* get and sort all devices     */
        if (( device_list = init_db(num_devices))==(diag_dev_info_ptr_t *)-1 )
                return((diag_dev_info_ptr_t*)-1);

        /* if a logical device name was not given, return entire list   */
        if ( !strlen(devicename) )
                return( device_list);

        /* else search for the device(s)        */
        ALL=(diag_dev_info_ptr_t *)calloc(*num_devices,sizeof(ALL[0]));
        length = 0;
        if ( strstr(devicename, "*"))
                length = strcspn(devicename, "*");
        for ( i = 0, index = 0; i < *num_devices; i++ ) {
                if ( length > 0 ) {
                        if ( !strncmp(device_list[i]->T_CuDv->name,
                                                      devicename,length) ) {
                                ALL[index++] = device_list[i];
                        }
                }
                else if ( !strcmp(device_list[i]->T_CuDv->name,
                                                      devicename) )
                                ALL[index++] = device_list[i];
        }
        *num_devices = index;
        if (!index)
                fprintf( stderr, MSGSTR( NODEV,
                        "lscfg: device %s not found.\n"), devicename);

        return(ALL);
}
/*  */
/*
 * NAME: build_vpd
 *
 * FUNCTION: Construct vpd data for each device
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

int build_vpd( dev_info )
dev_info_t *dev_info;
{
        int bsz = 2 * VPDSIZE;

        /* allocate output buffer */
        dev_info->vpd_text = (char *)calloc(1,bsz);
        memset(dev_info->vpd_text,0,bsz);

        /* put VPD in single buffer */

        if(dev_info->device->CuVPD_HW) {
                copy_vpd(dev_info->device->CuVPD_HW->vpd,
                                &dev_info->vpd_text,&bsz,NOT_MAN_ENTERED);
        }
        if(dev_info->device->CuVPD_USER)
                copy_vpd(dev_info->device->CuVPD_USER->vpd,
                                &dev_info->vpd_text,&bsz,MAN_ENTERED);

        return(0);
}

/*  */
/*
 * NAME: copy_vpd
 *
 * FUNCTION: Construct vpd data for each device
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

int copy_vpd( inbuf_ptr, ob_ptr, obsz, me_flg )
char    *inbuf_ptr;
char    **ob_ptr;
int     *obsz; /* size of outbuf_ptr */
int     me_flg;         /* manually entered flag        */
{
        int     sz_to_be;
        int     val;
        int     cnt;
        char    *outbuf_ptr = *ob_ptr + strlen(*ob_ptr);
        char    *obptr = outbuf_ptr; /* beg. of outbuf_ptr[] */
        char    *vpdvalue, *vpdptr, *tmp, *text;
        char    specific[126];
        VPDBUF  vbuf;
        char    *vptr;
        static char *defaultvpd[] = {
                "Addressing Field............",
                "Adapter Type................",
                "Adapter Card ID.............",
                "Date Code...................",
                "Device Driver Level.........",
                "Diagnostic Level............",
                "Drawer Level................",
                "Displayable Message.........",
                "Drawer Unit.................",
                "EC Level....................",
                "Feature Code................",
                "FRU Number..................",
                "Loadable Microcode Ptr......",
                "Loadable Microcode Level....",
                "Location....................",
                "Manufacturer................",
                "Network Address.............",
                "Next Adapter VPD Ptr........",
                "Processor Component ID......",
                "Processor Identification....",
                "Part Number.................",
                "ROS Code on Adapter Ptr.....",
                "ROS Level and ID............",
                "System Unit Name............",
                "Read/Write Register Ptr.....",
                "Slot Location...............",
                "Serial Number...............",
                "Size........................",
                "Machine Type and Model......",
                "User Data...................",
                "VPD Extended Data Ptr.......",
                "Device Specific.(%c%c)........"};


        vpdvalue = malloc(VPDSIZE);

        parse_vpd(inbuf_ptr,&vbuf,0);
        for(cnt = 0; cnt < vbuf.entries; cnt++) {
                memset(vpdvalue,0,VPDSIZE);
                vpdptr = vpdvalue;
                vptr = vbuf.vdat[cnt];
                if ( me_flg == DIAG_TRUE ) {
                        *vpdptr++ = 'M';
                        *vpdptr++ = 'E';
                }
                strcat(vpdptr,(vptr+4));
                tmp = outbuf_ptr;
                val = get_msgoffset(vptr);

                text = catgets( fdes, SET0,val, defaultvpd[val-1] );

		/* Add 1 byte for terminating string and 8 for tab */
		/* sz_to_be is the length of the obptr + length of */
		/* vpd data + length of description of vpd and tab */

                sz_to_be = strlen(obptr) + strlen(vpdvalue) + strlen(text) +9 ;
                if ( sz_to_be > *obsz) {
                        int new_size = ( sz_to_be/VPDSIZE + 1) * VPDSIZE;
                        char *new_ptr;
                        int ofset = abs(outbuf_ptr - obptr);

                        new_ptr = (char *)realloc((void *)obptr,new_size);
                        tmp = outbuf_ptr = &new_ptr[ofset];
                        obptr = new_ptr;
                        *obsz = new_size;
                        *ob_ptr = new_ptr;
                }
                if( val == Z0_MSG) {
                        sprintf(specific,text,*(vptr+1),*(vptr+2));
                        sprintf(outbuf_ptr,"        %s", specific);
                }
                else
                        sprintf(outbuf_ptr,"        %s", text);

                tmp += strlen(outbuf_ptr);
                mergetext( strlen(outbuf_ptr), tmp, vpdvalue );
                strcat(outbuf_ptr,"\n");
                outbuf_ptr += strlen(outbuf_ptr);
        }
        free_vbuf(&vbuf);
        free(vpdvalue);

        return(0);
}

get_msgoffset(vptr)
char *vptr;
{
        int cnt;
        int stop = Z0_MSG-AD_MSG;
        ++vptr;
        for(cnt=0; cnt < stop ; cnt++) {
                if(!strncmp(vptr,vpd_data[cnt].keyword,2))
                        return(cnt+AD_MSG);
        }
        return(cnt+AD_MSG);
}

/*   */
/*
 * NAME: mergetext
 *
 * FUNCTION: Adjust wraparound of text on screen
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

mergetext( string_length, buffer, text )
int     string_length;  /* current length of text in buffer     */
char    *buffer;        /* buffer to append text to             */
char    *text;          /* text to be appended                  */
{
        int     i;
        int     space_count;
        int     char_positions;

        /* determine if length of text string will fit on one line */
        char_positions = LINE_LENGTH - string_length;
        if ( char_positions < strlen(text))  {

                /* dont break the line in the middle of a word */
                if( text[char_positions] != ' ')
                        while ( --char_positions )
                                if( text[char_positions] == ' ')
                                        break;
                if ( char_positions == 0 )
                        char_positions = LINE_LENGTH - string_length;

                for ( i = 0; i < char_positions; i++, buffer++, text++ )
                        *buffer = ( *text == '\n' ) ? ' ' : *text ;
                *buffer++ = '\n';
                while ( *text == ' ' )   /* remove any leading blanks */
                        text++;
                space_count = string_length;
                while ( space_count-- )
                        *buffer++ = ' ';
                mergetext( string_length, buffer, text);
        }
        else
                sprintf(buffer, "%s", text);

        return(0);
}

usage()
{
        fprintf( stderr, MSGSTR( USAGE1,
                "Usage: lscfg [-l logical device ] [-v]\n"));
        exit(1);
}

