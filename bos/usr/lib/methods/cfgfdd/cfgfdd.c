#ifndef lint
static char sccsid[] = "@(#)34 1.24 src/bos/usr/lib/methods/cfgfdd/cfgfdd.c, cfgmethods, bos41J, 9513A_all 3/28/95 14:19:19";
#endif
/*
 *   COMPONENT_NAME: (CFGMETHODS) Diskette Device config method
 *
 *   FUNCTIONS: main
 *		build_dds
 *		generate_minor
 *		make_special_files
 *		
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>
#include <stdio.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/fd.h>
#include <sys/i_machine.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/pmdev.h>

#include "cfgcommon.h"
#include "cfgdebug.h"

extern  struct  CuAt    *getattr();


#define RWPERMS 0666


#define GETATT( DEST, TYPE, ATTR, CUDV ) {				\
		int rc;							\
		rc = getatt( DEST, TYPE, CuAt_CLASS, PdAt_CLASS,	\
			CUDV.name, CUDV.PdDvLn_Lvalue, ATTR,		\
			(struct attr *)NULL );				\
		if (rc)							\
			return(rc);					\
	}


/*
 * NAME: build_dds
 *
 * FUNCTION: This function builds the DDS(Defined Data Structure) for the
 *           Diskette Drive.
 *
 * EXECUTION ENVIRONMENT:
 *
 * This function operates as a device dependent subroutine called by the
 * generic configure method for all devices. It is used to build the dds
 * which describes the characteristics of a device to the device driver.
 *
 * NOTES: A pointer to the DDS built and its size are returned to the generic
 *        configure method.
 *
 * RETURNS: Returns E_OK on success.
 */

build_dds(logical_name,ddsptr,ddslen)
        char    *logical_name;          /* logical_name */
        char    **ddsptr;
        int     *ddslen;
{
        static	struct	fdd_config	dds;	/* dds */
	char	drive_type[256];
	int	rc;


        DEBUG_0("cfgfdd: in build_dds()\n");
	strncpy(dds.resource_name,logical_name,8);

        /* Get drive type */
	GETATT( &drive_type, 's', "fdtype", cudv )
        if (strncmp(drive_type,"3.5inch4Mb",10)==0)
                dds.type = D_1354H;
        else if (strncmp(drive_type,"3.5inch",7)==0)
                dds.type = D_135H;
        else dds.type = D_96;

	/* get power mgt attributes (only applicable to ISA driver) */
	rc = getatt (&dds.pm_device_id, 'i', CuAt_CLASS, PdAt_CLASS, 
		     cudv.name, cudv.PdDvLn_Lvalue, "pm_devid", 
		     (struct attr *)NULL );

	/* if the attribute isn't present, set a default value */
	/* otherwise, return an error			       */
	if (rc == E_NOATTR)
		dds.pm_device_id = PMDEV_UNKNOWN_OTHER;
	else if (rc != 0)
		return (rc);

	rc = getatt (&dds.device_idle_time, 'i', CuAt_CLASS, PdAt_CLASS, 
		     cudv.name, cudv.PdDvLn_Lvalue, "pm_dev_itime", 
		     (struct attr *)NULL );

	/* if the attribute isn't present, set a default value */
	/* otherwise, return an error			       */
	if (rc == E_NOATTR)
		dds.device_idle_time = 0;
	else if (rc != 0)
		return (rc);

	rc = getatt (&dds.device_standby_time, 'i', CuAt_CLASS, PdAt_CLASS, 
		     cudv.name, cudv.PdDvLn_Lvalue, "pm_dev_stime", 
		     (struct attr *)NULL );

	/* if the attribute isn't present, set a default value */
	/* otherwise, return an error			       */
	if (rc == E_NOATTR)
		dds.device_standby_time = 0;
	else if (rc != 0)
		return (rc);

	dds.pm_attribute = 0;

        DEBUG_2("build_dds: type = %d (%s)\n", dds.type, drive_type);
        DEBUG_2("build_dds: pm_device_id = %x  device_idle_time = %d\n", 
		dds.pm_device_id, dds.device_idle_time);
        DEBUG_2("build_dds: device_standby_time = %d  pm_attribute = %d\n", 
		dds.device_standby_time, dds.pm_attribute);

	/* set drive characteristics based on drive type retrieved from ODM */
	if (dds.type == D_96) {
		dds.head_settle = 26; /* all times in milliseconds */
		dds.step_rate = 3;
		dds.motor_start = 750;
	}
	else if (dds.type == D_135H) {
		dds.head_settle = 16;
		dds.step_rate = 6;
		dds.motor_start = 500;
	}
	else {	/* type D_1354H */
		dds.head_settle = 15;
		dds.step_rate = 3;
		dds.motor_start = 500;
	}

        *ddslen = sizeof(struct fdd_config);
	*ddsptr = (char *) &dds;

        DEBUG_0("cfgfdd: leaving mk_fd_config\n");
        return(E_OK);
}



