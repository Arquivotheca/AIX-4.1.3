#ifndef lint
static char sccsid[] = "@(#)84 1.3 src/bos/usr/lib/methods/cfgtty/rsdd/rsddutil.c, cfgtty, bos41J, 9521B_all 5/26/95 07:49:36";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) 
 *
 * FUNCTIONS: Run_Other_Method
 *            Generate_Minor
 *            Make_Special_Files
 *            Do_Other_Processing
 *            gen_std_minor
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */
#include <string.h>

#include <sys/mode.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */

#include <sys/str_tty.h>

#include "ttycfg.h"
#include "cfgdebug.h"

#include <sys/stat.h>
#include "pparms.h"
#include <ctype.h>
/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define MKNOD_FLAGS   S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define BUSID         "bus_id"
/*
 * =============================================================================
 *                       FUNCTIONS USED BY CONFIGURATION METHOD
 * =============================================================================
 */


int Run_Other_Method(cusdev,parcusdev,buscusdev,parpredev,attrList,ipl_stat)
struct CuDv      *cusdev;     /* customized device data  */
struct CuDv      *parcusdev;  /* parent device customized data  */
struct CuDv      *buscusdev;  /* bus customized data  */
struct PdDv      *parpredev;  /* parent predefined data  */
struct attr_list *attrList;   /* list of attributes  */
int              *ipl_stat;   /* phase of ipl  */

{ return(0); }



int Do_Other_Processing(cusdev,parcusdev,buscusdev,parpredev,attrList)
struct CuDv      *cusdev;     /* customized device data  */
struct CuDv      *parcusdev;  /* parent device customized data  */
struct CuDv      *buscusdev;  /* bus customized data  */
struct PdDv      *parpredev;  /* parent predefined data  */
struct attr_list *attrList;   /* list of attributes  */

{ return(0); }




/*
 * -----------------------------------------------------------------------------
 *                       GENERATE_MINOR
 * -----------------------------------------------------------------------------
 * This function generates the minor number(s) for the tty devices.
 *
 * First, we try to get existing (in ODM database) minors. If none are found,
 * we generate minor numbers and record them in the ODM CuDvDr database.
 *
 * Notes: Minor number is generated for the tty device being configured by
 * applying a special algorithm. The minor number is set in the database
 * by calling genminor() with the minor number to be used.
 *
 * Return code: Exits with 0 on success,
 *              -1 if minor is in use and the method should steal the minor.
 *              ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int Generate_Minor(struct CuDv *devcusobj, struct CuDv *parcusobj,
			struct CuDv *buscusobj, char *drvrname,
			long maj_no, long **minorP)
{
	/* specific routines for getting and generating minor numbers */
	extern int gen_std_minor();

	long *minor_list;
	int  count;
	int  rc;

	DEBUG_0("generate_minor: entered\n");

	/* see if we already have a minor number */
	if ((minor_list = getminor(maj_no,&count,devcusobj->name)) == NULL) {

		/*
		 * no minor numbers found for this device
		 * call the appropriate routine to create
		 * a minor number
		 */

		DEBUG_1("gen_min: no minors found for %s\n",devcusobj->name);


		/* call generic minor number generator */
		DEBUG_0("gen_min: calling gen_std_minor()\n");
		rc = gen_std_minor(devcusobj,parcusobj,buscusobj,
					 maj_no,&minor_list); 
		if (rc > 0){ 
			/* error generating minor number */
			DEBUG_1("gen_std_minor failed, rc=%d\n",rc);
			return(rc);
		}
		if (rc == -1){ 
			DEBUG_0("Generate_Minor: return -1\n");
			/* minor number  in use */
			*minorP = minor_list;
			return(rc);
		}
	}


	DEBUG_1("generate_minor: return OK, minor=%d\n",*minor_list);

	*minorP = minor_list;
	return(0);

} /* end generate_minor(...) */


