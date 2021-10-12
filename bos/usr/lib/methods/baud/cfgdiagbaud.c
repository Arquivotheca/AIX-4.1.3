static char sccsid[] = "@(#)45  1.2  cfgbaud.c, bos, bos320 4/14/93 16:40:52";
/*
** FUNCTIONS: cfgbaud     err_exit       build_dds
**
** ORIGINS: ME
*/

#include <stdio.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/mode.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <odmi.h>
#include <sys/xmem.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include "bauddd.h"

#define MKNOD_MODE S_IFMPX|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

extern  int   errno;
extern  int   optind;         /* for getopt function */
extern  char  *optarg;        /* for getopt function */
extern  int   odmerrno;

struct  baud_dds *dds_sp;    /* Device Driver Dependent Data */



main(argc, argv, envp)
   int argc;
   char **argv;
   char **envp;
{

    struct cfg_load load;
    struct cfg_dd dd;
    char   sstring[256];        /* search criteria pointer */
    char   devlognam[20];       /* Device logical name buffer */
    char   *logical_name;    /* Name:  baudx */
    struct  stat    buf;        /* Statistics buffer of /dev entry */

    struct Class *cusdev;       /* customized devices class ptr */
    struct Class *predev;       /* predefined devices class ptr */
    struct Class *cusvpd;       /* customized vpd class ptr */

    struct CuDv cusobj;         /* customized device object storage */
    struct PdDv preobj;         /* predefined device object storage */
    struct CuDv parobj;         /* customized device object storage */
    struct CuDv dmyobj;         /* customized device object storage */

    int    majorno;             /* major number assigned to device */
    int    minorno;             /* minor number assigned to device */
    long   *minor_list;         /* list returned by getminor */
    int    how_many;            /* number of minors in list */
    int    ipl_phase;           /* ipl phase: 0=run,1=phase1,2=phase2 */
    int    rc;                  /* return codes go here */
    int    errflg,c;            /* used in parsing parameters   */

    ipl_phase = RUNTIME_CFG;
    errflg = 0;
    logical_name = NULL;


    while ((c = getopt(argc,argv,"l:12")) != EOF) {


        switch (c) {
        case 'l':
            if (logical_name != NULL)
                errflg++;
            logical_name = optarg;
            break;
        case '1':
            if (ipl_phase != RUNTIME_CFG)
                errflg++;
            ipl_phase = PHASE1;
            break;
        case '2':
            if (ipl_phase != RUNTIME_CFG)
                errflg++;
            ipl_phase = PHASE2;
            break;
        default:
            errflg++;
        }
    }

    if (errflg)
        /* error parsing parameters */
        exit(E_ARGS);

    if (logical_name == NULL)           /* logical name must be specified */
        exit(E_LNAME);

    if (odm_initialize() == -1)         /* start up odm */
        /* initialization failed */
        exit(E_ODMINIT);

    if (odm_lock("/etc/objrepos/config_lock",0) == -1)  /* lock the database */
        err_exit(E_ODMLOCK);

    /* open customized devices object class */
    if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
        err_exit(E_ODMOPEN);

    /* search for customized object with this logical name */
    sprintf(sstring, "name = '%s'", logical_name);
    if ((rc = (int)odm_get_first(cusdev,sstring,&cusobj)) == 0) {
        err_exit(E_NOCuDv)         /* No CuDv object with this name */;
    } else if (rc == -1) {
        /* ODM failure */
        err_exit(E_ODMGET);
    }

    /* open predefined devices object class */
    if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1)
        err_exit(E_ODMOPEN);

    /* get predefined device object for this logical name */
    sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
    rc = (int)odm_get_first(predev, sstring, &preobj);
    if (rc==0) {
        /* No PdDv object for this device */
        err_exit(E_NOPdDv);
    } else if (rc==-1) {
        /* ODM failure */
        err_exit(E_ODMGET);
    }

    /* close predefined device object class */
    if (odm_close_class(predev) == -1)
        err_exit(E_ODMCLOSE);

    /******************************************************************
      Check to see if the bauddd is already configured (AVAILABLE).
      We actually go about the business of configuring the bauddd
      only if it is not configured yet. Configuring bauddd
      refers to the process of checking parent status, loading the
      device driver, allocating major and minor numbers, creating
      special files, building DDS, etc....
     ******************************************************************/

    if (cusobj.status == DEFINED) {     /* bauddd is not configured */
        /* get the device's parent object */
        sprintf(sstring, "name = '%s'", cusobj.parent);
        rc = (int)odm_get_first(cusdev,sstring,&parobj);
        if (rc==0) {
            /* Parent device not in CuDv */
            printf(" No parent device found \n");
            err_exit(E_NOCuDvPARENT);

        } else if (rc==-1) {
            /* ODM failure */
            printf(" No parent device found \n");
            err_exit(E_ODMGET);
        }


        /* Parent MUST be available to continue */
        if (parobj.status != AVAILABLE)
            /* parent is not AVAILABLE */
            err_exit(E_PARENTSTATE);

        /* make sure that no other devices are configured     */
        /* at this location                                   */
        sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status = %d",
            cusobj.parent, cusobj.connwhere, AVAILABLE);
        rc = (int)odm_get_first(cusdev,sstring,&dmyobj);
        if (rc == -1) {
            /* odm failure */
            err_exit(E_ODMGET);
        } else if (rc) {
            /* Error: device config'd at this location */
            err_exit(E_AVAILCONNECT);
        }

        /* call loadext to load the device driver */
        if ((dd.kmid = loadext("/etc/drivers/bauddiagdd", TRUE, FALSE)) == NULL)
            {
            /* error loading device driver */
            dd.kmid = loadext("/etc/drivers/bauddiagdd", FALSE, TRUE);
            err_exit(E_LOADEXT);
            };


        /* get major number      */
        if ((majorno = genmajor("bauddiagdd")) == -1) {
            (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
            err_exit(E_MAJORNO);
        }

        /* get minor number      */
        minor_list = getminor(majorno,&how_many,logical_name);
        if (minor_list == NULL || how_many == 0) {
            /* Need to allocate minor numbers */
            minor_list = genminor(logical_name, majorno, -1, 1, 1, 1);
            if (minor_list == NULL) {
                /* the genminor service failed */
                (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
                err_exit(E_MINORNO);
            }
        }

        minorno = *minor_list;

        /* create devno for this device */
        dd.devno = makedev(majorno, minorno);

        /* build the DDS  */
        rc = build_dds(logical_name, &dd.ddsptr, &dd.ddslen, dd.devno);
        if (rc) {
            /* error building dds */
            (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
            err_exit(rc);
        }


        /* make special files      */
        sprintf(devlognam, "/dev/%s", logical_name);
        if(stat(devlognam, &buf)) {
            /* stat failed, check that reason is ok */
            if( errno != ENOENT ) {
                DEBUG2("sysconfig: stat failed\n", errno);
                (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
                err_exit(E_MKSPECIAL);
            }

            /* file does not exist, so make it */
            if (mknod(devlognam, MKNOD_MODE, dd.devno) < 0)  {
                DEBUG2("sysconfig: mknod failed\n", errno);
                (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
                err_exit(E_MKSPECIAL);
            }

        } else {                /* stat succeeded, so file already exists */

            if (buf.st_rdev != dd.devno) { /* major/minor #s are not same */
                if (unlink(devlognam)) {   /* unlink special file name */
                    DEBUG2("sysconfig: mknod failed\n", errno);
                    (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
                    err_exit(E_MKSPECIAL);
                }
                if (mknod(devlognam, MKNOD_MODE, dd.devno) < 0)  {
                    DEBUG2("sysconfig: mknod failed\n", errno);
                    (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
                    err_exit(E_MKSPECIAL);
                }
            }
        }

        /* change mode of special file.  This is not in the same step as   */
        /* creating the special file because mknod applies umask setting.  */

        if (chmod(devlognam, MKNOD_MODE)) {
            DEBUG2("sysconfig: chmod failed\n", errno);
            (void) loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
            err_exit(E_MKSPECIAL);
        }


        /* call sysconfig to pass DDS to driver */
        dd.cmd = CFG_INIT;
        if (sysconfig(SYS_CFGDD, &dd, sizeof(struct cfg_dd )) == -1) {
            /* error configuring device */
            (void)loadext("/etc/drivers/bauddiagdd",FALSE,FALSE);
            free(dds_sp);
            err_exit(E_CFGINIT);
        }

        free(dds_sp);                /* No longer need this */

        /* If there was microcode to download we would do it here!!!! */
        /* and if there was Vital Product Data to get do it here!!!! */

        /* update customized device object with a change operation */
        cusobj.status = AVAILABLE;
        if (odm_change_obj(cusdev, &cusobj) == -1)
            /* ODM failure */
            err_exit(E_ODMUPDATE);


    } /* end if (audio device is not AVAILABLE) then ... */

    /* close customized device object class */
    if (odm_close_class(cusdev) == -1)
        err_exit(E_ODMCLOSE);

    odm_terminate();
    exit(E_OK);
}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:     None
 */

err_exit(exitcode)
char    exitcode;
{
    /* Close any open object class */
    odm_close_class(CuDv_CLASS);
    odm_close_class(PdDv_CLASS);
    odm_close_class(CuAt_CLASS);

    /* Terminate the ODM */
    odm_terminate();
    exit(exitcode);
}

/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   build_dds will allocate memory for the dds structure, reporting any
 *   errors, then open the Customized Attribute Class to get the attribute
 *   objects needed for filling the dds structure.
 *
 * RETURNS:
 *    0 on success
 *    positive return code on failure
 */

build_dds(lname, dds_ptr, dds_length, devno)
char    *lname;                 /* logical name of device */
char    **dds_ptr;              /* pointer to dds structure for return */
long    *dds_length;            /* pointer to dds structure size */
dev_t   devno;                  /* device number */
{
    /*
     * This function builds the dds for the baud device by getting the values
     * needed to complete the structure from the predefined and customized
     * attribute object classes. If an attribute can't be found, or another
     * error occurs, this function will return the appropriate error code. If
     * the dds gets built completely, then its address is returned to the
     * caller using the dds_out pointer and its size is returned through
     * the size pointer.
     */

    char sstring[80];
    int rc, count;
    struct Class *cusdev;
    struct CuAt *CuAt_ptr;        /* value to be returned from getattr */
    struct CuDv CuDv;             /* customized device object storage */


    *dds_length = sizeof(struct baud_dds);
    if((dds_sp = (struct baud_dds *) malloc(*dds_length)) == NULL) {
        return(E_MALLOC);
    } else {
        *dds_ptr = (char *)dds_sp;
    }


    /* set default value for all attributes to zero */
    bzero(dds_sp, *dds_length);

    if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
        free(dds_sp);
        err_exit(E_ODMOPEN);
    }

    /* search for customized object with this logical name */
    sprintf(sstring, "name = '%s'", lname);
    if ((rc = (int)odm_get_first(cusdev,sstring,&CuDv)) == 0) {
        free(dds_sp);
        err_exit(E_NOCuDv)         /* No CuDv object with this name */;
    } else if (rc == -1) {
        /* ODM failure */
        free(dds_sp);
        err_exit(E_ODMGET);
    }

    /* Use getattr() to obtain values from ODM */

    if((CuAt_ptr = getattr(CuDv.parent, "bus_id", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->bus_id = strtoul(CuAt_ptr->value, (char **)NULL, 0);

    DEBUG2(">>>> %x\n", dds_sp->bus_id);

    dds_sp->slotno = strtoul(CuDv.connwhere, (char **)NULL, 0);

    if((CuAt_ptr = getattr(lname, "cap_dma_level", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->cap_dma_lvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);

    if((CuAt_ptr = getattr(lname, "play_dma_level", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->play_dma_lvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);


    if((CuAt_ptr = getattr(lname, "bus_intr_level", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->bus_intr_lvl = strtoul(CuAt_ptr->value, (char **)NULL, 0);


    if((CuAt_ptr = getattr(lname, "bus_io_addr", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->bus_io_addr = strtoul(CuAt_ptr->value, (char **)NULL, 0);

    if((CuAt_ptr = getattr(lname, "dma_bus_mem", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->bus_mem_addr = strtoul(CuAt_ptr->value, (char **)NULL, 0);


    if((CuAt_ptr = getattr(lname, "intr_priority", FALSE, &count)) == NULL )
        return(E_NOATTR);
    dds_sp->intr_priority = strtoul(CuAt_ptr->value, (char **)NULL, 0);


    strncpy(dds_sp->lname, lname, strlen(lname));

    DEBUG1("Successfully build DDS\n");
    return 0;

}