/*
 * NAME: generate_minor 
 * 
 * FUNCTION: This function generates device minor number for the Parallel 
 *           Printer. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to generate device minor number for the specific device by calling
 *      lib function genminor.
 *
 * RETURNS: Returns 0 on SUCCESS, >0 Error code.
 */


int     generate_minor(logical_name,major,minorno)
        char    *logical_name;
        long    major;
        long    *minorno;
{
	char	drive_type[256];
        long    *list1;
        long    *list2;
        int     preferred,number,skip,align;

        /*
        minor numbers indicate drive type, so get current type
        bits in minor number are used to indicate drive type
        3.5     = 0,1,2                or   64,65,66
        3.54M   = 0,1,2,3              or   64,65,66,67
        5.25    = 0,33,34 (0x21,0x22)  or   64,97,98
        */



        /* Get drive type */
	GETATT( &drive_type, 's', "fdtype", cudv )
        DEBUG_1("generate_minor: type = %s\n",drive_type);

        skip = 1;
        if (strcmp(cudv.connwhere, "1") == 0)
                preferred = 64;
        else
                preferred = 0;
        DEBUG_1("generate_minor: connwhere = %s\n",cudv.connwhere);

        if (strcmp(drive_type, "3.5inch") == 0) { /* 3.5" minor numbers */
                number = 3;
                align = 64;
                list1=genminor(logical_name,major,preferred,number,skip,
                    align);
                if (list1 == NULL) {
                        DEBUG_0("cfgfdd: genminor failed");
                        return (E_MINORNO);
                }
                DEBUG_3("cfgfdd: mk_spcl_files: minors = %ld,%ld,%ld\n",
                        *list1,*(list1+1),*(list1+2));
        }  else /* 3.5 4Meg minor numbers */
        if (strcmp(drive_type, "3.5inch4Mb") == 0) { /* 3.5" minor numbers */
                number = 4;
                align = 64;
                list1=genminor(logical_name,major,preferred,number,skip,
                    align);
                if (list1 == NULL) {
                        DEBUG_0("cfgfdd: genminor failed");
                        return (E_MINORNO);
                }
                DEBUG_3("cfgfdd: mk_spcl_files: minors = %ld,%ld,%ld\n",
                        *list1,*(list1+1),*(list1+2));
        }  /* 3.5 4Meg minor numbers */
        else {  /* 5.25" minor numbers */
                number = 1;
                align = 64;
                list1=genminor(logical_name,major,preferred,number,skip,
                    align);
                if (list1 == NULL) {
                        DEBUG_0("cfgfdd: genminor failed");
                        return (E_MINORNO);
                }
                DEBUG_1("cfgfdd: generate_minor: minors = %ld,",*list1);
                preferred = *list1 + 33;
                number = 2;
                list2=genminor(logical_name,major,preferred,number,skip,
                    align);
                if (list2 == NULL) {
                        DEBUG_0("cfgfdd: generate_minor failed");
                        return (E_MINORNO);
                }
                DEBUG_2("%ld,%ld\n",*list2,*(list2+1));
        }  /* 5.25" minor numbers */

        *minorno = *list1;
        return(E_OK);

}  /* generate_minor() */



/*
 * NAME: make_special_file 
 * 
 * FUNCTION: This function creates special file(s) for the parallel printer. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to create special file(s) for the specific device.
 *
 * (NOTES:) 2 special files will be created for the parallel printer.
 *          for examples: /dev/lp0  and /dev/pp0
 *          both of them have the same major and minor numbers.
 *
 *          Make special files for the parallel printer according to MKNOD_MOD:
 *
 *               file type is S_IFCHR: character file,
 *               file permission is 666.
 *
 * RETURNS: Returns 0 on SUCCESS, >0 Error code.
 */