int gen_std_minor(struct CuDv *cusobj, struct CuDv *parobj, struct CuDv *busobj,
		  long major, long **minP)
{
    extern long *genminor();    /* generate minor number service */
    extern int get_slot();      /* returns slot location */
    extern int steal_minor();   /* handle genminor failure */
    extern int Get_Parent_Bus();/* searches up the device hierarchy until */
                                /* it finds a device with class bus */

    long * minor_ptr;           /* pointer to minor number returned */
    long   minor_no;            /* generated minor number */
    int    rc; 		        /* return code, minor number set */
    int    slot;                /* slot number of parent(adapter) */
    int    port;                /* port number of tty on parent */
    ulong  busid ;              /* bus id that tty is attached to */
    int    how_many;            /* How many attributes are found */

				/* ODM structures declarations */
    struct CuAt *  current_att;     /* current found attribute */


    DEBUG_3("gen_std_minor: name=%s parent=%s major=%d\n",
	    cusobj->name,parobj->name,major);

    /* ============================================================= */
    /* Get port number - Used to tell if configuring sio port A, B or C */
    /* ============================================================= */
    port = atoi(&(cusobj->connwhere[strcspn(cusobj->connwhere,"0123456789")]));

    /* Decrement port number for sio ports */
    /* So they are 0,1,2 instead of 1,2 */
    if (cusobj->connwhere[0] == 's') {
        port--;
    }

    /* =============== */
    /* Get slot number */
    /* =============== */
    if ((rc = get_slot(parobj->connwhere, &slot)) != 0) {
        return(E_NOATTR);
    }
    slot = (slot-1) & 0x0f;

    DEBUG_2("gen_std_minor: slot = %d, port = %d\n", slot, port);



    /* Get bus id */
    if ((current_att = getattr(busobj->name, "bus_id", FALSE, &how_many))
                             == NULL) {
        return(E_NOATTR);
    }
    /*convert values to type ulong*/
    busid = strtoul(current_att->value, (char ** )NULL, 0);  
    DEBUG_1("generate_minor: busid = %ld\n", busid);
    DEBUG_1("generate_minor: current_att->value = %s\n", current_att->value);
    busid = (((busid >> 20) & 0xff) - 0x20);

    /* Generate minor number */
    minor_no = (long) ((busid <<14) | (((slot+1)&0xf)<<8) | port);

    DEBUG_1("generate_minor: minor_no = %ld\n", minor_no);

    /* ========================================= */
    /* Set minor number for tty port in database */
    /* ========================================= */
    DEBUG_0("gen_std_minor: Trying to set in database:\n");
    DEBUG_3("gen_std_minor: deviceName = %s, major= %ld, minor_no = %ld\n",
             cusobj->name, major, minor_no);

    if ((minor_ptr=genminor(cusobj->name, major, minor_no, 1, 1, 1)) == NULL) {
        /* Failed to set minor number requested in CuDvDr Class */
        DEBUG_1("gen_std_minor: genminor failed minor=%ld\n",minor_no);

	/* see if we can steal this minor number */
    	*minP = minor_ptr;
    	return(-1);
    }
	
    /*
     * generated the minor number successfully
     * point the caller to the number and return OK
     */

    *minP = minor_ptr;
    return(0);

} /* End int gen_std_minor(...) */

/*
 * -----------------------------------------------------------------------------
 *                       MAKE_SPECIAL_FILES
 * -----------------------------------------------------------------------------
 * This function makes the special files for the tty devices.
 *
 * This function operates as a device dependent subroutine called 
 * by the configure method for all devices.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int Make_Special_Files(logicalName, driverName, majorNo, minorPtr)
char * logicalName;
char * driverName;
long   majorNo;
long * minorPtr;
{
    int    rc;
    dev_t  devno;
    char   file_name[ATTRVALSIZE];

    DEBUG_4("make_special_files: lname=%s drvr=%s major=%d minor=%d\n",
	    logicalName,driverName,majorNo,*minorPtr);


    devno = makedev(majorNo, *minorPtr);
    if ((rc = mk_sp_file(devno, logicalName, (long) MKNOD_FLAGS)) != 0) {
         DEBUG_1("make_special_files: Error creating %s\n", logicalName);
         return (E_MKSPECIAL);
    }

    /* That's OK */
    return(0);

} 