make_special_files(logical_name,devno)
        char    *logical_name;
        dev_t   devno;
{
	char	drive_type[256];
        int     rc;


        DEBUG_2("make_special_files: name = %s   devno = %x\n",
		logical_name, devno);

	GETATT( &drive_type, 's', "fdtype", cudv )
        DEBUG_1("make_special_files: type = %s\n",drive_type);


	/* Make special files common to all drive types */

	/* /dev/fdX and /dev/rfdX */
	rc = make_sf(logical_name, "", 0);
	if (rc) return(rc);

	if (strcmp(drive_type, "5.25inch") == 0) {
		/* Its a 5.25" drive */
		/* Make additional 5.25" special files */

		/* /dev/fdX.15 and /dev/rfdX.15 */
		rc = make_sf(logical_name, ".15", 0x21);
		if (rc) return(rc);

		/* /dev/fdX.9 and /dev/rfdX.9 */
		rc = make_sf(logical_name, ".9", 0x22);
		if (rc) return(rc);

		/* Now delete possible old 3.5" special files */
		/* /dev/fdXh and /dev/rfdXh */
		remove_sf(logical_name, "h");

		/* /dev/fdXl and /dev/rfdXl */
		remove_sf(logical_name, "l");

		/* /dev/fdX.36 and /dev/rfdX.36 */
		remove_sf(logical_name, ".36");

		/* /dev/fdX.18 and /dev/rfdX.18 */
		remove_sf(logical_name, ".18");
	}
	else {
		/* Its a 3.5" drive */
		/* Make common 3.5" special files */

		/* /dev/fdX.18 and /dev/rfdX.18 */
		rc = make_sf(logical_name, ".18", 1);
		if (rc) return(rc);

		/* /dev/fdX.9 and /dev/rfdX.9 */
		rc = make_sf(logical_name, ".9", 2);
		if (rc) return(rc);

		/* /dev/fdXl and /dev/rfdXl */
		link_sf(logical_name, ".9" ,"l");

		/* Make device specific special files */
        	if (strcmp(drive_type, "3.5inch4Mb") == 0) { 
			/* Its a 4MB drive */

			/* /dev/fdX.36 and /dev/rfdX.36 */
			rc = make_sf(logical_name, ".36", 3);
			if (rc) return(rc);

			/* /dev/fdXh and /dev/rfdXh*/
			link_sf(logical_name, ".36" ,"h");
		}
		else {
			/* Its not 4MB */

			/* /dev/fdXh and /dev/rfdXh*/
			link_sf(logical_name, ".18" ,"h");

			/* Need to remove possible 4MB special files */
			remove_sf(logical_name, ".36");
		}

		/* Now delete possible old 5.25" special files */
		/* /dev/fdX.15 and /dev/rfdX.15 */
		remove_sf(logical_name, ".15");
	}

        return(0);
}

download_microcode(logical_name)
        char    *logical_name;
{
        return(E_OK);                   /* no microcode */
}

query_vpd(newobj,kmid,devno,vpd)
        char    *newobj;
        mid_t   kmid;
        dev_t   devno;
        char    *vpd;
{
        return(E_OK);                   /* no vpd on floppy drives */
}

define_children(logical_name,phase)
        char    *logical_name;
        int     phase;
{
        return(E_OK);                   /* terminal device */
}

int device_specific()
{

        return(0);
}

int
make_sf(lname, suffix, minor)

char *lname;
char *suffix;
int  minor;

{
	char	name[32];
	int	rc;

	/* Make block special file */
	sprintf(name,"%s%s",lname,suffix);
	rc = mk_sp_file(devno|minor,name,S_IFBLK | RWPERMS);
	if (rc != 0)
		return(rc);

	/* Make character special file */
	sprintf(name,"r%s%s",lname,suffix);
	rc = mk_sp_file(devno|minor,name,S_IFCHR | RWPERMS);
	return(rc);

}

int
remove_sf(lname,suffix)

char *lname;
char *suffix;

{
	char	name[32];
        struct  stat    stat_buf;


	/* Remove block special file if it exists */
	sprintf(name,"/dev/%s%s",lname,suffix);
	if (stat(name,&stat_buf)==0)
		unlink(name);

	/* Remove character special file if it exists */
	sprintf(name,"/dev/r%s%s",lname,suffix);
	if (stat(name,&stat_buf)==0)
		unlink(name);

	return(0);	
}

int
link_sf(lname,suffix1,suffix2)

char *lname;
char *suffix1;
char *suffix2;

{
	char	name[32];
	char	link_name[32];
        struct  stat    stat_buf;

	/* Link block special file */
	sprintf(name,"/dev/%s%s",lname,suffix1);
	sprintf(link_name,"/dev/%s%s",lname,suffix2);
	link(name, link_name);

	/* Link character special file */
	sprintf(name,"/dev/r%s%s",lname,suffix1);
	sprintf(link_name,"/dev/r%s%s",lname,suffix2);
	link(name, link_name);

	return(0);
}
